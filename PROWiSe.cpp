
#define USE_MS_WinMain//Use WINAPI WinMain
//#define BETA_RELEASE//BETA_RELEASE info in AboutBox
#define VER_FINAL_RELEASE//VER_FINAL_RELEASE i.e. not test
//#define SETTS_REGKEY_HIDE//Use Ntkey.Size to disable access through WinAPI
//#define ENABLE_JOB_INFO//ENABLE_JOB_INFO and enable in .rc
#define IS_HM_VERSION//Is_HomeVer
//#define LOG_STEPS

#ifdef _DEBUG
 #define DISABLE_MYTOPLEVEL_EXCPTNHANDLER
 //#define WRITE_DATA_BIN//WRITE_DATA_BIN
#endif

#define LNGFILE_COMPATIBILITY_FLAG 14082006//LNGFILE_COMPATIBILITY_FLAG
#define LNGFILE_COMPATIBILITY_OLDFLAG 14082006

#define THIS_VERSION_PROGSETTS_COMPATABILITY_SIGN 0x0180//Для недопущения использования настроек предыдущей версии ;  last=0x1022
#define THIS_VERSION_PROGSETTS_VERSION 0x0180//Для распознавания версий
#define THIS_VERSION_PROGSETTS_SIGN ( (THIS_VERSION_PROGSETTS_VERSION<<16) | THIS_VERSION_PROGSETTS_COMPATABILITY_SIGN )

#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x501
#define WINVER 0x0500
#define SECURITY_WIN32

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <ntsecapi.h>
#include <Security.h>
#include <lm.h>
#include <lmcons.h>
#include <WinCred.h>

#include "..\htmlhlp.h"
#include "Definitions.h"
#include "crc32.h"
#include "..\NTstruct.h"
#include "TextStrings.h"
#include "HeaderStructs.h"
#include "Variables.h"
#include "FuncsDefines.h"

#include "StringF.cpp"// Функции для работы со строками
#include "HeadFuncs.cpp"
#include "Errors_Logs.cpp"
#include "HookCreateWindow.cpp"
#include "HeadFuncs2.cpp"
#include "resources.cpp"
#include "CPU_Usage.cpp"
#include "SaveColumns.cpp"
#include "Logon.cpp"
#include "ListViews.cpp"

#ifndef USE_MS_WinMain
 #define WinMain
#endif

BOOL IsAnother(BOOL bRemove){HANDLE hMutex=CreateMutex(0,0,(!bRemove)?progMutex:progUnInstMutex); return(hMutex==0 || GetLastError()==ERROR_ALREADY_EXISTS);}

HGLOBAL hg1; HMENU pmenu; HGLOBAL hg2;
char caption[10];
int Item=0,smret;
RECT clt,rect1,rect2,rect3;
LV_COLUMN lvcol;
bool showprocinfIsRun=0;
DWORD hgsz2,hgsz3,hgsz4;
DWORD pidSel=0;
DWORD n;
OPENFILENAME of;
char HeapCreate_txt[]="HeapCreate";

bool EnablePrivelege(char *privelegeName,DWORD dwState){LUID luid; HANDLE hToken; TOKEN_PRIVILEGES tkp; bool ret;
 OpenProcessToken((HANDLE)-1,TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken);
 tkp.PrivilegeCount=1;
 LookupPrivilegeValue(NULL,privelegeName,&luid);
 tkp.Privileges[0].Luid=luid;
 tkp.Privileges[0].Attributes=dwState;
 AdjustTokenPrivileges(hToken,FALSE,&tkp,NULL,(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL);
 ret=(ntdllFunctions.RtlGetLastWin32Error()==ERROR_SUCCESS);
 CloseHandle(hToken);
 return ret;
}

void ControlUpdateThread(DWORD Msg,WPARAM wParam,LPARAM lParam){DWORD excode;
 if(hUpdateThread==0)goto create_thread;
 GetExitCodeThread(hUpdateThread,&excode);
 if(excode!=STILL_ACTIVE){
  CloseHandle(hUpdateThread);
create_thread:
  hUpdateThread=CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)UpdateThread,0,0,&UpdateThreadId);
  if(hUpdateThread==0){FailMessage("Unable to create thread",0,FMSG_CallGLE|FMSG_WRITE_LOG|FMSG_SHOW_MSGBOX);return;}
  WaitForSingleObject(hUpdateThread,120);
 }
 if(Msg!=0)PostThreadMessage(UpdateThreadId,Msg,wParam,lParam);
}

LONG WINAPI myTopLevelExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo){char Text[256],buf[20];
 copystring(Text,AnException_txt);
 HexToString(ExceptionInfo->ExceptionRecord->ExceptionCode,buf); strappend(Text,buf);
 strappend(Text,ExceptionAdr_txt);
 HexToString((DWORD)(ExceptionInfo->ExceptionRecord->ExceptionAddress),buf); strappend(Text,buf);
 strappend(Text,TheProgWillTerminated_txt);
 FailMessage(Text,0,FMSG_NO_INFO|FMSG_NO_SUCCESSINFO|FMSG_WRITE_LOG|FMSG_SHOW_MSGBOX);
 return EXCEPTION_EXECUTE_HANDLER;
}

LONG ParseFile(char *fPath,LONG Len,char **pFPath,char *tmpbuf){
 HANDLE hfile; char *ptr=0; LONG ret=0; DWORD prevdw=0x01020304; BYTE prevbt=0;
 if(fPath==0 || pFPath==0 || tmpbuf==0)return 0;
 if(!Len)Len=getstrlen(fPath);
chkPath:
 if((hfile=CreateFile(fPath,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))==INVALID_HANDLE_VALUE){
  if(ptr==0){
   copybytes(tmpbuf,fPath,Len+1,1); fPath=tmpbuf;
   char *p; p=fPath; while(*p!=0){if(*p=='\\'){ptr=p;} p++;}
  }
  else {
   if(prevdw!=0x01020304){*(DWORD*)ptr=prevdw; *(ptr+4)=prevbt; prevdw=0x01020304;}
   *ptr++=0x20;
  }
  if(ptr){
   while(*ptr!=0){
    if(*ptr==0x20){
     *ptr=0;
      if(*(ptr-4)!=0x2E){//the . doesn't exist => add .exe\0 at the end
       prevdw=*(DWORD*)ptr; prevbt=*(ptr+4);
      *(DWORD*)ptr=0x6578652E; *(ptr+4)=0;
     }
     goto chkPath;
    }
    ptr++;
   }
  }
 }
 else {CloseHandle(hfile); ret=getstrlen(fPath);}
 *pFPath=fPath;
return ret;}

