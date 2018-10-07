/*
#include "handlesInfo.cpp"   //THREAD_STATE_RUNNING
#include "Chart.cpp"
#include "Performance.cpp"
#include "shutdown.cpp"
#include "Extras_Handles.cpp"
#include "Processes.cpp"
#include "SrvcGroups.cpp"
#include "SrvcProperties.cpp"
#include "ServicesRemote.cpp"
#include "Services.cpp"
#include "StartupRun.cpp"
#include "CLSIDinfo.cpp"
#include "WinStyles.cpp"
#include "Windows.cpp"
#include "Drivers.cpp"
#include "procInfo.cpp"
#include "SelectColumns.cpp"
#include "Settings.cpp"
#include "About.cpp"
#include "MenuStyle.cpp"
 #include "header.h"
 #include "stringf.cpp"
 #include "menus.h"
 #include "dialogs.h"
*/

HINSTANCE hVdmDbgLib,hWinStaLib; SYSTEMTIME stm; FILETIME ftm;
char _K[3]=" K\0";

void Process_DestroyData(PROCESS_INFO *pinf){PROCESS_INFO *ppinf; DWORD_PTR dwp1,dwp2; DWORD dw; int in;
 if(!IsVALID_PROCESS_INFO(pinf))return;
 ppinf=pinf->NextEntry;
 if(pinf==plastinf)plastinf=ppinf;
 else if(pinf==fpinfo)fpinfo=ppinf;
 if(IsVALID_PROCESS_INFO(ppinf))ppinf->PrevEntry=pinf->PrevEntry;
 if(IsVALID_PROCESS_INFO((ppinf=pinf->PrevEntry)))ppinf->NextEntry=pinf->NextEntry;
 pinf->cbSize=0;
 dwp1=(DWORD_PTR)&(pinf->Description);
 dwp2=(DWORD_PTR)&(pinf->PriorityText);
 for(;dwp1<=dwp2;dwp1+=sizeof(DWORD_PTR)){
  if(*(DWORD_PTR*)dwp1){
   dw=*(DWORD_PTR*)dwp1>*(DWORD_PTR*)smemTable;
   dw+=*(DWORD_PTR*)dwp1<(DWORD_PTR)Menus[0];
   if(dw!=2)LocalFree((void*)(*(DWORD_PTR*)dwp1));//+ PROCESS_THREAD_INFO *Threads;
  }
 }
 // Remove Icon from ImageList
 in=pinf->iIcon;
 if(in>1){//0=Application, 1=Idle
  ImageList_Remove(himgl,in);
  ppinf=fpinfo;
  while(IsVALID_PROCESS_INFO(ppinf)){
   if(ppinf->iIcon>in)ppinf->iIcon--;
   ppinf=ppinf->NextEntry;
  }
 }
 // Remove Unicode Strings
 __try{
  if(pinf->ImagePathName.Buffer && pinf->ImagePathName.Length>0)LocalFree(pinf->ImagePathName.Buffer);
  if(pinf->ProcessName.Buffer && pinf->ProcessName.Length>0)LocalFree(pinf->ProcessName.Buffer);//Here is EXCEPTION ERROR sometimes !!!
  LocalFree(pinf);
 }
 __except((n=__exception_code)!=0){
  Try_Except_Error(n);
  goto _bbreak;
 }
_bbreak:
}

BOOL Process_GetMachineUserName(DWORD ProcessID,LARGE_INTEGER *pCreateTime,void *OutputBuf,void *nbuf2,void *nbuf3,BOOL *pbIsCurrentUser){
 DWORD uu,nm,dwRet,dwLen; SID_NAME_USE snu; HANDLE hProcess; BOOL nb2=0,nb3=0;
 if(OutputBuf==0)return 0;
 *(DWORD*)OutputBuf=0;
 if(nbuf2==0){nbuf2=(void*)LocalAlloc(LPTR,1024);nb2=1;}
 if(nbuf3==0){nbuf3=(void*)LocalAlloc(LPTR,1024);nb3=1;}
 if(pbIsCurrentUser){*pbIsCurrentUser=0;}
winsta_getPid:
 if(WinStationGetProcessSid){
  nm=1024;
  if(pCreateTime && WinStationGetProcessSid(0,ProcessID,pCreateTime->LowPart,pCreateTime->HighPart,(SID*)nbuf2,&nm)==1){
   uu=1024; nm=1024;
   if(LookupAccountSid(0,(SID*)nbuf2,(LPTSTR)nbuf3,&uu,(LPTSTR)OutputBuf,&nm,&snu)!=0){
    *((BYTE*)OutputBuf+nm)='\\'; nm++; *((BYTE*)OutputBuf+nm)=0;
    strappend((LPTSTR)OutputBuf,(LPTSTR)nbuf3);
    dwRet=GETMACHINEUSER_ERROR_SUCCESS;
    dwLen=nm+uu;
   }
   goto exit;
  }
 }
 else if(hWinStaLib!=(HINSTANCE)-1){
  if(hWinStaLib==NULL)hWinStaLib=LoadLibrary(winstadll_txt);
  if(hWinStaLib!=NULL){
   WinStationGetProcessSid=(DWORD(_stdcall*)(DWORD,IN DWORD,DWORD,DWORD,PSID,DWORD*))GetProcAddress(hWinStaLib,WinStationGetProcessSid_txt);
   if(WinStationGetProcessSid==NULL){FreeLibrary(hWinStaLib); hWinStaLib=(HINSTANCE)-1;}
   else goto winsta_getPid;
  }
 }
 if(*(DWORD*)OutputBuf!=0)goto exit;
 if((hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,0,ProcessID))!=0){
  HANDLE hToken; hToken=0;
  if(OpenProcessToken(hProcess,TOKEN_QUERY,&hToken)){
   if(GetTokenInformation(hToken,TokenUser,OutputBuf,2048,&uu)){
    nm=2048; uu=2048;
    if(LookupAccountSid(0,((TOKEN_USER*)OutputBuf)->User.Sid,(LPTSTR)nbuf3,&uu,(LPTSTR)OutputBuf,&nm,&snu)!=0){
     strappend((LPTSTR)OutputBuf,"\\"); strappend((LPTSTR)OutputBuf,(LPTSTR)nbuf3);
     dwRet=GETMACHINEUSER_ERROR_SUCCESS;
     dwLen=nm+uu;
    }
    else {dwRet=GETMACHINEUSER_ERROR_UNKNOWNUSER; copystring((LPTSTR)OutputBuf,smemTable->unknown);}
   }
   else {dwRet=GETMACHINEUSER_ERROR_UNBLQUERYOWNER; copystring((LPTSTR)OutputBuf,smemTable->UnblQueryOwner);}
   CloseHandle(hToken);
  }
  else {dwRet=GETMACHINEUSER_ERROR_UNBLOPENTOKEN; copystring((LPTSTR)OutputBuf,smemTable->UnblOpentoken);}
  CloseHandle(hProcess);
 }
 else {
  if(ProcessID==0)copystring((LPTSTR)OutputBuf,NTAUTHORITY_SYSTEM_txt);
  else {dwRet=GETMACHINEUSER_ERROR_UNBLOPENPROCESS; copystring((LPTSTR)OutputBuf,smemTable->access_denied);}
 }
exit:
 if(nb2 && nbuf2)LocalFree(nbuf2);
 if(nb3 && nbuf3)LocalFree(nbuf3);
 if(dwRet==GETMACHINEUSER_ERROR_SUCCESS && pbIsCurrentUser && dwLen==mainSetts.dwCurrentUserNameSize && mainSetts.pCurrentUserName!=0){
  if(lstrcmp((LPTSTR)OutputBuf,mainSetts.pCurrentUserName)==0){*pbIsCurrentUser=1;}
 }
 return (dwRet==GETMACHINEUSER_ERROR_SUCCESS);
}

BOOL Process_Terminate(DWORD processID,BOOL bShowFailMsg){DWORD ErrCode=0; HANDLE hProcess;
 if((hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,processID))!=0){
  ErrCode=ntdllFunctions.pNtTerminateProcess(hProcess,0);
  CloseHandle(hProcess);
  if(ErrCode!=0 && bShowFailMsg)FailMessage(smemTable->UnblTrmntProcs,ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
 }
 else if(bShowFailMsg)FailMessage(smemTable->UnblTrmntProcs,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
 return (ErrCode==0);
}

void Process_Restart(DWORD dwPid){HANDLE hProcess; PROCESS_BASIC_INFORMATION pbi; PEB peb; NTSTATUS ntStatus; DWORD dw,nm; LPWSTR lpCurDir=0,lpImgPath=0,lpCmdLine=0,lpwstr;
 RTL_USER_PROCESS_PARAMETERS *pRtlupp=0; PROCESS_INFO *pinf; BOOL bPinfFound=0,bValidUserName=0,bCurrentUser=0,bCritical=0; int in; CREDUI_INFOW cdui;
 LPTSTR lpstr,lpptr;
 pinf=fpinfo;
 while(IsVALID_PROCESS_INFO(pinf)){
  if(pinf->Id==dwPid && pinf->dwStatus<2){bPinfFound=1; break;}
  pinf=pinf->NextEntry;
 }
 if(bPinfFound){
  if((pinf->Flags&PINF_FLAG_CURRENT_USER)!=PINF_FLAG_CURRENT_USER){
   if(pinf->UserName!=0){
    dw=(DWORD_PTR)pinf->UserName>*(DWORD_PTR*)smemTable;
    dw+=(DWORD_PTR)pinf->UserName<(DWORD_PTR)Menus[0];
    if(dw!=2)bValidUserName=1;
   }
  }
  else bCurrentUser=1;
 }
 if((hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwPid))!=0){
  if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessCriticalInformation,&dw,sizeof(DWORD),&nm)==0){
   bCritical=(BOOL)dw;
  }
  CloseHandle(hProcess);
  hProcess=0;
 }
 // ask
 dw=getstrlen(smemTable->AreYouSureToRestartProcess);
 if(bCritical)dw+=getstrlen(smemTable->ThisIsCriticalProcess);
 if(!bCurrentUser && bValidUserName)dw+=getstrlen(smemTable->ProcRunnedUnderAnotherUser);
 if((lpstr=(LPTSTR)LocalAlloc(LMEM_FIXED,dw+5))==0)goto exit;
 lpptr=lpstr;
 if(bCritical)lpptr+=copystring(lpptr,smemTable->ThisIsCriticalProcess);
 if(!bCurrentUser && bValidUserName)lpptr+=copystring(lpptr,smemTable->ProcRunnedUnderAnotherUser);
 copystring(lpptr,smemTable->AreYouSureToRestartProcess);
 dw=MessageBox(processes_win,lpstr,progTitle,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1);
 LocalFree(lpstr);
 if(dw!=IDYES)goto exit;
 //
 if((hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_TERMINATE,FALSE,dwPid))==0){
  FailMessage(smemTable->UnblOpenProcess,0,FMSG_SHOW_MSGBOX|FMSG_CallGLE|FMSG_NO_SUCCESSINFO|FMSG_ICONSTOP|FMSG_SMART_SHOWCODE);
  return;
 }
 if((ntStatus=ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessBasicInformation,&pbi,sizeof(pbi),&nm))==STATUS_SUCCESS){
  if(pbi.PebBaseAddress==0)goto set_readmem_err;
  if(ReadProcessMemory(hProcess,pbi.PebBaseAddress,&peb,sizeof(PEB),0)==0)goto set_readmem_err;
  if(peb.ProcessParameters==0)goto set_readmem_err;
  //if(ReadProcessMemory(hProcess,peb.ProcessParameters,&dw,sizeof(DWORD),0)==0)goto set_readmem_err;
  if((pRtlupp=(RTL_USER_PROCESS_PARAMETERS*)LocalAlloc(LMEM_FIXED,sizeof(RTL_USER_PROCESS_PARAMETERS)))==0)goto set_readmem_err;
  if(ReadProcessMemory(hProcess,peb.ProcessParameters,pRtlupp,sizeof(RTL_USER_PROCESS_PARAMETERS),0)==0)goto set_readmem_err;
  // Get Current Dir Path
  dw=pRtlupp->CurrentDirectoryPath.Length;
  if((lpCurDir=(LPWSTR)LocalAlloc(LMEM_FIXED,dw+(sizeof(WCHAR)*2)))==0)goto set_readmem_err;
  if(ReadProcessMemory(hProcess,pRtlupp->CurrentDirectoryPath.Buffer,lpCurDir,dw,0)==0)goto set_readmem_err;
  dw>>=1; *((LPWSTR*)(lpCurDir+dw))=0;
  // Get Image Path
  dw=pRtlupp->ImagePathName.Length;
  if((lpImgPath=(LPWSTR)LocalAlloc(LMEM_FIXED,dw+(sizeof(WCHAR)*2)))==0)goto set_readmem_err;
  if(ReadProcessMemory(hProcess,pRtlupp->ImagePathName.Buffer,lpImgPath,dw,0)==0)goto set_readmem_err;
  dw>>=1; *((LPWSTR*)(lpImgPath+dw))=0;
  // Get Command Line
  dw=pRtlupp->CommandLine.Length;
  if((lpCmdLine=(LPWSTR)LocalAlloc(LMEM_FIXED,dw+(sizeof(WCHAR)*2)))==0)goto set_readmem_err;
  if(ReadProcessMemory(hProcess,pRtlupp->CommandLine.Buffer,lpCmdLine,dw,0)==0)goto set_readmem_err;
  dw>>=1; *((LPWSTR*)(lpCmdLine+dw))=0;
  lpwstr=lpCmdLine;
  lpCmdLine=CmdLine_CreateString(lpCmdLine,0);
  LocalFree(lpwstr);
  // Terminate the Process
  if(Process_Terminate(dwPid,1)){
   if(bCurrentUser){
    // Run as Current User
    in=(int)ShellExecuteW(NULL,open_txtW,lpImgPath,lpCmdLine,lpCurDir,SW_SHOWNORMAL);
    if(in<=32){
     LPTSTR lpstr;
     FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER,0,ntdllFunctions.RtlGetLastWin32Error(),0,(LPTSTR)&lpstr,128,0);
     MessageBox(run_win,lpstr,ERROR_txt,MB_OK|MB_ICONSTOP);
     LocalFree(lpstr);
     goto exit;
    }
   }
   else{// Run as Another User
    LPWSTR lpUserName,lpUser,lpDomain,lpUserPsw,lpwstr2;
    lpUserName=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_USERNAME_LENGTH+1)*2);
    lpUser=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_USERNAME_LENGTH+1)*2);
    lpDomain=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_DOMAIN_TARGET_LENGTH+1)*2);
    lpUserPsw=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_PASSWORD_LENGTH+1)*2);
    // Load Credui.Dll
    HINSTANCE hCreduiLib=0;
    if(pCredUIPromptForCredentialsW==0 || pCredUIParseUserNameW==0){
     if((hCreduiLib=LoadLibrary(Creduidll_txt))!=0){
      pCredUIPromptForCredentialsW=(DWORD(WINAPI*)(PCREDUI_INFOW,PCWSTR,PCtxtHandle,DWORD,PWSTR,ULONG,PWSTR,ULONG,BOOL*,DWORD))GetProcAddress(hCreduiLib,"CredUIPromptForCredentialsW");
      pCredUIParseUserNameW=(DWORD(WINAPI*)(PCWSTR,PWSTR,ULONG,PWSTR,ULONG))GetProcAddress(hCreduiLib,"CredUIParseUserNameW");
     }
     if(pCredUIPromptForCredentialsW==0 || pCredUIParseUserNameW==0)goto free_user_mem;
    }
    // User name is known
    if(bValidUserName){
     nm=getstrlen(pinf->UserName);
     dw=(nm+1)*2;
     if((lpwstr=(LPWSTR)LocalAlloc(LMEM_FIXED,dw))==0)goto free_user_mem;
     STRING str; UNICODE_STRING ustr;
     ustr.Length=0; ustr.MaximumLength=(USHORT)dw; ustr.Buffer=lpwstr; *lpwstr=0;
     str.Length=(USHORT)nm; str.MaximumLength=str.Length; str.Buffer=pinf->UserName;
     if(!IsVALID_PROCESS_INFO(pinf))goto free_user_mem;// ensure pinf is valid yet.
     if(ntdllFunctions.RtlAnsiStringToUnicodeString(&ustr,&str,0)!=STATUS_SUCCESS)goto free_user_mem;
     *(ustr.Buffer+ustr.Length)=0;
     // copy domain name
     dw=0; lpwstr2=lpDomain;
     while(*ustr.Buffer!=0 && *ustr.Buffer!=L'\\'){
      if(dw<CREDUI_MAX_DOMAIN_TARGET_LENGTH){
       *lpwstr2=*ustr.Buffer; lpwstr2++;
       dw++;
      }
      ustr.Buffer++;
     }
     // copy user name
     dw=0; lpwstr2=lpUserName; ustr.Buffer++;
     while(*ustr.Buffer!=0 && *ustr.Buffer!=L'\\'){
      if(dw<CREDUI_MAX_USERNAME_LENGTH){
       *lpwstr2=*ustr.Buffer; lpwstr2++;
       dw++;
      }
      ustr.Buffer++;
     }
     //
     LocalFree(lpwstr); lpwstr=0;
    }
    else {// User name is unknown. Get this computer's name as domain name
     lpwstr=0;
     dw=CREDUI_MAX_DOMAIN_TARGET_LENGTH+1;
     GetComputerNameExW(ComputerNamePhysicalNetBIOS,lpDomain,&dw);
    }
    // Prompt for Password
    BOOL fSave;
 prompt_userdata:
    cdui.cbSize=sizeof(CREDUI_INFOW);
    cdui.hwndParent=main_win;
    cdui.pszMessageText=0;
    cdui.pszCaptionText=progTitleW;
    cdui.hbmBanner=0;
    dw=pCredUIPromptForCredentialsW(&cdui,lpDomain,NULL,0,lpUserName,CREDUI_MAX_USERNAME_LENGTH+1,lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH+1,&fSave,CREDUI_FLAGS_INCORRECT_PASSWORD|CREDUI_FLAGS_VALIDATE_USERNAME|CREDUI_FLAGS_DO_NOT_PERSIST);
    if(dw==NO_ERROR){
     dw=pCredUIParseUserNameW(lpUserName,lpUser,CREDUI_MAX_USERNAME_LENGTH+1,lpDomain,CREDUI_MAX_DOMAIN_TARGET_LENGTH+1);
     if(dw==NO_ERROR){
      STARTUPINFOW stpi; PROCESS_INFORMATION pi;
      RtlZeroMemory(&stpi,sizeof(STARTUPINFOW)); stpi.cb=sizeof(STARTUPINFOW);
      RtlZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
      // Create Process with Logon
      if(CreateProcessWithLogonW(lpUser,lpDomain,lpUserPsw,LOGON_WITH_PROFILE,lpImgPath,lpCmdLine,CREATE_DEFAULT_ERROR_MODE,0,lpCurDir,&stpi,&pi)!=0){
       CloseHandle(pi.hThread);
       CloseHandle(pi.hProcess);
      }
      else {
       dw=ntdllFunctions.RtlGetLastWin32Error();
       FailMessage(smemTable->UnableToRunApplication,dw,FMSG_SHOW_MSGBOX|FMSG_ICONSTOP|FMSG_HIDECODE);
       if(dw==ERROR_LOGON_FAILURE){
        RtlZeroMemory(lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH*2);
        goto prompt_userdata;
       }
      }
     }
     else {// CredUIParseUserNameW fails
      FailMessage(smemTable->VerifyingUserNameSyntax,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_ICONSTOP);
      RtlZeroMemory(lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH*2);
      goto prompt_userdata;
     }
     RtlZeroMemory(lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH*2);
     RtlZeroMemory(lpUserName,CREDUI_MAX_USERNAME_LENGTH*2);
     RtlZeroMemory(lpUser,CREDUI_MAX_USERNAME_LENGTH*2);
     RtlZeroMemory(lpDomain,CREDUI_MAX_DOMAIN_TARGET_LENGTH*2);
    }
    else if(dw==ERROR_CANCELLED){
     RtlZeroMemory(lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH*2);
     RtlZeroMemory(lpUserName,CREDUI_MAX_USERNAME_LENGTH*2);
     RtlZeroMemory(lpUser,CREDUI_MAX_USERNAME_LENGTH*2);
     RtlZeroMemory(lpDomain,CREDUI_MAX_DOMAIN_TARGET_LENGTH*2);
    }
    else FailMessage("Unable to accept credentials information.",0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_ICONSTOP);
 free_user_mem:
    if(lpwstr!=0)LocalFree(lpwstr);
    LocalFree(lpUserPsw);
    LocalFree(lpDomain);
    LocalFree(lpUser);
    LocalFree(lpUserName);
    if(hCreduiLib!=0){
     FreeLibrary(hCreduiLib);
     pCredUIPromptForCredentialsW=0;
     pCredUIParseUserNameW=0;
    }
   }
  }
 }
 else FailMessage(smemTable->UnblReadMem,ntStatus,FMSG_SHOW_MSGBOX|FMSG_NTSTATUS_CONVERT|FMSG_NO_SUCCESSINFO|FMSG_SMART_SHOWCODE);
