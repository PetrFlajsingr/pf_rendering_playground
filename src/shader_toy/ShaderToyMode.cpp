//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyMode.h"
#include "ShaderBuilder.h"
#include <pf_imgui/elements/Image.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/trim.hpp>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Include/ResourceLimits.h>

static TBuiltInResource TBuiltInResource_Default()
{
  TBuiltInResource Resources{};

  Resources.maxLights                                 = 32;
  Resources.maxClipPlanes                             = 6;
  Resources.maxTextureUnits                           = 32;
  Resources.maxTextureCoords                          = 32;
  Resources.maxVertexAttribs                          = 64;
  Resources.maxVertexUniformComponents                = 4096;
  Resources.maxVaryingFloats                          = 64;
  Resources.maxVertexTextureImageUnits                = 32;
  Resources.maxCombinedTextureImageUnits              = 80;
  Resources.maxTextureImageUnits                      = 32;
  Resources.maxFragmentUniformComponents              = 4096;
  Resources.maxDrawBuffers                            = 32;
  Resources.maxVertexUniformVectors                   = 128;
  Resources.maxVaryingVectors                         = 8;
  Resources.maxFragmentUniformVectors                 = 16;
  Resources.maxVertexOutputVectors                    = 16;
  Resources.maxFragmentInputVectors                   = 15;
  Resources.minProgramTexelOffset                     = -8;
  Resources.maxProgramTexelOffset                     = 7;
  Resources.maxClipDistances                          = 8;
  Resources.maxComputeWorkGroupCountX                 = 65535;
  Resources.maxComputeWorkGroupCountY                 = 65535;
  Resources.maxComputeWorkGroupCountZ                 = 65535;
  Resources.maxComputeWorkGroupSizeX                  = 1024;
  Resources.maxComputeWorkGroupSizeY                  = 1024;
  Resources.maxComputeWorkGroupSizeZ                  = 64;
  Resources.maxComputeUniformComponents               = 1024;
  Resources.maxComputeTextureImageUnits               = 16;
  Resources.maxComputeImageUniforms                   = 8;
  Resources.maxComputeAtomicCounters                  = 8;
  Resources.maxComputeAtomicCounterBuffers            = 1;
  Resources.maxVaryingComponents                      = 60;
  Resources.maxVertexOutputComponents                 = 64;
  Resources.maxGeometryInputComponents                = 64;
  Resources.maxGeometryOutputComponents               = 128;
  Resources.maxFragmentInputComponents                = 128;
  Resources.maxImageUnits                             = 8;
  Resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
  Resources.maxCombinedShaderOutputResources          = 8;
  Resources.maxImageSamples                           = 0;
  Resources.maxVertexImageUniforms                    = 0;
  Resources.maxTessControlImageUniforms               = 0;
  Resources.maxTessEvaluationImageUniforms            = 0;
  Resources.maxGeometryImageUniforms                  = 0;
  Resources.maxFragmentImageUniforms                  = 8;
  Resources.maxCombinedImageUniforms                  = 8;
  Resources.maxGeometryTextureImageUnits              = 16;
  Resources.maxGeometryOutputVertices                 = 256;
  Resources.maxGeometryTotalOutputComponents          = 1024;
  Resources.maxGeometryUniformComponents              = 1024;
  Resources.maxGeometryVaryingComponents              = 64;
  Resources.maxTessControlInputComponents             = 128;
  Resources.maxTessControlOutputComponents            = 128;
  Resources.maxTessControlTextureImageUnits           = 16;
  Resources.maxTessControlUniformComponents           = 1024;
  Resources.maxTessControlTotalOutputComponents       = 4096;
  Resources.maxTessEvaluationInputComponents          = 128;
  Resources.maxTessEvaluationOutputComponents         = 128;
  Resources.maxTessEvaluationTextureImageUnits        = 16;
  Resources.maxTessEvaluationUniformComponents        = 1024;
  Resources.maxTessPatchComponents                    = 120;
  Resources.maxPatchVertices                          = 32;
  Resources.maxTessGenLevel                           = 64;
  Resources.maxViewports                              = 16;
  Resources.maxVertexAtomicCounters                   = 0;
  Resources.maxTessControlAtomicCounters              = 0;
  Resources.maxTessEvaluationAtomicCounters           = 0;
  Resources.maxGeometryAtomicCounters                 = 0;
  Resources.maxFragmentAtomicCounters                 = 8;
  Resources.maxCombinedAtomicCounters                 = 8;
  Resources.maxAtomicCounterBindings                  = 1;
  Resources.maxVertexAtomicCounterBuffers             = 0;
  Resources.maxTessControlAtomicCounterBuffers        = 0;
  Resources.maxTessEvaluationAtomicCounterBuffers     = 0;
  Resources.maxGeometryAtomicCounterBuffers           = 0;
  Resources.maxFragmentAtomicCounterBuffers           = 1;
  Resources.maxCombinedAtomicCounterBuffers           = 1;
  Resources.maxAtomicCounterBufferSize                = 16384;
  Resources.maxTransformFeedbackBuffers               = 4;
  Resources.maxTransformFeedbackInterleavedComponents = 64;
  Resources.maxCullDistances                          = 8;
  Resources.maxCombinedClipAndCullDistances           = 8;
  Resources.maxSamples                                = 4;
  Resources.maxMeshOutputVerticesNV                   = 256;
  Resources.maxMeshOutputPrimitivesNV                 = 512;
  Resources.maxMeshWorkGroupSizeX_NV                  = 32;
  Resources.maxMeshWorkGroupSizeY_NV                  = 1;
  Resources.maxMeshWorkGroupSizeZ_NV                  = 1;
  Resources.maxTaskWorkGroupSizeX_NV                  = 32;
  Resources.maxTaskWorkGroupSizeY_NV                  = 1;
  Resources.maxTaskWorkGroupSizeZ_NV                  = 1;
  Resources.maxMeshViewCountNV                        = 4;

  Resources.limits.nonInductiveForLoops                 = 1;
  Resources.limits.whileLoops                           = 1;
  Resources.limits.doWhileLoops                         = 1;
  Resources.limits.generalUniformIndexing               = 1;
  Resources.limits.generalAttributeMatrixVectorIndexing = 1;
  Resources.limits.generalVaryingIndexing               = 1;
  Resources.limits.generalSamplerIndexing               = 1;
  Resources.limits.generalVariableIndexing              = 1;
  Resources.limits.generalConstantMatrixVectorIndexing  = 1;

  return Resources;
}

