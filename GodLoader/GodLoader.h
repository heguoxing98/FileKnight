#ifndef _GOD_LOADER_H_
#define _GOD_LOADER_H_

#include <Windows.h>

namespace loader
{
typedef BOOL(__stdcall* fnDllMain)(HINSTANCE instance, DWORD reason, LPVOID reserved);

/*
	* @brief: 加载内存DLL文件到进程中
	* @param: [data] 内存DLL文件数据的基址
	* @param: [dataSize] 内存DLL文件的内存大小
	* @return: 内存DLL加载到进程的加载基址
	*/
LPVOID LoadDll(LPVOID data, unsigned long dataSize);

/*
	* @brief: 获取PE文件加载到内存后的镜像大小
	* @param: [data] 内存DLL文件数据的基址
	* @return: 返回PE文件结构中IMAGE_NT_HEADERS.OptionalHeader.SizeOfImage值的大小
	*/
DWORD GetSizeOfImage(LPVOID data);

/*
	* @brief: 将内存DLL数据按SectionAlignment大小对齐映射到进程内存中
	* @param: [data] 内存DLL文件数据的基址
	* @param: [baseAddress] 内存DLL数据按SectionAlignment大小对齐映射到进程内存中的内存基址
	*/
bool MapDataToMemory(LPVOID data, LPVOID baseAddress);

/*
	* @brief: 修正PE文件重定位表信息
	* @return: 成功 [true]，失败 [false]
	*/
bool FixRelocationTable(LPVOID baseAddress);

/*
	* @brief: 修正PE文件导入表信息
	* @return: 成功 [true]，失败 [false]
	*/
bool FixImportTable(LPVOID baseAddress);

/*
	* @brief: 设置加载地址
	*/
bool SetImageBase(LPVOID baseAddress);

/*
	* @brief: 调用动态库入口
	*/
bool CallDllMain(LPVOID baseAddress);

/*
 * @brief: 同[::GetProcAddress]
 */
LPVOID GetProcAddressAnsi(LPVOID baseAddress, PCHAR desName);

/*
 * @brief: 释放所加载的dll  
 */
bool FreeDll(LPVOID baseAddress);



}


#endif
