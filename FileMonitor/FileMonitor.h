#ifndef _FILE_MONIOTR_H_
#define _FILE_MONIOTR_H_

#include <Windows.h>
#include <atlstr.h>

class FileMonitor
{
public:
	FileMonitor();
	~FileMonitor();

	/*
	 * @brief: 获取逻辑磁盘的句柄 
	 */
	HANDLE GetLogicDiskObject(const wchar_t diskSymbol);

	/*
	 * @brief: 创建USN日志 
	 */
	bool CreateUSN(HANDLE logicDiskObject);
};



#endif 