namespace pf::shader_toy {

std::string ShaderToyMode::getName() const { return "ShaderToy"; }

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface,
                                    const std::shared_ptr<glfw::Window> &window) {
  glfwWindow = window;
  ui = std::make_unique<UI>(imguiInterface, DEFAULT_SHADER_SOURCE);

  const auto updateTextureSizeFromUI = [this](auto) {
    initializeTexture({ui->outputWindow->widthCombobox->getValue(), ui->outputWindow->heightCombobox->getValue()});
  };

  ui->outputWindow->widthCombobox->addValueListener(updateTextureSizeFromUI);
  ui->outputWindow->heightCombobox->addValueListener(updateTextureSizeFromUI, true);

  ui->textInputWindow->compileButton->addClickListener([&] {
    spdlog::info("[ShaderToy] Compiling shader");
    if (auto err = compileShader(ui->textInputWindow->editor->getText()); err.has_value()) {
      spdlog::error("[ShaderToy] {}", err.value());
      std::ranges::for_each(err.value() | ranges::view::split('\n'), [&] (const auto &line) {
        auto parts = line | ranges::view::split(':') | ranges::view::transform([] (auto part) {
                       return part | ranges::to<std::string>;
                     }) | ranges::to_vector;
        if (parts.size() < 3) {
          return;
        }
        auto openBracketPos = std::ranges::find(parts[0], '(') + 1;
        auto closeBracketPos = std::ranges::find(parts[0], ')');
        int lineNum;
        std::from_chars(&*openBracketPos, &*closeBracketPos, lineNum);
        spdlog::debug("{}", shaderLineMapping(lineNum));
      });
    } else {
      totalTime = std::chrono::nanoseconds{0};
      frameCounter = 0;
      spdlog::info("[ShaderToy] Compiling shader success");
    }
  });
  ui->textInputWindow->restartButton->addClickListener([&] {
    spdlog::info("[ShaderToy] Restarting time");
    totalTime = std::chrono::nanoseconds{0};
  });
  ui->textInputWindow->timePausedCheckbox->bind(timeCounterPaused);

  ui->outputWindow->image->addMousePositionListener([&](auto pos) {
    const auto size = ui->outputWindow->image->getSize();
    const auto nX = pos.x / static_cast<float>(size.width);
    const auto nY = pos.y / static_cast<float>(size.height);
    const auto result = glm::vec2{getTextureSize()} * glm::vec2{nX, nY};
    mousePos.x = result.x;
    mousePos.y = result.y;
  });

  ui->hide();
}

