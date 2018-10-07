#define DLGHDR3_SIGNATURE 159753852

DWORD ControlIDsToFill[11]={1183,3,25,40,1015,1032,19,1035,35,1190,1007};

typedef struct _PROCESS_PROPERTIES{
 DWORD ProcessID;
 HICON hIcon;
 char *FileDescription;//неизменно +    // первый в LocalFree_loop !!!
 char *FileCompany;//неизменно     +
 char *FileVersion;//неизменно     +
 char *FileCopyright;//неизменно +
 char *FilePath;//неизменно        +
 char *CommandLine;//неизменно     +
 char *UserName;//неизменно        +
 char *LastCurrentDir;// перепроверять +
 char *ParentProcess;//неизменно   +
 char *Desktop;// перепроверять    +
 char *StartTime;//неизменно;      +    // последний в LocalFree_loop !!!
 DWORD BuildTimeStamp;//неизменно  +
 LARGE_INTEGER ExitTime;// определять при handle=0; неизменно +
 DWORD ExitStatus;
 bool bExit;
}PROCESS_PROPERTIES;

typedef struct _DLGHDR3{
 DWORD Signature;
 HWND hwndTab;
 HWND hwndDisplay;
 HANDLE hHeap;
 HANDLE hProcess;
 HFONT dlgFont3;
 RECT rcDisplay;
 DWORD prevPageFaults;
 LARGE_INTEGER prevKernelTm;
 LARGE_INTEGER prevTotalTm;
 unsigned __int64 prevIORead;
 unsigned __int64 prevIOWrite;
 unsigned __int64 prevIOOther;
 __int64 oldCurTm;
 VERTICAL_GRAPH_DATA vdCpu;
 VERTICAL_GRAPH_DATA vdPrivate;
 OSCILGRAPH_DATA ogdCpu;
 OSCILGRAPH_DATA ogdMem;
 OSCIL_GRAPH_DATA gdCpu;
 OSCIL_GRAPH_DATA gdCpuKernel;
 OSCIL_GRAPH_DATA gdPrivate;
 PROCESS_PROPERTIES prcprops;
 PROCESS_SERVICES *prcsvc;
 PROCESS_INFO *pinf;
 void *PerfWriteMem;//занят в WritePerfInfo постоянно
 DLGTEMPLATE *apRes[5];
 DLGPROC *dlgProC[5];
 WORD iSel3;
 SC_HANDLE scman;
 BYTE vbuf[300];//занят в GetPerfInfo на Perf_info Tab
 char buf[50];//занят в GetPerfInfo на Perf_info Tab
 HWND srvHlist;
 bool bTitleSet;
} DLGHDR3;

#define VALIDATE_ProcInfoWnd(pVar) (pVar->Signature=DLGHDR3_SIGNATURE)
#define INVALIDATE_ProcInfoWnd(pVar) (pVar->Signature=0)
#define IsVALID_ProcInfoWnd(pVar) ((BOOL)(pVar!=0 && pVar->Signature==DLGHDR3_SIGNATURE))
#define GETLONGPTR_ProcInfoWnd(hDlg,ppVar) ((BOOL)((*ppVar=(DLGHDR3*)GetWindowLongPtr(hDlg,GWLP_USERDATA))!=0 && *ppVar->Signature==DLGHDR3_SIGNATURE))

void OnChildDialogInit3(HWND hDlg){DLGHDR3 *pHdr3;
 if((pHdr3=(DLGHDR3*)GetWindowLong(GetParent(hDlg),GWL_USERDATA))==0 || pHdr3->Signature!=DLGHDR3_SIGNATURE)return;
 SetWindowPos(hDlg,HWND_TOP,5,tabtop+2,pHdr3->rcDisplay.right-5,pHdr3->rcDisplay.bottom-4,0);
 if(pEnableThemeDialogTexture>0)pEnableThemeDialogTexture(hDlg,0x00000006);//ETDT_ENABLETAB or ETDT_USETABTEXTURE
}

void OnTabSelChanged3(HWND hDlg){WORD iSel3; DLGHDR3 *pHdr3;
 if((pHdr3=(DLGHDR3*)GetWindowLong(hDlg,GWL_USERDATA))==0 || pHdr3->Signature!=DLGHDR3_SIGNATURE)return;
 iSel3=(WORD)SendMessage(pHdr3->hwndTab,TCM_GETCURSEL,0,0); pHdr3->iSel3=iSel3;
 if(pHdr3->hwndDisplay!=NULL)DestroyWindow(pHdr3->hwndDisplay);
 pHdr3->hwndDisplay=CreateDialogIndirect(gInst,pHdr3->apRes[iSel3],hDlg,(DLGPROC)pHdr3->dlgProC[iSel3]);
 ShowWindow(pHdr3->hwndDisplay,SW_SHOW);
}

//////////////////////////////// TimeToStr2 ////////////////////////////////////
void getTime(SYSTEMTIME *stm,char *tbuf,bool Mlsc){BYTE ni; char lta[5];
 ltoa(stm->wHour,lta,10);
 if(lta[1]==0){tbuf[0]='0';tbuf[1]=lta[0];}
 else {tbuf[0]=lta[0];tbuf[1]=lta[1];}
 ltoa(stm->wMinute,lta,10);
 if(lta[1]==0){tbuf[3]='0';tbuf[4]=lta[0];}
 else {tbuf[3]=lta[0];tbuf[4]=lta[1];}
 ltoa(stm->wSecond,lta,10);
 if(lta[1]==0){tbuf[6]='0';tbuf[7]=lta[0];}
 else {tbuf[6]=lta[0];tbuf[7]=lta[1];}
 if(Mlsc){
  ltoa(stm->wMilliseconds,lta,10); ni=0;
  if(lta[0]){
   ni=1;
   if(lta[1]){
    if(lta[2])ni=3;
    else ni=2;
   }
  }
  *(DWORD*)(tbuf+9)=0x00303030;
  if(n>0){*(DWORD*)(tbuf+12-ni)=*(DWORD*)lta;}
 }
 tbuf[2]=':'; tbuf[5]=':';
 if(Mlsc){tbuf[8]='.'; tbuf[12]=0;}
 else tbuf[8]=0;
}

//////////////////////////////// Get Performance Info //////////////////////////
void GetPerfInfo(HWND hDlg){DLGHDR3 *pHdr3; HANDLE hProcess; SYSTEMTIME stm; char *buf; void *vbuf; PROCESS_INFO *pinf;
 if((pHdr3=(DLGHDR3*)GetWindowLong(GetParent(hDlg),GWL_USERDATA))==0)return;
 vbuf=&(pHdr3->vbuf); buf=pHdr3->buf;
 hProcess=pHdr3->hProcess; pinf=pHdr3->pinf;
 if(!IsVALID_PROCESS_INFO(pinf))pinf=0;
 DWORD nn,t;
 if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessBasicInformation,vbuf,sizeof(PROCESS_BASIC_INFORMATION),&t)==0)nn=((PROCESS_BASIC_INFORMATION*)vbuf)->BasePriority;
 else if(pinf)nn=pinf->BasePriority;
 else goto skip1;
 SetDlgItemInt(hDlg,2042,nn,0);
skip1:
 if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessHandleCount,&nn,4,&t)!=0){
  if(pinf)nn=pinf->HandleCount;
  else goto skip2;
 }
 SetDlgItemNumText(hDlg,2048,ltoa(nn,buf,10));
skip2:
 if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessTimes,vbuf,sizeof(PROCESS_TIMES),&t)==0){
  PROCESS_TIMES *ptm; LARGE_INTEGER lint;
  ptm=(PROCESS_TIMES*)vbuf;
  FileTimeToSystemTime((FILETIME*)&(ptm->KernelTime),&stm); getTime(&stm,buf,1); SetDlgItemText(hDlg,2012,buf);
  FileTimeToSystemTime((FILETIME*)&(ptm->UserTime),&stm); getTime(&stm,buf,1); SetDlgItemText(hDlg,2013,buf);
  lint.QuadPart=ptm->KernelTime.QuadPart; lint.QuadPart+=ptm->UserTime.QuadPart;
  FileTimeToSystemTime((FILETIME*)&lint,&stm); getTime(&stm,buf,1); SetDlgItemText(hDlg,2014,buf);
 }
 VM_COUNTERS* vmc;
 if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessVmCounters,vbuf,sizeof(VM_COUNTERS),&t)==0)vmc=(VM_COUNTERS*)vbuf;
 else if(pinf && pinf->spi)vmc=&pinf->spi->VmCounters;
 else goto skip4;
 t=vmc->WorkingSetSize; t/=1024; SetDlgItemNumText(hDlg,2008,ltoa(t,buf,10));
 t=vmc->PeakWorkingSetSize; t/=1024; SetDlgItemNumText(hDlg,2064,ltoa(t,buf,10));
 SetDlgItemNumText(hDlg,2065,ltoa(vmc->PageFaultCount,buf,10));
 t=vmc->PagefileUsage; t/=1024; SetDlgItemNumText(hDlg,2009,ltoa(t,buf,10));
 t=vmc->PeakPagefileUsage; t/=1024; SetDlgItemNumText(hDlg,2010,ltoa(t,buf,10));
 t=vmc->VirtualSize; t/=1024; SetDlgItemNumText(hDlg,2007,ltoa(t,buf,10));
 t=vmc->PeakVirtualSize; t/=1024; SetDlgItemNumText(hDlg,2006,ltoa(t,buf,10));
 t=vmc->PageFaultCount-pHdr3->prevPageFaults; SetDlgItemNumText(hDlg,2066,ltoa(t,buf,10));
 pHdr3->prevPageFaults=vmc->PageFaultCount;
