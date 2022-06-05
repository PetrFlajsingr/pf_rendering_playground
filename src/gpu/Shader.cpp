//
// Created by xflajs00 on 07.05.2022.
//

#include "Shader.h"

namespace pf::gpu {

ShaderType Shader::getShaderType() const { return shaderType; }

std::string Shader::getDebugString() const {
  return fmt::format("{}\tType: {}", GpuObject::getDebugString(), magic_enum::enum_name(shaderType));
}

}  // namespace pf