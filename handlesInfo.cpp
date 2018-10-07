typedef struct _THParam{HANDLE hFile; char* pStr; ULONG retn;} THParam;

typedef struct _struct_obj1{
 HWND hWin;
 HWND hLVlist;
}STRUCT_OBJ1;

bool CloseObjHandle(HWND hDlg,HANDLE HandleValue,DWORD Flags,DWORD ProcessId){DWORD errcd;
 if(MessageBox(hDlg,smemTable->HndlClosLeadCrash,progTitle,MB_YESNO|MB_ICONQUESTION)!=IDYES)return 0;
 if(Flags&HANDLEFLAG_PROTECT_FROM_CLOSE){
  MessageBox(hDlg,smemTable->HndlIsPrtctClose,progTitle,MB_OK|MB_ICONSTOP);
  return 0;
 }
 HANDLE hRemoteProcess,handle; hRemoteProcess=0;
 if(ProcessId!=cur_pid){
  if((hRemoteProcess=OpenProcess(PROCESS_DUP_HANDLE,0,ProcessId))==0){
   FailMessage(smemTable->_UnblOpenProcess_,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
   return 0;
  }
  DuplicateHandle(hRemoteProcess,HandleValue,(HANDLE)-1,&handle,0,FALSE,DUPLICATE_CLOSE_SOURCE);//DUPLICATE_SAME_ACCESS
 }
 else handle=HandleValue;
 errcd=ntdllFunctions.NtClose(handle);
 if(ProcessId!=cur_pid && hRemoteProcess!=0)ntdllFunctions.NtClose(hRemoteProcess);
 if(errcd){FailMessage(smemTable->UnblCloseHndl,errcd,FMSG_NTSTATUS_CONVERT|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX); return 0;}
 else return 1;
}

/////////////////////////////// TypesListWin_Proc //////////////////////////////
BOOL CALLBACK TypesListWin_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
switch(Message){
case WM_INITDIALOG:
 HWND typlist; RECT rect; LV_COLUMN lvcol;
 SetWindowPos(hDlg,0,0,0,330,400,SWP_NOZORDER|SWP_NOMOVE);
 GetClientRect(hDlg,&rect);
 typlist=CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,0,LVS_SINGLESEL|LVS_SHOWSELALWAYS|LVS_REPORT|WS_CHILD|WS_VISIBLE,0,0,rect.right,rect.bottom,hDlg,(HMENU)101,gInst,NULL);
 if(typlist==0)break;
 SetWindowInCenter(hDlg,&rect,0);
 SetFocus(typlist);
 HIMAGELIST hlstImg;
 hlstImg=ImageList_LoadImage(gInst,(char*)7,16,0,0x00FF00FF,IMAGE_BITMAP,LR_CREATEDIBSECTION);
 SendMessage(typlist,LVM_SETIMAGELIST,LVSIL_SMALL,(LPARAM)hlstImg);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=LVCFMT_LEFT;
 lvcol.cx=220; lvcol.pszText="Name & Icon"; lvcol.iSubItem=0;
 SendMessage(typlist,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
 lvcol.cx=60; lvcol.pszText="Type ID"; lvcol.iSubItem=1;
 SendMessage(typlist,LVM_INSERTCOLUMN,(WPARAM)1,(LPARAM)&lvcol);
 SendMessage(typlist,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)LVS_EX_FULLROWSELECT);
 SetWindowText(hDlg,"Windows Objects: List of Types");
 OBJTYPES_NAMES_TABLE *otnt; DWORD dnum,imcnt; imcnt=ImageList_GetImageCount(hlstImg);
 if(ObjTypesNmTable==0)break;
 dnum=*(DWORD*)ObjTypesNmTable;
 otnt=(OBJTYPES_NAMES_TABLE*)(((DWORD*)ObjTypesNmTable)+2);
 STRING strng; UNICODE_STRING ustr; char *buf1; LV_ITEM lvi; DWORD dw;
 buf1=(char*)LocalAlloc(LPTR,256);
 strng.Buffer=buf1; strng.MaximumLength=256; strng.Length=0;
 ustr.MaximumLength=0;
 lvi.iItem=0;
 while(dnum){
  lvi.iSubItem=0; lvi.pszText=buf1; lvi.mask=LVIF_TEXT|LVIF_IMAGE;
  if(otnt->TypeName){
   dw=(DWORD)ObjTypesNmTable; dw+=(DWORD)(otnt->TypeName);
   ustr.Buffer=(LPWSTR)dw; ustr.Length=(WORD)(otnt->NameLength);
   ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,&ustr,0);
   *(strng.Buffer+strng.Length)=0;
  }
  else *buf1=0;
  if(imcnt>otnt->MyId)lvi.iImage=otnt->MyId;
  else lvi.iImage=0;
  SendMessage(typlist,LVM_INSERTITEM,0,(LPARAM)&lvi);
  ltoa(otnt->Typeid,buf1,10);
  lvi.iSubItem=1; lvi.mask=LVIF_TEXT;
  SendMessage(typlist,LVM_SETITEM,0,(LPARAM)&lvi);
  dnum--; otnt++; lvi.iItem++;
 }
 if(buf1)LocalFree(buf1);
 break;