skip4:
 if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessIoCounters,vbuf,sizeof(IO_COUNTERS),&t)==0){
  IO_COUNTERS *ioc; ioc=(IO_COUNTERS*)vbuf;
  SetDlgItemNumText(hDlg,2041,_i64toa(ioc->ReadOperationCount,buf,10));
  SetDlgItemNumText(hDlg,2043,_i64toa(ioc->WriteOperationCount,buf,10));
  SetDlgItemNumText(hDlg,2045,_i64toa(ioc->OtherOperationCount,buf,10));
  int q;
  q=(int)(ioc->ReadOperationCount-pHdr3->prevIORead); SetDlgItemNumText(hDlg,2044,ltoa(q,buf,10));
  q=(int)(ioc->WriteOperationCount-pHdr3->prevIOWrite); SetDlgItemNumText(hDlg,2050,ltoa(q,buf,10));
  q=(int)(ioc->OtherOperationCount-pHdr3->prevIOOther); SetDlgItemNumText(hDlg,2049,ltoa(q,buf,10));
  pHdr3->prevIORead=ioc->ReadOperationCount;
  pHdr3->prevIOWrite=ioc->WriteOperationCount;
  pHdr3->prevIOOther=ioc->OtherOperationCount;
 }
 if(hProcess!=0){
  SetDlgItemNumText(hDlg,2046,ltoa(GetGuiResources(hProcess,GR_GDIOBJECTS),buf,10));
  SetDlgItemNumText(hDlg,2047,ltoa(GetGuiResources(hProcess,GR_USEROBJECTS),buf,10));
 }
 else{
  FileTimeToSystemTime((FILETIME*)&(pHdr3->prevKernelTm),&stm); getTime(&stm,buf,1); SetDlgItemText(hDlg,2012,buf);
  FileTimeToSystemTime((FILETIME*)&(pHdr3->prevTotalTm),&stm); getTime(&stm,buf,1); SetDlgItemText(hDlg,2014,buf);
 }
}

void getDate(SYSTEMTIME *stm, char *dtBuf){char num[5];
 ltoa(stm->wDay,num,10);
 if(num[1]==0){*dtBuf=0x30;*(dtBuf+1)=num[0];}
 else {*dtBuf=num[0];*(dtBuf+1)=num[1];}
 *(dtBuf+2)='.'; dtBuf+=3;
 ltoa(stm->wMonth,num,10);
 if(num[1]==0){*dtBuf=0x30;*(dtBuf+1)=num[0];}
 else {*dtBuf=num[0];*(dtBuf+1)=num[1];}
 *(dtBuf+2)='.'; dtBuf+=3;
 ltoa(stm->wYear,num,10);
 *(DWORD*)dtBuf=*(DWORD*)num;
 *(dtBuf+4)=0;
}

void TimeStampToFileTime(DWORD tmst,char *tBuf){FILETIME ftm; SYSTEMTIME stm;
 LONGLONG ll=Int32x32To64(tmst,10000000)+116444736000000000;
 ftm.dwLowDateTime=(DWORD)ll; ll>>=32;
 ftm.dwHighDateTime=(DWORD)ll;
 FileTimeToSystemTime(&ftm,&stm);
 getTime(&stm,tBuf,0); *((DWORD*)tBuf+2)=0x202D20;
 getDate(&stm,tBuf+11);
}

void GetAndShowStartTime(DLGHDR3 *pHdr3,DWORD dwLowDateTime,DWORD dwHighDateTime){SYSTEMTIME stm;
 if(pHdr3->prcprops.StartTime)return;
 if(dwLowDateTime==0 && dwHighDateTime==0){copystring(pHdr3->buf,smemTable->n_a); goto settext;}
 FILETIME ftm; FILETIME lftm;
 ftm.dwLowDateTime=dwLowDateTime; ftm.dwHighDateTime=dwHighDateTime;
 FileTimeToLocalFileTime(&ftm,&lftm);
 FileTimeToSystemTime(&lftm,&stm);
 getTime(&stm,pHdr3->buf,1);
 getDate(&stm,pHdr3->buf+15);
 pHdr3->buf[12]=0x20;pHdr3->buf[13]='-';pHdr3->buf[14]=0x20;
settext:
 pHdr3->prcprops.StartTime=AllocateAndCopy(0,pHdr3->buf);
}

void StrAppendPID(DWORD pID, char *newStr,char *buff,char **prevStr){DWORD len;
 *(DWORD*)buff=0x00002820;
 ltoa(pID,(char*)(buff+2),10);
 strappend(newStr,buff);
 len=getstrlen(newStr);
 newStr[len]=')'; len++; newStr[len]=0;
 CompareAndReAllocateA(prevStr,newStr);
}

