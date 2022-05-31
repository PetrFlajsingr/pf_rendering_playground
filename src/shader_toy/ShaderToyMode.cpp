//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyMode.h"
#include "ShaderBuilder.h"
#include "gpu/opengl/Program.h"
#include "gpu/opengl/Shader.h"
#include "gpu/opengl/Texture.h"
#include "gpu/utils.h"
#include "utils/glsl/GlslToSpirv.h"
#include <future>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/glslang_c_interface.h>
#include <pf_common/Visitor.h>
#include <pf_imgui/elements/Image.h>
#include <pf_mainloop/MainLoop.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/trim.hpp>
#include <utility>
#include <utils/opengl_utils.h>
#include <utils/profiling.h>

#include "log/UISink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <gpu/utils.h>

void debugOpengl(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                 const void *) {
  spdlog::error("{} {}, {}, {}, {}", source, type, id, severity, std::string_view{message, message + length});
}

namespace pf::shader_toy {

ShaderToyMode::ShaderToyMode(std::filesystem::path resourcesPath) : configData{std::move(resourcesPath)} {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(debugOpengl, nullptr);
}

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window,
                                    std::shared_ptr<ThreadPool> threadPool) {
  auto isFirstRun = true;
  if (const auto iter = config.find("initialized"); iter != config.end()) {
    isFirstRun = !iter->second.value_or(false);
  }
  config.insert_or_assign("initialized", true);
  glfwWindow = window;
  workerThreads = threadPool;
  imGuiInterface = imguiInterface;

  imageLoader = std::make_shared<OpenGLStbImageLoader>(workerThreads);

  createModels();

  createControllers();

  loadModelsFromConfig();
  if (isFirstRun) {
    mainController->resetDocking(ui::ig::Size{static_cast<float>(glfwWindow->getSize().width),
                                              static_cast<float>(glfwWindow->getSize().height)});
  }

  getLogger().sinks().emplace_back(mainController->logWindowController->createSpdlogSink());

  mainController->hide();

  imGuiInterface->setStateFromConfig();
}

std::vector<std::shared_ptr<spdlog::sinks::sink>> ShaderToyMode::createLoggerSinks() {
  return std::vector<std::shared_ptr<spdlog::sinks::sink>>{std::make_shared<spdlog::sinks::stdout_color_sink_st>()};
}

void ShaderToyMode::activate_impl() {
  resetCounters();
  mainController->show();
  imGuiInterface->setStateFromConfig();
  // TODO: load data from config
  initializeTexture(TextureSize{TextureWidth{models.output->resolution->first},
                                TextureHeight{models.output->resolution->second}, TextureDepth{0u}});
}

void ShaderToyMode::deactivate_impl() {
  imGuiInterface->updateConfig();
  mainController->hide();
  outputTexture = nullptr;
}

void ShaderToyMode::deinitialize_impl() {
  // TODO: remove UI from ImGuiInterface here - via view's destructor?
  mainController = nullptr;
  TimeMeasure workerThreadWaitMeasure;
  getLogger().info("Waiting for worker threads");
  std::ranges::for_each(unfinishedWorkerTasks, &std::future<void>::wait);
  workerThreads = nullptr;
  getLogger().debug("Took {}", workerThreadWaitMeasure.getTimeElapsed());
}

void ShaderToyMode::render(std::chrono::nanoseconds timeDelta) {
  checkShaderStatus();
  if (mainProgram == nullptr) { return; }
  DEBUG_ASSERT(outputTexture != nullptr);

  const auto mouseState = getMouseState();

  const auto timeFloat = static_cast<float>(totalTime.count()) / 1'000'000'000.0f;
  const auto timeDeltaFloat = static_cast<float>(timeDelta.count()) / 1'000'000'000.0f;

  setUniforms(timeFloat, timeDeltaFloat, mouseState);
  setBindings();

  mainProgram->use();

  const auto textureSize = getTextureSize();
  const auto dispatchResult =
      mainProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);
  if (dispatchResult.has_value()) { getLogger().error("Program dispatch failed: '{}'", dispatchResult->message()); }
  fpsCounter.onFrame();
  updateUI();
  if (!timeCounterPaused) { totalTime += timeDelta; }
  ++frameCounter;
}

