
void MainMenu_InsertItem(char *pStr,HMENU hSubmenu,BYTE nPos,UINT wnID){MENUITEMINFO mii;
 mii.cbSize=sizeof(MENUITEMINFO);
 mii.fMask=MIIM_TYPE|MIIM_SUBMENU|MIIM_ID;
 mii.fType=MFT_STRING;
 mii.dwTypeData=pStr;
 mii.hSubMenu=hSubmenu;
 mii.wID=wnID;
 InsertMenuItem(main_menu,nPos,1,&mii);
}

void SaveSettings(){if(cmd_NoRegSetts)return;
 progSetts.cbSize=sizeof(PROGSETTINGS);
 RegSetValueEx(hMainKey,Settings_txt,NULL,REG_BINARY,(BYTE*)&progSetts,sizeof(PROGSETTINGS));
}

void CloseMainWindow(bool bForce){WINDOWPLACEMENT winpl;
 winpl.length=sizeof(WINDOWPLACEMENT);
 GetWindowPlacement(main_win,&winpl);
 progSetts.X=winpl.rcNormalPosition.left;
 progSetts.Y=winpl.rcNormalPosition.top;
 progSetts.wd=winpl.rcNormalPosition.right; progSetts.wd-=winpl.rcNormalPosition.left;
 progSetts.hg=winpl.rcNormalPosition.bottom; progSetts.hg-=winpl.rcNormalPosition.top;
 if(winpl.showCmd==SW_SHOWMAXIMIZED)progSetts.Maximized=1;
 else progSetts.Maximized=0;
 SaveColumns((DWORD)-1);
 if(!bForce && progSetts.CheckState[27])ShowWindow(main_win,SW_HIDE);//Hide On ESCape вместо закрытия
 else DestroyWindow(main_win);
}

// Меню по правому щелчку иконки в трее
void MainContextMenu(){POINT curpos; HMENU hMenu; MENUITEMINFO mii;
 GetCursorPos(&curpos);
 if((hMenu=BuildPopupMenu(Menus[9]))==0)return;
 mii.cbSize=sizeof(MENUITEMINFO); mii.fMask=MIIM_TYPE; mii.fType=MFT_STRING;
 if(IsIconic(main_win) || !IsWindowVisible(main_win)){
  mii.dwTypeData=smemTable->Restore;
  SetMenuDefaultItem(hMenu,50017,0);
 }
 else mii.dwTypeData=smemTable->Minimize;
 SetMenuItemInfo(hMenu,50017,0,&mii);
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
  ModernMenu_AddLeftBorder(hMenu,(DWORD)MainContextMenu_LeftLogo,1);
  ModernMenu_SetMenuStyle(0,hMenu,progSetts.MenuStyle);
 }
 SetForegroundWindow(main_win);
 TrackPopupMenuEx(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos.x,curpos.y,main_win,NULL);
 DestroyMenu(hMenu);
}

void resizeMainWin(LONG newhg,LONG newwd){int ststop,wplus;//,hplus;
 //hplus=newhg; hplus-=591;
 wplus=newwd; wplus-=682;
 if(pHdr!=NULL){
  pHdr->rcDisplay.bottom=newhg; pHdr->rcDisplay.bottom-=szstat.bottom;
  ststop=pHdr->rcDisplay.bottom;
  if(IsWindowVisible(pHdr->hwndTab)){
   pHdr->rcDisplay.right=newwd+2;
   SetWindowPos(pHdr->hwndTab,0,0,0,pHdr->rcDisplay.right,ststop+2,SWP_NOZORDER|SWP_NOREDRAW|SWP_NOOWNERZORDER|SWP_NOACTIVATE);
   pHdr->rcDisplay.bottom-=tabtop;
   SetWindowPos(pHdr->hwndDisplay,HWND_TOP,1,tabtop,pHdr->rcDisplay.right-4,pHdr->rcDisplay.bottom,SWP_NOACTIVATE);
  }
  else{
   pHdr->rcDisplay.right=newwd;
   SetWindowPos(pHdr->hwndDisplay,HWND_TOP,0,0,pHdr->rcDisplay.right,pHdr->rcDisplay.bottom,SWP_NOACTIVATE);
  }
 }
 SetWindowPos(hstatus,0,0,ststop,szstat.right+wplus,szstat.bottom,SWP_NOZORDER);
}