char *CorrectFilePath(LPSTR InputPath,int Len,void *tmpbuf){int in; bool path=0,expath=0; LPSTR fPath;
 if(Len==0)Len=getstrlen(InputPath);
 fPath=InputPath;
 if(*fPath=='\"'){fPath++; Len--; for(in=0;in<Len;in++){if(*(fPath+in)=='\"'){*(fPath+in)=0;break;}}}//if "path" "params"
 if(*(DWORD*)fPath==0x5C3F3F5C){fPath+=4; Len-=4;}//if \\??\\...
 else if(*fPath=='\\'){fPath++; Len--;}//if \\...
 if(*(DWORD*)fPath==0x74737953 || *(DWORD*)fPath==0x74737973){//System OR system
  int ofst;
  if(*((DWORD*)fPath+1)==0x6F526D65 && *((WORD*)fPath+4)==0x746F){ofst=11; goto modify;}//SystemRoot
  else if(*((WORD*)fPath+2)==0x6D65){ofst=0; goto modify;}//System...
  goto exit;
modify:
  copystring((char*)tmpbuf,WinDir);
  copystring((char*)tmpbuf+WinDirlen,fPath+ofst);
  fPath=(char*)tmpbuf; goto exit;
 }
 for(in=0;in<Len;in++){
  if(*(fPath+in)=='\\')path=1;
  else if(*(fPath+in)=='%')expath=1;
 }
 if(path && Len>=WinDirlen){
  int lw; lw=WinDirlen; lw-=5;
  if(*(fPath+lw)==*(WinDir+WinDirlen-2)){BYTE bc;
   lw+=2; bc=*(fPath+lw); *(fPath+lw)=0;
   if(lstrcmpi(WinDir+3,fPath)==0){
    copystring((char*)tmpbuf,WinDir);
    *(fPath+lw)=bc;
    copystring((char*)tmpbuf+WinDirlen,fPath+lw);
    fPath=(char*)tmpbuf; goto exit;
   }
   else {*(fPath+lw)=bc;}
  }
 }
 else if(!path){HANDLE hfile;
  copystring((char*)tmpbuf,DrvDir); lstrcat((char*)tmpbuf,fPath);
  if((hfile=CreateFile((char*)tmpbuf,0,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))==INVALID_HANDLE_VALUE){
   fPath=InputPath;
   goto exit;
  }
  CloseHandle(hfile);
  fPath=(char*)tmpbuf;
 }
 if(expath && path){
  ExpandEnvironmentStrings(fPath,tbExpand,512);
  fPath=tbExpand;
 }
exit:
 return fPath;
}

#include "shutdown.cpp"

