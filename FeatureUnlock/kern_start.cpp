//
//  kern_start.cpp
//  FeatureUnlock.kext
//
//  Copyright © 2020 osy86, DhinakG, 2021-2022 Mykola Grymalyuk. All rights reserved.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include <Headers/kern_user.hpp>
#include <Headers/kern_devinfo.hpp>
#include "kern_dyld_patch.hpp"
#include "kern_args.hpp"

#define MODULE_SHORT "fu_fix"

static mach_vm_address_t orig_cs_validate {};


bool allow_sidecar_ipad;
bool disable_sidecar_mac;
bool disable_nightshift;

bool os_supports_nightshift_old;
bool os_supports_nightshift_new;
bool os_supports_sidecar;
bool os_supports_airplay_to_mac;

bool model_is_iMac;
bool model_is_iMac_2012;
bool model_is_iMac_2013;
bool model_is_iMac_2014;
bool model_is_iMac_2015_17;
bool model_is_MacBook;
bool model_is_MacBookAir;
bool model_is_MacBookPro;
bool model_is_MacBookPro_2016_2017;
bool model_is_Macmini;
bool model_is_MacPro;

int number_of_loops = 0;
int total_allowed_loops = 0;

#pragma mark - Kernel patching code

template <size_t findSize, size_t replaceSize>
static inline void searchAndPatch(const void *haystack, size_t haystackSize, const char *path, const uint8_t (&needle)[findSize], const uint8_t (&patch)[replaceSize], const char *name) {
    if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(haystack), haystackSize, needle, findSize, patch, replaceSize))) {
        DBGLOG(MODULE_SHORT, "found function %s to patch at %s!", name, path);
        number_of_loops++;
        DBGLOG(MODULE_SHORT, "number of loops: %d", number_of_loops);
    }
}

#pragma mark - Patched functions

// pre Big Sur
static boolean_t patched_cs_validate_range(vnode_t vp, memory_object_t pager, memory_object_offset_t offset, const void *data, vm_size_t size, unsigned *result) {
    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    boolean_t res = FunctionCast(patched_cs_validate_range, orig_cs_validate)(vp, pager, offset, data, size, result);
    if (res && vn_getpath(vp, path, &pathlen) == 0 && UserPatcher::matchSharedCachePath(path)) {
        if (number_of_loops < total_allowed_loops) {
            if (!disable_sidecar_mac && os_supports_sidecar) {
                if (model_is_MacBookPro) {
                    searchAndPatch(data, size, path, kSideCarAirPlayMacBookProOriginal, kSideCarAirPlayMacBookProPatched, "Sidecar (MacBookPro)");
                } else if (model_is_MacBookAir || model_is_MacBook) {
                    searchAndPatch(data, size, path, kSideCarAirPlayMacBookOriginal, kSideCarAirPlayMacBookPatched, "Sidecar (MacBook/MacBookAir)");
                } else if (model_is_iMac){
                    searchAndPatch(data, size, path, kSideCarAirPlayiMacOriginal, kSideCarAirPlayiMacPatched, "Sidecar (iMac)");
                } else if (model_is_Macmini || model_is_MacPro) {
                    searchAndPatch(data, size, path, kSideCarAirPlayStandaloneDesktopOriginal, kSideCarAirPlayStandaloneDesktopPatched, "Sidecar (Macmini/MacPro)");
                }
            }
            if (allow_sidecar_ipad) {
                searchAndPatch(data, size, path, kSidecariPadModelOriginal, kSidecariPadModelPatched, "Sidecar (iPad)");
            }
            if (!disable_nightshift) {
                if (os_supports_nightshift_new) {
                    searchAndPatch(data, size, path, kNightShiftOriginal, kNightShiftPatched, "NightShift");
                } else if (os_supports_nightshift_old) {
                    searchAndPatch(data, size, path, kNightShiftLegacyOriginal, kNightShiftLegacyPatched, "NightShift Legacy");
                }
            }
        }
    }
    return res;
}

