#ifndef _GOD_LOADER_H_
#define _GOD_LOADER_H_

#include <Windows.h>

namespace loader
{
typedef BOOL(__stdcall* fnDllMain)(HINSTANCE instance, DWORD reason, LPVOID reserved);

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

/*
	* @brief: ���ü��ص�ַ
	*/
bool SetImageBase(LPVOID baseAddress);

/*
	* @brief: ���ö�̬�����
	*/
bool CallDllMain(LPVOID baseAddress);

/*
 * @brief: ͬ[::GetProcAddress]
 */
LPVOID GetProcAddressAnsi(LPVOID baseAddress, PCHAR desName);

/*
 * @brief: �ͷ������ص�dll  
 */
bool FreeDll(LPVOID baseAddress);



}


#endif
