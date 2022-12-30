#include <iostream>
#include <Windows.h>
#include "GodLoader.h"

// Ô­ÎÄÁ´½Ó£º
// https://www.cnblogs.com/sakura521/p/15260187.html

int main()
{
	const wchar_t fileName[] = L"TestDll.dll";
	
	HANDLE fileObj = CreateFile(fileName, 
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING,
		FILE_ATTRIBUTE_ARCHIVE, 
		NULL
	);

	if (INVALID_HANDLE_VALUE == fileObj)
		return 0;

	LARGE_INTEGER fileSize;
	if (0 == GetFileSizeEx(fileObj, &fileSize))
		return 0;

	byte* fileData = new byte[fileSize.u.LowPart];
	if (nullptr == fileData)
		return 0;

	if (FALSE == ReadFile(fileObj, fileData, fileSize.u.LowPart, nullptr, nullptr))
		return 0;

	LPVOID baseAddress = loader::LoadDll(fileData, fileSize.u.LowPart);
	if (NULL == baseAddress)
		return 0;


	bool ok = loader::FreeDll(baseAddress);
	if (false == ok)
	{
		
	}

	delete[] fileData;
	fileData = nullptr;
	CloseHandle(fileObj);

	system("pause");
	return 0;
}