void CreateMainMenu(HWND hDlg){HMENU hMenu; MENUITEMINFO mii;
 if((main_menu=CreateMenu())==0)return;
 //Enable Modern Menu
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT)ModernMenu_Enable();
 //Make Sub menus
 if((hMenu=BuildMenu(Menus[0]))!=NULL){MainMenu_InsertItem(smemTable->_File,hMenu,0,47500);}
 if((hMenu=BuildMenu(Menus[1]))!=NULL){MainMenu_InsertItem(smemTable->_View,hMenu,1,47501);}
 if((hMenu=BuildMenu(Menus[2]))!=NULL){MainMenu_InsertItem(smemTable->_Help,hMenu,2,47502);}
 ////////////
 mii.cbSize=sizeof(MENUITEMINFO);
 mii.fMask=MIIM_TYPE|MIIM_ID; mii.fType=MFT_STRING;
 mii.dwTypeData=smemTable->Sh_utDown; mii.wID=50011;
 InsertMenuItem(main_menu,47502,0,&mii);
 SetMenu(hDlg,main_menu);
}

void CreateMainPanelMenu(){HMENU hmenu; MENUITEMINFO mnii; DWORD dTab;
 mnii.cbSize=sizeof(MENUITEMINFO);
 dTab=(BYTE)SendMessage(pHdr->hwndTab,TCM_GETCURSEL,0,0);
 if(dTab==TAB_PROCESSES){//Processes
  if((hmenu=GetSubMenu(main_menu,1))!=0){
   mnii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID;
   mnii.fType=MFT_STRING; mnii.hSubMenu=NULL;
   if(bShowAllProcesses)mnii.fState=MFS_ENABLED|MFS_CHECKED;
   else mnii.fState=MFS_ENABLED|MFS_UNCHECKED;
   mnii.dwTypeData=smemTable->ShowProcessesFromAllUsers; mnii.wID=47003;
   InsertMenuItem(hmenu,0,1,&mnii);
   // Bottom-pane submenu
   if((mnii.hSubMenu=BuildMenu(Menus[6]))!=0){
    mnii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_SUBMENU|MIIM_ID;
    mnii.fType=MFT_STRING; mnii.fState=MFS_ENABLED;
    mnii.dwTypeData=smemTable->BottomPane; mnii.wID=47002;
    if((progSetts.MenuStyle!=MENU_STYLE_DEFAULT) && (mnii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(5))!=0)mnii.fMask|=MIIM_DATA;
    InsertMenuItem(hmenu,0,1,&mnii);
    mnii.fMask&=~MIIM_DATA;
    mnii.hSubMenu=0; mnii.fType=MFT_SEPARATOR; mnii.wID=47004;
    InsertMenuItem(hmenu,1,1,&mnii);
   }
   mnii.fMask=MIIM_BITMAP;
   mnii.hbmpItem=(HBITMAP)LoadImage(gInst,(LPTSTR)16,IMAGE_ICON,18,19,LR_DEFAULTCOLOR);
   SetMenuItemInfo(main_menu,50010,0,&mnii);
  }
  if((hmenu=BuildMenu(Menus[3]))!=0){
   DWORD bl; if(dependsPath && *dependsPath)bl=MF_BYCOMMAND|MF_ENABLED; else bl=MF_BYCOMMAND|MF_GRAYED;
   EnableMenuItem(hmenu,40016,bl);
   MainMenu_InsertItem(smemTable->_Process,hmenu,2,47001);
  }
  EnableMainMenuItem(47001,MF_BYCOMMAND|MF_GRAYED);
  goto exit;
 }
 if(dTab==TAB_WINDOWS){//Windows
  if((hmenu=BuildMenu(Menus[4]))!=0){
   MainMenu_InsertItem(smemTable->_Window,hmenu,2,47101);
   EnableMainMenuItem(47101,MF_BYCOMMAND|MF_GRAYED);
  }
  if((hmenu=GetSubMenu(main_menu,1))!=0){
   mnii.fMask=MIIM_TYPE|MIIM_ID;
   mnii.fType=MFT_SEPARATOR; mnii.wID=47102;
   InsertMenuItem(hmenu,0,1,&mnii);
   mnii.fType=MFT_STRING;
   mnii.dwTypeData=smemTable->ShowGridLines;
   mnii.wID=30018;
   InsertMenuItem(hmenu,0,1,&mnii);
  }
  goto exit;
 }
 if(dTab==TAB_SERVICES){//Services
  if((hmenu=GetSubMenu(main_menu,1))!=0){
   mnii.fMask=MIIM_TYPE|MIIM_ID;
   mnii.fType=MFT_SEPARATOR; mnii.wID=47202;
   InsertMenuItem(hmenu,0,1,&mnii);
   mnii.fType=MFT_STRING;
   mnii.dwTypeData=smemTable->ShowGridLines;
   mnii.wID=30016;
   InsertMenuItem(hmenu,0,1,&mnii);
  }
  if((hmenu=BuildMenu(Menus[5]))!=0){
   DWORD bl; if(dependsPath && *dependsPath)bl=MF_BYCOMMAND|MF_ENABLED; else bl=MF_BYCOMMAND|MF_GRAYED;
   EnableMenuItem(hmenu,40020,bl);
   MainMenu_InsertItem(smemTable->_Service,hmenu,2,47201);
  }
  goto exit;
 }
 if(dTab==TAB_DRIVERS){//Drivers
  if((hmenu=GetSubMenu(main_menu,1))!=0){
   mnii.fMask=MIIM_TYPE|MIIM_ID;
   mnii.fType=MFT_SEPARATOR; mnii.wID=47301;
   InsertMenuItem(hmenu,0,1,&mnii);
   mnii.fType=MFT_STRING;
   mnii.dwTypeData=smemTable->ShowGridLines;
   mnii.wID=30025;
   InsertMenuItem(hmenu,0,1,&mnii);
  }
 }
 if(dTab==TAB_STARTUP){//Startup
  if((hmenu=BuildMenu(Menus[7]))!=0)MainMenu_InsertItem(smemTable->Item,hmenu,2,47401);
  if((hmenu=GetSubMenu(main_menu,1))!=0){
   mnii.fMask=MIIM_TYPE|MIIM_ID;
   mnii.fType=MFT_SEPARATOR; mnii.wID=47402;
   InsertMenuItem(hmenu,0,1,&mnii);
   mnii.fType=MFT_STRING;
   mnii.dwTypeData=smemTable->ShowGridLines;
   mnii.wID=30017;
   InsertMenuItem(hmenu,0,1,&mnii);
  }
  EnableMainMenuItem(47401,MF_BYCOMMAND|MF_GRAYED);
  goto exit;
 }
 exit:
 DrawMenuBar(main_win);
}