void ShaderToyMode::activate_impl() {
  resetCounters();
  ui->show();
  ui->interface->setStateFromConfig();
}

void ShaderToyMode::deactivate_impl() {
  ui->interface->updateConfig();
  ui->hide();
}

void ShaderToyMode::deinitialize_impl() {}

void ShaderToyMode::render(std::chrono::nanoseconds timeDelta) {
  if (programHandle == -1) { return; }

  auto mouseState = MouseState::None;
  if (ui->outputWindow->image->isHovered()) {
    if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Left) == pf::glfw::ButtonState::Down) {
      mouseState = MouseState::LeftDown;
    } else if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Right) == pf::glfw::ButtonState::Down) {
      mouseState = MouseState::RightDown;
    }
  }

  const auto timeFloat = static_cast<float>(totalTime.count()) / 1'000'000'000.0f;
  const auto timeDeltaFloat = static_cast<float>(timeDelta.count()) / 1'000'000'000.0f;

  glUseProgram(programHandle);
  const auto textureSize = getTextureSize();
 /* renderProgram->use();
  outputTexture->bindImage(0);
  const auto textureSize = getTextureSize();
  // todo: change this
  if (renderProgram->isActiveUniform("time")) { renderProgram->set1f("time", timeFloat); }
  if (renderProgram->isActiveUniform("timeDelta")) { renderProgram->set1f("timeDelta", timeDeltaFloat); }
  if (renderProgram->isActiveUniform("frameNum")) { renderProgram->set1i("frameNum", frameCounter); }
  if (renderProgram->isActiveUniform("mouseState")) {
    renderProgram->set1i("mouseState", static_cast<int>(mouseState));
  }
  if (renderProgram->isActiveUniform("mousePos")) { renderProgram->set3fv("mousePos", &mousePos[0]); }
  for (const auto &[name, value] : userDefinedUniforms) {
    if (renderProgram->isActiveUniform(name)) {
      std::visit([&]<typename T>(T uniformValue) {
        if constexpr (std::same_as<T, bool>) { renderProgram->set1i(name, static_cast<int>(uniformValue)); }
        if constexpr (std::same_as<T, float>) { renderProgram->set1f(name, uniformValue); }
        if constexpr (std::same_as<T, unsigned int>) { renderProgram->set1ui(name, uniformValue); }
        if constexpr (std::same_as<T, int>) { renderProgram->set1i(name, uniformValue); }
        if constexpr (std::same_as<T, glm::vec2>) { renderProgram->set2fv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::vec3>) { renderProgram->set3fv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::vec4>) { renderProgram->set4fv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::ivec2>) { renderProgram->set2iv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::ivec3>) { renderProgram->set3iv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::ivec4>) { renderProgram->set4iv(name, glm::value_ptr(uniformValue)); }
        if constexpr (std::same_as<T, glm::bvec2>) {
          const glm::ivec2 data = uniformValue;
          renderProgram->set3iv(name, glm::value_ptr(data));
        }
        if constexpr (std::same_as<T, glm::bvec3>) {
          const glm::ivec3 data = uniformValue;
          renderProgram->set3iv(name, glm::value_ptr(data));
        }
        if constexpr (std::same_as<T, glm::bvec4>) {
          const glm::ivec4 data = uniformValue;
          renderProgram->set4iv(name, glm::value_ptr(data));
        }
        if constexpr (std::same_as<T, glm::uvec2>) {
          renderProgram->set2uiv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::uvec3>) {
          renderProgram->set3uiv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::uvec4>) {
          renderProgram->set4uiv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::mat2>) {
          renderProgram->setMatrix2fv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::mat3>) {
          renderProgram->setMatrix3fv(name, glm::value_ptr(uniformValue));
        }
        if constexpr (std::same_as<T, glm::mat4>) {
          renderProgram->setMatrix4fv(name, glm::value_ptr(uniformValue));
        }
      }, value->data);
    }
  }
  renderProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);*/

  fpsCounter.onFrame();
  updateUI();
  if (!timeCounterPaused) { totalTime += timeDelta; }
}
void ShaderToyMode::resetCounters() {
  frameCounter = 0;
  totalTime = std::chrono::nanoseconds{0};
}

void ShaderToyMode::initializeTexture(glm::uvec2 textureSize) {
  spdlog::info("[ShaderToy] Updating texture size to {}x{}", textureSize.x, textureSize.y);
  outputTexture = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, textureSize.x, textureSize.y);
  outputTexture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  outputTexture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  ui->outputWindow->image->setTextureId(
      reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(outputTexture->getId())));
}

