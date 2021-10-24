// FuckingService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include<Windows.h>
#include<thread>
#include <vector>
#include<tchar.h>
#include "MainClass.h"

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME  _T("My Sample Service")  
std::wstring path;
int _tmain(int argc, TCHAR* argv[])
{
	path = argv[0];

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{(LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	if(StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		return GetLastError();
	}

	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
	{
		DWORD Status = E_FAIL;

		// Register our service control handler with the SCM
		g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

		if(g_StatusHandle == NULL)
		{
			goto EXIT;
		}

		// Tell the service controller we are starting
		ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
		g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwServiceSpecificExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 0;

		if(SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}

		/*
		 * Perform tasks necessary to start the service here
		 */

		 // Create a service stop event to wait on later
		g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if(g_ServiceStopEvent == NULL)
		{
			// Error creating event
			// Tell service controller we are stopped and exit
			g_ServiceStatus.dwControlsAccepted = 0;
			g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			g_ServiceStatus.dwWin32ExitCode = GetLastError();
			g_ServiceStatus.dwCheckPoint = 1;

			if(SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
			{
				OutputDebugString(_T(
					"My Sample Service: ServiceMain: SetServiceStatus returned error"));
			}
			goto EXIT;
		}

		// Tell the service controller we are started
		g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 0;

		if(SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}

		// Start a thread that will perform the main task of the service
		HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

		// Wait until our worker thread exits signaling that the service needs to stop
		WaitForSingleObject(hThread, INFINITE);


		/*
		 * Perform any cleanup tasks
		 */

		CloseHandle(g_ServiceStopEvent);

		// Tell the service controller we are stopped
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 3;

		if(SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		}
	}
EXIT:
	{
		return;
	}
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch(CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if(g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		 * Perform tasks necessary to stop the service here
		 */

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if(SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString(_T(
				"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error"));
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	HANDLE event_log = RegisterEventSourceA(NULL, "Disk Free Space Monitor Service");
	//  Periodically check if the service has been requested to stop
	MainClass Disks(path);
	try
	{
		Disks.LoadData();
	}
	catch(std::invalid_argument e)
	{
		const char* message = e.what();
		ReportEventA(event_log, EVENTLOG_WARNING_TYPE, 0, 1, NULL, 1, 0, &message, NULL);
		ExitProcess(0);
	};
	std::vector<std::thread> threads;
	threads.emplace_back(&(MainClass::CheckForCoreSettingsChanges), std::ref(Disks));
	threads.emplace_back(&(MainClass::CheckForLimitSettingsChanges), std::ref(Disks));
	threads.emplace_back(&(MainClass::BeginMonitoring), std::ref(Disks));
	while(WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	for(auto& i : threads)
	{
		i.detach();
	}
	return ERROR_SUCCESS;
}



