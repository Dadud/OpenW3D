#include "WWAudio.h"
#include "render2dsentence.h"
#include <d3dx9.h>

WWAudioClass *WWAudioClass::_theInstance = nullptr;

int WWAudioClass::Create_Instant_Sound(int, const Matrix3D &, RefCountClass *, uint32, int)
{
    return 0;
}

int WWAudioClass::Create_Instant_Sound(const char *, const Matrix3D &, RefCountClass *, uint32, int)
{
    return 0;
}

void FontCharsClass::Add_Font(const char *)
{
}

void FontCharsClass::Remove_Font(const char *)
{
}

D3DXVECTOR4 *WINAPI D3DXVec3Transform(D3DXVECTOR4 *out, const D3DXVECTOR3 *v, const D3DXMATRIX *m)
{
    out->x = v->x * m->m[0][0] + v->y * m->m[1][0] + v->z * m->m[2][0] + m->m[3][0];
    out->y = v->x * m->m[0][1] + v->y * m->m[1][1] + v->z * m->m[2][1] + m->m[3][1];
    out->z = v->x * m->m[0][2] + v->y * m->m[1][2] + v->z * m->m[2][2] + m->m[3][2];
    out->w = v->x * m->m[0][3] + v->y * m->m[1][3] + v->z * m->m[2][3] + m->m[3][3];
    return out;
}
