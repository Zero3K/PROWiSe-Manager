/*
#include "prowise.cpp"
#include "WinTransparency.cpp"
#include "WinStyles.cpp"
*/
POINT sz3[2]; char *srchClassName=NULL; LV_FINDINFO lvfi;

#define FILTER_IF_CAPTION_EXIST 2
#define FILTER_CLASSNAME_MATCH 4
#define FILTER_IF_VISIBLE 8
#define FILTER_IF_NOPARENT 16
#define COMPARE_CASESENSITIVE 32

typedef struct _WINDOW_INFO_WHILESTRUSCT{
 DWORD dwFilter;
 BOOL bDestroy;
}WINDOW_INFO_WHILESTRUSCT;

#define IsVALID_WINDOW_INFO(pWinInfo) ((BOOL)(pWinInfo!=0 && pWinInfo->cbSize==sizeof(WINDOW_INFO)))

#include "WinStyles.cpp"
#include "WinTransparency.cpp"

WINDOW_INFO *fwinf=0,*winlastinf=0;

BOOL CALLBACK CloseWin_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);

void SwitchToWindow2(HWND hwnd){HWND hwnd2;
 if(hwnd==main_win || GetWindow(hwnd,GW_OWNER))goto mbeep;
 if(IsIconic(hwnd))ShowWindow(hwnd,SW_RESTORE);
 hwnd2=GetLastActivePopup(hwnd);
 if(IsWindow(hwnd2)!=0 && (GetWindowLong(hwnd2,GWL_STYLE)&WS_DISABLED)==0){
  if(user32Functions.SwitchToThisWindow!=NULL){
   user32Functions.SwitchToThisWindow(hwnd2,1);
   if(progSetts.CheckState[20])ShowWindow(main_win,SW_MINIMIZE);
  }
 }
 else{
mbeep:
  MessageBeep(MB_OK);
 }
}

void Window_Action(HWND hwin,WINDOW_INFO *pwinf,DWORD dwAction){DWORD n; DWORD_PTR dwp; BYTE btOpacity;
 switch(dwAction){
  case 1: SwitchToWindow2(hwin); break;
  case 2:
   if((dwp=GetWindowLongPtr(hwin,GWL_EXSTYLE))==0)break;
   SetWindowPos(hwin,(dwp&WS_EX_TOPMOST)?HWND_NOTOPMOST:HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
   break;
  case 3: case 4: case 8: case 9: case 10:
   if(dwAction==3)n=SW_MINIMIZE;
   else if(dwAction==4)n=SW_MAXIMIZE;
   else if(dwAction==8)n=SW_RESTORE;
   else if(dwAction==9)n=SW_HIDE;
   else if(dwAction==10)n=SW_SHOW;
   ShowWindowAsync(hwin,n);
   break;
  case 5:
   DialogBox(gInst,(char*)19,main_win,(DLGPROC)CloseWin_Proc);
   break;
  case 6://Process Properties
   n=(DWORD)-1;
   GetWindowThreadProcessId(hwin,&n);
   if(n==(DWORD)-1)break;
   CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)ProcInfoThread,(void*)n,0,&n));
   break;
  case 7:
   CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)WindowPropertiesThread,(void*)hwin,0,&n));
   break;
  case 11: case 12: case 13: case 14: case 15:
   if(!IsVALID_WINDOW_INFO(pwinf))break;
   dwp=GetWindowLongPtr(hwin,GWL_EXSTYLE);
   if(dwAction!=11){//if not 100%
    if((dwp&WS_EX_LAYERED)==WS_EX_LAYERED){
     if(pwinf->bLayered==0){pwinf->bLayered=1;}
    }
    else {
     if(pwinf->bLayered==0){pwinf->bLayered=2;}
     SetWindowLongPtr(hwin,GWL_EXSTYLE,dwp|WS_EX_LAYERED);
    }
   }
   if(dwAction==12)btOpacity=179;
   else if(dwAction==13)btOpacity=127;
   else if(dwAction==14)btOpacity=64;
   else if(dwAction==15)btOpacity=25;
   else btOpacity=255;
   user32Functions.SetLayeredWindowAttributes(hwin,0,btOpacity,LWA_ALPHA);
   if(dwAction==11 && pwinf->bLayered==2){
    dwp&=~WS_EX_LAYERED;
    SetWindowLongPtr(hwin,GWL_EXSTYLE,dwp);
    RedrawWindow(hwin,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN);
   }
   break;
  case 16:
   CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)WinCustomTransparency_Thread,(void*)pwinf,0,&n));
   break;
 }
}

void RECTtoString(RECT *rect,char *ptr){
 *ptr='('; ptr++;
 itoa(rect->left,ptr,10); ptr+=lstrlen(ptr); *ptr++=',';
 itoa(rect->top,ptr,10); ptr+=lstrlen(ptr); *ptr++=',';
 itoa(rect->right,ptr,10); ptr+=lstrlen(ptr); *ptr++=',';
 itoa(rect->bottom,ptr,10); ptr+=lstrlen(ptr);
 *ptr++=')'; *ptr++=','; *ptr++=0x20; *ptr=0;
 rect->right-=rect->left; itoa(rect->right,ptr,10); ptr+=lstrlen(ptr); *ptr++='x';
 rect->bottom-=rect->top; itoa(rect->bottom,ptr,10);
}

