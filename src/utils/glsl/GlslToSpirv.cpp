//
// Created by xflajs00 on 01.05.2022.
//

#include "GlslToSpirv.h"
#include "glslang/Include/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "glslang/SPIRV/Logger.h"
#include "glslang/SPIRV/SpvTools.h"
#include "pf_common/RAII.h"
#include <algorithm>
#include <string>
#include <vector>
#include "DirStackFileIncluder.h"
#include "DefaultBuiltinResource.h"

namespace pf {

tl::expected<SpirvCompilationResult, SpirvCompilationError>
glslComputeShaderSourceToSpirv(const std::string &glslSource) {
  glslang::InitializeProcess();
  RAII finalizeProcess{glslang::FinalizeProcess};
  {
    constexpr static auto DEFAULT_VERSION = 100;
    auto shader = glslang::TShader{EShLanguage::EShLangCompute};
    auto srcPtr = glslSource.c_str();
    shader.setStrings(&srcPtr, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, EShLangCompute, glslang::EShClientOpenGL, DEFAULT_VERSION);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

    const auto resources = TBuiltInResource_Default();
    {
      std::string preprocessedGlsl;
      DirStackFileIncluder Includer;
      /* TODO: use custom callbacks if they are available in 'i->callbacks' */
      if (!shader.preprocess(&resources, DEFAULT_VERSION, EProfile::ENoProfile, false, false,
                             EShMessages::EShMsgDefault, &preprocessedGlsl, Includer)) {
        return tl::make_unexpected(SpirvCompilationError{shader.getInfoLog(), shader.getInfoDebugLog()});
      }

      if (!shader.parse(&resources, DEFAULT_VERSION, false, EShMessages::EShMsgDefault)) {
        return tl::make_unexpected(SpirvCompilationError{shader.getInfoLog(), shader.getInfoDebugLog()});
      }
      {
        auto program = glslang::TProgram{};
        program.addShader(&shader);
        if (!program.link(EShMessages::EShMsgSpvRules)) {
          return tl::make_unexpected(SpirvCompilationError{shader.getInfoLog(), shader.getInfoDebugLog()});
        }

        std::vector<unsigned int> spirvData;
        const auto intermediate = program.getIntermediate(EShLangCompute);
        spv::SpvBuildLogger logger;
        glslang::SpvOptions spvOptions;
        spvOptions.validate = true;
        glslang::GlslangToSpv(*intermediate, spirvData, &logger, &spvOptions);

        auto result = SpirvCompilationResult{};
        result.messages = logger.getAllMessages();
        result.spirvData = std::move(spirvData);
        return result;
      }
    }
  }
}

}  // namespace pf