//#include <aclui.h>
//ISecurityInformation *pisf;

#define DLGHDR_SIGN_SETTS 0x1b4c2a5f

#define TCI_General 0
#define TCI_TaskManReplace 1
#define TCI_Processes 2
#define TCI_Windows 3
#define TCI_Performance 4
#define TCI_Language 5
#define TCI_Columns 6

#define SETTSWIN_COLUMNSET_ID TCI_Columns

typedef struct tag_dlghdr5{
 DWORD Signature;
 HWND hMain;
 HWND hwndDisplay;
 RECT rcDisplay;
 DLGTEMPLATE *apRes[7];
 DLGPROC *dlgProC[7];
 char *language_txt;
 LNGITEM_INFO *newLng;
 HBITMAP hbmp;
 HFONT hNormalFont;
 DWORD dwSel;
 DWORD SubSel;
} DLGHDR5;

#include "SelectColumns.cpp"

HBITMAP DrawTitleLine(HWND hwin,char *pText,HBITMAP *hbitmap){RECT rect; HBITMAP hbmp; HDC hdc; HPEN hpen; HFONT hFont;
 LOGFONT tabf; BITMAPINFO bmi; int x; double color,clrplus; bool reverse=0;
 hdc=CreateCompatibleDC(0);
 GetClientRect(hwin,&rect);
 if(hbitmap==0 || *hbitmap==0){
  RtlZeroMemory(&bmi,sizeof(BITMAPINFO));
  bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biHeight=rect.bottom;
  bmi.bmiHeader.biWidth=rect.right;
  bmi.bmiHeader.biPlanes=1; bmi.bmiHeader.biBitCount=24;
  hbmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,NULL,0,0);
  if(!hbmp)return 0;
  if(hbitmap)*hbitmap=hbmp;
 }
 else hbmp=*hbitmap;
 SelectObject(hdc,hbmp); SetStretchBltMode(hdc,HALFTONE); SetBkMode(hdc,TRANSPARENT);
 SetBkColor(hdc,0); SetTextColor(hdc,RGB(250,250,230));
 color=50; clrplus=(255-color)*2; clrplus/=rect.right;
 for(x=0;x<rect.right;x++){
  hpen=CreatePen(PS_SOLID,1,RGB(0,0,color)); DeleteObject(SelectObject(hdc,hpen));
  MoveToEx(hdc,x,0,0);
  LineTo(hdc,x,rect.bottom);
  if(color>=255)reverse=1; else if(color<=0)reverse=0;
  if(!reverse)color+=clrplus; else color-=clrplus;
  if(color>255)color=255;
 }
 DeleteObject(hpen);
 tabf.lfCharSet=RUSSIAN_CHARSET; tabf.lfHeight=20; tabf.lfWidth=0; tabf.lfWeight=FW_BOLD; tabf.lfOutPrecision=OUT_DEFAULT_PRECIS;
 copystring(tabf.lfFaceName,MSsansSerif_txt); tabf.lfEscapement=0; tabf.lfOrientation=0; tabf.lfUnderline=0; tabf.lfStrikeOut=0; tabf.lfItalic=0;
 hFont=CreateFontIndirect(&tabf);
 DeleteObject(SelectObject(hdc,hFont));
 if(pText)DrawTextEx(hdc,pText,getstrlen(pText),&rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE,0);
 DeleteObject(hFont);
 DeleteDC(hdc);
 return hbmp;
}

void EnableCheckBoxes(HWND hDlg){bool bEnable=tempSetts.CheckState[24]; int i=131;
 again:
 if(i==133 && bEnable && tempSetts.CheckState[31]!=SCROLL_TO_LASTRUNNED)bEnable=0;
 EnableWindow(GetDlgItem(hDlg,i),bEnable);
 if(i<133){i++;goto again;}
}

