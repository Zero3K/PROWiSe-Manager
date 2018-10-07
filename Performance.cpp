
#define RESIZE_SCHEME_SYSIWIN_CPUHIST 1
#define RESIZE_SCHEME_SYSIWIN_CPUBOTH 2

typedef struct _WINDOW_MOUSEMOVE{
 BOOL bMove;// set to TRUE when LBUTTONDOWN. set to FALSE on WM_LBUTTONUP || WM_NCLBUTTONUP
 POINT ptCursor;//Last cursor's coordinates
 HHOOK hMouseHook;
}WINDOW_MOUSEMOVE;

HWND hcpu,hcpugraf,hcomit,hcomitgraf,hdata;
RECT monsz[64]; WORD mdchg,mdcwd;
//bool PerfWinIconic=0;
DWORD_PTR dwpMainwin_style;
DWORD sysiwin_ResizeScheme;
BOOL bMainwinTransformed=0;
WINDOW_MOUSEMOVE mainwin_mousemove;

LRESULT CALLBACK MainWin_Hook_MouseProc(int nCode, WPARAM wParam, LPARAM lParam){
 if(nCode!=HC_ACTION)return (CallNextHookEx(mainwin_mousemove.hMouseHook,nCode,wParam,lParam));
 if(wParam==WM_LBUTTONDOWN){
  mainwin_mousemove.bMove=1;
  mainwin_mousemove.ptCursor.x=((MOUSEHOOKSTRUCT*)lParam)->pt.x;
  mainwin_mousemove.ptCursor.y=((MOUSEHOOKSTRUCT*)lParam)->pt.y;
  SetCapture(main_win);
 }
 else if(wParam==WM_LBUTTONUP || wParam==WM_NCLBUTTONUP){
  if(mainwin_mousemove.bMove){ReleaseCapture(); mainwin_mousemove.bMove=0;}
 }
 else if(wParam==WM_MOUSEMOVE && mainwin_mousemove.bMove){
  int x,y; RECT rect;
  if((GetWindowLongPtr(main_win,GWL_STYLE)&WS_MAXIMIZE)!=WS_MAXIMIZE){
   x=((MOUSEHOOKSTRUCT*)lParam)->pt.x-mainwin_mousemove.ptCursor.x;
   y=((MOUSEHOOKSTRUCT*)lParam)->pt.y-mainwin_mousemove.ptCursor.y;
   if(GetWindowRect(main_win,&rect)){
    rect.right-=rect.left;
    rect.bottom-=rect.top;
    rect.left+=x; rect.top+=y;
    if(MoveWindow(main_win,rect.left,rect.top,rect.right,rect.bottom,1)){
     mainwin_mousemove.ptCursor.x=((MOUSEHOOKSTRUCT*)lParam)->pt.x;
     mainwin_mousemove.ptCursor.y=((MOUSEHOOKSTRUCT*)lParam)->pt.y;
    }
   }
  }
 }
 return (CallNextHookEx(mainwin_mousemove.hMouseHook,nCode,wParam,lParam));
}

void Performance_ChangeScheme(BOOL bRestore){HWND hwin,hwin2; int in,nCmdShow;
 nCmdShow=(!bRestore)?SW_HIDE:SW_SHOW;
 ShowWindow(pHdr->hwndTab,nCmdShow);
 hwin=GetWindow(GetWindow(sysi_win,GW_CHILD),GW_HWNDFIRST);
 hwin2=GetWindow(hwin,GW_HWNDLAST);
 while(hwin!=0){
  in=GetDlgCtrlID(hwin);
  if(sysiwin_ResizeScheme==RESIZE_SCHEME_SYSIWIN_CPUHIST){
   if(in==1103 || in==1104)goto show_this;
  }
  else if(sysiwin_ResizeScheme==RESIZE_SCHEME_SYSIWIN_CPUBOTH){
   if(in==1101 || in==1102 || in==1103 || in==1104)goto show_this;
  }
  ShowWindow(hwin,nCmdShow);
gonext:
  if(hwin==hwin2)break;
  hwin=GetWindow(hwin,GW_HWNDNEXT);
  continue;
show_this:
  ShowWindow(hwin,SW_SHOW);
  goto gonext;
 }
}