void RegSetts_LoadColumns(LPTSTR lpRegValue,DWORD dwBuiltinCnt,BOOL *pbValidOrderArrary,int *pClnumber,BYTE *pColumnsCreateOrder,int *pOrderArray,WORD *pColumnWidth){
 BYTE *pbt,*pbtn; DWORD nm,n,dw; DWORD32 crc,read_crc;
 if(hMainKey==0 || cmd_NoRegSetts)goto set_default;
 pbt=(BYTE*)t0nbuf1; nm=1024;
 if(RegQueryValueEx(hMainKey,lpRegValue,NULL,&n,pbt,&nm)==ERROR_SUCCESS && n==REG_BINARY && nm>sizeof(COLUMNS_SETTS_HEADER)){
  if(((COLUMNS_SETTS_HEADER*)pbt)->dwColumnsBuiltinCount>dwBuiltinCnt)n=dwBuiltinCnt;
  else n=((COLUMNS_SETTS_HEADER*)pbt)->dwColumnsBuiltinCount;
  read_crc=((COLUMNS_SETTS_HEADER*)pbt)->crc;
  dw=((sizeof(BYTE)*2)+sizeof(WORD))*n;
  dw+=sizeof(COLUMNS_SETTS_HEADER);
  if(dw<=nm){
   // calc crc32
   dw-=sizeof(DWORD32);
   pbt+=sizeof(DWORD32);
   crc=(DWORD32)-1;
   for(nm=0;nm<dw;nm++){
    crc=CALC_CRC32(*pbt,crc);
    pbt++;
   }
   crc=~crc;
   // verify crc
   if(crc==read_crc){//go
    *pbValidOrderArrary=((COLUMNS_SETTS_HEADER*)t0nbuf1)->bValidOrderArray;
    // columns create order
    pbtn=pbt=(BYTE*)t0nbuf1+(sizeof(DWORD32)*2)+sizeof(BOOL);
    for(dw=0;dw<n;dw++){*(pClnumber+dw)=*(pColumnsCreateOrder+dw)=*pbt; pbt++;}
    // columns order array
    pbt=pbtn+(((COLUMNS_SETTS_HEADER*)t0nbuf1)->dwColumnsBuiltinCount*sizeof(BYTE));
    pbtn=pbt;
    for(dw=0;dw<n;dw++){*(pOrderArray+dw)=*pbt; pbt++;}
    // columns width
    pbt=pbtn+(((COLUMNS_SETTS_HEADER*)t0nbuf1)->dwColumnsBuiltinCount*sizeof(BYTE));
    for(dw=0;dw<n;dw++){*(pColumnWidth+dw)=*(WORD*)pbt; pbt+=sizeof(WORD);}
    // RETURN SUCCESS
    return;
   }
  }
 }
 // SET DEFAULTS
set_default:
 for(dw=0;dw<dwBuiltinCnt;dw++){*(pClnumber+dw)=*(pColumnsCreateOrder+dw);}
}