int CALLBACK WindowsListCompareFunc(LPARAM lParam1,LPARAM lParam2,LIST_SORT_DIRECTION *lParamSort){
 int ret,isub; WINDOW_INFO *wif1,*wif2; char *ptr1,*ptr2; LVITEM lvi; LV_FINDINFO lvf;
 if(lParamSort->SortDirection==2){
  wif1=fwinf; ret=0;
  while(wif1!=0 && wif1->cbSize==sizeof(WINDOW_INFO)){
   if((DWORD)wif1==(DWORD)lParam1){ret=-1;break;}
   else if((DWORD)wif1==(DWORD)lParam2){ret=1;break;}
   wif1=wif1->NextEntry;
  }
  goto quit;
 }
 else{
  lvf.flags=LVFI_PARAM;
  wif1=(WINDOW_INFO*)lParam1;
nextwif:
  if(wif1->hParent!=0){
   lvf.lParam=(DWORD)wif1;
   if((lvi.iItem=SendMessage(WindowsList,LVM_FINDITEM,-1,(LPARAM)&lvf))!=(DWORD)-1){
    lvi.mask=LVIF_INDENT; lvi.iSubItem=0;
    if(SendMessage(WindowsList,LVM_GETITEM,0,(LPARAM)&lvi)){
     if(lvi.iIndent>0){lvi.iIndent=0; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);}
    }
   }
  }
  if(wif1!=(WINDOW_INFO*)lParam2){wif1=(WINDOW_INFO*)lParam2; goto nextwif;}
 }
 isub=lParamSort->CurSubItem; isub++; ptr1=ptr2=0;
 wif1=(WINDOW_INFO*)lParam1; wif2=(WINDOW_INFO*)lParam2;
 if(isub==Winclnumber[0]){ret=(DWORD)wif1->hWnd; ret-=(DWORD)wif2->hWnd; goto quit;}
 else if(isub==Winclnumber[1]){ptr1=wif1->Title; ptr2=wif2->Title;}
 else if(isub==Winclnumber[2]){ptr1=wif1->ClassName; ptr2=wif2->ClassName;}
 else if(isub==Winclnumber[3]){ptr1=wif1->Coordinates; ptr2=wif2->Coordinates;}
 else if(isub==Winclnumber[4]){ret=wif1->Style; ret-=wif2->Style; goto quit;}
 else if(isub==Winclnumber[5]){ret=wif1->StyleEx; ret-=wif2->StyleEx; goto quit;}
 else if(isub==Winclnumber[6]){
  if(wif1->NotResponding==1)ptr1=smemTable->NotResponding;
  else ptr1=smemTable->Running;
  if(wif2->NotResponding==1)ptr2=smemTable->NotResponding;
  else ptr2=smemTable->Running;
 }
 if(ptr1 && ptr2)ret=lstrcmpi(ptr1,ptr2);
 else if(ptr1==0){if(ptr2)ret=-1; else ret=0;}
 else ret=1;
quit:
 if(lParamSort->SortDirection==1){if(ret<0)ret=1; else if(ret>0)ret=-1;}
 return ret;
}