void Performance_EnlargeOrRestoreGraph(DWORD dwSchemeType){DWORD_PTR dwp;
 if(!bMainwinTransformed){
  bMainwinTransformed=1;
  dwp=GetWindowLongPtr(main_win,GWL_STYLE);
  dwpMainwin_style=dwp;
  dwp&=~(WS_CAPTION);//|WS_MINIMIZEBOX|WS_MAXIMIZEBOX
  dwp|=WS_BORDER;
  SetMenu(main_win,0);
  sysiwin_ResizeScheme=dwSchemeType;
  Performance_ChangeScheme(0);
  mainwin_mousemove.hMouseHook=SetWindowsHookEx(WH_MOUSE,MainWin_Hook_MouseProc,(HINSTANCE)NULL,GetCurrentThreadId());
 }
 else {
  bMainwinTransformed=0;
  dwp=dwpMainwin_style;
  SetMenu(main_win,main_menu);
  Performance_ChangeScheme(1);
  sysiwin_ResizeScheme=0;
  UnhookWindowsHookEx(mainwin_mousemove.hMouseHook);
 }
 SetWindowLongPtr(main_win,GWL_STYLE,dwp);
 SetWindowPos(main_win,0,0,0,0,0,SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOZORDER|SWP_NOSIZE);
}

BOOL Performance_CPU_Graphs_OnDblClick(HWND hwnd, WPARAM, LPVOID lpGraphData, OUT LPBOOL lpbUpdate){
 Performance_EnlargeOrRestoreGraph(RESIZE_SCHEME_SYSIWIN_CPUBOTH);
 return 1;
}

void Performance_CPU_OscilGraph_PopupMenuInit(HMENU hmenu, LPVOID lpGraphData){MENUITEMINFO mii;
 mii.cbSize=sizeof(MENUITEMINFO);
 mii.fMask=MIIM_TYPE|MIIM_STATE|MIIM_ID;
 mii.fType=MFT_STRING;
 mii.dwTypeData=smemTable->PerfGraph_KernelUsage; mii.wID=30001;
 if(bTurnOffKernelCalc || progSetts.CheckState[26]==0)mii.fState=MFS_DISABLED;
 else if(gdCpuKrnl.dwFlags&CHART_FLAG_HIDDEN)mii.fState=MFS_ENABLED;
 else mii.fState=MFS_ENABLED|MFS_CHECKED;
 InsertMenuItem(hmenu,0,1,&mii);
 mii.dwTypeData=smemTable->PerfGraph_UserUsage; mii.wID=30002;
 if(gdCpu.dwFlags&CHART_FLAG_HIDDEN)mii.fState=MFS_ENABLED;
 else mii.fState=MFS_ENABLED|MFS_CHECKED;
 InsertMenuItem(hmenu,0,1,&mii);
 mii.dwTypeData=(!bMainwinTransformed)?smemTable->PerfGraph_Maximize:smemTable->PerfGraph_Restore; mii.wID=30003;
 mii.fState=MFS_ENABLED;
 InsertMenuItem(hmenu,2,1,&mii);
 mii.fMask=MIIM_TYPE; mii.fType=MFT_SEPARATOR;
 InsertMenuItem(hmenu,2,1,&mii);
}

