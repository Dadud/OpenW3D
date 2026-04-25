#ifndef WW3DBACKEND_H
#define WW3DBACKEND_H

#include "vector3.h"
#include "scene.h"
#include "rddesc.h"
#include "index.h"

class WW3DBackend
{
public:
    WW3DBackend();

    virtual bool Init(void * hwnd, bool lite = false);
    virtual void Shutdown();

    virtual bool Set_Any_Render_Device(void);
    virtual bool Set_Render_Device(const char * dev_name,int width=-1,int height=-1,int bits=-1,int windowed=-1,bool resize_window=false);
    virtual bool Set_Render_Device(int dev=-1,int resx=-1,int resy=-1,int bits=-1,int windowed=-1,bool resize_window = false);
    virtual bool Set_Next_Render_Device(void);
    virtual bool Toggle_Windowed(void);
    virtual bool Is_Windowed(void);

    virtual int Get_Render_Device_Count(void);
    virtual int Get_Render_Device(void);
    virtual const RenderDeviceDescClass & Get_Render_Device_Desc(int deviceidx);
    virtual const char * Get_Render_Device_Name(int device_index);
    virtual bool Set_Device_Resolution(int width=-1,int height=-1,int bits=-1,int windowed=-1, bool resize_window=false);
    virtual void Get_Device_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed);
    virtual void Get_Render_Target_Resolution(int & set_w,int & set_h,int & set_bits,bool & set_windowed);
    virtual int	Get_Device_Resolution_Width(void);
    virtual int	Get_Device_Resolution_Height(void);

    virtual bool Registry_Save_Render_Device( const char * sub_key );
    virtual bool Registry_Load_Render_Device( const char * sub_key, bool resize_window );
    virtual bool Registry_Save_Render_Device( const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth);
    virtual bool Registry_Load_Render_Device( const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth);

    virtual void Begin_Scene(void);
    virtual void End_Scene(bool flip_frame = true);

    // Flip until the primary buffer is visible.
    virtual void Flip_To_Primary(void);

    virtual void Clear_Full_Screen(void);
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z=1.0f, unsigned int stencil=0);

    virtual void SetRenderType(SceneClass::PolyRenderType);

    virtual void Set_Swap_Interval(int swap);
    virtual int Get_Swap_Interval(void);

    virtual void Set_Texture_Bitdepth(int depth);
    virtual int Get_Texture_Bitdepth(void);

};

#endif // WW3DBACKEND_H
