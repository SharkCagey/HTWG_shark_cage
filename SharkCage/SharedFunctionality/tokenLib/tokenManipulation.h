#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

#include <Windows.h>


namespace tokenLib {
	/**
	* Function gets a SID of a group and creates a token with a group entry added in the token
	* Source of the token is the token of the process itself. Returned token is identical to the calling process token, just includes one more group
	* SE_CREATE_TOKEN_NAME must be held and enabled by a calling process, to successfully call this method
	* Mind that there are security implications to the current implementations - such that f.x.:
	* every process launched with token created by this function has SE_CREATE_TOKEN_NAME privilege, granting it basically any access to the system
	* This can be mitigated by calling CreateRestrictedToken() function on the output of this method
	* @param sid pointer to sid to be added to the token (IN)
	* @param token reference to handle to requested token (OUT)
	* @return true if success
	**/
	DLLEXPORT bool constructUserTokenWithGroup(PSID sid, HANDLE &token);

	/**
	* Function gets a name of a group and creates a token with a group entry added in the token. The group must exist, otherwise the function will fail
	* The group will not be deleted at return, otherwise the token would be useless.
	* Source of the token is the token of the process itself. Returned token is identical to the calling process token, just includes one more group
	* SE_CREATE_TOKEN_NAME must be held and enabled by a calling process, to successfully call this method
	* Mind that there are security implications to the current implementations - such that f.x.:
	* every process launched with token created by this function has SE_CREATE_TOKEN_NAME privilege, granting it basically any access to the system
	* This can be mitigated by calling CreateRestrictedToken() function on the output of this method
	* @param groupName string literal representing the name of nonexistent group to be added to the token (IN)
	* @param token reference to handle to requested token (OUT)
	* @return true if success
	**/
	DLLEXPORT bool constructUserTokenWithGroup(LPWSTR groupName, HANDLE &token);

	//alternative token sourcing: 
	//another approach would be to use wtsQueryUserToken and determine session of current user
	//(this would require to run under a local system and have SE_CREATE_TOKEN_NAME at the same time  - which is suprisingly hard to achieve)
	//one more alternative is just to outsource the token aqusition and just take a handle to the template token as an input parameter


	/**
	* Functions findes a process running under LocalSystem with SE_CREATE_TOKEN_NAME and SE_TCB_NAME present in its token, gets this token duplicates it and returns a handle
	* @param token handle to new token having SeCreateTokenPrivilege
	* @return true if success
	**/
	DLLEXPORT bool aquireTokenWithPrivilegesForTokenManipulation(HANDLE &token);
}