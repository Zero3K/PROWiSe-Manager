//B6FFC24C-7E13-11D0-9B47-00C04FC2F51D

HWND rgwin[3]={0,0,0};

typedef struct {
 bool bDialogBox;
 char *clsid;
}CLSID_INFO_DLGPARAMS;

/////////////////////////// Enum Search Regedit TreeView ///////////////////////
BOOL CALLBACK SearchRegeditTreeView(HWND hwnd,LPARAM lParam){
if(lParam==1){
 if(GetParent(hwnd)==NULL){rgwin[1]=hwnd; return 0;}
}
else if(lParam==2){
 GetClassName(hwnd,t2buf2,105);
 if(lstrcmpi(t2buf2,SysTreeView32_txt)==0){rgwin[0]=hwnd; return 0;}
}
else if(lParam==3){
 GetClassName(hwnd,t2buf2,105);
 if(lstrcmpi(t2buf2,SysListView32_txt)==0){rgwin[2]=hwnd; return 0;}
}
return 1;}

///////////////////////////// Jump to Registry Editor //////////////////////////
void JumpToRegedit(char *pkey,bool noValueJmp){DWORD dw; LV_ITEM lvi;
 if(rgwin[1]!=NULL && IsWindow(rgwin[1]))goto controlRegedit;
 DWORD TreadId;
 rgwin[1]=NULL; rgwin[0]=NULL;
 DWORD nn; void *smem; smem=NULL;
 n=ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,NULL,0,&nn);
 if(n==0xC0000004 && nn>0){
  nn+=1024; dw=nn; smem=VirtualAlloc(NULL,nn,MEM_COMMIT,PAGE_READWRITE);
 }
 if(smem){
  n=ntdllFunctions.pNtQuerySystemInformation(SystemProcessesAndThreadsInformation,smem,dw,&nn);
  if(n==0){
   SYSTEM_PROCESS_INFORMATION *spi; TreadId=0; TreadId--;
   while(n<=nn){
    spi=(SYSTEM_PROCESS_INFORMATION*)((BYTE*)(smem)+n);
    n+=(spi->NextEntryOffset);
    if(lstrcmpiW(spi->ProcessName.Buffer,L"regedit.exe")==0){TreadId=spi->Threads[1].ClientId.UniqueThreadId; break;}
    if((spi->NextEntryOffset)==0)break;
   }
   VirtualFree(smem,dw,MEM_DECOMMIT);
   if(TreadId!=(DWORD)-1)goto findWindow;
  }
 }
 PROCESS_INFORMATION pi; STARTUPINFO stp;
 ZeroMemory(&stp,sizeof(stp)); stp.cb=sizeof(stp); ZeroMemory(&pi,sizeof(pi));
 *t2buf2=0; dw=GetWindowsDirectory(t2buf2,300);
 if(dw>0){if(*(t2buf2+dw)!='\\'){*(t2buf2+dw)='\\'; dw++; *(t2buf2+dw)=0;}}
 lstrcat(t2buf2,"regedit.exe");
 if(CreateProcess(0,t2buf2,0,0,0,0,0,0,&stp,&pi)==0){
  FailMessage(0,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX);
  return;
 }
 TreadId=pi.dwThreadId; Sleep(1500);
 findWindow:
 EnumThreadWindows(TreadId,SearchRegeditTreeView,1);
 if(rgwin[1]==NULL)return;
 EnumChildWindows(rgwin[1],SearchRegeditTreeView,2);
 if(rgwin[0]==NULL)return;
 EnumChildWindows(rgwin[1],SearchRegeditTreeView,3);
 controlRegedit:
 LONG wStyle;
 if((wStyle=GetWindowLong(rgwin[1],GWL_STYLE))!=0){
  if((wStyle&WS_VISIBLE)==0)ShowWindow(rgwin[1],SW_SHOW);
  if(wStyle&WS_MINIMIZE)ShowWindow(rgwin[1],SW_RESTORE);
 }
 SetForegroundWindow(rgwin[1]);
 BringWindowToTop(rgwin[1]);
 if(progSetts.CheckState[0] && progSetts.CheckState[20])ShowWindow(main_win,SW_MINIMIZE);
 for(int i=0;i<15;i++){
  SendMessage(rgwin[0],WM_KEYDOWN,VK_LEFT,0);
 }
 SendMessage(rgwin[0],WM_KEYDOWN,VK_HOME,0);
 SendMessage(rgwin[0],WM_KEYDOWN,VK_LEFT,0);
 Sleep(50);
 SendMessage(rgwin[0],WM_KEYDOWN,VK_RIGHT,0);
 while(*pkey!=0){
  if(*pkey!='\\')SendMessage(rgwin[0],WM_CHAR,*pkey,0);
  else {SendMessage(rgwin[0],WM_KEYDOWN,VK_RIGHT,0); Sleep(50);}
  pkey++;
 }
 if(noValueJmp)return;
 lvi.iItem=itemSel; lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iSubItem=0;
 SendMessage(StRunList,LVM_GETITEM,itemSel,(LPARAM)&lvi);
 lvi.pszText=t2buf1; lvi.cchTextMax=300; *t2buf1=0;
 if(lvi.lParam&ITEM_CLSID){
  lvi.iSubItem=1; SendMessage(StRunList,LVM_GETITEMTEXT,itemSel,(LPARAM)&lvi);
  if(*t2buf1==0)return;
  pkey=t2buf1;
  while(*pkey!=0){SendMessage(rgwin[0],WM_CHAR,*pkey,0); pkey++;}
  SendMessage(rgwin[0],WM_KILLFOCUS,(WPARAM)rgwin[0],0);
  SendMessage(rgwin[0],WM_SETFOCUS,(WPARAM)rgwin[2],0);
 }
 else if(lvi.lParam==ITEM_REGVALUE && rgwin[2]!=NULL){
  lvi.iSubItem=0; SendMessage(StRunList,LVM_GETITEMTEXT,itemSel,(LPARAM)&lvi);
  if(*t2buf1==0)return;
  pkey=t2buf1;
  SendMessage(rgwin[2],WM_KEYDOWN,VK_HOME,0); Sleep(50);
  while(*pkey!=0){SendMessage(rgwin[2],WM_CHAR,*pkey,0); pkey++;}
  SendMessage(rgwin[2],WM_SETFOCUS,(WPARAM)rgwin[0],0);
 }
}