void Setts_TaskManProc_CkeckButtons(HWND hDlg, BOOL bEnable){DWORD n;
 if(bEnable)tempSetts.CheckState[21]|=SETTS_REPLACETASKMAN_ENABLE;
 else tempSetts.CheckState[21]&=~SETTS_REPLACETASKMAN_ENABLE;
 EnableWindow(GetDlgItem(hDlg,503),bEnable);
 EnableWindow(GetDlgItem(hDlg,504),bEnable);
 EnableWindow(GetDlgItem(hDlg,505),bEnable);
 CheckDlgButton(hDlg,121,(bEnable)?BST_CHECKED:BST_UNCHECKED);
 if((tempSetts.CheckState[21]&SETTS_REPLACETASKMAN_SHOWLOGONALWAYS)==SETTS_REPLACETASKMAN_SHOWLOGONALWAYS)n=504;
 else if((tempSetts.CheckState[21]&SETTS_REPLACETASKMAN_SHOWLOGONIFNONADMIN)==SETTS_REPLACETASKMAN_SHOWLOGONIFNONADMIN)n=505;
 else n=503;
 CheckRadioButton(hDlg,503,505,n);
}

void SwitchRightDisplay(HWND hDlg,DWORD dwNew){DLGHDR5 *pHdr; char *ptitle; DLGITEM_TEXT *dgit=0; HWND hwin; DWORD bl;
 if((pHdr=(DLGHDR5*)GetWindowLongPtr(hDlg,GWLP_USERDATA))==0)return;
 if(pHdr->hwndDisplay!=NULL)DestroyWindow(pHdr->hwndDisplay);
 pHdr->hwndDisplay=hwin=CreateDialogIndirectParam(gInst,pHdr->apRes[dwNew],hDlg,(DLGPROC)pHdr->dlgProC[dwNew],(DWORD)pHdr);
 pHdr->dwSel=dwNew;
 SetWindowPos(hwin,HWND_TOP,pHdr->rcDisplay.left,pHdr->rcDisplay.top,pHdr->rcDisplay.right,pHdr->rcDisplay.bottom,0);
 if(dwNew!=TCI_Language && dwNew!=TCI_Columns){
  for(int i=0;i<CHECKstateCOUNT;i++){
   if(i>=7 && i<=10){bl=tempSetts.CheckState[7]; bl>>=i-7; if(bl!=1)bl=0;}
   else if(i==14 || i==15){bl=tempSetts.CheckState[14]; bl>>=i-14; if(bl!=1)bl=0;}
   else if(i==24 && dwNew==TCI_Processes){
    EnableCheckBoxes(hwin); goto getstate;
   }
   else if(i==21 && dwNew==TCI_TaskManReplace)continue;
   else if(i==30 && dwNew==TCI_General){
    HWND hcombo;
    if((hcombo=GetDlgItem(hwin,130))!=0){
     EnableWindow(hcombo,1);
     SendMessage(hcombo,CB_RESETCONTENT,0,0);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->ViewFromLastTime);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->Processes);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->Windows);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->Services);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->Drivers);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->Startup);
     SendMessage(hcombo,CB_ADDSTRING,0,(LPARAM)smemTable->Performance);
     SendMessage(hcombo,CB_SETCURSEL,(WPARAM)(BYTE)(tempSetts.CheckState[30]+1),0);
    }
   }
   else if((i==31 || i==32) && dwNew==TCI_Processes){
    bl=tempSetts.CheckState[31]; bl>>=i-31; if(bl!=1)bl=0;
   }
   else if(i==33 && dwNew==TCI_Processes){bool bul;
    if(tempSetts.CheckState[31]==SCROLL_TO_LASTRUNNED && tempSetts.CheckState[24])bul=1;
    else bul=0;
    EnableWindow(GetDlgItem(hwin,133),bul);
    goto getstate;
   }
   else {
    getstate:
    bl=tempSetts.CheckState[i];
   }
   CheckDlgButton(hwin,100+i,bl);
  }
  if(dwNew==TCI_General){
   if(tempSetts.MenuStyle==MENU_STYLE_DEFAULT)bl=501;
   else bl=502;
   CheckRadioButton(hwin,501,502,bl);
  }
 }
 ShowWindow(pHdr->hwndDisplay,SW_SHOW);
 if(dwNew==TCI_General){ptitle=smemTable->Generals;dgit=dlgTAT->dlg27;}
 else if(dwNew==TCI_TaskManReplace){ptitle=smemTable->TaskManager;dgit=dlgTAT->dlg35;}
 else if(dwNew==TCI_Processes){ptitle=smemTable->Processes;dgit=dlgTAT->dlg28;}
 else if(dwNew==TCI_Windows){ptitle=smemTable->Windows;dgit=dlgTAT->dlg33;}
 else if(dwNew==TCI_Performance){ptitle=smemTable->Performance;dgit=dlgTAT->dlg29;}
 else if(dwNew==TCI_Language){ptitle=pHdr->language_txt;}//dgit=dlgTAT->dlg30;
 else if(dwNew==TCI_Columns){ptitle=smemTable->Columns;}//dgit=dlgTAT->dlg30;
 else ptitle=0;
 WindowFillText(hwin,dgit); hwin=GetDlgItem(hDlg,303);
 SendMessage(hwin,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)DrawTitleLine(hwin,ptitle,&(pHdr->hbmp)));
}