void Main_LoadSettings(){BYTE *bptr; DWORD nm,n;
 RegSetts_LoadColumns(szProcessesColumns,PRCCOLUMNS_COUNT,&bValidProcsOrderArray,Prcclnumber,ColumnsCreateOrder,ProcsOrderArray,ColumnWidth);
 RegSetts_LoadColumns(szProcessesDllsColumns,DLLCOLUMNS_COUNT,&bValidProcsDllOrderArray,Dllclnumber,ProcsDllColumnsCreateOrder,ProcsDllOrderArray,ProcsDllColumnWidth);
 RegSetts_LoadColumns(szWindowsColumns,WINCOLUMNS_COUNT,&bValidWinOrderArray,Winclnumber,WinColumnsCreateOrder,WinOrderArray,WinColumnWidth);
 RegSetts_LoadColumns(szServicesColumns,SRVCOLUMNS_COUNT,&bValidSrvOrderArray,Srvclnumber,SrvColumnsCreateOrder,SrvOrderArray,ServColWidth);
 RegSetts_LoadColumns(szDriversColumns,DRVCOLUMNS_COUNT,&bValidDrvOrderArray,Drvclnumber,DrvColumnsCreateOrder,DrvOrderArray,DrvColumnWidth);
 //
 bptr=(BYTE*)t0nbuf1; nm=1024;
 if(RegQueryValueEx(hMainKey,Columns_txt,NULL,&n,bptr,&nm)==ERROR_SUCCESS && n==REG_BINARY){
  for(n=0;n<3;n++){HndlColumnWidth[n]=*(WORD*)bptr; bptr+=2;}
  for(n=0;n<3;n++){StartupColWidth[n]=*(WORD*)bptr; bptr+=2;}
  SrvInProccessColumnWidth[0]=*(WORD*)bptr; bptr+=2;
  SrvInProccessColumnWidth[1]=*(WORD*)bptr; //bptr+=2;
 }
}

BOOL CALLBACK Main_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){//main_win
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
  if(Message==WM_INITMENUPOPUP || Message==WM_UNINITMENUPOPUP || Message==WM_MEASUREITEM || Message==WM_DRAWITEM || Message==WM_ENTERMENULOOP || Message==WM_EXITMENULOOP){
   ModernMenu_DrawMenuProc(hDlg,Message,wParam,lParam);
  }
 }
 //else{
 // MenuDrawItemProc(hDlg,Message,wParam,lParam);
 //}