BOOL Performance_CPU_OscilGraph_PopupMenuWmCommand(HWND hwnd, DWORD dwItemId, LPVOID lpGraphData, OUT LPBOOL lpbUpdate){
 if(dwItemId==30001){
  if(gdCpuKrnl.dwFlags&CHART_FLAG_HIDDEN)gdCpuKrnl.dwFlags&=~CHART_FLAG_HIDDEN;
  else gdCpuKrnl.dwFlags|=CHART_FLAG_HIDDEN;
  *lpbUpdate=1;
  return 0;
 }
 else if(dwItemId==30002){
  if(gdCpu.dwFlags&CHART_FLAG_HIDDEN)gdCpu.dwFlags&=~CHART_FLAG_HIDDEN;
  else gdCpu.dwFlags|=CHART_FLAG_HIDDEN;
  *lpbUpdate=1;
  return 0;
 }
 else if(dwItemId==30003){
  Performance_EnlargeOrRestoreGraph(RESIZE_SCHEME_SYSIWIN_CPUHIST);
  return 0;
 }
 return 1;
}

//////////////////////////// Move Window Content on resize /////////////////////
void MoveContentPerformance(LONG newsz,LONG newwd){HWND htmp; int i,hplus,wplus; POINT pt; int hplus2; HDWP hdwp,hdwpcur;
hplus=newsz; hplus-=369; hplus2=hplus/2;
wplus=newwd; wplus-=551;
if(wplus==0)return;
// Everything is hidden except cpu-history
if(sysiwin_ResizeScheme==RESIZE_SCHEME_SYSIWIN_CPUHIST){
 htmp=GetDlgItem(sysi_win,1103);
 if(htmp!=NULL){SetWindowPos(htmp,0,2,2,newwd-4,newsz-4,SWP_NOZORDER);}
 i=monsz[2].right-monsz[3].right;
 wplus=i/2;
 hplus=monsz[2].bottom-monsz[3].bottom;
 htmp=GetDlgItem(sysi_win,1104);//////CpuGraph
 if(htmp!=NULL){SetWindowPos(htmp,0,2+wplus,4+(hplus/2),newwd-i-2,newsz-(hplus),SWP_NOZORDER|SWP_NOACTIVATE);}
 return;
}
// Everything is hidden except cpu graphs
if(sysiwin_ResizeScheme==RESIZE_SCHEME_SYSIWIN_CPUBOTH){
 if((hdwp=BeginDeferWindowPos(4))==0)return;
 hdwpcur=hdwp;
 // vertgraph
 if((htmp=GetDlgItem(sysi_win,1101))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,2,2,monsz[0].right,newsz-4,SWP_NOZORDER);}
 wplus=monsz[0].right-monsz[1].right;
 hplus=monsz[0].bottom-monsz[1].bottom;
 //
 hdwpcur=DeferWindowPos(hdwpcur,hcpu,0,2+(wplus/2),4+(hplus/2),monsz[1].right,newsz-hplus,SWP_NOZORDER|SWP_NOACTIVATE);
 // oscgraph
 wplus=2+(monsz[2].left-monsz[0].left);
 hplus=newwd-wplus-2;
 htmp=GetDlgItem(sysi_win,1103);
 if(htmp!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,wplus,2,hplus,newsz-4,SWP_NOZORDER);}
 //
 i=(monsz[2].right-monsz[3].right)/2;
 wplus+=i;
 hplus=monsz[2].bottom-monsz[3].bottom;
 htmp=GetDlgItem(sysi_win,1104);//////CpuGraph
 if(htmp!=NULL){DeferWindowPos(hdwpcur,htmp,0,wplus,4+(hplus/2),newwd-(wplus)-i,newsz-(hplus),SWP_NOZORDER|SWP_NOACTIVATE);}
 //
 EndDeferWindowPos(hdwp);
 return;
}
// Normal move content
if((hdwp=BeginDeferWindowPos(64))==0)return;
hdwpcur=hdwp;
for(i=8;i<64;i++){//i=1134;i<=1189;i++
 htmp=GetDlgItem(sysi_win,i+1126);
 if(htmp!=NULL){
  GetWindowRect(htmp,&clt);
  pt.x=clt.left; ScreenToClient(sysi_win,&pt);
  pt.y=monsz[i].top; pt.y+=hplus;
  hdwpcur=DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 }
}
if((htmp=GetDlgItem(sysi_win,1101))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,monsz[0].left,monsz[0].top,monsz[0].right,monsz[0].bottom+hplus2,SWP_NOZORDER);}
hdwpcur=DeferWindowPos(hdwpcur,hcpu,0,monsz[1].left,monsz[1].top,monsz[1].right,monsz[1].bottom+hplus2,SWP_NOZORDER|SWP_NOACTIVATE);//////cpuVert
if((htmp=GetDlgItem(sysi_win,1103))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,monsz[2].left,monsz[2].top,monsz[2].right+wplus,monsz[2].bottom+hplus2,SWP_NOZORDER);}
if((htmp=GetDlgItem(sysi_win,1104))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,monsz[3].left,monsz[3].top,monsz[3].right+wplus,monsz[3].bottom+hplus2,SWP_NOZORDER|SWP_NOACTIVATE);}//////CpuGraph
if((htmp=GetDlgItem(sysi_win,1105))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,monsz[4].left,monsz[4].top+hplus2,monsz[4].right,monsz[4].bottom+hplus2,SWP_NOZORDER);}
if((htmp=GetDlgItem(sysi_win,1106))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,monsz[5].left,monsz[5].top+hplus2,monsz[5].right,monsz[5].bottom+hplus2,SWP_NOZORDER|SWP_NOACTIVATE);}/////vertComit
if((htmp=GetDlgItem(sysi_win,1107))!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,monsz[6].left,monsz[6].top+hplus2,monsz[6].right+wplus,monsz[6].bottom+hplus2,SWP_NOZORDER);}
if((htmp=GetDlgItem(sysi_win,1108))!=NULL){DeferWindowPos(hdwpcur,htmp,0,monsz[7].left,monsz[7].top+hplus2,monsz[7].right+wplus,monsz[7].bottom+hplus2,SWP_NOZORDER|SWP_NOACTIVATE);}//////CommitGraph
EndDeferWindowPos(hdwp);
}