void SetBITparam(DWORD wID, DWORD id){
 tempSetts.CheckState[id]=tempSetts.CheckState[wID-100];
 wID-=100; wID-=id;
 tempSetts.CheckState[id]<<=wID;
}

BOOL CALLBACK Setts_CheckMarksProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
if(Message==WM_COMMAND){
 DWORD wID; wID=LOWORD(wParam);
 if(wID>=100 && wID<(100+CHECKstateCOUNT)){
  if(wID==130){
   wID=SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
   if(wID!=(DWORD)CB_ERR){tempSetts.CheckState[30]=(BYTE)wID;tempSetts.CheckState[30]--;}
  }
  else{
   tempSetts.CheckState[wID-100]=(BYTE)((IsDlgButtonChecked(hDlg,wID)==BST_CHECKED)?1:0);
   if(wID>=104 && wID<=106)SetBITparam(wID,4);
   else if(wID>=107 && wID<=110)SetBITparam(wID,7);
   else if(wID==114 || wID==115)SetBITparam(wID,14);
   else if(wID==124)EnableCheckBoxes(hDlg);
   else if(wID==131 || wID==132){
    SetBITparam(wID,31); bool bul;
    if(wID==132 && tempSetts.CheckState[31]==SCROLL_TO_LASTRUNNED)bul=1;
    else bul=0;
    EnableWindow(GetDlgItem(hDlg,133),bul);
   }
   else if(wID==135)tempSetts.CheckState[36]=0;
   else if(wID==136)tempSetts.CheckState[35]=0;
  }
 }
 else if(wID==501)tempSetts.MenuStyle=MENU_STYLE_DEFAULT;
 else if(wID==502)tempSetts.MenuStyle=MENU_STYLE_XNET2;
}
return 0;}