exit:
 if(hProcess!=0)CloseHandle(hProcess);
 LocalFree(pRtlupp);
 LocalFree(lpCurDir);
 LocalFree(lpImgPath);
 LocalFree(lpCmdLine);
 return;
set_readmem_err:
 FailMessage(smemTable->UnblReadMem,0,FMSG_SHOW_MSGBOX|FMSG_CallGLE|FMSG_NO_SUCCESSINFO|FMSG_SMART_SHOWCODE);
 goto exit;
}

#include "procInfo.cpp"

void Process_TryKill(){LVITEM dlvi; DWORD ln,n,dwPid; PROCESS_INFO *pinf; char *tmpb; HANDLE hProcess; BOOL bCritical;
 if((dlvi.iItem=SendMessage(hProcessList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)goto exit;
 dlvi.iSubItem=0; dlvi.mask=LVIF_PARAM;
 if(SendMessage(hProcessList,LVM_GETITEM,0,(LPARAM)&dlvi)==0)goto fails;
 pinf=(PROCESS_INFO*)dlvi.lParam;
 if(!IsVALID_PROCESS_INFO(pinf) || pinf->bPseudoProcess)goto exit;
 dwPid=((PROCESS_INFO*)dlvi.lParam)->Id;
 // Is it Critical Process
 bCritical=0;
 if((hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwPid))!=0){
  if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessCriticalInformation,&n,sizeof(DWORD),&ln)==0){
   bCritical=(BOOL)n;
  }
  CloseHandle(hProcess);
 }
 if(progSetts.CheckState[2] || bCritical){//Confirm Kill
  *(DWORD*)(t0buf1)=0x00002820;//add " ("
  ltoa(dwPid,(char*)t0buf1+2,10);
  strappend(t0buf1,") ?");
  ln=pinf->ProcessName.Length; ln*=0.5f;
  ln+=getstrlen(t0buf1);
  if(bCritical)ln+=getstrlen(smemTable->ThisIsCriticalProcess);
  ln+=getstrlen(smemTable->AreYouSureToKill); ln++;
  if((tmpb=(char*)LocalAlloc(LMEM_FIXED,ln))!=0){
   if(bCritical)n=copystring(tmpb,smemTable->ThisIsCriticalProcess);
   else n=0;
   n+=copystring(tmpb+n,smemTable->AreYouSureToKill); ln-=n;
   UnicodeStringToAnsiString(pinf->ProcessName.Buffer,pinf->ProcessName.Length,tmpb+n,ln);
   strappend(tmpb,t0buf1);
   n=MessageBox(processes_win,tmpb,progTitle,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1);
   LocalFree(tmpb);
   if(n!=IDYES)goto exit;
  }
 }
 if((pinf->Flags&PINF_FLAG_WOW_TASK)==0)Process_Terminate(dwPid,1);
 else if(VDMTerminateTaskWOW!=0){VDMTerminateTaskWOW(pinf->ParentProcessID,(WORD)pinf->HandleCount);}
 goto exit;
fails:
 FailMessage(smemTable->UnblToAction,0,FMSG_SHOW_MSGBOX|FMSG_CallGLE|FMSG_NO_SUCCESSINFO|FMSG_ICONSTOP);
exit:
}

PROCESS_INFO *Processes_GetCurrentPINF(){int iitem; PROCESS_INFO *pinf; LVITEM lvi;
 iitem=SendMessage(hProcessList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED);
 if(iitem==-1)return 0;
 lvi.mask=LVIF_PARAM; lvi.iItem=iitem;
 if(SendMessage(hProcessList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi)==0)return 0;
 pinf=(PROCESS_INFO*)lvi.lParam;
 if(!IsVALID_PROCESS_INFO(pinf))return 0;
 return pinf;
}

BOOL CALLBACK Process_MainWin_FindProc(HWND hwnd,LPARAM lParam){DWORD n; DWORD_PTR dwp;
 if(bPaneDestroy)return 0;
 dwp=GetWindowLongPtr(hwnd,GWL_STYLE);
 if((dwp&WS_VISIBLE)!=WS_VISIBLE)return 1;
 if(GetWindow(hwnd,GW_OWNER)!=0)return 1;
 n=(DWORD)-1;
 GetWindowThreadProcessId(hwnd,&n);
 if(n==((PROCESS_WND_FIND_STRUCT*)lParam)->dwProcessId){
  ((PROCESS_WND_FIND_STRUCT*)lParam)->hwndWin=hwnd;
  return 0;
 }
 else return 1;
}

void Process_MainWin_Find(PROCESS_INFO *pinf,PROCESS_WND_FIND_STRUCT *pprcwndf){DWORD dw;
 pprcwndf->hwndWin=0;
 if(IsWindow(pinf->hwndMainWin)){
  dw=(DWORD)-1;
  GetWindowThreadProcessId(pinf->hwndMainWin,&dw);
  if(dw==pinf->Id)pprcwndf->hwndWin=pinf->hwndMainWin;
 }
 if(pprcwndf->hwndWin==0){
  pprcwndf->dwProcessId=pinf->Id;
  EnumWindows(Process_MainWin_FindProc,(LPARAM)pprcwndf);
 }
}

void Process_MainWin_Action(PROCESS_INFO *pinf,DWORD dwAction){PROCESS_WND_FIND_STRUCT prcwndf;
 Process_MainWin_Find(pinf,&prcwndf);
 if(prcwndf.hwndWin!=0){
  pinf->hwndMainWin=prcwndf.hwndWin;
  Window_Action(prcwndf.hwndWin,0,dwAction);
 }
}

/*DWORD FindItem(DWORD lParam, int itemCount){LV_ITEM lvi;
lvi.iSubItem=0;
for(int i=0;i<itemCount;i++){//поиск номера существующего item'a
 lvi.iItem=i; lvi.mask=LVIF_PARAM; lvi.lParam=0;
 SendMessage(hlist,LVM_GETITEM,i,(LPARAM)&lvi);
 if(lParam==(DWORD)lvi.lParam)return i;
}
return((DWORD)-1);} */

void FileModule_GetIcon(char *szPath,PROCESS_INFO *pinf){HICON hIcon; DWORD dw; hIcon=0; //Надо еще и RemoveIconFromList
 /*
  ExtractAssociatedIconA
    MultiByteToWideChar
    ExtractAssociatedIconW
      ExtractIconW
      SHGetFileInfoW
      GetModuleFileNameW
      LoadIconW
 */
 ExtractIconEx(szPath,0,NULL,&hIcon,1);
 if(hIcon==0)ExtractIconEx(szPath,0,&hIcon,NULL,1);
 if(hIcon==0)hIcon=ExtractAssociatedIcon(gInst,szPath,(WORD*)&dw);
 if(hIcon!=0){
  pinf->iIcon=ImageList_AddIcon(himgl,hIcon);
  pinf->Flags|=PINF_FLAG_SET_IMAGE;
  DestroyIcon(hIcon);
 }
}

char *Process_MakeCreationTimeString(FILETIME *ftm){char *ret; FILETIME lftm;
 ret=smemTable->n_a;
 if(ftm==0 || (ftm->dwLowDateTime==0 && ftm->dwHighDateTime==0))goto quit;
 if(FileTimeToLocalFileTime(ftm,&lftm)==0)goto quit;
 if(FileTimeToSystemTime(&lftm,&stm)==0)goto quit;
 getTime(&stm,pvbuf1,1); *((DWORD*)pvbuf1+3)=0x00204020;
 getDate(&stm,pvbuf1+15);
 ret=pvbuf1;
quit:
 return ret;
}

void SetCreationTime(FILETIME *ftm){LVITEM lvi; FILETIME lftm;
 if(ftm->dwLowDateTime==0 && ftm->dwHighDateTime==0){lvi.pszText=smemTable->n_a; goto settext;}
 FileTimeToLocalFileTime(ftm,&lftm);
 FileTimeToSystemTime(&lftm,&stm);
 getTime(&stm,t1buf512,1); *((DWORD*)t1buf512+3)=0x00204020;
 getDate(&stm,t1buf512+15);
 lvi.pszText=t1buf512;
settext:
 lvi.iSubItem=Prcclnumber[8]-1; SendMessage(hProcessList,LVM_SETITEMTEXT,Item,(LPARAM)&lvi);
}

