//
// Created by xflajs00 on 18.04.2022.
//

#include "ShaderToyMode.h"
#include "ShaderToyShaderBuilder.h"
#include <pf_imgui/elements/Image.h>

namespace pf {

void ShaderToyMode::initialize_impl(const std::shared_ptr<ui::ig::ImGuiInterface> &imguiInterface, const std::shared_ptr<glfw::Window> &window) {
  glfwWindow = window;
  ui = std::make_unique<ShaderToyUI>(imguiInterface);

  const auto updateTextureSizeFromUI = [this](auto) {
    initializeTexture({ui->outputWindow->widthCombobox->getValue(), ui->outputWindow->heightCombobox->getValue()});
  };

  ui->outputWindow->widthCombobox->addValueListener(updateTextureSizeFromUI);
  ui->outputWindow->heightCombobox->addValueListener(updateTextureSizeFromUI, true);

  ui->textInputWindow->compileButton->addClickListener([&] {
    spdlog::info("Compiling shader: start");
    if (auto err = compileShader(ui->textInputWindow->editor->getText()); err.has_value()) {
      spdlog::error(err.value());
    } else {
      totalTime = std::chrono::nanoseconds{0};
      frameCounter = 0;
      spdlog::info("Compiling shader: success");
    }
  });

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

void ShaderToyMode::deinitialize_impl() {
}

void ShaderToyMode::render(std::chrono::nanoseconds timeDelta) {
  if (renderProgram == nullptr) {
    return;
  }

  auto mouseState = RendererMouseState::None;
  if (ui->outputWindow->image->isHovered()) {
    if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Left) == pf::glfw::ButtonState::Down) {
      mouseState = pf::RendererMouseState::LeftDown;
    } else if (glfwWindow->getLastMouseButtonState(pf::glfw::MouseButton::Right) == pf::glfw::ButtonState::Down) {
      mouseState = pf::RendererMouseState::RightDown;
    }
  }

  const auto timeFloat = static_cast<float>(totalTime.count()) / 1'000'000'000.0f;
  const auto timeDeltaFloat = static_cast<float>(timeDelta.count()) / 1'000'000'000.0f;

  renderProgram->use();
  outputTexture->bindImage(0);
  const auto textureSize = getTextureSize();
  if (renderProgram->isActiveUniform("time")) {
    renderProgram->set1f("time", timeFloat);
  }
  if (renderProgram->isActiveUniform("timeDelta")) {
    renderProgram->set1f("timeDelta", timeDeltaFloat);
  }
  if (renderProgram->isActiveUniform("frameNum")) {
    renderProgram->set1i("frameNum", frameCounter);
  }
  if (renderProgram->isActiveUniform("mouseState")) {
    renderProgram->set1i("mouseState", static_cast<int>(mouseState));
  }
  if (renderProgram->isActiveUniform("mousePos")) {
    renderProgram->set3fv("mousePos", &mousePos[0]);
  }
  renderProgram->dispatch(textureSize.x / COMPUTE_LOCAL_GROUP_SIZE.x, textureSize.y / COMPUTE_LOCAL_GROUP_SIZE.y);

  totalTime += timeDelta;
}
void ShaderToyMode::resetCounters() {
  frameCounter = 0;
  totalTime = std::chrono::nanoseconds{0};
}

void ShaderToyMode::initializeTexture(glm::uvec2 textureSize) {
  outputTexture = std::make_shared<Texture>(GL_TEXTURE_2D, GL_RGBA32F, 0, textureSize.x, textureSize.y);
  outputTexture->texParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  outputTexture->texParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  ui->outputWindow->image->setTextureId(reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(outputTexture->getId())));
}

glm::uvec2 ShaderToyMode::getTextureSize() const {
  return {outputTexture->getWidth(0), outputTexture->getHeight(0)};
}

std::optional<std::string> ShaderToyMode::compileShader(const std::string &shaderCode) {
  // clang-format off
  const auto source = ShaderToyShaderBuilder{}
                        .addUniform<float>("time")
                        .addUniform<float>("timeDelta")
                        .addUniform<int>("frameNum")
                        .addUniform("MOUSE_STATE", "mouseState")
                        .addUniform<glm::vec3>("mousePos")
                        .addDefine("MOUSE_STATE", "int")
                        .addDefine("MOUSE_LEFT_DOWN", "1")
                        .addDefine("MOUSE_RIGHT_DOWN", "2")
                        .addImage2D("rgba32f", 0, "outImage")
                        .setLocalGroupSize(COMPUTE_LOCAL_GROUP_SIZE)
                        .build(shaderCode);
  // clang-format on
  spdlog::trace(source);
  auto renderShader = std::make_shared<Shader>(GL_COMPUTE_SHADER, source);
  if (!renderShader->getCompileStatus()) {
    return renderShader->getInfoLog();
  }
  renderProgram = std::make_shared<Program>(std::move(renderShader));
  return std::nullopt;
}

}// namespace pf