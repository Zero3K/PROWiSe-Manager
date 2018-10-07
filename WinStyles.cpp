/*
#include "prowise.cpp"
#include "WinTransparency.cpp"
#include "WinStyles.cpp"
*/

/* List All Definitions
 style=pWinStyles; lvi.mask=LVIF_TEXT; lvi.iItem=0;
 while(pstyle!=0){
  lvi.iSubItem=0; lvi.pszText=pstyle->Name;
  SendMessage(hList,LVM_INSERTITEM,0,(LPARAM)&lvi);
  HexToString(pstyle->Value,wpd->buf1);
  lvi.iSubItem=1; lvi.pszText=wpd->buf1;
  SendMessage(hList,LVM_SETITEM,0,(LPARAM)&lvi);
  lvi.iItem++;
  pstyle=pstyle->NextEntry;
 }
*/

//#define SORT_DEFS

#define VAR_UNDEFINED 0
#define VAR_DEFINING 1
#define VAR_DEFINED 2

/* нужно:
    интерпретаци€ (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE) в качестве Value
*/

typedef struct _VARIABLE_DEF{
 BYTE State;
 char *Name;
 DWORD Value;
}VARIABLE_DEF;

typedef struct _WINDOW_STYLE{
 struct _WINDOW_STYLE *PrevEntry;
 struct _WINDOW_STYLE *NextEntry;
 char *Name;
 DWORD Value;
}WINDOW_STYLE;

typedef struct _WINDOW_PROPERTIES{
 HWND hWnd;
 char *Title;
 char *ClassName;
 char *TypeStyle;
 DWORD TypeStyleLength;
 RECT rect;
 DWORD Style;
 DWORD StyleEx;
 HICON hIcon;
 bool bExist;
}WINDOW_PROPERTIES;

typedef struct _WINDOW_PROPERTIES_DIALOG{
 struct {
  HWND hwndTab;
  HWND hwndDisplay;
  WORD iSel;
  DWORD tabtop;
  RECT rcDisplay;
  DLGTEMPLATE *apRes[2];
  DLGPROC *dlgProC[2];
 } hdr;
 HFONT hDlgFont;
 HWND hList;
 HWND hListEx;
 WINDOW_PROPERTIES winprops;
 char buf1[128];
 LIST_SORT_DIRECTION StyleSortd;
}WINDOW_PROPERTIES_DIALOG;

HINSTANCE glInst; WINDOW_STYLE *pWinStyles; bool HFileLoaded=0;
char *PredefClasses[]={"#32770","Button","Static","Edit","ListBox","SysListView32","ComboBox","ComboBoxEx32","ScrollBar","SysHeader32","SysTreeView32","SysTreeView","SysTabControl32","SysTabControl","SysPager","NativeFontCtl","tooltips_class32",0};
char *PredefClassesStyleStart[]={"DS_","BS_","SS_","ES_","LBS_","LVS_","CBS_","CBES_EX_","SBS_","HDS_","TVS_","TVS_","TCS_","TCS_","PGS_","NFS_","TTS_",0};

/*
WINDOW_STYLE WinStyles[]={
 {"WS_EX_DLGMODALFRAME",0x00000001},
 {"WS_EX_NOPARENTNOTIFY",  0x00000004L},
 {"WS_EX_TOPMOST",  0x00000008L},
 {"WS_EX_ACCEPTFILES",  0x00000010L},
 {"WS_EX_TRANSPARENT",  0x00000020L},
 {"WS_EX_MDICHILD",  0x00000040L},
 {"WS_EX_TOOLWINDOW",  0x00000080L},
 {"WS_EX_WINDOWEDGE",        0x00000100L},
 {"WS_EX_CLIENTEDGE",        0x00000200L},
 {"WS_EX_CONTEXTHELP",       0x00000400L},
 {"WS_EX_RIGHT",             0x00001000L},
 {"WS_EX_LEFT",              0x00000000L},
 {"WS_EX_RTLREADING",        0x00002000L},
 {"WS_EX_LTRREADING",        0x00000000L},
 {"WS_EX_APPWINDOW",         0x00040000L},
 {"WS_EX_OVERLAPPEDWINDOW (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)",WS_EX_OVERLAPPEDWINDOW}
};*/

#define CMD_NOTHING 0
#define CMD_DEFINE 1
#define CMD_COMMENTBLOCK 2
#define CMD_COMMENTLINE 4
#define CMD_LINECONTINUE 8
#define CMD_VALUE_DEFINED 16
#define CMD_OR_CHAR 32

