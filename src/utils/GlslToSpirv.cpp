//
// Created by xflajs00 on 01.05.2022.
//

#include "GlslToSpirv.h"
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Include/glslang_c_interface.h>
#include <pf_common/RAII.h>

namespace pf {

static TBuiltInResource TBuiltInResource_Default() {
  TBuiltInResource Resources{};

  Resources.maxLights = 32;
  Resources.maxClipPlanes = 6;
  Resources.maxTextureUnits = 32;
  Resources.maxTextureCoords = 32;
  Resources.maxVertexAttribs = 64;
  Resources.maxVertexUniformComponents = 4096;
  Resources.maxVaryingFloats = 64;
  Resources.maxVertexTextureImageUnits = 32;
  Resources.maxCombinedTextureImageUnits = 80;
  Resources.maxTextureImageUnits = 32;
  Resources.maxFragmentUniformComponents = 4096;
  Resources.maxDrawBuffers = 32;
  Resources.maxVertexUniformVectors = 128;
  Resources.maxVaryingVectors = 8;
  Resources.maxFragmentUniformVectors = 16;
  Resources.maxVertexOutputVectors = 16;
  Resources.maxFragmentInputVectors = 15;
  Resources.minProgramTexelOffset = -8;
  Resources.maxProgramTexelOffset = 7;
  Resources.maxClipDistances = 8;
  Resources.maxComputeWorkGroupCountX = 65535;
  Resources.maxComputeWorkGroupCountY = 65535;
  Resources.maxComputeWorkGroupCountZ = 65535;
  Resources.maxComputeWorkGroupSizeX = 1024;
  Resources.maxComputeWorkGroupSizeY = 1024;
  Resources.maxComputeWorkGroupSizeZ = 64;
  Resources.maxComputeUniformComponents = 1024;
  Resources.maxComputeTextureImageUnits = 16;
  Resources.maxComputeImageUniforms = 8;
  Resources.maxComputeAtomicCounters = 8;
  Resources.maxComputeAtomicCounterBuffers = 1;
  Resources.maxVaryingComponents = 60;
  Resources.maxVertexOutputComponents = 64;
  Resources.maxGeometryInputComponents = 64;
  Resources.maxGeometryOutputComponents = 128;
  Resources.maxFragmentInputComponents = 128;
  Resources.maxImageUnits = 8;
  Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
  Resources.maxCombinedShaderOutputResources = 8;
  Resources.maxImageSamples = 0;
  Resources.maxVertexImageUniforms = 0;
  Resources.maxTessControlImageUniforms = 0;
  Resources.maxTessEvaluationImageUniforms = 0;
  Resources.maxGeometryImageUniforms = 0;
  Resources.maxFragmentImageUniforms = 8;
  Resources.maxCombinedImageUniforms = 8;
  Resources.maxGeometryTextureImageUnits = 16;
  Resources.maxGeometryOutputVertices = 256;
  Resources.maxGeometryTotalOutputComponents = 1024;
  Resources.maxGeometryUniformComponents = 1024;
  Resources.maxGeometryVaryingComponents = 64;
  Resources.maxTessControlInputComponents = 128;
  Resources.maxTessControlOutputComponents = 128;
  Resources.maxTessControlTextureImageUnits = 16;
  Resources.maxTessControlUniformComponents = 1024;
  Resources.maxTessControlTotalOutputComponents = 4096;
  Resources.maxTessEvaluationInputComponents = 128;
  Resources.maxTessEvaluationOutputComponents = 128;
  Resources.maxTessEvaluationTextureImageUnits = 16;
  Resources.maxTessEvaluationUniformComponents = 1024;
  Resources.maxTessPatchComponents = 120;
  Resources.maxPatchVertices = 32;
  Resources.maxTessGenLevel = 64;
  Resources.maxViewports = 16;
  Resources.maxVertexAtomicCounters = 0;
  Resources.maxTessControlAtomicCounters = 0;
  Resources.maxTessEvaluationAtomicCounters = 0;
  Resources.maxGeometryAtomicCounters = 0;
  Resources.maxFragmentAtomicCounters = 8;
  Resources.maxCombinedAtomicCounters = 8;
  Resources.maxAtomicCounterBindings = 1;
  Resources.maxVertexAtomicCounterBuffers = 0;
  Resources.maxTessControlAtomicCounterBuffers = 0;
  Resources.maxTessEvaluationAtomicCounterBuffers = 0;
  Resources.maxGeometryAtomicCounterBuffers = 0;
  Resources.maxFragmentAtomicCounterBuffers = 1;
  Resources.maxCombinedAtomicCounterBuffers = 1;
  Resources.maxAtomicCounterBufferSize = 16384;
  Resources.maxTransformFeedbackBuffers = 4;
  Resources.maxTransformFeedbackInterleavedComponents = 64;
  Resources.maxCullDistances = 8;
  Resources.maxCombinedClipAndCullDistances = 8;
  Resources.maxSamples = 4;
  Resources.maxMeshOutputVerticesNV = 256;
  Resources.maxMeshOutputPrimitivesNV = 512;
  Resources.maxMeshWorkGroupSizeX_NV = 32;
  Resources.maxMeshWorkGroupSizeY_NV = 1;
  Resources.maxMeshWorkGroupSizeZ_NV = 1;
  Resources.maxTaskWorkGroupSizeX_NV = 32;
  Resources.maxTaskWorkGroupSizeY_NV = 1;
  Resources.maxTaskWorkGroupSizeZ_NV = 1;
  Resources.maxMeshViewCountNV = 4;

  Resources.limits.nonInductiveForLoops = 1;
  Resources.limits.whileLoops = 1;
  Resources.limits.doWhileLoops = 1;
  Resources.limits.generalUniformIndexing = 1;
  Resources.limits.generalAttributeMatrixVectorIndexing = 1;
  Resources.limits.generalVaryingIndexing = 1;
  Resources.limits.generalSamplerIndexing = 1;
  Resources.limits.generalVariableIndexing = 1;
  Resources.limits.generalConstantMatrixVectorIndexing = 1;

  return Resources;
}

tl::expected<SpirvCompilationResult, SpirvCompilationError> glslSourceToSpirv(const std::string &glslSource) {
  glslang_initialize_process();
  RAII finalizeProcess{glslang_initialize_process};
  {
    const auto resources = TBuiltInResource_Default();
    const glslang_input_t input = {.language = GLSLANG_SOURCE_GLSL,
                                   .stage = GLSLANG_STAGE_COMPUTE,
                                   .client = GLSLANG_CLIENT_OPENGL,
                                   .client_version = GLSLANG_TARGET_OPENGL_450,
                                   .target_language = GLSLANG_TARGET_SPV,
                                   .target_language_version = GLSLANG_TARGET_SPV_1_3,
                                   .code = glslSource.c_str(),
                                   .default_version = 100,
                                   .default_profile = GLSLANG_NO_PROFILE,
                                   .force_default_version_and_profile = false,
                                   .forward_compatible = false,
                                   .messages = GLSLANG_MSG_DEFAULT_BIT,
                                   .resource = reinterpret_cast<const glslang_resource_t *>(&resources)};

    glslang_shader_t *shader = glslang_shader_create(&input);
    RAII deleteShader{[shader] { glslang_shader_delete(shader); }};
    {
      if (!glslang_shader_preprocess(shader, &input)) {
        return tl::make_unexpected(
            SpirvCompilationError{glslang_shader_get_info_log(shader), glslang_shader_get_info_debug_log(shader)});
      }

      if (!glslang_shader_parse(shader, &input)) {
        return tl::make_unexpected(
            SpirvCompilationError{glslang_shader_get_info_log(shader), glslang_shader_get_info_debug_log(shader)});
      }

      glslang_program_t *program = glslang_program_create();
      RAII deleteProgram([program] { glslang_program_delete(program); });
      {
        glslang_program_add_shader(program, shader);

        if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
          return tl::make_unexpected(SpirvCompilationError{glslang_program_get_info_log(program),
                                                           glslang_program_get_info_debug_log(program)});
        }

        glslang_program_SPIRV_generate(program, input.stage);

        auto result = SpirvCompilationResult{};

        if (glslang_program_SPIRV_get_messages(program)) {
          result.messages = glslang_program_SPIRV_get_messages(program);
        }

        const auto spirvBuffer = glslang_program_SPIRV_get_ptr(program);
        const auto spirvBufferSize = glslang_program_SPIRV_get_size(program);

        result.spirvData = std::vector<unsigned int>{spirvBuffer, spirvBuffer + spirvBufferSize};
        return result;
      }
    }
  }
}

}  // namespace pf