BOOL EnumWOWTasksProc(DWORD dwThreadId,WORD hMod16,WORD hTask16,PSZ pszModName,PSZ pszFileName,LPARAM lpParam){
 PROCESS_INFO *pinf; STRING str; SYSTEM_PROCESS_INFORMATION *spi; DWORD n,ln,prevFlags,bMoreInfo; char *fptr; BOOL exist;
 exist=0;
 pinf=fpinfo;
 while(IsVALID_PROCESS_INFO(pinf)){
  if(pinf->Id==dwThreadId && pinf->dwStatus<2){
   //if(pinf->ProcessName.Length==0 || CompareUnicodeStr(&pinf->ProcessName,&spi->ProcessName)==0){
    exist=1; break;
   //}
  }
  pinf=pinf->NextEntry;
 }
 if(exist){prevFlags=(WORD)(pinf->Flags); goto skip_addnew;}
 processCount++;
 if((pinf=(PROCESS_INFO*)LocalAlloc(LPTR,sizeof(PROCESS_INFO)))==0)return 0;
 pinf->Id=dwThreadId;
 pinf->ParentProcessID=lpParam;
 pinf->HandleCount=hTask16;
 pinf->spi=(SYSTEM_PROCESS_INFORMATION*)LocalAlloc(LPTR,sizeof(SYSTEM_PROCESS_INFORMATION));
 //pinf->prevPrivate=lpParam; pinf->prevHandles=hTask16;
 n=ln=getstrlen(pszModName); n++; n<<=1;
 if((pinf->ProcessName.Buffer=(LPWSTR)LocalAlloc(LMEM_FIXED,n))!=0){
  str.Buffer=pszModName; str.Length=(USHORT)ln;
  pinf->ProcessName.MaximumLength=(USHORT)n;
  ntdllFunctions.RtlAnsiStringToUnicodeString(&pinf->ProcessName,&str,0);
 }
 n=ln=getstrlen(pszFileName); n++; n<<=1;
 if((pinf->ImagePathName.Buffer=(LPWSTR)LocalAlloc(LMEM_FIXED,n))!=0){
  str.Buffer=pszFileName; str.Length=(USHORT)ln;
  pinf->ImagePathName.MaximumLength=(USHORT)n;
  ntdllFunctions.RtlAnsiStringToUnicodeString(&pinf->ImagePathName,&str,0);
 }
 pinf->ImagePath=AllocateAndCopy(0,pszFileName);
 pinf->ThreadCount=pinf->VirtualSize=pinf->PagefileUsage=pinf->WorkingSetSize=pinf->CommitCharge=pinf->SessionId=(DWORD)-1;
 pinf->Flags=PINF_FLAG_WOW_TASK|PINF_FLAG_DONTSHOWPROPS;
 if(!firsttime)pinf->Flags|=PINF_FLAG_NEW_PROCESS;
 // Validate and Add the struct
 pinf->cbSize=sizeof(PROCESS_INFO);
 if(!IsVALID_PROCESS_INFO(fpinfo))fpinfo=pinf;
 else if(IsVALID_PROCESS_INFO(plastinf))plastinf->NextEntry=pinf;
 plastinf=pinf;
 //
 prevFlags=0;
skip_addnew:
 pinf->dwStatus=1;
 spi=pinf->spi;
 if(prevFlags&PINF_FLAG_NEW_PROCESS && exist)pinf->Flags&=~PINF_FLAG_NEW_PROCESS;
 if(firsttime)goto get_threadinfo;/// firsttime skip
 ////// after firsttime start
 bMoreInfo=0;
 if(exist){
  bMoreInfo+=Upd[16]; bMoreInfo+=Upd[17]; bMoreInfo+=Upd[18];
  if(bLoadIcons){
   if(pinf->ImagePath){
    n=(DWORD_PTR)pinf->ImagePath>*(DWORD_PTR*)smemTable;
    n+=(DWORD_PTR)pinf->ImagePath<(DWORD_PTR)Menus[0];
    if(n!=2)FileModule_GetIcon(CorrectFilePath(pinf->ImagePath,0,wpbuf3),pinf);
   }
   else bMoreInfo=1;
  }
  if(!bMoreInfo)goto get_threadinfo;
 }
 ///
 fptr=CorrectFilePath(pinf->ImagePath,0,wpbuf3);
 FileModule_GetIcon(fptr,pinf);
 if((Prcclnumber[16] || Prcclnumber[17] || Prcclnumber[18])){//Description,Version,Company
  DWORD_PTR w1,w2,w3; void *vmem;
  w1=Prcclnumber[17]; w2=Prcclnumber[16]; w3=Prcclnumber[18];
  vmem=FileModule_GetVerDescInfo(fptr,&w1,&w2,&w3,0);
  if(w2)pinf->Description=AllocateAndCopy(0,(LPTSTR)w2);
  if(w1)pinf->Version=AllocateAndCopy(0,(LPTSTR)w1);
  if(w3)pinf->CompanyName=AllocateAndCopy(0,(LPTSTR)w3);
  if(vmem)LocalFree(vmem);
 }
get_threadinfo:
 HANDLE hThread; CLIENT_ID cid; LSA_OBJECT_ATTRIBUTES oat;
 if(ntdllFunctions.pNtQueryInformationThread==0)goto skip_threadinfo;
 cid.UniqueProcessId=0; cid.UniqueThreadId=dwThreadId;
 RtlZeroMemory(&oat,sizeof(LSA_OBJECT_ATTRIBUTES));
 oat.Length=sizeof(LSA_OBJECT_ATTRIBUTES); hThread=0;
 if((ntdllFunctions.pNtOpenThread(&hThread,THREAD_QUERY_INFORMATION,&oat,&cid))!=0 || hThread==0)goto skip_threadinfo;
 if(ntdllFunctions.pNtQueryInformationThread(hThread,ThreadTimes,wpbuf1,sizeof(THREAD_TIMES_INFORMATION),&n)!=0)goto skip_threadtimes;
 THREAD_TIMES_INFORMATION *tti; tti=(THREAD_TIMES_INFORMATION*)wpbuf1;
 tti->KernelTime.QuadPart+=tti->UserTime.QuadPart;
 spi->KernelTime.QuadPart=pinf->CpuTime.QuadPart;
 pinf->CpuTime.QuadPart=tti->KernelTime.QuadPart;
 spi->ThreadCount=pinf->ThreadCount; pinf->ThreadCount=1;
 if(!exist){
  spi->CreateTime=pinf->CreateTime=tti->CreationTime;
 }
skip_threadtimes:
 if(ntdllFunctions.pNtQueryInformationThread(hThread,ThreadBasicInformation,wpbuf1,sizeof(THREAD_BASIC_INFORMATION),&n)!=0)goto skip_threadinfo;
 THREAD_BASIC_INFORMATION *tbi; tbi=(THREAD_BASIC_INFORMATION*)wpbuf1;
 spi->BasePriority=tbi->Priority;
 if(Prcclnumber[6] && (pinf->BasePriority!=spi->BasePriority || Upd[6])){
  if(spi->BasePriority>0){
   if(progSetts.CheckState[13]){
    if((DWORD_PTR)pinf->PriorityText>(DWORD_PTR)smemTable && (DWORD_PTR)pinf->PriorityText<(DWORD_PTR)Menus){
 alloc_priority_txt:
     pinf->PriorityText=(char*)LocalAlloc(LMEM_FIXED,5);
    }
    else {LocalFree(pinf->PriorityText); goto alloc_priority_txt;}
    ltoa(spi->BasePriority,pinf->PriorityText,10);
   }
   else{
    if(pinf->PriorityText){
     n=(DWORD_PTR)pinf->PriorityText>*(DWORD_PTR*)smemTable;
     n+=(DWORD_PTR)pinf->PriorityText<(DWORD_PTR)Menus[0];
     if(n!=2)LocalFree(pinf->PriorityText);
    }
    if(spi->BasePriority<6)fptr=smemTable->Idle;
    else if(spi->BasePriority<8)fptr=smemTable->BNormal;
    else if(spi->BasePriority<10)fptr=smemTable->Normal;
    else if(spi->BasePriority<13)fptr=smemTable->ANormal;
    else if(spi->BasePriority<24)fptr=smemTable->High;
    else if(spi->BasePriority>=24)fptr=smemTable->Realtime;
    pinf->PriorityText=fptr;
   }
  }
 }
skip_threadinfo:
 if((WORD)prevFlags!=(WORD)(pinf->Flags))pinf->Flags|=PINF_FLAG_UPDATE_ITEM;
 if(hThread)CloseHandle(hThread);
 return 1;
}

BOOL EnumWOWProcesses(DWORD dwProcessId,DWORD dwAttributes,LPARAM){PROCESS_INFO *pinf;
 pinf=fpinfo;
 while(pinf!=0){
  if(pinf->Id==dwProcessId && pinf->dwStatus<2)goto set_info;
  pinf=(PROCESS_INFO*)(pinf->NextEntry);
 }
 goto exit;
set_info:
 if((pinf->Flags&PINF_FLAG_WOW_PROCESS)==0){
  pinf->Flags|=PINF_FLAG_WOW_PROCESS;//|PINF_FLAG_UPDATE_ITEM;
 }
 VDMEnumTaskWOWEx(dwProcessId,EnumWOWTasksProc,dwProcessId);
exit:
 return 1;
}

/*
void FindSpy(){SYSTEM_PROCESS_INFORMATION *spi; DWORD n,nn,*im,*pid1,*pid2,*d1; //300 pIDs
queryAgain: n=ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,hg,hgsz,&nn);
if(n==0xC0000004 && hgsz<nn){hgsz=nn; hg=LocalReAlloc(hg,hgsz,LMEM_MOVEABLE); goto queryAgain;}
pid1=d2Start;
while(n<=nn){
 spi=(SYSTEM_PROCESS_INFORMATION*)((BYTE*)(hg)+n);
 n+=(spi->NextEntryOffset);
 *pid1=spi->UniqueProcessId; pid1++;
 if((spi->NextEntryOffset)==0)break;
}
if((int)(pid1-d2Start)>1200){FailMessage("Error1",0,FMSG_NO_INFO|FMSG_WRITE_LOG); return;}
pid2=d2Start+300;//+1200 bytes
HINSTANCE hntdll=LoadLibrary(ntdll_dll);
pNormalNtQuerySystemInformation=(DWORD(_stdcall*)(DWORD,VOID*,DWORD,ULONG*))GetProcAddress(hntdll,"NtQuerySystemInformation");
queryAgain2: n=pNormalNtQuerySystemInformation(0x05,hg,hgsz,&nn);
if(n==0xC0000004 && hgsz<nn){hgsz=nn; hg=LocalReAlloc(hg,hgsz,LMEM_MOVEABLE); goto queryAgain2;}
FreeLibrary(hntdll);
while(n<=nn){
 spi=(SYSTEM_PROCESS_INFORMATION*)((BYTE*)(hg)+n);
 n+=(spi->NextEntryOffset);
 *pid2=spi->UniqueProcessId; pid2++;
 if((spi->NextEntryOffset)==0)break;
}
if(((DWORD)pid2-(DWORD)d2Start)>2400){FailMessage("Error2",0,FMSG_NO_INFO|FMSG_WRITE_LOG); return;}
//if((int)(pid2-d2Start-1200)>(int)(pid1-d2Start))errMsg(); //trough GetProcAddr returned more pIDs - ???
for(im=d2Start;im<pid1;im++){
 for(d1=d2Start+300;d1<pid2;d1++){
  if(*d1==*im){*im=-1; continue;}//надо  xor im,im ;  dec im;
 }
}
pid2=d2Start+300;//+1200 bytes
for(im=d2Start;im<pid1;im++){
 if(*im!=0xFFFFFFFF){*pid2=*im; pid2++; continue;}//hidden pid
}
*pid2=-1;
}*/

