#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#include "wrappedldlinux.h"

#include "wrapper.h"
#include "bridge.h"
#include "library_private.h"
#include "x86emu.h"

int wrappedldlinux_init(library_t* lib)
{
    lib->priv.w.lib = dlopen("ld-linux.so.2", RTLD_NOW);
    if(!lib->priv.w.lib) {
        return -1;
    }
    lib->priv.w.bridge = NewBridge();
    return 0;
}
void wrappedldlinux_fini(library_t* lib)
{
    if(lib->priv.w.lib)
        dlclose(lib->priv.w.lib);
    lib->priv.w.lib = NULL;
    FreeBridge(&lib->priv.w.bridge);
}
int wrappedldlinux_get(library_t* lib, const char* name, uintptr_t *offs, uint32_t *sz)
{
    uintptr_t addr = 0;
    uint32_t size = 0;
    void* symbol = NULL;

#define GO(N, W) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
#define GOM(N, W) \
    if(strcmp(name, #N)==0) { \
        size = 12; \
        addr = AddBridge(lib->priv.w.bridge, W, my_##N); \
    } else
#define GO2(N, W, O) \
    if(strcmp(name, #N)==0) { \
        size = 12; \
        symbol=dlsym(lib->priv.w.lib, #O); \
        addr = AddBridge(lib->priv.w.bridge, W, symbol); \
    } else
#define DATA(N, W, S) \
    if(strcmp(name, #N)==0) { \
        symbol=dlsym(lib->priv.w.lib, #N); \
        size = S; \
        addr = (uintptr_t)symbol; \
    } else
#define END() {}

#include "wrappedldlinux_private.h"

#undef GO
#undef GOM
#undef GO2
#undef DATA
#undef END

    if(!addr)
        return 0;
    *offs = addr;
    *sz = size;
    return 1;
}

