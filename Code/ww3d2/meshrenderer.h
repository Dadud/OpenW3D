#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include "camera.h"
#include "mesh.h"
#include "simplevec.h"

class MeshRenderer
{
public:
    MeshRenderer();
    virtual ~MeshRenderer();

    virtual void						Init();
    virtual void						Shutdown();

    virtual void						Flush();
    virtual void						Clear_Pending_Delete_Lists();

    virtual void						Log_Statistics_String(bool only_visible);
    static void				Request_Log_Statistics();

    virtual void						Register_Mesh_Type(MeshClass* mesh);
    virtual void						Unregister_Mesh_Type(MeshClass* mesh);

    virtual void						Set_Camera(CameraClass* cam) { camera=cam; }
    virtual CameraClass *			Peek_Camera(void)	{ return camera; }
    virtual void						Add_To_Render_List(DecalMeshClass * decalmesh);

    // Enable or disable lighting on all objects inserted from now on. (Doesn't affect the objects that are already in the lists)
    virtual void						Enable_Lighting(bool enable) { enable_lighting=enable; }

    // This should be called at the beginning of a game or menu or after a major modifications to the scene...
    virtual void						Invalidate();

protected:
    void Render_Decal_Meshes(void);

    bool													enable_lighting;
    CameraClass *										camera;
    DecalMeshClass *									visible_decal_meshes;
};

#endif // MESHRENDERER_H
