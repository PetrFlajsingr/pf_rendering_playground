//
// Created by xflajs00 on 01.05.2022.
//

#include "GlslToSpirv.h"
#include <algorithm>
#include <fstream>
#include <glslang/Include/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/SPIRV/Logger.h>
#include <glslang/SPIRV/SpvTools.h>
#include <pf_common/RAII.h>
#include <set>
#include <string>
#include <vector>

// TODO: move this outta here
// Default include class for normal include convention of search backward
// through the stack of active include paths (for nested includes).
// Can be overridden to customize.
class DirStackFileIncluder : public glslang::TShader::Includer {
 public:
  DirStackFileIncluder() : externalLocalDirectoryCount(0) {}

  IncludeResult *includeLocal(const char *headerName, const char *includerName,
                                      size_t inclusionDepth) override {
    return readLocalPath(headerName, includerName, (int) inclusionDepth);
  }

  IncludeResult *includeSystem(const char *headerName, const char * /*includerName*/,
                                       size_t /*inclusionDepth*/) override {
    return readSystemPath(headerName);
  }

  // Externally set directories. E.g., from a command-line -I<dir>.
  //  - Most-recently pushed are checked first.
  //  - All these are checked after the parse-time stack of local directories
  //    is checked.
  //  - This only applies to the "local" form of #include.
  //  - Makes its own copy of the path.
  virtual void pushExternalLocalDirectory(const std::string &dir) {
    directoryStack.push_back(dir);
    externalLocalDirectoryCount = (int) directoryStack.size();
  }

  void releaseInclude(IncludeResult *result) override {
    if (result != nullptr) {
      delete[] static_cast<tUserDataElement *>(result->userData);
      delete result;
    }
  }

  virtual std::set<std::string> getIncludedFiles() { return includedFiles; }

  ~DirStackFileIncluder() override {}

 protected:
  typedef char tUserDataElement;
  std::vector<std::string> directoryStack;
  int externalLocalDirectoryCount;
  std::set<std::string> includedFiles;

  // Search for a valid "local" path based on combining the stack of include
  // directories and the nominal name of the header.
  virtual IncludeResult *readLocalPath(const char *headerName, const char *includerName, int depth) {
    // Discard popped include directories, and
    // initialize when at parse-time first level.
    directoryStack.resize(depth + externalLocalDirectoryCount);
    if (depth == 1) directoryStack.back() = getDirectory(includerName);

    // Find a directory that works, using a reverse search of the include stack.
    for (auto it = directoryStack.rbegin(); it != directoryStack.rend(); ++it) {
      std::string path = *it + '/' + headerName;
      std::replace(path.begin(), path.end(), '\\', '/');
      std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);
      if (file) {
        directoryStack.push_back(getDirectory(path));
        includedFiles.insert(path);
        return newIncludeResult(path, file, (int) file.tellg());
      }
    }

    return nullptr;
  }

  // Search for a valid <system> path.
  // Not implemented yet; returning nullptr signals failure to find.
  virtual IncludeResult *readSystemPath(const char * /*headerName*/) const { return nullptr; }

  // Do actual reading of the file, filling in a new include result.
  virtual IncludeResult *newIncludeResult(const std::string &path, std::ifstream &file, int length) const {
    char *content = new tUserDataElement[length];
    file.seekg(0, file.beg);
    file.read(content, length);
    return new IncludeResult(path, content, length, content);
  }

  // If no path markers, return current working directory.
  // Otherwise, strip file name and return path leading up to it.
  virtual std::string getDirectory(const std::string path) const {
    size_t last = path.find_last_of("/\\");
    return last == std::string::npos ? "." : path.substr(0, last);
  }
};

// TODO: move this outta here
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