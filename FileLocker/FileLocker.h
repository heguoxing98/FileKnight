#ifndef _FILE_LOCKER_H_
#define _FILE_LOCKER_H_

#include <iostream>
#include <Windows.h>
#include <vector>

using HandlesVector = std::vector<HANDLE>;

class FileLocker
{
public:
	static FileLocker* Instance()
	{
		if (fileLockerIns_ == nullptr)
			fileLockerIns_ = new FileLocker();

		return fileLockerIns_;
	}

	static void DestroyIns()
	{
		if (fileLockerIns_ != nullptr)
			delete fileLockerIns_;

		fileLockerIns_ = nullptr;
	}

public:
	FileLocker();
	~FileLocker();

	HandlesVector GetAllHandles(const std::string& objType);

private:
	static FileLocker* fileLockerIns_;

};

#endif 





