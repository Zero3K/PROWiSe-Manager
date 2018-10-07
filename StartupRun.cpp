/*
LVIF_GROUPID
LVIF_COLUMNS
#if (_WIN32_WINNT >= 0x501)
    int iGroupId;
    UINT cColumns; // tile view columns
    PUINT puColumns;
#endif
*/
RECT sz2[5]; IShellLink* pshl; IPersistFile* ppf;
int LItem; BYTE CoInitOk=0;

#define OFS_HKCU 20//offset to HKCU text in HkeyStr
#define STRTUP_GENERAL_ENTRIES 0x02
#define STRTUP_OTHER_ENTRIES 0x04
#define STRTUP_ALL_ENTRIES STRTUP_OTHER_ENTRIES|STRTUP_GENERAL_ENTRIES

char RunStr[]={"Run\0RunOnce\0RunOnceEx\0RunServices\0RunServicesOnce\0\0"};
char mainkeyStr[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\\0\0\0.DEFAULT\\\0\0";
char HkeyStr[]="HKEY_LOCAL_MACHINE\\\0HKEY_CURRENT_USER\\\0HKEY_USERS\\\0\0";
char NTkeyStr[]="SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows\\\0";
char NTkeyVlStr[]="Load\0Run\0\0";
char Keys2[]="Userinit\0Shell\0System\0VmApplet\0\0";
char Key3[]="SYSTEM\\CurrentControlSet\\Control\\SessionManager\\";
char BootExecute[]="BootExecute";
char CMD_Run[]="SOFTWARE\\Microsoft\\Command Processor\\";
char AutoRun[]="AutoRun";
char Key4[]="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\explorer\\Browser Helper Objects\\";
char PolicyRun[]="Policies\\Explorer\\Run";
char ShellFolders_txt[]="Explorer\\Shell Folders";
char AppInit_DLLs[]="AppInit_DLLs";
HKEY Hkeys[3]={HKEY_LOCAL_MACHINE,HKEY_CURRENT_USER,HKEY_USERS};

////////////////////////////// Count StRunList Items ///////////////////////////
void CountStRunList(){
 LItem=SendMessage(StRunList,LVM_GETITEMCOUNT,0,0);
 lstrcpy(t1buf1,smemTable->Total_); ltoa(LItem,t1buf1+lstrlen(t1buf1),10);
 SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)t1buf1);
}

//////////////////////////////////// IsPathValid ///////////////////////////////
bool IsPathValid(char *szPath){
 if(!szPath || *szPath=='{')return 0;
 while(*szPath!=0){
  if(*szPath==':' && (*(szPath+1)=='\\' || *(szPath+1)=='/')){
   if(*(szPath-2)!=0x20)return 1;
   break;
  }
  szPath++;
 }
 return 0;
}