BOOL CALLBACK Setts_LanguageProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){DLGHDR5 *pHdr; HWND hlist; LV_ITEM lvi; LNGITEM_INFO *lnginf;
switch(Message){
 case WM_INITDIALOG:
  LV_COLUMN lvcol;
  if((hlist=GetDlgItem(hDlg,402))==0)break;
  SendMessage(hlist,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)(LVS_EX_FULLROWSELECT|LVS_EX_LABELTIP|LVS_EX_FLATSB));
  pHdr=(DLGHDR5*)lParam;
  lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
  lvcol.fmt=LVCFMT_LEFT;
  lvcol.cx=120; lvcol.iSubItem=0;
  if(pHdr)lvcol.pszText=pHdr->language_txt; else lvcol.pszText=0;
  SendMessage(hlist,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
  lvcol.cx=200; lvcol.pszText=Author_txt; lvcol.iSubItem=1;
  SendMessage(hlist,LVM_INSERTCOLUMN,(WPARAM)1,(LPARAM)&lvcol);
  EnumLngFiles(0,1);
  if(lngFirstInf==0)break;
  lnginf=lngFirstInf; lvi.iItem=0;
  readstruct:
   lvi.lParam=(DWORD)lnginf; lvi.iSubItem=0; lvi.mask=LVIF_TEXT|LVIF_PARAM;//|LVIF_IMAGE; lvi.iImage=1;
   if(lnginf->lngID==tempSetts.LanguageID){
    lvi.mask|=LVIF_STATE; lvi.state=LVIS_SELECTED;
    SetDlgItemText(hDlg,407,lnginf->LanguageInfo);
   }
   lvi.pszText=lnginf->LanguageInfo;//lnginf->fileName;
   SendMessage(hlist,LVM_INSERTITEM,0,(LPARAM)&lvi);
   lvi.iSubItem=1; lvi.pszText+=getstrlen(lvi.pszText); lvi.pszText++; lvi.mask=LVIF_TEXT;
   SendMessage(hlist,LVM_SETITEM,0,(LPARAM)&lvi);
   lvi.iItem++;
  if(lnginf->NextEntry!=0){lnginf=(LNGITEM_INFO*)(lnginf->NextEntry); goto readstruct;}
  break;
 case WM_NOTIFY:
  if(((NMHDR*)lParam)->code==NM_DBLCLK){
   if((pHdr=(DLGHDR5*)GetWindowLong(GetParent(hDlg),GWL_USERDATA))==0)break;
   if((hlist=GetDlgItem(pHdr->hwndDisplay,402))==0)break;
   if(((NMHDR*)lParam)->hwndFrom!=hlist)break;
   int isel; isel=GetItemSelected(hlist);
   lvi.iSubItem=0; lvi.iItem=isel; lvi.mask=LVIF_PARAM;
   if(SendMessage(hlist,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   lnginf=(LNGITEM_INFO*)lvi.lParam;
   if(lnginf->lngID==tempSetts.LanguageID)break;
   HWND hwin;
   tempSetts.LanguageID=lnginf->lngID;
   ShowWindow(GetDlgItem(hDlg,404),SW_SHOWNA);
   hwin=GetDlgItem(hDlg,405);
   ShowWindow(hwin,SW_SHOWNA); SetWindowText(hwin,lnginf->LanguageInfo);
   pHdr->newLng=lnginf;
  }
  break;
}
return 0;}

BOOL CALLBACK Setts_TaskManProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
 if(Message==WM_INITDIALOG){
  //// Get Current Task Manager Replace-Software
  HKEY hKey; LPTSTR pPath; DWORD nm;
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,taskmgrKey,0,KEY_QUERY_VALUE,&hKey)==ERROR_SUCCESS){
   nm=0;
   if(RegQueryValueEx(hKey,Debugger_txt,NULL,0,0,&nm)==ERROR_SUCCESS && nm>0){
    nm+=sizeof(TCHAR);
    pPath=(LPTSTR)LocalAlloc(LMEM_FIXED,nm);
    if(pPath!=0){
     if(RegQueryValueEx(hKey,Debugger_txt,NULL,0,(BYTE*)pPath,&nm)==ERROR_SUCCESS){
      SetDlgItemText(hDlg,702,pPath);
      ShowWindow(GetDlgItem(hDlg,701),SW_SHOW);
      ShowWindow(GetDlgItem(hDlg,702),SW_SHOW);
     }
     LocalFree(pPath);
    }
   }
   RegCloseKey(hKey);
  }
  ////
  Setts_TaskManProc_CkeckButtons(hDlg,(tempSetts.CheckState[21]&SETTS_REPLACETASKMAN_ENABLE)==SETTS_REPLACETASKMAN_ENABLE);
 }
 else if(Message==WM_COMMAND){
  DWORD wID; wID=LOWORD(wParam);
  if(wID==121){Setts_TaskManProc_CkeckButtons(hDlg,IsDlgButtonChecked(hDlg,wID)==BST_CHECKED); return 1;}
  else if(wID>=503 && wID<=505){
   tempSetts.CheckState[21]&=SETTS_REPLACETASKMAN_NOLOGONMASK;
   if(IsDlgButtonChecked(hDlg,wID)==BST_CHECKED){
    if(wID==504)tempSetts.CheckState[21]|=SETTS_REPLACETASKMAN_SHOWLOGONALWAYS;
    else if(wID==505)tempSetts.CheckState[21]|=SETTS_REPLACETASKMAN_SHOWLOGONIFNONADMIN;
   }
   return 1;
  }
 }
 return 0;
}

