
#include "MainClass.h"
#include <thread>
#include <mutex>
#include<Windows.h>
#include <ctime>
#include<chrono>

//#include "Poco/Net/NetException.h"
#pragma warning(disable : 4996)
using namespace std;
mutex access;
HANDLE event_log = RegisterEventSourceA(NULL, "Disk Free Space Monitor Service");
MainClass::MainClass(wstring tmp)
	:_pathCoreSettings(tmp.substr(0, tmp.find_last_of(L"\\")) + L"\\CoreSettings.ini"),
	_pathLimitSettings(tmp.substr(0, tmp.find_last_of(L"\\")) + L"\\LimitSettings.ini"),
	_pathLog(tmp.substr(0, tmp.find_last_of(L"\\")) + L"\\DiskFreeSpaceMonitorService.log")
{
	_fileCoreSettings.open(_pathCoreSettings);
	_fileLimitSettings.open(_pathLimitSettings);
	ReportLog("[INFO]     Service started");
}


auto TimeNow()
{
	auto start = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(start);
	string tmp = ctime(&end_time);
	tmp.replace(tmp.find("\n"), 2, " ");
	return tmp;
}

void MainClass::ReportLog(string tmp)
{
	_fileLog.open(_pathLog, ios_base::app);
	_fileLog << TimeNow() << tmp << endl;
	_fileLog.close();
}

string quickAddThousandSeparators(std::string value, char thousandSep = ' ')
{
	int len = value.length();
	int dlen = 3;

	while(len > dlen)
	{
		value.insert(len - dlen, 1, thousandSep);
		dlen += 4;
		len += 1;
	}
	return value;
}

void MainClass::LoadData()
{

	if(!_fileLimitSettings.is_open() || !_fileCoreSettings.is_open())
	{
		if(!_fileLimitSettings.is_open())
		{
			ofstream tmp_Limit(_pathLimitSettings);
			UploadLimitSettings(tmp_Limit);
			tmp_Limit.close();
		}
		if(!_fileCoreSettings.is_open())
		{
			ofstream tmp_Core(_pathCoreSettings);
			UploadCoreSettings(tmp_Core);
			tmp_Core.close();
		}
		ReportLog("[WARNING]  Settings files were created, check them and restart service");
		throw invalid_argument("Settings files were created, check them and restart service");
	}
	_fileCoreSettings.close();
	_fileLimitSettings.close();
	LoadCoreSettings();
	LoadLimitSettings();
}





int MainClass::SendEmail(std::string body)
{
	Email Sender(data[0], stoi(data[1]), data[2], data[3], data[4]);
	return Sender.sendEmail(body);
}

void MainClass::CheckForLimitSettingsChanges()
{

	while(true)
	{
		access.lock();
		string fromFile{""};
		string tmpfullLimitData{""};
		_fileLimitSettings.open(_pathLimitSettings);
		while(_fileLimitSettings >> fromFile)
		{
			if(fromFile.find('=') != string::npos)
				tmpfullLimitData += fromFile;
			//_fileDebug << fromFile << endl;
			//_fileDebug.flush();
		}
		//_fileDebug << tmpfullLimitData <<  endl;
		if(tmpfullLimitData != _fullLimitData)
		{
			_fileLimitSettings.close();
			this->LoadLimitSettings();
			_fullLimitData = tmpfullLimitData;
			//_fileDebug << "Limit upd" << endl;
		}
		if(_fileLimitSettings.is_open())
			_fileLimitSettings.close();
		access.unlock();
		this_thread::sleep_for(chrono::seconds(15));
	}
}



void MainClass::CheckForCoreSettingsChanges()
{

	while(true)
	{
		access.lock();
		string fromFile{""};
		string tmpfullCoreData{""};
		_fileCoreSettings.open(_pathCoreSettings);
		while(_fileCoreSettings >> fromFile)
		{
			if(fromFile.find('=') != string::npos)
				tmpfullCoreData += fromFile;
			/*_fileDebug << fromFile << endl;
			_fileDebug.flush();*/
		}
		//_fileDebug << _tmpfullCoreData<<endl;
		if(tmpfullCoreData != _fullCoreData)
		{
			_fileCoreSettings.close();
			this->LoadCoreSettings();
			_fullCoreData = tmpfullCoreData;
			//_fileDebug << "Core upd"<< endl;
		}
		if(_fileCoreSettings.is_open())
			_fileCoreSettings.close();
		access.unlock();
		this_thread::sleep_for(chrono::seconds(15));
	}

}