void ShaderToyMode::resetCounters() {
  getLogger().debug("Resetting frame and time counters");
  frameCounter = 0;
  totalTime = std::chrono::nanoseconds{0};
}

void ShaderToyMode::initializeTexture(TextureSize textureSize) {
  getLogger().info("Updating texture size to {}x{}", textureSize.width.get(), textureSize.height.get());
  outputTexture =
      std::make_shared<OpenGlTexture>(TextureTarget::_2D, TextureFormat::RGBA32F, TextureLevel{0}, textureSize);
  if (const auto err = outputTexture->create(); err.has_value()) {
    // FIXME
    VERIFY(false, "Texture creation failure handling not implemented");
  }
  getLogger().debug("Texture created: {}", *outputTexture);
  outputTexture->setParam(TextureMinificationFilter::Linear);
  outputTexture->setParam(TextureMagnificationFilter::Linear);

  *models.output->texture.modify() = outputTexture;
}

glm::uvec2 ShaderToyMode::getTextureSize() const {
  return {outputTexture->getSize().width.get(), outputTexture->getSize().height.get()};
}

void ShaderToyMode::checkShaderStatus() {
  if (autoCompileShader && isShaderChanged
      && (std::chrono::steady_clock::now() - lastShaderChangeTime) > *models.codeEditor->autoCompilePeriod) {
    if (previousShaderCompilationDone) {
      getLogger().trace("Auto recompiling shader");
      compileShader(*models.codeEditor->code);
      isShaderChanged = false;
    }
  }
}

void ShaderToyMode::compileShader(const std::string &shaderCode) {
  *models.codeEditor->compiling.modify() = true;
  getLogger().trace("Compiling shader");
  compileShader_impl(shaderCode);
}