/////////////////////////////////// Get ImageInfo //////////////////////////////
void GetImageInfo(HWND hDlg){HANDLE hProcess; PROCESS_BASIC_INFORMATION pbi; PROCESS_TIMES ptm; DWORD t1,nm; void *dbuf1,*dbuf2;
 BYTE *lmem; char *sbuf; DLGHDR3 *pHdr3; lmem=0; DWORD_PTR dwp,dwp2;
 if((dwp=(DWORD_PTR)LocalAlloc(LPTR,2048+2048+512))==0)return;
 dbuf1=(void*)dwp; dwp+=2048;
 dbuf2=(void*)dwp; dwp+=2048;
 sbuf=(char*)dwp;//512 bytes
 pbi.PebBaseAddress=0; pbi.ParentProcessId=0xFFFFFFFF;
 if((pHdr3=(DLGHDR3*)GetWindowLongPtr(GetParent(hDlg),GWLP_USERDATA))==0)goto quit;
 if(pHdr3->hProcess!=0){
  hProcess=pHdr3->hProcess;
  if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessBasicInformation,&pbi,sizeof(pbi),&nm)!=0)goto skip_exit;
  if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessTimes,&ptm,sizeof(PROCESS_TIMES),&nm)==0){
   if(ptm.ExitTime.QuadPart>0){
    pHdr3->prcprops.ExitTime.QuadPart=ptm.ExitTime.QuadPart;
    pHdr3->prcprops.ExitStatus=pbi.ExitStatus;
    CloseHandle(pHdr3->hProcess);
    pHdr3->hProcess=0; pHdr3->prcprops.bExit=1;
    goto handle_null;
   }
  }
  if(pbi.PebBaseAddress==0)goto skip_exit;
  if(ReadProcessMemory(hProcess,pbi.PebBaseAddress,(void*)dbuf1,sizeof(PEB),0)==0)goto skip_exit;
  dwp=(DWORD_PTR)((PEB*)dbuf1)->ImageBaseAddress; dwp2=dwp;
  dwp+=0x3C;
  if(pHdr3->prcprops.BuildTimeStamp!=0)goto skip_timestamp;
  if(ReadProcessMemory(hProcess,(void*)dwp,(void*)dbuf2,4,0)==0)goto skip_timestamp;
  dwp2+=*(DWORD*)dbuf2;
  if(ReadProcessMemory(hProcess,(void*)dwp2,(void*)dbuf2,0x0C,0)==0)goto skip_timestamp;
  if(*(DWORD*)dbuf2!=0x00004550)goto skip_timestamp;
  dwp2=*((DWORD*)dbuf2+2); pHdr3->prcprops.BuildTimeStamp=dwp2;
 skip_timestamp:
  if(((PEB*)dbuf1)->ProcessParameters==NULL)goto skip_exit;
  char *ptr1;
  if(((PEB*)dbuf1)->BeingDebugged>0)ptr1=smemTable->Yes;
  else ptr1=smemTable->No;
  SetDlgItemText(hDlg,1161,ptr1);
  // get image path
  if(ReadProcessMemory(hProcess,((PEB*)dbuf1)->ProcessParameters,(void*)dbuf1,sizeof(RTL_USER_PROCESS_PARAMETERS),0)==0)goto skip_exit;
  if(pHdr3->prcprops.FilePath)goto skip_inf1;
  if(((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->ImagePathName.Buffer==NULL)goto skip_inf1;
  t1=((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->ImagePathName.Length; if(t1>2048)t1=2048;
  if(t1==0 || ReadProcessMemory(hProcess,((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->ImagePathName.Buffer,(void*)dbuf2,t1,0)==0)goto skip_inf1;
  UnicodeStringToAnsiString((LPWSTR)dbuf2,t1,sbuf,512);
  pHdr3->prcprops.FilePath=AllocateAndCopy(0,sbuf);
  WORD wn; char *fptr; wn=0;
  fptr=CorrectFilePath(sbuf,0,dbuf2);
  ExtractIconEx(fptr,0,&pHdr3->prcprops.hIcon,NULL,1);
  if(pHdr3->prcprops.hIcon==NULL)ExtractIconEx(fptr,0,NULL,&pHdr3->prcprops.hIcon,1);
  if(pHdr3->prcprops.hIcon==NULL)pHdr3->prcprops.hIcon=ExtractAssociatedIcon(gInst,fptr,&wn);
  DWORD w1,w2,w3,w4; void *vmem;
  w1=w2=w3=w4=1;
  vmem=FileModule_GetVerDescInfo(fptr,&w3,&w1,&w2,&w4);
  if(w1)pHdr3->prcprops.FileDescription=AllocateAndCopy(0,(char*)w1);
  if(w2)pHdr3->prcprops.FileCompany=AllocateAndCopy(0,(char*)w2);
  if(w3)pHdr3->prcprops.FileVersion=AllocateAndCopy(0,(char*)w3);
  if(w4)pHdr3->prcprops.FileCopyright=AllocateAndCopy(0,(char*)w4);
  if(vmem)LocalFree(vmem);
  /////////////////////
 skip_inf1:
  if(pHdr3->prcprops.CommandLine)goto skip_inf2;
  if(((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->CommandLine.Buffer==NULL)goto skip_inf2;
  t1=((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->CommandLine.Length; if(t1>2048)t1=2048;
  if(t1==0 || ReadProcessMemory(hProcess,((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->CommandLine.Buffer,(void*)dbuf2,t1,0)==0)goto skip_inf2;
  UnicodeStringToAnsiString((LPWSTR)dbuf2,t1,sbuf,512);
  pHdr3->prcprops.CommandLine=AllocateAndCopy(0,sbuf);
  /////////////////////
 skip_inf2:
  if(((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->CurrentDirectoryPath.Buffer==NULL)goto skip_inf3;
  t1=((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->CurrentDirectoryPath.Length; if(t1>2048)t1=2048;
  if(t1==0 || ReadProcessMemory(hProcess,((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->CurrentDirectoryPath.Buffer,(void*)dbuf2,t1,0)==0)goto skip_inf3;
  UnicodeStringToAnsiString((LPWSTR)dbuf2,t1,sbuf,512);
  CompareAndReAllocateA(&(pHdr3->prcprops.LastCurrentDir),sbuf);
 skip_inf3:
  if(((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->DesktopName.Buffer==NULL)goto skip_inf4;
  t1=((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->DesktopName.Length; if(t1>2048)t1=2048;
  if(t1==0 || ReadProcessMemory(hProcess,((RTL_USER_PROCESS_PARAMETERS*)dbuf1)->DesktopName.Buffer,(void*)dbuf2,t1,0)==0)goto skip_inf4;
  UnicodeStringToAnsiString((LPWSTR)dbuf2,t1,sbuf,512);
  CompareAndReAllocateA(&(pHdr3->prcprops.Desktop),sbuf);
 skip_inf4:
 skip_exit:
 }
 else {//Set ExitTime & ExitStatus
  handle_null:
  if(!pHdr3->prcprops.bExit)SetDlgItemText(hDlg,1183,smemTable->_UnblOpenProcess_);
  if(pHdr3->prcprops.ExitTime.QuadPart==0)goto skip_set_exitdata;
  HWND hwin;
  EnableWindow(GetDlgItem(hDlg,1004),0);
  hwin=GetDlgItem(hDlg,3011);
  SYSTEMTIME stm; FILETIME lftm; char *ptxt;
  FileTimeToLocalFileTime((FILETIME*)&(pHdr3->prcprops.ExitTime),&lftm);
  FileTimeToSystemTime(&lftm,&stm);
  getTime(&stm,pHdr3->buf,1);
  getDate(&stm,pHdr3->buf+15);
  pHdr3->buf[12]=0x20;pHdr3->buf[13]='-';pHdr3->buf[14]=0x20;
  nm=getstrlen(smemTable->ProcessIsTerminated_br);
  nm+=getstrlen(smemTable->_br_ExitCode_);
  nm+=getstrlen(pHdr3->buf);
  nm+=1+8+2; ptxt=(char*)LocalAlloc(LMEM_FIXED,nm);
  copystring(ptxt,smemTable->ProcessIsTerminated_br);
  strappend(ptxt,pHdr3->buf);
  if(pHdr3->prcprops.ExitStatus){*pHdr3->buf=0; HexToString(pHdr3->prcprops.ExitStatus,pHdr3->buf);}
  else *(DWORD*)pHdr3->buf=0x00000030;
  strappend(ptxt,smemTable->_br_ExitCode_);
  strappend(ptxt,pHdr3->buf);
  LOGFONT tabf; HDC hdc; HFONT hFont;
  if((hdc=GetDC(hwin))!=0){
   TEXTMETRIC txtm; bool bl;
   bl=GetTextMetrics(hdc,&txtm);
   ReleaseDC(hwin,hdc);
   if(bl){tabf.lfHeight=txtm.tmAscent; tabf.lfWidth=0; goto create_font;}
  }
  tabf.lfHeight=8; tabf.lfWidth=0;
  create_font:
  tabf.lfCharSet=RUSSIAN_CHARSET; tabf.lfWeight=FW_BOLD; tabf.lfOutPrecision=OUT_DEFAULT_PRECIS;
  lstrcpy(tabf.lfFaceName,MSsansSerif_txt); tabf.lfEscapement=0; tabf.lfOrientation=0; tabf.lfUnderline=0; tabf.lfStrikeOut=0; tabf.lfItalic=0;
  hFont=CreateFontIndirect(&tabf);
  SendMessage(hwin,WM_SETFONT,(WPARAM)hFont,(LPARAM)TRUE);
  SetWindowText(hwin,ptxt);
  LocalFree(ptxt);
  ShowWindow(hwin,SW_SHOW);
 }
 skip_set_exitdata:
 if(pbi.ParentProcessId==0xFFFFFFFF){
  if(pHdr3->pinf==0 || pHdr3->prcprops.ProcessID==0)goto skip_parent;
  pbi.ParentProcessId=pHdr3->pinf->ParentProcessID;
 }
 if(pHdr3->prcprops.ParentProcess && pHdr3->prcprops.UserName && pHdr3->prcprops.StartTime && pHdr3->bTitleSet)goto skip_parent;
 SYSTEM_PROCESS_INFORMATION *spi; DWORD n,sz; lmem=0; sz=0; t1=0;
 queryAgain: n=ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,lmem,sz,&nm);
 if(n==0xC0000004 && sz<=nm){
  sz=nm+1024;
  if(lmem)lmem=(BYTE*)LocalFree(lmem);
  if(t1<3){t1++;
   lmem=(BYTE*)LocalAlloc(LMEM_FIXED,sz);
   goto queryAgain;
  }
 }
 else if(n!=0)goto quit;
 n=0; bool ok; ok=0; STRING str; str.Length=0; str.MaximumLength=512;
 while(n<=nm){
  spi=(SYSTEM_PROCESS_INFORMATION*)(lmem+n);
  n+=(spi->NextEntryOffset);
  if(spi->UniqueProcessId==pbi.ParentProcessId){
   if(spi->UniqueProcessId!=0){
    if(spi->ProcessName.Length>2048)spi->ProcessName.Length=2048;
    str.Buffer=sbuf;
    ntdllFunctions.RtlUnicodeStringToAnsiString(&str,&spi->ProcessName,0);
    *(str.Buffer+str.Length)=0;
   }
   else copybytes(sbuf,smemTable->SysIdleProc,150,1);
   StrAppendPID(pbi.ParentProcessId,sbuf,(char*)dbuf1,&(pHdr3->prcprops.ParentProcess));
   ok=1;
  }
  else if(spi->UniqueProcessId==pHdr3->prcprops.ProcessID){
   if(!pHdr3->prcprops.StartTime){
    GetAndShowStartTime(pHdr3,spi->CreateTime.LowPart,spi->CreateTime.HighPart);
   }
   if(!pHdr3->bTitleSet){
    if(spi->ProcessName.Length>2048)spi->ProcessName.Length=2048;
    str.Buffer=sbuf;
    ntdllFunctions.RtlUnicodeStringToAnsiString(&str,&spi->ProcessName,0);
    *(str.Buffer+str.Length)=0;
    *(DWORD*)dbuf1=0x00002820;
    ltoa(spi->UniqueProcessId,((char*)(dbuf1))+2,10);
    strappend(sbuf,(char*)(dbuf1));
    strappend(sbuf,smemTable->_Properties);
    if(SetWindowText(GetParent(hDlg),sbuf))pHdr3->bTitleSet=1;
   }
   if(!pHdr3->prcprops.UserName){
    Process_GetMachineUserName(spi->UniqueProcessId,&(spi->CreateTime),dbuf1,0,0,0);
    if(*(DWORD*)dbuf1!=0)pHdr3->prcprops.UserName=AllocateAndCopy(0,(char*)dbuf1);
   }
  }
  if((spi->NextEntryOffset)==0)break;
 }
 if(!ok){//процесс-родитель не найден
  lstrcpy(sbuf,smemTable->NonExistantProcess);
  StrAppendPID(pbi.ParentProcessId,sbuf,(char*)dbuf1,&(pHdr3->prcprops.ParentProcess));
 }
skip_parent:
 if(pHdr3->prcprops.ProcessID==0)GetAndShowStartTime(pHdr3,sysTimeInf->BootTime.LowPart,sysTimeInf->BootTime.HighPart);
 if(pHdr3->prcprops.hIcon==0){
  if(pHdr3->prcprops.ProcessID==0)nm=21; else nm=4;
  pHdr3->prcprops.hIcon=LoadIcon(gInst,(char*)nm);
 }
 SendMessage(GetDlgItem(hDlg,1033),STM_SETIMAGE,IMAGE_ICON,(LPARAM)pHdr3->prcprops.hIcon);
 //// вывод свойств на экран
 if(pHdr3->prcprops.FilePath){
  TimeStampToFileTime(pHdr3->prcprops.BuildTimeStamp,pHdr3->buf);
  SetDlgItemText(hDlg,30,pHdr3->buf);
 }
 else{
  EnableWindow(GetDlgItem(hDlg,1005),0);
  EnableWindow(GetDlgItem(hDlg,3012),0);
 }
 if(pHdr3->prcprops.LastCurrentDir==0){
  EnableWindow(GetDlgItem(hDlg,3013),0);
 }
 dwp=(DWORD_PTR)&(pHdr3->prcprops.FileDescription);
 for(nm=0;nm<11;nm++){
  if(*(DWORD*)dwp)SetDlgItemText(hDlg,ControlIDsToFill[nm],(char*)(*(DWORD*)dwp));
  dwp+=4;
 }
 ////
quit:
 if(dbuf1)LocalFree(dbuf1);
 if(lmem)LocalFree(lmem);
}

LPTSTR ProcInfo_Msg_GetCaption(HWND hDlg){HWND hwnd; int len,sz; LPTSTR pszCaption;
 pszCaption=0;
 if((hwnd=GetParent(hDlg))==0)goto exit;
 len=GetWindowTextLength(hwnd);
 if(len<=0)goto exit;
 len++; sz=len*sizeof(TCHAR);
 if((pszCaption=(LPTSTR)LocalAlloc(LMEM_FIXED,sz))==0)goto exit;
 if(!GetWindowText(hwnd,pszCaption,len)){
  LocalFree(pszCaption); pszCaption=0;
 }
exit:
 return pszCaption;
}

BOOL ProcInfo_Msg_AreYouSureDestroy(HWND hDlg, HANDLE hProcess){LPTSTR pszCaption,tmpb; BOOL bCritical,bContinue; DWORD n,ln;
 bContinue=1;
 // Is it Critical Process
 bCritical=0;
 if(hProcess!=0){
  if(ntdllFunctions.pNtQueryInformationProcess(hProcess,ProcessCriticalInformation,&n,sizeof(DWORD),&ln)==0){
   bCritical=(BOOL)n;
  }
 }
 //
 if(progSetts.CheckState[2] || bCritical){
  pszCaption=ProcInfo_Msg_GetCaption(hDlg);
  if(bCritical)ln=lstrlen(smemTable->ThisIsCriticalProcess);
  ln+=lstrlen(smemTable->AreYouSureToKill);
  ln+=lstrlen(smemTable->theprocess);
  ln++; ln*=sizeof(TCHAR);
  if((tmpb=(char*)LocalAlloc(LMEM_FIXED,ln))!=0){
   if(bCritical)n=copystring(tmpb,smemTable->ThisIsCriticalProcess);
   else n=0;
   copystring(tmpb+n,smemTable->AreYouSureToKill); strappend(tmpb+n,smemTable->theprocess);
   if(MessageBox(hDlg,tmpb,pszCaption,MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON1)!=IDYES)bContinue=0;
   LocalFree(tmpb);
  }
  LocalFree(pszCaption);
 }
 return bContinue;
}

BOOL CALLBACK ProcInfo_ImageProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){DLGHDR3 *pHdr3; WORD wID;
switch(Message){
case WM_INITDIALOG:
 WindowFillText(hDlg,dlgTAT->dlg13);
 OnChildDialogInit3(hDlg);
 GetImageInfo(hDlg);
 return 0;
case WM_COMMAND:
 wID=LOWORD(wParam);
 switch(wID){
  case 1004://Kill Process
   if(!GETLONGPTR_ProcInfoWnd(GetParent(hDlg),&pHdr3))break;
   if(!ProcInfo_Msg_AreYouSureDestroy(hDlg,pHdr3->hProcess))break;
   Process_Terminate(pHdr3->prcprops.ProcessID,1);
  break;
  case 1005:
   if(!GETLONGPTR_ProcInfoWnd(GetParent(hDlg),&pHdr3))break;
   if(pHdr3->prcprops.FilePath==0)break;
   void *dbuf;
   if((dbuf=LocalAlloc(LMEM_FIXED,1024))!=0){
    File_OpenProperies(CorrectFilePath(pHdr3->prcprops.FilePath,0,dbuf));
    LocalFree(dbuf);
   }
   break;
  case 3012: case 3013:
   if(!GETLONGPTR_ProcInfoWnd(GetParent(hDlg),&pHdr3))break;
   char *tmem,*pstr; pstr=0;
   if(wID==3012)pstr=pHdr3->prcprops.FilePath;
   else if(wID==3013)pstr=pHdr3->prcprops.LastCurrentDir;
   if(pstr==0)break;
   tmem=(char*)LocalAlloc(LMEM_FIXED,getstrlen(pstr)+256);
   OpenFolderAndSelectFile(CorrectFilePath(pstr,0,tmem),hDlg);
   LocalFree(tmem);
   break;
 }
 break;
}
return 0;}

BOOL CALLBACK ProcInfo_PerfProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
switch(Message){
case WM_INITDIALOG:
 WindowFillText(hDlg,dlgTAT->dlg14);
 OnChildDialogInit3(hDlg);
 GetPerfInfo(hDlg);
 if(UpdateSleepTm!=(DWORD)-1)SetTimer(hDlg,1,UpdateSleepTm,(TIMERPROC)0);
 break;
case WM_DESTROY: KillTimer(hDlg,1); break;
case WM_TIMER:
 if(wParam==1){
  GetPerfInfo(hDlg);
 }
 break;
}
return 0;}

void WriteProcPerfHistory(HWND hwnd,UINT,UINT id,DWORD){DLGHDR3 *pHdr3; DWORD dw; DWORD_PTR dwp; HWND hpar;
 PROCESS_TIMES *ptm; SYSTEM_TIME_OF_DAY_INFORMATION *sysTimeInf; VM_COUNTERS* vmc; PROCESS_BASIC_INFORMATION pbi; 
 pHdr3=0;
 if(id!=0)pHdr3=(DLGHDR3*)id;
 if(pHdr3==0 || pHdr3->Signature!=DLGHDR3_SIGNATURE){
  pHdr3=(DLGHDR3*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
  if(pHdr3==0 || pHdr3->Signature!=DLGHDR3_SIGNATURE)return;
 }
 if(pHdr3->PerfWriteMem==0)return;
 if(pHdr3->prcprops.ProcessID==0){
  if(IsVALID_PROCESS_INFO(pHdr3->pinf) && pHdr3->pinf->spi!=0){
   pHdr3->prevTotalTm.QuadPart=pHdr3->prevKernelTm.QuadPart=pHdr3->pinf->spi->KernelTime.QuadPart;
  }
  goto GetIdleCpu;
 }
 dwp=(DWORD_PTR)(pHdr3->PerfWriteMem);
 ptm=(PROCESS_TIMES*)dwp; dwp+=sizeof(PROCESS_TIMES);
 sysTimeInf=(SYSTEM_TIME_OF_DAY_INFORMATION*)dwp; dwp+=28;
 vmc=(VM_COUNTERS*)dwp; //dwp=sizeof(PROCESS_TIMES); dwp++;
if(ntdllFunctions.pNtQueryInformationProcess(pHdr3->hProcess,ProcessTimes,ptm,sizeof(PROCESS_TIMES),&dw)==0 || IsVALID_PROCESS_INFO(pHdr3->pinf)){
 if(dw>sizeof(PROCESS_TIMES)){
  if(pHdr3->pinf->spi){
   ptm->KernelTime.QuadPart=pHdr3->pinf->spi->KernelTime.QuadPart;
   ptm->UserTime.QuadPart=pHdr3->pinf->spi->UserTime.QuadPart;
  }
  else{
   ptm->KernelTime.QuadPart=pHdr3->pinf->KernelTime.QuadPart;
   ptm->UserTime.QuadPart=pHdr3->pinf->UserTime.QuadPart;
   ptm->ExitTime.QuadPart=pHdr3->prcprops.ExitTime.QuadPart;
  }
 }
 if(ptm->ExitTime.QuadPart>0){
  KillTimer(hwnd,id);
  pHdr3->prcprops.ExitTime.QuadPart=ptm->ExitTime.QuadPart;
  if(ntdllFunctions.pNtQueryInformationProcess(pHdr3->hProcess,ProcessBasicInformation,&pbi,sizeof(pbi),&dw)==0){
   pHdr3->prcprops.ExitStatus=pbi.ExitStatus;
  }
  CloseHandle(pHdr3->hProcess);
  pHdr3->hProcess=0; pHdr3->prcprops.bExit=1;
  hpar=GetParent(pHdr3->hwndTab);
  *pHdr3->vbuf=0; int len;
  GetWindowText(hpar,(char*)pHdr3->vbuf,100);
  len=getstrlen((char*)pHdr3->vbuf);
  if(len>(int)(300-getstrlen(smemTable->ProcessIsTerminated_br)))len=0;
  copybytes((char*)pHdr3->vbuf+len," - ",300-len,1); len+=3;
  copybytes((char*)pHdr3->vbuf+len,smemTable->ProcessIsTerminated_,300-len,1);
  SetWindowText(hpar,(char*)pHdr3->vbuf);
  if(pHdr3->iSel3==0)GetImageInfo(pHdr3->hwndDisplay);
  return;
 }
 __int64 dSystemTime,newCurTm; LARGE_INTEGER lint;
 if(ntdllFunctions.pNtQuerySystemInformation(SystemTimeOfDayInformation,sysTimeInf,28,&dw)==0){//get new system time
  newCurTm=sysTimeInf->CurrentTime.QuadPart;
  dSystemTime=newCurTm; dSystemTime-=pHdr3->oldCurTm;
  pHdr3->oldCurTm=newCurTm;
 }
 lint.QuadPart=ptm->KernelTime.QuadPart; lint.QuadPart+=ptm->UserTime.QuadPart;
 double dd; bool blt; blt=0;
 if(pHdr3->prevTotalTm.QuadPart>0){
  dd=lint.QuadPart-pHdr3->prevTotalTm.QuadPart;//pHdr3->gdCpu.histCurPos
  dd/=dSystemTime; dd*=100.0;
  pHdr3->vdCpu.numData1=dd;
  *(pHdr3->gdCpu.histCurPos)=1.0-(dd*0.01);
  if((DWORD)(pHdr3->gdCpu.histCurPos)<(DWORD)(pHdr3->gdCpu.histLastAddr))pHdr3->gdCpu.histCurPos++;
  else pHdr3->gdCpu.histCurPos=(float*)pHdr3->gdCpu.histStartAddr;
  blt=1;
 }
 pHdr3->prevTotalTm.QuadPart=lint.QuadPart;
 if(progSetts.CheckState[26]){
  if(pHdr3->prevTotalTm.QuadPart>0){
   lint.QuadPart=ptm->KernelTime.QuadPart; lint.QuadPart-=pHdr3->prevKernelTm.QuadPart;
   dd=lint.QuadPart; dd/=dSystemTime; dd*=100.0;
   pHdr3->vdCpu.numData2=dd;
   *(pHdr3->gdCpuKernel.histCurPos)=1.0-(dd*0.01);
   if((DWORD_PTR)(pHdr3->gdCpuKernel.histCurPos)<(DWORD_PTR)(pHdr3->gdCpuKernel.histLastAddr))pHdr3->gdCpuKernel.histCurPos++;
   else pHdr3->gdCpuKernel.histCurPos=(float*)pHdr3->gdCpuKernel.histStartAddr;
   blt=1;
  }
  pHdr3->prevKernelTm.QuadPart=ptm->KernelTime.QuadPart;
  if(!blt)pHdr3->ogdCpu.oscGridStatic=1;
 }
}
 dw=sizeof(VM_COUNTERS); dw++;
 if(ntdllFunctions.pNtQueryInformationProcess(pHdr3->hProcess,ProcessVmCounters,vmc,sizeof(VM_COUNTERS),&dw)==0 || (IsVALID_PROCESS_INFO(pHdr3->pinf) && pHdr3->pinf->spi!=0)){
  if(dw>sizeof(VM_COUNTERS)){
   vmc=&pHdr3->pinf->spi->VmCounters;
  }
  pHdr3->vdPrivate.numData3=vmc->PagefileUsage;
  if(vmc->PeakPagefileUsage==0)pHdr3->vdPrivate.numData1=0;
  else pHdr3->vdPrivate.numData1=((float)(vmc->PagefileUsage))/vmc->PeakPagefileUsage;
  *(pHdr3->gdPrivate.histCurPos)=pHdr3->vdPrivate.numData1;
  if((DWORD)(pHdr3->gdPrivate.histCurPos)<(DWORD)(pHdr3->gdPrivate.histLastAddr))pHdr3->gdPrivate.histCurPos++;
  else pHdr3->gdPrivate.histCurPos=(float*)pHdr3->gdPrivate.histStartAddr;
 }
 goto UpdateInfo;
GetIdleCpu://для SystemIdleProcess (processID=0)
 pHdr3->vdCpu.numData1=cpuIdleTime; pHdr3->vdCpu.numData1*=100.0;
 *(pHdr3->gdCpu.histCurPos)=1.0-cpuIdleTime;
 if((DWORD)(pHdr3->gdCpu.histCurPos)<(DWORD)(pHdr3->gdCpu.histLastAddr))pHdr3->gdCpu.histCurPos++;
 else pHdr3->gdCpu.histCurPos=(float*)pHdr3->gdCpu.histStartAddr;
 //if(pHdr3->ogdCpu.oscWpls<10)pHdr3->ogdCpu.oscWpls+=2; else pHdr3->ogdCpu.oscWpls=0;
UpdateInfo:
 if(pHdr3->iSel3==2){
  Chart_VertGraph_Draw(GetDlgItem(pHdr3->hwndDisplay,1102));
  Chart_VertGraph_Draw(GetDlgItem(pHdr3->hwndDisplay,1106));
  Chart_OscilGraph_Draw(GetDlgItem(pHdr3->hwndDisplay,1104));
  Chart_OscilGraph_Draw(GetDlgItem(pHdr3->hwndDisplay,1108));
 }
}

BOOL CALLBACK ProcInfo_PerfGProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){DLGHDR3 *pHdr3;
switch(Message){
case WM_INITDIALOG:
 WindowFillText(hDlg,dlgTAT->dlg15);
 OnChildDialogInit3(hDlg);
 Chart_RegisterClass();
 if(!GETLONGPTR_ProcInfoWnd(GetParent(hDlg),&pHdr3))break;
 // OscilGraphData CPU-User
 pHdr3->gdCpu.dwFlags=CHART_FLAG_INVERSE_HG;
 pHdr3->gdCpu.hPen=CreatePen(PS_SOLID,1,0x0000ff00);
 // OscilGraphData CPU-Kernel
 pHdr3->gdCpuKernel.dwFlags=CHART_FLAG_INVERSE_HG;
 pHdr3->gdCpuKernel.hPen=CreatePen(PS_SOLID,1,0x000000ff);
 pHdr3->gdCpuKernel.NextGraphData=&(pHdr3->gdCpu);
 // OscilGraphData Memory
 pHdr3->gdPrivate.hPen=CreatePen(PS_SOLID,1,0x0000ffff);
 //
 HPEN hPenGrid; HBRUSH hBackgBrush;
 hBackgBrush=CreateSolidBrush(0);
 hPenGrid=CreatePen(PS_SOLID,1,0x00006500);
 // OscilGraph CPU
 pHdr3->ogdCpu.genInfo.BackgColor=0;
 pHdr3->ogdCpu.genInfo.hBackgBrush=hBackgBrush;
 pHdr3->ogdCpu.hPenGrid=hPenGrid;
 pHdr3->ogdCpu.oscGridStatic=0;
 pHdr3->ogdCpu.oscGridHorzMove=2;
 pHdr3->ogdCpu.FirstGraphData=&pHdr3->gdCpuKernel;
 // OscilGraph Memory
 pHdr3->ogdMem.genInfo.BackgColor=0;
 pHdr3->ogdMem.genInfo.hBackgBrush=hBackgBrush;
 pHdr3->ogdMem.hPenGrid=hPenGrid;
 pHdr3->ogdMem.oscGridStatic=0;
 pHdr3->ogdMem.oscGridHorzMove=2;
 pHdr3->ogdMem.FirstGraphData=&pHdr3->gdPrivate;
 if(pHdr3->prcprops.ProcessID==0)pHdr3->ogdMem.oscGridStatic=1;
 // VerticalGraph CPU
 pHdr3->vdCpu.genInfo.BackgColor=0;
 pHdr3->vdCpu.genInfo.hBackgBrush=hBackgBrush;
 pHdr3->vdCpu.drwColor=0x0000ff00;
 pHdr3->vdCpu.dType=GRAPHTYPE_PERCENT|GRAPHTYPE_DOUBLE;
 // VerticalGraph Memory
 pHdr3->vdPrivate.genInfo.BackgColor=0;
 pHdr3->vdPrivate.genInfo.hBackgBrush=hBackgBrush;
 if(pHdr3->prcprops.ProcessID!=0)pHdr3->vdPrivate.drwColor=0x0000ffff;
 else pHdr3->vdPrivate.drwColor=0x00006262;
 pHdr3->vdPrivate.dType=GRAPHTYPE_BYTESIZE;
 //
 HWND htmp; RECT strect;
 htmp=GetDlgItem(hDlg,1102); GetWindowRect(htmp,&strect);
 MapWindowPoints(NULL,hDlg,(LPPOINT)&strect,2);
 strect.right-=strect.left; strect.bottom-=strect.top; DestroyWindow(htmp);
 htmp=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,strect.left,strect.top,strect.right,strect.bottom,hDlg,(HMENU)1102,gInst,0);
  Chart_Initialize(htmp,&(pHdr3->vdCpu),CHART_VERTICAL);
 htmp=GetDlgItem(hDlg,1106); GetWindowRect(htmp,&strect);
 MapWindowPoints(NULL,hDlg,(LPPOINT)&strect,2);
 strect.right-=strect.left; strect.bottom-=strect.top; DestroyWindow(htmp);
 htmp=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,strect.left,strect.top,strect.right,strect.bottom,hDlg,(HMENU)1106,gInst,0);
  Chart_Initialize(htmp,&(pHdr3->vdPrivate),CHART_VERTICAL);
 htmp=GetDlgItem(hDlg,1104); GetWindowRect(htmp,&strect);
 MapWindowPoints(NULL,hDlg,(LPPOINT)&strect,2);
 strect.right-=strect.left; strect.bottom-=strect.top; DestroyWindow(htmp);
 htmp=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,strect.left,strect.top,strect.right,strect.bottom,hDlg,(HMENU)1104,gInst,0);
  Chart_Initialize(htmp,&(pHdr3->ogdCpu),CHART_DIAGRAMM);
 htmp=GetDlgItem(hDlg,1108); GetWindowRect(htmp,&strect);
 MapWindowPoints(NULL,hDlg,(LPPOINT)&strect,2);
 strect.right-=strect.left; strect.bottom-=strect.top; DestroyWindow(htmp);
 htmp=CreateWindowEx(0x200,GraphMonClass_txt,0,0x50000000,strect.left,strect.top,strect.right,strect.bottom,hDlg,(HMENU)1108,gInst,0);
  Chart_Initialize(htmp,&(pHdr3->ogdMem),CHART_DIAGRAMM);
 break;
case WM_DESTROY:
 int num; num=1102;
destroyn: DestroyWindow(GetDlgItem(hDlg,num)); num+=2; if(num<=1108)goto destroyn;
 if(GETLONGPTR_ProcInfoWnd(GetParent(hDlg),&pHdr3))break;
 // destroy chart data
 DeleteObject(pHdr3->ogdCpu.genInfo.hBackgBrush);
 DeleteObject(pHdr3->ogdCpu.hPenGrid);
 DeleteObject(pHdr3->gdCpu.hPen);
 DeleteObject(pHdr3->gdCpuKernel.hPen);
 DeleteObject(pHdr3->gdPrivate.hPen);
 break;
}
return 0;}

BOOL CALLBACK ProcInfo_SrvcsInProcess(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){DLGHDR3 *pHdr3; LV_ITEM lvi; PROCESS_SERVICES *prcsvc;
switch(Message){
case WM_INITDIALOG:
 WindowFillText(hDlg,dlgTAT->dlg2);
 OnChildDialogInit3(hDlg);
 LV_COLUMN lvcol; HWND hlist; RECT rect;
 hlist=GetDlgItem(hDlg,104);
 SetWindowPos(hlist,0,0,0,32,32,SWP_NOZORDER|SWP_NOMOVE);
 SendMessage(hlist,STM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(gInst,(char*)20,IMAGE_ICON,32,32,LR_DEFAULTCOLOR));
 hlist=GetDlgItem(hDlg,102);
 pHdr3=(DLGHDR3*)GetWindowLongPtr(GetParent(hDlg),GWLP_USERDATA);
 if(pHdr3==NULL || hlist==NULL)goto exit_init;
 HIMAGELIST himgl; int i;
 himgl=ImageList_Create(16,16,ILC_COLORDDB,6,1);
 ImageList_SetBkColor(himgl,0x00ffffff);
 for(i=4;i<10;i++)ImageList_AddIcon(himgl,LoadIcon(gInst,(char*)i));
 pHdr3->srvHlist=hlist;
 SendMessage(hlist,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT);
 ListView_SetImageList(hlist,himgl,LVSIL_SMALL);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=LVCFMT_LEFT;
 lvcol.pszText=smemTable->SrvcColumnTitle[0]; lvcol.iSubItem=0; lvcol.cx=SrvInProccessColumnWidth[0];
 SendMessage(hlist,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
 lvcol.pszText=smemTable->SrvcColumnTitle[1]; lvcol.iSubItem=1;
 if(SrvInProccessColumnWidth[1]==0){
  GetClientRect(hlist,&rect);
  rect.right-=SrvInProccessColumnWidth[0];
  SrvInProccessColumnWidth[1]=(WORD)rect.right;
  if(SrvInProccessColumnWidth[1]>30)SrvInProccessColumnWidth[1]-=(WORD)25;
 }
 lvcol.cx=SrvInProccessColumnWidth[1];
 SendMessage(hlist,LVM_INSERTCOLUMN,(WPARAM)1,(LPARAM)&lvcol);
 ///////
 SendMessage(hlist,LVM_DELETEALLITEMS,0,0);
 prcsvc=pHdr3->prcsvc; lvi.iItem=0; lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE; lvi.pszText=LPSTR_TEXTCALLBACK;
 while(prcsvc){
  lvi.iSubItem=0; lvi.lParam=(DWORD)prcsvc;
  switch(prcsvc->dwServiceType){
   case SERVICE_WIN32_OWN_PROCESS: lvi.iImage=0; break;
   case SERVICE_WIN32_SHARE_PROCESS: lvi.iImage=3; break;
   case SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS: lvi.iImage=5; break;
   case SERVICE_WIN32_SHARE_PROCESS|SERVICE_INTERACTIVE_PROCESS: lvi.iImage=4; break;
   case SERVICE_KERNEL_DRIVER: case SERVICE_ADAPTER: lvi.iImage=1; break;
   case SERVICE_FILE_SYSTEM_DRIVER: case SERVICE_RECOGNIZER_DRIVER: lvi.iImage=2; break;
   default: lvi.iImage=0; break;
  }
  SendMessage(hlist,LVM_INSERTITEM,0,(LPARAM)&lvi);
  lvi.iSubItem=1;
  SendMessage(hlist,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
  lvi.iItem++;
  if(prcsvc->NextStruct==0)goto exit_init;
  prcsvc=(PROCESS_SERVICES*)prcsvc->NextStruct;
 }
exit_init:
 SetFocus(hlist);
 break;
case WM_DESTROY:
 if((pHdr3=(DLGHDR3*)GetWindowLongPtr(GetParent(hDlg),GWLP_USERDATA))==0)break;
 DWORD wd;
 if(pHdr3->srvHlist){
  wd=SendMessage(pHdr3->srvHlist,LVM_GETCOLUMNWIDTH,0,0);
  if(wd)SrvInProccessColumnWidth[0]=(WORD)wd;
  wd=SendMessage(pHdr3->srvHlist,LVM_GETCOLUMNWIDTH,1,0);
  if(wd)SrvInProccessColumnWidth[1]=(WORD)wd;
 }
 pHdr3->srvHlist=0;
 break;
case WM_NOTIFY: DWORD dwCode;
 dwCode=((NMHDR*)lParam)->code;
 if(dwCode==NM_CLICK){
  mouse_click:
  if((pHdr3=(DLGHDR3*)GetWindowLongPtr(GetParent(hDlg),GWLP_USERDATA))==0)break;
  if(((NMHDR*)lParam)->hwndFrom!=pHdr3->srvHlist)break;
   int isel; isel=GetItemSelected(pHdr3->srvHlist);
   DWORD memsz,n,k,*vmem; char *ptxt;
   memsz=0; n=0; k=0; vmem=0;
   lvi.iSubItem=0; lvi.iItem=isel; lvi.mask=LVIF_PARAM;
   if(SendMessage(pHdr3->srvHlist,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)goto quit_descr;
   SC_HANDLE hsrv; hsrv=0; prcsvc=(PROCESS_SERVICES*)lvi.lParam;
   if(pHdr3->scman!=0)hsrv=OpenService(pHdr3->scman,prcsvc->SrvcName,SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS);
   if(hsrv==NULL){ptxt=""; goto set_descr;}
 querysc2:
   if(QueryServiceConfig2(hsrv,SERVICE_CONFIG_DESCRIPTION,(BYTE*)vmem,memsz,&n)==0){
    if(vmem)vmem=(DWORD*)LocalFree(vmem);
    if(memsz<=n && k<3){memsz=n; vmem=(DWORD*)LocalAlloc(LPTR,memsz); k++; goto querysc2;}
   }
   ptxt=(char*)*vmem;
   set_descr:
   SendMessage(GetDlgItem(hDlg,103),WM_SETTEXT,0,(LPARAM)ptxt);
 quit_descr:
   if(memsz && vmem)LocalFree(vmem);
   if(hsrv)CloseServiceHandle(hsrv);
 }
 else if(dwCode==LVN_GETDISPINFO){
  NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam;
  prcsvc=(PROCESS_SERVICES*)nvd->item.lParam;
  if(prcsvc==0)break;
  if(nvd->item.mask&LVIF_TEXT){
   int iw; iw=nvd->item.iSubItem;
   if(iw==0)nvd->item.pszText=prcsvc->SrvcName;
   else if(iw==1)nvd->item.pszText=prcsvc->DisplayName;
  }
 }
 else if(dwCode==NM_DBLCLK){
mouse_dblclick:
  if((pHdr3=(DLGHDR3*)GetWindowLong(GetParent(hDlg),GWL_USERDATA))==0)break;
  int isel; isel=GetItemSelected(pHdr3->srvHlist);
  lvi.iSubItem=0; lvi.iItem=isel; lvi.mask=LVIF_PARAM;
  if(SendMessage(pHdr3->srvHlist,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
  prcsvc=(PROCESS_SERVICES*)lvi.lParam;
  if(prcsvc->SrvcName==0)break;
  DATA_TO_SVCPROPS_THREAD *svdata;
  svdata=(DATA_TO_SVCPROPS_THREAD*)LocalAlloc(LMEM_FIXED,sizeof(DATA_TO_SVCPROPS_THREAD));
  svdata->SrvcName=prcsvc->SrvcName;
  svdata->szComputerName=0;//svwp->szComputerName;
  svdata->hMain=hDlg;
  CloseHandle(CreateRemoteThread((HANDLE)0xFFFFFFFF,0,0,(LPTHREAD_START_ROUTINE)SrvcPropsThread,(void*)svdata,0,&n));
 }
 else if(dwCode==LVN_KEYDOWN){
  if(((LPNMLVKEYDOWN)lParam)->wVKey==VK_SPACE || ((LPNMLVKEYDOWN)lParam)->wVKey==VK_RETURN)goto mouse_dblclick;
 }
 else if(dwCode==LVN_ITEMCHANGED){
  NMLISTVIEW *nmlv; nmlv=(LPNMLISTVIEW)lParam; if(nmlv==0)break;
  if(nmlv->uChanged&LVIF_STATE && nmlv->uNewState&LVIS_SELECTED)goto mouse_click;
 }
 break;
}
return 0;}

#ifdef ENABLE_JOB_INFO
HANDLE hJob;

void GetJobInfo(HWND hDlg){DWORD ret;
LSA_OBJECT_ATTRIBUTES oat;
RtlZeroMemory(&oat,sizeof(LSA_OBJECT_ATTRIBUTES));
oat.Length=sizeof(LSA_OBJECT_ATTRIBUTES);
oat.Attributes=OBJ_CASE_INSENSITIVE;
//oat.ObjectName=&gNamedPipeW;
ret=ntdllFunctions.NtCreateJobObject(&hJob,JOB_OBJECT_QUERY,&oat);
}

BOOL CALLBACK ProcInfo_Job(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
switch(Message){
case WM_INITDIALOG:
 //WindowFillText(hDlg,dlgTAT->dlg14);
 OnChildDialogInit3(hDlg);
 GetJobInfo(hDlg);
 //if(UpdateSleepTm!=(DWORD)-1)SetTimer(hDlg,1,UpdateSleepTm,(TIMERPROC)0);
 break;
}
return 0;}
#endif

void OnInit_ProcMain(HWND hDlg,LPARAM lParam){DLGHDR3 *pHdr3; DWORD cyMargin,dwDlgBase,cx,cy,dwTable; TCITEM tci; LOGFONT tabf; HWND hB;
 PROCESS_INFO *pinf;
 pHdr3=(DLGHDR3*)lParam;
 if(!IsVALID_ProcInfoWnd(pHdr3))return;
 SetWindowLongPtr(hDlg,GWLP_USERDATA,(DWORD_PTR)pHdr3);
 dwDlgBase=GetDialogBaseUnits();
 cyMargin=HIWORD(dwDlgBase)/8;
 pHdr3->hwndTab=CreateWindow(WC_TABCONTROL,0,WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|WS_TABSTOP,0,0,50,80,hDlg,0,gInst,NULL);
 tci.mask=TCIF_TEXT;
 tci.pszText=smemTable->Image;
 SendMessage(pHdr3->hwndTab,TCM_INSERTITEM,0,(LPARAM)&tci);
 tci.pszText=smemTable->Performance;
 SendMessage(pHdr3->hwndTab,TCM_INSERTITEM,1,(LPARAM)&tci);
 tci.pszText=smemTable->PerfGraph;
 SendMessage(pHdr3->hwndTab,TCM_INSERTITEM,2,(LPARAM)&tci);
 pHdr3->prcprops.hIcon=0;
 pHdr3->apRes[0]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(LPTSTR)13,RT_DIALOG)));
 pHdr3->apRes[1]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(LPTSTR)14,RT_DIALOG)));
 pHdr3->apRes[2]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(LPTSTR)15,RT_DIALOG)));
 pHdr3->dlgProC[0]=(DLGPROC*)ProcInfo_ImageProc;
 pHdr3->dlgProC[1]=(DLGPROC*)ProcInfo_PerfProc;
 pHdr3->dlgProC[2]=(DLGPROC*)ProcInfo_PerfGProc;
 dwTable=2;
 GetClientRect(hDlg,&pHdr3->rcDisplay);
 pHdr3->rcDisplay.left=0; pHdr3->rcDisplay.right-=8;
 pHdr3->rcDisplay.bottom-=cyMargin; pHdr3->rcDisplay.bottom-=30;
 SetWindowPos(pHdr3->hwndTab,NULL,4,2,pHdr3->rcDisplay.right-pHdr3->rcDisplay.left,pHdr3->rcDisplay.bottom-pHdr3->rcDisplay.top,SWP_NOZORDER);
 tabf.lfCharSet=RUSSIAN_CHARSET; tabf.lfHeight=8; tabf.lfWidth=0; tabf.lfWeight=100; tabf.lfOutPrecision=OUT_DEFAULT_PRECIS;
 lstrcpy(tabf.lfFaceName,MSsansSerif_txt); tabf.lfEscapement=0; tabf.lfOrientation=0; tabf.lfUnderline=0; tabf.lfStrikeOut=0; tabf.lfItalic=0;
 pHdr3->dlgFont3=CreateFontIndirect(&tabf);
 cx=pHdr3->rcDisplay.right; cx-=75;
 cy=pHdr3->rcDisplay.bottom+5;
 hB=CreateWindowEx(WS_EX_TOPMOST,BUTTON_txt,smemTable->Close,BS_PUSHBUTTON|BS_CENTER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,cx,cy,75,23,hDlg,(HMENU)IDCANCEL,gInst,NULL);
 SendMessage(hB,WM_SETFONT,(WPARAM)pHdr3->dlgFont3,(LPARAM)TRUE);
 pHdr3->rcDisplay.bottom-=tabtop;
 SendMessage(pHdr3->hwndTab,WM_SETFONT,(WPARAM)pHdr3->dlgFont3,(LPARAM)TRUE);
 pinf=fpinfo;
 while(IsVALID_PROCESS_INFO(pinf)){
  if(pinf->Id==pHdr3->prcprops.ProcessID && pinf->dwStatus<2){pHdr3->pinf=pinf; pinf->phdr3=pHdr3; break;}
  pinf=pinf->NextEntry;
 }
 pinf=pHdr3->pinf;
 if((pHdr3->hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pHdr3->prcprops.ProcessID))==0){
  cx=ntdllFunctions.RtlGetLastWin32Error();
  if(pHdr3->prcprops.ProcessID!=0){
   if(cx==ERROR_INVALID_PARAMETER){
    MessageBox(hDlg,smemTable->InvalidProcessId,ERROR_txt,MB_OK|MB_SETFOREGROUND|MB_ICONSTOP);
   }
   else if(pinf)pHdr3->prcprops.UserName=LocalAllocAndCopy(pinf->UserName);
  }
 }
 pHdr3->gdCpu.histStartAddr=(DWORD_PTR)HeapAlloc(pHdr3->hHeap,0,2052);
 pHdr3->gdCpu.histCurPos=(float*)pHdr3->gdCpu.histStartAddr;
 pHdr3->gdCpu.histLastAddr=(float*)(pHdr3->gdCpu.histStartAddr+2052);
 pHdr3->ogdCpu.FirstGraphData=&pHdr3->gdCpu;// можно сделать выбор что на первый план - kernel или total
 if(pHdr3->prcprops.ProcessID!=0){
  if(progSetts.CheckState[26]){
   pHdr3->gdCpuKernel.histStartAddr=(DWORD_PTR)HeapAlloc(pHdr3->hHeap,0,2052);
   pHdr3->gdCpuKernel.histCurPos=(float*)pHdr3->gdCpuKernel.histStartAddr;
   pHdr3->gdCpuKernel.histLastAddr=(float*)(pHdr3->gdCpuKernel.histStartAddr+2052);
   pHdr3->ogdCpu.FirstGraphData=&pHdr3->gdCpuKernel;
   FillMemory((void*)pHdr3->gdCpuKernel.histStartAddr,2052,0x03);
  }
  ///
  pHdr3->gdPrivate.histStartAddr=(DWORD_PTR)HeapAlloc(pHdr3->hHeap,0,2052);
  pHdr3->gdPrivate.histLastAddr=(float*)(pHdr3->gdPrivate.histStartAddr+2052);
  pHdr3->gdPrivate.histCurPos=(float*)pHdr3->gdPrivate.histStartAddr;
  FillMemory((void*)pHdr3->gdPrivate.histStartAddr,2052,0x03);
  char *pbuf; pbuf=(char*)(pHdr3->vbuf); *pbuf=0;
  pinf=pHdr3->pinf;
  if(pinf && pinf->ProcessName.Buffer!=0){
   if(pinf->Id!=0)cx=pinf->ProcessName.Length;
   else cx=getstrlenW(pinf->ProcessName.Buffer);
   UnicodeStringToAnsiString(pinf->ProcessName.Buffer,cx,pbuf,265);
   pbuf+=getstrlen(pbuf);
  }
  copystring(pbuf," ("); pbuf+=2;
  ltoa(pHdr3->prcprops.ProcessID,pbuf,10);
  pbuf+=getstrlen(pbuf);
  copybytes(pbuf,smemTable->_Properties,20,1);
 }
else {
 pHdr3->gdPrivate.histStartAddr=0;
 pHdr3->gdCpuKernel.histStartAddr=0;
 copystring((char*)(pHdr3->vbuf),smemTable->SysIdleProc);
 strappend((char*)(pHdr3->vbuf)," (0");
 strappend((char*)(pHdr3->vbuf),smemTable->_Properties);
}
SetWindowText(hDlg,(char*)(pHdr3->vbuf));
FillMemory((void*)pHdr3->gdCpu.histStartAddr,2052,0x03);
pHdr3->prevTotalTm.LowPart=0; pHdr3->prevTotalTm.HighPart=0;
pHdr3->PerfWriteMem=LocalAlloc(LPTR,28+sizeof(PROCESS_TIMES)+sizeof(VM_COUNTERS));
if(UpdateSleepTm!=(DWORD)-1)SetTimer(hDlg,(DWORD)pHdr3,UpdateSleepTm,(TIMERPROC)WriteProcPerfHistory);
////// does it have services ?
 void *hVMem;
 if((pHdr3->scman=OpenSCManager(0,0,SC_MANAGER_ENUMERATE_SERVICE))==0)goto skip_srvcs_full;//szComputerName надо брать из user_data: COMPUTER\\USER - НЕТ !!! Тормозит ЖУТКО !!!
 hVMem=VirtualAllocEx((HANDLE)-1,0,0x10000,MEM_COMMIT,PAGE_READWRITE);
 if(hVMem==NULL)goto skip_srvcs;
 ///////
 ENUM_SERVICE_STATUS_PROCESS *essp; BYTE RepeatEnum; PROCESS_SERVICES *prcsvc;
 DWORD svCount,stfrom,hMem_end,n; void *vd;
 essp=(ENUM_SERVICE_STATUS_PROCESS*)hVMem;
 svCount=0; stfrom=0; prcsvc=0;
 hMem_end=(DWORD)essp; hMem_end+=0x10000;
serviceEnumeration:
 if((EnumServicesStatusEx(pHdr3->scman,SC_ENUM_PROCESS_INFO,SERVICE_WIN32|SERVICE_INTERACTIVE_PROCESS,SERVICE_ACTIVE,(BYTE*)essp,0x10000,&n,&svCount,&stfrom,0))==0){
  if(ntdllFunctions.RtlGetLastWin32Error()==ERROR_MORE_DATA)RepeatEnum=1;
  else {FailMessage(EnumServicesStatus_txt,0,FMSG_CallGLE|FMSG_WRITE_LOG); goto skip_srvcs;}
 }
 else RepeatEnum=0;
 for(;svCount>0 && (DWORD)essp<hMem_end;svCount--){
  if(essp->ServiceStatusProcess.dwProcessId!=pHdr3->prcprops.ProcessID)goto goNext;
  vd=HeapAlloc(pHdr3->hHeap,0,sizeof(PROCESS_SERVICES));
  if(vd==0)goto skip_srvcs;
  if(prcsvc)prcsvc->NextStruct=(PROCESS_SERVICES*)vd;
  else if(pHdr3->prcsvc==0)pHdr3->prcsvc=(PROCESS_SERVICES*)vd;
  prcsvc=(PROCESS_SERVICES*)vd;
  prcsvc->SrvcName=AllocateAndCopy(pHdr3->hHeap,essp->lpServiceName);
  prcsvc->DisplayName=AllocateAndCopy(pHdr3->hHeap,essp->lpDisplayName);
  prcsvc->dwServiceType=essp->ServiceStatusProcess.dwServiceType;
  prcsvc->NextStruct=0;
  goNext:
  essp++;
 }
 if(RepeatEnum)goto serviceEnumeration;
 skip_srvcs:
 if(hVMem)VirtualFreeEx((HANDLE)-1,hVMem,0,MEM_RELEASE);
 skip_srvcs_full:
/////////////
if(pHdr3->prcsvc!=0){
 dwTable++;
 pHdr3->apRes[dwTable]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)2,RT_DIALOG)));
 pHdr3->dlgProC[dwTable]=(DLGPROC*)ProcInfo_SrvcsInProcess;
 tci.pszText=smemTable->Services;
 SendMessage(pHdr3->hwndTab,TCM_INSERTITEM,3,(LPARAM)&tci);
}
#ifdef ENABLE_JOB_INFO
if(asmIsProcessInJob(pHdr3->hProcess,0,(DWORD)(ntdllFunctions.NtIsProcessInJob))){
 dwTable++;
 pHdr3->apRes[dwTable]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)26,RT_DIALOG)));
 pHdr3->dlgProC[dwTable]=(DLGPROC*)ProcInfo_Job;
 tci.pszText=smemTable->Job;
 SendMessage(pHdr3->hwndTab,TCM_INSERTITEM,4,(LPARAM)&tci);
}
#endif
SendMessage(pHdr3->hwndTab,TCM_SETCURSEL,0,0);
OnTabSelChanged3(hDlg);
}

BOOL CALLBACK ProcInfoMain_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
DLGHDR3 *pHdr3;
switch(Message){
case WM_INITDIALOG:
 OnInit_ProcMain(hDlg,lParam);
 SetWinPos(hDlg,1);
 return 0;
case WM_COMMAND:
 switch(LOWORD(wParam)){
  case 50004: case IDCANCEL: goto closeWin;
  case 50008: OpenHelpTopic(0,0); break;
  case 50018:
   int cnt,nk;
   if(!GETLONGPTR_ProcInfoWnd(GetParent(hDlg),&pHdr3))break;
   cnt=SendMessage(pHdr3->hwndTab,TCM_GETITEMCOUNT,0,0);
   if(cnt==0)break; cnt--;
   nk=SendMessage(pHdr3->hwndTab,TCM_GETCURSEL,0,0);
   if(nk==-1)break;
   if(nk<cnt)nk++; else nk=0;
   SendMessage(pHdr3->hwndTab,TCM_SETCURSEL,nk,0);
   OnTabSelChanged3(hDlg);
   break;
 }
 break;
case WM_CLOSE: closeWin: DestroyWindow(hDlg); break;
case WM_DESTROY:
 if(GETLONGPTR_ProcInfoWnd(hDlg,&pHdr3)){
  INVALIDATE_ProcInfoWnd(pHdr3);
  if(pHdr3->hProcess!=0)CloseHandle(pHdr3->hProcess);
  KillTimer(hDlg,(DWORD)pHdr3); if(pHdr3->prcprops.hIcon!=NULL)DestroyIcon(pHdr3->prcprops.hIcon);
  DeleteObject(pHdr3->dlgFont3);
  HeapFree(pHdr3->hHeap,0,(void*)(pHdr3->gdCpu.histStartAddr));
  HeapFree(pHdr3->hHeap,0,(void*)(pHdr3->gdPrivate.histStartAddr));
  if(pHdr3->scman!=0)CloseServiceHandle(pHdr3->scman);
  if(pHdr3->PerfWriteMem!=0)LocalFree(pHdr3->PerfWriteMem);
  PROCESS_SERVICES *prcsvc,*p2; prcsvc=pHdr3->prcsvc;
  while(prcsvc!=0){
   if(prcsvc->SrvcName)HeapFree(pHdr3->hHeap,0,prcsvc->SrvcName);
   if(prcsvc->DisplayName)HeapFree(pHdr3->hHeap,0,prcsvc->DisplayName);
   p2=(PROCESS_SERVICES*)prcsvc->NextStruct;
   HeapFree(pHdr3->hHeap,0,(void*)prcsvc);
   prcsvc=p2;
  }
 }
 if(IsWindow(pHdr3->hwndDisplay))DestroyWindow(pHdr3->hwndDisplay);
 PostQuitMessage(0);
 break;
case WM_NOTIFY: if(((LPNMHDR)lParam)->code==TCN_SELCHANGE)OnTabSelChanged3(hDlg); break;
} return 0;}

