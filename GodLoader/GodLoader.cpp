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

	if (FALSE == FixImportTable(baseAddress))
		return nullptr;

	DWORD oldProtect = 0;
	if (FALSE == ::VirtualProtect(baseAddress, sizeOfImage, PAGE_EXECUTE_READWRITE, &oldProtect))
		return nullptr;

	if (FALSE == SetImageBase(baseAddress))
		return nullptr;

	// ����DLL����ں���DllMain,������ַ��ΪPE�ļ�����ڵ�IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
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
			if ((DWORD)(relocData[i] & 0x0000F000) == 0x00003000) //����һ����Ҫ�����ĵ�ַ
			{
				// 32λdll�ض�λ��IMAGE_REL_BASED_HIGHLOW
				// ����x86�Ŀ�ִ���ļ������еĻ�ַ�ض�λ����IMAGE_REL_BASED_HIGHLOW���͵ġ�
				// 
				// �����޸��ض�λ�����⣬https://cloud.tencent.com/developer/article/1432449
				// �ٸ����ӣ�
				// ǰ��PE�ļ�Ĭ�ϼ���λ����0x40000������������PE�ļ� B.dll ��ʱ����һ��ȫ�ֱ����� globalValue, globalValue �ĵ�ַ��0x40123��
				// ��һ��EXE���� B.dll ʱ������λ��Ϊ 0x5000������֪������ڽ��и�ֵ���Ĵ���ʱ��
				// ����ȡ��ַ����` mov ebp [0x401234] `���ǳ���ջ�ھͻ��и���ַ������Ϊ0x51567��-
				// - �����ַһ����mov ebp��ַ֮���һ����ַ����һ����ͨ��RVA����ʽ��¼��������ImageBase�仯���仯���洢 globalVlaue�ĵ�ַ 0x40123
				// ������ô��ִ����ôһ��ָ��` mov ebp [0x401234] `ʱ���ͻ����������Ҫ�� 0x40123 ����Ϊ 0x50123
				// �����ض�λ��Ĺ������һϵ�����㣬���õ�����һ����ַ��Ҳ�ʹ����е� address �ǵ��� 0x51567����address����ȡ��ַ�������������Ҫ������ַ
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

			// �жϵ�����������ŵ������Ǻ������Ƶ���
			if (0x80000000 & importNameArray[index].u1.Ordinal)
			{
				// ��ŵ���
				// ��IMAGE_THUNK_DATAֵ�����λΪ1ʱ����ʾ��������ŷ�ʽ���룬��ʱ����λ��������һ���������
				funcAddress = ::GetProcAddress(desDllHandle, (LPCSTR)(importNameArray[index].u1.Ordinal & 0x0000FFFF));
			}
			else
			{
				// ���Ƶ���
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



// ����SectionAlignment
// dwSize: ��ʾδ����ǰ�ڴ�Ĵ�С
// dwAlignment: �����Сֵ
// ����ֵ: �����ڴ����֮���ֵ
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




// ����DLL����ں���DllMain,������ַ��ΪPE�ļ�����ڵ�IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
// lpBaseAddress: �ڴ�DLL���ݰ�SectionAlignment��С����ӳ�䵽�����ڴ��е��ڴ��ַ
// ����ֵ: �ɹ�����TRUE�����򷵻�FALSE
BOOL CallDllMain(LPVOID lpBaseAddress)
{
	typedef_DllMain DllMain = NULL;
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG32)pDosHeader + pDosHeader->e_lfanew);
	DllMain = (typedef_DllMain)((ULONG32)pDosHeader + pNtHeaders->OptionalHeader.AddressOfEntryPoint);
	// ������ں���,���ӽ���DLL_PROCESS_ATTACH
	BOOL bRet = DllMain((HINSTANCE)lpBaseAddress, DLL_PROCESS_ATTACH, NULL);
	if (FALSE == bRet)
	{
		//ShowError("DllMain");
	}

	return bRet;
}


// ģ��GetProcAddress��ȡ�ڴ�DLL�ĵ�������
// lpBaseAddress: �ڴ�DLL�ļ����ص������еļ��ػ�ַ
// lpszFuncName: ��������������
// ����ֵ: ���ص��������ĵĵ�ַ
LPVOID MmGetProcAddress(LPVOID lpBaseAddress, PCHAR lpszFuncName)
{
	LPVOID lpFunc = NULL;
	// ��ȡ������
	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
	PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((ULONG32)pDosHeader + pDosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pExportTable = (PIMAGE_EXPORT_DIRECTORY)((DWORD)pDosHeader + pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	// ��ȡ�����������
	PDWORD lpAddressOfNamesArray = (PDWORD)((DWORD)pDosHeader + pExportTable->AddressOfNames);
	PCHAR lpFuncName = NULL;
	PWORD lpAddressOfNameOrdinalsArray = (PWORD)((DWORD)pDosHeader + pExportTable->AddressOfNameOrdinals);
	WORD wHint = 0;
	PDWORD lpAddressOfFunctionsArray = (PDWORD)((DWORD)pDosHeader + pExportTable->AddressOfFunctions);

	DWORD dwNumberOfNames = pExportTable->NumberOfNames;
	DWORD i = 0;
	// ����������ĵ�������������, ������ƥ��
	for (i = 0; i < dwNumberOfNames; i++)
	{
		lpFuncName = (PCHAR)((DWORD)pDosHeader + lpAddressOfNamesArray[i]);
		if (0 == ::lstrcmpiA(lpFuncName, lpszFuncName))
		{
			// ��ȡ����������ַ
			wHint = lpAddressOfNameOrdinalsArray[i];
			lpFunc = (LPVOID)((DWORD)pDosHeader + lpAddressOfFunctionsArray[wHint]);
			break;
		}
	}
	printf("11111112222222333333333");
	return lpFunc;
}


// �ͷŴ��ڴ���ص�DLL�������ڴ�Ŀռ�
// lpBaseAddress: �ڴ�DLL���ݰ�SectionAlignment��С����ӳ�䵽�����ڴ��е��ڴ��ַ
// ����ֵ: �ɹ�����TRUE�����򷵻�FALSE
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