// TODO: clean this up
void ShaderToyMode::compileShader_impl(const std::string &shaderCode) {
  // clang-format off
  auto builder = ShaderBuilder{};
  builder.addUniform<float>("time")
      .addUniform<float>("timeDelta")
      .addUniform<int>("frameNum")
      .addEnum<MouseState>()
      .addUniform<MouseState>("mouseState")
      .addUniform<glm::vec3>("mousePos")
      .addUniform<glm::vec3>("mousePosNormalized")
      .addImage2D("rgba32f", "outImage")
      .setLocalGroupSize(COMPUTE_LOCAL_GROUP_SIZE);
  // clang-format on

  std::ranges::for_each(models.shaderVariables->getVariables(),
                        [&](const std::shared_ptr<ShaderVariableModel> &variable) {
                          std::string_view typeName{};
                          std::visit(
                              [&]<typename T>(T) {
                                if constexpr (std::same_as<T, ui::ig::Color>) {
                                  typeName = getGLSLTypeName<glm::vec4>();
                                } else {
                                  typeName = getGLSLTypeName<T>();
                                }
                              },
                              *variable->value);
                          builder.addUniform(std::string{typeName}, *variable->name);
                        });

  std::ranges::for_each(models.imageAssets->getTextures(), [&](const std::shared_ptr<TextureAssetModel> &tex) {
    if (*tex->texture == nullptr) { return; }
    const auto texFormat = (*tex->texture)->getFormat();
    std::string formatStr;
    switch (texFormat) {
      case TextureFormat::R8: formatStr = "r8"; break;
      case TextureFormat::RGBA8: formatStr = "rgba8"; break;
      default:
        getLogger().error("Unsupported image type '{}' in ShaderToyMode::compileShader_impl",
                          magic_enum::enum_name(texFormat));
        return;
    }
    builder.addImage2D(formatStr, *tex->name);
  });

  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;

  getLogger().debug("Enqueueing shader compilation");
  previousShaderCompilationDone = false;
  unfinishedWorkerTasks.emplace_back(workerThreads->enqueue([=, this]() mutable {
    getLogger().debug("Shader compilation job started");
    const auto compilationStartTime = std::chrono::steady_clock::now();
    auto spirvResult = glslComputeShaderSourceToSpirv(source);
    const auto compilationDuration = std::chrono::steady_clock::now() - compilationStartTime;
    getLogger().debug("Compilation took {}",
                      std::chrono::duration_cast<std::chrono::milliseconds>(compilationDuration));
    pf::MainLoop::Get()->enqueue([spirvResult = std::move(spirvResult), source = std::move(source), this]() mutable {
      auto onDone = RAII{[this] {
        previousShaderCompilationDone = true;
        *models.codeEditor->compiling.modify() = false;
      }};
      mainController->glslEditorController->clearWarningMarkers();
      mainController->glslEditorController->clearErrorMarkers();
      if (spirvResult.has_value()) {
        auto shader = std::make_shared<OpenGlShader>();
        const auto shaderCreateResult = shader->create(spirvResult.value(), "main");
        if (shaderCreateResult.has_value()) {
          getLogger().error("Shader creation failed:");
          getLogger().error("{}", shaderCreateResult.value().message());
        } else {
          auto newProgram = std::make_unique<OpenGlProgram>(std::move(shader));
          const auto programCreateResult = newProgram->create();
          if (programCreateResult.has_value()) {
            getLogger().error("Program creation failed:");
            getLogger().error("{}", programCreateResult.value().message());
          } else {
            mainProgram = std::move(newProgram);

            totalTime = std::chrono::nanoseconds{0};
            frameCounter = 0;
            currentShaderSrc = std::move(source);
            getLogger().info("Compiling program success");
          }
        }
      } else {
        getLogger().info("Compiling shader failed");
        auto errors = spirvResult.error().getInfoRecords();
        for (SpirvErrorRecord rec : errors) {
          using enum SpirvErrorRecord::Type;
          if (!rec.line.has_value()) { continue; }
          const auto errMessage = fmt::format("{}: {}", rec.error, rec.errorDesc);
          const auto marker =
              ui::ig::TextEditorMarker{static_cast<uint32_t>(shaderLineMapping(rec.line.value())), errMessage};
          getLogger().error("{}", errMessage);
          switch (rec.type) {
            case Warning: mainController->glslEditorController->addWarningMarker(marker); break;
            case Error: mainController->glslEditorController->addErrorMarker(marker); break;
          }
        }
      }
    });
  }));
}

MouseState ShaderToyMode::getMouseState() const {
  if (*models.output->textureHovered) {
    if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Left) == pf::glfw::ButtonState::Down) {
      return MouseState::LeftDown;
    } else if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Right) == pf::glfw::ButtonState::Down) {
      return MouseState::RightDown;
    }
  }
  return MouseState::None;
}

void ShaderToyMode::setUniforms(float timeFloat, float timeDeltaFloat, MouseState mouseState) {
  auto ignoredResult = mainProgram->setUniform("time", timeFloat);
  ignoredResult = mainProgram->setUniform("timeDelta", timeDeltaFloat);
  ignoredResult = mainProgram->setUniform("frameNum", frameCounter);
  ignoredResult = mainProgram->setUniform("mouseState", static_cast<int>(mouseState));
  ignoredResult =
      mainProgram->setUniform("mousePos",
                              glm::vec3{mousePos.x * static_cast<float>(outputTexture->getSize().width.get()),
                                        mousePos.y * static_cast<float>(outputTexture->getSize().height.get()), 0.f});
  ignoredResult = mainProgram->setUniform("mousePosNormalized", glm::vec3{mousePos, 0.f});

  std::ranges::for_each(models.shaderVariables->getVariables(), [&](const auto &variable) {
    std::visit(
        [&]<typename T>(T uniformValue) {
          if constexpr (std::same_as<T, ui::ig::Color>) {
            ignoredResult = mainProgram->setUniform(
                *variable->name,
                glm::vec4{uniformValue.red(), uniformValue.green(), uniformValue.blue(), uniformValue.alpha()});
          } else {
            ignoredResult = mainProgram->setUniform(*variable->name, uniformValue);
          }
        },
        *variable->value);
  });
}

