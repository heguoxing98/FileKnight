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
	 * @brief: ��ȡ�߼����̵ľ�� 
	 */
	HANDLE GetLogicDiskObject(const wchar_t diskSymbol);

	/*
	 * @brief: ����USN��־ 
	 */
	bool CreateUSN(HANDLE logicDiskObject);
};



#endif 


