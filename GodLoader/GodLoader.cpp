#include "GodLoader.h"
#include<windows.h>
#include<stdio.h>

namespace loader
{
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

	if (false == FixRelocationTable(baseAddress))
		return nullptr;

	if (false == FixImportTable(baseAddress))
		return nullptr;

	DWORD oldProtect = 0;
	if (false == ::VirtualProtect(baseAddress, sizeOfImage, PAGE_EXECUTE_READWRITE, &oldProtect))
		return nullptr;

	if (false == SetImageBase(baseAddress))
		return nullptr;

	if (false == CallDllMain(baseAddress))
		return nullptr;

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
				// 关于修复重定位表的理解，https://cloud.tencent.com/developer/article/1432449
				// 举个例子：
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
	PIMAGE_DOS_HEADER dosHeaderPtr = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS ntHeadersPtr = (PIMAGE_NT_HEADERS)((ULONG32)dosHeaderPtr + dosHeaderPtr->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR importTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)dosHeaderPtr + ntHeadersPtr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	char* dllName = nullptr;
	HMODULE desDllHandle = NULL;
	PIMAGE_THUNK_DATA importNameArray = nullptr;
	PIMAGE_IMPORT_BY_NAME importByName = nullptr;
	PIMAGE_THUNK_DATA importFuncAddrArray = nullptr;
	FARPROC funcAddress = nullptr;
	DWORD index = 0;

	while (TRUE)
	{
		if (0 == importTable->OriginalFirstThunk)
		{
			break;
		}

		dllName = (char*)((DWORD)dosHeaderPtr + importTable->Name);
		desDllHandle = ::GetModuleHandleA(dllName);
		if (NULL == desDllHandle)
		{
			desDllHandle = ::LoadLibraryA(dllName);
			if (NULL == desDllHandle)
			{
				importTable++;
				continue;
			}
		}

		index = 0;
		importNameArray = (PIMAGE_THUNK_DATA)((DWORD)dosHeaderPtr + importTable->OriginalFirstThunk);
		importFuncAddrArray = (PIMAGE_THUNK_DATA)((DWORD)dosHeaderPtr + importTable->FirstThunk);
		while (TRUE)
		{
			if (0 == importNameArray[index].u1.AddressOfData)
			{
				break;
			}
			importByName = (PIMAGE_IMPORT_BY_NAME)((DWORD)dosHeaderPtr + importNameArray[index].u1.AddressOfData);

			// 判断导出函数是序号导出还是函数名称导出
			if (0x80000000 & importNameArray[index].u1.Ordinal)
			{
				// 序号导出
				// 当IMAGE_THUNK_DATA值的最高位为1时，表示函数以序号方式输入，这时，低位被看做是一个函数序号
				funcAddress = ::GetProcAddress(desDllHandle, (LPCSTR)(importNameArray[index].u1.Ordinal & 0x0000FFFF));
			}
			else
			{
				// 名称导出
				funcAddress = ::GetProcAddress(desDllHandle, (LPCSTR)importByName->Name);
			}

			importFuncAddrArray[index].u1.Function = (DWORD)funcAddress;
			index++;
		}

		importTable++;
	}

	return TRUE;
}

bool SetImageBase(LPVOID baseAddress)
{
	PIMAGE_DOS_HEADER dosHeaderPtr = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS ntHeadersPtr = (PIMAGE_NT_HEADERS)((ULONG32)dosHeaderPtr + dosHeaderPtr->e_lfanew);
	ntHeadersPtr->OptionalHeader.ImageBase = (ULONG32)baseAddress;

	return true;
}

bool CallDllMain(LPVOID baseAddress)
{
	fnDllMain _DllMain = nullptr;
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((ULONG32)dosHeader + dosHeader->e_lfanew);
	_DllMain = (fnDllMain)((ULONG32)dosHeader + ntHeaders->OptionalHeader.AddressOfEntryPoint);

	BOOL isOk = _DllMain((HINSTANCE)baseAddress, DLL_PROCESS_ATTACH, NULL);

	return isOk;
}

LPVOID GetProcAddressAnsi(LPVOID baseAddress, PCHAR desName)
{
	LPVOID desAddress = nullptr;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((ULONG32)dosHeader + dosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY exportTable = (PIMAGE_EXPORT_DIRECTORY)((DWORD)dosHeader + ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	PCHAR  funcName = nullptr;
	WORD   index = 0;
	DWORD  numberOfNames = exportTable->NumberOfNames;
	PDWORD addressOfNamesArray = (PDWORD)((DWORD)dosHeader + exportTable->AddressOfNames);
	PWORD  addressOfNameOrdinalsArray = (PWORD)((DWORD)dosHeader + exportTable->AddressOfNameOrdinals);
	PDWORD addressOfFunctionsArray = (PDWORD)((DWORD)dosHeader + exportTable->AddressOfFunctions);


	for (DWORD i = 0; i < numberOfNames; i++)
	{
		funcName = (PCHAR)((DWORD)dosHeader + addressOfNamesArray[i]);
		if (0 == ::lstrcmpiA(funcName, desName))
		{
			index = addressOfNameOrdinalsArray[i];
			desAddress = (LPVOID)((DWORD)dosHeader + addressOfFunctionsArray[index]);
			break;
		}
	}
	return desAddress;
}

bool FreeDll(LPVOID baseAddress)
{
	bool isOk = false;

	if (NULL == baseAddress)
	{
		return isOk;
	}

	isOk = VirtualFree(baseAddress, 0, MEM_RELEASE);
	baseAddress = NULL;

	return isOk;
}




}