void ShaderToyMode::setBindings() {
  auto ignoredResult = mainProgram->getUniformValue(
      "outImage",
      Visitor{[&](int binding) { outputTexture->bindImage(Binding{binding}, ImageTextureUnitAccess::ReadWrite); },
              [](auto) {}});
  // TODO: read only?
  std::ranges::for_each(models.imageAssets->getTextures(), [&](const auto &tex) {
    ignoredResult = mainProgram->getUniformValue(
        *tex->name,
        Visitor{[&](int binding) { (*tex->texture)->bindImage(Binding{binding}, ImageTextureUnitAccess::ReadWrite); },
                [](auto) {}});
  });
}

void ShaderToyMode::updateUI() {
  mainController->outputController->setFps(fpsCounter.currentFPS(), fpsCounter.averageFPS());
}

void ShaderToyMode::updateConfig() {
  auto shaderVarsToml = models.shaderVariables->toToml();
  config.insert_or_assign("shader_variables", std::move(shaderVarsToml));
  auto imagesToml = models.imageAssets->toToml();
  config.insert_or_assign("images", std::move(imagesToml));
  auto glslEditorToml = models.codeEditor->toToml();
  config.insert_or_assign("editor", std::move(glslEditorToml));
  auto outputToml = models.output->toToml();
  config.insert_or_assign("output", std::move(outputToml));
}

void ShaderToyMode::createModels() {
  const auto updateTextureSize = [this](auto resolution) {
    const TextureSize textureSize{TextureWidth{resolution.first}, TextureHeight{resolution.second}, TextureDepth{0u}};
    initializeTexture(textureSize);
  };
  const auto markShaderChanged = [this](auto...) {
    isShaderChanged = true;
    lastShaderChangeTime = std::chrono::steady_clock::now();
  };
  const auto markShaderChangedOrRecompile = [this, markShaderChanged](auto...) {
    markShaderChanged();
    if (!autoCompileShader) { compileShader(*models.codeEditor->code); }
  };

  models.output = std::make_shared<OutputModel>(std::pair{2048u, 1024u}, nullptr);
  models.output->resolution.addValueListener(updateTextureSize);

  models.codeEditor =
      std::make_shared<GlslEditorModel>(true, std::chrono::milliseconds{1000}, false, DEFAULT_SHADER_SOURCE);
  models.codeEditor->autoCompile.addValueListener([this](auto autoCompile) { autoCompileShader = autoCompile; });
  autoCompileShader = *models.codeEditor->autoCompile;
  models.codeEditor->timePaused.addValueListener([this](auto timePaused) { timeCounterPaused = timePaused; });
  timeCounterPaused = *models.codeEditor->timePaused;
  models.codeEditor->compilationRequested.addEventListener([this] { compileShader(*models.codeEditor->code); });
  models.codeEditor->restartRequested.addEventListener([this] {
    getLogger().info("Restarting time");
    totalTime = std::chrono::nanoseconds{0};
  });
  models.codeEditor->copyCodeToClipboardRequested.addEventListener(
      [this] { glfwWindow->setClipboardContents(*models.codeEditor->code); });

  models.shaderVariables = std::make_shared<ShaderVariablesModel>();
  models.shaderVariables->variableAddedEvent.addEventListener([this, markShaderChanged](const auto &varModel) {
    markShaderChanged();
    mainController->imageAssetsController->disallowedNames.emplace(*varModel->name);
  });
  models.shaderVariables->variableRemovedEvent.addEventListener(
      [this, markShaderChangedOrRecompile](const auto &varModel) {
        markShaderChangedOrRecompile();
        mainController->imageAssetsController->disallowedNames.erase(*varModel->name);
      });

  models.imageAssets = std::make_shared<UserImageAssetsModel>();
  models.imageAssets->imageAddedEvent.addEventListener([this, markShaderChanged](const auto &imgModel) {
    markShaderChanged();
    mainController->shaderVariablesController->disallowedNames.emplace(*imgModel->name);
  });
  models.imageAssets->imageRemovedEvent.addEventListener([this, markShaderChangedOrRecompile](const auto &imgModel) {
    markShaderChangedOrRecompile();
    mainController->shaderVariablesController->disallowedNames.erase(*imgModel->name);
  });

  models.codeEditor->code.addValueListener(markShaderChanged);

  models.output->mousePositionOnImageUV.addValueListener([this](auto uvPos) {
    mousePos.x = uvPos.x();
    mousePos.y = uvPos.y();
  });
}
void ShaderToyMode::loadModelsFromConfig() {
  if (const auto iter = config.find("shader_variables"); iter != config.end()) {
    if (const auto varsTbl = iter->second.as_table(); varsTbl != nullptr) {
      models.shaderVariables->setFromToml(*varsTbl);
    }
  }

  if (const auto iter = config.find("images"); iter != config.end()) {
    if (const auto imagesTbl = iter->second.as_table(); imagesTbl != nullptr) {
      models.imageAssets->setFromToml(*imagesTbl);
    }
  }
  std::ranges::for_each(models.imageAssets->getTextures(), [this](const auto &textureModel) {
    if (*textureModel->texture == nullptr) {
      const auto onLoadDone = [=, this](const tl::expected<std::shared_ptr<Texture>, std::string> &loadingResult) {
        MainLoop::Get()->enqueue([=] {
          if (loadingResult.has_value()) {
            *textureModel->texture.modify() = loadingResult.value();
          } else {
            imGuiInterface->getNotificationManager()
                .createNotification("notif_loading_err", "Texture loading failed")
                .createChild<ui::ig::Text>(
                    "notif_txt",
                    fmt::format("Texture loading failed: '{}'.\n{}", loadingResult.error(), *textureModel->imagePath))
                .setColor<ui::ig::style::ColorOf::Text>(ui::ig::Color::Red);
          }
        });
      };
      const auto imgInfo = imageLoader->getImageInfo(*textureModel->imagePath);
      if (imgInfo.has_value()) {
        ChannelCount requiredChannels{imgInfo->channels};
        if (requiredChannels.get() == 3) { requiredChannels = ChannelCount{4}; }
        imageLoader->loadTextureWithChannelsAsync(*textureModel->imagePath, requiredChannels, onLoadDone);
      }
    }
  });

  if (const auto iter = config.find("editor"); iter != config.end()) {
    if (const auto editorTbl = iter->second.as_table(); editorTbl != nullptr) {
      models.codeEditor->setFromToml(*editorTbl);
    }
  }
  if (const auto iter = config.find("output"); iter != config.end()) {
    if (const auto outputTbl = iter->second.as_table(); outputTbl != nullptr) {
      models.output->setFromToml(*outputTbl);
    }
  }
}