switch(Message){
case WM_INITDIALOG: DWORD n,dwDlgBase; TCITEM tci;
 main_win=hDlg;
 InitCommonControls();
 Main_LoadSettings();
 if((hThemeLib=LoadLibrary("UxTheme.dll"))!=0){
  pEnableThemeDialogTexture=(HRESULT(_stdcall*)(HWND,DWORD))GetProcAddress(hThemeLib,"EnableThemeDialogTexture");
  //pDrawThemeParentBackground=(HRESULT(_stdcall*)(HWND hwnd,HDC hdc,RECT *prc))GetProcAddress(hThemeLib,"DrawThemeParentBackground");
 }
 SetWindowPos(main_win,0,progSetts.X,progSetts.Y,690,640,SWP_NOZORDER);
 GetWindowRect(main_win,&strect);
 winHeight=strect.bottom-strect.top;
 GetClientRect(main_win,&strect); n=strect.bottom;
 hstatus=CreateWindowEx(0,STATUSCLASSNAME,0,SBARS_SIZEGRIP|WS_CHILD|WS_VISIBLE,0,0,0,0,hDlg,(HMENU)201,gInst,NULL);
 GetClientRect(hstatus,&szstat);
  hgminus+=GetSystemMetrics(SM_CYMENU);
  hgminus+=szstat.bottom; statusHg=szstat.bottom;
  n-=hgminus;
 SetWindowPos(hstatus,0,0,n,0,0,SWP_NOZORDER|SWP_NOSIZE);
 GetClientRect(hstatus,&szstat); szstat.top=n;
 pHdr=(DLGHDR*)LocalAlloc(LPTR,sizeof(DLGHDR));
 dwDlgBase=GetDialogBaseUnits();
 cxMargin=LOWORD(dwDlgBase)/4;
 cyMargin=HIWORD(dwDlgBase)/8;
 SetClassLongPtr(main_win,GCLP_HICON,(DWORD_PTR)LoadIcon(gInst,(char*)1));
 SetWindowLongPtr(main_win,GWLP_USERDATA,(DWORD_PTR)pHdr);
 SetWindowText(main_win,mainwin_title);
 pHdr->hwndTab=CreateWindow(WC_TABCONTROL,0,WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE,0,0,50,80,main_win,0,gInst,NULL);
 hTbimgl=ImageList_LoadImage(gInst,(char*)10,16,1,0x00FF00FF,IMAGE_BITMAP,LR_CREATEDIBSECTION);
 SendMessage(pHdr->hwndTab,TCM_SETIMAGELIST,0,(LPARAM)hTbimgl);
 tci.mask=TCIF_TEXT|TCIF_IMAGE;
 tci.iImage=0; tci.pszText=smemTable->Processes;
 SendMessage(pHdr->hwndTab,TCM_INSERTITEM,0,(LPARAM)&tci);
 tci.iImage++; tci.pszText=smemTable->Windows;
 SendMessage(pHdr->hwndTab,TCM_INSERTITEM,1,(LPARAM)&tci);
 tci.iImage++; tci.pszText=smemTable->Services;
 SendMessage(pHdr->hwndTab,TCM_INSERTITEM,2,(LPARAM)&tci);
 tci.iImage++; tci.pszText=smemTable->Drivers;
 SendMessage(pHdr->hwndTab,TCM_INSERTITEM,3,(LPARAM)&tci);
 tci.iImage++; tci.pszText=smemTable->Startup;
 SendMessage(pHdr->hwndTab,TCM_INSERTITEM,4,(LPARAM)&tci);
 tci.iImage++; tci.pszText=smemTable->Performance;
 SendMessage(pHdr->hwndTab,TCM_INSERTITEM,5,(LPARAM)&tci);
 SendMessage(pHdr->hwndTab,TCM_GETITEMRECT,0,(LPARAM)&strect);
 tabtop=strect.bottom+strect.top;
 pHdr->apRes[0]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)4,RT_DIALOG)));
 pHdr->apRes[1]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)17,RT_DIALOG)));
 pHdr->apRes[2]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)3,RT_DIALOG)));
 pHdr->apRes[3]=pHdr->apRes[0];
 pHdr->apRes[4]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)5,RT_DIALOG)));
 pHdr->apRes[5]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)6,RT_DIALOG)));
 pHdr->dlgProC[0]=(DLGPROC*)Processes_Proc;
 pHdr->dlgProC[1]=(DLGPROC*)WindowsWin_Proc;
 pHdr->dlgProC[2]=(DLGPROC*)Services_Proc;
 pHdr->dlgProC[3]=(DLGPROC*)DrvInfo_Proc;
 pHdr->dlgProC[4]=(DLGPROC*)RunEntries_Proc;
 pHdr->dlgProC[5]=(DLGPROC*)SysInfo_Proc;
 pHdr->updTimerProc[0]=(DWORD*)Processes_UpdateListView;
 pHdr->updTimerProc[1]=(DWORD*)UpdateEnumWindows;
 pHdr->updTimerProc[2]=pHdr->updTimerProc[3]=pHdr->updTimerProc[4]=0;
 pHdr->updTimerProc[5]=(DWORD*)Performance_UpdateInfo;
 //pHdr->OnDestroyCall[1]=(void*)DeleteAllItems_WindowsList;
 GetClientRect(main_win,&pHdr->rcDisplay); pHdr->rcDisplay.bottom=szstat.top;
 pHdr->rcDisplay.right+=2;
 LONG TabHeight;
 TabHeight=pHdr->rcDisplay.bottom-pHdr->rcDisplay.top;
 SetWindowPos(pHdr->hwndTab,NULL,0,2,pHdr->rcDisplay.right,TabHeight,SWP_NOZORDER);
 //tabtop=GetSystemMetrics(SM_CYCAPTION)+cyMargin; tabtop+=4;
 pHdr->rcDisplay.bottom-=tabtop;
 LOGFONT tabf;
 tabf.lfCharSet=RUSSIAN_CHARSET; tabf.lfHeight=8; tabf.lfWidth=0; tabf.lfWeight=100; tabf.lfOutPrecision=OUT_DEFAULT_PRECIS;
 lstrcpy(tabf.lfFaceName,MSsansSerif_txt); tabf.lfEscapement=0; tabf.lfOrientation=0; tabf.lfUnderline=0; tabf.lfStrikeOut=0; tabf.lfItalic=0;
 hDlgFont=CreateFontIndirect(&tabf);
 SendMessage(pHdr->hwndTab,WM_SETFONT,(WPARAM)hDlgFont,(LPARAM)TRUE);
 CreateMainMenu(hDlg);
 interpretChecks();
 haccel=LoadAccelerators(gInst,(char*)1);
 SendMessage(hstatus,SB_SIMPLE,(WPARAM)0,(LPARAM)0);
 pHdr->StatusBootTimeDays=(DWORD)-1;
 pHdr->StatusCPUUsgType=255;
 pHdr->StatusBarParts[0]=90; pHdr->StatusBarParts[1]=135;
 pHdr->StatusBarParts[2]=75; pHdr->StatusBarParts[3]=(DWORD)-1; //*(nbuf+3)=-1; //*(nbuf+2)=240; *(nbuf+3)=320;
 SetStatusBarParts();
 SendMessage(hstatus,SB_SETICON,(WPARAM)STATUSBAR_WINBOOT_ID,(LPARAM)LoadImage(gInst,(char*)22,IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
 if(bWarnSign)SendMessage(hstatus,SB_SETICON,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)LoadImage(gInst,(char*)18,IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
 copystring(t0buf1,smemTable->Processes_); strappend(t0buf1,"0");
 SendMessage(hstatus,SB_SETTEXT,(WPARAM)0,(LPARAM)t0buf1);
 SetWindowPos(main_win,0,0,0,progSetts.wd,progSetts.hg,SWP_NOZORDER|SWP_NOMOVE);
 if(progSetts.Maximized)ShowWindow(main_win,SW_SHOWMAXIMIZED);
 UpdateCPUusage();
 if(bTurnOffKernelCalc)progSetts.CheckState[26]=0;
 //gdpen=CreatePen(PS_SOLID,1,0x00006500);
 //gpen=CreatePen(PS_SOLID,1,0x0000ff00);
 //rpen=CreatePen(PS_SOLID,1,0x000000ff);
 //ylpen=CreatePen(PS_SOLID,1,0x0000ffff);
 DlgClassStyl=GetClassLong(hDlg,GCL_STYLE);
 if(progSetts.CheckState[30]>5)n=progSetts.LastTab;
 else n=progSetts.CheckState[30];
 SendMessage(pHdr->hwndTab,TCM_SETCURSEL,n,0); MainTab_OnTabSelChanged();
 break;
case WM_NOTIFY:
 if(((LPNMHDR)lParam)->code==TCN_SELCHANGE){
  MainTab_OnTabSelChanged();
  return 1;
 }
 break;
case WM_CLOSE:
 CloseMainWindow(0);
 return 1;
case WM_DESTROY:
 if(hThemeLib!=0){if(FreeLibrary(hThemeLib))hThemeLib=0;}
 SaveSettings();
 DeleteObject(hDlgFont);
 UnregisterClass(GraphMonClass_txt,gInst);
 ImageList_Destroy(hTbimgl);
 PostQuitMessage(0);
 return 1;
case WM_COMMAND: wID=LOWORD(wParam);
 if(wID>30001 && wID<50000){SendMessage(pHdr->hwndDisplay,WM_COMMAND,wParam,lParam); break;}
 switch(wID){
  case 50001:
   RunDlg_Show(hDlg);
   return 1;
  case 50004:
   HWND hwnd;
   if((hwnd=GetActiveWin())==main_win)CloseMainWindow(0);
   else SendMessage(hwnd,WM_CLOSE,0,0);
   return 1;
  case 50008:
   OpenHelpTopic(iSel,overview_htm);
   return 1;
  case 50009:
   if(abt_win!=NULL)SetForegroundWindow(abt_win);
   else CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)AboutThread,0,0,&n));
   return 1;
  case 50010:
   ControlUpdateThread(UPDATETHREAD_UPDATE,0xFF,0);
   SendMessage(pHdr->hwndDisplay,WM_COMMAND,wParam,lParam);
   return 1;
  case 50011:
   if(shtd_win==0)CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)ShutdownThread,0,0,&n));
   return 1;
  case 50016: case 50019:
   CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)RunThread,(void*)wID,0,&n));
   return 1;
  case 50017: showMainWin:
   if(IsWindowVisible(main_win)==0)ShowWindow(main_win,SW_SHOW);
   if(IsIconic(main_win))ShowWindow(main_win,SW_RESTORE);
   else {ShowWindow(main_win,SW_MINIMIZE); return 1;}
   SetForegroundWindow(main_win);
   LeaveWindowFocus(0);
   return 1;
  case 50018://CTRL+TAB
   int cnt,nk;
   cnt=SendMessage(pHdr->hwndTab,TCM_GETITEMCOUNT,0,0);
   if(cnt==0)break; cnt--;
   nk=SendMessage(pHdr->hwndTab,TCM_GETCURSEL,0,0);
   if(nk==-1)break;
   if(nk<cnt)nk++; else nk=0;
   SendMessage(pHdr->hwndTab,TCM_SETCURSEL,nk,0);
   MainTab_OnTabSelChanged();
   break;
  case 50022:
   CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)RemoteServicesWinThread,(void*)0,0,0));
   return 1;
  case 50023:
   CLSID_INFO_DLGPARAMS cid; cid.bDialogBox=0; cid.clsid=0;
   CreateDialogParam(gInst,(char*)16,main_win,(DLGPROC)CLSIDinfo_Proc,(DWORD)&cid);
   return 1;
  case 50024:
   CloseMainWindow(1);
   return 1;
  case 50025: case 50026:
   CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)ExtrasThread,(void*)((wID==50025)?LIST_MAILSLOTS:LIST_PIPES),0,&n));
   return 1;
  case 50027:
   if(settings_win==0)CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)SettingsWinThread,0,0,&n));
   else {
    user32Functions.SwitchToThisWindow(settings_win,1);
   }
   if(IsWindowVisible(main_win)==0)ShowWindow(main_win,SW_SHOW);
   if(IsIconic(main_win))ShowWindow(main_win,SW_RESTORE);
   SetForegroundWindow(main_win);
   LeaveWindowFocus(0);
   return 1;
  case 50028:
   OpenHelpTopic(iSel,license_htm);
   return 1;
  case 50029:
   OpenHelpTopic(iSel,payhelp_htm);
   return 1;
  case 50030:
   if(srvwinsets.hMain==0){
    srvwinsets.Caption=smemTable->ServicesGroups;
    srvwinsets.AddressToFree=0;
    srvwinsets.GroupNameFilter=0;
    if(CreateDialogParam(gInst,(char*)12,main_win,(DLGPROC)SrvcGroups_Proc,(LPARAM)&srvwinsets))ShowWindow(srvwinsets.hMain,SW_SHOW);
   }
   return 1;
  case 50031:
   if(LogOn_ShowLogonDlg()){
    CloseMainWindow(0);
   }
   return 1;
 }
 break;