void OpenHelpTopic(BYTE Selection,char *pTopicFile){HINSTANCE hHctrlLib;
 if(pHtmlHelpA==0){char hctrlpath[260]; HKEY hKey; char *libptr;
  if(RegOpenKeyEx(HKEY_CLASSES_ROOT,hhctrl_regkey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS){
   DWORD sz; sz=260;
   if(RegQueryValueEx(hKey,0,0,0,(LPBYTE)&hctrlpath,&sz)==ERROR_SUCCESS)libptr=hctrlpath;
   else libptr=hhctrl_ocx;
   RegCloseKey(hKey);
  }
  else libptr=hhctrl_ocx;
  if((hHctrlLib=LoadLibrary(libptr))==0)return;
  pHtmlHelpA=(HWND(__stdcall *)(HWND,LPCSTR,UINT,DWORD_PTR))GetProcAddress(hHctrlLib,ATOM_HTMLHELP_API_ANSI);
 }
 if(pHtmlHelpA){
  char *chmfl,*pfile; int sz; bool ntrpt; ntrpt=0; sz=ProgramDirPath.CharCount;
  if(pTopicFile==0)pTopicFile=intrnlTopics[Selection];
  sz+=getstrlen(pTopicFile);
  sz+=getstrlen(ActiveLngHelp);
  sz+=5;//1+sizeof("Help.chm"_main_part [i.e. Help])
  chmfl=(char*)LocalAlloc(LPTR,sz);
  if(!chmfl){FailMessage(smemTable->UnblAllocMem,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);return;}
  sz=UnicodeStringToAnsiString(ProgramDirPath.fPath,ProgramDirPath.pathLen,chmfl,sz);
  pfile=ActiveLngHelp;
openchm:
  copystring(chmfl+sz,pfile);
  strappend(chmfl+sz,pTopicFile);
  if(pHtmlHelpA(main_win,chmfl,HH_DISPLAY_TOPIC,0)==0){
   if(!ntrpt){ntrpt=1; pfile=HelpChm_txt; goto openchm;}
   MessageBox(main_win,smemTable->UnblOpenHelpFile,ERROR_txt,MB_OK|MB_ICONWARNING);
  }
  LocalFree(chmfl);
 }
}

DWORD LowPaneThread(DWORD){MSG msg;
 if(pHeap==0)pHeap=HeapCreate(0,1024,0);
 while(GetMessage(&msg,NULL,0,0)){
  if(msg.message==BTMPANE_UPDATE && msg.wParam){
   if(msg.wParam==DLLpane)ListDlls(((BTMPANE_UPDATE_STURCT*)msg.lParam)->CurSel);
   else if(msg.wParam==HANDLEpane)ListHandles(((BTMPANE_UPDATE_STURCT*)msg.lParam)->CurSel,LISTHANDLES_FILTER_PROCESS);
   if(((BTMPANE_UPDATE_STURCT*)msg.lParam)->EnsureVisible!=-1){
    int nd,ev; HWND hlst;
    hlst=(msg.wParam==DLLpane)?hModuleList:hHandleList;
    ev=((BTMPANE_UPDATE_STURCT*)msg.lParam)->EnsureVisible;
    nd=SendMessage(hlst,LVM_GETITEMCOUNT,0,0);
    if(nd<=ev){ev=nd;ev--;}
    SendMessage(hlst,LVM_ENSUREVISIBLE,ev,1);
   }
   LocalFree((void*)msg.lParam);
  }
  TranslateMessage(&msg); DispatchMessage(&msg);
 }
 if(IsWindow(hModuleList))SendMessage(hModuleList,LVM_DELETEALLITEMS,0,0);
 if(IsWindow(hHandleList))SendMessage(hHandleList,LVM_DELETEALLITEMS,0,0);
 if(pHeap && HeapDestroy(pHeap))pHeap=0;
 ExitThread(0);
 return 0;
}

BOOL ReplaceTaskManager(BYTE Param){HKEY hkey=0; LPWSTR pBuf; DWORD n,ErrCode,dwType; BOOL ret=0;
 if((Param&SETTS_REPLACETASKMAN_ENABLE)==SETTS_REPLACETASKMAN_ENABLE){//replace
  if((ErrCode=RegCreateKeyEx(HKEY_LOCAL_MACHINE,taskmgrKey,0,0,0,KEY_ALL_ACCESS,0,&hkey,&n))!=ERROR_SUCCESS){
   FailMessage(smemTable->ErrReplTskmgr,ErrCode,FMSG_SHOW_MSGBOX);
   return 0;
  }
  if(ProgramDirPath.fstrLen>(47*2)){
   OSVERSIONINFO osv; osv.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
   if(GetVersionEx(&osv) && osv.dwPlatformId==VER_PLATFORM_WIN32_NT){
    if(osv.dwMajorVersion==5 && (osv.dwMinorVersion<1 || osv.dwBuildNumber<2600)){
     MessageBox(main_win,smemTable->W2kRplTman,progTitle,MB_OK|MB_ICONWARNING);
     goto closeKey;
    }
   }
  }
  n=ProgramDirPath.fstrLen;
  if((Param&SETTS_REPLACETASKMAN_SHOWLOGONALWAYS)==SETTS_REPLACETASKMAN_SHOWLOGONALWAYS){dwType=1; n+=getstrlenW(cmdtxtW_runas)+2;}
  else if((Param&SETTS_REPLACETASKMAN_SHOWLOGONIFNONADMIN)==SETTS_REPLACETASKMAN_SHOWLOGONIFNONADMIN){dwType=2; n+=getstrlenW(cmdtxtW_runasadmin)+2;}
  else dwType=0;
  n+=2;
  if((pBuf=(LPWSTR)LocalAlloc(LPTR,n))!=0){
   copyMemBytes(pBuf,ProgramDirPath.fPath,ProgramDirPath.fstrLen);
   if(dwType){
    lstrcatW(pBuf,L" ");
    if(dwType==1)lstrcatW(pBuf,cmdtxtW_runas);
    else if(dwType==2)lstrcatW(pBuf,cmdtxtW_runasadmin);
   }
   if(RegSetValueExW(hkey,Debugger_txtW,NULL,REG_SZ,(BYTE*)pBuf,n)==ERROR_SUCCESS)ret=1;
   LocalFree(pBuf);
  }
closeKey:
  if(hkey)RegCloseKey(hkey);
 }
 else{//restore to previous state
  if((ErrCode=RegDeleteKey(HKEY_LOCAL_MACHINE,taskmgrKey))==ERROR_SUCCESS)ret=1;
  else if(!cmd_Remove)FailMessage(smemTable->ErrRestoreTskmgr,ErrCode,FMSG_SHOW_MSGBOX);
 }
 return ret;
}

#include "Run_Dlg.cpp"
#include "Chart.cpp"

#include "Performance.cpp"
#include "handlesInfo.cpp"
#include "Processes_BtmPane.cpp"
#include "Processes.cpp"
#include "Extras_Handles.cpp"
#include "CLSIDinfo.cpp"
#include "StartupRun.cpp"
#include "SrvcGroups.cpp"
#include "ServicesRemote.cpp"
#include "Services.cpp"
#include "Windows.cpp"
#include "Drivers.cpp"
#include "About.cpp"
#include "Settings.cpp"
#include "WriteDataBin.cpp"
#include "main_win.cpp"

//#include "TestWin.cpp"

void Updates_mainProc(){DWORD n;
 UpdateCPUusage();
 *gdCpu.histCurPos=1.0-(cpuUsage*0.01);
 //*gdCpuKrnl.histCurPos=1.0-(cpuKernelTime*0.01);
 if(progSetts.CheckState[14]==MEMORYTYPE_USAGE_COMMIT){//Commit charge (pagefile)
  commit_prc=((float)(sysPerfInf->TotalCommittedPages))/(sysPerfInf->TotalCommitLimit);
  *gdCommit.histCurPos=commit_prc;
 }
 else if(progSetts.CheckState[14]==MEMORYTYPE_USAGE_RAM){
  commit_prc=SysBasicInf->NumberOfPhysicalPages;
  commit_prc=sysPerfInf->AvailablePages;
  commit_prc=((float)commit_prc)/(SysBasicInf->NumberOfPhysicalPages);
  *gdCommit.histCurPos=commit_prc;
  commit_prc=commit_prc;
 }
watch_prc:
 WatchProcesses();
 if(pHdr!=0 && pHdr->updTimerProc[iSel]!=0){
  __try{
   ((void(_stdcall*)(void))pHdr->updTimerProc[iSel])();
  }
  __except((n=__exception_code)!=0){
   Try_Except_Error(n);
   goto _bbreak;
  }
 }
_bbreak:
 if(prevProcCount!=processCount){
  copystring(t1buf1,smemTable->Processes_); ltoa(processCount,t1buf1+lstrlen(t1buf1),10);
  if(SendMessage(hstatus,SB_SETTEXT,(WPARAM)0,(LPARAM)t1buf1))prevProcCount=processCount;
 }
 if(firsttime){
  firsttime=0;
  for(n=0;n<COLUMNS_COUNT;n++)Upd[n]=1;
  Upd[0]=Upd[1]=Upd[2]=Upd[4]=Upd[7]=Upd[8]=Upd[9]=Upd[11]=Upd[12]=Upd[13]=Upd[14]=Upd[15]=Upd[20]=Upd[21]=Upd[22]=0;
  if(intrdpcs && intrdpcs->spi)FillMemory(intrdpcs->spi,sizeof(SYSTEM_PROCESS_INFORMATION),0);
  FillMemory(((PROCESS_INFO*)(intrdpcs->NextEntry))->spi,sizeof(SYSTEM_PROCESS_INFORMATION),0);
  goto watch_prc;
 }
 else{
  for(n=0;n<COLUMNS_COUNT;n++)Upd[n]=0;
 }
 // Windows boot Time
 LARGE_INTEGER li; DWORD dd;
 li.QuadPart=sysTimeInf->CurrentTime.QuadPart;
 li.QuadPart-=sysTimeInf->BootTime.QuadPart;
 for(dd=0;li.QuadPart>864000000000;dd++)li.QuadPart-=864000000000;
 if(FileTimeToSystemTime((FILETIME*)&li,&stm)){
  if(dd){ltoa(dd,t1buf512,10); lstrcat(t1buf512,smemTable->days); dd=getstrlen(t1buf512);}
  getTime(&stm,t1buf512+dd,0);
  copystring(t1buf1,t1buf512);
  if(dd!=pHdr->StatusBootTimeDays){
   pHdr->StatusBootTimeDays=dd;
   dd=CalcStringAvgWidth(hstatus,t1buf1);
   if(dd!=pHdr->StatusBarParts[STATUSBAR_WINBOOT_ID]){
    dd+=16;
    pHdr->StatusBarParts[STATUSBAR_WINBOOT_ID]=dd;
    SetStatusBarParts();
   }
  }
  SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_WINBOOT_ID,(LPARAM)t1buf1);
 }
//
 if((DWORD)gdCpu.histCurPos<usrHistEnd)gdCpu.histCurPos++;
 else gdCpu.histCurPos=(float*)usrHistStart;
 if((DWORD)gdCpuKrnl.histCurPos<krnHistEnd)gdCpuKrnl.histCurPos++;
 else gdCpuKrnl.histCurPos=(float*)krnHistStart;
 if((DWORD)gdCommit.histCurPos<cmtHistEnd)gdCommit.histCurPos++;
 else gdCommit.histCurPos=(float*)cmtHistStart;
}

