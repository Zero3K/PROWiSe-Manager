
#include "MenuStyle.cpp"

// --
HMENU BuildMenu(MenuITEM *mnuit){HMENU hmenu,hsub; int p,i; MENUITEMINFO mii;
if((hmenu=CreateMenu())==0)return 0;
mii.cbSize=sizeof(MENUITEMINFO);
hsub=0; mii.fState=MFS_ENABLED;
for(i=0;mnuit[i].SubMenu!=0xFF;i++){
 if(mnuit[i].iMask==MIIM_SUBMENU){
  p=i+1; hsub=CreateMenu();
  for(;mnuit[p].SubMenu==1;p++){
   mii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID;
   mii.fType=mnuit[p].iType;
   mii.dwTypeData=mnuit[p].iText;
   mii.wID=mnuit[p].wID;
   if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT && mnuit[p].nIcon!=0){
    if((mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(mnuit[p].nIcon))!=0)mii.fMask|=MIIM_DATA;
   }
   InsertMenuItem(hsub,0,1,&mii);
  }
  mii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID|MIIM_SUBMENU;
 }
 else mii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID;
 mii.fType=mnuit[i].iType;
 mii.dwTypeData=mnuit[i].iText;
 mii.wID=mnuit[i].wID;
 mii.hSubMenu=hsub;
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT && mnuit[i].nIcon!=0){
  if((mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(mnuit[i].nIcon))!=0)mii.fMask|=MIIM_DATA;
 }
 InsertMenuItem(hmenu,0,1,&mii);
 if(hsub){hsub=0; i=p-1;}
}
if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT)ModernMenu_SetMenuStyle(0,hmenu,progSetts.MenuStyle);
return hmenu;}

// --
HMENU BuildPopupMenu(MenuITEM *mnuit){HMENU hmenu,hsub; int p,i; MENUITEMINFO mii;
if((hmenu=CreatePopupMenu())==0)return 0;
mii.cbSize=sizeof(MENUITEMINFO);
hsub=0; mii.fState=MFS_ENABLED;
for(i=0;mnuit[i].SubMenu!=0xFF;i++){
 if(mnuit[i].iMask==MIIM_SUBMENU){
  p=i+1; hsub=CreateMenu();
  for(;mnuit[p].SubMenu==1;p++){
   mii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID;
   mii.fType=mnuit[p].iType;
   mii.dwTypeData=mnuit[p].iText;
   mii.wID=mnuit[p].wID;
   if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT && mnuit[p].nIcon!=0){
    if((mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(mnuit[p].nIcon))!=0)mii.fMask|=MIIM_DATA;
   }
   InsertMenuItem(hsub,0,1,&mii);
  }
  mii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID|MIIM_SUBMENU;
 }
 else mii.fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID;
 mii.fType=mnuit[i].iType;
 mii.dwTypeData=mnuit[i].iText;
 mii.wID=mnuit[i].wID;
 mii.hSubMenu=hsub;
 if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT && mnuit[i].nIcon!=0){
  if((mii.dwItemData=(DWORD)ModernMenu_ImgGetIcon(mnuit[i].nIcon))!=0)mii.fMask|=MIIM_DATA;
 }
 InsertMenuItem(hmenu,0,1,&mii);
 if(hsub){hsub=0; i=p-1;}
}
return hmenu;}

// --
void WindowFillText(HWND hDlg, DLGITEM_TEXT* dgitxt){char **dptr;
 if(dgitxt==0 || (DWORD)dgitxt==0xBAADF00D)return;
 while(dgitxt->nIDDlgItem!=(DWORD)-1){
  dptr=(char**)(((DWORD_PTR)smemTable)+(DWORD)(dgitxt->lpString));
  SetDlgItemText(hDlg,dgitxt->nIDDlgItem,*dptr);
  dgitxt++;
 }
}

// --
HANDLE OpenLngFile(char *fPath,char **pheader){HANDLE LngFile; DWORD n;
 if((LngFile=CreateFileA(fPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL))==INVALID_HANDLE_VALUE)return 0;
 SetFilePointer(LngFile,0,NULL,FILE_BEGIN);
 if(ReadFile(LngFile,*pheader,LNGFILE_OFFSET_TEXTDATA,&n,NULL)==0 || n<(LNGFILE_OFFSET_TEXTDATA))goto closehndl;
 if(*(DWORD*)*pheader!=LNGFILE_COMPATIBILITY_FLAG)goto closehndl;
 *pheader+=LNGFILE_OFFSET_LNGID+4;
 return(LngFile);
 closehndl:
 CloseHandle(LngFile);
 return 0;
}