case WM_CLOSE: DestroyWindow(hDlg); break;
} return 0;}

////////////////////////// Fill ObjectTypes Table //////////////////////////////
void FillObjectTypesTable(BYTE bCmd){DWORD n,nn,m,vmsz,rpts,dw,locsz; WORD wi; BYTE Stage; char *pstr,*buf1; int in;
OBJECT_ALL_TYPES_INFO *oati; OBJECT_TYPE_INFORMATION *oti; OBJTYPES_NAMES_TABLE *otnt; STRING strng;
if(ObjTypesNmTable && LocalFree(ObjTypesNmTable))ObjTypesNmTable=0;
oati=0; vmsz=0; rpts=0;
queryAgain: n=ntdllFunctions.pNtQueryObject(0,ObjectAllTypesInfo,oati,vmsz,&nn);
if(n==0xC0000004 && vmsz<nn){
 if(oati && VirtualFreeEx((HANDLE)0xFFFFFFFF,oati,0,MEM_RELEASE))oati=0;
 if(rpts<3){
  vmsz=nn; vmsz+=512;
  oati=(OBJECT_ALL_TYPES_INFO*)VirtualAllocEx((HANDLE)0xFFFFFFFF,0,vmsz,MEM_COMMIT,PAGE_READWRITE);
  rpts++;
  goto queryAgain;
 }
}
if(n!=0){FailMessage("FOTT: NTQO_OATI",n,FMSG_NTSTATUS_CONVERT|FMSG_WRITE_LOG); goto exit;}
////// Fill Types Table
locsz=0; Stage=1; otnt=0; pstr=0;
buf1=(char*)LocalAlloc(LPTR,256); strng.Buffer=buf1; strng.Length=0; strng.MaximumLength=256;
 walk_types:
in=oati->NumberOfObjectTypes;
nn=(DWORD)oati; nn+=4; wi=1;
while(in){
 nn+=3; nn>>=2; nn<<=2;
 oti=(OBJECT_TYPE_INFORMATION*)nn;
 if(Stage==1){
  locsz+=oti->ObjectTypeName.Length; locsz+=2;
 }
 else if(Stage==2){
  otnt->NameLength=oti->ObjectTypeName.Length;
  if(otnt->NameLength>0){
   otnt->TypeName=(LPWSTR)pstr;
   pstr+=copybytes(pstr,(char*)(oti->ObjectTypeName.Buffer),otnt->NameLength,0);
   pstr+=2;
  }
  otnt->Typeid=wi;
  ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,&(oti->ObjectTypeName),0);
  *(strng.Buffer+strng.Length)=0;
  for(WORD wd=0;wd<=33;wd++){if(*(ObjTypes[wd])==*buf1 && lstrcmpi(ObjTypes[wd],buf1)==0){if(wd==32 || wd==33)otnt->MyId=17; else otnt->MyId=wd; break;}}
  otnt++;
 }
 nn=(DWORD)(oti->ObjectTypeName.Buffer); nn+=oti->ObjectTypeName.Length; nn+=2;
 in--; wi++;
}
////
if(Stage==1 && oati->NumberOfObjectTypes && locsz){
 Stage++; nn=0;
 nn+=sizeof(OBJTYPES_NAMES_TABLE)*(oati->NumberOfObjectTypes);//OBJTYPES_NAMES_TABLE collection
 locsz+=8;//Header: DWORD Number of items; DWORD sizeofFullStruct
 locsz+=nn;
 n=(DWORD)LocalAlloc(LPTR,locsz); ObjTypesNmTable=(OBJTYPES_NAMES_TABLE*)n;
 if(n==0){FailMessage(smemTable->UnblAllocMem,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG); goto exit;}
 *(DWORD*)n=oati->NumberOfObjectTypes; n+=4;
 *(DWORD*)n=locsz; n+=4;
 nn+=n;
 otnt=(OBJTYPES_NAMES_TABLE*)n;
 objTypesNames=(LPWSTR)nn; pstr=(char*)objTypesNames;
 goto walk_types;
}
#ifdef _DEBUG
otnt--;
in=(DWORD)(otnt->TypeName); in+=otnt->NameLength;
in-=(DWORD)ObjTypesNmTable;
if(in>(int)locsz){
 Beep(800,100); Beep(1000,200);
}
#endif
///// Replace Absolute Buffer Address with Relative
n=(DWORD)ObjTypesNmTable; m=*(DWORD*)n; n+=8;
otnt=(OBJTYPES_NAMES_TABLE*)n;
for(dw=0;dw<m;dw++){
 if(otnt->TypeName){
  otnt->TypeName=(LPWSTR)((DWORD)(otnt->TypeName)-(DWORD)ObjTypesNmTable);
 }
 otnt++;
}
//// Fill ListView
if((bCmd&2)==0)goto exit;
HWND hTypesListWin;
hTypesListWin=CreateDialog(gInst,(char*)12,main_win,(DLGPROC)TypesListWin_Proc);
if(hTypesListWin)ShowWindow(hTypesListWin,SW_SHOW);
exit:
if(oati)VirtualFreeEx((HANDLE)0xFFFFFFFF,oati,0,MEM_RELEASE);
if(buf1)LocalFree(buf1);
}