DWORD UpdateThread(DWORD updateTimer){MSG msg; DWORD tmn=0,ErrCode;//Beep(200,50);
if(updateTimer!=(DWORD)-1)goto starttimer;
while(GetMessage(&msg,NULL,0,0)){
 switch(msg.message){
  case WM_TIMER: if(msg.wParam!=tmn)break;
  case UPDATETHREAD_UPDATE: if(msg.message==UPDATETHREAD_UPDATE && msg.wParam!=0xFF)break;
   Updates_mainProc();
   break;
  case UPDATETHREAD_EXECUTE: if(ExecuteAddr==0)break;
   if(msg.wParam==ExecuteAddr && (BYTE)(msg.lParam)==0xFE){
    DWORD_PTR eadr; eadr=ExecuteAddr; ExecuteAddr=0;
    __try{
     ExcC05=0;
     if(msg.lParam!=0x87ABFE){((void(_stdcall*)(void))eadr)();}
     else {((void(_stdcall*)(HWND))eadr)(services_win);}
    }
    __except((ErrCode=__exception_code)!=0){
     Try_Except_Error(ErrCode);
     goto _bbreak;
    }
   }
   _bbreak:
   break;
  case UPDATETHREAD_START: if(msg.wParam!=0xFF)break;
starttimer:
   if(UpdateSleepTm!=0 && UpdateSleepTm!=(DWORD)-1)tmn=SetTimer(0,0,UpdateSleepTm,(TIMERPROC)0);
   break;
  case UPDATETHREAD_STOP: if(msg.wParam==0xFF && tmn)KillTimer(0,tmn); break;
  case UPDATETHREAD_RESTART: if(msg.wParam!=0xFF)break;
   if(tmn)KillTimer(0,tmn);
   goto starttimer;
  default: TranslateMessage(&msg); DispatchMessage(&msg);
 }
}
ExitThread(0);
return 0;}

void ExitProgram(){
 if(nid.cbSize!=0){nid.uID=1; nid.hWnd=main_win; Shell_NotifyIcon(NIM_DELETE,&nid);}
 if(hFailLog!=NULL)CloseHandle(hFailLog);
 #ifdef LOG_STEPS
  if(hStepsLog!=NULL)CloseHandle(hStepsLog);
 #endif
 #ifdef USE_MS_WinMain
  ExitProcess(0);
 #endif
}

void CreateCpuUsageImageList(){BITMAPINFO bmi; HBITMAP hbitmap,tbmp,hbmp; HDC hdc,tdc; HBRUSH bbr; RECT rfill;
 rfill.top=1; rfill.bottom=15; rfill.left=2; rfill.right=14;
 bbr=(HBRUSH)GetStockObject(BLACK_BRUSH);
 hbitmap=(HBITMAP)LoadImage(gInst,(char*)6,IMAGE_BITMAP,16,16,LR_DEFAULTCOLOR);
 hUsageImg=ImageList_Create(16,16,ILC_COLOR24|ILC_MASK,15,1);
 hdc=CreateCompatibleDC(0);
 tdc=CreateCompatibleDC(0);
 tbmp=CreateCompatibleBitmap(tdc,1,1);
 RtlZeroMemory(&bmi,sizeof(BITMAPINFO));
 bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
 bmi.bmiHeader.biWidth=16; bmi.bmiHeader.biHeight=16;
 bmi.bmiHeader.biPlanes=1; bmi.bmiHeader.biBitCount=24;
 hbmp=CreateDIBSection(tdc,&bmi,DIB_RGB_COLORS,NULL,0,0);
 SelectObject(hdc,hbitmap); SelectObject(tdc,hbmp);
 BitBlt(tdc,0,0,16,16,hdc,0,0,SRCCOPY);
 for(int i=0;i<15;i++){
  BitBlt(tdc,0,0,16,16,hdc,0,0,SRCCOPY);
  FillRect(tdc,&rfill,bbr); rfill.bottom--;
  SelectObject(tdc,tbmp);
  ImageList_AddMasked(hUsageImg,hbmp,0x00FF00FF);
  SelectObject(tdc,hbmp);
 }
 DeleteObject(hbitmap); DeleteObject(hbmp); DeleteObject(tbmp);
 DeleteDC(hdc); DeleteDC(tdc);
}

void Settings_SetDefault_WndPlacement(){RECT strect;
 GetClientRect(GetDesktopWindow(),&strect);
 strect.right-=700; strect.right/=2; progSetts.X=strect.right;
 strect.bottom-=640; strect.bottom/=2; progSetts.Y=strect.bottom;
 progSetts.wd=700; progSetts.hg=640; progSetts.Maximized=0;
}

void Settings_VerifyWndPlacement(){RECT strect;
 if(progSetts.wd<MAINWIN_MIN_WIDTH)goto set_defaults;
 if(progSetts.hg<MAINWIN_MIN_HEIGHT)goto set_defaults;
 if(progSetts.X<0 && (progSetts.X+progSetts.wd)<=14)goto set_defaults;
 if(progSetts.Y<0 && (progSetts.Y+progSetts.hg)<=14)goto set_defaults;
 if(GetClientRect(GetDesktopWindow(),&strect)==0)return;
 strect.right-=14; strect.bottom-=14;
 if(progSetts.X>strect.right)goto set_defaults;
 if(progSetts.Y>strect.bottom)goto set_defaults;
 return;
set_defaults:
 Settings_SetDefault_WndPlacement();
}

