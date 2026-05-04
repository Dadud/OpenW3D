# Set source files
set(WWLIB_SRC
    argv.cpp
    b64pipe.cpp
    b64straw.cpp
    base64.cpp
    buff.cpp
    bufffile.cpp
    chunkio.cpp
    crc.cpp
    cstraw.cpp
    FastAllocator.cpp
    ffactory.cpp
    gcd_lcm.cpp
    hash.cpp
    int.cpp
    jshell.cpp
    lzo.cpp
    lzo1x_c.cpp
    lzo1x_d.cpp
    mpmath.cpp
    mpu.cpp
    multilist.cpp
    mutex.cpp
    nstrdup.cpp
    pipe.cpp
    pk.cpp
    ramfile.cpp
    random.cpp
    realcrc.cpp
    rndstrng.cpp
    slnode.cpp
    straw.cpp
    systimer.cpp
    tagblock.cpp
    TARGA.CPP
    textfile.cpp
    trim.cpp
    vector.cpp
    widestring.cpp
    wwfile.cpp
    wwstring.cpp
    xpipe.cpp
    xstraw.cpp
    always.h
    argv.h
    b64pipe.h
    b64straw.h
    base64.h
    binheap.h
    bittype.h
    bound.h
    bsearch.h
    buff.h
    bufffile.h
    CallbackHook.h
    chunkio.h
    crc.h
    cstraw.h
    FastAllocator.h
    ffactory.h
    font.h
    gcd_lcm.h
    hash.h
    hashcalc.h
    hashlist.h
    hashtemplate.h
    iff.h
    index.h
    inisup.h
    int.h
    iostruct.h
    listnode.h
    lzo.h
    lzo1x.h
    lzo_conf.h
    lzoconf.h
    mempool.h
    mpmath.h
    mpu.h
    multilist.h
    mutex.h
    Notify.h
    nstrdup.h
    ntree.h
    pipe.h
    pk.h
    point.h
    ramfile.h
    random.h
    rcfile.h
    readline.h
    realcrc.h
    ref_ptr.h
    rndstraw.h
    rng.h
    sharebuf.h
    Signaler.h
    simplevec.h
    slist.h
    slnode.h
    straw.h
    systimer.h
    tagblock.h
    TARGA.H
    textfile.h
    timer.h
    trect.h
    trim.h
    uarray.h
    vector.h
    widestring.h
    win.h
    wwstring.h
    xmouse.h
    xpipe.h
    xstraw.h
)

# Platform-specific files — only included on WIN32
if(WIN32)
    list(APPEND WWLIB_SRC
        Except.cpp
        cpudetect.cpp
        LaunchWeb.cpp
        msgloop.cpp
        openw3d.cpp
        rawfile.cpp
        refcount.cpp
        thread.cpp
        verchk.cpp
        win.cpp
        wwdialog.cpp
        Except.h
        cpudetect.h
        LaunchWeb.h
        msgloop.h
        openw3d.h
        rawfile.h
        refcount.h
        thread.h
        verchk.h
        win.h
        wwdialog.h
    )
endif()

# Files with Win32 API calls that need runtime guards
list(APPEND WWLIB_SRC
    ini.cpp
    mixfile.cpp
    registry.cpp
    ini.h
    mixfile.h
    registry.h
)

if(W3D_BUILD_OPTION_FFMPEG AND FFmpeg_FOUND)
    list(APPEND WWLIB_SRC FFMpegFile.cpp FFMpegFile.h)
endif()

if(TARGET icu)
    list(APPEND WWLIB_SRC icu/unichar.h)
else()
    list(APPEND WWLIB_SRC wchar/unichar.h)
endif()

# Targets to build.
add_library(wwlib STATIC)

target_link_libraries(wwlib PRIVATE
    wwcommon
    wwutil
    version
)

if(W3D_BUILD_OPTION_SDL3)
    target_link_libraries(wwlib PRIVATE SDL3::SDL3-shared)
    target_link_libraries(wwlib PUBLIC SDL3::Headers)
endif()

target_sources(wwlib PRIVATE ${WWLIB_SRC})
