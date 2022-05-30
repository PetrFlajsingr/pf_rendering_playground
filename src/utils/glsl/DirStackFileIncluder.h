//
// Created by Petr on 30/05/2022.
//

#pragma once

#include "glslang/Public/ShaderLang.h"
#include <set>

// Default include class for normal include convention of search backward
// through the stack of active include paths (for nested includes).
// Can be overridden to customize.
class DirStackFileIncluder : public glslang::TShader::Includer {
 public:
  DirStackFileIncluder();

  IncludeResult *includeLocal(const char *headerName, const char *includerName, size_t inclusionDepth) override;

  IncludeResult *includeSystem(const char *headerName, const char * /*includerName*/,
                               size_t /*inclusionDepth*/) override;

  // Externally set directories. E.g., from a command-line -I<dir>.
  //  - Most-recently pushed are checked first.
  //  - All these are checked after the parse-time stack of local directories
  //    is checked.
  //  - This only applies to the "local" form of #include.
  //  - Makes its own copy of the path.
  virtual void pushExternalLocalDirectory(const std::string &dir);

  void releaseInclude(IncludeResult *result) override;

  virtual std::set<std::string> getIncludedFiles();

  ~DirStackFileIncluder() override = default;

 protected:
  typedef char tUserDataElement;
  std::vector<std::string> directoryStack{};
  int externalLocalDirectoryCount;
  std::set<std::string> includedFiles;

  // Search for a valid "local" path based on combining the stack of include
  // directories and the nominal name of the header.
  virtual IncludeResult *readLocalPath(const char *headerName, const char *includerName, int depth);

  // Search for a valid <system> path.
  // Not implemented yet; returning nullptr signals failure to find.
  virtual IncludeResult *readSystemPath(const char * /*headerName*/) const;

  // Do actual reading of the file, filling in a new include result.
  virtual IncludeResult *newIncludeResult(const std::string &path, std::ifstream &file, int length) const;

  // If no path markers, return current working directory.
  // Otherwise, strip file name and return path leading up to it.
  virtual std::string getDirectory(const std::string path) const;
};