void Settings_SetDefault_All(){DWORD n,nm;
 RtlZeroMemory((void*)&progSetts,sizeof(PROGSETTINGS));
 progSetts.cbSize=sizeof(PROGSETTINGS);
 progSetts.wSign=THIS_VERSION_PROGSETTS_SIGN;
 progSetts.CheckStateSize=sizeof(progSetts.CheckState);
 progSetts.ShtdCkBx[0]=1;
 progSetts.ShtdCkBx[3]=1;
 progSetts.LastServcEnumButn=102;
 progSetts.LastStartupEnumButn=102;
 progSetts.AWFtr=FILTER_IF_CAPTION_EXIST|FILTER_IF_VISIBLE|FILTER_IF_NOPARENT;
 Settings_SetDefault_WndPlacement();
 copybytes(&progSetts.CheckState,&DefCheckState,sizeof(DefCheckState),0);
 #ifdef _DEBUG
 progSetts.CheckState[0]=0;//Always On Top is Off in DebugVer
 #endif
 progSetts.ProcsSortd.CurSubItem=(DWORD)-1;
 progSetts.DllsSortd.CurSubItem=(DWORD)-1;
 progSetts.HndlsSortd.CurSubItem=(DWORD)-1;
 progSetts.WinSortd.CurSubItem=(DWORD)-1;
 progSetts.SrvSortd.CurSubItem=(DWORD)-1;
 progSetts.DrvSortd.CurSubItem=(DWORD)-1;
 progSetts.WinSortd.SortDirection=2;
 progSetts.ProcsSortd.SortDirection=2;
 n=GetSystemDefaultLangID(); nm=GetUserDefaultLangID();
 if(n==0x0419 || nm==0x0419)progSetts.LanguageID=RUS_LANGID;//0x0419 - Rus
 else progSetts.LanguageID=ENG_LANGID;
 FillHndlTbl=1;
 progSetts.MenuStyle=MENU_STYLE_XNET2;
}

DWORD iy,iny;

