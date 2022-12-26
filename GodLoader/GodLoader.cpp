#include "GodLoader.h"
#include<windows.h>
#include<stdio.h>

LPVOID LoadDll(LPVOID data, unsigned long dataSize)
{
	LPVOID baseAddress = nullptr;
	
	DWORD sizeOfImage = GetSizeOfImage(data);

	baseAddress = VirtualAlloc(NULL, sizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (nullptr == baseAddress)
		return nullptr;
	
	RtlZeroMemory(baseAddress, sizeOfImage);

	if (false == MapDataToMemory(data, baseAddress))
		return nullptr;

	if (FALSE == FixRelocationTable(baseAddress))
		return nullptr;

	// 填写PE文件导入表信息
	if (FALSE == DoImportTable(baseAddress))
		return nullptr;

	//修改页属性。应该根据每个页的属性单独设置其对应内存页的属性。
	//统一设置成一个属性PAGE_EXECUTE_READWRITE
	DWORD dwOldProtect = 0;
	if (FALSE == ::VirtualProtect(baseAddress, sizeOfImage, PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
		//ShowError("VirtualProtect");
		return NULL;
	}

	// 修改PE文件加载基址IMAGE_NT_HEADERS.OptionalHeader.ImageBase
	if (FALSE == SetImageBase(baseAddress))
	{
		//ShowError("SetImageBase");
		return NULL;
	}

	// 调用DLL的入口函数DllMain,函数地址即为PE文件的入口点IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
	if (FALSE == CallDllMain(baseAddress))
	{
		//ShowError("CallDllMain");
		return NULL;
	}

	return baseAddress;
}

DWORD GetSizeOfImage(LPVOID data)
{
	DWORD sizeOfImage = 0;
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)data;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((ULONG32)dosHeader + dosHeader->e_lfanew);
	sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	return sizeOfImage;
}

bool MapDataToMemory(LPVOID data, LPVOID baseAddress)
{
	PIMAGE_DOS_HEADER dosHeaderPtr = (PIMAGE_DOS_HEADER)data;
	PIMAGE_NT_HEADERS ntHeadesPtr = (PIMAGE_NT_HEADERS)((ULONG32)dosHeaderPtr + dosHeaderPtr->e_lfanew);
	
	DWORD sizeOfHeaders = ntHeadesPtr->OptionalHeader.SizeOfHeaders;
	RtlCopyMemory(baseAddress, data, sizeOfHeaders);

	PIMAGE_SECTION_HEADER sectionHeaderPtr = (PIMAGE_SECTION_HEADER)((DWORD)ntHeadesPtr + sizeof(IMAGE_NT_HEADERS));
	LPVOID srcMem = nullptr;
	LPVOID destMem = nullptr;
	DWORD  sizeOfRawData = 0;
	for (WORD i = 0; i < ntHeadesPtr->FileHeader.NumberOfSections; i++)
	{
		if ((0 == sectionHeaderPtr->VirtualAddress) || (0 == sectionHeaderPtr->SizeOfRawData))
		{
			sectionHeaderPtr++;
			continue;
		}

#ifdef _WIN64
		srcMem = (LPVOID)((unsigned __int64)data + (unsigned __int64)(sectionHeaderPtr->PointerToRawData) /*FOA*/);
		destMem = (LPVOID)((unsigned __int64)baseAddress + (unsigned __int64)(sectionHeaderPtr->VirtualAddress)/*RVA*/);
#else
		srcMem = (LPVOID)((unsigned __int32)data + (unsigned __int32)(sectionHeaderPtr->PointerToRawData) /*FOA*/);
		destMem = (LPVOID)((unsigned __int32)baseAddress + (unsigned __int32)(sectionHeaderPtr->VirtualAddress)/*RVA*/);
#endif 

		sizeOfRawData = sectionHeaderPtr->SizeOfRawData;
		
		::RtlCopyMemory(destMem, srcMem, sizeOfRawData);
		
		sectionHeaderPtr++;
	}
	
	return true;
}

bool FixRelocationTable(LPVOID baseAddress)
{
	PIMAGE_DOS_HEADER dosHeaderPtr = (PIMAGE_DOS_HEADER)baseAddress;

#ifdef _WIN64
	PIMAGE_NT_HEADERS ntHeadersPtr = (PIMAGE_NT_HEADERS)((unsigned __int64)dosHeaderPtr + (unsigned __int64)dosHeaderPtr->e_lfanew);
	PIMAGE_BASE_RELOCATION relocationPtr = (PIMAGE_BASE_RELOCATION)((unsigned __int64)dosHeaderPtr + (unsigned __int64)ntHeadersPtr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
#else
	PIMAGE_NT_HEADERS ntHeadersPtr = (PIMAGE_NT_HEADERS)((unsigned __int32)dosHeaderPtr + (unsigned __int32)dosHeaderPtr->e_lfanew);
	PIMAGE_BASE_RELOCATION relocationPtr = (PIMAGE_BASE_RELOCATION)((unsigned __int32)dosHeaderPtr + (unsigned __int32)ntHeadersPtr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
#endif 


	if ((PVOID)relocationPtr == (PVOID)dosHeaderPtr)
		return true;

	while ((relocationPtr->VirtualAddress + relocationPtr->SizeOfBlock) != 0)
	{
		WORD* relocData = (WORD*)((PBYTE)relocationPtr + sizeof(IMAGE_BASE_RELOCATION));
		int numberOfReloc = (relocationPtr->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

		for (int i = 0; i < numberOfReloc; i++)
		{
			if ((DWORD)(relocData[i] & 0x0000F000) == 0x00003000) //这是一个需要修正的地址
			{
				// 32位dll重定位，IMAGE_REL_BASED_HIGHLOW
				// 对于x86的可执行文件，所有的基址重定位都是IMAGE_REL_BASED_HIGHLOW类型的。
				// 
				// 关于修复重定位表的理解，举个例子：
				// 前提PE文件默认加载位置是0x40000。假设在生成PE文件 B.dll 的时候有一个全局变量的 globalValue, globalValue 的地址是0x40123。
				// 当一个EXE加载 B.dll 时，加载位置为 0x5000。我们知道汇编在进行赋值给寄存器时，
				// 会有取地址运算` mov ebp [0x401234] `，那程序栈内就会有个地址（假设为0x51567，-
				// - 这个地址一定是mov ebp地址之后的一个地址，且一般是通过RVA的形式记录下来，随ImageBase变化而变化）存储 globalVlaue的地址 0x40123
				// 但是那么当执行这么一条指令` mov ebp [0x401234] `时，就会出错。所以需要将 0x40123 修正为 0x50123
				// 按照重定位表的规则进行一系列运算，最后得到的了一个地址，也就代码中的 address 是等于 0x51567。将address进行取地址运算就是我们需要修正地址
				// *address = (0x5000-0x4000) + *address(0x40123)

				DWORD* address = (DWORD*)((PBYTE)dosHeaderPtr + relocationPtr->VirtualAddress + (relocData[i] & 0x0FFF));
				DWORD  delta = (DWORD)dosHeaderPtr - ntHeadersPtr->OptionalHeader.ImageBase;
				*address += delta;

			}
		}
		relocationPtr = (PIMAGE_BASE_RELOCATION)((PBYTE)relocationPtr + relocationPtr->SizeOfBlock);
	}

	return true;
}

bool FixImportTable(LPVOID baseAddress)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG32)pDosHeader + pDosHeader->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pDosHeader +
		pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	// 循环遍历DLL导入表中的DLL及获取导入表中的函数地址
	char* lpDllName = NULL;
	HMODULE hDll = NULL;
	PIMAGE_THUNK_DATA lpImportNameArray = NULL;
	PIMAGE_IMPORT_BY_NAME lpImportByName = NULL;
	PIMAGE_THUNK_DATA lpImportFuncAddrArray = NULL;
	FARPROC lpFuncAddress = NULL;
	DWORD i = 0;

	while (TRUE)
	{
		if (0 == pImportTable->OriginalFirstThunk)
		{
			break;
		}

		// 获取导入表中DLL的名称并加载DLL
		lpDllName = (char*)((DWORD)pDosHeader + pImportTable->Name);
		hDll = ::GetModuleHandleA(lpDllName);
		if (NULL == hDll)
		{
			hDll = ::LoadLibraryA(lpDllName);
			if (NULL == hDll)
			{
				pImportTable++;
				continue;
			}
		}

		i = 0;
		// 获取OriginalFirstThunk以及对应的导入函数名称表首地址
		lpImportNameArray = (PIMAGE_THUNK_DATA)((DWORD)pDosHeader + pImportTable->OriginalFirstThunk);
		// 获取FirstThunk以及对应的导入函数地址表首地址
		lpImportFuncAddrArray = (PIMAGE_THUNK_DATA)((DWORD)pDosHeader + pImportTable->FirstThunk);
		while (TRUE)
		{
			if (0 == lpImportNameArray[i].u1.AddressOfData)
			{
				break;
			}

			// 获取IMAGE_IMPORT_BY_NAME结构
			lpImportByName = (PIMAGE_IMPORT_BY_NAME)((DWORD)pDosHeader + lpImportNameArray[i].u1.AddressOfData);

			// 判断导出函数是序号导出还是函数名称导出
			if (0x80000000 & lpImportNameArray[i].u1.Ordinal)
			{
				// 序号导出
				// 当IMAGE_THUNK_DATA值的最高位为1时，表示函数以序号方式输入，这时，低位被看做是一个函数序号
				lpFuncAddress = ::GetProcAddress(hDll, (LPCSTR)(lpImportNameArray[i].u1.Ordinal & 0x0000FFFF));
			}
			else
			{
				// 名称导出
				lpFuncAddress = ::GetProcAddress(hDll, (LPCSTR)lpImportByName->Name);
			}
			// 注意此处的函数地址表的赋值，要对照PE格式进行装载，不要理解错了！！！
			lpImportFuncAddrArray[i].u1.Function = (DWORD)lpFuncAddress;
			i++;
		}

		pImportTable++;
	}

	return TRUE;
}



// 对齐SectionAlignment
// dwSize: 表示未对齐前内存的大小
// dwAlignment: 对齐大小值
// 返回值: 返回内存对齐之后的值
DWORD Align(DWORD dwSize, DWORD dwAlignment)
{
	DWORD dwRet = 0;
	DWORD i = 0, j = 0;
	i = dwSize / dwAlignment;
	j = dwSize % dwAlignment;
	if (0 != j)
	{
		i++;
	}
	dwRet = i * dwAlignment;
	return dwRet;
}

// 修改PE文件加载基址IMAGE_NT_HEADERS.OptionalHeader.ImageBase
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL SetImageBase(LPVOID lpBaseAddress)
{
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG32)pDosHeader + pDosHeader->e_lfanew);
	pNtHeaders->OptionalHeader.ImageBase = (ULONG32)lpBaseAddress;

	return TRUE;
}


