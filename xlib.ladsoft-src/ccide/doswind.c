#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "header.h"

struct _varData
{
    char *rvTitle; // zero for no RV
    char *rvBody; // zero for no RV
    char *path;
    char *exec;
    char *args;
    
} ;
extern char szInstallPath[];

static char oldPath[32768];
char *GetCmd(void)
{
    char *a;
	a = getenv("COMSPEC");
	if (!a)
		a = searchpath("cmd.exe");
	return a;
}
static LPTCH GetEnv(void)
{
	char buffer2[32768];
	GetEnvironmentVariable("PATH", oldPath, sizeof(oldPath));
	sprintf(buffer2,"%s\\bin;%s", szInstallPath, oldPath);
	SetEnvironmentVariable("PATH", buffer2);
	return GetEnvironmentStrings();
	
}
void FreeEnv(LPTCH env)
{
	FreeEnvironmentStrings(env);
	SetEnvironmentVariable("PATH", oldPath);
}
DWORD CALLBACK DosWindowThread(void *xx)
{
    struct _varData *vdata = (struct _varData *)xx;
	char *xcmd = GetCmd();
	if (xcmd)
	{
		BOOL bSeenInitialBP = FALSE;
		BOOL bContinue = TRUE;
		DWORD dwContinueStatus;
		BOOL bShownExitCode = FALSE;
	    DEBUG_EVENT stDE;
		LPTCH env = GetEnv();
		BOOL bRet;
	    STARTUPINFO stStartInfo;
    	PROCESS_INFORMATION stProcessInfo;
		DWORD exitCode;
		char wd[MAX_PATH], *p, cmd[MAX_PATH];
		if (vdata->path)
		{
			strcpy(wd, vdata->path);
			p = strrchr(wd, '\\');
			if (p)
				if (p[-1] == ':')
					p[1] = 0;
				else
					p[0] = 0;
			else
					wd[0] = 0;
		}
		else 
			GetCurrentDirectory(sizeof(wd), wd);
		if (vdata->exec)
		{
			sprintf(cmd,"\"%s\" %s", vdata->exec, vdata->args);
		}
		else
		{
			sprintf(cmd, "\"%s\"", xcmd);
		}
	    memset(&stStartInfo, 0, sizeof(STARTUPINFO));
	    memset(&stProcessInfo, 0, sizeof(PROCESS_INFORMATION));

	    stStartInfo.cb = sizeof(STARTUPINFO);
	    bRet = CreateProcess(NULL, cmd, NULL, NULL, TRUE, DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS | CREATE_NEW_CONSOLE, env, 
				wd,  &stStartInfo, &stProcessInfo);
	    if (!bRet)
	    {
			FreeEnv(env);
	        ExtendedMessageBox("Command Execution", MB_SETFOREGROUND | MB_SYSTEMMODAL, 
	            "Could not execute %s.", cmd);
	        return 0;
	    }
        ProcessToTop(stProcessInfo.dwProcessId);
	    while (TRUE == bContinue)
	    {
	        // Pause until a debug event notification happens.
	        bContinue = WaitForDebugEvent(&stDE, 500);
	        if (bContinue)
	        {
	            switch (stDE.dwDebugEventCode)
	            {
	                case CREATE_PROCESS_DEBUG_EVENT:
                        CloseHandle(stDE.u.CreateProcessInfo.hFile);
	                    {
	                        dwContinueStatus = DBG_CONTINUE;
	                    }
	                    break;
	                case CREATE_THREAD_DEBUG_EVENT:
	                case LOAD_DLL_DEBUG_EVENT:
	                case UNLOAD_DLL_DEBUG_EVENT:
	                case OUTPUT_DEBUG_STRING_EVENT:
	                case RIP_EVENT:
	                    {
	                        dwContinueStatus = DBG_CONTINUE;
	                    }
	                    break;
					case EXIT_THREAD_DEBUG_EVENT:
	                    {
                            // this catches the X button at the top right of the window.
                            // if it was pressed we do the dialog here to prevent the original window
                            // from auto-closing before we show it.
							if (stDE.u.ExitThread.dwExitCode == STATUS_CONTROL_C_EXIT)
							{
    							if (!bShownExitCode) 
                                {
	    							if (vdata->rvTitle)
		    						{
			    						ProcessToTop(stProcessInfo.dwProcessId);
				    					ExtendedMessageBox(vdata->rvTitle, MB_SETFOREGROUND | MB_SYSTEMMODAL, vdata->rvBody, stDE.u.ExitProcess.dwExitCode);
					    				bShownExitCode = TRUE;
						    		}
                                }
							}
	                        dwContinueStatus = DBG_CONTINUE;
	                    }
						break;
	                case EXIT_PROCESS_DEBUG_EVENT:
	                    {
							
							if (!bShownExitCode) 
							{
								if (vdata->rvTitle)
								{
									ProcessToTop(stProcessInfo.dwProcessId);
									ExtendedMessageBox(vdata->rvTitle, MB_SETFOREGROUND | MB_SYSTEMMODAL, vdata->rvBody, stDE.u.ExitProcess.dwExitCode);
									bShownExitCode = TRUE;
								}
							}
	                        dwContinueStatus = DBG_CONTINUE;
							bContinue = FALSE;
	                    }
	                    break;
	
	                case EXCEPTION_DEBUG_EVENT:
	                    {
	                        switch (stDE.u.Exception.ExceptionRecord.ExceptionCode)
	                        {
								default:
			                        dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
									break;
		                        case EXCEPTION_BREAKPOINT:
									if (!bSeenInitialBP)
									{
	                                    bSeenInitialBP = TRUE;									
	                                    dwContinueStatus = DBG_CONTINUE;
	                                }
									else
									{
				                        dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
									}
									break;
							}
	                    }
	                    break;
	
	                    // For any other events, just continue on.
	                default:
	                    {
	                        dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
	                    }
	                    break;
	            }
	        }
	        else
	        {
	            dwContinueStatus = DBG_CONTINUE;
	            bContinue = TRUE;
			}
	        // Pass on to the operating system.
	        ContinueDebugEvent(stDE.dwProcessId, stDE.dwThreadId, dwContinueStatus);
	
	    }
		CloseHandle(stProcessInfo.hProcess);
		CloseHandle(stProcessInfo.hThread);
		FreeEnv(env);
	}
    free(vdata->rvTitle);
    free(vdata->rvBody);
    free(vdata->path);
    free(vdata->exec);
    free(vdata->args);
    free(vdata);
	return 0;
}
void DosWindow(char *path, char *exec, char *args, char *rvTitle, char *rvBody)
{
    struct _varData *vdata = calloc(sizeof(struct _varData), 1);
    if (vdata)
    {
        DWORD id;
        if (path)
            vdata->path = strdup(path);
        if (exec)
            vdata->exec = strdup(exec);
        if (args)
            vdata->args = strdup(args);
        if (rvTitle)
            vdata->rvTitle = strdup(rvTitle);
        if (rvBody)
            vdata->rvBody = strdup(rvBody);
    	CloseHandle(CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DosWindowThread, (VOID *)vdata, 0, &id));
    }    
}