#include "stdafx.h"

#include "../SharedFunctionality/NetworkManager.h"
#include "../SharedFunctionality/SharedFunctions.h"
#include "../SharedFunctionality/CageData.h"

#include "Aclapi.h"

#include <unordered_set>
#include <thread>

#include "CageManager.h"
#include "CageLabeler.h"
#include "SecuritySetup.h"
#include "CageDesktop.h"

#pragma comment(lib, "Rpcrt4.lib")

NetworkManager network_manager(ContextType::MANAGER);

int main()
{
	CageManager cage_manager;

	SecuritySetup security_setup;
	auto security_attributes = security_setup.GetSecurityAttributes();

	if (!security_attributes.has_value())
	{
		std::cout << "Could not get security attributes" << std::endl;
		return 1;
	}

	// listen for the message
	std::wstring message = network_manager.Listen(10);
	std::wstring message_data;
	auto parse_result = SharedFunctions::ParseMessage(message, message_data);

	if (parse_result != CageMessage::START_PROCESS)
	{
		std::cout << "Could not process incoming message" << std::endl;
		return 1;
	}

	CageData cage_data = { message_data };
	if (!SharedFunctions::ParseStartProcessMessage(cage_data))
	{
		std::cout << "Could not process start process message" << std::endl;
		return 1;
	}

	const int work_area_width = 300;
	std::thread desktop_thread(
		&CageManager::StartCage,
		cage_manager,
		security_attributes.value(),
		cage_data
	);

	desktop_thread.join();

	return 0;
}

void CageManager::StartCage(SECURITY_ATTRIBUTES security_attributes, const CageData &cage_data)
{
	// name should be unique every time -> create UUID
	UUID uuid;
	if (::UuidCreate(&uuid) != RPC_S_OK)
	{
		std::cout << "Failed to create UUID" << std::endl;
		return;
	}

	RPC_WSTR uuid_str;
	if (::UuidToString(&uuid, &uuid_str) != RPC_S_OK)
	{
		std::cout << "Failed to convert UUID to rpc string" << std::endl;
		return;
	}

	std::wstring uuid_stl(reinterpret_cast<wchar_t*>(uuid_str));
	if (uuid_stl.empty())
	{
		::RpcStringFree(&uuid_str);
		std::cout << "Failed to convert UUID rpc string to stl string" << std::endl;
		return;
	}

	::RpcStringFree(&uuid_str);
	
	const std::wstring DESKTOP_NAME = std::wstring(L"shark_cage_desktop_").append(uuid_stl);
	const int work_area_width = 300;
	CageDesktop cage_desktop(
		security_attributes,
		work_area_width,
		DESKTOP_NAME);

	HDESK desktop_handle;
	if (!cage_desktop.Init(desktop_handle))
	{
		std::cout << "Failed to create/launch the cage desktop" << std::endl;
		return;
	}

	const std::wstring LABELER_WINDOW_CLASS_NAME = std::wstring(L"shark_cage_token_window").append(uuid_stl);
	std::thread labeler_thread(
		&CageManager::StartCageLabeler,
		this,
		desktop_handle,
		cage_data,
		work_area_width,
		LABELER_WINDOW_CLASS_NAME
	);

	//PETER´S ACCESS TOKEN THINGS

	// We need in order to create the process.
	STARTUPINFO info = {};
	info.dwFlags = STARTF_USESHOWWINDOW;
	info.wShowWindow = SW_MAXIMIZE;

	// The desktop's name where we are going to start the application. In this case, our new desktop.
	info.lpDesktop = const_cast<LPWSTR>(DESKTOP_NAME.c_str());

	// Create the process.
	PROCESS_INFORMATION process_info = {};
	std::vector<wchar_t> path_buf(cage_data.app_path.begin(), cage_data.app_path.end());
	path_buf.push_back(0);

	if (::CreateProcess(path_buf.data(), nullptr, &security_attributes, nullptr, FALSE, 0, nullptr, nullptr, &info, &process_info) == 0)
	{
		std::cout << "Failed to start process. Error: " << ::GetLastError() << std::endl;
	}

	PROCESS_INFORMATION process_info_additional_app = { 0 };
	if (cage_data.hasAdditionalAppInfo())
	{
		std::vector<wchar_t> additional_app_path_buf(cage_data.additional_app_path->begin(), cage_data.additional_app_path->end());
		additional_app_path_buf.push_back(0);

		if (::CreateProcess(additional_app_path_buf.data(), nullptr, &security_attributes, nullptr, FALSE, 0, nullptr, nullptr, &info, &process_info_additional_app) == 0)
		{
			std::cout << "Failed to start additional process. Error: " << GetLastError() << std::endl;
		}
	}

	bool keep_cage_running = true;
	std::vector<HANDLE> handles = { labeler_thread.native_handle(), process_info.hProcess };
	if (cage_data.hasAdditionalAppInfo())
	{
		handles.push_back(process_info_additional_app.hProcess);
	}

	// wait for all open window handles on desktop + cage_labeler
	while (keep_cage_running)
	{
		DWORD res = ::WaitForMultipleObjects(handles.size(), handles.data(), FALSE, 500);

		if (res != WAIT_TIMEOUT)
		{
			// this is always the labeler_thread
			if (res == WAIT_OBJECT_0)
			{
				keep_cage_running = false;
			}
			else
			{
				std::cout << "other handle" << std::endl;
				// check all other handles
				for (size_t i = 1; i < handles.size(); ++i)
				{
					if (res == WAIT_OBJECT_0 + i)
					{
						std::cout << "removing" << std::endl;
						// remove the handle we got an event for
						auto iter = handles.begin();
						std::advance(iter, i);
						handles.erase(iter);
					}
				}
			}
		}

		if (!keep_cage_running || (handles.size() < 2 && !cage_data.restrict_closing))
		{
			// labeler still running, tell it to shut down
			if (keep_cage_running)
			{
				auto labeler_hwnd = ::FindWindow(LABELER_WINDOW_CLASS_NAME.c_str(), nullptr);
				if (labeler_hwnd)
				{
					::SendMessage(labeler_hwnd, WM_CLOSE, NULL, NULL);
				}
				else
				{
					::PostThreadMessage(::GetThreadId(labeler_thread.native_handle()), WM_QUIT, NULL, NULL);
				}
			}
			labeler_thread.join();
			break;
		}
	}

	// we can't rely on the process handles to keep track of open processes on
	// the secure desktop as programs (e.g. Internet Explorer) spawn multiple processes and 
	// maybe even close the initial process we spawned ourselves
	// Solution: enumerate all top level windows on the desktop not belonging to our
	// process and message these handles
	std::pair<DWORD, std::unordered_set<HWND>*> callback_window_data;
	std::unordered_set<HWND> window_handles_to_signal;
	callback_window_data.first = ::GetCurrentProcessId();
	callback_window_data.second = &window_handles_to_signal;

	::EnumDesktopWindows(
		desktop_handle,
		&CageManager::GetOpenWindowHandles,
		reinterpret_cast<LPARAM>(&callback_window_data)
	);

	for (HWND hwnd_handle : window_handles_to_signal)
	{
		::SetLastError(0);
		::PostMessage(hwnd_handle, WM_CLOSE, NULL, NULL);
	}

	// and get all open process handles we have to wait for
	std::pair<DWORD, std::unordered_set<HANDLE>*> callback_process_data;
	std::unordered_set<HANDLE> process_handles_for_closing;
	callback_process_data.first = ::GetCurrentProcessId();
	callback_process_data.second = &process_handles_for_closing;

	::EnumDesktopWindows(
		desktop_handle,
		&CageManager::GetOpenProcesses,
		reinterpret_cast<LPARAM>(&callback_process_data)
	);

	// give users up to 5s to react to close prompt of process, maybe increase this?
	if (::WaitForMultipleObjects(
		process_handles_for_closing.size(),
		std::vector(process_handles_for_closing.begin(), process_handles_for_closing.end()).data(),
		TRUE,
		5000) != WAIT_OBJECT_0)
	{
		for (HANDLE process_handle : process_handles_for_closing)
		{
			::SetLastError(0);
			::TerminateProcess(process_handle, 0);
		}
	}

	// close our handles
	::CloseHandle(process_info.hProcess);
	::CloseHandle(process_info.hThread);

	if (cage_data.hasAdditionalAppInfo())
	{
		::CloseHandle(process_info_additional_app.hProcess);
		::CloseHandle(process_info_additional_app.hThread);
	}
}