void ShaderToyMode::createControllers() {
  auto outputController = std::make_unique<OutputController>(
      std::make_unique<OutputView>(imGuiInterface, "output_win", "Output"), models.output);
  auto logWindowController = std::make_unique<LogWindowController>(
      std::make_unique<LogWindowView>(imGuiInterface, "log_window", "Log"), std::make_shared<LogModel>());
  auto glslEditorController = std::make_unique<GlslEditorController>(
      std::make_unique<GlslEditorView>(imGuiInterface, "glsl_editor_win", "Code"), models.codeEditor);
  auto shaderVariablesController = std::make_unique<ShaderVariablesController>(
      std::make_unique<ShaderVariablesWindowView>(imGuiInterface, "shader_vars_win", "Shader variables"),
      models.shaderVariables, imGuiInterface);
  auto imageAssetsController = std::make_unique<ImageAssetsController>(
      std::make_unique<ImageAssetsView>(imGuiInterface, "image_assets_win", "Images"), models.imageAssets,
      imGuiInterface, imageLoader);

  mainController = std::make_unique<MainController>(
      std::make_unique<MainView>(imGuiInterface), std::make_unique<MainModel>(), imGuiInterface,
      ShaderToyControllers{std::move(outputController), std::move(logWindowController), std::move(glslEditorController),
                           std::move(shaderVariablesController), std::move(imageAssetsController)});
}

}  // namespace pf::shader_toy