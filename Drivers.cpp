
void ShowDrvEntries(){DWORD nnSize,memSize,ErrCode; void* drvMem,*vmem; SYSTEM_MODULE_INFORMATION *smdi;
 SYSTEM_MODULE *smd; MODULE_INFOBUF *mdi; int i,ln; LV_ITEM lvi;
 drvMem=0; memSize=nnSize=0;
 SendMessage(DrvList,LVM_DELETEALLITEMS,0,0);
 SendMessage(hdnList,LVM_DELETEALLITEMS,0,0);
 SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)smemTable->Wait_);
queryAgain:
 if(bPaneDestroy)return;
 if((ErrCode=ntdllFunctions.pNtQuerySystemInformation(SystemModuleInformation,drvMem,memSize,&nnSize))!=0){
  if(ErrCode==0xC0000004 && nnSize!=0 && drvMem==0){memSize=nnSize; memSize+=sizeof(SYSTEM_MODULE);
   drvMem=VirtualAllocEx((HANDLE)-1,0,memSize,MEM_COMMIT,PAGE_READWRITE);
   goto queryAgain;
  }
  else {FailMessage("NtQSI()",ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG); goto exit;}
 }
 smdi=(SYSTEM_MODULE_INFORMATION*)drvMem;
 smd=smdi->Modules;
 vmem=0;
 lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.iSubItem=0; lvi.pszText=LPSTR_TEXTCALLBACK; lvi.iItem=0;
 for(i=smdi->ModulesCount;i>0;i--){
  if(bPaneDestroy)break;
  mdi=(MODULE_INFOBUF*)HeapAlloc(pHeap,HEAP_ZERO_MEMORY,sizeof(MODULE_INFOBUF));
  if(mdi==0){meRp++; goto memFailexit;} meRp=0;
  mdi->NameOfs=smd->NameOffset; ln=lstrlen((char*)(smd->ModulePath)); ln++;
  mdi->String[0]=HeapAlloc(pHeap,HEAP_ZERO_MEMORY,ln);
  if(mdi->String[0])lstrcpy((char*)(mdi->String[0]),(char*)(smd->ModulePath));
  if(Drvclnumber[2] || Drvclnumber[3] || Drvclnumber[6]){
   DWORD w1,w2,w3;
   w1=Drvclnumber[2];
   w2=Drvclnumber[6];
   w3=Drvclnumber[3];
   vmem=FileModule_GetVerDescInfo(CorrectFilePath((char*)(smd->ModulePath),0,t1nbuf2),&w2,&w1,&w3,0);
   if(Drvclnumber[2] && w1){
    ln=lstrlen((char*)w1); ln++;
    if((mdi->String[1]=HeapAlloc(pHeap,HEAP_ZERO_MEMORY,ln))==0)goto memFailexit;
    lstrcpy((char*)(mdi->String[1]),(char*)w1);
   }
   if(Drvclnumber[3] && w3){
    ln=lstrlen((char*)w3); ln++;
    if((mdi->String[2]=HeapAlloc(pHeap,HEAP_ZERO_MEMORY,ln))==0)goto memFailexit;
    lstrcpy((char*)(mdi->String[2]),(char*)w3);
   }
   if(Drvclnumber[6] && w2){
    ln=lstrlen((char*)w2); ln++;
    if((mdi->String[3]=HeapAlloc(pHeap,HEAP_ZERO_MEMORY,ln))==0)goto memFailexit;
    lstrcpy((char*)(mdi->String[3]),(char*)w2);
   }
  }
  if(Drvclnumber[4])HexToString((DWORD)(smd->ImageBaseAddress),mdi->str1);
  if(Drvclnumber[5])HexToString((DWORD)(smd->ImageSize),mdi->str2);
  lvi.lParam=(DWORD)mdi;
  SendMessage(hdnList,LVM_INSERTITEM,0,(LPARAM)&lvi);
  lvi.iItem++; smd++;
 }
