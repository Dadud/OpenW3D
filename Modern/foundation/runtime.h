#pragma once

#include <cstdint>
#include <string_view>

namespace openw3d::modern {

struct RuntimeInfo {
    std::string_view name;
    std::uint32_t api_version;
};

RuntimeInfo runtime_info() noexcept;

} // namespace openw3d::modern
