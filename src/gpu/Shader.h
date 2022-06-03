//
// Created by xflajs00 on 07.05.2022.
//

#pragma once

#include "GpuObject.h"
#include "utils/glsl/GlslToSpirv.h"
#include "utils/glsl/glsl_typenames.h"
#include <string>

namespace pf {

enum class ShaderError { Compilation };

enum class ShaderType {
  // TODO: add more
  Compute,
  Vertex,
  TesselationControl,
  TesselationEvaluation,
  Geometry,
  Fragment
};

// TODO: consider using spirv reflection for info extraction
class Shader : public GpuObject {
 public:
  PF_GPU_OBJECT_TYPE(GpuObject::Type::Shader)

  [[nodiscard]] virtual GpuOperationResult<ShaderError> create(const SpirvCompilationResult &spirvData,
                                                               const std::string &entryPoint) = 0;
  [[nodiscard]] virtual GpuOperationResult<ShaderError> create(const std::string &source) = 0;

  [[nodiscard]] ShaderType getShaderType() const;

  [[nodiscard]] std::string getDebugString() const override;

 protected:
  ShaderType shaderType;
};

}  // namespace pf