int MainClass::BeginMonitoring()
{

	while(true)
	{
		access.lock();
		for(auto& i : _selectedDisks)
		{
			i.updateStatus();
			if((i.getLimit() > i.getFreeSpace()) && i.getChckStatus() && i.getFreeSpace() != 0)
			{
				string tmpBody = "Disk " + i.getName() + " has " + quickAddThousandSeparators(to_string(i.getFreeSpace())) + " free gigabytes, which is lower than " + quickAddThousandSeparators(to_string(i.getLimit()));
				try
				{
					SendEmail(tmpBody);
					tmpBody = "[INFO]     Message sent successfully with body \"" + tmpBody + "\" to address : " + data[2];
					auto tmp = tmpBody.c_str();
					ReportEventA(event_log, EVENTLOG_SUCCESS, 0, 1, NULL, 1, 0, &tmp, NULL);
					ReportLog(tmp);
				}
				catch(std::exception &e)
				{
					string tm = "[ERROR]    while sending message:"; tm += e.what();
					LPCSTR tmp = tm.c_str();
					ReportEventA(event_log, EVENTLOG_WARNING_TYPE, 0, 1, NULL, 1, 0, &tmp, NULL);
					ReportLog(tmp);
				}
				/*catch(Poco::Net::SMTPException& e)
				{
					string tm = "[ERROR]   while sending message: " + e.displayText();
					LPCSTR tmp = tm.c_str();
					ReportEventA(event_log, EVENTLOG_WARNING_TYPE, 0, 1, NULL, 1, 0, &tmp, NULL);
					ReportLog(tmp);
				}*/

			}
			else if(!i.getChckStatus() && (i.getLimit() <= i.getFreeSpace()))
				i.setChckStatus(true);
		}

		access.unlock();
		this_thread::sleep_for(chrono::hours(_checkRate));
	}
	return 0;
}


void MainClass::UploadLimitSettings(ofstream& temp_file)
{
	DiskInfo tmp{};
	temp_file << "//Specify threshold in gigabytes\n";
	for(auto i : tmp.getDisks())
	{

		temp_file << i.getName() << "=\n";

	}

}



void MainClass::UploadCoreSettings(ofstream& temp_file)
{
	temp_file << "SMTP_login=\n" << "SMTP_password=\n" << "SMTP_server=\n" << "SMTP_port=\n" << "mail_to=\n" << "//Specify checkperiod in hours\n" << "checkperiod=\n";
}

void MainClass::LoadLimitSettings()
{
	bool flagWarn{false};
	_fileLimitSettings.open(_pathLimitSettings);
	string fromFile{};
	_selectedDisks.clear();
	_fullLimitData = "";
	int i = 0;
	string tmpBody{"[INFO]     Limit settings loaded successfully, selected disks: "};
	string tmpBodyNo{"[WARNING]  These disks do not exist and will not be monitored: "};
	while(_fileLimitSettings >> fromFile)
	{
		if(fromFile.find('=') != string::npos)
		{
			_fullLimitData += fromFile;
			string after = fromFile.substr(fromFile.find("=") + 1, fromFile.size());
			string before = fromFile.substr(0, fromFile.find("="));
			if(GetDiskFreeSpaceA(before.c_str(), nullptr, nullptr, nullptr, nullptr) != 0)
			{
				try
				{
					auto tmp = stoll(after);
					tmpBody += before + " = " + after + "; ";
					_selectedDisks.emplace_back(SomeDisk(move(before)));
					_selectedDisks[i].UpdateLimit(tmp);
					i++;
				}

				catch(std::invalid_argument)
				{

				}
			}
			else
			{
				tmpBodyNo += before + "; ";
				flagWarn = true;
			}
		}
	}
	auto tmp = tmpBody.c_str();
	auto tmpNo = tmpBodyNo.c_str();
	ReportEventA(event_log, EVENTLOG_SUCCESS, 0, 1, NULL, 1, 0, &tmp, NULL);
	ReportLog(tmp);
	if(flagWarn)
	{
		ReportLog(tmpNo);
		ReportEventA(event_log, EVENTLOG_WARNING_TYPE, 0, 1, NULL, 1, 0, &tmpNo, NULL);
	}
	_fileLimitSettings.close();
}

void MainClass::LoadCoreSettings()
{

	_fileCoreSettings.open(_pathCoreSettings);
	int port{0};
	string fromFile{""};
	_fullCoreData = "";
	string msg = "[INFO]     Core settings loaded successfully, loaded data: ";
	while(_fileCoreSettings >> fromFile)
	{
		if(fromFile.find('=') != string::npos)
		{
			_fullCoreData += fromFile;

			string after = fromFile.substr(fromFile.find("=") + 1, fromFile.size());
			string before = fromFile.substr(0, fromFile.find("="));
			if(before == "SMTP_password")
				msg += before + "=***** ";
			else
				msg += fromFile + " ";
			if(before == "SMTP_server")
				data[0] = after;

			else if(before == "SMTP_port")
			{
				try
				{
					port = stoi(after);
					data[1] = after;
				}
				catch(invalid_argument)
				{
					ReportLog("[ERROR]   Port must be integral");
					throw invalid_argument("Port must be integral");
				}
			}

			else if(before == "SMTP_login")
				data[2] = after;
			else if(before == "mail_to")
			{
				data[4] = after;
			}
			else if(before == "SMTP_password")
				data[3] = after;

			else if(before == "checkperiod")
			{
				try
				{
					_checkRate = stoi(after);
				}
				catch(invalid_argument)
				{
					ReportLog("[ERROR]    Check rate must be integral");
					throw invalid_argument("Check rate must be integral");
				}
			}
		}
	}
	LPCSTR tmp = msg.c_str();
	ReportEventA(event_log, EVENTLOG_SUCCESS, 0, 1, NULL, 1, 0, &tmp, NULL);
	ReportLog(tmp);
	_fileCoreSettings.close();

}


MainClass::~MainClass()
{
	//_fileDebug.close();
	_fileLimitSettings.close();
	_fileCoreSettings.close();
	ReportLog("[INFO]     Service stopped");
}
