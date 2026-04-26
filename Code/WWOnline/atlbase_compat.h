#pragma once

#ifdef _MSC_VER

// Try to use ATL if available (defined when ATL headers are present)
#if defined(_ATL_VER)
#include <atlbase.h>
#else
// ATL not available - use minimal fallback
#ifdef _WIN32
#include <windows.h>
#include <ocidl.h>
#include <unknwn.h>
#include <combaseapi.h>
#else
// Stub COM types for Linux
#define IID void
typedef void* IUnknown;
typedef void* IConnectionPointContainer;
typedef void* IConnectionPoint;
typedef unsigned int DWORD;
typedef void* LPVOID;
typedef int HRESULT;
#define E_INVALIDARG 0x80070057
#define SUCCEEDED(hr) ((hr) >= 0)
#define S_OK 0
#endif

// Fallback CComPtr when ATL is not installed
template<typename T> class CComPtr;

template <typename T>
booloperator==(CComPtr<T>* pLeft, T* pRight);

template <typename T>
booloperator!=(CComPtr<T>* pLeft, T* pRight);

template <typename T>
class CComPtr final
{
public:
    CComPtr() : p(nullptr) {}
    explicit CComPtr(T* object) { Attach(object); }
    CComPtr(CComPtr&& other) : p(other.p) { other.p = nullptr; }
    CComPtr& operator=(CComPtr&& other) { T* obj = p; p = other.p; other.p = obj; return *this; }
    ~CComPtr() { Release(); }
    void Release() { if (p) { p->Release(); p = nullptr; } }
    void Attach(T* obj) { Release(); p = obj; if (p) p->AddRef(); }
    T* Detach() { T* obj = p; p = nullptr; return obj; }
    bool operator==(T* other) const { return p == other; }
    bool operator!=(T* other) const { return p != other; }
    T** operator&() { return &p; }
    operator T*() const { return p; }
    T* operator->() const { return p; }
    T* p;
};

template <typename T>
booloperator==(CComPtr<T>* pLeft, T* pRight) { return *pLeft == pRight; }

template <typename T>
booloperator!=(CComPtr<T>* pLeft, T* pRight) { return *pLeft != pRight; }

#endif // _ATL_VER

#else // !_MSC_VER

#ifdef _WIN32
#include <windows.h>
#include <ocidl.h>
#include <unknwn.h>
#else
// Stub COM types for Linux
#define IID void
typedef void* IUnknown;
typedef void* IConnectionPointContainer;
typedef void* IConnectionPoint;
typedef unsigned int DWORD;
typedef void* LPVOID;
typedef int HRESULT;
#define E_INVALIDARG 0x80070057
#define SUCCEEDED(hr) ((hr) >= 0)
#define S_OK 0
#endif

template <typename T>
class CComPtr final
{
public:
    CComPtr()
    : p(nullptr) {
    }
    explicit CComPtr(T *object) {
        Attach(object);
    }
    CComPtr(CComPtr &other)
    : p(other.p) {
        if (p) {
            p->AddRef();
        }
    }
    CComPtr(CComPtr &&other)
    : p(other.p) {
        other.p = nullptr;
    }
    CComPtr & operator=(CComPtr &other) {
        if (other.p != this->p) {
            Attach(other.p);
            if (p) {
                p->AddRef();
            }
        }
        return *this;
    }
    CComPtr & operator=(CComPtr &&other) {
        T *object = p;
        p = other.p;
        other.p = object;
        return *this;
    }
    CComPtr & operator=(T *object) {
        if (object != this->p) {
            Release();
            p = object;
            p->AddRef();
        }
        return *this;
    }

    ~CComPtr() {
        Release();
    }

    void Release() {
        if (p) {
            p->Release();
            p = nullptr;
        }
    }

    void Attach(T* p) {
        Release();
        p = p;
    }

    T * Detach() {
        T *obj = p;
        p = nullptr;
        return obj;
    }

    bool operator==(T* object) const
    {
        return p == object;
    }

    bool operator!=(T* object) const
    {
        return p != object;
    }

    T ** operator &() {
        return &p;
    }

    operator T*() const {
        return p;
    }

    T *operator->() const {
        return p;
    }

    T *p;
};


inline HRESULT AtlAdvise(
        IUnknown* pUnkCP,
        IUnknown* pUnk,
        void* iid,
        DWORD* pdw)
{
#ifdef _WIN32
    if(pUnkCP == NULL) {
        return E_INVALIDARG;
    }

    CComPtr<IConnectionPointContainer> pCPC;
    CComPtr<IConnectionPoint> pCP;
    HRESULT hRes = pUnkCP->QueryInterface(__uuidof(IConnectionPointContainer), (void**)&pCPC);
    if (SUCCEEDED(hRes)) {
        hRes = pCPC->FindConnectionPoint(iid, &pCP);
    }
    if (SUCCEEDED(hRes)) {
        hRes = pCP->Advise(pUnk, pdw);
    }
    return hRes;
#else
    (void)pUnkCP;
    (void)pUnk;
    (void)iid;
    (void)pdw;
    return S_OK;
#endif
}

inline HRESULT AtlUnadvise(
        IUnknown* pUnkCP,
        void* iid,
        DWORD dw)
{
#ifdef _WIN32
    if (pUnkCP == NULL) {
        return E_INVALIDARG;
    }

    CComPtr<IConnectionPointContainer> pCPC;
    CComPtr<IConnectionPoint> pCP;
    HRESULT hRes = pUnkCP->QueryInterface(__uuidof(IConnectionPointContainer), (void**)&pCPC);
    if (SUCCEEDED(hRes)) {
        hRes = pCPC->FindConnectionPoint(iid, &pCP);
    }
    if (SUCCEEDED(hRes)) {
        hRes = pCP->Unadvise(dw);
    }
    return hRes;
#else
    (void)pUnkCP;
    (void)iid;
    (void)dw;
    return S_OK;
#endif
}

#endif
