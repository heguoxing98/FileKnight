#include "ModuleManager.h"

#include <winternl.h>
#include <map>

#include"StringManager.h"

constexpr static size_t _defaultResultBufferSize = 64 * 1024;
constexpr static size_t _maxResultBufferSize = 1024 * 1024 * 1024;


/*************************************************************
 * NtDll
 *************************************************************/

module_manager::NtDll::~NtDll()
{
}

module_manager::NtDll::NtDll()
{
    moduleBase_ = GetModuleHandleW(L"ntdll.dll");
    if (moduleBase_ == 0)
    {
        throw std::runtime_error{ "GetModuleHandleW returned null" };
    }

    _NtQuerySystemInformation_ = (common::fnNtQuerySystemInformation)GetProcAddress(moduleBase_, "NtQuerySystemInformation");
    if (_NtQuerySystemInformation_ == 0)
    {
        throw std::runtime_error{ "GetProcAddress returned null for NtQuerySystemInformation" };
    }

    _NtDuplicateObject_ = (common::fnNtDuplicateObject)GetProcAddress(moduleBase_, "NtDuplicateObject");
    if (_NtDuplicateObject_ == 0)
    {
        throw std::runtime_error{ "GetProcAddress returned null for NtDuplicateObject" };
    }

    _NtQueryObject_ = (common::fnNtQueryObject)GetProcAddress(moduleBase_, "NtQueryObject");
    if (_NtQueryObject_ == 0)
    {
        throw std::runtime_error{ "GetProcAddress returned null for NtQueryObject" };
    }
}

NTSTATUS 
module_manager::NtDll::NtQuerySystemInformation(ULONG systemInformationClass,
    PVOID systemInformation, ULONG systemInformationLength, PULONG returnLength)
{
    return _NtQuerySystemInformation_(systemInformationClass, systemInformation, systemInformationLength, returnLength);
}

NTSTATUS 
module_manager::NtDll::NtDuplicateObject(HANDLE sourceProcessHandle,
    HANDLE sourceHandle, HANDLE targetProcessHandle, PHANDLE targetHandle, 
    ACCESS_MASK desiredAccess, ULONG attributes, ULONG options)
{
    return _NtDuplicateObject_(sourceProcessHandle, sourceHandle, targetProcessHandle, targetHandle, desiredAccess, attributes, options);
}

NTSTATUS 
module_manager::NtDll::NtQueryObject(HANDLE objectHandle,
    ULONG objectInformationClass, PVOID objectInformation, ULONG objectInformationLength, PULONG returnLength)
{
    return _NtQueryObject_(objectHandle, objectInformationClass, objectInformation, objectInformationLength, returnLength);
}

/*************************************************************
 * NtDllExt 
 *************************************************************/

module_manager::NtDllExt::NtDllExt()
{
}

module_manager::NtDllExt::~NtDllExt()
{
}

std::vector<common::HandleInfo> 
module_manager::NtDllExt::GetAllFileHandles()
{
    std::vector<common::HandleInfo> handleInfoVec;
    
    auto data = NtQuerySystemInformationLoop((ULONG)common::SystemInformationClass::SystemHandleInformation);
    if (NT_ERROR(data.status))
        return {};

    auto infoPtr = (common::SystemHandleInformation*)data.memory.data();
    std::map<DWORD, HANDLE> handleByPid;
    std::vector<BYTE> objInfo(_defaultResultBufferSize);

    for (ULONG i = 0; i < infoPtr->numberOfHandles; i++)
    {
        auto handleInfo = (common::SystemHandleTableEntryInfo)infoPtr->handles[i];
        
        if (handleInfo.grantedAccess == 0x0012019f) 
            continue;

        DWORD pid = handleInfo.uniqueProcessID;

        HANDLE processHandle = NULL;
        auto iter = handleByPid.find(pid);
        if (iter != handleByPid.end())
        {
            processHandle = iter->second;
        }
        else
        {
            processHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
            if (!processHandle)
            {
                continue;
            }
            handleByPid[pid] = processHandle;
        }


        HANDLE dupHandle = NULL;
        bool isOk = DuplicateHandle(processHandle, (HANDLE)handleInfo.handleValue, GetCurrentProcess(), &dupHandle, 0, 0, DUPLICATE_SAME_ACCESS);
        if (false == isOk)
        {
            continue;
        }

        ULONG outLen;
        auto status = NtQueryObject(dupHandle, ObjectTypeInformation, objInfo.data(), (ULONG)objInfo.size(), &outLen);
        if (NT_ERROR(status))
        {
            CloseHandle(dupHandle);
            continue;
        }

        std::wstring fileName;
        auto objTypeInfo = (common::ObjectTypeInformationPtr)objInfo.data();
        auto typeName = common::StringManager::UnicodeToWs(objTypeInfo->typeName);
        if (typeName == L"File")
        {
            fileName = FileHandleToKernelName(dupHandle, objInfo);
        }
        if(!fileName.empty())
            handleInfoVec.push_back(common::HandleInfo{ pid, handleInfo.handleValue, fileName });

        CloseHandle(dupHandle);
    }

    for (auto iter: handleByPid)
    {
        CloseHandle(iter.second);
    }

    return handleInfoVec;

}

common::SystemInfoData
module_manager::NtDllExt::NtQuerySystemInformationLoop(ULONG type)
{
    common::SystemInfoData data;
    data.memory.resize(_defaultResultBufferSize);

    while (data.memory.size() <= _maxResultBufferSize)
    {
        ULONG outLen;
        data.status = NtQuerySystemInformation(type, data.memory.data(), (ULONG)data.memory.size(), &outLen);

        if (data.status == STATUS_INFO_LENGTH_MISMATCH)
        {
            data.memory.resize(data.memory.size() * 2);
            continue;
        }

        if (NT_ERROR(data.status))
            data.memory.clear();

        return data;
    }

    data.status = STATUS_INFO_LENGTH_MISMATCH;
    data.memory.clear();
    return data;
}

std::wstring 
module_manager::NtDllExt::FileHandleToKernelName(HANDLE fileHandle, std::vector<BYTE>& buffer)
{
    if (GetFileType(fileHandle) != FILE_TYPE_DISK)
    {
        return L"";
    }

    ULONG outLen;
    auto status = NtQueryObject(fileHandle, (ULONG)common::ObjectInformationClass::ObjectNameInformation, buffer.data(), (ULONG)buffer.size(), &outLen);
    if (NT_SUCCESS(status))
    {
        auto object_name_info = (common::UnicodePtr)buffer.data();
        return common::StringManager::UnicodeToWs(*object_name_info);
    }

    return L"";
}