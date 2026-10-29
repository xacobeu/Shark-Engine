export module shark:util.ResourcePath;

import std;

export namespace shark {
[[nodiscard]] inline std::string resolveResourcePath(const std::string& path) {
    const bool                       relative   = !path.empty() && path.front() != '/';
    const std::array<std::string, 3> candidates = {
        path,
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

} // namespace shark