DWORD GetFileNameThread(PVOID pParam){THParam *tp=(THParam*)pParam; UCHAR lpBuffer[1000];
 IO_STATUS_BLOCK iob; //FILE_NAME_INFORMATION fni;
 tp->retn=ntdllFunctions.pNtQueryInformationFile(tp->hFile,&iob,&lpBuffer,1000,FileNameInformation);
 /*if(tp->retn==0){
  if(((FILE_NAME_INFORMATION*)lpBuffer)->FileNameLength){
   //copybytes((char*)tp->pStr,(char*)lpBuffer,((FILE_NAME_INFORMATION*)&lpBuffer)->FileNameLength+4,0);
  }
 }*/
 return 0;
}

DWORD GetFileName(HANDLE handle, char* str){DWORD n; HANDLE hThread; THParam tp;//Return: 0-OK, 1-Failed, 2-ThreadNotRespond
tp.hFile=handle; tp.pStr=str; tp.retn=1;
if((hThread=CreateRemoteThread((HANDLE)0xFFFFFFFF,0,0,(LPTHREAD_START_ROUTINE)GetFileNameThread,&tp,0,&n))!=0){
 if(WaitForSingleObject(hThread,100)==WAIT_TIMEOUT){//Wait for finishing the thread & terminate it if access is denied
  TerminateThread(hThread,0); copystring(str,smemTable->access_denied);
  tp.retn=(DWORD)-1;
 }
 CloseHandle(hThread);
}
return tp.retn;
}