////////////////////////////// Delete Startup Item /////////////////////////////
void DeleteStartupItem(){int ik; LV_ITEM lvi; DWORD ErrCode; char *cbuf1;
if((cbuf1=(char*)LocalAlloc(LPTR,1024))==0)return;
lvi.iItem=itemSel; *cbuf1=0;
lvi.iSubItem=2; lvi.pszText=cbuf1; lvi.cchTextMax=300;
lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.lParam=0;
if(SendMessage(StRunList,LVM_GETITEM,0,(LPARAM)&lvi)==0)goto exitf;
in=lstrlen(cbuf1);
if(*(DWORD*)cbuf1!=0x59454B48)goto deletefile;
HKEY hKey,hKey2,hRegKEY; BYTE character;
for(ik=0;ik<in && *(cbuf1+ik)!='\\';ik++); ik++; character=*(cbuf1+ik); *(cbuf1+ik)=0;
if(*(cbuf1+5)=='L' && lstrcmpi(cbuf1,HkeyStr)==0)hRegKEY=HKEY_LOCAL_MACHINE;
else if(*(cbuf1+5)=='C' && lstrcmpi(cbuf1,HkeyStr+20)==0)hRegKEY=HKEY_CURRENT_USER;
else if(*(cbuf1+5)=='U' && lstrcmpi(cbuf1,HkeyStr+39)==0)hRegKEY=HKEY_USERS;
else{ErrCode=2; goto failExit;}
*(cbuf1+ik)=character;
ErrCode=RegOpenKey(hRegKEY,cbuf1+ik,&hKey);
if(ErrCode!=ERROR_SUCCESS){failExit: FailMessage(smemTable->UnblOpenRegKey,ErrCode,FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG); goto exitf;}
if((lvi.lParam&ITEM_1)==0)lvi.iSubItem=0; else lvi.iSubItem=1;
SendMessage(StRunList,LVM_GETITEMTEXT,itemSel,(LPARAM)&lvi);
if(lvi.lParam&ITEM_REGVALUE){
 lstrcpy(t0buf2,smemTable->AreYouSureYouWanna); lstrcat(t0buf2,smemTable->removeRegValue);
 if(MessageBox(strtrun_win,t0buf2,progTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)goto RegExit;
 if(RegDeleteValue(hKey,cbuf1)==ERROR_SUCCESS)MessageBox(strtrun_win,smemTable->Done,progTitle,MB_OK);
}
else if(lvi.lParam&ITEM_REGSUBKEY){
 lstrcpy(t0buf2,smemTable->AreYouSureYouWanna); lstrcat(t0buf2,smemTable->removeRegSubkey);
 if(MessageBox(strtrun_win,t0buf2,progTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)goto RegExit;
 if(RegDeleteKey(hKey,cbuf1)==ERROR_SUCCESS)MessageBox(strtrun_win,smemTable->Done,progTitle,MB_OK);
 else {
  int i; i=0; in=0;
  if(RegOpenKey(hKey,cbuf1,&hKey2)==ERROR_SUCCESS){
   do{
    n=300; ik=RegEnumKeyEx(hKey2,i,tbuf3,&n,NULL,NULL,NULL,NULL); i++;
    if(ik==ERROR_SUCCESS){
     if(!in){in=1; if(MessageBox(main_win,smemTable->KeyContSubkeys,progTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)goto RegExit;}
     if(RegDeleteKey(hKey2,tbuf3)==ERROR_SUCCESS)i=0;
    }
   }while(ik!=ERROR_NO_MORE_ITEMS);
   RegCloseKey(hKey2);
   if(RegDeleteKey(hKey,cbuf1)==ERROR_SUCCESS)MessageBox(strtrun_win,smemTable->Done,progTitle,MB_OK);
   else goto exitRegFail;
  }
  else goto exitRegFail;
 }
}
SendMessage(StRunList,LVM_DELETEITEM,(WPARAM)itemSel,(LPARAM)0);
CountStRunList();
RegExit:
if(hKey!=NULL)RegCloseKey(hKey);
goto exitf;
deletefile:
if(!IsPathValid(cbuf1)){
 MessageBox(strtrun_win,smemTable->PathIsNotValid,progTitle,MB_OK|MB_ICONSTOP);
 goto exitf;
}
if(*(cbuf1+in-1)!='\\'){*(cbuf1+in)='\\'; in++; *(cbuf1+in)=0;}
lvi.pszText=cbuf1+in; lvi.cchTextMax=500-in;
if((lvi.lParam&ITEM_1)==0)lvi.iSubItem=0; else lvi.iSubItem=1;
SendMessage(StRunList,LVM_GETITEMTEXT,itemSel,(LPARAM)&lvi);
lstrcpy(tbuf3,smemTable->AreYouSureYouWanna);
lstrcat(tbuf3,smemTable->delete_file_); lstrcat(tbuf3,cbuf1+in); lstrcat(tbuf3," ?");
if(MessageBox(strtrun_win,tbuf3,progTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)goto exitf;
if(DeleteFile(cbuf1)){
 MessageBox(strtrun_win,smemTable->Done,progTitle,MB_OK);
 SendMessage(StRunList,LVM_DELETEITEM,(WPARAM)itemSel,(LPARAM)0);
 CountStRunList();
}
else MessageBox(strtrun_win,smemTable->UnblDelFile,progTitle,MB_OK|MB_ICONWARNING);
goto exitf;
exitRegFail:
MessageBox(strtrun_win,smemTable->UnblDelRegKey,progTitle,MB_OK|MB_ICONWARNING);
if(hKey!=NULL)RegCloseKey(hKey);
exitf:
 LocalFree(cbuf1);
}

///////////////////////////// Create Right Context Menu ////////////////////////
void CreateRMenu(int iItem){HMENU hMenu; POINT pt;
hMenu=GetSubMenu(main_menu,2);
GetCursorPos(&pt);
TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_RIGHTBUTTON,pt.x,pt.y,0,strtrun_win,NULL);
}

///////////////////////////// Get Program_Files Dir ////////////////////////////
bool GetProgramFilesDir(LPTSTR lpBuffer,DWORD *uSize){HKEY hKey=NULL; DWORD dType;
 if(RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion",&hKey)!=ERROR_SUCCESS)return 0;
 if(RegQueryValueEx(hKey,"ProgramFilesDir",NULL,&dType,(LPBYTE)lpBuffer,uSize)!=ERROR_SUCCESS)goto exit;
 if(*uSize<1 || (dType!=REG_SZ && dType!=REG_EXPAND_SZ && dType!=REG_MULTI_SZ)){SetLastError(ERROR_INVALID_DATA); goto exit;}
 RegCloseKey(hKey);
 return 1;
 exit:
 if(hKey!=NULL)RegCloseKey(hKey);
 return 0;
}

///////////////////////////////// AddIconToList ////////////////////////////////
int AddIconToList(char *ptr){HICON hIcon; hIcon=0; int ImgNum; ImgNum=0;
ExtractIconEx(ptr,in,NULL,&hIcon,1);
if(hIcon!=0){
 ImgNum=ImageList_AddIcon(himgl,hIcon);
 DestroyIcon(hIcon);
}
return ImgNum;}

///////////////////////////// Retrieve path from LinkFile //////////////////////
char *ResolveLinkFile(char *lnkPath,int *ImgNum){HRESULT res; STRING str; UNICODE_STRING ustr; char *pret=0;
if(pshl==NULL){
 res=CoCreateInstance((REFCLSID)CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,(REFIID)IID_IShellLink,(void **)&pshl);// Get a pointer to the IShellLink interface.
 if(FAILED(res)){FailMessage(CoCreateInstance_txt,res,FMSG_WRITE_LOG); return 0;}
 res=pshl->QueryInterface((REFIID)IID_IPersistFile,(void**)&ppf);// Get a pointer to the IPersistFile interface.
 if(FAILED(res)){FailMessage(QueryInterface_txt,res,FMSG_WRITE_LOG); return 0;}
}
str.Buffer=lnkPath;
str.MaximumLength=str.Length=(USHORT)getstrlen(lnkPath); str.MaximumLength++;
ustr.MaximumLength=(USHORT)(str.MaximumLength*2);
ustr.Buffer=(wchar_t*)LocalAlloc(LMEM_FIXED,ustr.MaximumLength);
ustr.Length=0;
if(ntdllFunctions.RtlAnsiStringToUnicodeString(&ustr,&str,0)!=0)return 0;// Ensure that the string is Unicode.
*(ustr.Buffer+str.Length)=0;
if(FAILED(ppf->Load(ustr.Buffer,STGM_READ)))goto quit;// Load the shortcut.
res=pshl->GetPath(tbuf3,300,NULL,SLGP_UNCPRIORITY);// Get path and file name of a shell link object.
if(SUCCEEDED(pshl->GetIconLocation(t1buf2,300,&in))){
 char *ptr;
 if(*t1buf2==0){
  if(FAILED(res))goto quit;
  ptr=tbuf3;
 }
 else ptr=t1buf2;
 *ImgNum=AddIconToList(ptr);
}
if(SUCCEEDED(res))pret=tbuf3;
quit:
if(ustr.Buffer)LocalFree(ustr.Buffer);
return pret;}

//////////////////////// Enum Executables and Shortcuts at Path ////////////////
void EnumExecInFolder(){
HANDLE search; WIN32_FIND_DATA *fdata; LV_ITEM lvi; fdata=(WIN32_FIND_DATA*)t1nbuf1; int ik; char *runstr;
ik=lstrlen(t1buf1);
if(*(t1buf1+ik)!='\\'){*(t1buf1+ik)='\\'; ik++; *(t1buf1+ik)=0;}
lstrcpy(t1buf1+ik,"*.*");
search=FindFirstFile(t1buf1,fdata);
if(search==INVALID_HANDLE_VALUE)return;
do{
 if(fdata->cFileName[0]==0x2E)continue;
 if(fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;
 runstr=fdata->cFileName;
 in=lstrlen(runstr); in-=4; runstr+=in; in=0;
 lstrcpy(t1buf1+ik,fdata->cFileName);
 if(lstrcmpi(runstr,".exe")==0 || lstrcmpi(runstr,".bat")==0 || lstrcmpi(runstr,".cmd")==0 || lstrcmpi(runstr,".com")==0){in=1;runstr=t1buf1;}
 lvi.iItem=LItem++; lvi.lParam=ITEM_PHYSPATH;
 lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
 lvi.iSubItem=0; lvi.pszText=fdata->cFileName; lvi.iImage=0;
 if(!in)runstr=ResolveLinkFile(t1buf1,&lvi.iImage);
 else {in=0; lvi.iImage=AddIconToList(runstr);}
 SendMessage(StRunList,LVM_INSERTITEM,0,(LPARAM)&lvi);
 if(runstr){
  lvi.iSubItem=1; lvi.pszText=runstr; lvi.mask=LVIF_TEXT;
  SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
 }
 *(t1buf1+ik)=0;
 lvi.iSubItem=2; lvi.pszText=t1buf1; lvi.mask=LVIF_TEXT;
 SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
}
while(FindNextFile(search,fdata));
}

////////////////////////////// Query Reg String Value //////////////////////////
void QueryRegSZValue(HKEY hKey,char *hkeystr, char *szKey, char *pData, char *sz1, char *sz2, LV_ITEM *lvi){DWORD dw;
if(RegQueryValueEx(hKey,szKey,NULL,&dw,(LPBYTE)pData,&n)==ERROR_SUCCESS && n>1 && (dw==REG_SZ || dw==REG_EXPAND_SZ || dw==REG_MULTI_SZ)){
 nextMultiStr:
 lvi->iItem=LItem++; lvi->lParam=ITEM_REGVALUE;
 lvi->mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
 lvi->iSubItem=0; lvi->pszText=szKey;
 SendMessage(StRunList,LVM_INSERTITEM,0,(LPARAM)lvi);
 lvi->iSubItem=1; lvi->pszText=pData;
 SendMessage(StRunList,LVM_SETITEMTEXT,lvi->iItem,(LPARAM)lvi);
 lstrcpy(t1buf2,hkeystr);
 if(sz1!=NULL)lstrcat(t1buf2,sz1);
 if(sz2!=NULL)lstrcat(t1buf2,sz2);
 lvi->iSubItem=2; lvi->pszText=t1buf2; lvi->mask=LVIF_TEXT;
 SendMessage(StRunList,LVM_SETITEMTEXT,lvi->iItem,(LPARAM)lvi);
 if(dw==REG_MULTI_SZ){pData+=lstrlen(pData)+1; if(*pData!=0)goto nextMultiStr;}
}}

///////////////////////////////// RegKeyEnumValues /////////////////////////////
void RegKeyEnumValues(HKEY hKey,char* hkeystr,char* runstr){LONG ret; DWORD t,k; int i; LV_ITEM lvi;
i=0; do{
 n=300; k=300;
 ret=RegEnumValue(hKey,i,t1buf1,&n,NULL,&t,(LPBYTE)t1buf2,&k);
 if(ret==ERROR_SUCCESS && (t==REG_SZ || t==REG_EXPAND_SZ)){
  if(k<2 && n<1)goto nextvalue;
  lvi.iItem=LItem++; lvi.lParam=ITEM_REGVALUE;
  lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
  lvi.iSubItem=0; lvi.pszText=t1buf1; lvi.iImage=1;
  SendMessage(StRunList,LVM_INSERTITEM,0,(LPARAM)&lvi);
  lvi.iSubItem=1; lvi.pszText=t1buf2; lvi.mask=LVIF_TEXT;
  SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
  lstrcpy(t1buf2,hkeystr);
  if(*tbuf3!=0)lstrcat(t1buf2,tbuf3);
  if(runstr!=0)lstrcat(t1buf2,runstr);
  lvi.iSubItem=2; lvi.pszText=t1buf2; lvi.mask=LVIF_TEXT;
  SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
 }
 nextvalue: i++;
}while(ret!=ERROR_NO_MORE_ITEMS);
}

/////////////////////////////// ShowStartupEntries /////////////////////////////
void ShowStartupEntries(){
HKEY hMainKey,hKey; int i,ik,m; char *runstr,*hkeystr; BYTE stpTypeShow; LV_ITEM lvi;
SendMessage(StRunList,LVM_DELETEALLITEMS,0,0);
if(!CoInitOk && CoInitializeEx(0,COINIT_APARTMENTTHREADED)==S_OK)CoInitOk=1;
if(IsDlgButtonChecked(strtrun_win,102)==BST_CHECKED)stpTypeShow=STRTUP_ALL_ENTRIES;
else if(IsDlgButtonChecked(strtrun_win,103)==BST_CHECKED)stpTypeShow=STRTUP_GENERAL_ENTRIES;
else if(IsDlgButtonChecked(strtrun_win,104)==BST_CHECKED)stpTypeShow=STRTUP_OTHER_ENTRIES;
LItem=0; hkeystr=(char*)HkeyStr; ik=0; m=lstrlen(mainkeyStr); m++;
viewHKey:
if(stpTypeShow&STRTUP_GENERAL_ENTRIES){
//HKLM Run,RunOnce,etc.
lstrcpy(tbuf3,mainkeyStr+m); lstrcat(tbuf3,mainkeyStr); m+=lstrlen(mainkeyStr+m); m++;
if(RegOpenKey(Hkeys[ik],tbuf3,&hMainKey)==ERROR_SUCCESS){
 runstr=(char*)RunStr;
 viewRunKey:
 if(RegOpenKey(hMainKey,runstr,&hKey)==ERROR_SUCCESS){
  RegKeyEnumValues(hKey,hkeystr,runstr);
  RegCloseKey(hKey);
 }
 runstr+=lstrlen(runstr)+1;
 if(*(BYTE*)runstr!=0)goto viewRunKey;
 RegCloseKey(hMainKey);
}
ik++;
if(ik<3){hkeystr+=lstrlen(hkeystr)+1; goto viewHKey;}
lstrcpy(tbuf3,mainkeyStr); lstrcat(tbuf3,PolicyRun);
if(RegOpenKey(HKEY_LOCAL_MACHINE,tbuf3,&hKey)==ERROR_SUCCESS){
 RegKeyEnumValues(hKey,HkeyStr,0);
 RegCloseKey(hKey);
}
lstrcpy(tbuf3,mainkeyStr); lstrcat(tbuf3,PolicyRun);
if(RegOpenKey(HKEY_CURRENT_USER,tbuf3,&hKey)==ERROR_SUCCESS){
 RegKeyEnumValues(hKey,HkeyStr+OFS_HKCU,0);
 RegCloseKey(hKey);
}
// HKLM & HKCR Windows NT sub-keys 1
ik=0; hkeystr=HkeyStr;
viewNThKey:
lstrcpy(tbuf3,NTkeyStr);
if(RegOpenKey(Hkeys[ik],tbuf3,&hMainKey)==ERROR_SUCCESS){
 runstr=(char*)RunStr;
 viewRunNTKey:
 if(RegOpenKey(hMainKey,runstr,&hKey)==ERROR_SUCCESS){
  RegKeyEnumValues(hKey,hkeystr,runstr);
  RegCloseKey(hKey);
 }
 runstr+=lstrlen(runstr)+1;
 if(*(BYTE*)runstr!=0)goto viewRunNTKey;
 RegCloseKey(hMainKey);
}
ik++;
if(ik<2){hkeystr+=OFS_HKCU; goto viewNThKey;}
}
if(stpTypeShow&STRTUP_OTHER_ENTRIES){
// HKLM & HKCR Windows NT key: Load & Run values
ik=0; m=0; lvi.iImage=1;
lstrcpy(tbuf3,NTkeyStr);
hkeystr=HkeyStr; runstr=(char*)NTkeyVlStr;
viewNThKey2:
if(RegOpenKey(Hkeys[ik],tbuf3,&hMainKey)==ERROR_SUCCESS){
 n=300;
 QueryRegSZValue(hMainKey,hkeystr,runstr,t1buf1,tbuf3,runstr,&lvi);
 RegCloseKey(hMainKey);
}
ik++;
if(ik<2){hkeystr+=OFS_HKCU; goto viewNThKey2;}
m++;
if(m<2){
 ik=2; runstr+=lstrlen(runstr)+1;
 hkeystr=HkeyStr;
 goto viewNThKey2;
}
lvi.iImage=3;
// HKLM Windows NT: Winlogon sub-key values
lstrcpyn(tbuf3,NTkeyStr,lstrlen(NTkeyStr)-7); lstrcat(tbuf3,"Winlogon");
if(RegOpenKey(HKEY_LOCAL_MACHINE,tbuf3,&hKey)==ERROR_SUCCESS){
 runstr=(char*)Keys2;
 viewWinlogonValues:
 n=300;
 QueryRegSZValue(hKey,HkeyStr,runstr,t1buf1,tbuf3,0,&lvi);
 runstr+=lstrlen(runstr)+1;
 if(*(BYTE*)runstr!=0)goto viewWinlogonValues;
 RegCloseKey(hKey);
}
// HKLM System\CurrentControlSet\Control\SessionManager: value BootExecute  [NT/2K]
if(RegOpenKey(HKEY_LOCAL_MACHINE,Key3,&hKey)==ERROR_SUCCESS){
 n=300;
 QueryRegSZValue(hKey,HkeyStr,BootExecute,t1buf1,Key3,0,&lvi);
 RegCloseKey(hKey);
}
lvi.iImage=3;
// HKLM Windows NT: App_InitDLL sub-key
lstrcpy(tbuf3,NTkeyStr);lstrcat(tbuf3,AppInit_DLLs);
if(RegOpenKey(HKEY_LOCAL_MACHINE,NTkeyStr,&hMainKey)==ERROR_SUCCESS){
 n=300;
 QueryRegSZValue(hMainKey,HkeyStr,AppInit_DLLs,t1buf1,NTkeyStr,0,&lvi);
 RegCloseKey(hMainKey);
}
// запуск программ вместе с запуском командного процессора CMD.EXE
if(RegOpenKey(HKEY_LOCAL_MACHINE,CMD_Run,&hMainKey)==ERROR_SUCCESS){
 n=300;
 QueryRegSZValue(hMainKey,HkeyStr,AutoRun,t1buf1,CMD_Run,0,&lvi);
 RegCloseKey(hMainKey);
}
hkeystr=HkeyStr+OFS_HKCU;
if(RegOpenKey(HKEY_CURRENT_USER,CMD_Run,&hMainKey)==ERROR_SUCCESS){
 n=300;
 QueryRegSZValue(hMainKey,hkeystr,AutoRun,t1buf1,CMD_Run,0,&lvi);
 RegCloseKey(hMainKey);
}
// Explorer BHO objects
lstrcpy(t1buf2,HkeyStr); lstrcat(t1buf2,Key4);
if(RegOpenKey(HKEY_LOCAL_MACHINE,Key4,&hMainKey)==ERROR_SUCCESS){
 i=0;
 do{
  n=300;
  ik=RegEnumKeyEx(hMainKey,i,t1buf1,&n,NULL,NULL,NULL,NULL);
  if(ik==ERROR_SUCCESS){
   lvi.iItem=LItem++; lvi.lParam=ITEM_1|ITEM_REGSUBKEY;
   n--;
   if(*t1buf1=='{' && *(t1buf1+n)=='}' && *(t1buf1+9)=='-' && *(t1buf1+14)=='-' && *(t1buf1+19)=='-' && *(t1buf1+24)=='-'){
    lvi.lParam|=ITEM_CLSID;
   }
   lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
   lvi.iSubItem=0; lvi.pszText="Explorer BHO"; lvi.iImage=3;
   SendMessage(StRunList,LVM_INSERTITEM,0,(LPARAM)&lvi);
   lvi.iSubItem=1; lvi.pszText=t1buf1; lvi.mask=LVIF_TEXT;
   SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
   lvi.iSubItem=2; lvi.pszText=t1buf2; lvi.mask=LVIF_TEXT;
   SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
  }
  i++;
 }while(ik!=ERROR_NO_MORE_ITEMS);
 RegCloseKey(hMainKey);
}
}
if(stpTypeShow&STRTUP_GENERAL_ENTRIES){
 // startup-folder of local machine
 lstrcpy(tbuf3,mainkeyStr); lstrcat(tbuf3,ShellFolders_txt);
 if(RegOpenKey(HKEY_LOCAL_MACHINE,tbuf3,&hKey)==ERROR_SUCCESS){
  n=300; if(RegQueryValueEx(hKey,"Common Startup",NULL,NULL,(LPBYTE)t1buf1,&n)==ERROR_SUCCESS)EnumExecInFolder();
  RegCloseKey(hKey);
 }
 // startup-folder of current user
 lstrcpy(tbuf3,mainkeyStr); lstrcat(tbuf3,ShellFolders_txt);
 if(RegOpenKey(HKEY_CURRENT_USER,tbuf3,&hKey)==ERROR_SUCCESS){
  n=300; if(RegQueryValueEx(hKey,"Startup",NULL,NULL,(LPBYTE)t1buf1,&n)==ERROR_SUCCESS)EnumExecInFolder();
  RegCloseKey(hKey);
 }
}
if(stpTypeShow&STRTUP_OTHER_ENTRIES){
//enum PROGRAM FILES\INTERNET EXPLORER\PLUGINS
n=300; if(GetProgramFilesDir(t1buf1,&n)){
HANDLE search; WIN32_FIND_DATA *fdata; fdata=(WIN32_FIND_DATA*)t1nbuf3;
ik=n-1;
if(*(t1buf1+ik)!='\\'){*(t1buf1+ik)='\\'; ik++; *(t1buf1+ik)=0;}
lstrcat(t1buf1,"Internet Explorer\\Plugins\\"); ik=lstrlen(t1buf1);
lstrcpy(t1buf1+ik,"*.*");
search=FindFirstFile(t1buf1,fdata);
if(search==INVALID_HANDLE_VALUE)return;
do{
 if(fdata->cFileName[0]==0x2E)continue;
 if(fdata->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)continue;
 lvi.iItem=LItem++; lvi.lParam=ITEM_PHYSPATH|ITEM_1;
 lvi.mask=LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
 lvi.iSubItem=0; lvi.pszText="IExplorer Plugin"; lvi.iImage=2;
 lstrcpy(t1buf1+ik,fdata->cFileName);
 SendMessage(StRunList,LVM_INSERTITEM,0,(LPARAM)&lvi);
 lvi.iSubItem=1; lvi.pszText=fdata->cFileName; lvi.mask=LVIF_TEXT;
 SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
 *(t1buf1+ik)=0;
 lvi.iSubItem=2; lvi.pszText=t1buf1; lvi.mask=LVIF_TEXT;
 SendMessage(StRunList,LVM_SETITEMTEXT,lvi.iItem,(LPARAM)&lvi);
}
while(FindNextFile(search,fdata));
}
}
//view boot.ini: /kernel=filename
lstrcpy(t1buf1,smemTable->Total_); ltoa(LItem,t1buf1+lstrlen(t1buf1),10);
SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)t1buf1);
}

//////////////////////////////// Resize Window /////////////////////////////////
void resizeStartupWin(LONG newsz,LONG newwd){HWND htmp; POINT pt; HDWP hdwp,hdwpcur; int hplus,wplus;
hplus=newsz; hplus-=initHg;
wplus=newwd; wplus-=initWd;
hdwp=BeginDeferWindowPos(6);
if(hdwp==0)return;
hdwpcur=hdwp;
DeferWindowPos(hdwpcur,StRunList,0,0,0,listsz.right+wplus,listsz.bottom+hplus,SWP_NOZORDER|SWP_NOMOVE);
htmp=GetDlgItem(strtrun_win,110);
if(htmp!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,sz2[0].left,sz2[0].top+hplus,0,0,SWP_NOZORDER|SWP_NOSIZE);}
htmp=GetDlgItem(strtrun_win,113);
if(htmp!=NULL){hdwpcur=DeferWindowPos(hdwpcur,htmp,0,sz2[3].left,sz2[3].top+hplus,0,0,SWP_NOZORDER|SWP_NOSIZE);}
htmp=GetDlgItem(strtrun_win,111);
if(htmp!=NULL){
 pt.x=newwd; pt.x-=sz2[1].left; pt.x-=8;
 hdwpcur=DeferWindowPos(hdwpcur,htmp,0,sz2[1].left,sz2[1].top+hplus,pt.x,sz2[1].bottom,SWP_NOZORDER);
}
htmp=GetDlgItem(strtrun_win,112);
if(htmp!=NULL){
 pt.x=newwd; pt.x-=sz2[2].left; pt.x-=8;
 hdwpcur=DeferWindowPos(hdwpcur,htmp,0,sz2[2].left,sz2[2].top+hplus,pt.x,sz2[2].bottom,SWP_NOZORDER);
}
htmp=GetDlgItem(strtrun_win,114);
if(htmp!=NULL){
 pt.x=newwd; pt.x-=sz2[4].left; pt.x-=3;
 DeferWindowPos(hdwpcur,htmp,0,sz2[4].left,sz2[4].top+hplus,pt.x,sz2[4].bottom,SWP_NOZORDER);
}
EndDeferWindowPos(hdwp);
}

//////////////////////////////////// On Init ///////////////////////////////////
void OnInit_RunEntries(){DWORD n;
himgl=ImageList_Create(16,16,ILC_COLORDDB,4,4);
ImageList_SetBkColor(himgl,0x00ffffff);
n=10; while(n<=13){ImageList_AddIcon(himgl,LoadIcon(gInst,(char*)n)); n++;}
listsz.left=3;
listsz.top=20;
listsz.right=pHdr->rcDisplay.right-10;
listsz.bottom=pHdr->rcDisplay.bottom-137+58;
StRunList=GetDlgItem(strtrun_win,101);
SetWindowPos(StRunList,0,listsz.left,listsz.top,listsz.right,listsz.bottom,SWP_NOZORDER);
lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
lvcol.fmt=LVCFMT_LEFT;
lvcol.cx=StartupColWidth[0]; lvcol.pszText=smemTable->Item; lvcol.iSubItem=0;
SendMessage(StRunList,LVM_INSERTCOLUMN,(WPARAM)0,(LPARAM)&lvcol);
lvcol.cx=StartupColWidth[1]; lvcol.pszText=smemTable->ItemParameters; lvcol.iSubItem=1;
SendMessage(StRunList,LVM_INSERTCOLUMN,(WPARAM)1,(LPARAM)&lvcol);
lvcol.cx=StartupColWidth[2]; lvcol.pszText=smemTable->Location; lvcol.iSubItem=2;
SendMessage(StRunList,LVM_INSERTCOLUMN,(WPARAM)2,(LPARAM)&lvcol);
n=LVS_MY_EXTENDED_STYLE; if(progSetts.CheckState[17]==MFS_CHECKED)n|=LVS_EX_GRIDLINES;
SendMessage(StRunList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
ListView_SetImageList(StRunList,himgl,LVSIL_SMALL);
CheckMarkMenuItems();
}

///////////////////////////////// RunEntries_Proc //////////////////////////////
BOOL CALLBACK RunEntries_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){LV_ITEM lvi;
if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
 if(Message==WM_INITMENUPOPUP || Message==WM_UNINITMENUPOPUP || Message==WM_MEASUREITEM || Message==WM_DRAWITEM || Message==WM_ENTERMENULOOP || Message==WM_EXITMENULOOP){
  ModernMenu_DrawMenuProc(hDlg,Message,wParam,lParam);
 }
}
switch(Message){
case WM_INITDIALOG:
 strtrun_win=hDlg;
 initHg=pHdr->rcDisplay.bottom;
 initWd=pHdr->rcDisplay.right-4;
 HWND htmp; POINT pt; int hpls,i; hpls=initHg-137;
 for(i=0;i<5;i++){//110...114
  htmp=GetDlgItem(strtrun_win,i+110);
  if(htmp!=NULL){
   GetWindowRect(htmp,&clt); pt.y=clt.top; pt.x=clt.left;
   ScreenToClient(strtrun_win,&pt);
   sz2[i].left=pt.x;
   sz2[i].top=pt.y+hpls;
   sz2[i].bottom=clt.bottom; sz2[i].bottom-=clt.top;
   sz2[i].right=clt.right; sz2[i].right-=clt.left;
  }
 }
 WindowFillText(hDlg,dlgTAT->dlg5);
 MainTab_OnChildDialogInit(hDlg); pHdr->hwndDisplay=hDlg;
 OnInit_RunEntries();
 itemSel=-1; pshl=NULL; ppf=NULL; SetFocus(StRunList);
 if(progSetts.LastStartupEnumButn<102 || progSetts.LastStartupEnumButn>104)progSetts.LastStartupEnumButn=102;
 CheckRadioButton(hDlg,102,104,progSetts.LastStartupEnumButn);
 ExecuteAddr=(DWORD)ShowStartupEntries;
 ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0xFE);
 break;
case WM_DESTROY:
 if(ImageList_Destroy(himgl))himgl=NULL;
 if(StRunList!=NULL && DestroyWindow(StRunList))StRunList=NULL;
 DeleteMenu(main_menu,30017,MF_BYCOMMAND);
 DeleteMenu(main_menu,47401,MF_BYCOMMAND);
 DeleteMenu(main_menu,47402,MF_BYCOMMAND);
 SendMessage(hstatus,SB_SETTEXT,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)0);
 strtrun_win=NULL;
 if(pshl)pshl->Release(); if(ppf)ppf->Release();
 if(CoInitOk){CoUninitialize();CoInitOk=0;}
 break;
case WM_COMMAND: wID=LOWORD(wParam);
 switch(wID){
  case 102: case 103: case 104: progSetts.LastStartupEnumButn=wID;
  case 50010://Refresh
   ExecuteAddr=(DWORD)ShowStartupEntries;
   ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0xFE);
   break;
  case 30051://clsid info
   get_clsid_info:
   if((itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.iItem=itemSel; *t0buf1=0;
   lvi.iSubItem=1; lvi.pszText=t0buf1; lvi.cchTextMax=300;
   lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.lParam=0;
   SendMessage(StRunList,LVM_GETITEM,itemSel,(LPARAM)&lvi);
   if(lvi.lParam&ITEM_CLSID){
    CLSID_INFO_DLGPARAMS cid; cid.bDialogBox=1; cid.clsid=t0buf1;
    DialogBoxParam(gInst,(char*)16,strtrun_win,(DLGPROC)CLSIDinfo_Proc,(LPARAM)&cid);
   }
   break;
  case 30050://jump to Location (HKEY or Folder)
   if((itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.iItem=itemSel; *t0buf1=0;
   lvi.iSubItem=2; lvi.pszText=t0buf1; lvi.cchTextMax=300;
   lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.lParam=0;
   SendMessage(StRunList,LVM_GETITEM,itemSel,(LPARAM)&lvi);
   if(*(DWORD*)t0buf1==0x59454B48)JumpToRegedit(t0buf1,0);
   else if(lvi.lParam&ITEM_PHYSPATH){ShellExecute(strtrun_win,open_txt,t0buf1,NULL,NULL,SW_SHOWNORMAL);}
   break;
  case 30052://jump to HKEY_CLASSES_ROOT\\CLSID\\
   if((itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.iItem=itemSel; lstrcpy(t0buf1,HKEYCLASSESROOT_CLSID);
   lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.lParam=0;
   lvi.iSubItem=1; lvi.pszText=t0buf1+24; lvi.cchTextMax=300-24;
   SendMessage(StRunList,LVM_GETITEM,itemSel,(LPARAM)&lvi);
   if(lvi.lParam&ITEM_CLSID)JumpToRegedit(t0buf1,0);
   break;
  case 30053://Jump to Path
   if((itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.iItem=itemSel; *t0buf1=0; lvi.iSubItem=1; lvi.pszText=t0buf1;
   lvi.cchTextMax=300; lvi.mask=LVIF_PARAM|LVIF_TEXT; lvi.lParam=0;
   SendMessage(StRunList,LVM_GETITEM,itemSel,(LPARAM)&lvi);
   if(*t0buf1==0)break;
   if(!IsPathValid(t0buf1))break;
   OpenFolderAndSelectFile(t0buf1,strtrun_win);
   break;
  case 30054://Delete item
   if((itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))!=-1)DeleteStartupItem();
   break;
  case 30017:
   n=LVS_MY_EXTENDED_STYLE;
   if(progSetts.CheckState[17]==MFS_CHECKED)progSetts.CheckState[17]=MFS_UNCHECKED;
   else {progSetts.CheckState[17]=MFS_CHECKED; n|=LVS_EX_GRIDLINES;}
   CheckMenuItem(main_menu,30017,progSetts.CheckState[17]);
   SendMessage(StRunList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
   break;
 }
 break;
case WM_SIZE: resizeStartupWin(HIWORD(lParam),LOWORD(lParam)); break;
case WM_NOTIFY:
 DWORD nfcode;
 nfcode=((NMHDR*)lParam)->code;
 if(((NMHDR*)lParam)->hwndFrom!=StRunList)break;
 if(nfcode==NM_CLICK || nfcode==NM_RCLICK || nfcode==NM_RETURN){
  itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED);
  if(SendMessage(StRunList,LVM_GETITEMCOUNT,0,0)==itemSel)break;// ???
  adtnl_info:
  DWORD dw;
  if(itemSel==-1)dw=MF_BYCOMMAND|MF_GRAYED;
  else dw=MF_BYCOMMAND|MF_ENABLED;
  EnableMainMenuItem(47401,dw);
  if(itemSel==-1)break;
  if(itemSel!=prevSel){
   prevSel=itemSel; lvi.lParam=0;
   lvi.iItem=itemSel; *t0buf1=0; lvi.mask=LVIF_PARAM|LVIF_TEXT;
   lvi.iSubItem=1; lvi.pszText=t0buf1; lvi.cchTextMax=300;
   SendMessage(StRunList,LVM_GETITEM,itemSel,(LPARAM)&lvi);
   SendMessage(GetDlgItem(strtrun_win,111),WM_SETTEXT,0,(LPARAM)t0buf1);
   if(!IsPathValid(t0buf1))dw=MF_BYCOMMAND|MF_GRAYED;
   else dw=MF_BYCOMMAND|MF_ENABLED;
   EnableMenuItem(main_menu,30053,dw);
   if(lvi.lParam&ITEM_CLSID)dw=MF_BYCOMMAND|MF_ENABLED;
   else dw=MF_BYCOMMAND|MF_GRAYED;
   EnableMenuItem(main_menu,30051,dw);
   EnableMenuItem(main_menu,30052,dw);
   lvi.iSubItem=2;
   SendMessage(StRunList,LVM_GETITEMTEXT,itemSel,(LPARAM)&lvi);
   SendMessage(GetDlgItem(strtrun_win,112),WM_SETTEXT,0,(LPARAM)t0buf1);
  }
  if(nfcode==NM_RCLICK)CreateRMenu(((LPNMLISTVIEW)lParam)->iItem);
 }
 else if(nfcode==LVN_KEYDOWN && ((LPNMLVKEYDOWN)lParam)->wVKey==VK_DELETE){
  if((itemSel=SendMessage(StRunList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
  DeleteStartupItem(); prevSel=-1;
 }
 else if(nfcode==LVN_ITEMCHANGED){
  if(((LPNMLISTVIEW)lParam)->uChanged&LVIF_STATE && ((LPNMLISTVIEW)lParam)->uNewState&LVIS_SELECTED){
   itemSel=((LPNMLISTVIEW)lParam)->iItem;
   goto adtnl_info;
  }
 }
 else if(nfcode==NM_DBLCLK)goto get_clsid_info;
 break;
} return 0;}

/*
LOWORD = NE or PE and HIWORD = 3.0, 3.5, or 4.0  Windows application 
LOWORD = MZ and HIWORD = 0  MS-DOS .exe, .com, or .bat file 
LOWORD = PE and HIWORD = 0  Win32 console application 
*/