#ifdef SORT_DEFS
void SortDefines(WINDOW_STYLE **FirstDefine){WINDOW_STYLE *pst,*pstyle,*fndpst,*ppdef; DWORD fndValue=0;
if(FirstDefine==0 || *FirstDefine==0)return;
again:
pstyle=*FirstDefine;
while(pstyle!=0){
 fndpst=0; pst=*FirstDefine;
 while(pst!=0){
  if(fndpst){
   if(pst->Value>fndValue && pst->Value<pstyle->Value){fndValue=pst->Value; fndpst=pst;}
  }
  else if(pst->Value<pstyle->Value){fndValue=pst->Value; fndpst=pst;}
  pst=pst->NextEntry;
 }
 if(fndpst && fndpst!=pstyle->NextEntry){
  //а если предыдущие - это цепочка с этим же Value и среди них есть этот def ? “огда оставим в покое и едем дальше.
  ppdef=fndpst->PrevEntry;
  while(ppdef){
   if(ppdef->Value!=pstyle->Value)break;
   if(ppdef==pstyle){
    ppdef=ppdef;
    goto goNext;
   }
   ppdef=ppdef->PrevEntry;
  }
  // соедин€ем соседние две
  if(pstyle->PrevEntry)pstyle->PrevEntry->NextEntry=pstyle->NextEntry;
  else *FirstDefine=pstyle->NextEntry;
  if(pstyle->NextEntry)pstyle->NextEntry->PrevEntry=pstyle->PrevEntry;
  // вклиниваемс€ между найденной и еЄ предыдущей
  pstyle->NextEntry=fndpst;
  pstyle->PrevEntry=fndpst->PrevEntry;
  fndpst->PrevEntry=pstyle;
  if(pstyle->PrevEntry)pstyle->PrevEntry->NextEntry=pstyle;
  else *FirstDefine=pstyle;
  goto again;//начинаем заново
 }
 goNext:
 pstyle=pstyle->NextEntry;
}
}
#endif