bool GetTypeToken(HANDLE handle, char* str, WORD *type){DWORD tn,nn; //2048
STRING strng; strng.Length=0; strng.MaximumLength=120; strng.Buffer=str;
if(ntdllFunctions.pNtQueryObject(handle,ObjectTypeInfo,nbuf5,2048,&tn)!=0){
 if(ObjTypesNmTable==0){
  FillObjectTypesTable(0);
  if(ObjTypesNmTable==0)goto err_exit;
 }
 n=(DWORD)ObjTypesNmTable; nn=*(DWORD*)n; n+=8;
 OBJTYPES_NAMES_TABLE *otnt;
 otnt=(OBJTYPES_NAMES_TABLE*)n;
 for(n=0;n<nn;n++){
  if((BYTE)(otnt->Typeid)==*type){
   UNICODE_STRING ustr;
   n=(DWORD)ObjTypesNmTable; n+=(DWORD)otnt->TypeName;
   ustr.Buffer=(LPWSTR)n; ustr.Length=(WORD)(otnt->NameLength);
   *type=(WORD)(otnt->MyId);
   ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,&ustr,0);
   return 1;
  }
  otnt++;
 }
 err_exit:
 copystring((char*)t1nbuf4,smemTable->access_denied); *str=0;
 return 0;
}
ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,(UNICODE_STRING*)nbuf5,0);
*(strng.Buffer+strng.Length)=0;
for(WORD wi=0;wi<=33;wi++){if(*(ObjTypes[wi])==*str && lstrcmpi(ObjTypes[wi],str)==0){if(wi==32 || wi==33)*type=17; else *type=wi; return 1;}}
return 0;}

HANDLE_INFOBUF *GetNameType(SYSTEM_HANDLE *shn,int lItem){HANDLE hObj,handle,hRemoteProcess; WORD type; DWORD rType,tn,*err; BOOL remotehfail;
 LV_ITEM lvi; char lbuf[15]; HANDLE_INFOBUF *hif; STRING strng;
 hObj=(HANDLE)(shn->HandleValue);
 type=shn->HandleTypeNumber;
 rType=0; remotehfail=0; handle=0; hRemoteProcess=0; hif=0;
 if(shn->ProcessID!=cur_pid){
  hRemoteProcess=OpenProcess(PROCESS_DUP_HANDLE,0,shn->ProcessID);
  if(hRemoteProcess==NULL){handle=0; remotehfail=1;}
  else {
   DuplicateHandle(hRemoteProcess,hObj,(HANDLE)-1,&handle,0,FALSE,DUPLICATE_SAME_ACCESS);
   //ntdllFunctions.NtDuplicateObject(hRemoteProcess,hObj,(HANDLE)-1,&handle,0,0,DUPLICATE_SAME_ACCESS);
  }
 }
 else handle=hObj;
 *t1nbuf4=0;
 if(!GetTypeToken(handle,tbuf8,&type)){rType=type; type=0; *tbuf8=0;}
 err=(DWORD*)t1nbuf4; err++;
 switch(type){
 case 0:
  char *pstr;
  if(*(DWORD*)t1nbuf4==*(DWORD*)(smemTable->access_denied)){
   pstr=(char*)tbuf8; pstr+=copystring(pstr,"0x");
   HexToString(rType,pstr);
  }
  else{
   pstr=(char*)t1nbuf4; pstr+=copystring(pstr,"-= type id=0x");
   pstr+=getstrlen(pstr); HexToString(rType,pstr); strappend(pstr," =-");
  }
  goto skipQuery;
 case 18://Process
  PROCESS_BASIC_INFORMATION pi;
  if((*err=ntdllFunctions.pNtQueryInformationProcess(handle,ProcessBasicInformation,&pi,sizeof(pi),0))==0){
   char *pstr;
   pstr=(char*)t1nbuf4; lstrcpy(pstr,"PID: 0x");
   pstr+=7; HexToString(pi.UniqueProcessId,pstr);
   pstr+=8; *(DWORD*)pstr=0x00002820; pstr+=2;
   ltoa(pi.UniqueProcessId,pstr,10);
   while(*(++pstr)); *(WORD*)pstr=0x0029;
   goto skipQuery;
  }
  *(DWORD*)t1nbuf4=0x03020100;
  goto skipQuery;
 case 23://Thread
  THREAD_BASIC_INFORMATION ti;
  if((*err=ntdllFunctions.pNtQueryInformationThread(handle,ThreadBasicInformation,&ti,sizeof(ti),NULL))==0){
   char *pstr;
   pstr=(char*)t1nbuf4; lstrcpy((char*)t1nbuf4,"TID: 0x");
   pstr+=7; HexToString(ti.ClientId.UniqueThreadId,pstr);
   pstr+=8; *(DWORD*)pstr=0x00002820; pstr+=2;
   ltoa(ti.ClientId.UniqueThreadId,pstr,10);
   while(*(++pstr)); *(WORD*)pstr=0x0029;
   goto skipQuery;
  }
  *(DWORD*)t1nbuf4=0x03020100;
  goto skipQuery;
 case 11://File
  DWORD ret; ret=GetFileName(handle,(char*)t1nbuf4);
  if(ret==(DWORD)-1)goto skipQuery;
  if(ret==0){
   if(/**(BYTE*)t1nbuf4==0 ||*/ *t1nbuf4==*(DWORD*)(smemTable->access_denied))goto skipQuery;
   //((UNICODE_STRING*)nbuf5)->Buffer=(WCHAR*)nbuf4; ((UNICODE_STRING*)nbuf5)->Buffer+=2;
   //goto set_string;
  }
  break;
 }
 //GetKernelObjectSecurity();//GetSidIdentifierAuthority//GetSidSubAuthority//GetUserObjectSecurity
 if((*err=ntdllFunctions.pNtQueryObject(handle,ObjectNameInfo,nbuf5,2048,&tn))!=0){
  *(DWORD*)t1nbuf4=0x03020100;
  goto skipQuery;
 }
 //set_string
 strng.Length=0; strng.MaximumLength=2048; strng.Buffer=(LPSTR)t1nbuf4;
 ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,(UNICODE_STRING*)nbuf5,0);
 *(strng.Buffer+strng.Length)=0;
 if(type==11){//Object is an File. Let's check is it Pipe or Mailslot
  if(!gMailslotW.Buffer || !gNamedPipeW.Buffer)GetPipeAndMailslotPaths();
  if(CompareStrPartW(gNamedPipeW.Buffer,((UNICODE_STRING*)nbuf5)->Buffer,gNamedPipeW.Length,0)==0)type=31;
  else if(CompareStrPartW(gMailslotW.Buffer,((UNICODE_STRING*)nbuf5)->Buffer,gMailslotW.Length,0)==0)type=30;
 }
