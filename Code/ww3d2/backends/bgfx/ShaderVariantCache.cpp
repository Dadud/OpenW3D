#include "ShaderVariantCache.h"
#include <bgfx/bgfx.h>
#include <bx/file.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Release callback for bgfx::makeRef — frees memory allocated with malloc
static void ReleaseShaderMemory(void* _ptr, void* _userData)
{
    free(_ptr);
    (void)_userData;
}

bgfx::ShaderHandle ShaderVariantCache::LoadShader(const std::string& path)
{
    bx::FileReader reader;
    if (!bx::open(&reader, path.c_str()))
    {
        return BGFX_INVALID_HANDLE;
    }

    const uint32_t size = static_cast<uint32_t>(bx::getSize(&reader));
    if (size == 0)
    {
        bx::close(&reader);
        return BGFX_INVALID_HANDLE;
    }

    void* buffer = malloc(size);
    if (!buffer)
    {
        bx::close(&reader);
        return BGFX_INVALID_HANDLE;
    }

    bx::read(&reader, buffer, size, bx::ErrorAssert{});
    bx::close(&reader);

    const bgfx::Memory* mem = bgfx::makeRef(buffer, size, ReleaseShaderMemory, nullptr);
    if (!mem || !mem->data)
    {
        free(buffer);
        return BGFX_INVALID_HANDLE;
    }

    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    if (!bgfx::isValid(handle))
    {
        return BGFX_INVALID_HANDLE;
    }

    return handle;
}

// Map bgfx RendererType to the profile suffix used in compiled shader filenames.
// Must match the suffixes produced by Code/ww3d2/backends/bgfx/CMakeLists.txt.
static const char* GetRendererSuffix()
{
    bgfx::RendererType::Enum rt = bgfx::getRendererType();
    switch (rt)
    {
        case bgfx::RendererType::Vulkan:       return "spirv";
        case bgfx::RendererType::Metal:         return "metal";
        case bgfx::RendererType::OpenGLES:     return "120";
        case bgfx::RendererType::Direct3D11:    return "s_5_0";
        case bgfx::RendererType::Direct3D12:    return "s_5_0";
        case bgfx::RendererType::Direct3D9:     return "s_3_0";
        case bgfx::RendererType::OpenGL:        return "120";
        default:                                return "120";
    }
}

// Search paths tried when loading shaders. Order matters — prefer more specific.
static const char* kShaderSearchPaths[] = {
    "./shaders/",
    "../shaders/",
    "shaders/",
    "../../shaders/",
    nullptr
};

std::string ShaderVariantCache::GetShaderPath(const ShaderKey& key, bool vertex)
{
    const char* suffix = GetRendererSuffix();

    // Uber shader variant path: "vs_uber_<profile>.bin" or "fs_uber_<profile>.bin"
    // The ShaderKey hash is used only for cache lookups; all variants currently
    // share the same uber shader binary (key is stored in cache for future use
    // when per-variant shader compilation is added).
    char path[512];
    if (vertex)
    {
        snprintf(path, sizeof(path), "vs_uber_%s.bin", suffix);
    }
    else
    {
        snprintf(path, sizeof(path), "fs_uber_%s.bin", suffix);
    }
    return std::string(path);
}

const ShaderVariantCache::ShaderVariant* ShaderVariantCache::GetOrCreate(const ShaderKey& key)
{
    auto it = m_cache.find(key);
    if (it != m_cache.end())
    {
        it->second.uses++;
        return &it->second;
    }

    ShaderVariant variant = LoadOrCreateProgram(key);
    if (!bgfx::isValid(variant.program))
    {
        return nullptr;
    }

    variant.uses = 1;
    auto result = m_cache.emplace(key, std::move(variant));
    return &result.first->second;
}

ShaderVariantCache::ShaderVariant ShaderVariantCache::LoadOrCreateProgram(const ShaderKey& key)
{
    ShaderVariant variant;

    const char* suffix = GetRendererSuffix();

    // Build a list of candidate (vsPath, fsPath) pairs from search paths + suffixes
    // For the uber shader we have one fixed suffix per renderer, so we just try
    // all search paths with that suffix.
    const char* vsBaseName = "vs_uber_";
    const char* fsBaseName = "fs_uber_";
    size_t baseNameLen = 9; // strlen("vs_uber_")

    char vsCandidate[512];
    char fsCandidate[512];

    for (int i = 0; kShaderSearchPaths[i] != nullptr; ++i)
    {
        const char* dir = kShaderSearchPaths[i];

        // Build VS path: <dir>vs_uber_<suffix>.bin
        snprintf(vsCandidate, sizeof(vsCandidate), "%s%s%s.bin", dir, vsBaseName, suffix);
        snprintf(fsCandidate, sizeof(fsCandidate), "%s%s%s.bin", dir, fsBaseName, suffix);

        variant.vs = LoadShader(vsCandidate);
        if (!bgfx::isValid(variant.vs))
        {
            continue;
        }

        variant.fs = LoadShader(fsCandidate);
        if (!bgfx::isValid(variant.fs))
        {
            bgfx::destroy(variant.vs);
            variant.vs = BGFX_INVALID_HANDLE;
            continue;
        }

        variant.program = bgfx::createProgram(variant.vs, variant.fs, true);
        if (!bgfx::isValid(variant.program))
        {
            bgfx::destroy(variant.vs);
            bgfx::destroy(variant.fs);
            variant.vs = BGFX_INVALID_HANDLE;
            variant.fs = BGFX_INVALID_HANDLE;
            continue;
        }

        // createProgram with _destroyShaders=true means bgfx owns the shader handles
        variant.vs = BGFX_INVALID_HANDLE;
        variant.fs = BGFX_INVALID_HANDLE;
        return variant;
    }

    // Fallback: try the ShaderKey-derived path (kept for backwards compatibility)
    std::string vsPath = GetShaderPath(key, true);
    std::string fsPath = GetShaderPath(key, false);

    variant.vs = LoadShader(vsPath);
    if (!bgfx::isValid(variant.vs))
    {
        return variant;
    }

    variant.fs = LoadShader(fsPath);
    if (!bgfx::isValid(variant.fs))
    {
        bgfx::destroy(variant.vs);
        variant.vs = BGFX_INVALID_HANDLE;
        return variant;
    }

    variant.program = bgfx::createProgram(variant.vs, variant.fs, true);
    if (!bgfx::isValid(variant.program))
    {
        variant.vs = BGFX_INVALID_HANDLE;
        variant.fs = BGFX_INVALID_HANDLE;
        return variant;
    }

    variant.vs = BGFX_INVALID_HANDLE;
    variant.fs = BGFX_INVALID_HANDLE;
    return variant;
}

void ShaderVariantCache::Clear()
{
    for (auto& pair : m_cache)
    {
        ShaderVariant& variant = pair.second;
        if (bgfx::isValid(variant.program))
        {
            bgfx::destroy(variant.program);
        }
        // variant.vs / variant.fs are invalid when program was created with
        // _destroyShaders=true (bgfx owns and destroyed them).
        if (bgfx::isValid(variant.vs))
        {
            bgfx::destroy(variant.vs);
        }
        if (bgfx::isValid(variant.fs))
        {
            bgfx::destroy(variant.fs);
        }
    }
    m_cache.clear();
}
