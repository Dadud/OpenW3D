#pragma once

#include "backend.h"

#include <cstdint>
#include <memory>
#include <string_view>

namespace openw3d::renderer {

enum class Format {
    Unknown,
    RGBA8Unorm,
    Depth24Stencil8,
};

struct DeviceDesc {
    std::string_view application_name;
    bool headless = false;
};

struct BufferDesc {
    std::uint64_t size = 0;
    bool index_buffer = false;
};

struct TextureDesc {
    std::uint32_t width = 1;
    std::uint32_t height = 1;
    Format format = Format::RGBA8Unorm;
};

class Device {
public:
    virtual ~Device() = default;
    virtual BackendInfo info() const = 0;
    virtual bool create_buffer(const BufferDesc&) = 0;
    virtual bool create_texture(const TextureDesc&) = 0;
    virtual bool begin_frame() = 0;
    virtual bool end_frame() = 0;
};

std::unique_ptr<Device> create_device(const DeviceDesc&);

} // namespace openw3d::renderer