void OnInit_SettingsWin(HWND hDlg,DWORD lParam){HWND hTree; RECT wrect; HWND hwin; HFONT hFont;
DLGHDR5 *pHdr; pHdr=(DLGHDR5*)LocalAlloc(LPTR,sizeof(DLGHDR5)); if(pHdr==0)return;
pHdr->Signature=DLGHDR_SIGN_SETTS;
SetWindowLong(hDlg,GWL_USERDATA,(DWORD)pHdr);
pHdr->hMain=hDlg;
wrect.top=wrect.left=0;
wrect.right=525; wrect.bottom=454;
SetWindowPos(hDlg,HWND_TOPMOST,0,0,525,454,SWP_NOMOVE|SWP_NOZORDER);
SetWindowInCenter(hDlg,&wrect,main_win);
GetClientRect(hDlg,&pHdr->rcDisplay);
pHdr->rcDisplay.left=150; pHdr->rcDisplay.bottom-=4; pHdr->rcDisplay.bottom-=32;
hTree=CreateWindowEx(WS_EX_STATICEDGE|WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_NOPARENTNOTIFY,WC_TREEVIEW,0,WS_VISIBLE|WS_TABSTOP|WS_CHILD|TVS_HASLINES|TVS_HASBUTTONS|TVS_LINESATROOT|TVS_DISABLEDRAGDROP|TVS_SHOWSELALWAYS|TVS_TRACKSELECT,2,2,pHdr->rcDisplay.left,pHdr->rcDisplay.bottom,hDlg,(HMENU)301,gInst,NULL);
pHdr->rcDisplay.left+=6; pHdr->rcDisplay.right-=pHdr->rcDisplay.left; pHdr->rcDisplay.right-=2; pHdr->rcDisplay.bottom-=28;
CreateWindowEx(WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_NOPARENTNOTIFY,STATIC_txt,0,WS_VISIBLE|WS_CHILD|SS_ETCHEDFRAME,pHdr->rcDisplay.left,30,pHdr->rcDisplay.right,pHdr->rcDisplay.bottom,hDlg,(HMENU)302,gInst,NULL);
 CreateWindowEx(0,STATIC_txt,0,SS_BITMAP|SS_CENTERIMAGE|WS_CHILD|WS_VISIBLE,pHdr->rcDisplay.left,2,pHdr->rcDisplay.right,25,hDlg,(HMENU)303,gInst,NULL);
 wrect.left=pHdr->rcDisplay.left; wrect.left+=pHdr->rcDisplay.right; wrect.right--;
 pHdr->rcDisplay.left++; pHdr->rcDisplay.right-=3; pHdr->rcDisplay.bottom-=3; pHdr->rcDisplay.top=31;
pHdr->hNormalFont=hFont=GetNormalFont();
wrect.bottom=pHdr->rcDisplay.bottom; wrect.bottom+=pHdr->rcDisplay.top; wrect.bottom+=7;
hwin=CreateWindowEx(0,BUTTON_txt,smemTable->OK,BS_PUSHBUTTON|BS_TEXT|BS_VCENTER|BS_CENTER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,wrect.right-290,wrect.bottom,90,24,hDlg,(HMENU)IDOK,gInst,NULL);
SendMessage(hwin,WM_SETFONT,(WPARAM)hFont,(LPARAM)TRUE);
hwin=CreateWindowEx(0,BUTTON_txt,smemTable->Cancel,BS_PUSHBUTTON|BS_TEXT|BS_VCENTER|BS_CENTER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,wrect.right-190,wrect.bottom,90,24,hDlg,(HMENU)IDCANCEL,gInst,NULL);
SendMessage(hwin,WM_SETFONT,(WPARAM)hFont,(LPARAM)TRUE);
hwin=CreateWindowEx(0,BUTTON_txt,smemTable->Help,BS_PUSHBUTTON|BS_TEXT|BS_VCENTER|BS_CENTER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,wrect.left-90,wrect.bottom,90,24,hDlg,(HMENU)IDHELP,gInst,NULL);
SendMessage(hwin,WM_SETFONT,(WPARAM)hFont,(LPARAM)TRUE);
//Make Language(язык) string
wrect.top=getstrlen(smemTable->Language);
if(ActiveLangID!=ENG_LANGID)wrect.top+=getstrlen(Language_txt);
wrect.top+=2;
pHdr->language_txt=(LPTSTR)LocalAlloc(LMEM_FIXED,wrect.top);
if(ActiveLangID!=ENG_LANGID){
 wrect.top=copystring(pHdr->language_txt,Language_txt);
 wrect.top+=copystring(pHdr->language_txt+wrect.top,smemTable->Language);
 strappend(pHdr->language_txt+wrect.top,skobka);
}
else copystring(pHdr->language_txt,smemTable->Language);
//create tree-view
TVINSERTSTRUCT tvins; HTREEITEM hti,htiSel;
tvins.item.mask=TVIF_TEXT|TVIF_PARAM|TVIF_STATE; tvins.hInsertAfter=TVI_LAST; tvins.hParent=0;
tvins.item.stateMask=tvins.item.state=TVIS_EXPANDED;
tvins.item.pszText=smemTable->Generals; tvins.item.lParam=0;
hti=(HTREEITEM)SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
  tvins.item.pszText=smemTable->TaskManager; tvins.item.lParam++;
  SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
 tvins.item.pszText=smemTable->Processes; tvins.item.lParam++;
 SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
  tvins.item.pszText=smemTable->Windows; tvins.item.lParam++;
  SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
tvins.item.pszText=smemTable->Performance; tvins.item.lParam++;
SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
 tvins.hParent=hti; tvins.item.pszText=pHdr->language_txt; tvins.item.lParam++;
 SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
 tvins.hParent=0;
tvins.item.pszText=smemTable->Columns; tvins.item.lParam++;
htiSel=(HTREEITEM)SendMessage(hTree,TVM_INSERTITEM,0,(LPARAM)(LPTVINSERTSTRUCT)&tvins);
if((WORD)lParam!=tvins.item.lParam)htiSel=0;
pHdr->apRes[TCI_General]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)27,RT_DIALOG)));
pHdr->apRes[TCI_Language]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)30,RT_DIALOG)));
pHdr->apRes[TCI_TaskManReplace]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)35,RT_DIALOG)));
pHdr->apRes[TCI_Processes]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)28,RT_DIALOG)));
pHdr->apRes[TCI_Performance]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)29,RT_DIALOG)));
DLGTEMPLATE *pdlg;
pHdr->apRes[TCI_Columns]=pdlg=(DLGTEMPLATE*)LocalAlloc(LPTR,sizeof(DLGTEMPLATE)+140);
 pdlg->style=DS_CONTROL|DS_3DLOOK|DS_CENTER|DS_SETFOREGROUND|WS_CHILDWINDOW;
 pdlg->dwExtendedStyle=WS_EX_LEFT|WS_EX_LTRREADING|WS_EX_RIGHTSCROLLBAR|WS_EX_CONTROLPARENT;
 pdlg->cdit=0; pdlg->x=0; pdlg->y=0;
 pdlg->cx=(short)(pHdr->rcDisplay.right-pHdr->rcDisplay.left);
 pdlg->cy=(short)(pHdr->rcDisplay.bottom-pHdr->rcDisplay.top);