glm::uvec2 ShaderToyMode::getTextureSize() const { return {outputTexture->getWidth(0), outputTexture->getHeight(0)}; }

std::optional<std::string> ShaderToyMode::compileShader(const std::string &shaderCode) {
  // clang-format off
  auto builder = ShaderBuilder{};
  builder.addUniform<float>("time")
      .addUniform<float>("timeDelta")
      .addUniform<int>("frameNum")
      .addEnum<MouseState>()
      .addUniform<MouseState>("mouseState")
      .addUniform<glm::vec3>("mousePos")
      .addImage2D("rgba32f", 0, "outImage")
      .setLocalGroupSize(COMPUTE_LOCAL_GROUP_SIZE);
  for (const auto &[name, value] : ui->textInputWindow->varPanel->getValueRecords()) {
    builder.addUniform(value->typeName, name);
  }
  // clang-format on
  const auto &[source, lineMapping] = builder.build(shaderCode);
  shaderLineMapping = lineMapping;
  spdlog::trace(source);

  const auto resources = TBuiltInResource_Default();
  const glslang_input_t input =
      {
          .language = GLSLANG_SOURCE_GLSL,
          .stage = GLSLANG_STAGE_COMPUTE,
          .client = GLSLANG_CLIENT_OPENGL,
          .client_version = GLSLANG_TARGET_OPENGL_450,
          .target_language = GLSLANG_TARGET_SPV,
          .target_language_version = GLSLANG_TARGET_SPV_1_3,
          .code = source.c_str(),
          .default_version = 100,
          .default_profile = GLSLANG_NO_PROFILE,
          .force_default_version_and_profile = false,
          .forward_compatible = false,
          .messages = GLSLANG_MSG_DEFAULT_BIT,
          .resource = reinterpret_cast<const glslang_resource_t *>(&resources)};


  glslang_initialize_process();

  glslang_shader_t* shader = glslang_shader_create( &input );

  if ( !glslang_shader_preprocess(shader, &input) )
  {
    spdlog::debug(glslang_shader_get_info_log(shader));
    spdlog::debug(glslang_shader_get_info_debug_log(shader));
  }

  if ( !glslang_shader_parse(shader, &input) )
  {
    spdlog::debug(glslang_shader_get_info_log(shader));
    spdlog::debug(glslang_shader_get_info_debug_log(shader));
  }

  glslang_program_t* program = glslang_program_create();
  glslang_program_add_shader( program, shader );

  if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
  {
    spdlog::debug(glslang_program_get_info_log(program));
    spdlog::debug(glslang_program_get_info_debug_log(program));
  }

  glslang_program_SPIRV_generate( program, input.stage );

  if ( glslang_program_SPIRV_get_messages(program) )
  {
    printf("%s", glslang_program_SPIRV_get_messages(program));
  }

  glslang_shader_delete( shader );

  const auto spirvBuffer = glslang_program_SPIRV_get_ptr(program);
  const auto spirvBufferSize = glslang_program_SPIRV_get_size(program) * sizeof(unsigned int);



  static GLuint gl_shader = -1;
  if (gl_shader != -1) {
    glDeleteShader(gl_shader);
  }
  gl_shader = glCreateShader(GL_COMPUTE_SHADER);

  if (programHandle != -1) {
    glDeleteProgram(programHandle);
  }
  glShaderBinary(1, &gl_shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvBuffer, spirvBufferSize);
  glSpecializeShader(gl_shader, "main", 0, 0, 0);

  int compiled = 0;
  glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &compiled);
  if (compiled) {
    glAttachShader(programHandle, gl_shader);
  } else {
    spdlog::error("This is fucked up");
  }


    glslang_program_delete( program );
  /*auto renderShader = std::make_shared<Shader>(GL_COMPUTE_SHADER, source);
  if (!renderShader->getCompileStatus()) {
    return renderShader->getInfoLog();
  }
  renderProgram = std::make_shared<Program>(std::move(renderShader));*/
  userDefinedUniforms = ui->textInputWindow->varPanel->getValueRecords();
  return std::nullopt;
}

void ShaderToyMode::updateUI() {
  ui->outputWindow->fpsAveragePlot->addValue(fpsCounter.averageFPS());
  ui->outputWindow->fpsText->setText("FPS: {}", fpsCounter.averageFPS());
}

}  // namespace pf