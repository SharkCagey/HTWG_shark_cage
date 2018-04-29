#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable:4996)

#include "../Cage service/NetworkManager.h"
#include "../Cage service/MSG_to_manager.h"

#include <Windows.h>
#include "stdio.h"
#include "Aclapi.h"
#include <tchar.h>
#include "sddl.h"
#include <string>
#include <LMaccess.h>
#include <lmerr.h>
#pragma comment(lib, "netapi32.lib")



PSID createSID();
bool createACL(PSID groupSid);
std::string onReceive(std::string message);
bool beginsWith(const std::string string, const std::string prefix);
BOOL IsProcessRunning(HANDLE process);

NetworkManager networkMgr(MANAGER);

int main() {
	PSID groupSid = createSID();
	return createACL(groupSid);
	// createProcessToken()
}

PSID createSID() {

	LPWSTR  group_name = L"Shark_cage_group";
	LOCALGROUP_INFO_0 localgroup_info;
	LPTSTR   pUser_name = NULL;
	HANDLE current_process_handle;
	HANDLE user_token_h;
	DWORD bufferSize = 0;

	//create a group
	localgroup_info.lgrpi0_name = group_name;
	NetLocalGroupAdd(NULL, 0, (LPBYTE)&localgroup_info, NULL);

	//obtain sid
	const DWORD INITIAL_SIZE = 32;
	DWORD cbSid = 0;
	WCHAR * wszDomainName = NULL;
	DWORD dwDomainBufferSize = INITIAL_SIZE;
	DWORD cchDomainName = 0;
	SID_NAME_USE eSidType;
	DWORD dwSidBufferSize = INITIAL_SIZE;
	PSID ppSid;
	wszDomainName = new WCHAR[dwDomainBufferSize];
	cchDomainName = dwDomainBufferSize;

	// First call of the function in order to get the size needed for the SID
	LookupAccountName(
		NULL,            // Computer name. NULL for the local computer  
		group_name,
		NULL,
		&cbSid,
		wszDomainName,
		&cchDomainName,
		&eSidType
	);

	ppSid = (PSID) new BYTE[cbSid];

	// Second call of the function in order to get the SID
	LookupAccountName(
		NULL,
		group_name,
		ppSid,
		&cbSid,
		wszDomainName,
		&cchDomainName,
		&eSidType
	);
	/*LPTSTR StringSid;
	ConvertSidToStringSid(
	ppSid,
	&StringSid
	);
	ppSid;
	StringSid;*/

	return &ppSid;
}


bool beginsWith(const std::string string, const std::string prefix) {
	if (prefix.length() > string.length()) {
		return false;
		// Throw Exception "Bad parameters: prefix longer than the actual string"
	}
	else {
		if (string.compare(0, prefix.length(), prefix) == 0) {
			return true;
		}
		else {
			return false;
		}
	}
}

// Function to decode the message and do a respective action
// "START_PC" "path/to.exe"
std::string onReceive(std::string message) {
	std::string path = "";
	if (beginsWith(message, MSG_TO_MANAGER_toString(MGR_START_PC))) {
		// Start process
		path = message.substr(9);
	}
	else if (beginsWith(message, MSG_TO_MANAGER_toString(MGR_STOP_PC))) {
		// Stop process
	}
	else {
		// Unrecognized message - Do nothing
	}
	return path;
}

std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}


bool createACL(PSID groupSid) {
	DWORD dwRes;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	SECURITY_ATTRIBUTES sa;
	HDESK newDesktop = NULL;


	//Listen for the message
	std::string message = networkMgr.listen();
	std::string path = "";
	path = onReceive(message);
	//path = "C:\\Program Files\\Notepad++\\notepad++.exe";
	// create SID for BUILTIN\Administrators group
	PSID sid_admin = NULL;
	SID_IDENTIFIER_AUTHORITY sid_authnt = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&sid_authnt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &sid_admin)) {
		_tprintf(_T("Obtain Admin SID Error %u\n"), GetLastError());
		//free(group_sid);
		return FALSE;
	}

	// create EXPLICIT_ACCESS structure for an ACE
	EXPLICIT_ACCESS ea[2];
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

	// EXPLICIT_ACCESS for created group
	ea[0].grfAccessPermissions = GENERIC_ALL;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)groupSid;
	// EXPLICIT_ACCESS with second ACE for admin group
	ea[1].grfAccessPermissions = GENERIC_ALL;
	ea[1].grfAccessMode = SET_ACCESS; //DENY_ACCES
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)sid_admin;

	std::wstring widePath = s2ws(path);
	STARTUPINFO info = { sizeof(info) };
	std::vector<wchar_t> vec(widePath.begin(), widePath.end());

	// Create a new ACL that contains the new ACEs.
	dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes)
	{
		_tprintf(_T("SetEntriesInAcl Error %u\n"), GetLastError());
		//goto Cleanup;
	}

	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (NULL == pSD)
	{
		_tprintf(_T("LocalAlloc Error %u\n"), GetLastError());
		//goto Cleanup;
	}

	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION))
	{
		_tprintf(_T("InitializeSecurityDescriptor Error %u\n"),
			GetLastError());
		//goto Cleanup;
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		_tprintf(_T("SetSecurityDescriptorDacl Error %u\n"),
			GetLastError());
		//goto Cleanup;
	}

	// Initialize a security attributes structure.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	//SAVE THE OLD DESKTOP. This is in order to come back to our desktop.
	HDESK oldDesktop = GetThreadDesktop(GetCurrentThreadId());

	// Use the security attributes to set the security descriptor 
	// when you create a desktop.
	ACCESS_MASK desk_access_mask = DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW | DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALPLAYBACK | DESKTOP_JOURNALRECORD | DESKTOP_READOBJECTS | DESKTOP_SWITCHDESKTOP | DESKTOP_WRITEOBJECTS | READ_CONTROL | WRITE_DAC | WRITE_OWNER;
	newDesktop = CreateDesktop(TEXT("SharkCageDesktop"), NULL, NULL, NULL, desk_access_mask, &sa);

	//Switch to de new desktop.
	SwitchDesktop(newDesktop);

	//We need in order to create the process.
	PROCESS_INFORMATION processInfo;

	//The desktop's name where we are going to start the application. In this case, our new desktop.
	LPTSTR desktop = _tcsdup(TEXT("SharkCageDesktop"));
	info.lpDesktop = desktop;
	vec.push_back(L'\0');

	//PETER´S ACCESS TOKEN THINGS





	//Create the process.
	CreateProcess(NULL, &vec[0], NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);


	while (IsProcessRunning(processInfo.hProcess)) {
		//printf("PROCESS RUNNING\n");
	}

	//Sleep(500);

	//SWITCH TO THE OLD DESKTOP. This is in order to come back to our desktop.
	SwitchDesktop(oldDesktop);

	return 0;
}

BOOL IsProcessRunning(HANDLE process)
{
	DWORD ret = WaitForSingleObject(process, 0);
	return (ret == WAIT_TIMEOUT);
}
