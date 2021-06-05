#pragma once
#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_FAMILY_DESKTOP_APP)
class CIntelCpuCount
{
public:
    CIntelCpuCount() {};
    virtual ~CIntelCpuCount() {};

    bool Count(unsigned int &TotAvailLogical, unsigned int &TotAvailCore, unsigned int &PhysicalNum);

private:
    unsigned int isHwMTSupported();
    unsigned int maxLogicalProcPerPhysicalProc();
    unsigned int maxCorePerPhysicalProc();
    unsigned char getAPIC_ID();
    unsigned int findmaskwidth(unsigned int count);
    unsigned char getCoreID(unsigned char apicID, unsigned char maxCorePerPhysicalProc, unsigned char maxLogPerCore);
    unsigned char getPhyID(unsigned char apicID, unsigned char maxLogicalProcPerPhysicalProc);
};
#endif
class CpuCount
{
public:
    CpuCount();
    virtual ~CpuCount();

    bool HasSSE2();
    bool HasSSSE3();
    bool HasSSE41();
    int GetPhysicalCoreNumber();
    int GetSuggestedThreadNumber();
    bool detect();

private:
    bool m_has_sse2;
    bool m_has_ssse3;
    bool m_has_sse41;
    int m_nAvailCoreNum;
    int m_nAvailLogicalProcNum;
#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_FAMILY_DESKTOP_APP)
    CIntelCpuCount m_IntelCpuCount;
#endif
};