case WM_SIZING:
 mainRect=(RECT*)lParam;
 newWidth=mainRect->right-mainRect->left;
 newHeight=mainRect->bottom-mainRect->top;
 if(winWidth==newWidth && winHeight==newHeight)return 0;
 if(newWidth<=MAINWIN_MIN_WIDTH){
  newWidth=MAINWIN_MIN_WIDTH; winWidth=MAINWIN_MIN_WIDTH;
  mainRect->right=mainRect->left+MAINWIN_MIN_WIDTH;
 }
 if(newHeight<=MAINWIN_MIN_HEIGHT){
  newHeight=MAINWIN_MIN_HEIGHT; winHeight=MAINWIN_MIN_HEIGHT;
  mainRect->bottom=mainRect->top+MAINWIN_MIN_HEIGHT;
 }
 winHeight=newHeight; winWidth=newWidth;
 return 1;
case WM_SIZE:
 if(wParam==SIZE_MINIMIZED && progSetts.CheckState[22])ShowWindow(main_win,SW_HIDE);//HideWhenMinimize
 else resizeMainWin(HIWORD(lParam),LOWORD(lParam));
 return 1;
case WM_INITMENU:
 if((HMENU)wParam!=main_menu)break;
 BYTE dTab;
 dTab=(BYTE)SendMessage(pHdr->hwndTab,TCM_GETCURSEL,0,0);
 if(dTab==TAB_PROCESSES){
  MENUITEMINFO mii;
  mii.cbSize=sizeof(MENUITEMINFO);
  mii.fMask=MIIM_STATE;
  if(!bShowAllProcesses)mii.fState=MFS_UNCHECKED;
  else mii.fState=MFS_CHECKED;
  SetMenuItemInfo(main_menu,47003,0,&mii);
 }
 return 1;
