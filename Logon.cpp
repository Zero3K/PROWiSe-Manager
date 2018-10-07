HBITMAP hbmpLogon; LPWSTR pszLogonAsUserName;
wchar_t cmdtxtW_runas[]=L"/runas";
wchar_t cmdtxtW_runasadmin[]=L"/runasadmin";

void CmdLine_Determine(LPWSTR lpCmdLineBuffer, void *tmpmem, LPSTR *LngFileName){LPWSTR *pcmd,pwstr,pwptr,*CmdLineArray; int in,cnt,i;
 pszLogonAsUserName=0;
 CmdLineArray=CommandLineToArgvW(lpCmdLineBuffer,&cnt);
 if(CmdLineArray==0)return;
 pcmd=CmdLineArray;
 for(i=0;i<cnt;i++){
  if(lstrcmpiW(*pcmd,L"/nosv")==0)cmd_NoRegSetts=1;
  else if(CompareStrPartW(*pcmd,L"/lng=",5*2,0)==0){
   *pcmd+=5; pwstr=*pcmd; in=0;
   while(*pwstr++)in+=2;
   UnicodeStringToAnsiString(*pcmd,in,(char*)tmpmem,2048);
   *LngFileName=AllocateAndCopy(0,(char*)tmpmem);
  }
  else if(lstrcmpiW(*pcmd,L"/drkey")==0)cmd_Remove=1;
  else if(CompareStrPartW(*pcmd,cmdtxtW_runasadmin,11*2,0)==0){
   cmd_LogonAsAdmin=1;
   *pcmd+=11; pwstr=*pcmd;
   if(*pwstr==L'=')goto runas_username;
   if(*pwstr!=0)cmd_LogonAsAdmin=0;
  }
  else if(CompareStrPartW(*pcmd,cmdtxtW_runas,6*2,0)==0){
   cmd_LogonAs=1;
   *pcmd+=6; pwstr=*pcmd;
   if(*pwstr==L'='){
 runas_username:
    pwstr++;
    if(*pwstr==L'\"')pwstr++;
    pwptr=(LPWSTR)tmpmem;
    while(*pwstr){*pwptr=*pwstr; pwstr++; pwptr++;}
    if(*(pwptr-1)==L'\"')pwptr--;
    *pwptr=0;
    if(pszLogonAsUserName)LocalFree(pszLogonAsUserName);
    if((pszLogonAsUserName=(LPWSTR)LocalAlloc(LMEM_FIXED,(DWORD_PTR)pwptr-(DWORD_PTR)tmpmem+sizeof(WCHAR)))!=0){
     lstrcpyW(pszLogonAsUserName,(LPWSTR)tmpmem);
    }
   }
   else if(*pwstr!=0)cmd_LogonAs=0;
  }
  else if(lstrcmpiW(*pcmd,L"/logon")==0)cmd_Logon=1;
  else if(lstrcmpiW(*pcmd,L"/stplog")==0)LogSteps=1;
  else if(lstrcmpiW(*pcmd,L"/shhtyp")==0)FillHndlTbl=2;
  else if(lstrcmpiW(*pcmd,L"/hide")==0)bHideOnRun=1;
  pcmd++;
 }
 LocalFree(CmdLineArray);
 if(cmd_Logon){
  if(cmd_LogonAs || cmd_LogonAsAdmin){
   cmd_LogonAs=0; cmd_LogonAsAdmin=0;
   if(pszLogonAsUserName)LocalFree(pszLogonAsUserName);
  }
 }
 else if(cmd_LogonAsAdmin){
  LPWKSTA_USER_INFO_1 pwsktui;
  cmd_LogonAs=1;
  if(NetWkstaUserGetInfo(0,1,(LPBYTE*)&pwsktui)==NERR_Success){
   USER_INFO_1 *puinf;
   if(NetUserGetInfo(pwsktui->wkui1_logon_server,pwsktui->wkui1_username,1,(LPBYTE*)&puinf)==NERR_Success){
    if(puinf->usri1_priv==USER_PRIV_ADMIN){
     cmd_LogonAs=0;
     if(pszLogonAsUserName)LocalFree(pszLogonAsUserName);
    }
    NetApiBufferFree(puinf);
   }
   NetApiBufferFree(pwsktui);
  }
 }
}

