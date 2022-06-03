//
// Created by xflajs00 on 22.10.2021.
//

#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace pf {

/**
 * @return absolute path to folder of current executable
 */
[[nodiscard]] std::filesystem::path getExeFolder();

[[nodiscard]] std::optional<std::string> readFile(const std::filesystem::path &path);

}  // namespace pf