void GetCLSIDinfo(HWND hDlg, char *clsid){HKEY hMainKey,hKey; DWORD dw; char *lmem,*pmem; bool bst; int in;
 WindowFillText(hDlg,dlgTAT->dlg16);
 lmem=(char*)LocalAlloc(LMEM_FIXED,400); if(lmem==0)goto exit; pmem=lmem;
 if(clsid==0){
  if(GetDlgItemText(hDlg,100,pmem,55)){clsid=pmem; pmem+=100;}
  else {
   SetFocus(GetDlgItem(hDlg,100));
   goto exit;
  }
 }
 copystring(pmem,"CLSID\\");
 if(*clsid!='{')strappend(pmem,"{");
 strappend(pmem,clsid);
 in=getstrlen(clsid); in--;
 if(*(clsid+in)!='}')strappend(pmem,"}");
 SendMessage(GetDlgItem(hDlg,100),WM_SETTEXT,0,(LPARAM)(pmem+6));
 if(RegOpenKey(HKEY_CLASSES_ROOT,pmem,&hMainKey)==ERROR_SUCCESS){//HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID
  SendMessage(GetDlgItem(hDlg,500),WM_SETTEXT,0,(LPARAM)0);
  n=300;
  if(RegQueryValueEx(hMainKey,NULL,NULL,&dw,(BYTE*)pmem,&n)==ERROR_SUCCESS && (dw==REG_SZ || dw==REG_EXPAND_SZ || dw==REG_MULTI_SZ)){
   SendMessage(GetDlgItem(hDlg,201),WM_SETTEXT,0,(LPARAM)pmem);
  }
  ////
  char *psinf; int tn;
  psinf=CLSID_ServerInfo; in=203; tn=103;
  qnext: bst=0;
  if(RegOpenKey(hMainKey,psinf,&hKey)==ERROR_SUCCESS){
   n=300;
   if(RegQueryValueEx(hKey,NULL,NULL,&dw,(BYTE*)pmem,&n)==ERROR_SUCCESS && (dw==REG_SZ || dw==REG_EXPAND_SZ || dw==REG_MULTI_SZ)){
    SendMessage(GetDlgItem(hDlg,in),WM_SETTEXT,0,(LPARAM)pmem); bst=1;
   }
   RegCloseKey(hKey);
  }
  EnableWindow(GetDlgItem(hDlg,tn),bst);
  EnableWindow(GetDlgItem(hDlg,in),bst);
  psinf+=getstrlen(psinf); psinf++;
  if(*psinf){in++; tn++; goto qnext;}
  ////
  if(RegOpenKey(hMainKey,ProgID_txt,&hKey)==ERROR_SUCCESS){
   n=300;
   if(RegQueryValueEx(hKey,NULL,NULL,&dw,(BYTE*)pmem,&n)==ERROR_SUCCESS && (dw==REG_SZ || dw==REG_EXPAND_SZ || dw==REG_MULTI_SZ)){
    SendMessage(GetDlgItem(hDlg,202),WM_SETTEXT,0,(LPARAM)pmem);
   }
   RegCloseKey(hKey);
  }
  RegCloseKey(hMainKey);
 }
 exit:
 EnableWindow(GetDlgItem(hDlg,400),1); EnableWindow(GetDlgItem(hDlg,401),1);
 if(lmem)LocalFree(lmem);
}