void WatchProcesses(){SYSTEM_PROCESS_INFORMATION *spi; HANDLE hProcess; PROCESS_BASIC_INFORMATION pbi; PROCESS_INFO *pinf; LV_FINDINFO lvf;
 DWORD nn,nnSize,n,nm,ln,prevFlags,bReadMem; BOOL exist,bCurntUser; char *pstr,*fptr;
queryAgain:
 nn=ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,spi_mem,spi_memsz,&nnSize);
 if(nn!=0){
  if(nn==STATUS_INFO_LENGTH_MISMATCH && spi_memsz<nnSize){spi_memsz=nnSize; spi_memsz+=1024; spi_mem=LocalReAlloc(spi_mem,spi_memsz,GMEM_MOVEABLE); goto queryAgain;}
  else {
   //FailMessage(tbuf1,nn,FMSG_WRITE_LOG);
   return;
  }
 }
 //set exist_status to null
 pinf=fpinfo;
 while(IsVALID_PROCESS_INFO(pinf)){
  if(pinf->dwStatus<2)pinf->dwStatus=0;
  pinf=pinf->NextEntry;
 }
 while(nn<=nnSize){
  spi=(SYSTEM_PROCESS_INFORMATION*)((BYTE*)(spi_mem)+nn);
  nn+=(spi->NextEntryOffset); exist=0;
  if(fpinfo==0)goto skip_fmatch;
  pinf=fpinfo;
  while(IsVALID_PROCESS_INFO(pinf)){
   if(pinf->Id==spi->UniqueProcessId && pinf->dwStatus<2 && pinf->ProcessName.Length==spi->ProcessName.Length){
    if(pinf->ProcessName.Length==0 || CompareUnicodeStr(&pinf->ProcessName,&spi->ProcessName)==0){exist=1; break;}
   }
   pinf=pinf->NextEntry;
  }
  if(exist){prevFlags=(WORD)(pinf->Flags); goto skip_addnew;}
skip_fmatch:
  // Add the Process
  processCount++;
  if((pinf=(PROCESS_INFO*)LocalAlloc(LPTR,sizeof(PROCESS_INFO)))==0)goto skip_add_processes;
  pinf->Id=spi->UniqueProcessId;
  pinf->ParentProcessID=spi->ParentProcessID;
  if(spi->UniqueProcessId!=0){
   n=ln=spi->ProcessName.Length;
   if((pinf->ProcessName.Buffer=(LPWSTR)LocalAlloc(LMEM_FIXED,n+sizeof(WCHAR)))!=0){
    pstr=(LPSTR)pinf->ProcessName.Buffer;
    pstr+=copybytes(pstr,spi->ProcessName.Buffer,ln,0); *(LPWSTR)pstr=0;
    pinf->ProcessName.Length=(USHORT)ln;
   }
  }
  else{
   STRING str; str.Buffer=smemTable->SysIdleProc;
   n=getstrlen(str.Buffer); str.Length=(USHORT)n; n++; n*=2; pinf->ProcessName.MaximumLength=(USHORT)n;
   pinf->ProcessName.Buffer=(LPWSTR)LocalAlloc(LMEM_FIXED,n);
   if(ntdllFunctions.RtlAnsiStringToUnicodeString(&pinf->ProcessName,&str,0)!=0){LocalFree(pinf->ProcessName.Buffer); pinf->ProcessName.Buffer=0;}
   pinf->ProcessName.Length=0;
  }
  pinf->ThreadCount=pinf->HandleCount=pinf->VirtualSize=pinf->PagefileUsage=pinf->WorkingSetSize=pinf->CommitCharge=pinf->SessionId=(DWORD)-1;
  if(!firsttime)pinf->Flags=PINF_FLAG_NEW_PROCESS;
  // Validate and Add the struct
  pinf->cbSize=sizeof(PROCESS_INFO);
  if(!IsVALID_PROCESS_INFO(fpinfo))fpinfo=pinf;
  else if(IsVALID_PROCESS_INFO(plastinf))plastinf->NextEntry=pinf;
  else{
   PROCESS_INFO *ppinf;
   ppinf=fpinfo;
   while(IsVALID_PROCESS_INFO(ppinf)){plastinf=ppinf; ppinf=ppinf->NextEntry;}
   plastinf->NextEntry=pinf;
  }
  pinf->PrevEntry=plastinf;
  plastinf=pinf;
  //
  prevFlags=0;
skip_addnew:
  pinf->dwStatus=1; pinf->spi=spi;
  if(prevFlags&PINF_FLAG_NEW_PROCESS && exist)pinf->Flags&=~PINF_FLAG_NEW_PROCESS;
  //get User info
  if((!exist || Upd[5]) && (Prcclnumber[5] || !bShowAllProcesses)){
   Process_GetMachineUserName(spi->UniqueProcessId,&(spi->CreateTime),wpbuf1,((BYTE*)wpbuf1)+1024,wpbuf3,&bCurntUser);
   CompareAndReAllocateA(&pinf->UserName,(char*)wpbuf1);
   if(bCurntUser){
    pinf->Flags|=PINF_FLAG_CURRENT_USER;
    processCountUser++;
   }
  }
  //get Priority info
  if(Prcclnumber[6] && (pinf->BasePriority!=spi->BasePriority || Upd[6])){
   if(spi->BasePriority>0){
    if(progSetts.CheckState[13]){// numeric output
     if((DWORD_PTR)pinf->PriorityText>(DWORD_PTR)stringsMem && (DWORD_PTR)pinf->PriorityText<(DWORD_PTR)Menus){
 alloc_priority_txt:
      pinf->PriorityText=(LPTSTR)LocalAlloc(LMEM_FIXED,5*sizeof(TCHAR));
     }
     else {LocalFree(pinf->PriorityText); goto alloc_priority_txt;}
     if(spi->BasePriority>9999)spi->BasePriority=9999;// 4 digits limit
     ltoa(spi->BasePriority,pinf->PriorityText,10);
    }
    else{// non-numeric output
     if(pinf->PriorityText){
      n=(DWORD_PTR)pinf->PriorityText>*(DWORD_PTR*)smemTable;
      n+=(DWORD_PTR)pinf->PriorityText<(DWORD_PTR)Menus[0];
      if(n!=2)LocalFree(pinf->PriorityText);
     }
     if(spi->BasePriority<6)fptr=smemTable->Idle;
     else if(spi->BasePriority<8)fptr=smemTable->BNormal;
     else if(spi->BasePriority<10)fptr=smemTable->Normal;
     else if(spi->BasePriority<13)fptr=smemTable->ANormal;
     else if(spi->BasePriority<24)fptr=smemTable->High;
     else if(spi->BasePriority>=24)fptr=smemTable->Realtime;
     pinf->PriorityText=fptr;
    }
   }
  }
  pinf->CreateTime=spi->CreateTime;
  // CPU usage & CPU (kernel+user) Time
  lint.QuadPart=spi->KernelTime.QuadPart;
  lint.QuadPart+=spi->UserTime.QuadPart;
  spi->KernelTime.QuadPart=pinf->CpuTime.QuadPart;// store current CpuTime as PrevCpuTime
  pinf->CpuTime.QuadPart=lint.QuadPart;
  if(firsttime)goto goNext;//at first time show only the information retrieved form NtQuerySysInfo
  ////// after firsttime start
  bReadMem=0;
  if((!exist || Upd[20] || pinf->SessionId==(DWORD)-1) && Prcclnumber[20])bReadMem=1;
  //Read Memory:
  if(exist){
   bReadMem+=Upd[3]; bReadMem+=Upd[16]; bReadMem+=Upd[17]; bReadMem+=Upd[18];
   if(bLoadIcons){
    if(pinf->ImagePath){
     n=(DWORD_PTR)pinf->ImagePath>*(DWORD_PTR*)smemTable;
     n+=(DWORD_PTR)pinf->ImagePath<(DWORD_PTR)Menus[0];
     if(n!=2)FileModule_GetIcon(CorrectFilePath(pinf->ImagePath,0,wpbuf3),pinf);
    }
    else bReadMem=1;
   }
   if((pinf->Flags&PINF_FLAG_READ_MEM_AGAIN)==PINF_FLAG_READ_MEM_AGAIN){
    bReadMem=1;
   }
   if(!bReadMem)goto skip_readmem;
  }
  if((hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,spi->UniqueProcessId))!=0){
   if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessBasicInformation,&pbi,sizeof(pbi),&nm)!=0)goto set_readmem_err;
   if(pbi.PebBaseAddress==0)goto set_readmem_err;
   if(ReadProcessMemory(hProcess,pbi.PebBaseAddress,wpbuf1,sizeof(PEB),0)==0){
    if(!exist)pinf->Flags|=PINF_FLAG_READ_MEM_AGAIN;
 set_readmem_err:
    pinf->Flags|=PINF_FLAG_UNBL_QUERY;
    pinf->ImagePath=pinf->CommandLine=pinf->CompanyName=pinf->Description=pinf->Version=smemTable->UnblReadMem;
    if(pinf->Id==4){pinf->SessionId=0;if(Prcclnumber[20])spi->Reserved01=0xFFAAFF15;}
    goto skip_inf;
   }
   pinf->SessionId=((PEB*)wpbuf1)->SessionId;
   if(Prcclnumber[20])spi->Reserved01=0xFFAAFF15;//it is update-sign
   if(((PEB*)wpbuf1)->ProcessParameters==0)goto skip_inf;
   if(ReadProcessMemory(hProcess,((PEB*)wpbuf1)->ProcessParameters,wpbuf1,sizeof(RTL_USER_PROCESS_PARAMETERS),0)){
    if(pinf->ImagePath==0 || pinf->ImagePath==smemTable->_UnblOpenProcess_ || ((pinf->Flags&PINF_FLAG_READ_MEM_AGAIN)==PINF_FLAG_READ_MEM_AGAIN)){
     if(((RTL_USER_PROCESS_PARAMETERS*)wpbuf1)->ImagePathName.Buffer==0)goto skip_imageinfo_err;
     n=((RTL_USER_PROCESS_PARAMETERS*)wpbuf1)->ImagePathName.Length;
     pinf->ImagePathName.Buffer=(LPWSTR)LocalAlloc(LMEM_FIXED,n+sizeof(WCHAR));
     if(ReadProcessMemory(hProcess,((RTL_USER_PROCESS_PARAMETERS*)wpbuf1)->ImagePathName.Buffer,pinf->ImagePathName.Buffer,n,&n)==0){
      LocalFree(pinf->ImagePathName.Buffer);
      goto skip_imageinfo_err;
     }
     ln=pinf->ImagePathName.Length=(USHORT)n; ln/=2; ln++;
     pinf->ImagePath=(LPSTR)LocalAlloc(LMEM_FIXED,ln);
     UnicodeStringToAnsiString(pinf->ImagePathName.Buffer,n,pinf->ImagePath,ln);
    }
    n=(DWORD_PTR)pinf->ImagePath>*(DWORD_PTR*)smemTable;
    n+=(DWORD_PTR)pinf->ImagePath<(DWORD_PTR)Menus[0];
    if(n!=2){
     fptr=CorrectFilePath(pinf->ImagePath,0,wpbuf3);
     FileModule_GetIcon(fptr,pinf);
     if((Prcclnumber[16] || Prcclnumber[17] || Prcclnumber[18])){//Description,Version,Company
      DWORD w1,w2,w3; void *vmem;
      w1=Prcclnumber[17]; w2=Prcclnumber[16]; w3=Prcclnumber[18];
      vmem=FileModule_GetVerDescInfo(fptr,&w1,&w2,&w3,0);
      if(w2)pinf->Description=AllocateAndCopy(0,(char*)w2);
      if(w1)pinf->Version=AllocateAndCopy(0,(char*)w1);
      if(w3)pinf->CompanyName=AllocateAndCopy(0,(char*)w3);
      if(vmem)LocalFree(vmem);
     }
    }
   }
   else {//unable to get ImagePath, so other image info is unavailable too.
 skip_imageinfo_err:
    if(!exist)pinf->Flags|=PINF_FLAG_READ_MEM_AGAIN;
    pinf->ImagePath=pinf->CompanyName=pinf->Description=pinf->Version=smemTable->UnblReadMem;
   }
   if(Prcclnumber[10]){// get CommandLine info
    if(((RTL_USER_PROCESS_PARAMETERS*)wpbuf1)->CommandLine.Buffer!=0){
     n=((RTL_USER_PROCESS_PARAMETERS*)wpbuf1)->CommandLine.Length;
     if(ReadProcessMemory(hProcess,((RTL_USER_PROCESS_PARAMETERS*)wpbuf1)->CommandLine.Buffer,(void*)wpbuf3,n,&n)!=0){
      ln=n; ln/=2; ln++;
      pinf->CommandLine=(LPSTR)LocalAlloc(LMEM_FIXED,ln);
      UnicodeStringToAnsiString((LPWSTR)wpbuf3,n,pinf->CommandLine,ln);
     }
     else pinf->CommandLine=smemTable->UnblReadMem;
    }
   }
 skip_inf:
   CloseHandle(hProcess);
  }
  else {
   pinf->Flags|=PINF_FLAG_UNBL_QUERY;
   if(Prcclnumber[3] || Prcclnumber[10] || Prcclnumber[16] || Prcclnumber[17] || Prcclnumber[18]){// || Prcclnumber[19] -> WindowTitle
    pinf->ImagePath=pinf->CommandLine=pinf->CompanyName=pinf->Description=pinf->Version=smemTable->_UnblOpenProcess_;
    if(pinf->Id==0){pinf->SessionId=0; if(Prcclnumber[20])spi->Reserved01=0xFFAAFF15;}
   }
  }
 skip_readmem:
  if(exist)pinf->Flags&=~PINF_FLAG_READ_MEM_AGAIN;
 goNext:
  // Suspended or Not
  ln=0; for(n=1;n<=spi->ThreadCount;n++){
   if(spi->Threads[n].WaitReason==5 || spi->Threads[n].WaitReason==12)ln++; // Suspended || WrSuspended
  }
  if(ln==spi->ThreadCount)pinf->Flags|=PINF_FLAG_SUSPENDED;
  else pinf->Flags&=~PINF_FLAG_SUSPENDED;
  // store Prev info
  n=spi->ThreadCount; spi->ThreadCount=pinf->ThreadCount; pinf->ThreadCount=n;
  n=spi->BasePriority; spi->BasePriority=pinf->BasePriority; pinf->BasePriority=n;
  n=spi->HandleCount; spi->HandleCount=pinf->HandleCount; pinf->HandleCount=n;
  n=spi->VmCounters.VirtualSize; spi->VmCounters.VirtualSize=pinf->VirtualSize; pinf->VirtualSize=n;
  n=spi->VmCounters.PagefileUsage; spi->VmCounters.PagefileUsage=pinf->PagefileUsage; pinf->PagefileUsage=n;
  n=spi->VmCounters.WorkingSetSize; spi->VmCounters.WorkingSetSize=pinf->WorkingSetSize; pinf->WorkingSetSize=n;
  n=spi->CommitCharge; spi->CommitCharge=pinf->CommitCharge; pinf->CommitCharge=n;
  if((WORD)prevFlags!=(WORD)(pinf->Flags))pinf->Flags|=PINF_FLAG_UPDATE_ITEM;
  // finish
  if(pinf->Id==0){//insert Interrupts and DPCs
   if(!exist){
    n=PINF_PSEUDOPROCESS_ID_INT;
 add_more:
    if((pinf=(PROCESS_INFO*)LocalAlloc(LPTR,sizeof(PROCESS_INFO)))==0)goto skip_add_processes;
    pinf->bPseudoProcess=1;
    pinf->Id=n;
    pinf->Flags=PINF_FLAG_DONTSHOWPROPS;
    pinf->PriorityText=pinf->UserName=pinf->ImagePath=pinf->CommandLine=pinf->CompanyName=pinf->Description=pinf->Version=smemTable->n_a;
    if(n==PINF_PSEUDOPROCESS_ID_INT){
     pinf->ProcessName.Buffer=Interrupts_txtW;
     pinf->Description="Hardware Interrupts";
     n=PINF_PSEUDOPROCESS_ID_DPC;
    }
    else{
     pinf->ProcessName.Buffer=DPCs_txtW;
     pinf->Description="Deferred Procedure Calls";
     n=PINF_PSEUDOPROCESS_ID_LAST;
    }
    pinf->ProcessName.Length=getstrlenW(pinf->ProcessName.Buffer);
    pinf->spi=(SYSTEM_PROCESS_INFORMATION*)LocalAlloc(LMEM_FIXED,sizeof(SYSTEM_PROCESS_INFORMATION));
    FillMemory(pinf->spi,sizeof(SYSTEM_PROCESS_INFORMATION),0x01);
    pinf->cbSize=sizeof(PROCESS_INFO);
    plastinf->NextEntry=pinf; plastinf=pinf;
    if(n<PINF_PSEUDOPROCESS_ID_LAST){intrdpcs=pinf; goto add_more;}
    if(intrdpcs!=pinf)intrdpcs->NextEntry=pinf;
   }
  }
  if((spi->NextEntryOffset)==0)break;
 }