void ReadHFile(){HANDLE hFile; IO_STATUS_BLOCK iob[1]; DWORD ErrCode,n,sz,dw; FILE_STANDARD_INFORMATION fsi; void *memstyles;
 WINDOW_STYLE *pstyle,*ppstyle; BYTE *pb,*pbb; VARIABLE_DEF vdef; bool bNewDefine=0;
 if(HFileLoaded)return;
 memstyles=0; pstyle=0;
 if((hFile=CreateFile("WinStyles.h",GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))==INVALID_HANDLE_VALUE){
  return;
 }
 if((ErrCode=ntdllFunctions.pNtQueryInformationFile(hFile,iob,&fsi,sizeof(fsi),FileStandardInformation))!=0){
  FailMessage("NQIF",ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
  goto closeFile;
 }
 sz=fsi.EndOfFile.LowPart;
 if(sz>5242880){
  sz=5242880;//5 Mbytes
  FailMessage("Size of Header file exceeds 5 Mb.",ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
 }
 if((memstyles=VirtualAllocEx((HANDLE)0xFFFFFFFF,0,sz,MEM_COMMIT,PAGE_READWRITE))==0)goto closeFile;
 SetFilePointer(hFile,0,NULL,FILE_BEGIN);
 if(ReadFile(hFile,memstyles,sz,&n,NULL)==0 || n==0)goto closeFile;//12808
 sz+=(DWORD)memstyles;
 pb=(BYTE*)memstyles;
 DWORD cmd; cmd=CMD_NOTHING; vdef.State=VAR_UNDEFINED;
 read_line:
  if((DWORD)pb>=sz)goto endoffile;
  if((cmd&CMD_COMMENTBLOCK)==0){//outside COMMENT BLOCK
   while((DWORD)pb<sz){
    if(*pb==0x20 || *pb==0x09)pb++;
    else break;
   }
   if(cmd&CMD_LINECONTINUE){
    if(cmd&CMD_COMMENTLINE)goto search_lineEnd;
   }
   else{
    cmd&=~CMD_COMMENTLINE;
   }
   if(*pb=='#' && (pb==memstyles || *(pb-1)=='\n')){
    pb++;
    if(CompareStrPart((char*)pb,"define",6)==0){
     pb+=6;
     if(vdef.State!=VAR_UNDEFINED){//set next & prev offsets
      bNewDefine=1;
      goto save_definition;
      continue_newdef:
      bNewDefine=0;
     }
     cmd=CMD_DEFINE;
    }
   }
   else if(*pb==0x2F && *(pb+1)==0x2A){pb+=2; cmd|=CMD_COMMENTBLOCK;}// /*
   else if(*pb==0x28)pb++;// (
   else if(*pb==0x7C){pb++; cmd|=CMD_OR_CHAR;}// |
   else if(*pb==0x2F && *(pb+1)==0x2F){pb+=2; cmd|=CMD_COMMENTLINE; goto search_lineEnd;}// //
   else if(*pb==0x5C && *(pb+1)==0x0D && *(pb+2)==0x0A){pb+=3; cmd|=CMD_LINECONTINUE; goto read_line;}// Continuation character
   else if(cmd&CMD_DEFINE){
    if(cmd&CMD_VALUE_DEFINED && (cmd&CMD_OR_CHAR)==0)goto save_definition;
    pbb=pb; n=0;
    while((DWORD)pbb<sz){
     if((*pbb>=0x41 && *pbb<=0x5A) || (*pbb>=0x61 && *pbb<=0x7A) || *pbb==0x5F)pbb++;//a...z || A...Z || _
     else if(*pbb>=0x30 && *pbb<=0x39)pbb++;//0...9
     else {n=*pbb; *pbb=0; break;}
    }
    if(vdef.State==VAR_DEFINING || cmd&CMD_VALUE_DEFINED)goto set_value;
    vdef.Name=LocalAllocAndCopy((char*)pb);
    vdef.Value=0;
    vdef.State=VAR_DEFINING;
    goto skip_setvalue;
    set_value:
    if((cmd&CMD_VALUE_DEFINED)==0 || cmd&CMD_OR_CHAR){
     cmd|=CMD_VALUE_DEFINED;
     if(*pb>=0x30 && *pb<=0x39){
      if(*(pb+1)=='x')dw=HexStringToDword((char*)pb);
      else dw=atol((char*)pb);
      vdef.Value|=dw;
     }
    }
    cmd&=~CMD_OR_CHAR;
    skip_setvalue:
    if(n)*pbb=(BYTE)n;
    pb=pbb;
   }
   else if(*pb=='\r' && *(pb+1)=='\n')pb+=2;
   else pb++;
  }
  else{// inside COMMENT BLOCK
   while((DWORD)pb<sz){
    if(*pb==0x2A && *(pb+1)==0x2F){pb+=2; cmd&=~CMD_COMMENTBLOCK; break;}// */
    pb++;
   }
  }
  ///
  if(*pb=='\r' && *(pb+1)=='\n')pb+=2;
  goto read_line;
  ////
  save_definition:
  if(vdef.State!=VAR_UNDEFINED){
   ppstyle=pstyle;
   if((pstyle=(WINDOW_STYLE*)LocalAlloc(LMEM_FIXED,sizeof(WINDOW_STYLE)))!=0){
    if(pWinStyles==0)pWinStyles=pstyle;
    pstyle->Name=vdef.Name;
    pstyle->Value=vdef.Value;
    pstyle->PrevEntry=ppstyle;
    if(ppstyle)ppstyle->NextEntry=pstyle;
    pstyle->NextEntry=0;
   }
  }
  vdef.State=VAR_UNDEFINED;
  cmd&=~CMD_VALUE_DEFINED;
  if(bNewDefine)goto continue_newdef;
  else cmd&=~CMD_DEFINE;
  ////
  search_lineEnd:
  while((DWORD)pb<sz){
   if(*pb=='\r' && *(pb+1)=='\n'){
    if(*(pb-1)==0x5C)cmd|=CMD_LINECONTINUE;// Continuation character is last character
    else cmd&=~CMD_LINECONTINUE;
    pb+=2;
    break;
   }
   pb++;
  }
  goto read_line;
 endoffile:
 HFileLoaded=1;
 closeFile:
 if(memstyles)VirtualFreeEx((HANDLE)0xFFFFFFFF,memstyles,0,MEM_RELEASE);
 CloseHandle(hFile);
 #ifdef SORT_DEFS
 SortDefines(&pWinStyles);
 #endif
}

WINDOW_PROPERTIES_DIALOG *WinProps_OnChildDialogInit(HWND hDlg){WINDOW_PROPERTIES_DIALOG *wpd;
 if((wpd=(WINDOW_PROPERTIES_DIALOG*)GetWindowLong(GetParent(hDlg),GWL_USERDATA))==0)return 0;
 SetWindowPos(hDlg,HWND_TOP,5,wpd->hdr.tabtop,wpd->hdr.rcDisplay.right-5,wpd->hdr.rcDisplay.bottom-1,0);
 if(pEnableThemeDialogTexture>0)pEnableThemeDialogTexture(hDlg,0x00000006);//ETDT_ENABLETAB or ETDT_USETABTEXTURE
 return wpd;
}

void WinProps_OnTabSelChanged(HWND hDlg){WINDOW_PROPERTIES_DIALOG *wpd;
 if((wpd=(WINDOW_PROPERTIES_DIALOG*)GetWindowLong(hDlg,GWL_USERDATA))==0)return;
 wpd->hdr.iSel=(WORD)SendMessage(wpd->hdr.hwndTab,TCM_GETCURSEL,0,0);
 if(wpd->hdr.hwndDisplay!=NULL)DestroyWindow(wpd->hdr.hwndDisplay);
 wpd->hdr.hwndDisplay=CreateDialogIndirect(glInst,wpd->hdr.apRes[wpd->hdr.iSel],hDlg,(DLGPROC)wpd->hdr.dlgProC[wpd->hdr.iSel]);
 ShowWindow(wpd->hdr.hwndDisplay,SW_SHOW);
}

void WinProps_StylesInit(HWND hDlg){WINDOW_PROPERTIES_DIALOG *wpd; WINDOW_STYLE *pstyle; LV_COLUMN lvcol; LV_ITEM lvi; HWND hList,hListEx,hwin;
 DWORD n,nn,dw,ds; int i,in; bool bOverlapped;
 //SendMessage(GetDlgItem(hDlg,7),STM_SETICON,(long)LoadIcon(gInst,(char*)2),0);
 WindowFillText(hDlg,dlgTAT->dlg32);
 if((wpd=WinProps_OnChildDialogInit(hDlg))==0)return;
 ReadHFile();
 wpd->hList=hList=GetDlgItem(hDlg,402);
 wpd->hListEx=hListEx=GetDlgItem(hDlg,404);
 hwin=hList;
  again:
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH; lvcol.fmt=LVCFMT_LEFT;
 lvcol.cx=230; lvcol.pszText=smemTable->Style; lvcol.iSubItem=0;
 SendMessage(hwin,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
 lvcol.cx=80; lvcol.pszText=smemTable->Value; lvcol.iSubItem=1;
 SendMessage(hwin,LVM_INSERTCOLUMN,(WPARAM)1,(LPARAM)&lvcol);
 SendMessage(hwin,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)LVS_EX_FULLROWSELECT|LVS_EX_FLATSB|LVS_EX_LABELTIP);
 if(hwin!=hListEx){hwin=hListEx; goto again;}
 //
 n=wpd->winprops.Style;
 nn=wpd->winprops.StyleEx;
 HexToString(n,wpd->buf1); SetDlgItemText(hDlg,401,wpd->buf1);
 HexToString(nn,wpd->buf1); SetDlgItemText(hDlg,403,wpd->buf1);
 pstyle=pWinStyles; i=in=0; hwin=0; bOverlapped=0;
 while(pstyle!=0){
  //Show StyleEx
  if(CompareStrPart((char*)(pstyle->Name),"WS_EX",5)==0){
   if((nn&pstyle->Value)==pstyle->Value){
    hwin=hListEx;
    lvi.iItem=in; in++;
    nn&=~(pstyle->Value);
    goto insert_item;
   }
  }
  //Show common Style & TypeStyle
  else if(CompareStrPart((char*)(pstyle->Name),"WS_",3)==0 && CompareStrPart((char*)(pstyle->Name),"WS_EX",5)!=0){
   if((n&pstyle->Value)==pstyle->Value){
    dw=pstyle->Value; ds=wpd->winprops.Style;
    if(dw==WS_OVERLAPPED){
     if(bOverlapped)goto goNext;
     if((ds&WS_POPUP)==WS_POPUP)goto goNext;
     if((ds&WS_CHILD)==WS_POPUP)goto goNext;
    }
    else if(dw==WS_OVERLAPPEDWINDOW)bOverlapped=1;
    else if(dw==WS_GROUP || dw==WS_TABSTOP){
     if(lstrcmpi(pstyle->Name,"WS_GROUP")==0 || lstrcmpi(pstyle->Name,"WS_TABSTOP")==0){
      if((ds&WS_CHILD)!=WS_CHILD)goto goNext;
     }
    }
    /*else if(dw==WS_DLGFRAME || dw==WS_CAPTION){
     if((ds&WS_DLGFRAME)==WS_DLGFRAME)goto goNext;
     else if((ds&WS_CAPTION)==WS_CAPTION)goto goNext;
     else if((ds&WS_DLGFRAME)==WS_DLGFRAME)goto goNext;
    }*/
    add_style:
    hwin=hList;
    lvi.iItem=i; i++;
    n&=~(pstyle->Value);
    goto insert_item;
   }
  }
  else if(CompareStrPart((char*)(pstyle->Name),wpd->winprops.TypeStyle,wpd->winprops.TypeStyleLength)==0){
   if((n&pstyle->Value)==pstyle->Value)goto add_style;
  }
  insert_item:
  if(hwin){
   if(pstyle->Value==0 && progSetts.CheckState[34]==0)goto skip_insert;
   lvi.lParam=(DWORD)pstyle; lvi.mask=LVIF_TEXT|LVIF_PARAM;
   lvi.iSubItem=0; lvi.pszText=pstyle->Name;
   lvi.iItem=SendMessage(hwin,LVM_INSERTITEM,0,(LPARAM)&lvi);
   HexToString(pstyle->Value,wpd->buf1);
   lvi.mask=LVIF_TEXT;
   lvi.iSubItem=1; lvi.pszText=wpd->buf1;
   SendMessage(hwin,LVM_SETITEM,0,(LPARAM)&lvi);
   skip_insert:
   hwin=0;
  }
  goNext:
  pstyle=pstyle->NextEntry;
 }
 //
 hwin=0;
 unkn:
 if(n!=0){hwin=hList; dw=n; n=0; lvi.iItem=i;}
 else if(nn!=0){hwin=hListEx; dw=nn; nn=0; lvi.iItem=in;}
 if(hwin){
  lvi.lParam=(DWORD)-1; lvi.mask=LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem=0; lvi.pszText="-= UNKNOWN =-";
  lvi.iItem=SendMessage(hwin,LVM_INSERTITEM,0,(LPARAM)&lvi);
  HexToString(dw,wpd->buf1);
  lvi.mask=LVIF_TEXT;
  lvi.iSubItem=1; lvi.pszText=wpd->buf1;
  SendMessage(hwin,LVM_SETITEM,0,(LPARAM)&lvi);
  hwin=0;
  goto unkn;
 }
}

int CALLBACK StylesListCompareFunc(LPARAM lParam1,LPARAM lParam2,LIST_SORT_DIRECTION *lParamSort){
int ret,isub; WINDOW_STYLE *wst1,*wst2; char *ptr1,*ptr2;
if((DWORD)lParam1==(DWORD)-1){ret=1;goto exit;}
if((DWORD)lParam2==(DWORD)-1){ret=-1;goto exit;}
if(lParamSort->SortDirection==2){
 wst1=pWinStyles; ret=0;
 while(wst1!=0){
  if((DWORD)wst1==(DWORD)lParam1){ret=-1;break;}
  else if((DWORD)wst1==(DWORD)lParam2){ret=1;break;}
  wst1=wst1->NextEntry;
 }
 goto quit;
}
isub=lParamSort->CurSubItem;
wst1=(WINDOW_STYLE*)lParam1; wst2=(WINDOW_STYLE*)lParam2;
if(isub==0){
 ptr1=wst1->Name; ptr2=wst2->Name;
 if(ptr1 && ptr2)ret=lstrcmpi(ptr1,ptr2);
 else if(ptr1==0){if(ptr2)ret=-1; else ret=0;}
 else ret=1;
}
else if(isub==1){ret=(DWORD)wst1->Value; ret-=(DWORD)wst2->Value;}
quit:
if(lParamSort->SortDirection==1){if(ret<0)ret=1; else if(ret>0)ret=-1;}
exit:
return ret;
}

BOOL CALLBACK WinProps_StylesProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){WINDOW_PROPERTIES_DIALOG *wpd;
switch(Message){
case WM_INITDIALOG: WinProps_StylesInit(hDlg); break;
case WM_CLOSE: EndDialog(hDlg,1); break;
/*case WM_COMMAND:
 switch(LOWORD(wParam)){
 }
 break;*/
case WM_NOTIFY:
 DWORD dCode; dCode=((NMHDR*)lParam)->code;
 if(dCode==LVN_COLUMNCLICK){
  if((wpd=(WINDOW_PROPERTIES_DIALOG*)GetWindowLong(GetParent(hDlg),GWL_USERDATA))==0)break;
  HWND hFrom; hFrom=((NMHDR*)lParam)->hwndFrom;
  if(hFrom!=wpd->hList && hFrom!=wpd->hListEx)break;
  wpd->StyleSortd.hFrom=hFrom;
  LV_ColumnClickSort((NM_LISTVIEW*)lParam,&(wpd->StyleSortd),(DWORD)StylesListCompareFunc);
 }
 break;
} return 0;}

void WinProps_OnInit(HWND hDlg,WINDOW_PROPERTIES_DIALOG *wpd){TCITEM tci; DWORD cyMargin,dwDlgBase; HFONT hDlgFont;
 SetWindowText(hDlg,smemTable->WindowProperties);
 dwDlgBase=GetDialogBaseUnits();
 cyMargin=HIWORD(dwDlgBase)/8;
 wpd->hdr.hwndTab=CreateWindow(WC_TABCONTROL,0,WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|WS_TABSTOP,0,0,50,80,hDlg,0,glInst,NULL);
 tci.mask=TCIF_TEXT;
 tci.pszText=smemTable->Styles;
 SendMessage(wpd->hdr.hwndTab,TCM_INSERTITEM,0,(LPARAM)&tci);
 //tci.pszText=smemTable->Load_LogOn;
 //SendMessage(wpd->hdr.hwndTab,TCM_INSERTITEM,1,(LPARAM)&tci);
 RECT rtmp;
 SendMessage(wpd->hdr.hwndTab,TCM_GETITEMRECT,0,(LPARAM)&rtmp);
 wpd->hdr.tabtop=rtmp.bottom+rtmp.top;
 wpd->hdr.apRes[0]=(DLGTEMPLATE*)LockResource(LoadResource(gInst,FindResource(NULL,(char*)32,RT_DIALOG)));
 wpd->hdr.dlgProC[0]=(DLGPROC*)WinProps_StylesProc;
 GetClientRect(hDlg,&wpd->hdr.rcDisplay);
 wpd->hdr.rcDisplay.left=0; wpd->hdr.rcDisplay.right-=8;
 wpd->hdr.rcDisplay.bottom-=cyMargin; wpd->hdr.rcDisplay.bottom-=30;
 SetWindowPos(wpd->hdr.hwndTab,NULL,4,2,wpd->hdr.rcDisplay.right-wpd->hdr.rcDisplay.left,wpd->hdr.rcDisplay.bottom-wpd->hdr.rcDisplay.top,SWP_NOZORDER);
 DWORD cx,cy; HWND hB;
 cx=wpd->hdr.rcDisplay.right; cx-=75;
 cy=wpd->hdr.rcDisplay.bottom+5;
 cx-=80;
 hDlgFont=wpd->hDlgFont;
 hB=CreateWindowEx(WS_EX_TOPMOST,BUTTON_txt,smemTable->OK,BS_PUSHBUTTON|BS_CENTER|WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|WS_DISABLED,cx,cy,75,23,hDlg,(HMENU)IDOK,glInst,NULL);
 SendMessage(hB,WM_SETFONT,(WPARAM)hDlgFont,(LPARAM)TRUE); cx+=80;
 hB=CreateWindowEx(WS_EX_TOPMOST,BUTTON_txt,smemTable->Cancel,BS_PUSHBUTTON|BS_CENTER|WS_CHILD|WS_VISIBLE|WS_TABSTOP,cx,cy,75,23,hDlg,(HMENU)IDCANCEL,glInst,NULL);
 SendMessage(hB,WM_SETFONT,(WPARAM)hDlgFont,(LPARAM)TRUE);
 wpd->hdr.rcDisplay.bottom-=tabtop;
 SendMessage(wpd->hdr.hwndTab,WM_SETFONT,(WPARAM)hDlgFont,(LPARAM)TRUE);
 SendMessage(wpd->hdr.hwndTab,TCM_SETCURSEL,0,0);
 //int ln; ln=copybytes((char*)(wpd->hnbuf),smemTable->ServiceProperties,0,2);
 //copybytes((char*)(wpd->hnbuf)+ln,wpd->SvcProps.SvcName,0,2);
 //SetWindowText(hDlg,(char*)(pHdr->hnbuf));
}

BOOL CALLBACK WinPropsMain_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){//WINDOW_PROPERTIES_DIALOG *wpd;
switch(Message){
case WM_INITDIALOG:
 SetWindowLong(hDlg,GWL_USERDATA,(DWORD)lParam);
 SetWindowPos(hDlg,0,0,0,380,300,SWP_NOZORDER|SWP_NOMOVE);
 SetWinPos(hDlg,1);
 WinProps_OnInit(hDlg,(WINDOW_PROPERTIES_DIALOG*)lParam);
 goto sel_changed;
case WM_COMMAND:
 WORD wID; wID=LOWORD(wParam);
 switch(wID){
  case 50004: case IDCANCEL: goto closeWin;
  //case 50008: OpenHelpTopic(2,0); break;
 }
 break;
case WM_CLOSE: closeWin: DestroyWindow(hDlg); break;
case WM_DESTROY: PostQuitMessage(0); return 1;
case WM_NOTIFY:
 if(((LPNMHDR)lParam)->code==TCN_SELCHANGE){
  sel_changed:
  WinProps_OnTabSelChanged(hDlg);
 }
 break;
} return 0;}