BOOL NtQSI_SysHandleInfo(){DWORD n,nn; BYTE rpts=0;
 queryHandlesAgain: n=ntdllFunctions.pNtQuerySystemInformation(SystemHandleInformation,handleInfo,vm1sz,&nn);
 if(n==STATUS_INFO_LENGTH_MISMATCH && vm1sz<=nn){
  if(handleInfo && VirtualFreeEx((HANDLE)-1,handleInfo,0,MEM_RELEASE))handleInfo=0;
  if(rpts<4){
   vm1sz=nn+512;
   handleInfo=(SYSTEM_HANDLE_INFORMATION*)VirtualAllocEx((HANDLE)-1,0,vm1sz,MEM_COMMIT,PAGE_READWRITE);
   rpts++;
   goto queryHandlesAgain;
  }
 }
 if(n!=0){FailMessage("NtQSI_SHI",n,FMSG_NTSTATUS_CONVERT|FMSG_WRITE_LOG); return 0;}
 return 1;
}

///////////////////////////// Get System Performance Info //////////////////////
void Performance_GetSysInfo(){char lta[33];
 //if(NtQSI_SysHandleInfo()){SetDlgItemNumText2(sysi_win,1181,ltoa(handleInfo->Count,lta,10),t1buf4);}//Handles count
 SYSTEM_PROCESS_INFORMATION *spi; DWORD processesCount=0,threadsCount=0,n,nn,handlCnt,temp1,temp2,temp3; int p;
queryProcessesAgain:
 if(ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,spi_mem,spi_memsz,&nn)==STATUS_INFO_LENGTH_MISMATCH && spi_memsz<nn){
  spi_memsz=nn; spi_mem=LocalReAlloc(spi_mem,spi_memsz,LMEM_MOVEABLE);
  goto queryProcessesAgain;
 }
 handlCnt=0; n=0;
 while(n<=nn){
  spi=(SYSTEM_PROCESS_INFORMATION*)((BYTE*)(spi_mem)+n);
  n+=(spi->NextEntryOffset);
  processesCount++;
  handlCnt+=spi->HandleCount;
  threadsCount+=spi->ThreadCount;
  if((spi->NextEntryOffset)==0)break;
 }
 SetDlgItemNumText2(sysi_win,1181,ltoa(handlCnt,lta,10),t1buf4);//Handles count
 SetDlgItemText(sysi_win,1179,ltoa(processesCount,lta,10));//Processes count
 SetDlgItemText(sysi_win,1180,ltoa(threadsCount,lta,10));//Threads count
 if(basicInfo){
  temp1=SysBasicInf->PhysicalPageSize;
  temp1/=1024;
 }
 else temp1=1;
 temp3=sysPerfInf->TotalCommitLimit;
 temp3*=temp1;
 SetDlgItemNumText2(sysi_win,1158,ltoa(temp3,lta,10),t1buf4);//Commit limit
 temp2=sysPerfInf->TotalCommittedPages;
 temp2*=temp1;
 SetDlgItemNumText2(sysi_win,1157,ltoa(temp2,lta,10),t1buf4);//Commit current
 Math_div(temp2,temp3,lta,1);
 SetDlgItemText(sysi_win,1186,lta);//Commit current/limit - Ќ≈ ѕ–ј¬»Ћ№Ќќ
 temp2=sysPerfInf->PeakCommitment;
 temp2*=temp1;
 SetDlgItemNumText2(sysi_win,1159,ltoa(temp2,lta,10),t1buf4);//Commit peak
 Math_div(temp2,temp3,lta,1);
 SetDlgItemText(sysi_win,1184,lta);//Commit peak/limit
 temp2=sysPerfInf->AvailablePages;
 temp2*=temp1;
 SetDlgItemNumText2(sysi_win,1188,ltoa(temp2,lta,10),t1buf4);//Available Physical Memory
 temp2=sysPerfInf->PagedPoolPage;
 temp2*=temp1;
 SetDlgItemNumText2(sysi_win,1160,ltoa(temp2,lta,10),t1buf4);//Paged Physical Kernel Memory
 temp2=sysPerfInf->PagedPoolUsage;
 temp2*=temp1;
 SetDlgItemNumText2(sysi_win,1161,ltoa(temp2,lta,10),t1buf4);//Paged Virtual Kernel Memory
 temp2=sysPerfInf->NonPagedPoolUsage;
 temp2*=temp1;
 SetDlgItemNumText2(sysi_win,1138,ltoa(temp2,lta,10),t1buf4);//NonPaged Kernel Memory (неподкачиваема€ пам€ть [из KmdKit.chm])
 p=sysPerfInf->PageFaults;
 p-=prevPageFaults; SetDlgItemNumText2(sysi_win,1171,ltoa(p,lta,10),t1buf4);
  prevPageFaults=sysPerfInf->PageFaults;
  p=sysPerfInf->PagesRead;
  p-=prevPageRead; SetDlgItemNumText2(sysi_win,1172,ltoa(p,lta,10),t1buf4);
  prevPageRead=sysPerfInf->PagesRead;
 p=sysPerfInf->PagefilePagesWritten;
 p-=prevPageWrite; SetDlgItemNumText2(sysi_win,1173,ltoa(p,lta,10),t1buf4);
  prevPageWrite=sysPerfInf->PagefilePagesWritten;
  p=sysPerfInf->MappedFilePagesWritten;
  p-=prevMappedWrite; SetDlgItemNumText2(sysi_win,1174,ltoa(p,lta,10),t1buf4);
  prevMappedWrite=sysPerfInf->MappedFilePagesWritten;
 p=sysPerfInf->ContextSwitches;
 p-=prevContextSwitch; SetDlgItemNumText2(sysi_win,1183,ltoa(p,lta,10),t1buf4);
  prevContextSwitch=sysPerfInf->ContextSwitches;
  p=sysPerfInf->ReadOperationCount;
  p-=prevIORead; SetDlgItemNumText2(sysi_win,1176,ltoa(p,lta,10),t1buf4);
  prevIORead=sysPerfInf->ReadOperationCount;
 p=sysPerfInf->WriteOperationCount;
 p-=prevIOWrite; SetDlgItemNumText2(sysi_win,1177,ltoa(p,lta,10),t1buf4);
  prevIOWrite=sysPerfInf->WriteOperationCount;
  p=sysPerfInf->OtherOperationCount;
  p-=prevIOOther; SetDlgItemNumText2(sysi_win,1178,ltoa(p,lta,10),t1buf4);
  prevIOOther=sysPerfInf->OtherOperationCount;
 if(basicInfo){
  temp2=SysBasicInf->NumberOfPhysicalPages;
  temp2*=temp1;
  SetDlgItemNumText2(sysi_win,1182,ltoa(temp2,lta,10),t1buf4);//Total Physical Memory
  temp2=SysBasicInf->PhysicalPageSize;
  temp2/=1024;
  SetDlgItemText(sysi_win,1185,ltoa(temp2,lta,10));//Memory Page Size
  temp2=SysBasicInf->AllocationGranularity;
  temp2/=1024;
  SetDlgItemText(sysi_win,1187,ltoa(temp2,lta,10));//Memory Allocation Size
 }
 if(ntdllFunctions.pNtQuerySystemInformation(SystemCacheInformation,t1nbuf1,2048,&nn)==STATUS_SUCCESS){
  temp2=((SYSTEM_CACHE_INFORMATION*)t1nbuf1)->TransitionSharedPages;
  temp2*=temp1;
  SetDlgItemNumText2(sysi_win,1189,ltoa(temp2,lta,10),t1buf4);//Available Physical Memory
 }
 /*SYSTEM_PROCESSOR_STATISTICS sspi;
 if(pNtQuerySystemInformation(SystemProcessorStatistics,nbuf,2048,&nn)==0){
 }*/
}

