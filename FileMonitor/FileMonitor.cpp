#include "FileMonitor.h"

FileMonitor::FileMonitor()
{
}

FileMonitor::~FileMonitor()
{
}

HANDLE FileMonitor::GetLogicDiskObject(const wchar_t diskSymbol)
{
    CStringW logicDisk(L"\\\\.\\c:");
    logicDisk.SetAt(4, L'\0');

     return CreateFileW(logicDisk, GENERIC_READ | GENERIC_WRITE,
         FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);

}

bool FileMonitor::CreateUSN(HANDLE logicDiskObject)
{
    CREATE_USN_JOURNAL_DATA CUJD = { 0,0 };
    DWORD returnBytes = 0;

    BOOL isSuccess = DeviceIoControl(logicDiskObject,
        FSCTL_CREATE_USN_JOURNAL, &CUJD, sizeof(CUJD), nullptr, 0, &returnBytes, nullptr);

    if (FALSE == isSuccess)
        return false;

    return true;
}