pHdr->apRes[TCI_Windows]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)33,RT_DIALOG)));
pHdr->dlgProC[TCI_Windows]=pHdr->dlgProC[TCI_Performance]=pHdr->dlgProC[TCI_Processes]=pHdr->dlgProC[TCI_General]=(DLGPROC*)Setts_CheckMarksProc;
pHdr->dlgProC[TCI_Language]=(DLGPROC*)Setts_LanguageProc;
pHdr->dlgProC[TCI_TaskManReplace]=(DLGPROC*)Setts_TaskManProc;
pHdr->dlgProC[TCI_Columns]=(DLGPROC*)ColumnSelDlg_Proc;
copybytes((char*)&tempSetts,(char*)&progSetts,sizeof(PROGSETTINGS),0);
pHdr->SubSel=HIWORD(lParam);
if(htiSel)SendMessage(hTree,TVM_SELECTITEM,TVGN_CARET,(LPARAM)htiSel);
SetFocus(hTree);
}

///////////////////////////////// Settings_Proc ////////////////////////////////
BOOL CALLBACK SettingsWin_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){DLGHDR5 *pHdr;
switch(Message){
case WM_INITDIALOG:
 //SetClassLong(hDlg,GCL_HICON,(LONG)LoadImage(gInst,(char*)1,IMAGE_ICON,cxsmallicon,cxsmallicon,LR_DEFAULTCOLOR));
 OnInit_SettingsWin(hDlg,lParam);
 break;
case WM_DESTROY:
 if((pHdr=(DLGHDR5*)GetWindowLongPtr(hDlg,GWLP_USERDATA))!=0 && pHdr->Signature==DLGHDR_SIGN_SETTS){
  pHdr->Signature=0;
  LocalFree(pHdr->language_txt);
  LocalFree(pHdr->apRes[TCI_Columns]);
  if(pHdr->hbmp)DeleteObject(pHdr->hbmp);
  if(pHdr->hNormalFont)ReleaseNormalFont();
  LocalFree(pHdr);
 }
 settings_win=0;
 PostQuitMessage(0);
 break;
//case WM_SIZE: resizeServicesWin(HIWORD(lParam),LOWORD(lParam)); break;
case WM_COMMAND:
 switch(LOWORD(wParam)){
  case IDOK:
   if((pHdr=(DLGHDR5*)GetWindowLongPtr(hDlg,GWLP_USERDATA))!=0 && pHdr->Signature==DLGHDR_SIGN_SETTS){
    BYTE bNewLng,bupdCpu,bUpdMenu=0,bUpd[7],bUpdMemUsg=0,bUpdKernelShow=0;
    int i; for(i=0;i<7;i++)bUpd[i]=0;
    if(progSetts.LanguageID!=tempSetts.LanguageID)bNewLng=1;
    else bNewLng=0;
    if(progSetts.CheckState[0]!=tempSetts.CheckState[0])SetWindowPos(main_win,(tempSetts.CheckState[0])?HWND_TOPMOST:HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
    if(progSetts.CheckState[7]!=tempSetts.CheckState[7]){
     switch(tempSetts.CheckState[7]){
      case UPDATESPEED_05s:UpdateSleepTm=500;break;
      case UPDATESPEED_1s:UpdateSleepTm=1000;break;
      case UPDATESPEED_2s:UpdateSleepTm=2000;break;
      case UPDATESPEED_PAUSED:UpdateSleepTm=-1;break;
     }
     ControlUpdateThread(UPDATETHREAD_RESTART,0xFF,0);//UPDATESPEED_PAUSED-> UPDATETHREAD_STOP
    }
    if(progSetts.CheckState[11]!=tempSetts.CheckState[11])bupdCpu=1; else bupdCpu=0;
    if(progSetts.CheckState[12]!=tempSetts.CheckState[12])bUpd[4]=1;
    if(progSetts.CheckState[13]!=tempSetts.CheckState[13])bUpd[6]=1;
    if(progSetts.CheckState[14]!=tempSetts.CheckState[14])bUpdMemUsg=1;
    if(progSetts.CheckState[21]!=tempSetts.CheckState[21]){
     if(!ReplaceTaskManager(tempSetts.CheckState[21]))tempSetts.CheckState[21]=progSetts.CheckState[21];
    }
    if(progSetts.CheckState[26]!=tempSetts.CheckState[26])bUpdKernelShow=1;
    if(progSetts.CheckState[38]!=tempSetts.CheckState[38])bUpd[5]=1;
    if(progSetts.MenuStyle!=tempSetts.MenuStyle)bUpdMenu=1;
    if(progSetts.CheckState[41]!=tempSetts.CheckState[41]){
     bShowAllProcesses=tempSetts.CheckState[41];
     processesUpd=1;
    }
    //
    copyMemBytes(&progSetts,&tempSetts,sizeof(PROGSETTINGS));
    if(bUpdMemUsg){
     if(sysi_win){
      if(progSetts.CheckState[14]==MEMORYTYPE_USAGE_COMMIT){SetDlgItemText(sysi_win,1105,smemTable->Commit); SetDlgItemText(sysi_win,1107,smemTable->CommitHistory);}
      else if(progSetts.CheckState[14]==MEMORYTYPE_USAGE_RAM){SetDlgItemText(sysi_win,1105,smemTable->RAMusage); SetDlgItemText(sysi_win,1107,smemTable->PhysMemUsageHist);}
     }
     gdCommit.histCurPos=(float*)cmtHistStart;
     *(DWORD*)cmtHistEnd=0x03030303;
    }
    if(bUpdMenu){//change Menu Style
     DestroyMenu(main_menu);
     if(tempSetts.MenuStyle==MENU_STYLE_DEFAULT){
      ModernMenu_SetMenuStyle(main_win,main_menu,MENU_STYLE_DEFAULT);
      ModernMenu_Disable();
     }
     else {
      progSetts.MenuStyle=tempSetts.MenuStyle;
     }
     CreateMainMenu(main_win); CreateMainPanelMenu();
    }
    SaveSettings();
    if(bNewLng && pHdr->newLng){
     HANDLE LngFile; DWORD dw,n; char *fpath,*pheader,*hdrmem; bool bMsg; bMsg=0;
     fpath=AllocPathBuffer(); strappend(fpath,pHdr->newLng->fileName);
     dw=LNGFILE_OFFSET_TEXTDATA; pheader=hdrmem=(char*)LocalAlloc(LMEM_FIXED,dw);
     if((LngFile=OpenLngFile(fpath,&pheader))!=0){
      if(*(DWORD*)pheader!=progSetts.LanguageID)goto closeFile;
      pheader+=4;
      dw=*((DWORD*)pheader); pheader+=4; dw+=*((DWORD*)pheader); pheader+=4;//dw=length of lng_name + lng_author);  *pheader=length of first string
      if(SetFilePointer(LngFile,dw,NULL,FILE_CURRENT)==0xFFFFFFFF)goto closeFile;
      dw=*pheader;
      pheader=(char*)LocalAlloc(LMEM_FIXED,dw);
      if(dw && ReadFile(LngFile,pheader,dw,&n,0)!=0 && n==dw)bMsg=1;
      closeFile:
      CloseHandle(LngFile);
      if(bMsg)MessageBox(hDlg,pheader,progTitle,MB_OK|MB_SETFOREGROUND|MB_ICONINFORMATION);
      LocalFree(pheader);
     }
     LocalFree(fpath);
     LocalFree(hdrmem);
    }
    if(bUpdKernelShow && !bTurnOffKernelCalc){
     if(progSetts.CheckState[26]==0){cpuKernelTime=0; FillMemory((void*)(gdCpuKrnl.histStartAddr),((DWORD_PTR)gdCpuKrnl.histLastAddr)-gdCpuKrnl.histStartAddr,0x03);}
     else oldKernelTm=0;
    }
    for(i=0;i<7;i++){Upd[i]=bUpd[i];}
    //add/remove columns
    ApplyColumnChanges(); 
    colmem=(BYTE*)LocalFree(colmem);
    if(bupdCpu)updCpu=1;
   }
  case IDCANCEL: goto on_wmclose;
  case IDHELP:
   if((pHdr=(DLGHDR5*)GetWindowLongPtr(hDlg,GWLP_USERDATA))==0 || pHdr->Signature!=DLGHDR_SIGN_SETTS)break;
   if(pHdr->dwSel==TCI_Processes)wParam=7;
   else if(pHdr->dwSel==TCI_Windows)wParam=8;
   else if(pHdr->dwSel==TCI_TaskManReplace)wParam=9;
   else wParam=6;
   OpenHelpTopic((BYTE)wParam,0);
   break;
 }
 break;
case WM_CLOSE: on_wmclose: DestroyWindow(hDlg); break;
case WM_NOTIFY:
 if(((NMHDR*)lParam)->code==TVN_SELCHANGED){
  SwitchRightDisplay(hDlg,(((NM_TREEVIEW*)lParam)->itemNew).lParam);
 }
 break;
} return 0;}

DWORD SettingsWinThread(LPVOID lParam){MSG msg; HWND hSetsWin;
 if(settings_win!=0)goto quit;
 //pisf->GetObjectInformation=0;
 if((hSetsWin=CreateDialogParam(gInst,(LPTSTR)12,main_win,(DLGPROC)SettingsWin_Proc,(LPARAM)lParam))==0)goto exit;
 settings_win=hSetsWin;
 SetWindowText(hSetsWin,smemTable->Settings);
 ShowWindow(hSetsWin,SW_SHOW);
 AddHwndToWinChain(hSetsWin);
 while(GetMessage(&msg,NULL,0,0)){
  if(!IsDialogMessage(hSetsWin,&msg)){
   TranslateMessage(&msg);DispatchMessage(&msg);
  }
 }
exit:
 LeaveWindowFocus(hSetsWin);
quit:
 ExitThread(0);
 return 0;
}