skipQuery:
 //is non-empty string
 if(*(LPSTR)t1nbuf4==0){
  if(progSetts.CheckState[40]==0)goto exit;
 }
 lvi.lParam=(LPARAM)HeapAlloc(pHeap,HEAP_ZERO_MEMORY,sizeof(HANDLE_INFOBUF));
 hif=(HANDLE_INFOBUF*)lvi.lParam;
 if(lvi.lParam!=0){
  hif->TypeNum=type; hif->HandleValue=(USHORT)hObj; int len;
  hif->ProcessId=shn->ProcessID;
  hif->Flags=shn->Flags;
  if(type==30)lvi.pszText=File_Mailslot;
  else if(type==31)lvi.pszText=File_Pipe;
  else lvi.pszText=tbuf8;
  lstrcpy(hif->nmType,lvi.pszText);
  if(*(BYTE*)t1nbuf4){
   len=getstrlen((char*)t1nbuf4); len++;
   hif->ObjName=(char*)HeapAlloc(pHeap,HEAP_ZERO_MEMORY,len);
   copybytes(hif->ObjName,(char*)t1nbuf4,len,0);
  }
  else {
   if(*(DWORD*)t1nbuf4==0x03020100){
    if(*err==0xC0000022 || (remotehfail && *err!=0)){hif->ObjName=smemTable->access_denied;}
    else ErrorStringFromNtError(*err,ERRSTRING_BORDER|ERRSTRING_ERRCODE_IF_NOSTRING|ERRSTRING_ALLOCATE_BUFFER,&hif->ObjName);
   }
   else *(BYTE*)t1nbuf4=0;
  }
 }
 lvi.mask=LVIF_PARAM|LVIF_IMAGE|LVIF_TEXT;
 lvi.iItem=lItem; lvi.iSubItem=0; lvi.iImage=I_IMAGECALLBACK;
 HexToString((DWORD)hObj,lbuf); lvi.pszText=lbuf;
 SendMessage(hdnList,LVM_INSERTITEM,0,(LPARAM)&lvi);
exit:
 if(shn->ProcessID!=cur_pid && !remotehfail){
  if(hRemoteProcess!=NULL)ntdllFunctions.NtClose(hRemoteProcess);
  if(handle!=NULL)ntdllFunctions.NtClose(handle);
 }
 return hif;
}

