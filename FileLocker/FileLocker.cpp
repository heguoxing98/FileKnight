#include "FileLocker.h"

FileLocker::FileLocker()
{
}

FileLocker::~FileLocker()
{
}

HandlesVector 
FileLocker::GetAllHandles(const std::string& objType)
{
	HandlesVector handlesVec;
	if (objType.empty())
		return handlesVec;



}