LPWSTR CmdLine_CreateString(LPWSTR lpCmdLineBuffer, LPWSTR lpCmdsToAppend){LPWSTR *pcmd,*CmdLineArray,pCmdString; int cnt,i,need_sz;
 pcmd=CmdLineArray=CommandLineToArgvW(lpCmdLineBuffer,&cnt);
 if(pcmd!=0){
  pcmd++; cnt--;
  // calc memory size need
  need_sz=0;
  for(i=0;i<cnt;i++){
   need_sz+=lstrlenW(*pcmd)+1+2;//1 is for space, 2 - for ""
   pcmd++;
  }
  if(lpCmdsToAppend)need_sz+=lstrlenW(lpCmdsToAppend);
  need_sz++; need_sz*=sizeof(WCHAR);// plus NULL, convert chars to bytes
  // allocate memory
  if((pCmdString=(LPWSTR)LocalAlloc(LPTR,need_sz))!=0){
   // copy commands to string
   pcmd=CmdLineArray; pcmd++;
   for(i=0;i<cnt;i++){
    lstrcatW(pCmdString,L"\"");
    lstrcatW(pCmdString,*pcmd);
    lstrcatW(pCmdString,L"\" ");
    pcmd++;
   }
   if(lpCmdsToAppend)lstrcatW(pCmdString,lpCmdsToAppend);
   return pCmdString;
  }
  LocalFree(CmdLineArray);
 }
 return 0;
}

void LogOn_TryRunAs(HWND hwndMain){CREDUI_INFOW cdui; LPWSTR lpUserName,lpUser,lpDomain,lpUserPsw,lpCmdLine; BOOL fSave; DWORD dw;
 STARTUPINFOW stpi; PROCESS_INFORMATION pi; RTL_USER_PROCESS_PARAMETERS *rtup;
 if(pCredUIPromptForCredentialsW==0 || pCredUIParseUserNameW==0)return;
 cdui.cbSize=sizeof(CREDUI_INFOW);
 cdui.hwndParent=hwndMain;
 cdui.pszMessageText=0;
 cdui.pszCaptionText=progTitleW;
 cdui.hbmBanner=0;
 if(!IsWindow(main_win))main_win=hwndMain;
 lpUserName=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_USERNAME_LENGTH+1)*2);
 lpUser=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_USERNAME_LENGTH+1)*2);
 lpDomain=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_DOMAIN_TARGET_LENGTH+1)*2);
 lpUserPsw=(LPWSTR)LocalAlloc(LPTR,(CREDUI_MAX_PASSWORD_LENGTH+1)*2);
 dw=CREDUI_MAX_DOMAIN_TARGET_LENGTH+1;
 GetComputerNameExW(ComputerNamePhysicalNetBIOS,lpDomain,&dw);
 if(pszLogonAsUserName)lstrcpynW(lpUserName,pszLogonAsUserName,CREDUI_MAX_USERNAME_LENGTH+1);
 dw=pCredUIPromptForCredentialsW(&cdui,lpDomain,NULL,0,lpUserName,CREDUI_MAX_USERNAME_LENGTH+1,lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH+1,&fSave,CREDUI_FLAGS_INCORRECT_PASSWORD|CREDUI_FLAGS_VALIDATE_USERNAME|CREDUI_FLAGS_DO_NOT_PERSIST);
 if(dw==NO_ERROR){
  dw=pCredUIParseUserNameW(lpUserName,lpUser,CREDUI_MAX_USERNAME_LENGTH+1,lpDomain,CREDUI_MAX_DOMAIN_TARGET_LENGTH+1);
  if(dw==NO_ERROR){
   if(pszLogonAsUserName){
    if(lstrcmpW(pszLogonAsUserName,lpUser)!=0 && lstrcmpW(pszLogonAsUserName,lpUserName)!=0){
     LocalFree(pszLogonAsUserName); pszLogonAsUserName=0;
    }
   }
   RtlZeroMemory(&stpi,sizeof(STARTUPINFOW)); stpi.cb=sizeof(STARTUPINFOW);
   RtlZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
   // Logon and Run
   rtup=my_peb->ProcessParameters;
   lpCmdLine=CmdLine_CreateString(rtup->CommandLine.Buffer,L"/logon");
   if(CreateProcessWithLogonW(lpUser,lpDomain,lpUserPsw,LOGON_WITH_PROFILE,rtup->ImagePathName.Buffer,lpCmdLine,CREATE_DEFAULT_ERROR_MODE,0,rtup->CurrentDirectoryPath.Buffer,&stpi,&pi)!=0){
    WaitForInputIdle(pi.hProcess,5000);
    if(GetExitCodeProcess(pi.hProcess,&dw) && dw==STILL_ACTIVE){
     dwLogonType=LOGON_SUCCESS_EXIT;
     EndDialog(hwndMain,0);
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
   }
   else FailMessage(smemTable->UnableToRunApplication,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_ICONSTOP|FMSG_HIDECODE);
   LocalFree(lpCmdLine);
  }
  else {// CredUIParseUserNameW fails
   if(pszLogonAsUserName){
    if(lstrcmpW(pszLogonAsUserName,lpUserName)!=0){
     LocalFree(pszLogonAsUserName); pszLogonAsUserName=0;
    }
   }
   FailMessage(smemTable->VerifyingUserNameSyntax,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_ICONSTOP);
  }
  RtlZeroMemory(lpUserPsw,CREDUI_MAX_PASSWORD_LENGTH*2);
  RtlZeroMemory(lpUserName,CREDUI_MAX_USERNAME_LENGTH*2);
  RtlZeroMemory(lpUser,CREDUI_MAX_USERNAME_LENGTH*2);
  RtlZeroMemory(lpDomain,CREDUI_MAX_DOMAIN_TARGET_LENGTH*2);
 }
 else if(dw!=ERROR_CANCELLED)FailMessage("Unable to accept credentials information.",0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_ICONSTOP);
 LocalFree(lpUserPsw);
 LocalFree(lpDomain);
 LocalFree(lpUser);
 LocalFree(lpUserName);
}

