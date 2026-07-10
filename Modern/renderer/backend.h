#pragma once

namespace openw3d::renderer {

enum class BackendKind {
    LegacyD3D9Compatibility,
    Null,
};

struct BackendInfo {
    BackendKind kind;
    const char* name;
    bool modern_api_boundary;
};

BackendInfo selected_backend();

} // namespace openw3d::renderer