/////////////////////////////// AllWindowsList /////////////////////////////////
BOOL CALLBACK AllWindowsList(HWND hwnd,LPARAM lpFilter){RECT rect; HWND hOwner; WINDOW_INFO *pwinf; DWORD n,dwStyle; bool bExist;
 if(bPaneDestroy)return 0;
 GetWindowText(hwnd,t1buf4,300);
 if(lpFilter&FILTER_IF_CAPTION_EXIST && *t1buf4==0)return 1;
 if(lpFilter&FILTER_CLASSNAME_MATCH && srchClassName!=NULL){
  GetClassName(hwnd,t1buf1,300);
  if(lpFilter&COMPARE_CASESENSITIVE){
   if(lstrcmp(t1buf1,srchClassName)!=0)return 1;
  }
  else {
   if(lstrcmpi(t1buf1,srchClassName)!=0)return 1;
  }
 }
 dwStyle=GetWindowLong(hwnd,GWL_STYLE);
 if(lpFilter&FILTER_IF_VISIBLE && (dwStyle&WS_VISIBLE)!=WS_VISIBLE)return 1;
 hOwner=GetWindow(hwnd,GW_OWNER);
 if(lpFilter&FILTER_IF_NOPARENT){
  if(hOwner!=0)return 1;
  if((lpFilter&FILTER_CLASSNAME_MATCH)==0){
   GetClassName(hwnd,t1buf1,300);
   if(lstrcmp(t1buf1,TOOLTIPS_CLASS)==0)return 1;
  }
 }
 if(fwinf==0)goto addnew;
 pwinf=fwinf;
 while(pwinf!=0 && pwinf->cbSize==sizeof(WINDOW_INFO)){
  if(pwinf->hWnd==hwnd){bExist=1; goto skip_addnew;}
  pwinf=pwinf->NextEntry;
 }
 // Add New
addnew:
 bExist=0;
 HICON hIcon; hIcon=0;
 if((pwinf=(WINDOW_INFO*)LocalAlloc(LPTR,sizeof(WINDOW_INFO)))==0)return 0;
 if(fwinf==0)fwinf=pwinf;
 else winlastinf->NextEntry=pwinf;
 pwinf->PrevEntry=winlastinf;
 winlastinf=pwinf;
 pwinf->hWnd=hwnd;
 pwinf->hParent=hOwner;
 pwinf->cbSize=sizeof(WINDOW_INFO);
 if(SendMessageTimeout(hwnd,WM_GETICON,ICON_SMALL,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,100,(DWORD*)&hIcon)==FALSE){
  hIcon=(HICON)GetClassLong(hwnd,GCL_HICONSM);
 }
 if(hIcon==0){
  if(SendMessageTimeout(hwnd,WM_GETICON,ICON_BIG,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,100,(DWORD*)&hIcon)==FALSE)hIcon=NULL;
 }
 if(hIcon==0)hIcon=(HICON)GetClassLong(hwnd,GCL_HICON);
 pwinf->hIcon=hIcon;
 //if(Winclnumber[2]){//Class Name
  if(GetClassName(hwnd,t1buf1,300)!=0)pwinf->ClassName=AllocateAndCopy(0,t1buf1);
 //}
 // Update Info
skip_addnew:
 pwinf->bExist=1;
 if(Winclnumber[1]){
  if(CompareAndReAllocateA(&pwinf->Title,t1buf4))UpdWin[1]=1;
 }
 if(bExist && progSetts.CheckState[19]==0)goto skip_UpdProps;
 if(Winclnumber[3]){//window RECT
  GetWindowRect(hwnd,&rect);
  RECTtoString(&rect,(char*)t1buf4);
  if(CompareAndReAllocateA(&pwinf->Coordinates,t1buf4))UpdWin[3]=1;
 }
 if(Winclnumber[4]){//Style
  if(dwStyle!=pwinf->Style){
   UpdWin[4]=1;
  }
 }
 if(Winclnumber[5]){//ExStyle
  if((n=GetWindowLong(hwnd,GWL_EXSTYLE))!=pwinf->StyleEx){
   pwinf->StyleEx=n; UpdWin[5]=1;
  }
 }
skip_UpdProps:
 pwinf->Style=dwStyle;
 if(Winclnumber[6] && hOwner==0){//Status
  if(user32Functions.IsHungAppWindow!=0 && user32Functions.IsHungAppWindow(hwnd)){
   Sleep(100);
   if(user32Functions.IsHungAppWindow(hwnd))n=1;
   else goto runnng;
  }
  else {
 runnng:
   n=2;
  }
  if(pwinf->NotResponding!=(BYTE)n){
   pwinf->NotResponding=(BYTE)n; UpdWin[5]=1;
  }
 }
 return 1;
}

void DecreaseTreeDepth(HWND hWnd){WINDOW_INFO *pwinf; LVITEM lvi; LV_FINDINFO lvf;
 pwinf=fwinf;
 while(pwinf!=0 && pwinf->cbSize==sizeof(WINDOW_INFO)){
  if(pwinf->hParent==hWnd && pwinf->bExist && pwinf->cbSize==sizeof(WINDOW_INFO)){
   lvf.flags=LVFI_PARAM; lvf.lParam=(DWORD)pwinf;
   if((lvi.iItem=SendMessage(WindowsList,LVM_FINDITEM,-1,(LPARAM)&lvf))==(DWORD)-1)goto goNext;//item is hidden, so skip it.
   lvi.mask=LVIF_INDENT; lvi.iSubItem=0;
   if(!SendMessage(WindowsList,LVM_GETITEM,0,(LPARAM)&lvi))goto goNext;
   if(lvi.iIndent>0){lvi.iIndent--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);}
   DecreaseTreeDepth(pwinf->hWnd);
  }
 goNext:
  pwinf=pwinf->NextEntry;
 }
}

void DeleteItem_WindowsList(int iItem,WINDOW_INFO *pwinf,bool bLvnDelete){LV_ITEM lvi; WINDOW_INFO *ppwinf; HWND hWnd;
 if(IsVALID_WINDOW_INFO(pwinf)){
  if(!bLvnDelete){
   lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=iItem; lvi.iSubItem=0;
   SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
  }
  if(IsVALID_WINDOW_INFO((ppwinf=pwinf->NextEntry)))ppwinf->PrevEntry=pwinf->PrevEntry;
  if(IsVALID_WINDOW_INFO((ppwinf=pwinf->PrevEntry)))ppwinf->NextEntry=pwinf->NextEntry;
  if(pwinf==fwinf)fwinf=pwinf->NextEntry;
  else if(pwinf==winlastinf)winlastinf=ppwinf;
  pwinf->cbSize=0;
  LocalFree(pwinf->Title);
  LocalFree(pwinf->ClassName);
  LocalFree(pwinf->Coordinates);
  hWnd=pwinf->hWnd;
  LocalFree(pwinf);
 }
 if(!bLvnDelete){
  SendMessage(WindowsList,LVM_DELETEITEM,iItem,(LPARAM)&lvi);
  DecreaseTreeDepth(hWnd);
 }
}

