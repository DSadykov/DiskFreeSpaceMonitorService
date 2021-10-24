#pragma once
#include "Disk.h"
class DiskInfo
{
private:
    std::vector<SomeDisk> _Disks{};
    std::string _AllDisks{};

public:
    DiskInfo();
    void loadDisks();
    void deleteDisks();
    std::vector<SomeDisk>& getDisks();
};

