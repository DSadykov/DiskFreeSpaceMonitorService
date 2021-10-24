#pragma once
#include<fstream>
#include "DiskInfo.h"
#include "Email.h"
#include<array>
class MainClass
{
private:
	std::wstring _pathLimitSettings{};
	std::wstring _pathCoreSettings{};
	std::wstring _pathLog{};
	std::ifstream _fileLimitSettings;
	std::ifstream _fileCoreSettings;
	std::ofstream _fileLog;
	//std::ofstream _fileDebug{"e:\\OneDrive\\!VisStud\\C++\\FuckingService\\x64\\Release\\debug.txt"};
	std::vector<SomeDisk> _selectedDisks{};
	//std::vector<std::thread> _threads{};
	std::string _fullLimitData{};
	std::string _fullCoreData{};
	std::array < std::string, 5> data{};
	int _checkRate{};
	bool _checkActive = true;
	void UploadLimitSettings(std::ofstream&);
	void UploadCoreSettings(std::ofstream&);
	void LoadLimitSettings();
	void LoadCoreSettings();
	int SendEmail(std::string);
	void ReportLog(std::string);
public:
	MainClass(std::wstring);
	void LoadData();
	void CheckForLimitSettingsChanges();
	void CheckForCoreSettingsChanges();
	int BeginMonitoring();
	~MainClass();

};

