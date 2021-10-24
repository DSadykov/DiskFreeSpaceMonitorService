#include "Disk.h"
#include <Windows.h>
#include <fstream>
using namespace std;

SomeDisk::SomeDisk(std::string&& st) :_Name(st)
{
	loadInfo(forward<string>(st));

}

void SomeDisk::UpdateLimit(long long tmp)
{
	_Limit = tmp;
}

string SomeDisk::getName()
{
	return _Name;
}

bool SomeDisk::getChckStatus()
{
	return _chckStatus;
}

void SomeDisk::setChckStatus(bool st)
{ 
	_chckStatus = st;
}


long long SomeDisk::getFreeSpace()
{
	return _FreeSpace;
}


long long SomeDisk::getLimit()
{
	return _Limit;
}



void SomeDisk::updateStatus()
{
	ULARGE_INTEGER tmp{};
	GetDiskFreeSpaceExA(_Name.c_str(), &tmp, nullptr, nullptr);
	_FreeSpace = tmp.QuadPart / 1024 / 1024/1024;
}

void SomeDisk::loadInfo(string&& st)
{
	ULARGE_INTEGER tmpFree{};
	GetDiskFreeSpaceExA(st.c_str(), &tmpFree, nullptr, nullptr);
	_FreeSpace = move(tmpFree.QuadPart / 1024 / 1024/1024);
}
