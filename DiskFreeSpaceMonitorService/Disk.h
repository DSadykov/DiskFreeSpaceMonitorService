#pragma once
#include <string>
#include<vector>

class SomeDisk
{
    std::string _Name;
    long long _FreeSpace;
    long long _Limit;
    bool _chckStatus{true};
    void loadInfo(std::string&&);
public:
    SomeDisk() = delete;
    SomeDisk(std::string&& st);
    void UpdateLimit(long long);
    std::string getName();
    bool getChckStatus();
    void setChckStatus(bool);
    long long getFreeSpace();
    long long getLimit();
    void updateStatus();
};