BOOL EntryPoint_Init(BOOL *runOK){
 OSVERSIONINFO osv; DWORD ErrCode,nm; char *LngFileName; BOOL bRunOK,isFirstRun; int i,in;
 DWORD_PTR dwp; void *lbuf2048;
 LngFileName=0; bRunOK=0;
 gInst=GetModuleHandle(0);
 /////// Get OS Version
 osv.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
 if(GetVersionEx(&osv)){
  if(osv.dwPlatformId!=VER_PLATFORM_WIN32_NT)goto win_ver_err;
  if(osv.dwMajorVersion<5)goto win_ver_err;
  if(osv.dwMajorVersion==5){
   if(osv.dwMinorVersion<1)goto win_nt_ver_err;
   if((osv.dwMinorVersion==1 && osv.dwBuildNumber<2600))goto win_nt_ver_err;
  }
 }
 goto continue_run;
win_ver_err:
  MessageBox(main_win,"Windows XP is required !",progTitle,MB_OK|MB_ICONSTOP);
  goto exit;
win_nt_ver_err:
  if(MessageBox(main_win,"Windows XP is required.\r\rContinue anyway ?",progTitle,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2)!=IDYES)goto exit;
continue_run:
 //bRunOK=0;
 nm=sizeof(PROGSETTINGS);
 /// Is First Time
 isFirstRun=0;
 /// Load NTDLL functions
 DWORD *pdw;
 GetFuncProcAddress((DWORD)GetModuleHandle(ntdll_dll),&ntdllFunctions,&ntdllHASHTable);
 pdw=(DWORD*)&ntdllFunctions;
 in=0; for(i=0;ntdllHASHTable[i]!=0xFFFF;i++){if(*(pdw++)!=0)in++;}
 iy=i; iny=in;
 if(in!=i && MessageIntErr(1))goto exit;
 /////
 lbuf2048=LocalAlloc(LPTR,2048);
 //// Get Current Directory via PEB
 my_peb=asmGetCurrentPeb();
 STRING str; UNICODE_STRING *uPath; WORD *wptr; RTL_USER_PROCESS_PARAMETERS *rtup;
 str.Length=0; str.MaximumLength=2048; str.Buffer=(char*)lbuf2048;
 rtup=my_peb->ProcessParameters;
 uPath=&(rtup->ImagePathName);
 ProgramDirPath.fPath=uPath->Buffer;
 wptr=(WORD*)((BYTE*)(uPath->Buffer)+uPath->Length);
 for(i=0;i<uPath->Length;i+=2){if(*wptr==0x005C)break; wptr--;}
 ProgramDirPath.fstrLen=uPath->Length;
 ProgramDirPath.pathLen=uPath->Length; if(i>0)i-=2; ProgramDirPath.pathLen-=(USHORT)i;
 ProgramDirPath.CharCount=ProgramDirPath.pathLen; ProgramDirPath.CharCount/=(USHORT)2;
 //// Обработка CommandLine
 cmd_NoRegSetts=0; cmd_Remove=0;
 CmdLine_Determine(rtup->CommandLine.Buffer,lbuf2048,&LngFileName);
 //// Установка настроек
 if(cmd_NoRegSetts)goto SetDefaultSetts;
 //// Load Settings from Registry
 LSA_OBJECT_ATTRIBUTES oat; UNICODE_STRING ustr;//KEY_VALUE_PARTIAL_INFORMATION *kvp; kvp=(KEY_VALUE_PARTIAL_INFORMATION*)lbuf2048;
 RtlZeroMemory(&oat,sizeof(oat)); hMainKey=0; n=0;
 // Registry - Open DMoNsoft key
 ustr.Buffer=hk_DMoNsoftW; ustr.Length=getstrlenW(ustr.Buffer);
 oat.Length=sizeof(LSA_OBJECT_ATTRIBUTES);
 oat.RootDirectory=0;//(HANDLE)HKEY_Convert((DWORD)HKEY_LOCAL_MACHINE);
 oat.ObjectName=&ustr; ustr.MaximumLength=(USHORT)(ustr.Length);
 oat.Attributes=OBJ_CASE_INSENSITIVE;
 if((ErrCode=ntdllFunctions.NtCreateKey((HANDLE*)&hMainKey,KEY_CREATE_SUB_KEY|KEY_QUERY_VALUE,&oat,0,0,0,&n))!=0){
  if(ErrCode==0xC0000022){
   if((ErrCode=ntdllFunctions.NtCreateKey((HANDLE*)&hMainKey,KEY_QUERY_VALUE,&oat,0,0,0,&n))==0)goto open_prog_subkey;
  }
  FailMessage(UnblCrtOpenKey_txt,ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_WRITE_LOG);
  goto SetDefaultSetts;
 }
 // Registry - Open PROWiSe subkey
open_prog_subkey:
 ustr.Buffer=hk_mainW; ustr.Length=getstrlenW(ustr.Buffer);
 #ifdef SETTS_REGKEY_HIDE
  ustr.Length++;//чтобы WinAPI не открыл
 #endif
 oat.RootDirectory=hMainKey;
 if((ErrCode=ntdllFunctions.NtCreateKey((HANDLE*)&hMainKey,KEY_QUERY_VALUE|KEY_SET_VALUE,&oat,0,0,0,&n))!=0){
  if(ErrCode==0xC0000022){
   if((ErrCode=ntdllFunctions.NtCreateKey((HANDLE*)&hMainKey,KEY_QUERY_VALUE,&oat,0,0,0,&n))==0)goto queryreg;
  }
  ntdllFunctions.NtClose(oat.RootDirectory);
  FailMessage(UnblCrtOpenKey_txt,ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_WRITE_LOG);
  goto SetDefaultSetts;
 }
queryreg:
 ustr.MaximumLength=0;
 if(n==REG_CREATED_NEW_KEY || RegQueryValueEx(hMainKey,Settings_txt,NULL,&n,0,&nm)!=ERROR_SUCCESS || n!=REG_BINARY || nm<16 || nm>=2048)goto First_Run;
 BYTE *mem_fill;
 if(nm!=sizeof(PROGSETTINGS))mem_fill=(BYTE*)lbuf2048;
 else mem_fill=(BYTE*)&progSetts;
 if(RegQueryValueEx(hMainKey,Settings_txt,NULL,&n,mem_fill,&nm)!=ERROR_SUCCESS)goto First_Run;
 PROGSETTINGS *psets; psets=(PROGSETTINGS*)mem_fill;
 if(HIWORD(psets->wSign)!=THIS_VERSION_PROGSETTS_VERSION)isFirstRun=1;
 if(nm!=sizeof(PROGSETTINGS) && psets->cbSize<=sizeof(PROGSETTINGS)){
  if((WORD)(psets->wSign)!=THIS_VERSION_PROGSETTS_COMPATABILITY_SIGN)goto First_Run;
  Settings_SetDefault_All();
  if(psets->CheckStateSize>progSetts.CheckStateSize || nm<=psets->CheckStateSize)goto end_load_setts;
  copybytes((char*)progSetts.CheckState,(char*)psets->CheckState,psets->CheckStateSize,0);
 }
 goto end_load_setts;
First_Run:
 isFirstRun=1;
SetDefaultSetts://default setts
 Settings_SetDefault_All();
end_load_setts:
 progSetts.wSign=THIS_VERSION_PROGSETTS_SIGN;
 progSetts.CheckState[CHECKstateCOUNT-1]=0xFF;
 //// Check is another instance
 if(!cmd_Logon && progSetts.CheckState[1] && IsAnother(cmd_Remove)){
  if((main_win=FindWindow((char*)32770,progTitle))!=0){
   if(SendMessageTimeout(main_win,WM_MY_TRAYICON,2,WM_LBUTTONDOWN,SMTO_ABORTIFHUNG,150,0)==0){
    if(IsWindowVisible(main_win)==0)ShowWindow(main_win,SW_SHOW);
    if(IsIconic(main_win))ShowWindow(main_win,SW_RESTORE);
    SetForegroundWindow(main_win);
   }
   bRunOK=1;
  }
  goto exit;
 }
 LocalFree(lbuf2048);
 //// Verify main_win coordinates
 Settings_VerifyWndPlacement();
 //// Load other functions
 GetFuncProcAddress((DWORD)LoadLibrary(user32dll_txt),&user32Functions,&user32HASHTable);
 in=0; pdw=(DWORD*)&user32Functions;
 for(i=0;user32HASHTable[i]!=0xFFFF;i++){if(*(pdw++)!=0)in++;}
 if(in!=i && MessageIntErr(2))goto exit;
 SetPriorityClass((HANDLE)-1,HIGH_PRIORITY_CLASS);//NORMAL_PRIORITY_CLASS
 #ifndef DISABLE_MYTOPLEVEL_EXCPTNHANDLER
  SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)myTopLevelExceptionFilter);
 #endif
 //// Allocate Memory
 spi_memsz=0x10000; spi_mem=LocalAlloc(LPTR,spi_memsz);
 hgsz2=44800; hg2=LocalAlloc(LPTR,hgsz2);//0x10000
 SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
 dwp=(DWORD_PTR)hg2;
 t0nbuf1=(DWORD*)dwp; dwp+=2048;
 t0nbuf2=(DWORD*)dwp; dwp+=2048;
 t1nbuf1=(DWORD*)dwp; dwp+=2048;
 t1nbuf2=(DWORD*)dwp; dwp+=2048;
 t1nbuf3=(DWORD*)dwp; dwp+=1024;
 t2nbuf1=(DWORD*)dwp; dwp+=2048;
 t2nbuf2=(DWORD*)dwp; dwp+=1024;
 ///
 sysPerfInf=(SYSTEM_PERFORMANCE_INFORMATION*)dwp; dwp+=400;
 sysTimeInf=(SYSTEM_TIME_OF_DAY_INFORMATION*)dwp; dwp+=28;
 SysBasicInf=(SYSTEM_BASIC_INFORMATION*)dwp; dwp+=44;
 sysProcessorTm=(SYSTEM_PROCESSOR_TIMES*)dwp; dwp+=48*4;
 usrHistStart=(DWORD*)dwp; dwp+=4096; usrHistEnd=dwp;//cpuUsage User history data
 krnHistStart=(DWORD*)dwp; dwp+=4096; krnHistEnd=dwp;//cpuUsage Kernel history data
 cmtHistStart=(DWORD*)dwp; dwp+=4096; cmtHistEnd=dwp;//Commit history data
 gdCpu.histCurPos=(float*)usrHistStart;
 gdCpuKrnl.histCurPos=(float*)krnHistStart;
 gdCommit.histCurPos=(float*)cmtHistStart;
 t0buf1=(char*)dwp; dwp+=600;
 t1buf1=(char*)dwp; dwp+=300;
 t2buf1=(char*)dwp; dwp+=300;
 t0buf2=(char*)dwp; dwp+=600;
 t1buf2=(char*)dwp; dwp+=300;
 t2buf2=(char*)dwp; dwp+=300;
 tbuf3=(char*)dwp; dwp+=300;//потоки не пересекаются
 t1buf4=(char*)dwp; dwp+=300;//Windows,Performance
 t2buf4=(char*)dwp; dwp+=300;//About
 //hwndsStart=(DWORD*)n; n+=4000; hwndsEnd=n;//HWNDs для Windows_tab = 1000 DWORDs
 t1buf512=(char*)dwp; dwp+=512;//getTime(&), getDate(&)
 pvbuf1=(char*)dwp; dwp+=524;//Processes
 wpbuf1=(void*)dwp; dwp+=2048;//check isn't too more //Processes
 wpbuf3=(void*)dwp; dwp+=2048;//check isn't too more //Processes
 WinDir=(char*)dwp; dwp+=256;
 DrvDir=(char*)dwp; dwp+=256;
 SysDir=(char*)dwp; dwp+=256;
 t0nbuf4=(DWORD*)dwp; dwp+=2048;
 t1nbuf4=(DWORD*)dwp; dwp+=2048;
 t2nbuf4=(DWORD*)dwp; dwp+=2048;
 nbuf5=(DWORD*)dwp; dwp+=2048;//handlepane,thread1
 tbuf8=(char*)dwp; dwp+=120;//handlepane
 t1buf9=(char*)dwp; dwp+=1024;
 tbuf10=(char*)dwp; dwp+=512;
 tbuf_gfvdi=(char*)dwp; dwp+=512;//для GetFileVerDescInfo
 tbExpand=(char*)dwp; //dwp+=512;
 #ifdef _DEBUG
  DWORD_PTR dwpEnd; dwpEnd=dwp; dwpEnd-=(DWORD_PTR)hg2;
  if(dwpEnd>hgsz2){
   Beep(1000,100);
   FailMessage("Buffer is too small!",dwpEnd,FMSG_NO_INFO|FMSG_SHOW_MSGBOX);
   return 0;
  }
 #endif
 hWinChain=(HWND*)LocalAlloc(LPTR,400);
 WinChain_End=(DWORD_PTR)hWinChain; WinChain_End+=LocalSize(hWinChain); WinChain_End-=4;
 FillMemory(usrHistStart,4096*3,0x03);//fill user,kernel,commit history
 //// Load Language File & Fill ResourcesAllocationTable
 if(FillResourcesAllocationTable(LngFileName)==0){FailMessage(ReadOfLngFileFailed_txt,0,FMSG_NO_SUCCESSINFO|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG); goto exit;}
 LocalFree(LngFileName);
 ErrCode_txt=smemTable->ErrorCode_0x;
 ERROR_txt=smemTable->ERROR_txt;
 //// Logon As another user
 if(cmd_LogonAs && !cmd_Logon){
  if(LogOn_ShowLogonDlg()){
   bRunOK=1; goto exit;
  }
 }
 //// Снятие программы при drkey
 if(cmd_Remove){//UnInstall Mode
  HKEY hKey,hKeyCompany;
  if(MessageBox(main_win,smemTable->AYouRemoveCngs,progTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)==IDYES){
   ustr.Buffer=hk_DMoNsoftW; ustr.Length=getstrlenW(ustr.Buffer);
   RtlZeroMemory(&oat,sizeof(oat));
   oat.Length=sizeof(LSA_OBJECT_ATTRIBUTES);
   oat.RootDirectory=0;
   oat.ObjectName=&ustr; ustr.MaximumLength=(USHORT)(ustr.Length);
   oat.Attributes=OBJ_CASE_INSENSITIVE;
   if((ErrCode=ntdllFunctions.NtOpenKey((HANDLE*)&hKeyCompany,DELETE,&oat))!=0)goto openKey_fails;
   //open PROWiSe sub-key
   oat.RootDirectory=hKeyCompany;
   ustr.Buffer=hk_mainW; ustr.Length=getstrlenW(ustr.Buffer);
   #ifdef SETTS_REGKEY_HIDE
    ustr.Length+=2;//чтобы WinAPI не открыл
   #endif
   if((ErrCode=ntdllFunctions.NtOpenKey((HANDLE*)&hKey,DELETE,&oat))!=0){
    openKey_fails:
    FailMessage(smemTable->UnblOpenRegKey,ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_ICONSTOP|FMSG_WRITE_LOG|FMSG_SHOW_MSGBOX);
   }
   else{
    if((ErrCode=ntdllFunctions.NtDeleteKey(hKey))!=0){FailMessage(smemTable->UnblDelRegKey,ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_ICONSTOP|FMSG_WRITE_LOG|FMSG_SHOW_MSGBOX); ntdllFunctions.NtClose(hKey);}
    else MessageBox(0,smemTable->RegKeyDelSuccess,progTitle,MB_OK|MB_SETFOREGROUND|MB_ICONINFORMATION);
   }
   #ifndef _DEBUG
    if(ErrCode!=(DWORD)STATUS_ACCESS_DENIED)ReplaceTaskManager(0);
   #endif
   if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,UnInstallKey,0,0,0,KEY_SET_VALUE,0,&hKey,&n)==ERROR_SUCCESS){
    RegSetValueEx(hKey,"ContinueUninstall",NULL,REG_SZ,(BYTE*)"Yes",3);
    RegCloseKey(hKey);
   }
   return 0;
  }
  cmd_Remove=0;
 }
 //// Get Windows, System and Drivers dir
 SysDirlen=GetSystemDirectory(SysDir,256); if(SysDir[SysDirlen]!='\\'){SysDir[SysDirlen]=0x5C; SysDir[++SysDirlen]=0;}
 WinDirlen=SysDirlen; WinDirlen--;
 while(SysDir[--WinDirlen]!=0 && SysDir[WinDirlen]!=0x5C); WinDirlen++;
 copybytes(WinDir,SysDir,WinDirlen,1); WinDir[WinDirlen]=0;
 copybytes(DrvDir,SysDir,SysDirlen,1);
 copybytes(DrvDir+SysDirlen,"Drivers\\",8,1);
 cur_pid=asmGetCurrentProcessId();
 //// Check is Task Manager Replace enabled
 HKEY hKey; progSetts.CheckState[21]&=~SETTS_REPLACETASKMAN_ENABLE;
 if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,taskmgrKey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS){
  nm=2048;
  if(RegQueryValueExW(hKey,Debugger_txtW,NULL,&n,(BYTE*)t0nbuf1,&nm)==ERROR_SUCCESS && n==REG_SZ){
   if(CompareStrPartW((LPWSTR)t0nbuf1,ProgramDirPath.fPath,ProgramDirPath.fstrLen,0)==0){
    progSetts.CheckState[21]|=SETTS_REPLACETASKMAN_ENABLE;
   }
  }
  RegCloseKey(hKey);
 }
 //// Enable priveleges
 LUID luid; HANDLE hToken; TOKEN_PRIVILEGES *tkp;
 if(OpenProcessToken((HANDLE)-1,TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)){
  tkp=(TOKEN_PRIVILEGES*)t0nbuf1;
  tkp->PrivilegeCount=2;
  LookupPrivilegeValue(NULL,SE_SECURITY_NAME,&luid);
  tkp->Privileges[0].Luid=luid;
  tkp->Privileges[0].Attributes=SE_PRIVILEGE_ENABLED|SE_PRIVILEGE_USED_FOR_ACCESS;
  LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&luid);
  tkp->Privileges[1].Luid=luid;
  tkp->Privileges[1].Attributes=SE_PRIVILEGE_ENABLED|SE_PRIVILEGE_USED_FOR_ACCESS;
  if(AdjustTokenPrivileges(hToken,FALSE,tkp,NULL,(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL)==0)FailMessage(smemTable->CantAdjustPrivel,0,FMSG_CallGLE|FMSG_WRITE_LOG);
  CloseHandle(hToken);
 }
 //// Look for DependsWalker
 *t1buf9=0; hKey=0;
 if(RegCreateKeyEx(HKEY_CLASSES_ROOT,exefile_depends,0,0,0,KEY_READ,0,&hKey,&n)==ERROR_SUCCESS){
  nm=1024;
  if(RegQueryValueEx(hKey,0,NULL,&n,(BYTE*)t1buf9,&nm)==ERROR_SUCCESS && n==REG_SZ)goto storepath;
 }
 if(hKey)RegCloseKey(hKey);
 if(RegCreateKeyEx(HKEY_CLASSES_ROOT,dllfile_depends,0,0,0,KEY_READ,0,&hKey,&n)==ERROR_SUCCESS){
  nm=1024;
  if(RegQueryValueEx(hKey,0,NULL,&n,(BYTE*)t1buf9,&nm)==ERROR_SUCCESS && n==REG_SZ)goto storepath;
 }
