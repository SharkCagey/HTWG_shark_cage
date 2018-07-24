#pragma once

class CageManager
{
public:
	void CageManager::StartCage(SECURITY_ATTRIBUTES security_attributes, const CageData &cage_data);

private:
	void StartCageLabeler(
		HDESK desktop_handle,
		const CageData &cage_data,
		const int work_area_width,
		const std::wstring &labeler_window_class_name);

	// FIXME: extra class for this? process handling? -> could also do the wait stuff
	static BOOL CALLBACK GetOpenWindowHandles(_In_ HWND hwnd, _In_ LPARAM l_param);
	static BOOL CALLBACK GetOpenProcesses(_In_ HWND hwnd, _In_ LPARAM l_param);
};