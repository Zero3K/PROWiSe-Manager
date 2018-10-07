typedef struct {
 HWND hList;
 HANDLE hHeap;
 SIZE winSize;
 SIZE ListSize;
 POINT pt_total_txt;
 POINT pt_total_result;
 DWORD iType;
 LIST_SORT_DIRECTION lsd;
}EXTRAS_WINSIZE_PARAMS;

typedef struct _struct1{BYTE InfoType; HWND hLVlist; HANDLE hHeap;}STRUCT1;

void CreateExtrasPopupMenu(HWND hDlg,HWND hList,bool bSingleSel){POINT curpos; HMENU pmenu;
 GetCursorPos(&curpos);
 pmenu=CreatePopupMenu();
 if(bSingleSel){//Add [CloseHandle]
  LV_ITEM lvi; DWORD dw; dw=MF_BYPOSITION|MF_STRING;
  if((lvi.iItem=SendMessage(hList,LVM_GETSELECTIONMARK,0,0))!=-1){
   lvi.mask=LVIF_PARAM;
   if(SendMessage(hList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi) && lvi.lParam){
    if(((HANDLE_INFOBUF*)lvi.lParam)->Flags&HANDLEFLAG_PROTECT_FROM_CLOSE)dw|=MF_GRAYED;
   }
  }
  InsertMenu(pmenu,0,dw,40101,smemTable->CloseHandle);
  InsertMenu(pmenu,0,MF_BYPOSITION|MF_SEPARATOR,0,0);
 }
 InsertMenu(pmenu,0,MF_BYPOSITION|MF_STRING,40100,smemTable->CopyToClipboard);
 TrackPopupMenuEx(pmenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos.x,curpos.y,hDlg,NULL);
 DestroyMenu(pmenu);
}