///////////////////////////// Update Performance Info //////////////////////////
void Performance_UpdateInfo(){
 vdCpu.numData1=cpuUsage;
 vdCpu.numData2=cpuKernelTime;
 if(progSetts.CheckState[14]==MEMORYTYPE_USAGE_COMMIT)vdCommit.numData3=(SysBasicInf->PhysicalPageSize)*(sysPerfInf->TotalCommittedPages);
 else {
  vdCommit.numData3=SysBasicInf->NumberOfPhysicalPages;
  vdCommit.numData3-=sysPerfInf->AvailablePages;
  vdCommit.numData3*=SysBasicInf->PhysicalPageSize;
 }
 vdCommit.numData1=commit_prc;
 //if(!PerfWinIconic){
  Chart_VertGraph_Draw(hcpu);
  Chart_VertGraph_Draw(hcomit);
  Chart_OscilGraph_Draw(hcpugraf);
  Chart_OscilGraph_Draw(hcomitgraf);
 //}
 Performance_GetSysInfo();
}

/////////////////////////// SysPerformance Info MainProc ///////////////////////
BOOL CALLBACK SysInfo_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
switch(Message){
case WM_INITDIALOG: sysi_win=hDlg;
 Chart_RegisterClass();
 WindowFillText(hDlg,dlgTAT->dlg6);
 hcpu=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,15,21,69,65,hDlg,(HMENU)1102,gInst,0);
 hcomit=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,15,107,69,65,hDlg,(HMENU)1106,gInst,0);
 // OscilGraphData CPU-User
 gdCpu.dwFlags|=CHART_FLAG_INVERSE_HG;
 gdCpu.hPen=CreatePen(PS_SOLID,1,0x0000ff00);
 gdCpu.histStartAddr=(DWORD_PTR)usrHistStart;
 gdCpu.histLastAddr=(float*)usrHistEnd;
 gdCpu.NextGraphData=0;
 // OscilGraphData CPU-Kernel
 gdCpuKrnl.dwFlags|=CHART_FLAG_INVERSE_HG;
 gdCpuKrnl.hPen=CreatePen(PS_SOLID,1,0x000000ff);
 gdCpuKrnl.histStartAddr=(DWORD_PTR)krnHistStart;
 gdCpuKrnl.histLastAddr=(float*)krnHistEnd;
 gdCpuKrnl.NextGraphData=&gdCpu;
 // OscilGraphData Memory
 gdCommit.hPen=CreatePen(PS_SOLID,1,0x0000ffff);
 gdCommit.histStartAddr=(DWORD_PTR)cmtHistStart;
 gdCommit.histLastAddr=(float*)cmtHistEnd;
 gdCommit.NextGraphData=0;
 //
 HPEN hPenGrid; HBRUSH hBackgBrush;
 hBackgBrush=CreateSolidBrush(0);
 hPenGrid=CreatePen(PS_SOLID,1,0x00006500);
 // OscilGraph CPU
 mainperf_ogdCpu.genInfo.BackgColor=0;
 mainperf_ogdCpu.genInfo.hBackgBrush=hBackgBrush;
 mainperf_ogdCpu.genInfo.PopupMenuInit=Performance_CPU_OscilGraph_PopupMenuInit;
 mainperf_ogdCpu.genInfo.PopupMenuWmCommand=Performance_CPU_OscilGraph_PopupMenuWmCommand;
 mainperf_ogdCpu.genInfo.OnDblClick=Performance_CPU_Graphs_OnDblClick;
 mainperf_ogdCpu.hPenGrid=hPenGrid;
 mainperf_ogdCpu.oscGridStatic=0;
 mainperf_ogdCpu.oscGridHorzMove=2;
 mainperf_ogdCpu.FirstGraphData=&gdCpuKrnl; // можно сделать выбор что на первый план - kernel или total
 // OscilGraph Memory
 mainperf_ogdMem.genInfo.BackgColor=0;
 mainperf_ogdMem.genInfo.hBackgBrush=hBackgBrush;
 mainperf_ogdMem.hPenGrid=hPenGrid;
 mainperf_ogdMem.oscGridStatic=0;
 mainperf_ogdMem.oscGridHorzMove=2;
 mainperf_ogdMem.FirstGraphData=&gdCommit;
 // VerticalGraph CPU
 vdCpu.genInfo.BackgColor=0;
 vdCpu.genInfo.hBackgBrush=hBackgBrush;
 vdCpu.genInfo.OnDblClick=Performance_CPU_Graphs_OnDblClick;
 vdCpu.drwColor=0x0000ff00;
 vdCpu.dType=GRAPHTYPE_PERCENT|GRAPHTYPE_DOUBLE;
 vdCpu.numData1=cpuUsage;
 // VerticalGraph Memory
 vdCommit.genInfo.BackgColor=0;
 vdCommit.genInfo.hBackgBrush=hBackgBrush;
 vdCommit.drwColor=0x0000ffff;
 vdCommit.dType=GRAPHTYPE_BYTESIZE;
 //
  hcpugraf=GetDlgItem(hDlg,1104); GetWindowRect(hcpugraf,&strect);
  MapWindowPoints(NULL,hDlg,(LPPOINT)&strect,2);
  strect.right-=strect.left; strect.bottom-=strect.top; DestroyWindow(hcpugraf);
 hcpugraf=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,strect.left,strect.top,strect.right,strect.bottom,hDlg,(HMENU)1104,gInst,0);
  hcomitgraf=GetDlgItem(hDlg,1108); GetWindowRect(hcomitgraf,&strect);
  MapWindowPoints(NULL,hDlg,(LPPOINT)&strect,2);
  strect.right-=strect.left; strect.bottom-=strect.top; DestroyWindow(hcomitgraf);
 hcomitgraf=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,strect.left,strect.top,strect.right,strect.bottom,hDlg,(HMENU)1108,gInst,0);
 Chart_Initialize(hcpu,&vdCpu,CHART_VERTICAL);
 Chart_Initialize(hcomit,&vdCommit,CHART_VERTICAL);
 Chart_Initialize(hcomitgraf,&mainperf_ogdMem,CHART_DIAGRAMM);
 Chart_Initialize(hcpugraf,&mainperf_ogdCpu,CHART_DIAGRAMM);
 if(progSetts.CheckState[14]){
  SetDlgItemText(sysi_win,1105,smemTable->Commit);
  SetDlgItemText(sysi_win,1107,smemTable->CommitHistory);
 }
 else if(progSetts.CheckState[15]){
  SetDlgItemText(sysi_win,1105,smemTable->RAMusage);
  SetDlgItemText(sysi_win,1107,smemTable->PhysMemUsageHist);
 }
 HWND htmp; POINT pt; int i;
 for(i=0;i<=7;i++){
  htmp=GetDlgItem(sysi_win,i+1101);
  if(htmp!=NULL){
   GetClientRect(htmp,&monsz[i]);
   GetWindowRect(htmp,&clt);
   pt.y=clt.top; pt.x=clt.left;
   ScreenToClient(sysi_win,&pt);
   monsz[i].left=pt.x;
   monsz[i].top=pt.y;
  }
 }
  for(i=8;i<64;i++){//i=1134;i<=1189;i++
  htmp=GetDlgItem(sysi_win,i+1126);
  if(htmp!=NULL){
   GetWindowRect(htmp,&clt); pt.y=clt.top; pt.x=clt.left;
   ScreenToClient(sysi_win,&pt);
   monsz[i].left=pt.x;
   monsz[i].top=pt.y;
  }
 }
 MainTab_OnChildDialogInit(hDlg);
 GetClientRect(hDlg,&strect);
 MoveContentPerformance(strect.bottom,strect.right);
 bPaneDestroy=0;
 ControlUpdateThread(UPDATETHREAD_UPDATE,0xFF,0);
 break;