// For Big Sur and newer
static void patched_cs_validate_page(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void *data, int *validated_p, int *tainted_p, int *nx_p) {
    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    FunctionCast(patched_cs_validate_page, orig_cs_validate)(vp, pager, page_offset, data, validated_p, tainted_p, nx_p);
	if (vn_getpath(vp, path, &pathlen) == 0) {
        // dyld cache patching
        if (UserPatcher::matchSharedCachePath(path)) {
            if (number_of_loops < total_allowed_loops) {
                // We check the number of times we've patched to limit wasted loops
                // Because AirPlay/Sidecar patches are long in length, repetitively calling the function is expensive
                // Since we know how many times these strings will appear in the dyld, we can effectively avoid extra checks
                // This resolves issues of system stability when a system is slower/low on memory as well as improve battery life
                if (!disable_sidecar_mac && os_supports_sidecar) {
                    if (model_is_MacBookPro) {
                        searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookProOriginal, kSideCarAirPlayMacBookProPatched, "Sidecar/AirPlay/UniversalControl (MacBookPro)");
                    } else if (model_is_MacBookAir || model_is_MacBook) {
                        searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookOriginal, kSideCarAirPlayMacBookPatched, "Sidecar/AirPlay/UniversalControl (MacBook/MacBookAir)");
                    } else if (model_is_iMac) {
                        if (model_is_iMac_2012) {
                            searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacAlternative2012Original, kSideCarAirPlayiMacAlternative2012Patched, "Sidecar/AirPlay/UniversalControl (iMac 2012)");
                        } else if (model_is_iMac_2013) {
                            searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacAlternative2013Original, kSideCarAirPlayiMacAlternative2013Patched, "Sidecar/AirPlay/UniversalControl (iMac 2013)");
                        } else if (model_is_iMac_2014) {
                            searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacAlternative2014Original, kSideCarAirPlayiMacAlternative2014Patched, "Sidecar/AirPlay/UniversalControl (iMac 2014)");
                        }
                    } else if (model_is_Macmini || model_is_MacPro) {
                        searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayStandaloneDesktopOriginal, kSideCarAirPlayStandaloneDesktopPatched, "Sidecar/AirPlay/UniversalControl (Macmini/MacPro)");
                    }
                    if (os_supports_airplay_to_mac && (model_is_MacBookPro_2016_2017 || model_is_iMac_2015_17)) {
                        searchAndPatch(data, PAGE_SIZE, path, kMacModelAirplayExtendedOriginal, kMacModelAirplayExtendedPatched, "AirPlay to Mac (Extended)");
                    }
                }
                if (allow_sidecar_ipad && os_supports_sidecar) {
                    searchAndPatch(data, PAGE_SIZE, path, kSidecariPadModelOriginal, kSidecariPadModelPatched, "Sidecar/UniversalControl (iPad)");
                }
                if (!disable_nightshift && (os_supports_nightshift_new || os_supports_nightshift_old)) {
                    searchAndPatch(data, PAGE_SIZE, path, kNightShiftOriginal, kNightShiftPatched, "NightShift");
                }
            }
        }
    }
}

#pragma mark - Detect Model

static void detectModel() {
    auto deviceInfo = BaseDeviceInfo::get();
    if (strstr(deviceInfo.modelIdentifier, "Book", sizeof("Book")-1)) {
        if (strstr(deviceInfo.modelIdentifier, "Pro", sizeof("Pro")-1)) {
            DBGLOG(MODULE_SHORT, "Detected MacBookPro");
            model_is_MacBookPro = true;
            if (strncmp(deviceInfo.modelIdentifier, "MacBookPro13", sizeof("MacBookPro13")-1) || strncmp(deviceInfo.modelIdentifier, "MacBookPro14", sizeof("MacBookPro14")-1)) {
                DBGLOG(MODULE_SHORT, "Detected MacBookPro13,x or MacBookPro14,x");
                model_is_MacBookPro_2016_2017 = true;
            }
        } else if (strstr(deviceInfo.modelIdentifier, "Air", sizeof("Air")-1)) {
            DBGLOG(MODULE_SHORT, "Detected MacBookAir");
            model_is_MacBookAir = true;
        } else {
            DBGLOG(MODULE_SHORT, "Detected MacBook");
            model_is_MacBook = true;
        }
    } else if (strstr(deviceInfo.modelIdentifier, "iMac", sizeof("iMac")-1)) {
        DBGLOG(MODULE_SHORT, "Detected iMac");
        model_is_iMac = true;
        if (strncmp(deviceInfo.modelIdentifier, "iMac13", sizeof("iMac13")-1) == 0) {
            DBGLOG(MODULE_SHORT, "Detected iMac13,x");
            model_is_iMac_2012 = true;
        } else if (strncmp(deviceInfo.modelIdentifier, "iMac14", sizeof("iMac14")-1) == 0) {
            DBGLOG(MODULE_SHORT, "Detected iMac14,x");
            model_is_iMac_2013 = true;
        } else if (strncmp(deviceInfo.modelIdentifier, "iMac15", sizeof("iMac15")-1) == 0 or strncmp(deviceInfo.modelIdentifier, "iMac16", sizeof("iMac16")-1) == 0) {
            DBGLOG(MODULE_SHORT, "Detected iMac15,x or iMac16,x");
            model_is_iMac_2014 = true;
        } else if (strncmp(deviceInfo.modelIdentifier, "iMac17", sizeof("iMac17")-1) == 0 or strncmp(deviceInfo.modelIdentifier, "iMac18", sizeof("iMac18")-1) == 0) {
            DBGLOG(MODULE_SHORT, "Detected iMac17,x or iMac18,x");
            model_is_iMac_2015_17 = true;
        }
    } else {
        if (strncmp(deviceInfo.modelIdentifier, "Macmini", sizeof("Macmini")-1)) {
            DBGLOG(MODULE_SHORT, "Detected Macmini");
            model_is_Macmini = true;
        } else if (strncmp(deviceInfo.modelIdentifier, "MacPro", sizeof("MacPro")-1)) {
            DBGLOG(MODULE_SHORT, "Detected MacPro");
            model_is_MacPro = true;
        }
    }
}

