#pragma once

#include <array>
#include <filesystem>
#include <string>

[[nodiscard]] inline std::string resolveResourcePath(const std::string& path) {
    const bool                       relative   = !path.empty() && path.front() != '/';
    const std::array<std::string, 4> candidates = {
        path,
        "build/desktop-dev/example/" + path,
        "example/" + path,
        relative ? "/" + path : path,
    };

    std::error_code ec;
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate, ec)) {
            return candidate;
        }
    }
    return path;
}