case WM_MY_TRAYICON:
 if(wParam==2 && lParam==WM_LBUTTONDOWN){// this message sends by another instance to show the existing window
  if(IsWindowVisible(main_win)==0)ShowWindow(main_win,SW_SHOW);
  if(IsIconic(main_win))ShowWindow(main_win,SW_RESTORE);
  SetForegroundWindow(main_win);
  LeaveWindowFocus(0);
  return 1;
 }
 if(wParam!=1)break;
 if(lParam==WM_LBUTTONDOWN)goto showMainWin;
 else if(lParam==WM_RBUTTONDOWN){
  MainContextMenu();
  return 1;
 }
 break;
case CREATE_SRVGROUP_DIALOG:
 if(CreateDialogParam(gInst,(char*)12,main_win,(DLGPROC)SrvcGroups_Proc,(LPARAM)lParam)){
  if(((SRVCGROUPS_WINSETTS*)lParam)->firstGroup)SendMessage(((SRVCGROUPS_WINSETTS*)lParam)->hTree,TVM_EXPAND,TVE_EXPAND,(LPARAM)((SRVCGROUPS_WINSETTS*)lParam)->firstGroup->hGrpItm);
  ShowWindow(((SRVCGROUPS_WINSETTS*)lParam)->hMain,SW_SHOW);
  return 1;
 }
 return 0;
} return 0;}