// --
BOOL EnumLngFiles(char *lngfl, BYTE bCmd){HANDLE search,LngFile; WIN32_FIND_DATA fdata; DWORD n; LNGITEM_INFO *lnginf;
BOOL isFirstfl,isLngFileFound=0; int strsz,len,memsz; char *nbuf,*pheader,*membuf;
if((nbuf=AllocPathBuffer())==0)return 0;
memsz=LNGFILE_OFFSET_TEXTDATA;
membuf=(char*)LocalAlloc(LMEM_FIXED,memsz);
strsz=ProgramDirPath.CharCount;
if(strsz<2)goto exit;
if(bCmd==2)SetCurrentDirectory((char*)nbuf);
strappend(nbuf,"*.lng");
if((search=FindFirstFile(nbuf,&fdata))==INVALID_HANDLE_VALUE)goto exit;
if(lngfl){*lngfl=0; isFirstfl=1;} else isFirstfl=0;
if(bCmd==1 && lngFirstInf){
 void *pnext;
 lnginf=lngFirstInf;
 walkstruct:
  pnext=lnginf->NextEntry;
  LocalFree(lnginf);
 if(pnext!=0){lnginf=(LNGITEM_INFO*)pnext; goto walkstruct;}
 lngFirstInf=0;
}
do{
 if(fdata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY || fdata.cFileName[0]==0x2E)continue;
 copystring((nbuf+strsz),fdata.cFileName);
 if(isFirstfl){copystring(lngfl,fdata.cFileName);isFirstfl=0;}
 pheader=membuf;
 if((LngFile=OpenLngFile(nbuf,&pheader))==0)continue;
 n=*(DWORD*)pheader; pheader+=4;//Get LanguageID DWORD and skip it with an extend byte
 if(n==progSetts.LanguageID){
  isLngFileFound=1;
  if(lngfl){
   copybytes(lngfl,nbuf,strsz,1); copystring(lngfl+strsz,fdata.cFileName);
   int lnn; lnn=getstrlen(fdata.cFileName); lnn+=5;
   ActiveLngHelp=(char*)LocalAlloc(LMEM_FIXED,lnn); lnn-=9;
   copybytes(ActiveLngHelp,fdata.cFileName,lnn,1);
   *(DWORD*)(ActiveLngHelp+lnn)=0;
   strappend(ActiveLngHelp,_chm__txt);
  }
 }
 if(bCmd==1){
  len=getstrlen(fdata.cFileName); len+=sizeof(LNGITEM_INFO)+1; len+=*((DWORD*)pheader); len+=*((DWORD*)pheader+1);
  if(lngFirstInf==0){lnginf=lngFirstInf=(LNGITEM_INFO*)LocalAlloc(LPTR,len);}
  else{
   lnginf->NextEntry=(LNGITEM_INFO*)LocalAlloc(LPTR,len);
   lnginf=(LNGITEM_INFO*)(lnginf->NextEntry);
  }
  lnginf->NextEntry=0; lnginf->lngID=n;
  lnginf->fileName=(char*)lnginf; lnginf->fileName+=sizeof(LNGITEM_INFO);
  lnginf->LanguageInfo=lnginf->fileName;
  lnginf->LanguageInfo+=copystring(lnginf->fileName,fdata.cFileName);
  lnginf->LanguageInfo++; len=*((DWORD*)pheader); len+=*((DWORD*)pheader+1);
  if(memsz<len){
   if((pheader=(char*)LocalReAlloc(membuf,len,LMEM_MOVEABLE))!=0){
    membuf=pheader; memsz=len;
   }
  }
  if(memsz>=len && ReadFile(LngFile,membuf,len,&n,NULL)!=0 && n>=(DWORD)len){
   copybytes(lnginf->LanguageInfo,membuf,len,0);
  }
 }
 CloseHandle(LngFile);
 if(bCmd!=1 && isLngFileFound)break;
}while(FindNextFile(search,&fdata));
FindClose(search); SetLastError(0);
exit:
LocalFree(membuf);
LocalFree(nbuf);
return isLngFileFound;
}

