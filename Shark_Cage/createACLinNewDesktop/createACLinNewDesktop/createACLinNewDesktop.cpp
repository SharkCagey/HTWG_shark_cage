// createACLinNewDesktop.cpp: define el punto de entrada de la aplicación de consola.
//

#include "stdafx.h"
#include "Windows.h"
#include "stdio.h"
#include "Aclapi.h"
#include "tchar.h"
#include "sddl.h"

int main()
{
	DWORD dwRes;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	//EXPLICIT_ACCESS ea[1];
	SECURITY_ATTRIBUTES sa;
	HDESK newDesktop = NULL;


	// create sid for BUILTIN\System group
	PSID sid_system = NULL;
	SID_IDENTIFIER_AUTHORITY sid_authsystem = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&sid_authsystem, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &sid_system)) {
		DWORD err = GetLastError();
		//free(group_sid);
		return FALSE;
	}

	// create SID for BUILTIN\Administrators group
	PSID sid_admin = NULL;
	SID_IDENTIFIER_AUTHORITY sid_authnt = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&sid_authnt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &sid_admin)) {
		DWORD err = GetLastError();
		//free(group_sid);
		LocalFree(sid_system);
		return FALSE;
	}

	// create SID for BUILTIN\Administrators group
	PSID sid_user = NULL;
	SID_IDENTIFIER_AUTHORITY sid_authuser = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&sid_authuser, 1, SE_GROUP_OWNER, 0, 0, 0, 0, 0, 0, 0, &sid_user)) {
		DWORD err = GetLastError();
		//free(group_sid);
		LocalFree(sid_system);
		LocalFree(sid_admin);
		return FALSE;
	}


	// create EXPLICIT_ACCESS structure for an ACE
	EXPLICIT_ACCESS ea[2];
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));

	/*PSID mysid = NULL;
	ConvertStringSidToSid(TEXT("CURRENT_USER"), &mysid); */

	ea[0].grfAccessPermissions = GENERIC_ALL;	// access rights for this entity
	ea[0].grfAccessMode = SET_ACCESS;			// what this entity shall do: set rights, remove them, ...
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	//ea[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
	//ea[0].Trustee.ptstrName = (LPTSTR)sid_system;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)sid_system;
	// fill EXPLICIT_ACCESS with second ACE for admin group
	ea[1].grfAccessPermissions = GENERIC_ALL;
	ea[1].grfAccessMode = SET_ACCESS; //DENY_ACCES
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = (LPTSTR)sid_admin;

	/*
	ea[2].grfAccessPermissions = GENERIC_ALL;
	ea[2].grfAccessMode = SET_ACCESS; //DENY_ACCES
	ea[2].grfInheritance = NO_INHERITANCE;
	ea[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[2].Trustee.TrusteeType = TRUSTEE_IS_USER;
	ea[2].Trustee.ptstrName = (LPTSTR)sid_user; */


	// Create a new ACL that contains the new ACEs.
	dwRes = SetEntriesInAcl(2, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes)
	{
		_tprintf(_T("SetEntriesInAcl Error %u\n"), GetLastError());
		goto Cleanup;
	}

	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (NULL == pSD)
	{
		_tprintf(_T("LocalAlloc Error %u\n"), GetLastError());
		goto Cleanup;
	}

	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION))
	{
		_tprintf(_T("InitializeSecurityDescriptor Error %u\n"),
			GetLastError());
		goto Cleanup;
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		_tprintf(_T("SetSecurityDescriptorDacl Error %u\n"),
			GetLastError());
		goto Cleanup;
	}

	// Initialize a security attributes structure.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	////////////

	//SAVE THE OLD DESKTOP. This is in order to come back to our desktop.
	HDESK oldDesktop = GetThreadDesktop(GetCurrentThreadId());

	////////////

	// Use the security attributes to set the security descriptor 
	// when you create a desktop.
	ACCESS_MASK desk_access_mask = DESKTOP_CREATEMENU | DESKTOP_CREATEWINDOW | DESKTOP_ENUMERATE | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALPLAYBACK | DESKTOP_JOURNALRECORD | DESKTOP_READOBJECTS | DESKTOP_SWITCHDESKTOP | DESKTOP_WRITEOBJECTS | READ_CONTROL | WRITE_DAC | WRITE_OWNER;
	newDesktop = CreateDesktop(TEXT("DesktopName"), NULL, NULL, NULL, desk_access_mask, &sa);

	////////////////////////////////////////////////////////////////////////////////

	if (newDesktop != NULL) {

		//Switch to de new desktop.
		SwitchDesktop(newDesktop);

		//The path where is the application that we are going to start in the new desktop.
		LPTSTR path = _tcsdup(TEXT("C:\\Program Files\\Notepad++\\notepad++.exe"));
		//LPTSTR path = _tcsdup(TEXT("C:\\Windows\\System32\\cmd.exe"));

		//We need in order to create the process.
		STARTUPINFO info = { sizeof(info) };
		PROCESS_INFORMATION processInfo;

		//The desktop's name where we are going to start the application. In this case, our new desktop.
		LPTSTR desktop = _tcsdup(TEXT("DesktopName"));
		info.lpDesktop = desktop;

		


		//PETER, HERE IS WHERE YOU HAVE TO WRITE YOUR CODE





		//Create the process.
		if (!CreateProcess(NULL, path, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
		{
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			//Handle error here.

		}

		Sleep(5000);
		//SWITCH TO THE OLD DESKTOP. This is in order to come back to our desktop.
		SwitchDesktop(oldDesktop);

	}
	else {
		//Handle error here.
	}

	///////////////////////////////////////////////////////////////////////////////

Cleanup:

	if (sid_system)
		FreeSid(sid_system);
	if (sid_admin)
		FreeSid(sid_admin);
	if (sid_user)
		FreeSid(sid_user);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);

	return 0;

}

