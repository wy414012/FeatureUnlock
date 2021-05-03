//
//  kern_start.cpp
//  SidecarFixup.kext
//
//  Copyright © 2020 osy86, DhinakG, 2021 Mykola Grymalyuk. All rights reserved.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include <Headers/kern_user.hpp>

#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define MODULE_SHORT "sidecar"

extern "C" void *memmem(const void *h0, size_t k, const void *n0, size_t l);

static const int kPathMaxLen = 1024;

#pragma mark - Patches

// Replaces Mac with Nac in SidecarCore
static const uint8_t kMacModelOriginal[] = { 0x69, 0x4D, 0x61, 0x63, 0x31, 0x33, 0x2C, 0x31, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x33, 0x2C, 0x32, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x33, 0x2C, 0x33, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x31, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x32, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x33, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x34, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x35, 0x2C, 0x31, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x36, 0x2C, 0x31, 0x00, 0x69, 0x4D, 0x61, 0x63, 0x31, 0x36, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x38, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x35, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x35, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x36, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x36, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x37, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x37, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x39, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x39, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x30, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x30, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x33, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x34, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x35, 0x00, 0x4D, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x32, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x6D, 0x69, 0x6E, 0x69, 0x36, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x6D, 0x69, 0x6E, 0x69, 0x36, 0x2C, 0x32, 0x00, 0x4D, 0x61, 0x63, 0x6D, 0x69, 0x6E, 0x69, 0x37, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x50, 0x72, 0x6F, 0x35, 0x2C, 0x31, 0x00, 0x4D, 0x61, 0x63, 0x50, 0x72, 0x6F, 0x36, 0x2C, 0x31
};

static const uint8_t kMacModelPatched[] = { 0x69, 0x4E, 0x61, 0x63, 0x31, 0x33, 0x2C, 0x31, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x33, 0x2C, 0x32, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x33, 0x2C, 0x33, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x31, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x32, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x33, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x34, 0x2C, 0x34, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x35, 0x2C, 0x31, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x36, 0x2C, 0x31, 0x00, 0x69, 0x4E, 0x61, 0x63, 0x31, 0x36, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x38, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x35, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x35, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x36, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x36, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x37, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x41, 0x69, 0x72, 0x37, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x39, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x39, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x30, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x30, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x33, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x34, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x31, 0x2C, 0x35, 0x00, 0x4E, 0x61, 0x63, 0x42, 0x6F, 0x6F, 0x6B, 0x50, 0x72, 0x6F, 0x31, 0x32, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x6D, 0x69, 0x6E, 0x69, 0x36, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x6D, 0x69, 0x6E, 0x69, 0x36, 0x2C, 0x32, 0x00, 0x4E, 0x61, 0x63, 0x6D, 0x69, 0x6E, 0x69, 0x37, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x50, 0x72, 0x6F, 0x35, 0x2C, 0x31, 0x00, 0x4E, 0x61, 0x63, 0x50, 0x72, 0x6F, 0x36, 0x2C, 0x31
};

// Replaces iPad with hPad
static const uint8_t kIPadModelOriginal[] = { 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x31, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x32, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x33, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x34, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x35, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x36, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x37, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x38, 0x00, 0x69, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x39, 0x00, 0x69, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x31, 0x00, 0x69, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x32, 0x00, 0x69, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x33, 0x00, 0x69, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x34, 0x00, 0x69, 0x50, 0x61, 0x64, 0x36, 0x2C, 0x31, 0x31, 0x00, 0x69, 0x50, 0x61, 0x64, 0x36, 0x2C, 0x31, 0x32
};

static const uint8_t kIPadModelPatched[] = { 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x31, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x32, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x33, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x34, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x35, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x36, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x37, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x38, 0x00, 0x68, 0x50, 0x61, 0x64, 0x34, 0x2C, 0x39, 0x00, 0x68, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x31, 0x00, 0x68, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x32, 0x00, 0x68, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x33, 0x00, 0x68, 0x50, 0x61, 0x64, 0x35, 0x2C, 0x34, 0x00, 0x68, 0x50, 0x61, 0x64, 0x36, 0x2C, 0x31, 0x31, 0x00, 0x68, 0x50, 0x61, 0x64, 0x36, 0x2C, 0x31, 0x32
};

static constexpr size_t kMacModelOriginalSize = sizeof(kMacModelOriginal);

static_assert(kMacModelOriginalSize == sizeof(kMacModelPatched), "patch size invalid");

static constexpr size_t kIPadModelOriginalSize = sizeof(kIPadModelOriginal);

static_assert(kIPadModelOriginalSize == sizeof(kIPadModelPatched), "patch size invalid");

static const char kSidecarCorePath[kPathMaxLen] = "/System/Library/PrivateFrameworks/SidecarCore.framework/Versions/A/SidecarCore";

static const char kDyldCachePath[kPathMaxLen] = "/private/var/db/dyld/dyld_shared_cache_x86_64h";

static const char kBigSurDyldCachePath[kPathMaxLen] = "/System/Library/dyld/dyld_shared_cache_x86_64h";

static mach_vm_address_t orig_cs_validate {};

#pragma mark - Kernel patching code

/**
 * Call block with interrupts and protections disabled
 */