case WM_DESTROY: sysi_win=NULL;
 DestroyWindow(hcpu); DestroyWindow(hcomit);
 DestroyWindow(hcpugraf); DestroyWindow(hcomitgraf);
 DeleteObject(mainperf_ogdCpu.genInfo.hBackgBrush);
 DeleteObject(mainperf_ogdCpu.hPenGrid);
 DeleteObject(gdCpu.hPen);
 DeleteObject(gdCpuKrnl.hPen);
 DeleteObject(gdCommit.hPen);
 return 1;
case WM_SIZE:
 MoveContentPerformance(HIWORD(lParam),LOWORD(lParam));
 return 1;
/*case WM_SHOWWINDOW:
 if(wParam==0)PerfWinIconic=1;
 else {PerfWinIconic=0;Performance_UpdateInfo();}
 break;*/
} return 0;}

/*
SystemPagefileInformation содержит physical information on:
 current size of pagefile
 total used
 peak used
 filename
SystemBasicInformation содержит:
 PhysicalPageSize [надо дл€ current_size*physicalPageSize и т.д.]
SystemPerformanceInformation содержит:
 TotalCommittedPages  =>  pagefile usage = TotalCommittedPages*physicalPageSize
 TotalCommitLimit
 PeakCommitment
 ContextSwitches
*/

 /* taskman button props:
 style=0x5800000B = WS_CHILDWINDOW|WS_VISIBLE|WS_DISABLED|BS_OWNERDRAW
 styleEx=0x204 = WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_NOPARENTNOTIFY|WS_EX_CLIENTEDGE
 class styles=0x408B = CS_VREDRAW|CS_HREDRAW|CS_DBLCLKS|CS_PARENTDC|CS_GLOBALCLASS
 windowbytes=4
 menu=icon=bckgrnd=none
   prowise:
 style=0x50000000 = WS_CHILDWINDOW|WS_VISIBLE
 styleEx=0x200 = WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_CLIENTEDGE
 class styles=0
 windowbytes=0
 menu=icon=none
 bckgrnd=.....
 */