// --
BYTE FillResourcesAllocationTable(char *pLngName){HANDLE LngFile; bool isOK; isOK=0; int strsz; char *lngfl; DWORD tflag; DWORD nm,n;
 strsz=UnicodeStringToAnsiString(ProgramDirPath.fPath,ProgramDirPath.pathLen,(char*)t0nbuf1,ProgramDirPath.CharCount+1);
 if(strsz<2)return 0;
 lngfl=(char*)LocalAlloc(LMEM_FIXED,MAX_PATH);
 if(pLngName && *pLngName){
  copystring(lngfl,(char*)t0nbuf1);
  strappend(lngfl,pLngName);
  strappend(lngfl,".lng");
 }
 else{
enum_lngfiles:
  if(!EnumLngFiles(lngfl,2)){// Search for lng-file
   FailMessage("The file corresponding to the selected language isn't found.",0,FMSG_NO_INFO|FMSG_NO_SUCCESSINFO|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
  }
 }
 if(*lngfl)LngFile=CreateFileA(lngfl,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
 if(LngFile==INVALID_HANDLE_VALUE){
  if(pLngName){pLngName=0; goto enum_lngfiles;}
  LngFile=0; goto closeFile;
 }
 ///////
 IO_STATUS_BLOCK iob[1]; DWORD ErrCode; FILE_STANDARD_INFORMATION *fsi; fsi=(FILE_STANDARD_INFORMATION*)t0nbuf1;
 nm=2048; querysz:
 ErrCode=ntdllFunctions.pNtQueryInformationFile(LngFile,iob,t0nbuf1,nm,FileStandardInformation);
 if(ErrCode!=0){
  if(nm==2048){nm=sizeof(FILE_STANDARD_INFORMATION);goto querysz;}
  else {FailMessage("NQIF",ErrCode,FMSG_NTSTATUS_CONVERT|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG); goto closeFile;}
}
 SetFilePointer(LngFile,0,NULL,FILE_BEGIN);
 if(ReadFile(LngFile,&tflag,4,&n,NULL)==0 || n==0)goto closeFile;//12808
 fsi->EndOfFile.QuadPart-=4+LNGFILE_OFFSET_LNGID+12;
 if(tflag!=LNGFILE_COMPATIBILITY_FLAG || (int)(fsi->EndOfFile.QuadPart)<=0){
  copystring((char*)t0nbuf1,lngfl); strappend((char*)t0nbuf1," is not compatible with this version.");
  FailMessage((char*)t0nbuf1,0,FMSG_NO_SUCCESSINFO|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
  if(pLngName){pLngName=0; goto enum_lngfiles;}
  goto closeFile;
 }
 if(SetFilePointer(LngFile,LNGFILE_OFFSET_LNGID,NULL,FILE_CURRENT)==0xFFFFFFFF)goto closeFile;
 if(ReadFile(LngFile,t0nbuf2,12,&n,NULL)==0 || n<12)goto closeFile;
 ActiveLangID=*t0nbuf2;
 n=*(t0nbuf2+1); n+=*(t0nbuf2+2); n+=4;
 if(SetFilePointer(LngFile,n,NULL,FILE_CURRENT)==0xFFFFFFFF)goto closeFile;
 fsi->EndOfFile.QuadPart-=n;
 if((stringsMem=VirtualAllocEx((HANDLE)-1,0,(DWORD)fsi->EndOfFile.QuadPart,MEM_COMMIT,PAGE_READWRITE))==0)goto closeFile;
 if(ReadFile(LngFile,stringsMem,(DWORD)fsi->EndOfFile.QuadPart,&n,NULL)==0 || n<4)goto closeFile;
 isOK=1;
closeFile:
 if(lngfl)LocalFree(lngfl);
 if(LngFile)CloseHandle(LngFile);
 if(!isOK)return 0;
 smemTable=(STRINGS_MEM_TABLE*)VirtualAllocEx((HANDLE)-1,0,sizeof(STRINGS_MEM_TABLE),MEM_COMMIT,PAGE_READWRITE);
 DWORD_PTR str_end,smem_end,*psmem; char* pstr;
 pstr=(char*)stringsMem; fsi->EndOfFile.LowPart-=16;
 str_end=(DWORD_PTR)pstr; str_end+=fsi->EndOfFile.LowPart;
 psmem=(DWORD_PTR*)smemTable; smem_end=(DWORD_PTR)psmem; smem_end+=sizeof(STRINGS_MEM_TABLE);
 while((DWORD_PTR)psmem<smem_end){
  if(*pstr==0 || *(DWORD*)pstr==0x46464646)break;
  *psmem=(DWORD_PTR)pstr; psmem++;//fill item and switch to next
  while((DWORD_PTR)pstr<str_end && *pstr!=0)pstr++;//search end of string
  pstr++;
 }
 while((DWORD_PTR)pstr<str_end && *(DWORD*)pstr!=0x46464646)pstr++;//find FFFF
 if(*(DWORD*)pstr!=0x46464646)return 0;//file corrupted if no FFFF found
 /////// fill string addresses in menus
 pstr+=4; MenuITEM *pmni; DWORD_PTR *pdwp;
 pmni=(MenuITEM*)pstr;
 for(in=0;in<MenuITEMS_COUNT;in++){
  Menus[in]=pmni;
  while(pmni->SubMenu!=0xFF && (DWORD_PTR)pmni<str_end){
   pdwp=(DWORD_PTR*)((DWORD_PTR)smemTable+(DWORD_PTR)(pmni->iText));
   pmni->iText=(char*)*pdwp; pmni++;
  }
  pmni++;
 }
 /////// create and fill dialog_text alloc table
 dlgTAT=(DLGITEM_TEXT_TABLE*)LocalAlloc(LPTR,sizeof(DLGITEM_TEXT_TABLE));
 if(dlgTAT==0)return 0;
 DLGITEM_TEXT *dgtxt,**dtat;
 pstr=(char*)pmni; pstr+=4;
 dgtxt=(DLGITEM_TEXT*)pstr;
 dtat=(DLGITEM_TEXT**)dlgTAT;
 while(dgtxt->lpString!=0x5D3F3F5B && (DWORD_PTR)dgtxt<str_end){
  *dtat=dgtxt;
  while(dgtxt->nIDDlgItem!=0xFFFFFFFF && (DWORD_PTR)dgtxt<str_end)dgtxt++;
  dgtxt++;
  dtat++;
 }
 return 1;
}