exit2:
 if(vmem)LocalFree(vmem);
 if(progSetts.DrvSortd.CurSubItem!=(DWORD)-1 && progSetts.DrvSortd.SortDirection<2){
  progSetts.DrvSortd.hFrom=(HWND)&Drvclnumber;
  SendMessage(hdnList,LVM_SORTITEMS,(WPARAM)&progSetts.DrvSortd,(LPARAM)(PFNLVCOMPARE)DllsListCompareFunc);
 }
 i=lvi.iItem; lvi.iSubItem=0; lvi.pszText=LPSTR_TEXTCALLBACK;
 for(lvi.iItem=0;lvi.iItem<i;lvi.iItem++){
  lvi.mask=LVIF_PARAM;
  SendMessage(hdnList,LVM_GETITEM,0,(LPARAM)&lvi);
  lvi.mask=LVIF_PARAM|LVIF_TEXT;
  SendMessage(DrvList,LVM_INSERTITEM,0,(LPARAM)&lvi);
 }
 SendMessage(hdnList,LVM_DELETEALLITEMS,0,0);
exit:
 lstrcpy(t1buf1,smemTable->Drivers_);
 ltoa(smdi->ModulesCount,t1buf1+lstrlen(t1buf1),10);
 if(drvMem)VirtualFreeEx((HANDLE)-1,drvMem,0,MEM_RELEASE);
 SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)t1buf1);
 return;
memFailexit:
 if(meRp<3)FailMessage(smemTable->UnblAllocMem,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
 meRp++; goto exit2;
}

//////////////////////////////////// On Init ///////////////////////////////////
void OnInit_DrvEntries(){DWORD cnt; int i;
 listsz.right=pHdr->rcDisplay.right-10;
 listsz.bottom=pHdr->rcDisplay.bottom-6;
 DrvList=CreateWindowEx(WS_EX_CLIENTEDGE,WC_LISTVIEW,0,LVS_SINGLESEL|LVS_SHOWSELALWAYS|LVS_REPORT|WS_CHILD|WS_VISIBLE,3,3,listsz.right,listsz.bottom,drvi_win,(HMENU)101,gInst,NULL);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=LVCFMT_LEFT;
 cnt=0;
 for(i=0;i<DRVCOLUMNS_COUNT;i++){
  for(int t=0;t<DRVCOLUMNS_COUNT;t++){
   if(DrvColumnsCreateOrder[t]==i+1 && smemTable->DllColumnTitle[t][0]){
    if(DrvColumnsAlign[t]==DT_LEFT)lvcol.fmt=LVCFMT_LEFT;
    else if(DrvColumnsAlign[t]==DT_RIGHT)lvcol.fmt=LVCFMT_RIGHT;
    lvcol.pszText=smemTable->DllColumnTitle[t];
    lvcol.cx=DrvColumnWidth[t]; lvcol.iSubItem=i;
    Drvclnumber[t]=SendMessage(DrvList,LVM_INSERTCOLUMN,(WPARAM)i,(LPARAM)&lvcol); Drvclnumber[t]++;
    if(Drvclnumber[t]>0)cnt++;
    break;
   }
  }
 }
 n=LVS_MY_EXTENDED_STYLE; if(progSetts.CheckState[25]==MFS_CHECKED)n|=LVS_EX_GRIDLINES;
 SendMessage(DrvList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
 if(bValidDrvOrderArray)SendMessage(DrvList,LVM_SETCOLUMNORDERARRAY,(WPARAM)cnt,(LPARAM)&DrvOrderArray);
 CheckMarkMenuItems();
 if(pHeap==0)pHeap=HeapCreate(0,1024,0);
}

///////////////////////////////// DrvInfo_Proc /////////////////////////////////
BOOL CALLBACK DrvInfo_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){LV_ITEM lvi;
if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
 if(Message==WM_INITMENUPOPUP || Message==WM_UNINITMENUPOPUP || Message==WM_MEASUREITEM || Message==WM_DRAWITEM || Message==WM_ENTERMENULOOP || Message==WM_EXITMENULOOP){
  ModernMenu_DrawMenuProc(hDlg,Message,wParam,lParam);
 }
}
switch(Message){
case WM_INITDIALOG:
 drvi_win=hDlg;
 initHg=pHdr->rcDisplay.bottom;
 initWd=pHdr->rcDisplay.right-4;
 MainTab_OnChildDialogInit(hDlg); pHdr->hwndDisplay=hDlg;
 OnInit_DrvEntries();
 itemSel=-1; SetFocus(DrvList);
 bPaneDestroy=0; ExecuteAddr=(DWORD_PTR)ShowDrvEntries;
 ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0xFE);
 if(progSetts.DrvSortd.CurSubItem!=(DWORD)-1)SetSortArrowIcon(DrvList,(DWORD)-1,progSetts.DrvSortd.CurSubItem,progSetts.DrvSortd.SortDirection);
 break;