skip_add_processes:
 //Show WOW-tasks (16-bit)
 if(progSetts.CheckState[23]){if(VDMEnumProcessWOW!=0)VDMEnumProcessWOW(EnumWOWProcesses,0);}
 // Finish storing the processes info
 if(bLoadIcons){bLoadIcons=0;bReSortList=1;}
 if(firsttime)return;
 // Remove terminated processes info
 PROCESS_INFO *ppinf; LVITEM lvi; BOOL bDelete;
 ppinf=pinf=fpinfo; lvf.flags=LVFI_PARAM;
 while(IsVALID_PROCESS_INFO(pinf)){
  if(pinf->dwStatus==0 && !pinf->bPseudoProcess){
   processCount--;
   if(pinf->phdr3!=0){
    //pinf->phdr3->pinf=pinf;
    pinf->dwStatus=2;
    pinf->KernelTime.QuadPart=pinf->spi->KernelTime.QuadPart;
    pinf->UserTime.QuadPart=pinf->spi->UserTime.QuadPart;
    pinf->phdr3->prcprops.ExitTime.QuadPart=sysTimeInf->CurrentTime.QuadPart;
    pinf->spi=0;
    bDelete=0;
   }
   else bDelete=1;
   if(progSetts.DataDeleteDelay==0){
    lvf.lParam=(LPARAM)pinf;
    if((n=SendMessage(hProcessList,LVM_FINDITEM,-1,(LPARAM)&lvf))!=(DWORD)-1){
     SendMessage(hProcessList,LVM_DELETEITEM,n,(LPARAM)&lvi);
    }
    //del_data:
    ppinf->NextEntry=pinf->NextEntry;
    if(pinf==plastinf)plastinf=ppinf;
    if(bDelete)Process_DestroyData(pinf);
    pinf=ppinf;
   }
   else pinf->dwStatus=2;
  }
  //else if(pinf->dwStatus>1){//не проверено
  // if(pinf->dwStatus>=progSetts.ItemDeleteDelay){
  //  lvf.lParam=(DWORD)pinf; lvf.flags=LVFI_PARAM;
  //  if((n=SendMessage(hProcessList,LVM_FINDITEM,-1,(LPARAM)&lvf))!=0xFFFFFFFF){
  //   SendMessage(hProcessList,LVM_DELETEITEM,n,(LPARAM)&lvi);
  //  }
  // }
  // if(pinf->dwStatus>=progSetts.DataDeleteDelay)goto del_data;
  // pinf->dwStatus++;
  //}
  ppinf=pinf;
  pinf=pinf->NextEntry;
 }
}

//////////////////////////// ProcessesListCompareFunc //////////////////////////
int CALLBACK Processes_ListCompareFunc(LPARAM lParam1,LPARAM lParam2,LIST_SORT_DIRECTION *lParamSort){char *str1,*str2;
 int n1,n2,ret=0;
 if(lParamSort->SortDirection==2){
  PROCESS_INFO *pinf; pinf=fpinfo; n1=n2=0;
  while(IsVALID_PROCESS_INFO(pinf)){
   if((DWORD_PTR)pinf==(DWORD_PTR)lParam1){n1=1;break;}
   else if((DWORD_PTR)pinf==(DWORD_PTR)lParam2){n2=1;break;}
   pinf=pinf->NextEntry;
  }
  if(n1)ret=-1; else if(n2)ret=1;
  return ret;
 }
 n1=lParamSort->CurSubItem; n1++;
 PROCESS_INFO *pinf1,*pinf2;
 pinf1=(PROCESS_INFO*)lParam1; pinf2=(PROCESS_INFO*)lParam2;
 if(n1==Prcclnumber[0]){ret=lstrcmpiW(pinf1->ProcessName.Buffer,pinf2->ProcessName.Buffer);}
 else if(n1==Prcclnumber[1]){ret=pinf1->Id; ret-=pinf2->Id;}
 else if(n1==Prcclnumber[2]){ret=pinf1->ParentProcessID; ret-=pinf2->ParentProcessID;}
 else if(n1==Prcclnumber[3]){str1=pinf1->ImagePath; str2=pinf2->ImagePath; goto compare_strings;}
 else if(n1==Prcclnumber[4]){ret=pinf1->prevCpuUsage-pinf2->prevCpuUsage;}
 else if(n1==Prcclnumber[5]){
  str1=pinf1->UserName;
  str2=pinf2->UserName;
  if(progSetts.CheckState[38]==0){//domain name is invisible. so compare user's part only
   if(str1!=0){
    while(*str1!=0 && *str1!='\\')str1++;
    if(*str1=='\\')str1++;
   }
   if(str2!=0){
    while(*str2!=0 && *str2!='\\')str2++;
    if(*str2=='\\')str2++;
   }
  }
  goto compare_strings;
 }
 else if(n1==Prcclnumber[6]){ret=pinf1->BasePriority; ret-=pinf2->BasePriority;}
 else if(n1==Prcclnumber[7]){
  if(!pinf1->bPseudoProcess)ret=pinf1->ThreadCount;
  else ret=0;
  if(!pinf2->bPseudoProcess)ret-=pinf2->ThreadCount;
 }
 else if(n1==Prcclnumber[8]){ret=CompareFileTime((FILETIME*)&(pinf1->CreateTime),(FILETIME*)&(pinf2->CreateTime));}
 else if(n1==Prcclnumber[9]){ret=CompareFileTime((FILETIME*)&(pinf1->CpuTime),(FILETIME*)&(pinf2->CpuTime));}
 else if(n1==Prcclnumber[10]){str1=pinf1->CommandLine; str2=pinf2->CommandLine; goto compare_strings;}
 else if(n1==Prcclnumber[11]){ret=pinf1->HandleCount; ret-=pinf2->HandleCount;}
 else if(n1==Prcclnumber[12]){ret=pinf1->VirtualSize; ret-=pinf2->VirtualSize;}
 else if(n1==Prcclnumber[13]){ret=pinf1->PagefileUsage; ret-=pinf2->PagefileUsage;}
 else if(n1==Prcclnumber[14]){ret=pinf1->WorkingSetSize; ret-=pinf2->WorkingSetSize;}
 else if(n1==Prcclnumber[15]){ret=pinf1->CommitCharge; ret-=pinf2->CommitCharge;}
 else if(n1==Prcclnumber[16]){str1=pinf1->Description; str2=pinf2->Description; goto compare_strings;}
 else if(n1==Prcclnumber[17]){str1=pinf1->Version; str2=pinf2->Version; goto compare_strings;}
 else if(n1==Prcclnumber[18]){str1=pinf1->CompanyName; str2=pinf2->CompanyName; goto compare_strings;}
 else if(n1==Prcclnumber[20]){ret=pinf1->SessionId; ret-=pinf2->SessionId;}
 else if(n1==Prcclnumber[21]){ret=(DWORD)(pinf1->prevContextSwitches-pinf2->prevContextSwitches);}
 else if(n1==Prcclnumber[22]){ret=pinf1->prevCSwDelta-pinf2->prevCSwDelta;}
quit:
 if(lParamSort->SortDirection==1){if(ret<0)ret=1; else if(ret>0)ret=-1;}
 return ret;
compare_strings:
 if(str1 && str2)ret=lstrcmpi(str1,str2);
 else if(str1==0){if(str2)ret=-1; else ret=0;}
 else ret=1;
 goto quit;
}

void ProcessContextMenu_SetItemsState(HMENU pmenu){LVITEM lvi; DWORD dwf,dwf2; DWORD_PTR dwp; MENUITEMINFO mii; BOOL bl,bbl; HMENU hmenu; PROCESS_INFO *pinf;
 lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=itemSel;
 if(SendMessage(hProcessList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || !IsVALID_PROCESS_INFO(((PROCESS_INFO*)(lvi.lParam))))goto exit;
 pinf=((PROCESS_INFO*)(lvi.lParam));
 dwf=((pinf->ImagePathName.Length==0)?MF_GRAYED:MF_ENABLED);
 dwf|=MF_BYCOMMAND;
 EnableMenuItem(main_menu,40016,dwf);
 mii.cbSize=sizeof(MENUITEMINFO);
 ///
 mii.fMask=MIIM_TYPE; mii.fType=MFT_STRING;
 if(pinf->Flags&PINF_FLAG_SUSPENDED){
  mii.dwTypeData=smemTable->Resume; dwf2=18;//18 play, 19 pause
 }
 else {mii.dwTypeData=smemTable->Suspend; dwf2=19;}
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
  if((progSetts.MenuStyle!=MENU_STYLE_DEFAULT) && (mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(dwf2))!=0)mii.fMask|=MIIM_DATA;
 }
 SetMenuItemInfo(main_menu,40015,0,&mii);
 /// [Priority] Sub-menu
 mii.fMask=MIIM_STATE|MIIM_FTYPE; mii.fType=MFT_STRING|MFT_RADIOCHECK;
 mii.fState=(pinf->BasePriority==24)?MFS_CHECKED:MFS_UNCHECKED;
 SetMenuItemInfo(main_menu,40022,0,&mii);
 mii.fState=(pinf->BasePriority==13)?MFS_CHECKED:MFS_UNCHECKED;
 SetMenuItemInfo(main_menu,40023,0,&mii);
 mii.fState=(pinf->BasePriority==8)?MFS_CHECKED:MFS_UNCHECKED;
 SetMenuItemInfo(main_menu,40024,0,&mii);
 mii.fState=(pinf->BasePriority==4)?MFS_CHECKED:MFS_UNCHECKED;
 SetMenuItemInfo(main_menu,40025,0,&mii);
 /// [Window] Sub-menu
 if((hmenu=GetSubMenu(main_menu,2))==0)return;
 //Find main window
 PROCESS_WND_FIND_STRUCT prcwndf;
 Process_MainWin_Find(pinf,&prcwndf);
 if(prcwndf.hwndWin!=0){
  pinf->hwndMainWin=prcwndf.hwndWin;
  dwf=MF_BYCOMMAND|MF_ENABLED;
  bl=1;
 }
 else {
  dwf=MF_BYCOMMAND|MF_GRAYED;
  bl=0;
 }
 EnableMenuItem(hmenu,65011,dwf);
 //
 if(bl){
  bbl=0;
  hmenu=GetSubMenu(hmenu,0);
  dwp=GetWindowLongPtr(pinf->hwndMainWin,GWL_STYLE);
  //minimize-item
  if((dwp&WS_MINIMIZE)==WS_MINIMIZE){dwf=MF_BYCOMMAND|MF_GRAYED; bbl=1;}
  else dwf=MF_BYCOMMAND|MF_ENABLED;
  EnableMenuItem(hmenu,40129,dwf);
  //maximize-item
  if((dwp&WS_MAXIMIZE)==WS_MAXIMIZE){dwf=MF_BYCOMMAND|MF_GRAYED; bbl=1;}
  else dwf=MF_BYCOMMAND|MF_ENABLED;
  EnableMenuItem(hmenu,40128,dwf);
  //restore-item
  if(!bbl)dwf=MF_BYCOMMAND|MF_GRAYED;
  else dwf=MF_BYCOMMAND|MF_ENABLED;
  EnableMenuItem(hmenu,40130,dwf);
  //hide-item
  if((dwp&WS_VISIBLE)==WS_VISIBLE){dwf=MF_BYCOMMAND|MF_GRAYED; dwf2=MF_BYCOMMAND|MF_ENABLED;}
  else {dwf=MF_BYCOMMAND|MF_ENABLED; dwf2=MF_BYCOMMAND|MF_GRAYED;}
  EnableMenuItem(hmenu,40127,dwf2);
  EnableMenuItem(hmenu,40133,dwf);
  //Extended Style
  dwp=GetWindowLongPtr(pinf->hwndMainWin,GWL_EXSTYLE);
  //alwaysOnTop-item
  mii.fMask=MIIM_TYPE; mii.fType=MFT_STRING;
  if((dwp&WS_EX_TOPMOST)==WS_EX_TOPMOST){mii.dwTypeData=smemTable->DisableAlwaysOnTop; n=23;}
  else {mii.dwTypeData=smemTable->EnableAlwaysOnTop; n=24;}
  if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
   if((mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(n))!=0)mii.fMask|=MIIM_DATA;
  }
  SetMenuItemInfo(hmenu,40132,0,&mii);
 }
 ///
 if(pinf->Flags&PINF_FLAG_DONTSHOWPROPS){EnableContextMenuItems(pmenu,MF_BYCOMMAND|MF_GRAYED); mnProcsDisbl=1;}
 else if(mnProcsDisbl){EnableContextMenuItems(pmenu,MF_BYCOMMAND|MF_ENABLED); mnProcsDisbl=0;}
exit:
}

