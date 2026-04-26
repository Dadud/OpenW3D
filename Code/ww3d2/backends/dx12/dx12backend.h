/*
**	Community contribution - Licensed under GPLv3
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// DX12 backend for ww3d2.
// Wraps DX12 API behind the WW3DBackend interface.

#ifndef DX12BACKEND_H
#define DX12BACKEND_H

#include "ww3dbackend.h"

/************************************************************************************************
 * DX12-specific backend implementation.                                                    *
 * All DX12 COM objects are stored as void* to avoid d3d12.h inclusion in the header.        *
 * Include d3d12.h and dxgi.h in the .cpp file before using them.                            *
 ************************************************************************************************/

class DX12Backend : public WW3DBackend
{
public:
    DX12Backend();
    virtual ~DX12Backend();

    // WW3DBackend interface - Initialization
    virtual bool Init(void * hwnd, bool lite = false) override;
    virtual void Shutdown() override;

    // WW3DBackend interface - Device
    virtual bool Set_Any_Render_Device() override;
    virtual bool Set_Render_Device(const char * dev_name, int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual bool Set_Render_Device(int dev = -1, int resx = -1, int resy = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual bool Set_Next_Render_Device() override;
    virtual bool Toggle_Windowed() override;
    virtual bool Is_Windowed() override;

    virtual int Get_Render_Device_Count() override;
    virtual int Get_Render_Device() override;
    virtual const RenderDeviceDescClass & Get_Render_Device_Desc(int deviceidx) override;
    virtual const char * Get_Render_Device_Name(int device_index) override;
    virtual bool Set_Device_Resolution(int width = -1, int height = -1, int bits = -1, int windowed = -1, bool resize_window = false) override;
    virtual void Get_Device_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) override;
    virtual void Get_Render_Target_Resolution(int & set_w, int & set_h, int & set_bits, bool & set_windowed) override;
    virtual int Get_Device_Resolution_Width() override;
    virtual int Get_Device_Resolution_Height() override;

    // WW3DBackend interface - Registry
    virtual bool Registry_Save_Render_Device(const char * sub_key) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, bool resize_window) override;
    virtual bool Registry_Save_Render_Device(const char *sub_key, int device, int width, int height, int depth, bool windowed, int texture_depth) override;
    virtual bool Registry_Load_Render_Device(const char * sub_key, char *device, int device_len, int &width, int &height, int &depth, int &windowed, int &texture_depth) override;

    // WW3DBackend interface - Swapchain & Presentation
    virtual bool Create_Swapchain(int width, int height) override;
    virtual void Begin_Scene() override;
    virtual void End_Scene(bool flip_frame = true) override;
    virtual void Flip_To_Primary() override;
    virtual void Clear(bool clear_color, bool clear_z_stencil, const Vector3 &color, float z = 1.0f, unsigned int stencil = 0) override;
    virtual void Set_Swap_Interval(int swap) override;
    virtual int Get_Swap_Interval() override;

    // WW3DBackend interface - Texture
    virtual void Set_Texture_Bitdepth(int depth) override;
    virtual int Get_Texture_Bitdepth() override;

    // WW3DBackend interface - Viewport & Render Target
    virtual void Set_Viewport(const void* viewport) override;
    virtual void Set_Render_Target(void* target) override;  // nullptr = reset to default
    virtual void Set_DX8_Render_State(int state, unsigned value) override;
    virtual void Set_Light_Environment(const void* env) override;

    // WW3DBackend interface - Surface access
    virtual void Get_Front_Buffer_Surface(BackendSurfaceHandle* out_handle) override;
    virtual void Lock_Front_Buffer_Surface(BackendSurfaceHandle* handle, int width, int height, SurfaceLockData* out_data) override;
    virtual void Unlock_Front_Buffer_Surface(BackendSurfaceHandle* handle) override;

private:
    // DX12 objects stored as void* to keep d3d12.h out of the header
    void* m_dxgi_factory;
    void* m_adapter;
    void* m_device;
    void* m_command_queue;
    void* m_swap_chain;
    void* m_rtv_heap;
    void* m_command_allocator;
    void* m_command_list;
    void* m_fence;
    void* m_srv_heap;
    void* m_dsv_heap;

    void* m_hwnd;
    int m_width;
    int m_height;
    int m_bit_depth;
    bool m_windowed;
    bool m_initialized;
    int m_vsync;
    int m_texture_bit_depth;
    int m_current_device_index;
    int m_render_device;
    void* m_render_target;
    void* m_default_render_target;
    void* m_bound_textures[8];

    unsigned int m_rtv_descriptor_size;
    unsigned int m_srv_descriptor_size;
    unsigned int m_dsv_descriptor_size;
    unsigned long long m_fence_value;
    void* m_fence_event;

    bool Create_DX12_Device(void* adapter);
    bool Create_Command_Objects();
    bool Create_Swap_Chain_Buffers();
    bool Create_Descriptor_Heaps();
    bool Create_Default_Render_Target();
    void Wait_for_GPU();
    void MoveToNextFrame();
    void Bind_Texture(unsigned int slot, void* texture);
};

#endif // DX12BACKEND_H