case WM_DESTROY:
 SaveColumns_Drivers();
 if(DrvList!=0 && DestroyWindow(DrvList))DrvList=NULL;
 if(hdnList!=0){DestroyWindow(hdnList); hdnList=NULL;}
 DeleteMenu(main_menu,47301,MF_BYCOMMAND);
 DeleteMenu(main_menu,30025,MF_BYCOMMAND);
 if(pHeap){HeapDestroy(pHeap); pHeap=0;}
 SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)0);
 drvi_win=NULL;
 break;
case WM_COMMAND: wID=LOWORD(wParam);
 switch(wID){
  case 30025:
   n=LVS_MY_EXTENDED_STYLE;
   if(progSetts.CheckState[25]==MFS_CHECKED)progSetts.CheckState[25]=MFS_UNCHECKED;
   else {progSetts.CheckState[25]=MFS_CHECKED; n|=LVS_EX_GRIDLINES;}
   CheckMenuItem(main_menu,30025,progSetts.CheckState[25]);
   SendMessage(DrvList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
   break;
  case 40018:
   lvi.mask=LVIF_PARAM; lvi.iSubItem=0; lvi.iItem=itemSel;
   if(SendMessage(DrvList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   MODULE_INFOBUF *mdi; mdi=(MODULE_INFOBUF*)lvi.lParam;
   if(mdi->String[0]==0)break;
   File_OpenProperies(CorrectFilePath((char*)(mdi->String[0]),0,t0nbuf2));
   break;
  case 40026://Copy Path to clipboard
   int selcnt,sel; DWORD memsz;
   selcnt=SendMessage(DrvList,LVM_GETSELECTEDCOUNT,0,0);
   if(selcnt==0 || !OpenClipboard(hDlg))break;
   sel=selcnt; memsz=0; lvi.mask=LVIF_PARAM; lvi.iItem=-1;
   while(selcnt){
    if((lvi.iItem=SendMessage(DrvList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)break;
    if(SendMessage(DrvList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
     memsz+=getstrlen((char*)((MODULE_INFOBUF*)(lvi.lParam))->String[0]); memsz+=3;
    }
    selcnt--;
   }
   HGLOBAL clipbuffer; char *buffer;
   clipbuffer=GlobalAlloc(GMEM_DDESHARE,memsz);
   buffer=(char*)GlobalLock(clipbuffer); *buffer=0; lvi.iItem=-1;
   for(selcnt=sel;selcnt;selcnt--){
    if((lvi.iItem=SendMessage(DrvList,LVM_GETNEXTITEM,lvi.iItem,LVNI_SELECTED))==-1)break;
    if(SendMessage(DrvList,LVM_GETITEM,lvi.iItem,(LPARAM)&lvi) && lvi.lParam){
     buffer+=copystring(buffer,(char*)((MODULE_INFOBUF*)(lvi.lParam))->String[0]);
     buffer+=copystring(buffer,"\x0D\x0A");
    }
   }
   buffer-=2; *buffer=0;
   EmptyClipboard();
   GlobalUnlock(clipbuffer);
   SetClipboardData(CF_TEXT,clipbuffer);
   CloseClipboard();
   break;
  case 40030: CreateColumnSelDlg(4); break;
  case 50010:
   ExecuteAddr=(DWORD_PTR)ShowDrvEntries;
   ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0xFE);
   SetSortArrowIcon(WindowsList,progSetts.DrvSortd.CurSubItem,(DWORD)-1,0);
   break;
 }
 break;
case WM_SIZE:
 int wplus,hplus;
 wplus=LOWORD(lParam); wplus-=initWd;
 hplus=HIWORD(lParam); hplus-=initHg;
 SetWindowPos(DrvList,0,0,0,listsz.right+wplus,listsz.bottom+hplus,SWP_NOZORDER|SWP_NOMOVE);
 break;
case WM_NOTIFY:
 DWORD dwCode; POINT curpos;
 dwCode=((NMHDR*)lParam)->code;
 if(dwCode==NM_RCLICK){
  itemSel=((NMLISTVIEW*)lParam)->iItem;
  GetCursorPos(&curpos);
  if(CheckHeaderClick(DrvList,((NMHDR*)lParam)->hwndFrom,&curpos)==1 || itemSel==-1)break;//header or non-item clicked
 mouse_Rclick:
  FileModule_PopupMenu(hDlg,&curpos,DRIVERS_tab);
  break;
 }
 if(((NMHDR*)lParam)->hwndFrom!=DrvList)break;
 if(dwCode==LVN_GETDISPINFO){
  NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam; MODULE_INFOBUF *mif;
  mif=(MODULE_INFOBUF*)nvd->item.lParam; if(mif==0)break;
  int iw; iw=nvd->item.iSubItem; iw++;
  if(nvd->item.mask&LVIF_TEXT){
   if(iw==Drvclnumber[0]){if(mif->String[0])nvd->item.pszText=(char*)(mif->String[0])+mif->NameOfs;}
   else if(iw==Drvclnumber[1]){if(*(DWORD*)(mif->str1)!=0xFFFFFF00)nvd->item.pszText=(char*)(mif->String[0]);}
   else if(iw==Drvclnumber[2])nvd->item.pszText=(char*)(mif->String[1]);
   else if(iw==Drvclnumber[3])nvd->item.pszText=(char*)(mif->String[2]);
   else if(iw==Drvclnumber[4])nvd->item.pszText=mif->str1;
   else if(iw==Drvclnumber[5])nvd->item.pszText=mif->str2;
   else if(iw==Drvclnumber[6])nvd->item.pszText=(char*)(mif->String[3]);
   if(nvd->item.pszText==0)nvd->item.pszText="";
  }
 }
 else if(dwCode==LVN_DELETEITEM){
  lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=((NM_LISTVIEW*)lParam)->iItem;
  if(SendMessage(DrvList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
  MODULE_INFOBUF *mif; mif=(MODULE_INFOBUF*)lvi.lParam;
  int i; for(i=0;i<4;i++){if(mif->String[i])HeapFree(pHeap,0,mif->String[i]);}
  HeapFree(pHeap,0,(void*)lvi.lParam);
 }
 else if(dwCode==LVN_COLUMNCLICK){
  progSetts.DrvSortd.hFrom=(HWND)&Drvclnumber;
  progSetts.DrvSortd.bRestoreSort=0;
  ListViewColumnSortClick((NM_LISTVIEW*)lParam,&progSetts.DrvSortd,(DWORD)DllsListCompareFunc);
 }
 else if(dwCode==NM_RETURN){
  RECT rect; rect.left=LVIR_BOUNDS;
  if(((NM_LISTVIEW*)lParam)->hdr.hwndFrom!=DrvList)break;
  itemSel=GetItemSelected(DrvList);
  if(SendMessage(DrvList,LVM_GETITEMRECT,(WPARAM)itemSel,(LPARAM)&rect)){
   curpos.y=rect.top; rect.bottom-=rect.top; rect.bottom/=2; curpos.y+=rect.bottom;
   curpos.x=rect.right; curpos.x-=rect.left;
   GetWindowRect(DrvList,&rect);
   curpos.y+=rect.top; rect.right-=rect.left;
   if(curpos.x>rect.right)curpos.x=rect.right;
   curpos.x/=2; curpos.x+=rect.left;
   ((NMHDR*)lParam)->code=NM_RCLICK;
   lvi.lParam=0; lvi.mask=LVIF_PARAM; lvi.iItem=itemSel;
   SendMessage(DrvList,LVM_GETITEM,(WPARAM)0,(LPARAM)&lvi);
   goto mouse_Rclick;
  }
 }
 break;
} return 0;}