DWORD WindowPropertiesThread(LPARAM lParam){HWND mainWin,props_win; WINDOW_PROPERTIES_DIALOG *wpd; MSG msg; HACCEL haccel;
 DWORD dw; char *ptr,**pptr,**pptr2;
 mainWin=main_win; glInst=gInst; wpd=0;
 if(!IsWindow((HWND)lParam)){
  MessageBox(mainWin,smemTable->HandleDoesntIdentifyExistingWindow,ERROR_txt,MB_OK|MB_ICONSTOP);
  goto exit;
 }
 if((wpd=(WINDOW_PROPERTIES_DIALOG*)LocalAlloc(LPTR,sizeof(WINDOW_PROPERTIES_DIALOG)))==0)goto exit;
 wpd->hDlgFont=hDlgFont;
 wpd->winprops.hWnd=(HWND)lParam;
 wpd->winprops.Style=GetWindowLong((HWND)lParam,GWL_STYLE);
 wpd->winprops.StyleEx=GetWindowLong((HWND)lParam,GWL_EXSTYLE);
 //get title
 if((dw=GetWindowTextLength((HWND)lParam))!=0){
  if((ptr=(char*)LocalAlloc(LMEM_FIXED,++dw))!=0){
   if(GetWindowText((HWND)lParam,ptr,dw)){
    wpd->winprops.Title=LocalAllocAndCopy(ptr);
   }
   LocalFree(ptr);
  }
 }
 //get class name
 if(GetClassName((HWND)lParam,wpd->buf1,128)){
  wpd->winprops.ClassName=LocalAllocAndCopy(wpd->buf1);
 }
 //hIcon, rect
 //GetClassInfoEx
 //
 pptr=PredefClasses; pptr2=PredefClassesStyleStart;
 if(wpd->winprops.ClassName!=0){
  while(*pptr){
   if(lstrcmpi(*pptr,wpd->winprops.ClassName)==0){
    wpd->winprops.TypeStyle=*pptr2;
    wpd->winprops.TypeStyleLength=getstrlen(wpd->winprops.TypeStyle);
    break;
   }
   pptr++; pptr2++;
  }
 }
 wpd->StyleSortd.CurSubItem=0xFFFFFFFF;
 wpd->StyleSortd.SortDirection=2;
 if((props_win=CreateDialogParam(glInst,(char*)12,mainWin,(DLGPROC)WinPropsMain_Proc,(DWORD)wpd))==0)goto exit;
 ShowWindow(props_win,SW_SHOW);
 AddHwndToWinChain(props_win);
 haccel=LoadAccelerators(glInst,(char*)2);
 while(GetMessage(&msg,NULL,0,0)){
  if(haccel!=0 && TranslateAccelerator(props_win,haccel,&msg))continue;
  if(!IsDialogMessage(props_win,&msg)){
   TranslateMessage(&msg);DispatchMessage(&msg);
  }
 }
 exit:
 if(wpd)LocalFree(wpd);
 LeaveWindowFocus(props_win);
 ExitThread(0);
return 0;}


