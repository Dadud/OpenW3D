#include "rhi.h"

namespace openw3d::renderer {
namespace {

class NullDevice final : public Device {
public:
    BackendInfo info() const override {
        return {BackendKind::Null, "null", true};
    }

    bool create_buffer(const BufferDesc& desc) override {
        return desc.size > 0;
    }

    bool create_texture(const TextureDesc& desc) override {
        return desc.width > 0 && desc.height > 0 && desc.format != Format::Unknown;
    }

    bool begin_frame() override { return true; }
    bool end_frame() override { return true; }
};

} // namespace

std::unique_ptr<Device> create_device(const DeviceDesc&) {
    return std::make_unique<NullDevice>();
}

} // namespace openw3d::renderer
