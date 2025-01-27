#include <Windows.h>
#include <iostream>
#include <tchar.h>
#include <string>

SERVICE_STATUS gSvcStatus;
SERVICE_STATUS_HANDLE gSvcStatusHandle = nullptr;
HANDLE gSvcStopEvent = nullptr;
#define SVCNAME LPWSTR("MiniSvc")
void WriteLog(const char* msg)
{
	OutputDebugStringA(msg);
}

void __stdcall SvcMain(DWORD, LPSTR*);
void __stdcall SvcCtrlHandler(DWORD);
void LogEvent(const std::wstring& message);

int __cdecl _tmain(int argc, TCHAR* argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[] = {
		{SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain},
		{nullptr, nullptr}
	};
	if (!StartServiceCtrlDispatcher(ServiceTable)) return 1;
	return 0;
}

void __stdcall SvcMain(DWORD argc, LPSTR* argv) {
	gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);
	if (!gSvcStatusHandle) return;
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwCurrentState = SERVICE_START_PENDING;
	gSvcStatus.dwControlsAccepted = 0;
	gSvcStatus.dwWin32ExitCode = 0;
	gSvcStatus.dwServiceSpecificExitCode = 0;
	gSvcStatus.dwCheckPoint = 0;
	gSvcStatus.dwWaitHint = 0;

	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);

	gSvcStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (!gSvcStopEvent)
	{
		gSvcStatus.dwCurrentState = SERVICE_STOPPED;
		gSvcStatus.dwWin32ExitCode = GetLastError();
		SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
		return;
	}
	gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	gSvcStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
	WriteLog("MiniSvc is running\n");
	//main
	DWORD counter = 0;
	while (WaitForSingleObject(gSvcStopEvent, 0) != WAIT_OBJECT_0) 
	{
		counter++;
		std::wstring msg = std::to_wstring(counter);
		LogEvent(msg);
		Sleep(10000);
	}

	//cleanup
	WriteLog("MiniSvc is stopping\n");
	CloseHandle(gSvcStopEvent);
	gSvcStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

void __stdcall SvcCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP:
		WriteLog("Svc stop req");
		gSvcStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
		SetEvent(gSvcStopEvent);
		break;
	default:
		break;
	}
}
void LogEvent(const std::wstring& msg)
{
	HANDLE gEventSrc = RegisterEventSource(NULL, SVCNAME);
	if (gEventSrc) {
		LPCWSTR pmsg = msg.c_str();
		ReportEvent(gEventSrc, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &pmsg, NULL);
		DeregisterEventSource(gEventSrc);
	}
}