void DeleteAllItems_WindowsList(){WINDOW_INFO *pwinf; DWORD n; LV_FINDINFO lvf;
 pwinf=fwinf; lvf.flags=LVFI_PARAM;
 while(IsVALID_WINDOW_INFO(pwinf)){
  lvf.lParam=(LPARAM)pwinf;
  pwinf=pwinf->NextEntry;
  if((n=SendMessage(WindowsList,LVM_FINDITEM,-1,(LPARAM)&lvf))!=0xFFFFFFFF){
   DeleteItem_WindowsList(n,(WINDOW_INFO*)(lvf.lParam),0);
  }
 }
 winlastinf=fwinf=0;
}

////////////////////////// Update WindowsInfo View /////////////////////////////
void UpdateWindowsInfoView(){WINDOW_INFO *pwinf; int ds,totalItems; bool exist; DWORD n; LVITEM lvi; LV_FINDINFO lvf; int dwRetries=0; bool bRetry=0;
updateinfoview:
pwinf=fwinf; lvf.flags=LVFI_PARAM;
ds=progSetts.WinSortd.CurSubItem; //if(ds!=0xFFFFFFFF)
ds++; totalItems=SendMessage(WindowsList,LVM_GETITEMCOUNT,0,0);
while(pwinf!=0){
 if(bPaneDestroy)return;
 if(pwinf->cbSize!=sizeof(WINDOW_INFO))break;
 if(pwinf->bExist==0)goto goNext;
 lvf.lParam=(DWORD)pwinf; n=SendMessage(WindowsList,LVM_FINDITEM,-1,(LPARAM)&lvf);
 if(n!=(DWORD)-1){lvi.iItem=n;exist=1;if(!bRetry)goto update_info; else goto goNext;}
 exist=0;
 if(progSetts.WinSortd.SortDirection==2){
  lvi.iItem=totalItems;
  goto ins_item;
 }
 lvi.mask=LVIF_PARAM;
 for(lvi.iItem=0;lvi.iItem<totalItems;lvi.iItem++){
  lvi.iSubItem=0;
  SendMessage(WindowsList,LVM_GETITEM,0,(LPARAM)&lvi);
  if(WindowsListCompareFunc(lvi.lParam,(DWORD)pwinf,&(progSetts.WinSortd))>0){
   goto ins_item;
  }
 }
 ins_item:
 lvi.lParam=(DWORD)pwinf; lvi.mask=LVIF_PARAM|LVIF_INDENT; lvi.iSubItem=0;
 lvi.iIndent=0;
 // determine childs tree depth
 WINDOW_INFO *pwinf2; HWND hParent; bool bParentVisible;
 if(pwinf->hParent!=0 && progSetts.WinSortd.SortDirection==2){
  bParentVisible=1;
  hParent=pwinf->hParent;
  find_parentwin:
  pwinf2=fwinf;
  while(pwinf2!=0 && pwinf2->cbSize==sizeof(WINDOW_INFO)){
   if(pwinf2->hWnd==hParent){
    lvf.lParam=(DWORD)pwinf2;
    if((n=SendMessage(WindowsList,LVM_FINDITEM,-1,(LPARAM)&lvf))==(DWORD)-1){//Parent item is hidden, so this too.
     lvi.iIndent++;
     if(dwRetries<lvi.iIndent)dwRetries=lvi.iIndent;
     bParentVisible=0;
     //goto goNext;
    }
    else if(bParentVisible){
     if(lvi.iIndent==0){lvi.iItem=n; lvi.iItem++;}
     lvi.iIndent++;
    }
    if(pwinf2->hParent==0)break;//Last parent found. continue insert item...
    hParent=pwinf2->hParent;
    goto find_parentwin;
   }
   pwinf2=pwinf2->NextEntry;
  }
  if(!bParentVisible)goto goNext;//Parent item is hidden, so this too.
 }
 //insert item
 lvi.iItem=SendMessage(WindowsList,LVM_INSERTITEM,0,(LPARAM)&lvi);
 totalItems++;
 lvi.mask=LVIF_IMAGE; lvi.iSubItem=0;
 if(pwinf->hIcon!=0)lvi.iImage=ImageList_AddIcon(himgl,pwinf->hIcon);
 else {
  lvi.iImage=((pwinf->Style&WS_VISIBLE)==WS_VISIBLE);
 }
 SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 update_info:
 lvi.mask=LVIF_TEXT; lvi.pszText=t1buf4;
 if((!exist || UpdWin[0]) && Winclnumber[0]){
  HexToString((DWORD)(pwinf->hWnd),t1buf4);
  lvi.iSubItem=Winclnumber[0];lvi.iSubItem--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 if((!exist || UpdWin[4]) && Winclnumber[4]){
  HexToString(pwinf->Style,t1buf4);
  lvi.iSubItem=Winclnumber[4];lvi.iSubItem--;
  SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
  if(UpdWin[4] && pwinf->hIcon==0){
   n=((pwinf->Style&WS_VISIBLE)==WS_VISIBLE);
   if((bool)n!=pwinf->bWasVisible){
    lvi.mask=LVIF_IMAGE;
    lvi.iImage=n;
    SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
    lvi.mask=LVIF_TEXT;
   }
   pwinf->bWasVisible=n;
  }
 }
 if((!exist || UpdWin[5]) && Winclnumber[5]){
  HexToString(pwinf->StyleEx,t1buf4);
  lvi.iSubItem=Winclnumber[5];lvi.iSubItem--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 lvi.pszText=LPSTR_TEXTCALLBACK;
 if((!exist || UpdWin[1]) && Winclnumber[1]){
  lvi.iSubItem=Winclnumber[1];lvi.iSubItem--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 if((!exist || UpdWin[2]) && Winclnumber[2]){
  lvi.iSubItem=Winclnumber[2];lvi.iSubItem--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 if((!exist || UpdWin[3]) && Winclnumber[3]){
  lvi.iSubItem=Winclnumber[3];lvi.iSubItem--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 if((!exist || UpdWin[6]) && Winclnumber[6]){
  lvi.iSubItem=Winclnumber[6];lvi.iSubItem--; SendMessage(WindowsList,LVM_SETITEM,0,(LPARAM)&lvi);
 }
 goNext:
 pwinf=pwinf->NextEntry;
}
if((UpdWin[1] || UpdWin[6]) && progSetts.WinSortd.SortDirection!=2){
 if((ds==Winclnumber[1] && UpdWin[1]) || (ds==Winclnumber[6] && UpdWin[6])){
  SendMessage(WindowsList,LVM_SORTITEMS,(WPARAM)&(progSetts.WinSortd),(LPARAM)WindowsListCompareFunc);
 }
}
if(dwRetries>0){dwRetries--;bRetry=1; goto updateinfoview;}
copystring(t1buf1,smemTable->Windows_); ltoa(SendMessage(WindowsList,LVM_GETITEMCOUNT,0,0),t1buf1+lstrlen(t1buf1),10);
SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)t1buf1);
WINDOW_INFO *ppwinf; pwinf=fwinf;
while(pwinf!=0 && pwinf->cbSize==sizeof(WINDOW_INFO)){
 if(pwinf->bExist==0){
  ppwinf=pwinf->NextEntry;
  lvf.lParam=(DWORD)pwinf;
  if((n=SendMessage(WindowsList,LVM_FINDITEM,-1,(LPARAM)&lvf))!=0xFFFFFFFF){
   DeleteItem_WindowsList(n,pwinf,0);
  }
  if(ppwinf==0)break;
  pwinf=ppwinf;
  continue;
 }
 pwinf=pwinf->NextEntry;
}}

/////////////////////////////// Show Windows ///////////////////////////////////
void UpdateEnumWindows(){WINDOW_INFO *pwinf;
 pwinf=fwinf; while(IsVALID_WINDOW_INFO(pwinf)){pwinf->bExist=0; pwinf=pwinf->NextEntry;}
 EnumWindows(AllWindowsList,progSetts.AWFtr);
 UpdateWindowsInfoView();
 for(int i=0;i<WINCOLUMNS_COUNT;i++)UpdWin[i]=0;
}

//////////////////////////////// Resize Window /////////////////////////////////
void resizeEnumWindowsWin(LONG newsz,LONG newwd){HDWP hdwp,hdwpcur; int hplus,wplus; HWND htmp; POINT pt;
 hplus=newsz; hplus-=initHg;
 wplus=newwd; wplus-=initWd;
 hdwp=BeginDeferWindowPos(3);
 if(hdwp==0)return;
 hdwpcur=hdwp;
 DeferWindowPos(hdwpcur,WindowsList,0,0,0,listsz.right+wplus,listsz.bottom+hplus,SWP_NOZORDER|SWP_NOMOVE);
 htmp=GetDlgItem(windows_win,202);
 if(htmp!=NULL){
  pt.x=newwd; pt.x-=sz3[1].x; pt.x-=5;
  pt.y=sz3[1].y; pt.y+=hplus;
  hdwpcur=DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 }
 htmp=GetDlgItem(windows_win,201);
 if(htmp!=NULL){
  pt.x=newwd; pt.x-=sz3[0].x; pt.x-=sz3[1].x; pt.x-=20;
  pt.y=sz3[0].y; pt.y+=hplus;
  DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 }
 EndDeferWindowPos(hdwp);
}

//////////////////////////////////// On Init ///////////////////////////////////
void OnInit_EnumWindowsWin(){int i; DWORD n,cnt;
 himgl=ImageList_Create(16,16,ILC_COLORDDB,1,2);
 ImageList_SetBkColor(himgl,0x00ffffff);
 ImageList_AddIcon(himgl,LoadIcon(gInst,(char*)25));
 ImageList_AddIcon(himgl,LoadIcon(gInst,(char*)24));
 listsz.left=3;
 listsz.top=26;
 listsz.right=pHdr->rcDisplay.right-10;
 listsz.bottom=pHdr->rcDisplay.bottom-59;
 WindowsList=GetDlgItem(windows_win,101);
 SetWindowPos(WindowsList,0,listsz.left,listsz.top,listsz.right,listsz.bottom,SWP_NOZORDER);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=LVCFMT_LEFT;
 cnt=0;
 for(i=0;i<WINCOLUMNS_COUNT;i++){
  for(int t=0;t<WINCOLUMNS_COUNT;t++){
   if(WinColumnsCreateOrder[t]==i+1 && smemTable->WinColumnTitle[t][0]){
    lvcol.fmt=LVCFMT_LEFT; lvcol.pszText=smemTable->WinColumnTitle[t];
    lvcol.cx=WinColumnWidth[t]; lvcol.iSubItem=i;
    Winclnumber[t]=SendMessage(WindowsList,LVM_INSERTCOLUMN,(WPARAM)i,(LPARAM)&lvcol); Winclnumber[t]++;
    if(Winclnumber[t]>0)cnt++;
    break;
   }
  }
 }
 n=LVS_MY_EXTENDED_STYLE; if(progSetts.CheckState[18]==MFS_CHECKED)n|=LVS_EX_GRIDLINES;
 SendMessage(WindowsList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
 SendMessage(WindowsList,LVM_SETIMAGELIST,LVSIL_SMALL,(LPARAM)himgl);
 if(bValidWinOrderArray)SendMessage(WindowsList,LVM_SETCOLUMNORDERARRAY,(WPARAM)cnt,(LPARAM)&WinOrderArray);
 CheckMarkMenuItems();
 if(progSetts.WinSortd.CurSubItem!=(DWORD)-1)SetSortArrowIcon(WindowsList,(DWORD)-1,progSetts.WinSortd.CurSubItem,progSetts.WinSortd.SortDirection);
}

////////////////////////////////// Close Window ////////////////////////////////
void CloseWin(BYTE Type){LV_ITEM lvi; HWND hWnd; int itemSel;
 if((itemSel=SendMessage(WindowsList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)return;
 lvi.iItem=itemSel; lvi.mask=LVIF_PARAM; lvi.iSubItem=0;
 if(SendMessage(WindowsList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)return;
 hWnd=((WINDOW_INFO*)lvi.lParam)->hWnd;
 if(user32Functions.EndTask!=NULL)user32Functions.EndTask(hWnd,0,Type);
 else SendMessageTimeout(hWnd,WM_CLOSE,0,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,100,NULL);
}

void EnableWindowCntxtMenuItems(HMENU hmenu,int iItem){HWND hwin; DWORD dwf,dwf2,n; DWORD_PTR dwp; LV_ITEM lvi; bool bbl; MENUITEMINFO mii;
 if(hmenu!=0)goto change_menu;
 if((hmenu=GetSubMenu(main_menu,2))!=0){
change_menu:
  lvi.mask=LVIF_PARAM; lvi.iItem=iItem;
  if(SendMessage(WindowsList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)return;
  hwin=((WINDOW_INFO*)(lvi.lParam))->hWnd; bbl=0;
  dwp=GetWindowLongPtr(hwin,GWL_STYLE);
  //minimize-item
  if((dwp&WS_MINIMIZE)==WS_MINIMIZE){dwf=MF_BYCOMMAND|MF_GRAYED; bbl=1;}
  else dwf=MF_BYCOMMAND|MF_ENABLED;
  EnableMenuItem(hmenu,40033,dwf);
  //maximize-item
  if((dwp&WS_MAXIMIZE)==WS_MAXIMIZE){dwf=MF_BYCOMMAND|MF_GRAYED; bbl=1;}
  else dwf=MF_BYCOMMAND|MF_ENABLED;
  EnableMenuItem(hmenu,40034,dwf);
  //restore-item
  if(!bbl)dwf=MF_BYCOMMAND|MF_GRAYED;
  else dwf=MF_BYCOMMAND|MF_ENABLED;
  EnableMenuItem(hmenu,40038,dwf);
  //hide-item
  if((dwp&WS_VISIBLE)==WS_VISIBLE){dwf=MF_BYCOMMAND|MF_GRAYED; dwf2=MF_BYCOMMAND|MF_ENABLED;}
  else {dwf=MF_BYCOMMAND|MF_ENABLED; dwf2=MF_BYCOMMAND|MF_GRAYED;}
  EnableMenuItem(hmenu,40039,dwf2);
  EnableMenuItem(hmenu,40040,dwf);
  // Extended Style
  dwp=GetWindowLongPtr(hwin,GWL_EXSTYLE);
  //alwaysOnTop-item
  mii.cbSize=sizeof(MENUITEMINFO);
  mii.fMask=MIIM_TYPE;
  mii.fType=MFT_STRING;
  if((dwp&WS_EX_TOPMOST)==WS_EX_TOPMOST){mii.dwTypeData=smemTable->DisableAlwaysOnTop; n=23;}
  else {mii.dwTypeData=smemTable->EnableAlwaysOnTop; n=24;}
  if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
   if((mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(n))!=0)mii.fMask|=MIIM_DATA;
  }
  SetMenuItemInfo(main_menu,40032,0,&mii);
 }
}

///////////////////////////////// WindowsWin_Proc //////////////////////////////
BOOL CALLBACK WindowsWin_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){LV_ITEM lvi; int itemSel; WINDOW_INFO *pwinf;
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
  if(Message==WM_INITMENUPOPUP || Message==WM_UNINITMENUPOPUP || Message==WM_MEASUREITEM || Message==WM_DRAWITEM || Message==WM_ENTERMENULOOP || Message==WM_EXITMENULOOP){
   ModernMenu_DrawMenuProc(hDlg,Message,wParam,lParam);
  }
 }
 switch(Message){
  case WM_INITDIALOG:
  windows_win=hDlg;
  initHg=pHdr->rcDisplay.bottom;
  initWd=pHdr->rcDisplay.right-4;
  HWND htmp; POINT pt; int hpls; hpls=initHg-132;
  htmp=GetDlgItem(windows_win,201);
  if(htmp!=NULL){
   GetWindowRect(htmp,&clt); pt.y=clt.top; pt.x=clt.left;
   ScreenToClient(windows_win,&pt);
   sz3[0].y=pt.y+hpls; sz3[0].x=clt.right;sz3[0].x-=clt.left;
  }
  htmp=GetDlgItem(windows_win,202);
  if(htmp!=NULL){
   GetWindowRect(htmp,&clt); pt.y=clt.top; pt.x=clt.left;
   ScreenToClient(windows_win,&pt);
   sz3[1].y=pt.y+hpls; sz3[1].x=clt.right; sz3[1].x-=clt.left;
  }
  WindowFillText(hDlg,dlgTAT->dlg17);
  MainTab_OnChildDialogInit(hDlg); pHdr->hwndDisplay=hDlg;
  OnInit_EnumWindowsWin();
  SetFocus(WindowsList);
  CheckDlgButton(hDlg,103,(progSetts.AWFtr&FILTER_IF_VISIBLE)?MFS_CHECKED:MFS_UNCHECKED);
  CheckDlgButton(hDlg,104,(progSetts.AWFtr&FILTER_IF_CAPTION_EXIST)?MFS_CHECKED:MFS_UNCHECKED);
  CheckDlgButton(hDlg,105,(progSetts.AWFtr&FILTER_IF_NOPARENT)?MFS_CHECKED:MFS_UNCHECKED);
  for(hpls=0;hpls<WINCOLUMNS_COUNT;hpls++)UpdWin[hpls]=0;
  bPaneDestroy=0;// ExecuteAddr=(DWORD)UpdateEnumWindows;
  //ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0xFE);
  ControlUpdateThread(UPDATETHREAD_UPDATE,0xFF,0);
  break;
 case WM_DESTROY:
  DeleteAllItems_WindowsList();
  if(ImageList_Destroy(himgl))himgl=NULL;
  SaveColumns_Windows();
  if(WindowsList!=0){DestroyWindow(WindowsList); WindowsList=0;}
  DeleteMenu(main_menu,47101,MF_BYCOMMAND);
  DeleteMenu(main_menu,47102,MF_BYCOMMAND);
  DeleteMenu(main_menu,30018,MF_BYCOMMAND);
  SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)0);
  windows_win=NULL;
  break;
 case WM_COMMAND: wID=LOWORD(wParam);
  switch(wID){
   case 103:
    if(IsDlgButtonChecked(windows_win,103)==BST_CHECKED)progSetts.AWFtr|=FILTER_IF_VISIBLE;
    else progSetts.AWFtr&=~FILTER_IF_VISIBLE;
    goto showenumwins;
   case 104:
    if(IsDlgButtonChecked(windows_win,104)==BST_CHECKED)progSetts.AWFtr|=FILTER_IF_CAPTION_EXIST;
    else progSetts.AWFtr&=~FILTER_IF_CAPTION_EXIST;
    goto showenumwins;
   case 105:
    if(IsDlgButtonChecked(windows_win,105)==BST_CHECKED)progSetts.AWFtr|=FILTER_IF_NOPARENT;
    else progSetts.AWFtr&=~FILTER_IF_NOPARENT;
   case 50010:
 showenumwins:
    ControlUpdateThread(UPDATETHREAD_UPDATE,0xFF,0);
    break;
   case 30018:
    n=LVS_MY_EXTENDED_STYLE;
    checkMenuItem(18);
    if(progSetts.CheckState[18]==MFS_CHECKED)n|=LVS_EX_GRIDLINES;
    SendMessage(WindowsList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
    break;
   case 40030:
    CreateColumnSelDlg(2);
    break;
   case 201:
    wID=40035;
    goto rmenuclk;
   case 202:
    SendMessage(main_win,WM_COMMAND,50001,0);
    break;
  }
  if(wID>=40031 && wID<=40046){
 rmenuclk:
   if((itemSel=SendMessage(WindowsList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.iItem=itemSel; lvi.mask=LVIF_PARAM; lvi.iSubItem=0;
   if(SendMessage(WindowsList,LVM_GETITEM,itemSel,(LPARAM)&lvi)==FALSE)break;
   if(!IsVALID_WINDOW_INFO(((WINDOW_INFO*)lvi.lParam)))break;
   Window_Action(((WINDOW_INFO*)lvi.lParam)->hWnd,((WINDOW_INFO*)lvi.lParam),wID-40030);
  }
  break;
 case WM_SIZE:
  resizeEnumWindowsWin(HIWORD(lParam),LOWORD(lParam));
  break;
 case WM_NOTIFY:
  DWORD dCode; dCode=((NMHDR*)lParam)->code;
  if(dCode==NM_RCLICK || dCode==NM_CLICK){
   itemSel=((NMLISTVIEW*)lParam)->iItem;
   if(((NMHDR*)lParam)->hwndFrom==WindowsList){
    EnableMainMenuItem(47101,MF_BYCOMMAND|(((itemSel==-1)?MF_GRAYED:MF_ENABLED)));
   }
   if(dCode!=NM_RCLICK)break;
   POINT curpos; GetCursorPos(&curpos);
   if(CheckHeaderClick(WindowsList,((NMHDR*)lParam)->hwndFrom,&curpos)==1 || itemSel==-1)break;
   else {
    HMENU hMenu;
    if((hMenu=GetSubMenu(main_menu,2))==0)break;
    EnableWindowCntxtMenuItems(hMenu,itemSel);
    if(progSetts.CheckState[36])itemSel=40037;
    else itemSel=40031;
    SetMenuDefaultItem(hMenu,itemSel,FALSE);
    TrackPopupMenuEx(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos.x,curpos.y,windows_win,NULL);
    SetMenuDefaultItem(hMenu,-1,FALSE);
   }
   break;
  }
  if(((NMHDR*)lParam)->hwndFrom!=WindowsList)break;
  if(dCode==NM_DBLCLK){
   if(progSetts.CheckState[36])wID=40037;
   else wID=40031;
   goto rmenuclk;
  }
  else if(dCode==LVN_KEYDOWN && ((LPNMLVKEYDOWN)lParam)->wVKey==VK_DELETE)DialogBox(gInst,(char*)19,main_win,(DLGPROC)CloseWin_Proc);
  else if(dCode==LVN_COLUMNCLICK){
   progSetts.WinSortd.hFrom=WindowsList;
   progSetts.WinSortd.bRestoreSort=1;
   ListViewColumnSortClick((NM_LISTVIEW*)lParam,&(progSetts.WinSortd),(DWORD)WindowsListCompareFunc);
  }
  else if(dCode==LVN_GETDISPINFO){
   NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam;
   if((pwinf=(WINDOW_INFO*)nvd->item.lParam)==0){
    nvd->item.pszText=0; break;
   }
   if(nvd->item.mask&LVIF_TEXT){
    int iSub; iSub=nvd->item.iSubItem; iSub++;
    if(iSub==Winclnumber[1])nvd->item.pszText=pwinf->Title;
    else if(iSub==Winclnumber[2])nvd->item.pszText=pwinf->ClassName;
    else if(iSub==Winclnumber[3])nvd->item.pszText=pwinf->Coordinates;
    else if(iSub==Winclnumber[6]){
     if(pwinf->NotResponding==1)nvd->item.pszText=smemTable->NotResponding;
     else if(pwinf->NotResponding==2) nvd->item.pszText=smemTable->Running;
    }
    if(nvd->item.pszText==0)nvd->item.pszText="";
   }
  }
  else if(dCode==LVN_DELETEITEM){
   lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=((NM_LISTVIEW*)lParam)->iItem;
   if(SendMessage(WindowsList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   pwinf=(WINDOW_INFO*)lvi.lParam;
   if(pwinf->cbSize!=sizeof(WINDOW_INFO)){winlastinf=fwinf=0; break;}
   DeleteItem_WindowsList(lvi.iItem,(WINDOW_INFO*)lvi.lParam,1);
  }
  else if(dCode==LVN_ITEMCHANGED){
   NMLISTVIEW *nmlv; nmlv=(LPNMLISTVIEW)lParam; if(nmlv==0)break;
   if(nmlv->uChanged&LVIF_STATE && nmlv->uNewState&LVIS_SELECTED){
    itemSel=((NMLISTVIEW*)lParam)->iItem;
    EnableMainMenuItem(47101,MF_BYCOMMAND|(((itemSel==-1)?MF_GRAYED:MF_ENABLED)));
    if(itemSel!=-1)EnableWindowCntxtMenuItems(0,itemSel);
   }
  }
  break;
 }
 return 0;
}

BOOL CALLBACK CloseWin_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
 switch(Message){
  case WM_INITDIALOG:
   SetWindowText(hDlg,smemTable->CaptionEndTask);
   WindowFillText(hDlg,dlgTAT->dlg19);
   MessageBeep(MB_ICONQUESTION);
   SetClassLong(hDlg,GCL_STYLE,DlgClassStyl|CS_DROPSHADOW);
   SendMessage(GetDlgItem(hDlg,104),STM_SETIMAGE,IMAGE_ICON,(LPARAM)LoadIcon(0,IDI_QUESTION)); return 1;
  case WM_CLOSE: EndDialog(hDlg,1);
  case WM_DESTROY: SetClassLong(hDlg,GCL_STYLE,DlgClassStyl); break;
  case WM_COMMAND:
   wID=LOWORD(wParam);
   if(wID==102 || wID==103){CloseWin((BYTE)(wID-102)); EndDialog(hDlg,1);}
   else if(wID==2)SendMessage(hDlg,WM_CLOSE,0,0);
   break;
 }
 return 0;
}