static void doKernelPatch(void (^patchFunc)(void)) {
    if (MachInfo::setKernelWriting(true, KernelPatcher::kernelWriteLock) == KERN_SUCCESS) {
        DBGLOG(MODULE_SHORT, "obtained write permssions");
    } else {
        SYSLOG(MODULE_SHORT, "failed to obtain write permissions");
        return;
    }
    
    patchFunc();
    
    if (MachInfo::setKernelWriting(false, KernelPatcher::kernelWriteLock) == KERN_SUCCESS) {
        DBGLOG(MODULE_SHORT, "restored write permssions");
    } else {
        SYSLOG(MODULE_SHORT, "failed to restore write permissions");
    }
}

template <size_t patchSize>
static inline void searchAndPatch(const void *haystack,
                                  size_t haystackSize,
                                  const char (&path)[kPathMaxLen],
                                  const uint8_t (&needle)[patchSize],
                                  const uint8_t (&patch)[patchSize]) {
    if (UNLIKELY(strncmp(path, kSidecarCorePath, sizeof(kSidecarCorePath)) == 0) ||
        UNLIKELY(strncmp(path, UserPatcher::getSharedCachePath(), sizeof(UserPatcher::getSharedCachePath())) == 0)) {
        void *res;
        if (UNLIKELY((res = memmem(haystack, haystackSize, needle, patchSize)) != NULL)) {
            SYSLOG(MODULE_SHORT, "found function to patch!");
            SYSLOG(MODULE_SHORT, "path: %s", path);
            doKernelPatch(^{
                lilu_os_memcpy(res, patch, patchSize);
            });
        }
    }
}

#pragma mark - Patched functions

// pre Big Sur
static boolean_t patched_cs_validate_range(vnode_t vp,
                                           memory_object_t pager,
                                           memory_object_offset_t offset,
                                           const void *data,
                                           vm_size_t size,
                                           unsigned *result) {
    char path[kPathMaxLen];
    int pathlen = kPathMaxLen;
    boolean_t res = FunctionCast(patched_cs_validate_range, orig_cs_validate)(vp, pager, offset, data, size, result);
    if (res && vn_getpath(vp, path, &pathlen) == 0) {
        searchAndPatch(data, size, path, kMacModelOriginal, kMacModelPatched);
        searchAndPatch(data, size, path, kIPadModelOriginal, kIPadModelPatched);
    }
    return res;
}

// For Big Sur
static void patched_cs_validate_page(vnode_t vp,
                                          memory_object_t pager,
                                          memory_object_offset_t page_offset,
                                          const void *data,
                                          int *arg4,
                                          int *arg5,
                                          int *arg6) {
    char path[kPathMaxLen];
    int pathlen = kPathMaxLen;
    FunctionCast(patched_cs_validate_page, orig_cs_validate)(vp, pager, page_offset, data, arg4, arg5, arg6);
    if (vn_getpath(vp, path, &pathlen) == 0) {
        searchAndPatch(data, PAGE_SIZE, path, kMacModelOriginal, kMacModelPatched);
        searchAndPatch(data, PAGE_SIZE, path, kIPadModelOriginal, kIPadModelPatched);
    }
}

#pragma mark - Patches on start/stop

static void pluginStart() {
    LiluAPI::Error error;
    
    DBGLOG(MODULE_SHORT, "start");
    if (getKernelVersion() < KernelVersion::BigSur) {
        error = lilu.onPatcherLoad([](void *user, KernelPatcher &patcher){
            DBGLOG(MODULE_SHORT, "patching cs_validate_range");
            mach_vm_address_t kern = patcher.solveSymbol(KernelPatcher::KernelID, "_cs_validate_range");
            
            if (patcher.getError() == KernelPatcher::Error::NoError) {
                orig_cs_validate = patcher.routeFunctionLong(kern, reinterpret_cast<mach_vm_address_t>(patched_cs_validate_range), true, true);
                
                if (patcher.getError() != KernelPatcher::Error::NoError) {
                    SYSLOG(MODULE_SHORT, "failed to hook _cs_validate_range");
                } else {
                    DBGLOG(MODULE_SHORT, "hooked cs_validate_range");
                }
            } else {
                SYSLOG(MODULE_SHORT, "failed to find _cs_validate_range");
            }
        });
    } else { // >= macOS 11
        error = lilu.onPatcherLoad([](void *user, KernelPatcher &patcher){
            DBGLOG(MODULE_SHORT, "patching cs_validate_page");
            mach_vm_address_t kern = patcher.solveSymbol(KernelPatcher::KernelID, "_cs_validate_page");
            
            if (patcher.getError() == KernelPatcher::Error::NoError) {
                orig_cs_validate = patcher.routeFunctionLong(kern, reinterpret_cast<mach_vm_address_t>(patched_cs_validate_page), true, true);
                
                if (patcher.getError() != KernelPatcher::Error::NoError) {
                    SYSLOG(MODULE_SHORT, "failed to hook _cs_validate_page");
                } else {
                    DBGLOG(MODULE_SHORT, "hooked cs_validate_page");
                }
            } else {
                SYSLOG(MODULE_SHORT, "failed to find _cs_validate_page");
            }
        });
    }
    if (error != LiluAPI::Error::NoError) {
        SYSLOG(MODULE_SHORT, "failed to register onPatcherLoad method: %d", error);
    }
}

// Boot args.
static const char *bootargOff[] {
    "-sidecaroff"
};
static const char *bootargDebug[] {
    "-sidecardbg"
};
static const char *bootargBeta[] {
    "-sidecarbeta"
};

// Plugin configuration.
PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::Catalina,
    KernelVersion::BigSur,
    pluginStart
};
