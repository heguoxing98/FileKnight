#ifndef _MODULE_MANAGER_H_
#define _MODULE_MANAGER_H_

#include "Common.h"
#include <vector>


namespace module_manager {
class NtDll
{
public:
	NtDll();
	~NtDll();

    NTSTATUS NtQuerySystemInformation(ULONG, PVOID, ULONG, PULONG);
    NTSTATUS NtDuplicateObject(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, ULONG, ULONG);
    NTSTATUS NtQueryObject(HANDLE, ULONG, PVOID, ULONG, PULONG);

private:
    HMODULE                            moduleBase_;
    common::fnNtQuerySystemInformation _NtQuerySystemInformation_;
    common::fnNtQueryObject            _NtQueryObject_;
    common::fnNtDuplicateObject        _NtDuplicateObject_;

};

class NtDllExt : public NtDll
{
//public:
//    static NtDllExt* GetIns()
//    {
//        if (ntDllExt_ == nullptr)
//            ntDllExt_ = new NtDllExt();
//        return ntDllExt_;
//    }
//
//    static void DelIns()
//    {
//        if (ntDllExt_ != nullptr)
//            delete ntDllExt_;
//        ntDllExt_ = nullptr;
//    }
private:
    static NtDllExt* ntDllExt_;

public:
    NtDllExt();
    ~NtDllExt();

    std::vector<common::HandleInfo> GetAllFileHandles();
    common::SystemInfoData NtQuerySystemInformationLoop(ULONG type);
    std::wstring FileHandleToKernelName(HANDLE fileHandle, std::vector<BYTE>& buffer);
};

}




#endif // !_MODULE_MANAGER_H_

