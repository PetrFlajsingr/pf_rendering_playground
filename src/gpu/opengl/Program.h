//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "../Program.h"

#include "OpenGl.h"
#include "Shader.h"
#include <utility>

namespace pf {

class OpenGlProgram : public Program, public OpenGlHandleOwner {
 public:
  PF_GPU_OBJECT_API(GpuApi::OpenGl)

  explicit OpenGlProgram(RangeOf<std::shared_ptr<OpenGlShader>> auto &&programShaders)
      : Program(programShaders | ranges::views::transform([](const auto &shader) {
                  return std::static_pointer_cast<Shader>(shader);
                })) {}
  explicit OpenGlProgram(std::shared_ptr<Shader> shader);

 protected:
  [[nodiscard]] GpuOperationResult<ProgramError> createImpl() override;

  [[nodiscard]] ProgramInfos extractProgramInfos() override;

  [[nodiscard]] std::vector<UniformInfo> extractUniforms();
  [[nodiscard]] std::vector<AttributeInfo> extractAttributes();
  [[nodiscard]] std::vector<BufferInfo> extractBuffers();

  void useImpl() override;
  void setUniformImpl(UniformLocation location, std::variant<PF_SHADER_VALUE_TYPES> value) override;
  std::variant<PF_SHADER_VALUE_TYPES> getUniformValueImpl(const UniformInfo &info) override;
  void dispatchImpl(std::uint32_t x, std::uint32_t y, std::uint32_t z) override;

  // TODO: add missing
  [[nodiscard]] static constexpr std::optional<ShaderValueType> ShaderValueTypeFromGlConstant(GLenum glEnum);

  void deleteOpenGlObject(GLuint objectHandle) const override;
};

}  // namespace pf