DWORD ProcInfoThread(LPVOID lpv){MSG msg; HACCEL haccel2; HWND prcinf_win; DLGHDR3 *pHdr3; HANDLE hHeap;
 if((hHeap=HeapCreate(0,4096,0xFFFFFF))==NULL){FailMessage(HeapCreate_txt,0,FMSG_CallGLE|FMSG_WRITE_LOG); goto quit;}
 pHdr3=(DLGHDR3*)HeapAlloc(hHeap,HEAP_ZERO_MEMORY,sizeof(DLGHDR3));
 VALIDATE_ProcInfoWnd(pHdr3);
 pHdr3->hHeap=hHeap;
 pHdr3->prcprops.ProcessID=(DWORD)lpv;
 if((prcinf_win=CreateDialogParam(gInst,(LPTSTR)12,main_win,(DLGPROC)ProcInfoMain_Proc,(LPARAM)pHdr3))==0)goto exit;
 ShowWindow(prcinf_win,SW_SHOW);
 AddHwndToWinChain(prcinf_win);
 haccel2=LoadAccelerators(gInst,(LPTSTR)2);
 while(GetMessage(&msg,NULL,0,0)){
  if(haccel2!=0 && TranslateAccelerator(prcinf_win,haccel2,&msg))continue;
  if(msg.message==WM_KEYDOWN && msg.wParam==VK_RETURN && msg.hwnd==pHdr3->srvHlist)goto trans_dispatch;
  if(!IsDialogMessage(prcinf_win,&msg)){
 trans_dispatch:
   TranslateMessage(&msg);DispatchMessage(&msg);
  }
 }
exit:
 if(pHdr3->hProcess!=0/* && pHdr3->pinf!=0*/){
  DWORD_PTR free_from,free_last,pinf_like;
  free_from=(DWORD_PTR)&(pHdr3->prcprops.FileDescription);
  free_last=(DWORD_PTR)&(pHdr3->prcprops.StartTime);
  pinf_like=(DWORD_PTR)&(pHdr3->pinf->Description);
  for(;free_from<=free_last;free_from+=4){
   if(*(DWORD_PTR*)free_from && *(DWORD_PTR*)free_from!=*(DWORD_PTR*)pinf_like)LocalFree((void*)(*(DWORD_PTR*)free_from));
   pinf_like+=sizeof(DWORD_PTR);
  }
 }
 if(pHdr3){
  PROCESS_INFO *pinf; DWORD ln; DWORD_PTR dwp1,dwp2;
  pinf=pHdr3->pinf;
  if(IsVALID_PROCESS_INFO(pinf)){
   pinf->phdr3=0;
   INVALIDATE_ProcInfoWnd(pHdr3);
   if(pinf->dwStatus>=2 && pinf->spi==0){//не зависнет если только progSetts.DataDeleteDelay=0
    dwp1=(DWORD_PTR)&(pinf->UserName);
    dwp2=(DWORD_PTR)&(pinf->PriorityText);
    for(;dwp1<=dwp2;dwp1+=sizeof(DWORD_PTR)){
     if(*(DWORD_PTR*)dwp1){
      ln=*(DWORD_PTR*)dwp1>*(DWORD_PTR*)smemTable;
      ln+=*(DWORD_PTR*)dwp1<(DWORD_PTR)Menus[0];
      if(ln!=2)LocalFree((void*)(*(DWORD_PTR*)dwp1));//+ PROCESS_THREAD_INFO *Threads;
     }
    }
    if(pinf->ImagePathName.Buffer)LocalFree(pinf->ImagePathName.Buffer);
    LocalFree(pinf);
   }
  }
  HeapFree(hHeap,0,(void*)pHdr3);
 }
 if(hHeap)HeapDestroy(hHeap);
 LeaveWindowFocus(prcinf_win);
quit:
 ExitThread(0);
 return 0;
}
