#include <iostream>
#include <windows.h>



BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup 
        &luid))        // receives LUID of privilege
    {
        printf("LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.

    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL))
    {
        printf("AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

    {
        printf("The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;
}



int main()
{
    //From https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-getfirmwareenvironmentvariablea
    /*
    To read a firmware environment variable,
    the user account that the app is running under must have the SE_SYSTEM_ENVIRONMENT_NAME privilege. 
    
    */
    LPCTSTR lpszPrivilege = SE_SYSTEM_ENVIRONMENT_NAME;

    // Change this BOOL value to set/unset the SE_PRIVILEGE_ENABLED attribute
    BOOL bEnablePrivilege = TRUE;
    HANDLE hToken;
    // Open a handle to the access token for the calling process. That is this running program
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        printf("OpenProcessToken() error %u\n", GetLastError());
        return FALSE;
    }
    BOOL test = SetPrivilege(hToken, lpszPrivilege, bEnablePrivilege);
    uint8_t buf[16] = { 1 };
    wchar_t str[256];
    UINT Status;
    char Var_GUID[] = "{6ACCE65D-DA35-4B39-B64B-5ED927A7DC7E}";
    char Var_name[] = "cE!";
    //Check if exist
    Status = GetFirmwareEnvironmentVariableA(Var_name, Var_GUID, &buf, 1);
    //Remove if Exist
    if (Status != 0)
    {
        printf("Locking\n");
        Status = SetFirmwareEnvironmentVariableA(Var_name, Var_GUID, &buf, 0);
    }
    else
    {
        //Create If not exist
        buf[0] = 1;
        printf("Unlocking\n");
        Status= SetFirmwareEnvironmentVariableA(Var_name, Var_GUID, &buf, 1);
    }
    if (Status==0)
    {
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            str, (sizeof(str) / sizeof(wchar_t)), NULL);
        std::wcout << str << "\n";
    }
    else
    {
        printf("Success\n");
    }
    
    
    //Remove the SE_SYSTEM_ENVIRONMENT_NAME Privileges
    bEnablePrivilege = FALSE;
    BOOL test1 = SetPrivilege(hToken, lpszPrivilege, bEnablePrivilege);
    system("pause");
}

