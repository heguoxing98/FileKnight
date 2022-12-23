#include <iostream>
#include <Windows.h>
#include "GodLoader.h"

// 原文链接：
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

	// 将内存DLL加载到程序中
	LPVOID lpBaseAddress = LoadDll(fileData, fileSize.u.LowPart);
	printf("2222");
	if (NULL == lpBaseAddress)
	{
		//ShowError("MmLoadLibrary");
		return 3;
	}
	printf("DLL加载成功\n");

	// 获取DLL导出函数并调用
	typedef BOOL(*typedef_ShowMessage)(char* lpszText, char* lpszCaption);
	typedef_ShowMessage ShowMessage = (typedef_ShowMessage)MmGetProcAddress(lpBaseAddress, (PCHAR)"ShowMessage");
	if (NULL == ShowMessage)
	{
		//ShowError("MmGetProcAddress");
		return 4;
	}
	//ShowMessage("I am Demon·Gan\n", "Who Are You");

	// 释放从内存加载的DLL
	BOOL bRet = MmFreeLibrary(lpBaseAddress);
	if (FALSE == bRet)
	{
		//ShowError("MmFreeLirbary");
	}

	// 释放
	delete[] fileData;
	fileData = NULL;
	CloseHandle(fileObj);

	system("pause");
	return 0;
}