////////////////////////////// Update ProcessesView ////////////////////////////
void Processes_UpdateListView(){PROCESS_INFO *pinf, *plinf; LVITEM lvi; LV_FINDINFO lvf; DWORD n; BOOL exist,bReSort,bl; LRESULT lres;
 SYSTEM_PROCESS_INFORMATION *spi; FILETIME ftm; SYSTEMTIME stm; BYTE bUpd[COLUMNS_COUNT]; char lta[20]; int ds;
 plinf=pinf=fpinfo; bReSort=0;
 lvi.iItem=0; lvf.flags=LVFI_PARAM;
 if(processesUpd){
  for(n=0;n<COLUMNS_COUNT;n++)Upd[n]=1;
 }
 copybytes((char*)bUpd,(char*)Upd,COLUMNS_COUNT,0);
 bUpd[0]=bUpd[1]=bUpd[2]=bUpd[4]=bUpd[7]=bUpd[8]=bUpd[9]=bUpd[11]=bUpd[12]=bUpd[13]=bUpd[14]=bUpd[15]=bUpd[20]=bUpd[21]=bUpd[22]=0;
 ds=progSetts.ProcsSortd.CurSubItem;
 ds++;
while(IsVALID_PROCESS_INFO(pinf)){
 if(bPaneDestroy)return;
 ////
 if(!bShowAllProcesses && (pinf->Flags&PINF_FLAG_CURRENT_USER)!=PINF_FLAG_CURRENT_USER){
  if(!pinf->bPseudoProcess && pinf->Id!=0){
   if((pinf->Flags&PINF_FLAG_LIST_VISIBLE)!=PINF_FLAG_LIST_VISIBLE)goto next_pinf;
   else bl=1;
  }
  else bl=0;
 }
 else bl=0;
 ////
 lvf.lParam=(LPARAM)pinf; lres=SendMessage(hProcessList,LVM_FINDITEM,-1,(LPARAM)&lvf);
 if(lres!=-1){
  if(bl){//it is visible but must be invisible! so do it.
   if(SendMessage(hProcessList,LVM_DELETEITEM,(WPARAM)lres,(LPARAM)&lvi)){
    pinf->Flags&=~PINF_FLAG_LIST_VISIBLE;
    pinf->Flags|=PINF_FLAG_SET_IMAGE;
    goto next_pinf;
   }
  }
  lvi.iItem=lres;
  exist=1;
  goto update_info;
 }
 exist=0;
 if(progSetts.ProcsSortd.SortDirection==2)goto ins_item;
 lres=SendMessage(hProcessList,LVM_GETITEMCOUNT,0,0); lvi.mask=LVIF_PARAM;
 for(lvi.iItem=0;lvi.iItem<lres;lvi.iItem++){
  lvi.iSubItem=0;
  SendMessage(hProcessList,LVM_GETITEM,0,(LPARAM)&lvi);
  if(Processes_ListCompareFunc(lvi.lParam,(LPARAM)pinf,&(progSetts.ProcsSortd))>0){
   goto ins_item;
  }
 }
ins_item:
 lvi.lParam=(LPARAM)pinf; lvi.mask=LVIF_PARAM; lvi.iSubItem=0;
 if((lvi.iItem=SendMessage(hProcessList,LVM_INSERTITEM,0,(LPARAM)&lvi))!=-1)pinf->Flags|=PINF_FLAG_LIST_VISIBLE;
 if(pinf->Id==0 && pinf->ProcessName.Length==0){
  lvi.mask=LVIF_IMAGE; lvi.iImage=1; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 // Scroll to Last runned process
 if(pinf->NextEntry==0){
  if(progSetts.CheckState[24] && SendMessage(hProcessList,LVM_GETSELECTEDCOUNT,0,0)==0){
   if(progSetts.CheckState[31]==SCROLL_TO_LASTRUNNED){
    if(pinf->Id==cur_pid){
     lvf.lParam=(LPARAM)plinf;
     lres=SendMessage(hProcessList,LVM_FINDITEM,-1,(LPARAM)&lvf);
    }
    else lres=lvi.iItem;
    SendMessage(hProcessList,LVM_ENSUREVISIBLE,lres,0);
   }
   else {//i.e. if(progSetts.CheckState[31]==SCROLL_TO_ENDOFLIST)
    lres=0xFFFF;
    SendMessage(hProcessList,LVM_SCROLL,0,(LPARAM)lres);
   }
   if(progSetts.CheckState[33]){//Select the Process scrolled to
    LVITEM lvt; lvt.stateMask=LVIS_SELECTED|LVIS_FOCUSED;
    lvt.state=LVIS_SELECTED|LVIS_FOCUSED;
    SendMessage(hProcessList,LVM_SETITEMSTATE,lres,(LPARAM)&lvt);
   }
  }
  else if(progSetts.CheckState[39]){//Scroll To New Process
   SendMessage(hProcessList,LVM_ENSUREVISIBLE,lvi.iItem,0);
  }
 }
update_info:
 spi=pinf->spi;
 // Pseudo processes info
 if(pinf->bPseudoProcess){
  //Interrupts and DPCs CPU time calc
  SYSTEM_PROCESSOR_TIMES *pspt; LARGE_INTEGER lint;
  pspt=sysProcessorTm; lint.QuadPart=0; 
  if(Prcclnumber[4] || Prcclnumber[9]){
   if(pinf->Id==PINF_PSEUDOPROCESS_ID_INT){
    for(n=0;n<(DWORD)SysBasicInf->NumberProcessors;n++){lint.QuadPart+=pspt->InterruptTime.QuadPart; pspt++;}
   }
   else if(pinf->Id==PINF_PSEUDOPROCESS_ID_DPC){
    for(n=0;n<(DWORD)SysBasicInf->NumberProcessors;n++){lint.QuadPart+=pspt->DpcTime.QuadPart; pspt++;}
   }
   spi->KernelTime.QuadPart=pinf->CpuTime.QuadPart;
   pinf->CpuTime.QuadPart=lint.QuadPart;
  }
 }
 if((pinf->Flags&PINF_FLAG_SET_IMAGE)==PINF_FLAG_SET_IMAGE){//Set Image
  lvi.mask=LVIF_IMAGE; lvi.iSubItem=0;
  if(pinf->iIcon==-1)pinf->iIcon=0;
  lvi.iImage=I_IMAGECALLBACK;
  SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
  pinf->Flags&=~PINF_FLAG_SET_IMAGE;
 }
 lvi.mask=LVIF_TEXT;
 if((!exist || Upd[8]) && Prcclnumber[8]){//Start Time
  lvi.pszText=Process_MakeCreationTimeString((FILETIME*)&(pinf->CreateTime));
  lvi.iSubItem=Prcclnumber[8];lvi.iSubItem--;
  SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 if((!exist || Upd[0]) && Prcclnumber[0]){//Process Name
  if(pinf->ProcessName.Buffer!=0){
   n=pinf->ProcessName.Length;
   if(pinf->Id==0)n=getstrlenW(pinf->ProcessName.Buffer);
   UnicodeStringToAnsiString(pinf->ProcessName.Buffer,n,pvbuf1,524);
   lvi.pszText=pvbuf1;
  }
  else if(pinf->Id==0)lvi.pszText=smemTable->SysIdleProc;
  lvi.iSubItem=Prcclnumber[0];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 if((!exist || Upd[1]) && Prcclnumber[1]){//Process ID
  if(!pinf->bPseudoProcess){ltoa(pinf->Id,pvbuf1,10);lvi.pszText=pvbuf1;}
  else {lvi.pszText=smemTable->n_a;}
  lvi.iSubItem=Prcclnumber[1];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 lvi.pszText=pvbuf1;
 if((!exist || Upd[2]) && Prcclnumber[2]){//Parent Process ID
  ltoa(pinf->ParentProcessID,pvbuf1,10);
  lvi.iSubItem=Prcclnumber[2];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 ///////////// ВСЕ ltoa  и  FormatNumString надо сделать авто выбор Kb,Mb....
 if(!spi)goto skip_contextsw;
 if(Prcclnumber[7]){
  if(pinf->ThreadCount!=spi->ThreadCount){if(ds==Prcclnumber[7])bReSort=1;goto set_cmn7;}
  if(!Upd[7])goto skip_cmn7;
set_cmn7:
  ltoa(pinf->ThreadCount,pvbuf1+100,10); FormatNumString(pvbuf1+100,pvbuf1);
  lvi.iSubItem=Prcclnumber[7];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
 }
skip_cmn7:
 BOOL iswowtask;
 if(pinf->Flags&PINF_FLAG_WOW_TASK){iswowtask=1; goto wowtask_skip;} else iswowtask=0;
 if(Prcclnumber[11]){
  if(pinf->HandleCount!=spi->HandleCount){if(ds==Prcclnumber[11])bReSort=1;goto set_cmn11;}
  if(!Upd[11])goto skip_cmn11;
set_cmn11:
  ltoa(pinf->HandleCount,pvbuf1+100,10); FormatNumString(pvbuf1+100,pvbuf1);
  lvi.iSubItem=Prcclnumber[11];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
skip_cmn11:
 if(Prcclnumber[12]){
  if(pinf->VirtualSize!=spi->VmCounters.VirtualSize){if(ds==Prcclnumber[12])bReSort=1;goto set_cmn12;}
  if(!Upd[12])goto skip_cmn12;
set_cmn12:
  ltoa(pinf->VirtualSize/1024,pvbuf1+100,10); FormatNumString(pvbuf1+100,pvbuf1); strappend((char*)pvbuf1,_K);
  lvi.iSubItem=Prcclnumber[12];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
skip_cmn12:
 if(Prcclnumber[13]){
  if(pinf->PagefileUsage!=spi->VmCounters.PagefileUsage){if(ds==Prcclnumber[13])bReSort=1;goto set_cmn13;}
  if(!Upd[13])goto skip_cmn13;
set_cmn13:
  ltoa(pinf->PagefileUsage/1024,pvbuf1+100,10); FormatNumString(pvbuf1+100,pvbuf1); strappend((char*)pvbuf1,_K);
  lvi.iSubItem=Prcclnumber[13];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
skip_cmn13:
 if(Prcclnumber[14]){
  if(pinf->WorkingSetSize!=spi->VmCounters.WorkingSetSize){if(ds==Prcclnumber[14])bReSort=1;goto set_cmn14;}
  if(!Upd[14])goto skip_cmn14;
set_cmn14:
  ltoa(pinf->WorkingSetSize/1024,pvbuf1+100,10); FormatNumString(pvbuf1+100,pvbuf1); strappend((char*)pvbuf1,_K);
  lvi.iSubItem=Prcclnumber[14];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
skip_cmn14:
 if(Prcclnumber[15]){
  if(pinf->CommitCharge!=spi->CommitCharge){if(ds==Prcclnumber[15])bReSort=1;goto set_cmn15;}
  if(!Upd[15])goto skip_cmn15;
set_cmn15:
  ltoa(pinf->CommitCharge/1024,pvbuf1+100,10); FormatNumString(pvbuf1+100,pvbuf1); strappend((char*)pvbuf1,_K);
  lvi.iSubItem=Prcclnumber[15];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
skip_cmn15:
wowtask_skip:
 if(Prcclnumber[6]){
  if(pinf->BasePriority!=spi->BasePriority && ds==Prcclnumber[6])bReSort=1;
 }
 if(!exist || (exist && (pinf->CpuTime.QuadPart!=spi->KernelTime.QuadPart || pinf->Flags&PINF_FLAG_UPDATE_CPU || Upd[4]))){
  if(Prcclnumber[4]){//CPU usage
   double d2;
   if(oldIdleTm!=0 && spi->KernelTime.QuadPart>0){
    d2=pinf->CpuTime.QuadPart-spi->KernelTime.QuadPart;
    if(pinf->Id==0){d2=100.0-cpuUsage;}//idle from processor_times
    else{
     if(dbSystemTime!=0.0){d2/=dbSystemTime; d2*=100.0;}
     else d2=0;
    }
    if(progSetts.CheckState[12] && d2<0.005){*pvbuf1=0; goto set_cpuusage_text;}
    if(progSetts.CheckState[11]){cpuToFractStr(d2,pvbuf1,lta);}
    else{
     if((d2-(DWORD)d2)>=0.5)d2+=0.5;
     ltoa((DWORD)d2,pvbuf1,10);
    }
   }
   else {
    if(progSetts.CheckState[12])*pvbuf1=0;
    else if(progSetts.CheckState[11]){*(DWORD*)pvbuf1=0x30302E30; *((DWORD*)pvbuf1+1)=0;}//set "0.00"
    else *(DWORD*)pvbuf1=0x0030;
   }
set_cpuusage_text:
   lvi.iSubItem=Prcclnumber[4];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
   if(pinf->prevCpuUsage!=d2 && ds==Prcclnumber[4])bReSort=1;
   pinf->prevCpuUsage=d2;
  }
  if(Prcclnumber[9]){//CPU (kernel+user) Time
   ftm.dwLowDateTime=pinf->CpuTime.LowPart;
   ftm.dwHighDateTime=pinf->CpuTime.HighPart;
   FileTimeToSystemTime(&ftm,&stm);
   getTime(&stm,pvbuf1,1);
   lvi.iSubItem=Prcclnumber[9];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
   if(pinf->CpuTime.QuadPart!=spi->KernelTime.QuadPart && ds==Prcclnumber[9])bReSort=1;
  }
  if(pinf->CpuTime.QuadPart==spi->KernelTime.QuadPart && pinf->Flags&PINF_FLAG_UPDATE_CPU){pinf->Flags&=~PINF_FLAG_UPDATE_CPU;}
  else pinf->Flags|=PINF_FLAG_UPDATE_CPU;
 }
 if((Prcclnumber[21] || Prcclnumber[22]) && !iswowtask){//ContextSwitches
  unsigned __int64 uu; uu=0;
  if(!pinf->bPseudoProcess){for(n=1;n<=pinf->ThreadCount;n++){uu+=spi->Threads[n].ContextSwitches;}}
  else{
   if(pinf->Id==PINF_PSEUDOPROCESS_ID_INT){
    SYSTEM_PROCESSOR_TIMES *pspt;
    pspt=sysProcessorTm; for(n=0;n<(DWORD)SysBasicInf->NumberProcessors;n++){uu+=pspt->InterruptCount; pspt++;}
   }
   else if(pinf->Id==PINF_PSEUDOPROCESS_ID_DPC && ntdllFunctions.pNtQuerySystemInformation(SystemProcessorStatistics,pvbuf1,524,&n)==0){
    uu=((SYSTEM_PROCESSOR_STATISTICS*)pvbuf1)->DpcCount;//надо per cpu
   }
   if(uu==0)goto skip_contextsw;
  }
  if(Prcclnumber[21]){//Context Switches
   _i64toa(uu,pvbuf1+100,10); FormatNumString(pvbuf1+100,(char*)pvbuf1);
   lvi.iSubItem=Prcclnumber[21];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
   if(pinf->prevContextSwitches!=uu && ds==Prcclnumber[21])bReSort=1;
  }
  if(Prcclnumber[22] && !firsttime){//ContextSwitches Delta
   n=(DWORD)(uu-pinf->prevContextSwitches);
   if(n>0){ltoa(n,pvbuf1+100,10); FormatNumString(pvbuf1+100,(char*)pvbuf1);}
   else *pvbuf1=0;
   lvi.iSubItem=Prcclnumber[22];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
   if(pinf->prevCSwDelta!=n && ds==Prcclnumber[22])bReSort=1;
   pinf->prevCSwDelta=n;
  }
  pinf->prevContextSwitches=uu;
 }
skip_contextsw:
 if((!exist || Upd[20] || (spi && spi->Reserved01==0xFFAAFF15)) && Prcclnumber[20]){
  if(pinf->SessionId!=(DWORD)-1){
   ltoa(pinf->SessionId,pvbuf1,10);
   lvi.iSubItem=Prcclnumber[20];lvi.iSubItem--; SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);
  }
 }
 lvi.pszText=LPSTR_TEXTCALLBACK; lvi.mask=LVIF_TEXT;
 for(n=0;n<COLUMNS_COUNT;n++){
  if(Prcclnumber[n] && bUpd[n]){lvi.iSubItem=Prcclnumber[n];lvi.iSubItem--;SendMessage(hProcessList,LVM_SETITEM,0,(LPARAM)&lvi);}
 }
 if(pinf->Flags&PINF_FLAG_UPDATE_ITEM){
  pinf->Flags&=~PINF_FLAG_UPDATE_ITEM;
  SendMessage(hProcessList,LVM_UPDATE,lvi.iItem,0);
 }
 lvi.iItem++;
 plinf=pinf;
next_pinf:
 pinf=pinf->NextEntry;
}
if(processesUpd){processesUpd=0;bLoadIcons=1;}
if(bReSortList){bReSortList=0;bReSort=1;}
if(bReSort && progSetts.ProcsSortd.SortDirection!=2)SendMessage(hProcessList,LVM_SORTITEMS,(WPARAM)&(progSetts.ProcsSortd),(LPARAM)Processes_ListCompareFunc);
if(firsttime)bReSortList=1;
}

LRESULT Processes_LViewCustomDraw(LPARAM lParam){PROCESS_INFO *pinf; LPNMLVCUSTOMDRAW lplvcd;
 lplvcd=(LPNMLVCUSTOMDRAW)lParam;
 switch(lplvcd->nmcd.dwDrawStage){
  case CDDS_PREPAINT: return CDRF_NOTIFYITEMDRAW;
  case CDDS_ITEMPREPAINT:
   pinf=(PROCESS_INFO*)lplvcd->nmcd.lItemlParam;
   if(!IsVALID_PROCESS_INFO(pinf))break;
   if(pinf->Flags&PINF_FLAG_HIDDENPROC){
    lplvcd->clrText=0x00ffffff; lplvcd->clrTextBk=0x005500f2;
   }
   else if(pinf->Flags&PINF_FLAG_NEW_PROCESS){
    lplvcd->clrText=0; lplvcd->clrTextBk=0x00ff00;
   }
   else if(pinf->Flags&PINF_FLAG_WOW_TASK){
    lplvcd->clrText=0; lplvcd->clrTextBk=0xfff0ec;
   }
   else if(pinf->Flags&PINF_FLAG_SUSPENDED){
    lplvcd->clrText=0; lplvcd->clrTextBk=0xc0c0c0;
   }
   if(pinf->dwStatus>1 && !pinf->bPseudoProcess){
    lplvcd->clrText=0; lplvcd->clrTextBk=0x0000ff;
   }
   break;
 }
 return CDRF_DODEFAULT;
}

void Processes_ResizeWin(int newsz,int newwd){double dd; HDWP hdwp,hdwpcur;
 GetClientRect(hProcessList,&clt);
 hdwp=BeginDeferWindowPos(2); newsz-=2;
 dd=newsz;
 if(paneType){dd*=0.5; dd-=1.5;}
 hdwpcur=DeferWindowPos(hdwp,hProcessList,0,0,0,newwd,dd,SWP_NOZORDER|SWP_NOMOVE);
 if(paneType){
  newsz-=dd; dd+=2.5;
  DeferWindowPos(hdwpcur,(paneType==DLLpane)?hModuleList:hHandleList,0,0,newsz,newwd,dd,SWP_NOZORDER);
 }
 EndDeferWindowPos(hdwp);
}

//////////////////////////////// Processes_Proc ////////////////////////////////
BOOL CALLBACK Processes_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){LV_ITEM lvi; DWORD dn; MENUITEMINFO mii; HWND htmp;
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
  if(Message==WM_INITMENUPOPUP || Message==WM_UNINITMENUPOPUP || Message==WM_MEASUREITEM || Message==WM_DRAWITEM || Message==WM_ENTERMENULOOP || Message==WM_EXITMENULOOP){
   ModernMenu_DrawMenuProc(hDlg,Message,wParam,lParam);
  }
 }
 switch(Message){
  case WM_INITDIALOG:
   processes_win=hDlg;
   MainTab_OnChildDialogInit(hDlg);
   pHdr->hwndDisplay=hDlg;
   if(!firsttime){processesUpd=1; bLoadIcons=1;}
   int s,i; DWORD cnt;
   Processes_BottomPane_Show(1);//LVS_NOSORTHEADER
   himgl=ImageList_Create(16,16,ILC_COLORDDB,0,255);
   ImageList_SetBkColor(himgl,0x00ffffff);
   ImageList_AddIcon(himgl,LoadIcon(gInst,(LPTSTR)4));
   ImageList_AddIcon(himgl,LoadIcon(gInst,(LPTSTR)21));
   hProcessList=CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,0,LVS_SINGLESEL|LVS_SHOWSELALWAYS|LVS_REPORT|WS_CHILD|WS_VISIBLE,0,2,strect.right,strect.top,hDlg,(HMENU)101,gInst,NULL);
   lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
   cnt=0; RtlZeroMemory(Prcclnumber,sizeof(Prcclnumber));
   for(i=1;i<PRCCOLUMNS_COUNT;i++){
    for(int t=0;t<PRCCOLUMNS_COUNT;t++){
     if(ColumnsCreateOrder[t]==i && smemTable->ColumnTitle[t][0]){
      if(ColumnsAlign[t]==DT_LEFT)lvcol.fmt=LVCFMT_LEFT;
      else if(ColumnsAlign[t]==DT_RIGHT)lvcol.fmt=LVCFMT_RIGHT;
      lvcol.cx=ColumnWidth[t]; lvcol.pszText=smemTable->ColumnTitle[t]; lvcol.iSubItem=t;
      s=SendMessage(hProcessList,LVM_INSERTCOLUMN,(WPARAM)i,(LPARAM)&lvcol);
      if(s>=0){Prcclnumber[t]=s; Prcclnumber[t]++; cnt++;}
     }
    }
   }
   SendMessage(hProcessList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_HEADERDRAGDROP|LVS_EX_FLATSB|LVS_EX_LABELTIP);
   if(bValidProcsOrderArray)SendMessage(hProcessList,LVM_SETCOLUMNORDERARRAY,(WPARAM)cnt,(LPARAM)&ProcsOrderArray);
   ListView_SetImageList(hProcessList,himgl,LVSIL_SMALL);
   if(progSetts.ProcsSortd.CurSubItem!=(DWORD)-1)SetSortArrowIcon(hProcessList,(DWORD)-1,progSetts.ProcsSortd.CurSubItem,progSetts.ProcsSortd.SortDirection);
   CheckMarkMenuItems();
   if(hVdmDbgLib==NULL){//Need GetSysDir to be sure dll is an system's
    hVdmDbgLib=LoadLibrary(vdmdbgdll);
    if(hVdmDbgLib!=NULL){
     VDMEnumProcessWOW=(INT(WINAPI*)(PROCESSENUMPROC,LPARAM))GetProcAddress(hVdmDbgLib,VDMEnumProcessWOW_txt);
     VDMEnumTaskWOWEx=(INT(WINAPI*)(DWORD,TASKENUMPROCEX,LPARAM))GetProcAddress(hVdmDbgLib,VDMEnumTaskWOWEx_txt);
     VDMTerminateTaskWOW=(BOOL(WINAPI*)(DWORD,WORD))GetProcAddress(hVdmDbgLib,VDMTerminateTaskWOW_txt);
     if(VDMEnumProcessWOW==NULL || VDMEnumTaskWOWEx==NULL){VDMEnumProcessWOW=NULL; FreeLibrary(hVdmDbgLib); hVdmDbgLib=0;}
    }
   }
   Item=0; itemSel=(DWORD)-1;
   ControlUpdateThread(UPDATETHREAD_UPDATE,0xFF,0);
   SetFocus(hProcessList);
   break;
  case WM_SIZE:
   Processes_ResizeWin(HIWORD(lParam),LOWORD(lParam));
   break;
  case WM_DESTROY:
   if(ImageList_Destroy(himgl))himgl=0;
   SaveColumns_Processes();
   if(hProcessList!=0){DestroyWindow(hProcessList); hProcessList=0;}
   if(hdnList!=0){DestroyWindow(hdnList); hdnList=0;}
   if(lPaneThread){
    n=0; GetExitCodeThread(lPaneThread,&n);
    if(n==STILL_ACTIVE)PostThreadMessage(lPaneThreadId,WM_QUIT,0,0);
    CloseHandle(lPaneThread);lPaneThread=0;
   }
   if(hHandleList!=0){DestroyWindow(hHandleList);hHandleList=0;}
   if(hModuleList!=0){
    SaveColumns_ProcessesDlls();
    DestroyWindow(hModuleList);
    hModuleList=0;
   }
   int c; for(c=47001;c<=47004;c++)DeleteMenu(main_menu,c,MF_BYCOMMAND);
   processes_win=0; pidSel=(DWORD)-1;
   break;
case WM_COMMAND: wID=LOWORD(wParam);
 switch(wID){
  case 40013: Process_TryKill(); break;
  case 50010: if(paneType!=0)Processes_BottomPane_Update(-5,0,-1); break;
  case 30004:
   //turn off the bottom pane
   paneType=0;
   Processes_BottomPane_Show(0);
   progSetts.CheckState[4]=MFS_CHECKED;
   progSetts.CheckState[5]=MFS_UNCHECKED; progSetts.CheckState[6]=MFS_UNCHECKED;
   mii.fMask=MIIM_STATE;
   mii.cbSize=sizeof(MENUITEMINFO);
   for(i=4;i<7;i++){
    mii.fState=progSetts.CheckState[i];
    SetMenuItemInfo(main_menu,30000+i,0,&mii);
   }
   break;
  case 30005:
   if(paneType==DLLpane && hModuleList!=NULL && IsWindow(hModuleList)){
    paneType=0; progSetts.CheckState[5]=MFS_UNCHECKED;
    progSetts.CheckState[4]=MFS_CHECKED;
   }
   else {
    paneType=DLLpane; progSetts.CheckState[5]=MFS_CHECKED;
    progSetts.CheckState[4]=MFS_UNCHECKED;
   }
   progSetts.CheckState[6]=MFS_UNCHECKED;
   Processes_BottomPane_Show(0);
   mii.fMask=MIIM_STATE;
   mii.cbSize=sizeof(MENUITEMINFO);
   for(i=4;i<7;i++){
    mii.fState=progSetts.CheckState[i];
    SetMenuItemInfo(main_menu,30000+i,0,&mii);
   }
   break;
  case 30006:
   if(paneType==HANDLEpane && hHandleList!=NULL && IsWindow(hHandleList)){//turn off a bottom pane
    paneType=0; progSetts.CheckState[6]=MFS_UNCHECKED;
    progSetts.CheckState[4]=MFS_CHECKED;
   }
   else {//turn on a bottom pane
    paneType=HANDLEpane; progSetts.CheckState[6]=MFS_CHECKED;
    progSetts.CheckState[4]=MFS_UNCHECKED;
   }
   progSetts.CheckState[5]=MFS_UNCHECKED;
   Processes_BottomPane_Show(0); //SetFocus(hProcessList);
   mii.fMask=MIIM_STATE;
   mii.cbSize=sizeof(MENUITEMINFO);
   for(i=4;i<7;i++){
    mii.fState=progSetts.CheckState[i];
    SetMenuItemInfo(main_menu,30000+i,0,&mii);
   }
   break;
  //case 30012: checkMenuItem(12); Upd[4]=1; break;//CPU Usage Normal/float
  //case 30013: checkMenuItem(13); Upd[6]=1; break;//Priority Num/Str
  case 40018: case 40019:
   int in;
   in=SendMessage(hModuleList,LVM_GETSELECTIONMARK,0,0);
   if(in!=-1 && SendMessage(hModuleList,LVM_GETITEMSTATE,in,LVIS_SELECTED)==0)in=-1;
   if(in==-1)break;
   lvi.mask=LVIF_PARAM; lvi.cchTextMax=1024; lvi.iSubItem=0; lvi.iItem=in;
   if(SendMessage(hModuleList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   MODULE_INFOBUF *mdi; mdi=(MODULE_INFOBUF*)lvi.lParam;
   if(*(DWORD*)(mdi->str1)==0xFFFFFF00)break;
   if(mdi->String[0]==0)break;
   char *pbuf; pbuf=CorrectFilePath((char*)(mdi->String[0]),0,t0nbuf2);
   if(wID==40018)File_OpenProperies(pbuf);
   else {
    *((char*)t0nbuf4)='\"';
    copystring((char*)t0nbuf4+1,pbuf);
    strappend((char*)t0nbuf4,"\"");
    if((DWORD)ShellExecute(NULL,open_txt,dependsPath,(char*)t0nbuf4,0,SW_SHOWNORMAL)>32){
     if(progSetts.CheckState[0] && progSetts.CheckState[20])ShowWindow(main_win,SW_MINIMIZE);
    }
   }
   break;
  case 40020://Close ObjHandle
   if((lvi.iItem=SendMessage(hHandleList,LVM_GETSELECTIONMARK,0,0))==-1)break;
   lvi.mask=LVIF_PARAM; lvi.lParam=0;
   if(SendMessage(hHandleList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   HANDLE_INFOBUF *hif; hif=(HANDLE_INFOBUF*)lvi.lParam;
   if(CloseObjHandle(hDlg,(HANDLE)(hif->HandleValue),hif->Flags,hif->ProcessId))Processes_BottomPane_Update(-5,0,lvi.iItem);
   break;
  case 40026://Copy ObjName to clipboard
   int selcnt,sel; DWORD memsz; HWND hwndlist; HGLOBAL clipbuffer; char *buffer;
   if(hHandleList==0 && hModuleList==0)break;
   if(hHandleList)hwndlist=hHandleList;
   else hwndlist=hModuleList;
   selcnt=SendMessage(hwndlist,LVM_GETSELECTEDCOUNT,0,0);
   if(selcnt==0 || !OpenClipboard(hDlg))break;
   sel=selcnt; memsz=0; lvi.mask=LVIF_PARAM; lvi.iItem=-1;
   ///
   if(hHandleList){
    while(selcnt){
     if((lvi.iItem=SendMessage(hHandleList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)goto close_clipb;
     if(SendMessage(hHandleList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
      memsz+=getstrlen(((HANDLE_INFOBUF*)(lvi.lParam))->ObjName); memsz+=3;
     }
     selcnt--;
    }
   }
   else {//i.e. if(hModuleList)
    while(selcnt){
     if((lvi.iItem=SendMessage(hModuleList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)goto close_clipb;
     if(SendMessage(hModuleList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
      memsz+=getstrlen((char*)((MODULE_INFOBUF*)(lvi.lParam))->String[0]); memsz+=3;
     }
     selcnt--;
    }
   }
   ///
   if((clipbuffer=GlobalAlloc(GMEM_DDESHARE,memsz))==0)goto close_clipb;
   buffer=(char*)GlobalLock(clipbuffer); *buffer=0; lvi.iItem=-1;
   ///
   if(hHandleList){
    for(selcnt=sel;selcnt;selcnt--){
     if((lvi.iItem=SendMessage(hHandleList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)break;
     if(SendMessage(hHandleList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
      buffer+=copystring(buffer,((HANDLE_INFOBUF*)(lvi.lParam))->ObjName);
      buffer+=copystring(buffer,"\x0D\x0A");
     }
    }
   }
   else {//i.e. if(hModuleList)
    for(selcnt=sel;selcnt;selcnt--){
     if((lvi.iItem=SendMessage(hModuleList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)break;
     if(SendMessage(hModuleList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
      buffer+=copystring(buffer,(char*)((MODULE_INFOBUF*)(lvi.lParam))->String[0]);
      buffer+=copystring(buffer,"\x0D\x0A");
     }
    }
   }
   buffer-=2; *buffer=0;
   EmptyClipboard();
   GlobalUnlock(clipbuffer);
   SetClipboardData(CF_TEXT,clipbuffer);
 close_clipb:
   CloseClipboard();
   return 1;
  case 40030:
   CreateColumnSelDlg(0);
   return 1;
  case 40027:
   PROCESS_INFO *pinf;
   if((pinf=Processes_GetCurrentPINF())==0)break;
   Process_Restart(pinf->Id);
   return 1;
  case 47003:
   if(!bShowAllProcesses){bShowAllProcesses=1; processesUpd=1;}
   else {bShowAllProcesses=0;}
   ControlUpdateThread(UPDATETHREAD_UPDATE,0xFF,0);
   return 1;
 }
 if((wID>=40015 && wID<=40017) || (wID>40021 && wID<=40033) || (wID>=40127 && wID<=40133)){
contextmenu_click:
  if((itemSel=SendMessage(hProcessList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
  lvi.mask=LVIF_PARAM; lvi.iItem=itemSel; lvi.lParam=0;
  if(SendMessage(hProcessList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi)==0)break;
  PROCESS_INFO *pinf;
  pinf=(PROCESS_INFO*)lvi.lParam;
  if(!IsVALID_PROCESS_INFO(pinf))break;
  if(pinf->Flags&PINF_FLAG_DONTSHOWPROPS)break;
  pidSel=pinf->Id; HANDLE hProcess; DWORD ErrCode;
  switch(wID){
   case 40015:
    if((hProcess=OpenProcess(PROCESS_ALL_ACCESS,FALSE,pidSel))!=0){
     if((pinf->Flags&PINF_FLAG_SUSPENDED)==0)ErrCode=ntdllFunctions.pNtSuspendProcess(hProcess);
     else ErrCode=ntdllFunctions.pNtResumeProcess(hProcess);
     CloseHandle(hProcess);
     if(ErrCode)FailMessage(smemTable->UnblSuspResume,ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
    }
    else FailMessage(smemTable->UnblOpenProcess,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
    break;
   case 40016://Launch Depends
    char *pbuf;
    if(pinf->ImagePathName.Length!=0){
     pbuf=CorrectFilePath(pinf->ImagePath,0,t0nbuf2);
    }
    else break;//if(GetProcessPathById(pinf->Id,&pbuf)==0)break;
    *((char*)t0nbuf4)='\"';
    lstrcat(pbuf,"\"");
    lstrcpy((char*)t0nbuf4+1,pbuf);
    ErrCode=(DWORD)ShellExecute(NULL,open_txt,dependsPath,(char*)t0nbuf4,0,SW_SHOWNORMAL);
    if(ErrCode<33)FailMessage(ShellExecute_failed_txt,ErrCode,FMSG_NO_INFO|FMSG_SHOW_MSGBOX);
    else if(progSetts.CheckState[0] && progSetts.CheckState[20])ShowWindow(main_win,SW_MINIMIZE);
    break;
   case 40017: CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)ProcInfoThread,(void*)pidSel,0,&n)); break;
   case 40022: case 40023: case 40024: case 40025:
    if((hProcess=OpenProcess(PROCESS_SET_INFORMATION,FALSE,pidSel))!=NULL){
     if(wID==40022)n=REALTIME_PRIORITY_CLASS;
     else if(wID==40023)n=HIGH_PRIORITY_CLASS;
     else if(wID==40024)n=NORMAL_PRIORITY_CLASS;
     else if(wID==40025)n=IDLE_PRIORITY_CLASS;
     SetPriorityClass(hProcess,n);
     CloseHandle(hProcess);
    }
    break;
   case 40127: case 40128: case 40129: case 40130: case 40131: case 40132: case 40133:
    switch(wID){
     case 40127: n=9; break;
     case 40128: n=4; break;
     case 40129: n=3; break;
     case 40130: n=8; break;
     case 40131: n=1; break;
     case 40132: n=2; break;
     case 40133: n=10; break;
    }
    Process_MainWin_Action(pinf,n);
    break;
  }
 }
 break;
case WM_NOTIFY:
 POINT curpos; DWORD dCode; dCode=((NMHDR*)lParam)->code;
 if(dCode==NM_CLICK || dCode==NM_RCLICK){
  HWND hfrom;
  itemSel=((NMLISTVIEW*)lParam)->iItem;
  hfrom=((NMHDR*)lParam)->hwndFrom;
  lvi.mask=LVIF_PARAM; lvi.iItem=itemSel; lvi.lParam=0;
  SendMessage(hfrom,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi);
  if(hfrom==hProcessList){
   dn=((itemSel==-1)?MF_GRAYED:MF_ENABLED); dn|=MF_BYCOMMAND;
   EnableMainMenuItem(47001,dn);
   Processes_BottomPane_Update(itemSel,1,-1);
  }
  if(dCode!=NM_RCLICK)break;
  if(itemSel==-1)goto chkHeaderClick;
  GetCursorPos(&curpos);
mouse_Rclick:
  if(hfrom==hProcessList){
   HMENU pmenu;
   pmenu=GetSubMenu(main_menu,2);//BuildPopupMenu(Menus[3]);
   SetMenuDefaultItem(pmenu,40017,FALSE);
   ProcessContextMenu_SetItemsState(pmenu);
   TrackPopupMenuEx(pmenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos.x,curpos.y,hDlg,NULL);
   SetMenuDefaultItem(pmenu,-1,FALSE);//DestroyMenu(pmenu);
  }
  else if(((NMHDR*)lParam)->idFrom==102){
   if(*(DWORD*)(((MODULE_INFOBUF*)lvi.lParam)->str1)==0xFFFFFF00)break;
   FileModule_PopupMenu(hDlg,&curpos,paneType);
  }
  else{
 chkHeaderClick:
   GetCursorPos(&curpos);
   htmp=((NMHDR*)lParam)->hwndFrom;
   CheckHeaderClick(GetParent(htmp),htmp,&curpos);
  }
  break;
 }
 if(((NMHDR*)lParam)->hwndFrom==hProcessList){
  if(dCode==NM_CUSTOMDRAW){SetWindowLongPtr(hDlg,DWLP_MSGRESULT,(DWORD_PTR)Processes_LViewCustomDraw(lParam)); return 1;}
  if(dCode==LVN_GETDISPINFO){
   NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam;
   PROCESS_INFO *pinf; pinf=(PROCESS_INFO*)(nvd->item.lParam);
   if(!IsVALID_PROCESS_INFO(pinf))break;
   //if(hif==0 || nvd==0)break; //не надо
   if(nvd->item.mask&LVIF_TEXT){
    int iSub; iSub=nvd->item.iSubItem; iSub++;
    if(iSub==Prcclnumber[3])nvd->item.pszText=pinf->ImagePath;
    else if(iSub==Prcclnumber[5]){
     nvd->item.pszText=pinf->UserName;
     if(progSetts.CheckState[38]==0 && nvd->item.pszText!=0){
      while(*nvd->item.pszText!=0 && *nvd->item.pszText!='\\')nvd->item.pszText++;
      if(*nvd->item.pszText=='\\')nvd->item.pszText++;
      else nvd->item.pszText=pinf->UserName;
     }
    }
    else if(iSub==Prcclnumber[6])nvd->item.pszText=pinf->PriorityText;
    else if(iSub==Prcclnumber[10])nvd->item.pszText=pinf->CommandLine;
    else if(iSub==Prcclnumber[16])nvd->item.pszText=pinf->Description;
    else if(iSub==Prcclnumber[17])nvd->item.pszText=pinf->Version;
    else if(iSub==Prcclnumber[18])nvd->item.pszText=pinf->CompanyName;
    if(nvd->item.pszText==0){
     if(!firsttime)nvd->item.pszText=smemTable->n_a;
     else nvd->item.pszText="";
    }
   }
   else if(nvd->item.mask&LVIF_IMAGE){
    if(nvd->item.iSubItem==0)nvd->item.iImage=pinf->iIcon;
   }
   break;
  }
  else if(dCode==LVN_KEYDOWN){
   switch(((LPNMLVKEYDOWN)lParam)->wVKey){
    case VK_DELETE: Process_TryKill(); break;
    case VK_TAB:
     if(paneType==0)break;
     htmp=(paneType==HANDLEpane)?hHandleList:hModuleList;
     SetFocus(htmp);
     int in; in=SendMessage(htmp,LVM_GETSELECTIONMARK,0,0);
     if(in!=-1){
      if(SendMessage(htmp,LVM_GETITEMSTATE,in,LVIS_SELECTED)==0)in=-1;
     }
     if(in==-1){if(paneType==1)in=1; else in=0;}
     lvi.stateMask=LVIS_FOCUSED|LVIS_SELECTED; lvi.state=lvi.stateMask;
     SendMessage(htmp,LVM_SETITEMSTATE,(WPARAM)in,(LPARAM)&lvi);
     break;
    case VK_SPACE: Processes_BottomPane_Update(-5,0,-1); break;
    case VK_RETURN: wID=40017; goto contextmenu_click;
   }
   break;
  }
  else if(dCode==LVN_ITEMCHANGED){
   NMLISTVIEW *nmlv; nmlv=(LPNMLISTVIEW)lParam; if(nmlv==0)break;
   if(nmlv->uChanged&LVIF_STATE && nmlv->uNewState&LVIS_SELECTED){
    itemSel=((NMLISTVIEW*)lParam)->iItem;
    dn=((itemSel==-1)?MF_GRAYED:MF_ENABLED); dn|=MF_BYCOMMAND;
    EnableMainMenuItem(47001,dn);
    if(itemSel!=-1){
     ProcessContextMenu_SetItemsState(GetSubMenu(main_menu,2));
    }
   }
  }
  else if(dCode==NM_DBLCLK){wID=40017; goto contextmenu_click;}
 }
 else if(((NMHDR*)lParam)->idFrom==102){
  if(dCode==LVN_KEYDOWN){
   if(((LPNMLVKEYDOWN)lParam)->wVKey==VK_TAB)SetFocus(hProcessList);
  }
  else if(paneType==DLLpane && dCode==NM_CUSTOMDRAW){SetWindowLong(hDlg,DWL_MSGRESULT,(LONG)DllListCustomDraw(lParam)); return 1;}
 }
 if(((NMHDR*)lParam)->hwndFrom==hHandleList){
  if(dCode==LVN_GETDISPINFO){
   NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam;
   HANDLE_INFOBUF *hif; hif=(HANDLE_INFOBUF*)nvd->item.lParam;
   //if(hif==0 || nvd==0)break; //не надо
   if(nvd->item.mask&LVIF_IMAGE)nvd->item.iImage=hif->TypeNum;
   if(nvd->item.mask&LVIF_TEXT){
    if(nvd->item.iSubItem==1)nvd->item.pszText=hif->nmType;
    else if(nvd->item.iSubItem==2)nvd->item.pszText=hif->ObjName;
    if(nvd->item.pszText==0)nvd->item.pszText="";
   }
  }
  else if(dCode==LVN_DELETEITEM){
   lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=((NM_LISTVIEW*)lParam)->iItem;
   if(SendMessage(hHandleList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   HANDLE_INFOBUF *hif; hif=(HANDLE_INFOBUF*)lvi.lParam;
   if(hif->ObjName && hif->ObjName!=smemTable->access_denied)HeapFree(pHeap,0,(void*)hif->ObjName);
   HeapFree(pHeap,0,(void*)lvi.lParam);
  }
 }
 else if(((NMHDR*)lParam)->hwndFrom==hModuleList){
  if(dCode==LVN_GETDISPINFO){
   NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam; MODULE_INFOBUF *mif;
   mif=(MODULE_INFOBUF*)nvd->item.lParam;
   if(!IsVALID_MODULE_INFOBUF(mif))break;
   int iw; iw=nvd->item.iSubItem; iw++;
   if(nvd->item.mask&LVIF_TEXT){
    if(iw==Dllclnumber[0]){if(mif->String[0])nvd->item.pszText=(char*)(mif->String[0])+mif->NameOfs;}
    else if(iw==Dllclnumber[1]){if(*(DWORD*)(mif->str1)!=0xFFFFFF00)nvd->item.pszText=(char*)(mif->String[0]);}
    else if(iw==Dllclnumber[2])nvd->item.pszText=(char*)(mif->String[1]);
    else if(iw==Dllclnumber[3])nvd->item.pszText=(char*)(mif->String[2]);
    else if(iw==Dllclnumber[4])nvd->item.pszText=mif->str1;
    else if(iw==Dllclnumber[5])nvd->item.pszText=mif->str2;
    else if(iw==Dllclnumber[6])nvd->item.pszText=(char*)(mif->String[3]);
    else if(iw==Dllclnumber[7])nvd->item.pszText=mif->str3;
    if(nvd->item.pszText==0)nvd->item.pszText="";
   }
  }
  else if(dCode==LVN_DELETEITEM){
   lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=((NM_LISTVIEW*)lParam)->iItem;
   if(SendMessage(hModuleList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   MODULE_INFOBUF *mif; mif=(MODULE_INFOBUF*)lvi.lParam;
   int i; for(i=0;i<4;i++){if(mif->String[i])HeapFree(pHeap,0,mif->String[i]);}
   if(pFirstModuleInfo==mif)pFirstModuleInfo=0;
   HeapFree(pHeap,0,(void*)lvi.lParam);
  }
 }
 if(dCode==LVN_COLUMNCLICK){
  HWND hfrom; hfrom=((NM_LISTVIEW*)lParam)->hdr.hwndFrom;
  LIST_SORT_DIRECTION *lsd; DWORD flvc;
  if(hfrom==hProcessList){lsd=&(progSetts.ProcsSortd);flvc=(DWORD_PTR)Processes_ListCompareFunc; progSetts.ProcsSortd.bRestoreSort=1;}
  else if(hfrom==hModuleList){lsd=&(progSetts.DllsSortd);flvc=(DWORD_PTR)DllsListCompareFunc; progSetts.DllsSortd.bRestoreSort=1;}
  else if(hfrom==hHandleList){lsd=&(progSetts.HndlsSortd);flvc=(DWORD_PTR)HandleListCompareFunc; progSetts.HndlsSortd.bRestoreSort=0;}
  if(hfrom==hModuleList)lsd->hFrom=(HWND)&Dllclnumber;
  else lsd->hFrom=hfrom;
  ListViewColumnSortClick((NM_LISTVIEW*)lParam,lsd,flvc);
 }
 else if(dCode==NM_RETURN){
  RECT rect; rect.left=LVIR_BOUNDS; HWND hFrom;
  hFrom=((NM_LISTVIEW*)lParam)->hdr.hwndFrom;
  if(hFrom!=hHandleList && hFrom!=hModuleList)break;
  itemSel=GetItemSelected(hFrom);
  if(SendMessage(hFrom,LVM_GETITEMRECT,(WPARAM)itemSel,(LPARAM)&rect)){
   curpos.y=rect.top; rect.bottom-=rect.top; rect.bottom/=2; curpos.y+=rect.bottom;
   curpos.x=rect.right; curpos.x-=rect.left;
   GetWindowRect(hFrom,&rect);
   curpos.y+=rect.top; rect.right-=rect.left;
   if(curpos.x>rect.right)curpos.x=rect.right;
   curpos.x/=2; curpos.x+=rect.left;
   ((NMHDR*)lParam)->code=NM_RCLICK;
   lvi.lParam=0; lvi.mask=LVIF_PARAM; lvi.iItem=itemSel;
   SendMessage(hFrom,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi);
   goto mouse_Rclick;
  }
 }
 break;
}
return 0;}