HBITMAP AppendTextToImg(char *ImgRcName,int imgWd,int imgHg,char *szText,HWND hwnd){
HDC hdc,tdc; HBITMAP hbmp,hImg; RECT rect; BITMAPINFO bmi; DWORD dw; SIZE sz; HBRUSH hbr; TEXTMETRIC txm;
hImg=(HBITMAP)LoadImage(gInst,ImgRcName,IMAGE_BITMAP,imgWd,imgHg,LR_LOADMAP3DCOLORS);
if(!hImg)return 0;
hdc=CreateCompatibleDC(0); tdc=CreateCompatibleDC(hdc);
RtlZeroMemory(&bmi,sizeof(BITMAPINFO));
bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
bmi.bmiHeader.biHeight=imgHg;
bmi.bmiHeader.biWidth=imgWd;
SelectObject(hdc,(HANDLE)SendMessage(hwnd,WM_GETFONT,0,0));
dw=getstrlen(szText);
if(!GetTextExtentPoint32(hdc,szText,dw,&sz)){
 if(GetClientRect(hwnd,&rect))sz.cx=rect.right; else sz.cx=0;
}
bmi.bmiHeader.biWidth+=sz.cx; bmi.bmiHeader.biWidth+=5;
if(GetTextMetrics(hdc,&txm))bmi.bmiHeader.biWidth+=txm.tmAveCharWidth;
bmi.bmiHeader.biPlanes=1; bmi.bmiHeader.biBitCount=24;
hbmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,NULL,0,0);
if(!hbmp)return hImg;
SelectObject(tdc,hImg); SelectObject(hdc,hbmp);
dw=GetSysColor(COLOR_BTNFACE);
SetBkColor(hdc,dw);
rect.left=0; rect.top=0; rect.right=bmi.bmiHeader.biWidth; rect.bottom=imgHg;
hbr=CreateSolidBrush(dw); FillRect(hdc,&rect,hbr); DeleteObject(hbr);
BitBlt(hdc,0,0,imgWd,imgHg,tdc,0,0,SRCCOPY);
rect.left=imgWd; rect.left+=5; rect.right--; rect.top=1;
SetTextColor(hdc,GetSysColor(COLOR_BTNTEXT));
DrawTextEx(hdc,szText,-1,&rect,DT_LEFT|DT_MODIFYSTRING|DT_VCENTER|DT_END_ELLIPSIS,0);
DeleteDC(hdc); DeleteDC(tdc);
return hbmp;//надо DeleteObject(hbmp);
}

BOOL CALLBACK CLSIDinfo_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
switch(Message){
 case WM_INITDIALOG:
  if(pEnableThemeDialogTexture>0)pEnableThemeDialogTexture(hDlg,0x00000006);
  SendMessage(GetDlgItem(hDlg,400),BM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadImage(gInst,(char*)3,IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
  HBITMAP hbmp;
  HWND hbtn; hbtn=GetDlgItem(hDlg,401);
  hbmp=AppendTextToImg((char*)12,19,16,smemTable->JumpToRegistry,hbtn);
  if(hbmp)SendMessage(hbtn,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hbmp);
  CLSID_INFO_DLGPARAMS cid;
  if(lParam!=0){
   cid.bDialogBox=((CLSID_INFO_DLGPARAMS*)lParam)->bDialogBox; cid.clsid=((CLSID_INFO_DLGPARAMS*)lParam)->clsid;
  }
  else {cid.bDialogBox=0; cid.clsid=0;}
  SetWindowLong(hDlg,GWL_USERDATA,cid.bDialogBox);
  GetCLSIDinfo(hDlg,(char*)(cid.clsid));
  SetWinPos(hDlg,1);
  if(!cid.bDialogBox)AddHwndToWinChain(hDlg);
  SetWindowText(hDlg,smemTable->CaptionCLSIDinfo);
  ShowWindow(hDlg,SW_SHOW);
  break;
 case WM_COMMAND:
  if(LOWORD(wParam)==IDCANCEL)goto close_win;
  if(LOWORD(wParam)==400)GetCLSIDinfo(hDlg,0);
  else if(LOWORD(wParam)==401){
   char *lmem; lmem=(char*)LocalAlloc(LMEM_FIXED,150);
   if(lmem==0)break;
   int ln;
   ln=copystring(lmem,HKEYCLASSESROOT_CLSID);
   if(GetDlgItemText(hDlg,100,lmem+ln,55))JumpToRegedit(lmem,0);
   LocalFree(lmem);
  }
  break;
 case WM_KEYDOWN:
  MSG msgx;
  msgx.hwnd=hDlg;
  msgx.message=Message;
  msgx.wParam=wParam;
  msgx.lParam=lParam;
  IsDialogMessage(hDlg,&msgx);
  break;
 case WM_CLOSE: close_win:
  if(GetWindowLong(hDlg,GWL_USERDATA))EndDialog(hDlg,1);
  else DestroyWindow(hDlg);
  break;
 case WM_DESTROY:
  DeleteObject((HANDLE)SendMessage(GetDlgItem(hDlg,401),BM_GETIMAGE,IMAGE_BITMAP,0));
  LeaveWindowFocus(hDlg);
  break;
} return 0;}