BOOL CALLBACK LogonDlg_Proc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam){
 switch(uMessage){
  case WM_INITDIALOG:
   SetWindowText(hDlg,smemTable->ProwiseManagerLogon);
   if(dlgTAT)WindowFillText(hDlg,dlgTAT->dlg50);
   hbmpLogon=(HBITMAP)LoadImage(gInst,(char*)14,IMAGE_BITMAP,346,54,LR_DEFAULTCOLOR);
   SendMessage(GetDlgItem(hDlg,101),STM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)hbmpLogon);
   SetWindowPos(hDlg,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
   SetClassLongPtr(hDlg,GCLP_HICON,(DWORD_PTR)LoadIcon(gInst,(char*)1));
   if(lParam==0){
    EnableWindow(GetDlgItem(hDlg,201),0);
    SetTimer(hDlg,1,500,0);
   }
   SetFocus(GetDlgItem(hDlg,201));
   if(IsWindow(main_win)){
    SetWindowLongPtr(hDlg,GWL_STYLE,GetWindowLongPtr(hDlg,GWL_STYLE)|WS_SYSMENU);
    EnableWindow(GetDlgItem(hDlg,202),0);
   }
   ShowWindow(hDlg,SW_SHOW);
   LogOn_TryRunAs(hDlg);
   return 0;
  case WM_COMMAND:
   if(lParam!=0){// i.e. the message is from a control
    switch(LOWORD(wParam)){
     case IDCANCEL:
      dwLogonType=LOGON_CANCEL_EXIT;
      EndDialog(hDlg,0);
      return 1;
     case 202:
      dwLogonType=LOGON_CURRENT_USER;
      EndDialog(hDlg,0);
      return 1;
     case 201:
      LogOn_TryRunAs(hDlg);
      return 1;
    }
   }
   break;
  case WM_TIMER:
   if(wParam==1){
    dwLogonType=LOGON_CURRENT_USER;
    EndDialog(hDlg,0);
    return 1;
   }
   break;
  case WM_CLOSE:
   EndDialog(hDlg,0);
   return 1;
  case WM_DESTROY:
   if(hbmpLogon!=0){DeleteObject(hbmpLogon); hbmpLogon=0;}
   break;
 }
 return 0;
}

BOOL LogOn_ShowLogonDlg(){HINSTANCE hLib; BOOL bl;
 dwLogonType=LOGON_CURRENT_USER; bl=0;
 if((hLib=LoadLibrary(Creduidll_txt))!=0){
  pCredUIPromptForCredentialsW=(DWORD(WINAPI*)(PCREDUI_INFOW,PCWSTR,PCtxtHandle,DWORD,PWSTR,ULONG,PWSTR,ULONG,BOOL*,DWORD))GetProcAddress(hLib,"CredUIPromptForCredentialsW");
  pCredUIParseUserNameW=(DWORD(WINAPI*)(PCWSTR,PWSTR,ULONG,PWSTR,ULONG))GetProcAddress(hLib,"CredUIParseUserNameW");
  if(pCredUIPromptForCredentialsW!=0 && pCredUIParseUserNameW!=0)bl=1;
 }
 else{
  pCredUIPromptForCredentialsW=0; pCredUIParseUserNameW=0;
 }
 DialogBoxParam(gInst,(LPTSTR)50,main_win,LogonDlg_Proc,bl);
 if(hLib!=0){
  FreeLibrary(hLib);
  pCredUIPromptForCredentialsW=0;
  pCredUIParseUserNameW=0;
 }
 LocalFree(pszLogonAsUserName);
 if(dwLogonType==LOGON_SUCCESS_EXIT)return 1;
 return 0;
}

