//
// Created by xflajs00 on 07.05.2022.
//

#include "Program.h"

namespace pf::gpu {

GpuOperationResult<ProgramError> Program::create() {
  if (auto result = createImpl(); result.has_value()) { return result; }
  infos = extractProgramInfos();
  return std::nullopt;
}

const std::vector<UniformInfo> &Program::getUniforms() const { return infos.uniforms; }

ExpectedGpuOperationResult<ShaderValueType, ProgramError> Program::getUniformType(const std::string &name) const {
  if (const auto infoOpt = findUniformInfo(name); infoOpt.has_value()) { return infoOpt.value()->type; }
  return tl::make_unexpected(GpuError{ProgramError::UniformNotFound, fmt::format("Uniform '{}' is not active.", name)});
}

const std::vector<AttributeInfo> &Program::getAttributes() const { return infos.attributes; }

const std::vector<BufferInfo> &Program::getBuffers() const { return infos.buffers; }

GpuOperationResult<ProgramError> Program::dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
  if (std::ranges::any_of(
          shaders, [](ShaderType type) { return type != ShaderType::Compute; }, &Shader::getShaderType)) {
    return GpuError{ProgramError::InvalidOperationForType, "Can not dispatch non compute program."};
  }
  dispatchImpl(x, y, z);
  return std::nullopt;
}

std::optional<const UniformInfo *> Program::findUniformInfo(const std::string &name) const {
  if (const auto iter = std::ranges::find(infos.uniforms, name, &UniformInfo::name); iter != infos.uniforms.end()) {
    return &*iter;
  }
  return std::nullopt;
}

std::string Program::getDebugString() const { return GpuObject::getDebugString(); }

}  // namespace pf::gpu