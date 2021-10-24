#include "DiskInfo.h"
#include <Windows.h>

using namespace std;

DiskInfo::DiskInfo()
{
    loadDisks();
}

void DiskInfo::loadDisks()
{
    deleteDisks();
    DWORD sizebuf = MAX_PATH, temp;
    CHAR temp_disks[MAX_PATH] = {0};
    temp = GetLogicalDriveStringsA(sizebuf, temp_disks);
    if(temp > 0 && temp <= MAX_PATH)
    {
        CHAR* szSingleDrive = temp_disks;
        while(*szSingleDrive)
        {
            _Disks.push_back(SomeDisk(static_cast<string>(szSingleDrive)));
            _AllDisks += szSingleDrive;
            szSingleDrive += strlen(szSingleDrive) + 1;
        }
    }
}

void DiskInfo::deleteDisks()
{
    _Disks.clear();
}


std::vector<SomeDisk>& DiskInfo::getDisks()
{
    return _Disks;
}