void CageManager::StartCageLabeler(
	HDESK desktop_handle,
	const CageData &cage_data,
	const int work_area_width,
	const std::wstring &labeler_window_class_name)
{
	::SetThreadDesktop(desktop_handle);
	CageLabeler cage_labeler = CageLabeler(cage_data, work_area_width, labeler_window_class_name);
	cage_labeler.Init();
}

BOOL CALLBACK CageManager::GetOpenProcesses(_In_ HWND hwnd, _In_ LPARAM l_param)
{
	auto data = reinterpret_cast<std::pair<DWORD, std::unordered_set<HANDLE> *> *>(l_param);
	auto current_process_id = data->first;
	auto handles = data->second;

	DWORD process_id;
	::GetWindowThreadProcessId(hwnd, &process_id);

	if (process_id != current_process_id && ::IsWindowVisible(hwnd))
	{
		::SetLastError(0);
		auto handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
		handles->insert(handle);
	}

	return TRUE;
}

BOOL CALLBACK CageManager::GetOpenWindowHandles(_In_ HWND hwnd, _In_ LPARAM l_param)
{
	auto data = reinterpret_cast<std::pair<DWORD, std::unordered_set<HWND> *> *>(l_param);
	auto current_process_id = data->first;
	auto hwnds = data->second;

	::SetLastError(0);
	DWORD process_id;
	::GetWindowThreadProcessId(hwnd, &process_id);

	if (process_id != current_process_id && ::IsWindowVisible(hwnd))
	{
		hwnds->insert(hwnd);
	}

	return TRUE;
}