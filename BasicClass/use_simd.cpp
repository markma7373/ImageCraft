#include "stdafx.h"
#include "use_simd.h"

#if defined(ANDROID_X86) || defined(ANDROID_ARM)
#include <cpu-features.h>
#elif defined (IOS_ARM) || defined (MAC_OSX)
#include <sys/sysctl.h>
#elif defined(LINUX_SERVER)
#include <cpuid.h>
#include <x86intrin.h>
#else
#include <intrin.h>
#endif

bool g_is_support_SSE2 = false;
bool g_is_support_SSSE3 = false;
bool g_is_support_SSE41 = false;
bool g_is_support_NEON = false;
bool g_is_houdini = false;

bool DetectSIMD()
{
    g_is_support_SSE2 = false;
    g_is_support_SSSE3 = false;
    g_is_support_SSE41 = false;
    g_is_support_NEON = false;

#ifdef ANDROID_X86
    uint64_t features = android_getCpuFeatures();
    if (features & ANDROID_CPU_X86_FEATURE_SSSE3)
    {
        g_is_support_SSE2 = true;
        g_is_support_SSSE3 = false;
    }
    else
    {
        g_is_support_SSE2 = false;
        g_is_support_SSSE3 = false;
    }
    g_is_support_SSE41 = false;
#elif defined(ANDROID_ARM)
    uint64_t features = android_getCpuFeatures();

    if ((features & ANDROID_CPU_ARM_FEATURE_ARMv7) != 0)
    {
        if ((features & ANDROID_CPU_ARM_FEATURE_NEON) != 0)
            g_is_support_NEON = true;
    }
#elif defined(IOS_ARM)

    // iOS devices support NEON since armv7 (iPhone 3GS)
    // No stable method to detect iOS NEON. It's better to set to true always.
    g_is_support_NEON = true;

#elif defined(LINUX_SERVER)
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    g_is_support_SSE2  = (edx & bit_SSE2)   ? true: false;
    g_is_support_SSSE3 = (ecx & bit_SSSE3)  ? true: false;
    g_is_support_SSE41 = (ecx & bit_SSE4_1) ? true: false;
#else
    int cpu_info[4] = {-1};
    __cpuid(cpu_info, 1);
    g_is_support_SSE2  = (cpu_info[3] & (1 << 26)) ? true: false;
    g_is_support_SSSE3 = (cpu_info[2] & (1 <<  9)) ? true: false;
    g_is_support_SSE41 = (cpu_info[2] & (1 << 19)) ? true: false;
#endif

    return true;
}

bool SIMD_dummy = DetectSIMD();