storepath:
 if(hKey)RegCloseKey(hKey);
 if(*t1buf9){
  char *ppr; ppr=t1buf9; ppr+=nm; ppr-=2+3;
  if(lstrcmpi(ppr,dde_txt)==0){ppr--; *ppr=0; nm-=4;}
  dependsPath=(char*)LocalAlloc(LMEM_FIXED,nm);
  if(dependsPath)copystring(dependsPath,t1buf9);
 }
 //// Check is Windows PE
 if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,"ControlSet001\\Control\\MiniNT",0,0,0,KEY_READ,0,&hKey,&n)!=ERROR_SUCCESS)isMiniNT=0;
 else {
  isMiniNT=1; RegCloseKey(hKey);
  mainwin_title=(char*)LocalAlloc(LPTR,getstrlen(progTitle)+getstrlen(WinPEmode)+4);
  copystring(mainwin_title+copystring(mainwin_title,progTitle),WinPEmode);
 }
 //// Get Current User Name
 if(Process_GetMachineUserName(cur_pid,0,wpbuf1,((BYTE*)wpbuf1)+1024,wpbuf3,0)){
  if((mainSetts.pCurrentUserName=LocalAllocAndCopy((LPTSTR)wpbuf1))!=0){
   mainSetts.dwCurrentUserNameSize=lstrlen(mainSetts.pCurrentUserName);
  }
  else mainSetts.dwCurrentUserNameSize=0;
 }
 //// Check Is User An Admin
 if(progSetts.CheckState[41])bShowAllProcesses=1;
 else {
  bShowAllProcesses=0;
  LPWKSTA_USER_INFO_1 pwsktui;
  if(NetWkstaUserGetInfo(0,1,(LPBYTE*)&pwsktui)==NERR_Success){
   USER_INFO_1 *puinf;
   if(NetUserGetInfo(pwsktui->wkui1_logon_server,pwsktui->wkui1_username,1,(LPBYTE*)&puinf)==NERR_Success){
    if(puinf->usri1_priv==USER_PRIV_ADMIN)bShowAllProcesses=1;
    NetApiBufferFree(puinf);
   }
   NetApiBufferFree(pwsktui);
  }
 }
 //// Create & Show MainWindow and TrayIcon
 CreateDialog(gInst,(char*)1,NULL,(DLGPROC)Main_Proc);
 ShowWindow(main_win,(bHideOnRun)?SW_HIDE:SW_SHOW);
 CreateCpuUsageImageList();
 //// set tray icon
 nid.cbSize=sizeof(NOTIFYICONDATA);
 nid.hWnd=main_win; nid.uID=1;
 nid.uFlags=NIF_INFO|NIF_MESSAGE|NIF_ICON|NIF_TIP;
 nid.uCallbackMessage=WM_MY_TRAYICON;
 nid.uVersion=NOTIFYICON_VERSION;
 copystring(nid.szTip,progTitle);
 nid.hIcon=ImageList_GetIcon(hUsageImg,0,ILD_TRANSPARENT);
 //CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)AddIconToTray,(void*)&nid,0,&n));
 Shell_NotifyIcon(NIM_ADD,&nid);
 DestroyIcon(nid.hIcon);
 ///////
 if(FillHndlTbl)FillObjectTypesTable(FillHndlTbl);

 bRunOK=1;
 #ifndef IS_HM_VERSION
  if(isFirstRun)SendMessage(main_win,WM_COMMAND,0x0000C359,0);
 #endif
 *runOK=bRunOK;
 return 1;
