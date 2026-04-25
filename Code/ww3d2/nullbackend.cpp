#include "nullbackend.h"
#include "ww3dbackend.h"

// Disable warnings for unused parameters
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4100)
#endif

class NullBackend : public WW3DBackend
{
public:
    NullBackend() : WW3DBackend() {}

    bool Init(void* /*hwnd*/, bool /*lite*/ = false) override { return true; }
    void Shutdown() override {}

    bool Set_Any_Render_Device(void) override { return false; }
    bool Set_Render_Device(const char* /*dev_name*/, int /*width*/ /*= -1*/, int /*height*/ /*= -1*/, int /*bits*/ /*= -1*/, int /*windowed*/ /*= -1*/, bool /*resize_window*/ /*= false*/) override { return false; }
    bool Set_Render_Device(int /*dev*/ /*= -1*/, int /*resx*/ /*= -1*/, int /*resy*/ /*= -1*/, int /*bits*/ /*= -1*/, int /*windowed*/ /*= -1*/, bool /*resize_window*/ /*= false*/) override { return false; }
    bool Set_Next_Render_Device(void) override { return false; }
    bool Toggle_Windowed(void) override { return false; }
    bool Is_Windowed(void) override { return false; }

    int Get_Render_Device_Count(void) override { return 0; }
    int Get_Render_Device(void) override { return 0; }
    const RenderDeviceDescClass& Get_Render_Device_Desc(int /*deviceidx*/) override
    {
        static RenderDeviceDescClass dummy;
        return dummy;
    }
    const char* Get_Render_Device_Name(int /*device_index*/) override { return ""; }
    bool Set_Device_Resolution(int /*width*/ /*= -1*/, int /*height*/ /*= -1*/, int /*bits*/ /*= -1*/, int /*windowed*/ /*= -1*/, bool /*resize_window*/ /*= false*/) override { return false; }
    void Get_Device_Resolution(int& set_w, int& set_h, int& set_bits, bool& set_windowed) override
    {
        set_w = 0; set_h = 0; set_bits = 0; set_windowed = false;
    }
    void Get_Render_Target_Resolution(int& set_w, int& set_h, int& set_bits, bool& set_windowed) override
    {
        set_w = 0; set_h = 0; set_bits = 0; set_windowed = false;
    }
    int Get_Device_Resolution_Width(void) override { return 0; }
    int Get_Device_Resolution_Height(void) override { return 0; }

    bool Registry_Save_Render_Device(const char* /*sub_key*/) override { return false; }
    bool Registry_Load_Render_Device(const char* /*sub_key*/, bool /*resize_window*/) override { return false; }
    bool Registry_Save_Render_Device(const char* /*sub_key*/, int /*device*/, int /*width*/, int /*height*/, int /*depth*/, bool /*windowed*/, int /*texture_depth*/) override { return false; }
    bool Registry_Load_Render_Device(const char* /*sub_key*/, char* /*device*/, int /*device_len*/, int& /*width*/, int& /*height*/, int& /*depth*/, int& /*windowed*/, int& /*texture_depth*/) override { return false; }

    void Begin_Scene(void) override {}
    void End_Scene(bool /*flip_frame*/ /*= true*/) override {}

    void Flip_To_Primary(void) override {}

    void Clear_Full_Screen(void) override {}
    void Clear(bool /*clear_color*/, bool /*clear_z_stencil*/, const Vector3& /*color*/, float /*z*/ /*= 1.0f*/, unsigned int /*stencil*/ /*= 0*/) override {}

    void SetRenderType(SceneClass::PolyRenderType) override {}

    void Set_Swap_Interval(int /*swap*/) override {}
    int Get_Swap_Interval(void) override { return 0; }

    void Set_Texture_Bitdepth(int /*depth*/) override {}
    int Get_Texture_Bitdepth(void) override { return 0; }

    // Pure virtual methods from WW3DBackend
    BackendTextureHandle Create_Texture(int /*width*/, int /*height*/, WW3DFormat /*format*/, int /*mip_level_count*/, int /*pool*/, bool /*render_target*/) override
    {
        return nullptr;
    }

    BackendSurfaceHandle Create_Surface(int /*width*/, int /*height*/, WW3DFormat /*format*/, int /*pool*/) override
    {
        return nullptr;
    }

    void Set_Texture(BackendTextureHandle /*handle*/, unsigned int /*stage*/) override {}

    BackendTextureHandle Get_Missing_Texture() override
    {
        return nullptr;
    }

    BackendSurfaceHandle Create_Missing_Surface() override
    {
        return nullptr;
    }
};

// Factory method
WW3DBackend* WW3DBackend::Create_Null_Backend()
{
    return new NullBackend();
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