///////////////////////////// HandleList Compare ///////////////////////////////
int CALLBACK HandleListCompareFunc(LPARAM lParam1,LPARAM lParam2,LIST_SORT_DIRECTION *lParamSort){
HANDLE_INFOBUF *hif1,*hif2; int ret; char *ptr1,*ptr2; ptr1=ptr2=0;
hif1=(HANDLE_INFOBUF*)lParam1; hif2=(HANDLE_INFOBUF*)lParam2;
if(lParamSort->CurSubItem==0){ret=hif1->HandleValue; ret-=hif2->HandleValue; goto quit;}
if(lParamSort->CurSubItem==1){ptr1=hif1->nmType; ptr2=hif2->nmType;}
else if(lParamSort->CurSubItem==2){ptr1=hif1->ObjName; ptr2=hif2->ObjName;}
//else if(lParamSort->subLast==3){ptr1=hif1->Process; ptr2=hif2->Process;}
////////
if(ptr1 && ptr2)ret=lstrcmpi(ptr1,ptr2);
else if(ptr1==0){if(ptr2)ret=-1; else ret=0;}
else ret=1;
///////
quit:
if(lParamSort->SortDirection==1){if(ret<0)ret=1; else if(ret>0)ret=-1;}
return ret;}

int ListHandles(DWORD pidSel,DWORD fOptions){DWORD hi; int lItem; SYSTEM_HANDLE *shn; LV_ITEM lvi;
SendMessage(hHandleList,LVM_DELETEALLITEMS,0,0);
NtQSI_SysHandleInfo();
if(handleInfo==0)return 0;
lItem=0; shn=(SYSTEM_HANDLE*)(handleInfo->Handles);
//////// process' handles
if(fOptions&LISTHANDLES_FILTER_PROCESS){
 for(hi=0;hi<handleInfo->Count;hi++){
  if(shn->ProcessID==pidSel){
   GetNameType(shn,lItem);
   lItem++;
  }
  shn++;
 }
}
//obj_type: 9 - Event
//GrantedAcces: 0x001F0003
//Flags=0x01
//////// all handles
/*else if(fOptions&LISTHANDLES_LIST_ALL){
 for(hi=0;hi<handleInfo->Count;hi++){
  HANDLE_INFOBUF *hib;
  if((hib=GetNameType((HANDLE)(shn->HandleValue),shn->HandleTypeNumber,shn->ProcessID,lItem))!=0){
   if((hib->Process=(char*)HeapAlloc(pHeap,0,11))!=0){
    *(DWORD*)hib->Process=0x00007830;
    HexToString(shn->ProcessID,hib->Process+2);
   }
  }
  lItem++;
  shn++;
 }
}*/
//////// fill List
if(progSetts.HndlsSortd.CurSubItem!=0xFFFFFFFF && progSetts.HndlsSortd.SortDirection<2){
 progSetts.HndlsSortd.hFrom=hdnList;
 SendMessage(hdnList,LVM_SORTITEMS,(WPARAM)&progSetts.HndlsSortd,(LPARAM)(PFNLVCOMPARE)HandleListCompareFunc);
}
for(hi=0;hi<(DWORD)lItem;hi++){
 lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.iItem=hi; lvi.iSubItem=0;
 lvi.pszText=(char*)t1nbuf4; lvi.cchTextMax=2048;
 if(!SendMessage(hdnList,LVM_GETITEM,0,(LPARAM)&lvi))continue;
 lvi.mask=LVIF_PARAM|LVIF_TEXT|LVIF_IMAGE; lvi.iImage=I_IMAGECALLBACK;
 SendMessage(hHandleList,LVM_INSERTITEM,0,(LPARAM)&lvi);
 lvi.mask=LVIF_TEXT; lvi.iSubItem=1;
 lvi.pszText=LPSTR_TEXTCALLBACK;
 SendMessage(hHandleList,LVM_SETITEM,0,(LPARAM)&lvi);
 lvi.iSubItem=2;
 SendMessage(hHandleList,LVM_SETITEM,0,(LPARAM)&lvi);
}
SendMessage(hdnList,LVM_DELETEALLITEMS,0,0);
return lItem;
}


