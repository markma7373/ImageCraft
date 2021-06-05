#include "stdafx.h"
#include "CpuCount.h"

#ifndef ARM_SUPPORT
extern "C" {
    void __cpuid(int CPUInfo[4], int InfoType);
}
#endif

#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_FAMILY_DESKTOP_APP)
//return 0 if cpuid instruction is unavailable
//return 0 if cpuid instruction is unavailable
static unsigned int isCpuidSupported()
{
    unsigned int maxInputValue;
    __try
    {
        maxInputValue = 0;
        _asm
        {
            xor eax, eax
            cpuid
            mov maxInputValue, eax
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return 0;  // cpuid instruction is unavailable
    }

    return maxInputValue;
}

//return 0 if the processor is not a Genuine Intel Processor
static unsigned int isGenuineIntelProcessor()
{
    if (!isCpuidSupported())
        return 0;  // cpuid instruction is unavailable

    unsigned int VendorID[3] = {
        0, 0, 0
    };
    _asm
    {
        xor eax, eax
        cpuid                   // call cpuid with eax = 0 to get vendor id string
        mov VendorID, ebx
        mov VendorID + 4, edx
        mov VendorID + 8, ecx
    }

    return ( (VendorID[0] == 0x756E6547) &&   //'uneG'
            (VendorID[1] == 0x49656E69) &&    //'Ieni'
            (VendorID[2] == 0x6C65746E));     //'letn'
}
#endif
///////////////////////////////////////////////////////
CpuCount::CpuCount()
{
    m_has_sse2 = false;
    m_has_ssse3 = false;
    m_has_sse41 = false;
    m_nAvailCoreNum = 0;
    m_nAvailLogicalProcNum = 0;
}

CpuCount::~CpuCount()
{}

bool CpuCount::HasSSE2()
{
    return m_has_sse2;
}

bool CpuCount::HasSSSE3()
{
    return m_has_ssse3;
}

bool CpuCount::HasSSE41()
{
    return m_has_sse41;
}

int CpuCount::GetSuggestedThreadNumber()
{
    // balance the performance and memory usage
    int thread_number = 1;

    if (m_nAvailLogicalProcNum <= 2)
        thread_number = m_nAvailLogicalProcNum;
    else
        thread_number = m_nAvailCoreNum;

    //workaround for ebug DVD103324-0020,
    //the m_nAvailCoreNum we calculate in this sample machine is wrong.
    //We get m_nAvailCoreNum = 1, but it is 2 actually. But we get m_nAvailLogicalProcNum = 4 is right.
    thread_number = max(thread_number, m_nAvailLogicalProcNum / 2);

    return min(12, thread_number);
}

int CpuCount::GetPhysicalCoreNumber()
{
    //workaround for ebug DVD103324-0020,
    //the m_nAvailCoreNum we calculate in this sample machine is wrong. 
    return max(m_nAvailCoreNum, m_nAvailLogicalProcNum / 2);
}

bool CpuCount::detect()
{
    SYSTEM_INFO system_info;

	memset(&system_info, 0, sizeof(SYSTEM_INFO)); 

    GetNativeSystemInfo(&system_info);

    // get available logical processor number
    m_nAvailLogicalProcNum = min(12, max(1, system_info.dwNumberOfProcessors));
#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_FAMILY_DESKTOP_APP)
    // get available core number
    if (isGenuineIntelProcessor())
    {
        unsigned int tolAvailLogi, tolAvailCore, phyNum;
        bool ret = m_IntelCpuCount.Count(tolAvailLogi, tolAvailCore, phyNum);
        m_nAvailCoreNum = ret ? tolAvailCore : info.dwNumberOfProcessors;
    }
    else
    {
        m_nAvailCoreNum = info.dwNumberOfProcessors;
    }
#elif WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_FAMILY_APP)
    m_nAvailCoreNum = min(12, max(1, system_info.dwNumberOfProcessors));
#endif

#ifndef ARM_SUPPORT
    int CPUInfo[4] = { -1 };
    __cpuid(CPUInfo, 1);
    m_has_sse2 = (CPUInfo[3] & (1 << 26)) ? true : false;
    m_has_ssse3 = (CPUInfo[2] & (1 << 9)) ? true : false;
    m_has_sse41 = (CPUInfo[2] & (1 << 19)) ? true : false;
#endif

    return true;
}

#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_FAMILY_DESKTOP_APP)
///////////////////////////////////////////////////////

#define HW_MT_BIT         0x10000000      // EDX[28]  Bit 28 is set if HT or multi-core is supported
#define NUM_LOGICAL_BITS  0x00FF0000      // EBX[23:16] Bit 16-23 in ebx contains the number of logical
                                          // processors per physical processor when execute cpuid with
                                          // eax set to 1
#define NUM_CORE_BITS  0xFC000000         // EAX[31:26] Bit 26-31 in eax contains the number of cores minus one
                                          // per physical processor when execute cpuid with
                                          // eax set to 4.
#define INITIAL_APIC_ID_BITS  0xFF000000  // EBX[31:24] Bits 24-31 (8 bits) return the 8-bit unique
                                          // initial APIC ID for the processor this code is running on.

bool CIntelCpuCount::Count(unsigned int &TotAvailLogical, unsigned int &TotAvailCore, unsigned int &PhysicalNum)
{
    TotAvailLogical = 0;
    TotAvailCore = 0;
    PhysicalNum = 0;

    if (!isGenuineIntelProcessor()) //not Genuine Intel Processor
        return false;

    DWORD_PTR dwProcessAffinity, dwSystemAffinity;
    GetProcessAffinityMask(GetCurrentProcess(), &dwProcessAffinity, &dwSystemAffinity);
    if (dwProcessAffinity != dwSystemAffinity) // user config issue
        return false;

    // Assumwe that cores within a package have the SAME number of logical processors.
    int MaxLogProcPerCore = maxLogicalProcPerPhysicalProc() / maxCorePerPhysicalProc();

    DWORD dwAffinityMask = 1;
    unsigned int numLPEnabled = 0;
    unsigned char tblPkgID[256], tblCoreID[256];
    memset(tblPkgID, 0, 256);
    memset(tblCoreID, 0, 256);

    int j = 0;
    while (dwAffinityMask && dwAffinityMask <= dwSystemAffinity)
    {
        if (SetThreadAffinityMask(GetCurrentThread(), dwAffinityMask))
        {
            Sleep(0);  // Ensure system to switch to the right CPU

            unsigned char apicID = getAPIC_ID();

            // Store core ID of each logical processor
            tblCoreID[j] = getCoreID(apicID, (unsigned char)maxCorePerPhysicalProc(), (unsigned char)MaxLogProcPerCore);

            // Store physical processor ID of each logical processor, assume single cluster.
            tblPkgID[j] = getPhyID(apicID, (unsigned char)maxLogicalProcPerPhysicalProc());

            numLPEnabled++; //Number of available logical processors in the system.
        }

        j++;
        dwAffinityMask = 1 << j;

        // error handling for special machine which core number is over 32
        if (j > 31)
            break;
    }

    // restore the affinity setting to its original state
    SetThreadAffinityMask(GetCurrentThread(), dwProcessAffinity);
    Sleep(0);

    TotAvailLogical = numLPEnabled;

    //
    // Count available cores (TotAvailCore) in the system
    //
    TotAvailCore = 1;

    unsigned char CoreIDBucket[256];
    unsigned int i, logicalProcIdx;

    CoreIDBucket[0] = tblPkgID[0] | tblCoreID[0];
    for (logicalProcIdx = 1; logicalProcIdx < numLPEnabled; logicalProcIdx++)
    {
        for (i = 0; i < TotAvailCore; i++)
        {
            // Comparing bit-fields of logical processors residing in different packages
            if ((tblPkgID[logicalProcIdx] | tblCoreID[logicalProcIdx]) == CoreIDBucket[i])
                break;
        }

        if (i == TotAvailCore) // did not match any bucket.  Start a new one.
        {
            CoreIDBucket[i] = tblPkgID[logicalProcIdx] | tblCoreID[logicalProcIdx];
            TotAvailCore++;
        }
    }

    //
    // Count physical processor (PhysicalNum) in the system
    //
    PhysicalNum = 1;

    unsigned char PackageIDBucket[256];

    PackageIDBucket[0] = tblPkgID[0];
    for (logicalProcIdx = 1; logicalProcIdx < numLPEnabled; logicalProcIdx++)
    {
        for (i = 0; i < PhysicalNum; i++)
        {
            // Comparing bit-fields of logical processors residing in different packages
            if (tblPkgID[logicalProcIdx] == PackageIDBucket[i])
                break;
        }

        if (i == PhysicalNum)  // did not match any bucket.  Start a new one.
        {
            PackageIDBucket[i] = tblPkgID[logicalProcIdx];
            PhysicalNum++;
        }
    }

    //
    return true;
}

//return 0 if the hardware multi-threaded is not supported
unsigned int CIntelCpuCount::isHwMTSupported()
{
    unsigned int regedx = 0;
    _asm
    {
        mov eax, 1
        cpuid
        mov regedx, edx
    }

    return (regedx & HW_MT_BIT);
}

unsigned int CIntelCpuCount::maxLogicalProcPerPhysicalProc()
{
    unsigned int regebx = 0;

    if (!isHwMTSupported())
        return (unsigned int)1;

    _asm
    {
        mov eax, 1
        cpuid
        mov regebx, ebx
    }
    return (unsigned int)((regebx & NUM_LOGICAL_BITS) >> 16);
}

unsigned int CIntelCpuCount::maxCorePerPhysicalProc(void)
{
    unsigned int regeax = 0;

    if (!isHwMTSupported())
        return (unsigned int)1;

    _asm
    {
        xor eax, eax
        cpuid
        cmp eax, 4          // check if cpuid supports leaf 4
        jl single_core      // single core
        mov eax, 4
        mov ecx, 0          // start with index = 0; Leaf 4 reports at least one valid cache level
        cpuid
        mov regeax, eax
        jmp multi_core

single_core:
        xor eax, eax

multi_core:
    }

    return (unsigned int)((regeax & NUM_CORE_BITS) >> 26) + 1;
}

unsigned char CIntelCpuCount::getAPIC_ID()
{
    unsigned int regebx = 0;
    _asm
    {
        mov eax, 1
        cpuid
        mov regebx, ebx
    }

    return (unsigned char)((regebx & INITIAL_APIC_ID_BITS) >> 24);
}

unsigned char CIntelCpuCount::getCoreID(unsigned char apicID, unsigned char maxCorePerPhysicalProc, unsigned char maxLogPerCore)
{
    unsigned int MaskWidth = findmaskwidth(maxCorePerPhysicalProc);
    unsigned int ShiftCount = findmaskwidth(maxLogPerCore);
    unsigned char MaskBits = (unsigned char)( (0xff << ShiftCount) ^ ((unsigned char)(0xff << (ShiftCount + MaskWidth))) );

    return (apicID & MaskBits);
}

unsigned char CIntelCpuCount::getPhyID(unsigned char apicID, unsigned char maxLogicalProcPerPhysicalProc)
{
    unsigned int ShiftCount = findmaskwidth(maxLogicalProcPerPhysicalProc);
    unsigned char MaskBits = (unsigned char)(0xff << ShiftCount);

    return (apicID & MaskBits);
}

// Determine the width of the bit field that can represent the value count_item.
unsigned int CIntelCpuCount::findmaskwidth(unsigned int count)
{
    unsigned int MaskWidth;

    _asm
    {
        mov eax, count
        mov ecx, 0
        mov MaskWidth, ecx
        dec eax
        bsr cx, ax
        jz next
        inc cx
        mov MaskWidth, ecx
next:
    }

    return MaskWidth;
}

//
#undef HW_MT_BIT
#undef NUM_LOGICAL_BITS
#undef NUM_CORE_BITS
#undef INITIAL_APIC_ID_BITS

///////////////////////////////////////////////////////
#endif