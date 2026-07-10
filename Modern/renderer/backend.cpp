#include "backend.h"

namespace openw3d::renderer {

BackendInfo selected_backend() {
#if defined(W3D_RENDERER_NULL)
    return {BackendKind::Null, "null", true};
#else
    return {BackendKind::LegacyD3D9Compatibility, "legacy-d3d9-compatibility", false};
#endif
}

} // namespace openw3d::renderer