// 调用DLL的入口函数DllMain,函数地址即为PE文件的入口点IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL CallDllMain(LPVOID lpBaseAddress)
{
	typedef_DllMain DllMain = NULL;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG32)pDosHeader + pDosHeader->e_lfanew);
	DllMain = (typedef_DllMain)((ULONG32)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);
	// 调用入口函数,附加进程DLL_PROCESS_ATTACH
	BOOL bRet = DllMain((HINSTANCE)lpBaseAddress, DLL_PROCESS_ATTACH, NULL);
	if (FALSE == bRet)
	{
		//ShowError("DllMain");
	}

	return bRet;
}


// 模拟GetProcAddress获取内存DLL的导出函数
// lpBaseAddress: 内存DLL文件加载到进程中的加载基址
// lpszFuncName: 导出函数的名字
// 返回值: 返回导出函数的的地址
LPVOID MmGetProcAddress(LPVOID lpBaseAddress, PCHAR lpszFuncName)
{
	LPVOID lpFunc = NULL;
	// 获取导出表
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG32)pDosHeader + pDosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	// 获取导出表的数据
	PDWORD lpAddressOfNamesArray = (PDWORD)((DWORD)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpFuncName = NULL;
	PWORD lpAddressOfNameOrdinalsArray = (PWORD)((DWORD)pDosHeader + pExportTable->AddressOfNameOrdinals);
	WORD wHint = 0;
	PDWORD lpAddressOfFunctionsArray = (PDWORD)((DWORD)pDosHeader + pExportTable->AddressOfFunctions);

	DWORD dwNumberOfNames = pExportTable->NumberOfNames;
	DWORD i = 0;
	// 遍历导出表的导出函数的名称, 并进行匹配
	for (i = 0; i < dwNumberOfNames; i++)
	{
		lpFuncName = (PCHAR)((DWORD)pDosHeader + lpAddressOfNamesArray[i]);
		if (0 == ::lstrcmpiA(lpFuncName, lpszFuncName))
		{
			// 获取导出函数地址
			wHint = lpAddressOfNameOrdinalsArray[i];
			lpFunc = (LPVOID)((DWORD)pDosHeader + lpAddressOfFunctionsArray[wHint]);
			break;
		}
	}
	printf("11111112222222333333333");
	return lpFunc;
}


// 释放从内存加载的DLL到进程内存的空间
// lpBaseAddress: 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
// 返回值: 成功返回TRUE，否则返回FALSE
BOOL MmFreeLibrary(LPVOID lpBaseAddress)
{
	BOOL bRet = FALSE;

	if (NULL == lpBaseAddress)
	{
		return bRet;
	}

	bRet = VirtualFree(lpBaseAddress, 0, MEM_RELEASE);
	lpBaseAddress = NULL;

	return bRet;
}