void ListPipeMailSlotHandles(HWND hLVList,DWORD iType,HANDLE hHeap){if(!hHeap)return;
int i; SYSTEM_HANDLE *shn; LV_COLUMN lvcol; LV_ITEM lvi; HWND hDlg; HANDLE hExtra=0; DWORD ErrCode;
lvi.iItem=0;
hDlg=GetParent(hLVList);
SendMessage(hLVList,LVM_DELETEALLITEMS,0,0);
if(!gMailslotW.Buffer || !gNamedPipeW.Buffer)GetPipeAndMailslotPaths();
if(!gMailslotW.Buffer || !gNamedPipeW.Buffer)return;
if(iType&LIST_QUICK){// через NtOpenFile() & Directory listing
 for(i=2;i>=0;i--)SendMessage(hLVList,LVM_DELETECOLUMN,i,0);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=0;
 lvcol.cx=340; lvcol.pszText=smemTable->Name; lvcol.iSubItem=0;
 SendMessage(hLVList,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
 ///
 DWORD size; BOOL firstQuery;
 PFILE_DIRECTORY_INFORMATION dirInfo,curEntry; IO_STATUS_BLOCK ioStatus;
 LSA_OBJECT_ATTRIBUTES oat; EXTRAS_INFOBUF_HEADER *exih;
 RtlZeroMemory(&oat,sizeof(LSA_OBJECT_ATTRIBUTES));
 oat.Length=sizeof(LSA_OBJECT_ATTRIBUTES);
 oat.Attributes=OBJ_CASE_INSENSITIVE;
 if(iType&LIST_PIPES)oat.ObjectName=&gNamedPipeW;
 else if(iType&LIST_MAILSLOTS)oat.ObjectName=&gMailslotW;
 ErrCode=ntdllFunctions.NtOpenFile(&hExtra,GENERIC_READ,&oat,&ioStatus,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,0);
 lvcol.pszText=RelativePath_txt(oat.ObjectName);
 if(lvcol.pszText){
  lvcol.mask=LVCF_TEXT; SendMessage(hLVList,LVM_SETCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
  LocalFree(lvcol.pszText);
 }
 if(ErrCode!=0){
  fail_exit:
  FailMessage("NOF",ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_SHOW_MSGBOX);
  goto exit_quick;
 }
 else if(ioStatus.Status!=0){ErrCode=ioStatus.Status; goto fail_exit;}
 else if(ioStatus.Information!=FILE_OPENED)goto fail_exit;
 DWORD dirInfoSize; dirInfoSize=1024;
 start_enum:
 dirInfo=(PFILE_DIRECTORY_INFORMATION)LocalAlloc(LMEM_FIXED,dirInfoSize);
 lvi.iItem=0; firstQuery=1;
 lvi.pszText=LPSTR_TEXTCALLBACK; lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.iSubItem=0;
 while(1){
  ErrCode=ntdllFunctions.NtQueryDirectoryFile(hExtra,0,0,0,&ioStatus,dirInfo,dirInfoSize,FileDirectoryInformation,0,0,firstQuery);
  if(ErrCode!=0){
   if(ErrCode==0x80000005){//MORE data available
    dirInfoSize+=1024;
    if(dirInfo)LocalFree(dirInfo);
    SendMessage(hLVList,LVM_DELETEALLITEMS,0,0);
    goto start_enum;
   }
   else if(ErrCode!=STATUS_NO_MORE_FILES){FailMessage("NQDF",ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_SHOW_MSGBOX);}
   break;
  }
  curEntry=dirInfo; 
  while(1){
   if(curEntry->FileNameLength && curEntry->FileName){
    size=curEntry->FileNameLength; size/=2; size++;
    if((lvi.lParam=(DWORD)HeapAlloc(hHeap,0,sizeof(EXTRAS_INFOBUF_HEADER)+size))!=0){
     exih=(EXTRAS_INFOBUF_HEADER*)(lvi.lParam);
     exih->ObjName=(char*)exih; exih->ObjName+=sizeof(EXTRAS_INFOBUF_HEADER);
     if(UnicodeStringToAnsiString(curEntry->FileName,curEntry->FileNameLength,exih->ObjName,size)){
      exih->HandleValue=0;
      SendMessage(hLVList,LVM_INSERTITEM,0,(LPARAM)&lvi);
      lvi.iItem++;
     }
     else HeapFree(hHeap,0,(void*)lvi.lParam);
    }
    else{
     FailMessage(smemTable->UnblAllocMem,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG); goto exit_quick;
    }
   }
   if(curEntry->NextEntryOffset==0)break;
   curEntry=(PFILE_DIRECTORY_INFORMATION)((PCHAR)curEntry+curEntry->NextEntryOffset);
  }
  firstQuery=FALSE;
 }
 exit_quick:
 if(dirInfo)LocalFree(dirInfo);
 if(hExtra)ntdllFunctions.NtClose(hExtra);
}
else if(iType&LIST_FULL){// ѕодробный список через обработку NtQuerySystemInfo(HandlesInfo)
 DWORD hi,pi_sz,*dbuf,*dbuf2,tn,ret; WORD type; HANDLE handle,hRemoteProcess;
 char *tbuf; bool remotehfail,pi_ok; BYTE *lmem=0;
 UNICODE_STRING *ustr,*fustr; STRING strng; SYSTEM_PROCESS_INFORMATION *spi;
 for(i=2;i>=0;i--)SendMessage(hLVList,LVM_DELETECOLUMN,i,0);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=LVCFMT_LEFT;
 lvcol.cx=65; lvcol.pszText=smemTable->WinColumnTitle[0]; lvcol.iSubItem=0;
 SendMessage(hLVList,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
  if(iType&LIST_PIPES)fustr=&gNamedPipeW;
  else if(iType&LIST_MAILSLOTS)fustr=&gMailslotW;
  tbuf=RelativePath_txt(fustr);
  if(tbuf){lvcol.pszText=tbuf;} else {lvcol.pszText=smemTable->Name;}
  lvcol.cx=240; lvcol.iSubItem=1;
  SendMessage(hLVList,LVM_INSERTCOLUMN,(WPARAM)1,(LPARAM)&lvcol);
  if(tbuf)LocalFree(tbuf);
 lvcol.cx=120; lvcol.pszText=smemTable->Process; lvcol.iSubItem=2;
 SendMessage(hLVList,LVM_INSERTCOLUMN,(WPARAM)2,(LPARAM)&lvcol);
 ///
 NtQSI_SysHandleInfo();
 if(handleInfo==0)goto exit_full;
 tn=0; hi=0; pi_ok=0; pi_sz=0;
 query_ntqsi: ret=ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,lmem,tn,&pi_sz);
 if(ret==0xC0000004 && tn<=pi_sz){
  tn=pi_sz; tn+=1024;
  if(lmem)lmem=(BYTE*)LocalFree(lmem);
  if(hi<3){hi++;
   lmem=(BYTE*)LocalAlloc(LMEM_FIXED,tn);
   goto query_ntqsi;
  }
 }
 else if(ret==0)pi_ok=1;
 shn=(SYSTEM_HANDLE*)(handleInfo->Handles);
 hi=(DWORD)LocalAlloc(LMEM_FIXED,2048+120);
 dbuf=(DWORD*)hi; hi+=1024;
 dbuf2=(DWORD*)hi; hi+=1024;
 tbuf=(char*)hi;
 for(hi=0;hi<handleInfo->Count;hi++){
  remotehfail=0; handle=0;
  if(shn->ProcessID!=cur_pid){
   hRemoteProcess=OpenProcess(PROCESS_DUP_HANDLE,0,shn->ProcessID);
   if(hRemoteProcess==NULL){handle=0; remotehfail=1;}
   if(DuplicateHandle(hRemoteProcess,(HANDLE)(shn->HandleValue),(HANDLE)0xFFFFFFFF,&handle,0,FALSE,DUPLICATE_SAME_ACCESS)==0)goto goNextHandle;
  }
  else handle=(HANDLE)(shn->HandleValue);
  if(!GetTypeToken(handle,tbuf,&type))goto goNextHandle;
  if(type!=11)goto goNextHandle;// Skip if not a File object
  *dbuf=0;
  if(GetFileName(handle,(char*)dbuf)!=0)goto goNextHandle;
  if(ntdllFunctions.pNtQueryObject(handle,ObjectNameInfo,dbuf2,1024,&tn)!=0)goto goNextHandle;
  ustr=(UNICODE_STRING*)dbuf2;
  if(CompareStrPartW(fustr->Buffer,ustr->Buffer,fustr->Length,0)!=0)goto goNextHandle;
  tn=fustr->Length; ustr->Length=(USHORT)(ustr->Length-fustr->Length);
  strng.Length=0; strng.MaximumLength=1024; strng.Buffer=(char*)dbuf;
  tn/=2; ustr->Buffer+=tn;
  ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,ustr,0);
  *(strng.Buffer+strng.Length)=0;
  DWORD allocsz; char *pstr; EXTRAS_INFOBUF *exif;
  allocsz=sizeof(EXTRAS_INFOBUF);
  pstr=strng.Buffer; do{allocsz++;} while(*(pstr++));//calc size of ObjName
  if(pi_ok){tn=0;
   strng.Length=0; strng.MaximumLength=512; strng.Buffer=(char*)dbuf2;
   while(tn<=pi_sz){
    spi=(SYSTEM_PROCESS_INFORMATION*)(lmem+tn);
    tn+=(spi->NextEntryOffset);
    if(spi->UniqueProcessId==shn->ProcessID){
     if(spi->UniqueProcessId!=0){
      if(spi->ProcessName.Length>1000)spi->ProcessName.Length=1000;
      ntdllFunctions.RtlUnicodeStringToAnsiString(&strng,&spi->ProcessName,0);
      *(strng.Buffer+strng.Length)=0;
     }
     else copystring((char*)dbuf2,smemTable->SysIdleProc);
     strappend((char*)dbuf2," <");
     ltoa(spi->UniqueProcessId,(char*)dbuf2+getstrlen((char*)dbuf2),10);
     strappend((char*)dbuf2,">");
     allocsz+=getstrlen((char*)dbuf2); allocsz++;
     break;
    }
   }
   if((spi->NextEntryOffset)==0)break;
  }
  lvi.lParam=(DWORD)HeapAlloc(hHeap,/*HEAP_ZERO_MEMORY*/0,allocsz);
  if(lvi.lParam){
   exif=(EXTRAS_INFOBUF*)lvi.lParam;
   lvi.mask=LVIF_PARAM; lvi.iSubItem=0;
   SendMessage(hLVList,LVM_INSERTITEM,0,(LPARAM)&lvi);
   pstr=(char*)lvi.lParam; pstr+=sizeof(EXTRAS_INFOBUF);
   exif->header.ObjName=pstr; pstr+=copystring(pstr,(char*)dbuf); pstr++;//store ObjName
   exif->ProcessName=pstr; copystring(pstr,(char*)dbuf2);//store ProcessName
   exif->header.HandleValue=shn->HandleValue;
   HexToString(shn->HandleValue,(char*)dbuf);
   lvi.mask=LVIF_TEXT; lvi.pszText=(char*)dbuf;
   SendMessage(hLVList,LVM_SETITEM,0,(LPARAM)&lvi);
   exif->Flags=shn->Flags;
   exif->ProcessId=shn->ProcessID;
   lvi.iItem++;
  }
  goNextHandle:
  if(shn->ProcessID!=cur_pid && !remotehfail){
   if(hRemoteProcess!=NULL)CloseHandle(hRemoteProcess);
   if(handle!=NULL)CloseHandle(handle);
  }
  shn++;
 }
 LocalFree(dbuf);
 exit_full:
 if(lmem)LocalFree(lmem);
}
SetDlgItemInt(hDlg,105,lvi.iItem,0);
}

int CALLBACK ExtrasCompareFunc(LPARAM lParam1,LPARAM lParam2,LIST_SORT_DIRECTION *lParamSort){
int ret=0; EXTRAS_INFOBUF *exi1,*exi2;
exi1=(EXTRAS_INFOBUF*)lParam1; exi2=(EXTRAS_INFOBUF*)lParam2;
if(exi1==0){ret=-1; goto ret_result;}
if(exi2==0){ret=1; goto ret_result;}
if(!exi1->header.HandleValue){//quick list => ObjName only
 if(lParamSort->iSub==0){
  ret=lstrcmpi(exi1->header.ObjName,exi2->header.ObjName);
 }
 goto ret_result;
}
else{
 if(lParamSort->iSub==0){ret=exi1->header.HandleValue; ret-=exi2->header.HandleValue;}
 else if(lParamSort->iSub==1){
  ret=lstrcmpi(exi1->header.ObjName,exi2->header.ObjName);
 }
 else if(lParamSort->iSub==2){
  ret=lstrcmpi(exi1->ProcessName,exi2->ProcessName);
 }
}
ret_result:
if(lParamSort->SortDirection==1){if(ret<0)ret=1; else if(ret>0)ret=-1;}
return ret;
}

/////////////////////////////// SystemHandles_Proc /////////////////////////////
BOOL CALLBACK Extras_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){EXTRAS_WINSIZE_PARAMS *wsp; LV_ITEM lvi;
switch(Message){
case WM_INITDIALOG:
 HWND hndlist; RECT rect; if(lParam==0)break;
 STRUCT1 *sprm; sprm=(STRUCT1*)lParam;
 SetWinPos(hDlg,1);
 hndlist=GetDlgItem(hDlg,101); if(hndlist==0)break;
 wsp=(EXTRAS_WINSIZE_PARAMS*)LocalAlloc(LPTR,sizeof(EXTRAS_WINSIZE_PARAMS));
 if(wsp){
  GetClientRect(hDlg,&rect);
  SetWindowLong(hDlg,GWL_USERDATA,(DWORD)wsp);
  wsp->winSize.cy=rect.bottom; wsp->winSize.cx=rect.right;
  GetWindowRect(hndlist,&rect);
  rect.right-=rect.left; rect.bottom-=rect.top;
  wsp->ListSize.cy=rect.bottom; wsp->ListSize.cx=rect.right;
  wsp->hList=hndlist; wsp->lsd.hFrom=hndlist; wsp->lsd.SortDirection=0; wsp->lsd.CurSubItem=(DWORD)-1;
  wsp->iType=sprm->InfoType; wsp->hHeap=sprm->hHeap;
  if(sprm->InfoType&LIST_PIPES || sprm->InfoType&LIST_MAILSLOTS){
   GetWindowRect(GetDlgItem(hDlg,104),&rect); wsp->pt_total_txt.x=rect.left; wsp->pt_total_txt.y=rect.top; ScreenToClient(hDlg,&wsp->pt_total_txt);
   GetWindowRect(GetDlgItem(hDlg,105),&rect); wsp->pt_total_result.x=rect.left; wsp->pt_total_result.y=rect.top; ScreenToClient(hDlg,&wsp->pt_total_result);
   CheckRadioButton(hDlg,102,103,102);
  }
 }
 char *ptitle; ptitle=0;
 if(sprm->InfoType&LIST_PIPES)ptitle=smemTable->Pipes;
 else if(sprm->InfoType&LIST_MAILSLOTS)ptitle=smemTable->MailSlots;
 SetWindowText(hDlg,ptitle);
 WindowFillText(hDlg,dlgTAT->dlg25);
 sprm->hLVlist=hndlist; SetFocus(hndlist);
 SendMessage(hndlist,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_FLATSB|LVS_EX_LABELTIP|LVS_EX_GRIDLINES);
 if(pEnableThemeDialogTexture>0)pEnableThemeDialogTexture(hDlg,0x00000006);
 break;
case WM_CLOSE:DestroyWindow(hDlg); break;
case WM_DESTROY:
 wsp=(EXTRAS_WINSIZE_PARAMS*)GetWindowLong(hDlg,GWL_USERDATA); if(!wsp)break;
 SendMessage(wsp->hList,LVM_DELETEALLITEMS,0,0);
 LocalFree(wsp);
 PostQuitMessage(0);
 break;
case WM_SIZING:
 RECT *mRect; int nwd,nhg;
 mRect=(RECT*)lParam;
 nwd=mRect->right; nwd-=mRect->left;
 nhg=mRect->bottom; nhg-=mRect->top;
 if(nwd<380){mRect->right=mRect->left; mRect->right+=380;}
 if(nhg<270){mRect->bottom=mRect->top; mRect->bottom+=270;}
 return 1;
case WM_SIZE:
 wsp=(EXTRAS_WINSIZE_PARAMS*)GetWindowLong(hDlg,GWL_USERDATA);
 if(!wsp)break;
 int hplus,wplus;
 wplus=LOWORD(lParam); wplus-=wsp->winSize.cx;
 hplus=HIWORD(lParam); hplus-=wsp->winSize.cy;
 HDWP hdwp,hdwpcur;
 hdwpcur=hdwp=BeginDeferWindowPos(3); if(hdwp==0)break;
 DeferWindowPos(hdwp,wsp->hList,0,0,0,wsp->ListSize.cx+wplus,wsp->ListSize.cy+hplus,SWP_NOZORDER|SWP_NOMOVE);
 hdwpcur=DeferWindowPos(hdwpcur,GetDlgItem(hDlg,104),0,wsp->pt_total_txt.x+wplus,wsp->pt_total_txt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 hdwpcur=DeferWindowPos(hdwpcur,GetDlgItem(hDlg,105),0,wsp->pt_total_result.x+wplus,wsp->pt_total_result.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 EndDeferWindowPos(hdwpcur);
 break;
case WM_COMMAND:
 WORD wID; wID=LOWORD(wParam);
 if(wID==50004)DestroyWindow(hDlg);
 else if(wID==102 || wID==103){
  wsp=(EXTRAS_WINSIZE_PARAMS*)GetWindowLong(hDlg,GWL_USERDATA); if(!wsp)break;
  wsp->iType&=~(LIST_QUICK|LIST_FULL);
  wsp->iType|=((wID==102)?LIST_QUICK:LIST_FULL);
  wsp->lsd.SortDirection=0; wsp->lsd.CurSubItem=-1;
  ListPipeMailSlotHandles(wsp->hList,wsp->iType,wsp->hHeap);
 }
 else if(wID==40100){//Copy ObjName to clipboard
  wsp=(EXTRAS_WINSIZE_PARAMS*)GetWindowLong(hDlg,GWL_USERDATA); if(!wsp)break;
  int selcnt,sel; DWORD memsz;
  selcnt=SendMessage(wsp->hList,LVM_GETSELECTEDCOUNT,0,0);
  if(selcnt==0 || !OpenClipboard(hDlg))break;
  sel=selcnt; memsz=0; lvi.mask=LVIF_PARAM; lvi.iItem=-1;
  while(selcnt){
   if((lvi.iItem=SendMessage(wsp->hList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)break;
   if(SendMessage(wsp->hList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
    memsz+=getstrlen(((EXTRAS_INFOBUF*)(lvi.lParam))->header.ObjName); memsz+=3;
   }
   selcnt--;
  }
  HGLOBAL clipbuffer; char *buffer;
  clipbuffer=GlobalAlloc(GMEM_DDESHARE,memsz);
  buffer=(char*)GlobalLock(clipbuffer); *buffer=0; lvi.iItem=-1;
  for(selcnt=sel;selcnt;selcnt--){
   if((lvi.iItem=SendMessage(wsp->hList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)break;
   if(SendMessage(wsp->hList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
    buffer+=copystring(buffer,((EXTRAS_INFOBUF*)(lvi.lParam))->header.ObjName);
    buffer+=copystring(buffer,"\x0D\x0A");
   }
  }
  buffer-=2; *buffer=0;
  EmptyClipboard();
  GlobalUnlock(clipbuffer);
  SetClipboardData(CF_TEXT,clipbuffer);
  CloseClipboard();
 }
 else if(wID==40101){
  wsp=(EXTRAS_WINSIZE_PARAMS*)GetWindowLong(hDlg,GWL_USERDATA); if(!wsp)break;
  if((lvi.iItem=SendMessage(hHandleList,LVM_GETSELECTIONMARK,0,0))==-1)break;
  lvi.mask=LVIF_PARAM;
  if(SendMessage(wsp->hList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
  EXTRAS_INFOBUF *exif; exif=(EXTRAS_INFOBUF*)lvi.lParam;
  if(exif->header.HandleValue==0)break;
  if(CloseObjHandle(hDlg,(HANDLE)(exif->header.HandleValue),exif->Flags,exif->ProcessId)){
   SendMessage(wsp->hList,LVM_DELETEITEM,(WPARAM)lvi.iItem,0);
   SetDlgItemInt(hDlg,105,SendMessage(wsp->hList,LVM_GETITEMCOUNT,0,0),0);
  }
 }
 break;
case WM_NOTIFY:
 DWORD dCode; dCode=((NMHDR*)lParam)->code;
 if(dCode!=LVN_COLUMNCLICK && dCode!=NM_RCLICK && dCode!=LVN_GETDISPINFO && dCode!=LVN_DELETEITEM)break;
 wsp=(EXTRAS_WINSIZE_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA); if(!wsp)break;
 if(wsp->hList!=((NM_LISTVIEW*)lParam)->hdr.hwndFrom)break;
 if(dCode==LVN_COLUMNCLICK){
  wsp->lsd.iSub=(DWORD)((NM_LISTVIEW*)lParam)->iSubItem;
  wsp->lsd.bRestoreSort=0;
  ListViewColumnSortClick((NM_LISTVIEW*)lParam,&wsp->lsd,(DWORD_PTR)ExtrasCompareFunc);
 }
 else if(dCode==NM_RCLICK){
  LONG selcnt; bool bl;
  selcnt=SendMessage(wsp->hList,LVM_GETSELECTEDCOUNT,0,0);
  if(selcnt==0)break;
  if(selcnt==1)bl=1; else bl=0;
  if(wsp->iType&LIST_QUICK)bl=0;
  CreateExtrasPopupMenu(hDlg,wsp->hList,bl);
 }
 else if(dCode==LVN_GETDISPINFO){
  NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam;
  EXTRAS_INFOBUF *exinf; exinf=(EXTRAS_INFOBUF*)(nvd->item.lParam); if(exinf==0)break;
  if(nvd->item.mask&LVIF_TEXT){
   int iw; iw=nvd->item.iSubItem;
   if(exinf->header.HandleValue){
    if(iw==1)nvd->item.pszText=exinf->header.ObjName;
    else if(iw==2)nvd->item.pszText=exinf->ProcessName;
   }
   else{
    if(iw==0)nvd->item.pszText=exinf->header.ObjName;
   }
   if(nvd->item.pszText==0)nvd->item.pszText="";
  }
 }
 else if(dCode==LVN_DELETEITEM){
  lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=((NM_LISTVIEW*)lParam)->iItem;
  if(SendMessage(((NMHDR*)lParam)->hwndFrom,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
  HeapFree(wsp->hHeap,0,(void*)lvi.lParam);
 }
 break;
} return 0;}

DWORD ExtrasThread(BYTE InfoType){MSG msg; HACCEL haccel; HWND extras_win; STRUCT1 sprm; HANDLE hHeap;
if((hHeap=HeapCreate(0,4096,0xFFFFFF))==NULL){FailMessage(HeapCreate_txt,0,FMSG_CallGLE|FMSG_WRITE_LOG); return 0;}
InfoType&=~LIST_FULL; InfoType|=LIST_QUICK;
sprm.InfoType=InfoType;
sprm.hHeap=hHeap;
extras_win=CreateDialogParam(gInst,(char*)25,main_win,(DLGPROC)Extras_Proc,(DWORD)&sprm);
if(extras_win==NULL)goto exit;
ShowWindow(extras_win,SW_SHOW);
AddHwndToWinChain(extras_win);
haccel=LoadAccelerators(gInst,(char*)2);
ListPipeMailSlotHandles(sprm.hLVlist,sprm.InfoType,hHeap);
while(GetMessage(&msg,NULL,0,0)){
 if(haccel!=NULL && TranslateAccelerator(extras_win,haccel,&msg))continue;
 if(!IsDialogMessage(extras_win,&msg)){
  TranslateMessage(&msg);DispatchMessage(&msg);
 }
}
LeaveWindowFocus(extras_win);
exit:
if(hHeap)HeapDestroy(hHeap);
ExitThread(0);
return 0;}