#pragma mark - Detect Supported Patch sets

static void detectSupportedPatchSets() {
    if ((getKernelVersion() == KernelVersion::HighSierra && getKernelMinorVersion() >= 2) || getKernelVersion() >= KernelVersion::Mojave) {
        DBGLOG(MODULE_SHORT, "Detected Modern NightShift Support");
        os_supports_nightshift_new = true;
    } else if (getKernelVersion() == KernelVersion::Sierra && getKernelMinorVersion() >= 5) {
        DBGLOG(MODULE_SHORT, "Detected Legacy NightShift Support");
        os_supports_nightshift_old = true;
    }
    if (getKernelVersion() >= KernelVersion::Catalina) {
        DBGLOG(MODULE_SHORT, "Detected Sidecar support");
        os_supports_sidecar = true;
    }
    if (getKernelVersion() >= KernelVersion::Monterey) {
        DBGLOG(MODULE_SHORT, "Detected AirPlay to Mac support");
        os_supports_airplay_to_mac = true;
    }
}

static void detectNumberOfPatches() {
    if ((os_supports_nightshift_new || os_supports_nightshift_old) && !disable_nightshift) {
        total_allowed_loops++;
    }
    if (os_supports_sidecar) {
        if (!disable_sidecar_mac) {
            total_allowed_loops++;
            if (os_supports_airplay_to_mac) {
                total_allowed_loops++;
            }
        }
        if (allow_sidecar_ipad) {
            total_allowed_loops++;
        }
    }
    if (os_supports_airplay_to_mac && !disable_sidecar_mac) {
        total_allowed_loops++;
    }
    DBGLOG(MODULE_SHORT, "Total allowed loops: %d", total_allowed_loops);
}

#pragma mark - Patches on start/stop

static void pluginStart() {
	DBGLOG(MODULE_SHORT, "start");
    allow_sidecar_ipad = checkKernelArgument("-allow_sidecar_ipad");
    disable_sidecar_mac = checkKernelArgument("-disable_sidecar_mac");
    disable_nightshift = checkKernelArgument("-disable_nightshift");
    detectModel();
    detectSupportedPatchSets();
    detectNumberOfPatches();
	lilu.onPatcherLoadForce([](void *user, KernelPatcher &patcher) {
		KernelPatcher::RouteRequest csRoute =
			getKernelVersion() >= KernelVersion::BigSur ?
			KernelPatcher::RouteRequest("_cs_validate_page", patched_cs_validate_page, orig_cs_validate) :
			KernelPatcher::RouteRequest("_cs_validate_range", patched_cs_validate_range, orig_cs_validate);
		if (!patcher.routeMultipleLong(KernelPatcher::KernelID, &csRoute, 1))
			SYSLOG(MODULE_SHORT, "failed to route cs validation pages");
	});
}

// Boot args.
static const char *bootargOff[] {
    "-caroff"
};
static const char *bootargDebug[] {
    "-cardbg"
};
static const char *bootargBeta[] {
    "-carbeta"
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
    KernelVersion::Sierra,
    KernelVersion::Monterey,
    pluginStart
};