exit:
 *runOK=bRunOK;
 return 0;
}

///////////////////////////// E N T R Y   P O I N T ////////////////////////////
#ifdef USE_MS_WinMain
 WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
#else
 extern "C" DWORD _stdcall PROWiSe_Main(){
#endif
#ifdef WRITE_DATA_BIN
 WriteDataBin(); Beep(1000,50); return 0;
#endif
 MSG msg; BOOL runOK;
 if(!EntryPoint_Init(&runOK))goto exit;
 while(GetMessage(&msg,NULL,0,0)){
  if(haccel!=NULL && TranslateAccelerator(main_win,haccel,&msg))continue;
  if(msg.message==WM_KEYDOWN){
   if(services_win && msg.wParam==VK_RETURN)goto trans_dispatch;
   if(processes_win || drvi_win)goto trans_dispatch;
   if(!IsDialogMessage(pHdr->hwndDisplay,&msg))goto trans_dispatch;
  }
  else{
trans_dispatch:
   TranslateMessage(&msg); DispatchMessage(&msg);
  }
  if(lPaneThread!=NULL)PostThreadMessage(lPaneThreadId,msg.message,msg.wParam,msg.lParam);
 }
exit:
 if(!runOK && (progSetts.CheckState[21]&SETTS_REPLACETASKMAN_ENABLE)==SETTS_REPLACETASKMAN_ENABLE){
  FailMessage("Error during startup.",0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_WRITE_LOG);
  ReplaceTaskManager(0);//restore taskmgr if an error occured during startup
 }
 if(hMainKey!=0)RegCloseKey(hMainKey);
 ExitProgram();
 return 0;
}
