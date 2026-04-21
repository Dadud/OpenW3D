// Stub header for Linux builds - D3DX math not available on Linux
#pragma once

#include <d3d9.h>  // For Windows-compatible types

// Map D3DX types to WWMath equivalents
#include "WWMath/vector3.h"
#include "WWMath/vector4.h"
#include "WWMath/matrix4.h"

#if defined(_WIN32)
#include <d3dx9math.h>
#else

// D3DXVECTOR3 - compatible with Vector3 (x,y,z float triplet)
typedef Vector3 D3DXVECTOR3;

// D3DXVECTOR4 - compatible with Vector4 (x,y,z,w float quad)
typedef Vector4 D3DXVECTOR4;

// D3DXMATRIX - 4x4 matrix, standalone implementation
// D3DXMATRIX is row-major (row,col indexing like D3D), Matrix4 is column-major internally
// But Matrix4 stores rows as Vector4 Row[4], so D3DXMATRIX mirrors that layout.
struct D3DXMATRIX {
	float m[4][4];

	D3DXMATRIX() {}
	explicit D3DXMATRIX(bool identity) { if (identity) Make_Identity(); }
	D3DXMATRIX(const Matrix4& mat) { *this = mat; }
	D3DXMATRIX(const D3DXMATRIX& mat) { for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = mat.m[i][j]; }

	// Assignment from Matrix4 (row-major D3DXMATRIX = column-major Matrix4, so transpose on copy)
	D3DXMATRIX& operator=(const Matrix4& mat) {
		for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = mat[j][i];
		return *this;
	}

	WWINLINE void Make_Identity() { for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = (i == j) ? 1.0f : 0.0f; }

	// D3DX-style (row, col) access
	float& operator()(unsigned int row, unsigned int col) { return m[row][col]; }
	const float& operator()(unsigned int row, unsigned int col) const { return m[row][col]; }

	// D3DX-style named element accessors (return by value for const, ref for non-const)
	float& _11() { return m[0][0]; } float& _12() { return m[0][1]; } float& _13() { return m[0][2]; } float& _14() { return m[0][3]; }
	float& _21() { return m[1][0]; } float& _22() { return m[1][1]; } float& _23() { return m[1][2]; } float& _24() { return m[1][3]; }
	float& _31() { return m[2][0]; } float& _32() { return m[2][1]; } float& _33() { return m[2][2]; } float& _34() { return m[2][3]; }
	float& _41() { return m[3][0]; } float& _42() { return m[3][1]; } float& _43() { return m[3][2]; } float& _44() { return m[3][3]; }
	const float _11() const { return m[0][0]; } const float _12() const { return m[0][1]; } const float _13() const { return m[0][2]; } const float _14() const { return m[0][3]; }
	const float _21() const { return m[1][0]; } const float _22() const { return m[1][1]; } const float _23() const { return m[1][2]; } const float _24() const { return m[1][3]; }
	const float _31() const { return m[2][0]; } const float _32() const { return m[2][1]; } const float _33() const { return m[2][2]; } const float _34() const { return m[2][3]; }
	const float _41() const { return m[3][0]; } const float _42() const { return m[3][1]; } const float _43() const { return m[3][2]; } const float _44() const { return m[3][3]; }

	// For implicit conversion back to Matrix4
	operator Matrix4() const {
		Matrix4 result;
		for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) result[i][j] = m[i][j];
		return result;
	}
};

// D3DXMatrixTranspose - returns transpose of a matrix
WWINLINE void D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pIn)
{
	WWASSERT(pOut);
	WWASSERT(pIn);
	for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) pOut->m[i][j] = pIn->m[j][i];
}

// D3DXMatrixIdentity - sets matrix to identity
WWINLINE void D3DXMatrixIdentity(D3DXMATRIX* pOut)
{
	WWASSERT(pOut);
	pOut->Make_Identity();
}

// D3DXMatrixMultiply - multiplies two matrices
WWINLINE void D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pA, const D3DXMATRIX* pB)
{
	WWASSERT(pOut);
	WWASSERT(pA);
	WWASSERT(pB);
	// Row-major multiplication: result[i][j] = sum_k A[i][k] * B[k][j]
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			pOut->m[i][j] = 0.0f;
			for (int k = 0; k < 4; k++) {
				pOut->m[i][j] += pA->m[i][k] * pB->m[k][j];
			}
		}
	}
}

// D3DXVec3Transform - transforms a 3D vector by a matrix (produces a 4D vector result)
// Stores the transformed vector in pOut; Input is 3D vector, 4x4 matrix
WWINLINE void D3DXVec3Transform(D3DXVECTOR4* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
	WWASSERT(pOut);
	WWASSERT(pV);
	WWASSERT(pM);
	*pOut = (*pM) * Vector4(pV->X, pV->Y, pV->Z, 1.0f);
}

// D3DXVec3TransformCoord - transforms a 3D vector by a matrix and returns 3D result (w=1)
WWINLINE void D3DXVec3TransformCoord(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
	WWASSERT(pOut);
	WWASSERT(pV);
	WWASSERT(pM);
	Vector4 result = (*pM) * Vector4(pV->X, pV->Y, pV->Z, 1.0f);
	float invW = 1.0f / result.W;
	pOut->X = result.X * invW;
	pOut->Y = result.Y * invW;
	pOut->Z = result.Z * invW;
}

#endif // !_WIN32
