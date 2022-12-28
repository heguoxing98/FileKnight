#ifndef _GOD_LOADER_H_
#define _GOD_LOADER_H_

#include <Windows.h>

typedef BOOL(__stdcall* typedef_DllMain)(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved);

/*
 * @brief: �����ڴ�DLL�ļ���������
 * @param: [data] �ڴ�DLL�ļ����ݵĻ�ַ
 * @param: [dataSize] �ڴ�DLL�ļ����ڴ��С
 * @return: �ڴ�DLL���ص����̵ļ��ػ�ַ
 */
LPVOID LoadDll(LPVOID data, unsigned long dataSize);

/*
 * @brief: ��ȡPE�ļ����ص��ڴ��ľ����С
 * @param: [data] �ڴ�DLL�ļ����ݵĻ�ַ
 * @return: ����PE�ļ��ṹ��IMAGE_NT_HEADERS.OptionalHeader.SizeOfImageֵ�Ĵ�С
 */
DWORD GetSizeOfImage(LPVOID data);

/*
 * @brief: ���ڴ�DLL���ݰ�SectionAlignment��С����ӳ�䵽�����ڴ���
 * @param: [data] �ڴ�DLL�ļ����ݵĻ�ַ
 * @param: [baseAddress] �ڴ�DLL���ݰ�SectionAlignment��С����ӳ�䵽�����ڴ��е��ڴ��ַ
 */
bool MapDataToMemory(LPVOID data, LPVOID baseAddress);

/*
 * @brief: ����PE�ļ��ض�λ����Ϣ
 * @return: �ɹ� [true]��ʧ�� [false]
 */
bool FixRelocationTable(LPVOID baseAddress);

/*
 * @brief: ����PE�ļ��������Ϣ
 * @return: �ɹ� [true]��ʧ�� [false]
 */
bool FixImportTable(LPVOID baseAddress);

bool SetImageBase(LPVOID baseAddress);


// ����SectionAlignment
// dwSize: ��ʾδ����ǰ�ڴ�Ĵ�С
// dwAlignment: �����Сֵ
// ����ֵ: �����ڴ����֮���ֵ
DWORD Align(DWORD dwSize, DWORD dwAlignment);



// ����DLL����ں���DllMain,������ַ��ΪPE�ļ�����ڵ�IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint
// lpBaseAddress: �ڴ�DLL���ݰ�SectionAlignment��С����ӳ�䵽�����ڴ��е��ڴ��ַ
// ����ֵ: �ɹ�����TRUE�����򷵻�FALSE
BOOL CallDllMain(LPVOID lpBaseAddress);

// ģ��GetProcAddress��ȡ�ڴ�DLL�ĵ�������
// lpBaseAddress: �ڴ�DLL�ļ����ص������еļ��ػ�ַ
// lpszFuncName: ��������������
// ����ֵ: ���ص��������ĵĵ�ַ
LPVOID MmGetProcAddress(LPVOID lpBaseAddress, PCHAR lpszFuncName);

// �ͷŴ��ڴ���ص�DLL�������ڴ�Ŀռ�
// lpBaseAddress: �ڴ�DLL���ݰ�SectionAlignment��С����ӳ�䵽�����ڴ��е��ڴ��ַ
// ����ֵ: �ɹ�����TRUE�����򷵻�FALSE
BOOL MmFreeLibrary(LPVOID lpBaseAddress);

#endif
