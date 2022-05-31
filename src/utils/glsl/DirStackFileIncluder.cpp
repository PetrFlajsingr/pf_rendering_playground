//
// Created by Petr on 30/05/2022.
//

#include "DirStackFileIncluder.h"
#include <algorithm>
#include <fstream>

DirStackFileIncluder::DirStackFileIncluder() : externalLocalDirectoryCount(0) {}

glslang::TShader::Includer::IncludeResult *
DirStackFileIncluder::includeLocal(const char *headerName, const char *includerName, size_t inclusionDepth) {
  return readLocalPath(headerName, includerName, (int) inclusionDepth);
}

glslang::TShader::Includer::IncludeResult *
DirStackFileIncluder::includeSystem(const char *headerName, const char * /*includerName*/, size_t /*inclusionDepth*/) {
  return readSystemPath(headerName);
}

void DirStackFileIncluder::pushExternalLocalDirectory(const std::string &dir) {
  directoryStack.push_back(dir);
  externalLocalDirectoryCount = (int) directoryStack.size();
}

void DirStackFileIncluder::releaseInclude(glslang::TShader::Includer::IncludeResult *result) {
  if (result != nullptr) {
    delete[] static_cast<tUserDataElement *>(result->userData);
    delete result;
  }
}

std::set<std::string> DirStackFileIncluder::getIncludedFiles() { return includedFiles; }

glslang::TShader::Includer::IncludeResult *DirStackFileIncluder::readLocalPath(const char *headerName,
                                                                               const char *includerName, int depth) {
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

glslang::TShader::Includer::IncludeResult *DirStackFileIncluder::readSystemPath(const char *) const { return nullptr; }

glslang::TShader::Includer::IncludeResult *
DirStackFileIncluder::newIncludeResult(const std::string &path, std::ifstream &file, int length) const {
  char *content = new tUserDataElement[length];
  file.seekg(0, file.beg);
  file.read(content, length);
  return new IncludeResult(path, content, length, content);
}

std::string DirStackFileIncluder::getDirectory(const std::string path) const {
  size_t last = path.find_last_of("/\\");
  return last == std::string::npos ? "." : path.substr(0, last);
}
