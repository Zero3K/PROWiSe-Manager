
void ErrorStringFromNtError(DWORD ErrorCode,DWORD fOptions,void *strBuffer){char *perb; DWORD dwFlags,ln,DosError;
 bool allocBuf=0; if(strBuffer==0)return;
 DosError=ntdllFunctions.RtlNtStatusToDosError(ErrorCode);
 dwFlags=FORMAT_MESSAGE_FROM_SYSTEM;
 if(fOptions&ERRSTRING_ALLOCATE_BUFFER){allocBuf=1; dwFlags|=FORMAT_MESSAGE_ALLOCATE_BUFFER;}
 else perb=(char*)strBuffer;
 if((ln=FormatMessage(dwFlags,0,DosError,0,((allocBuf)?((LPTSTR)&perb):perb),180,0))!=0){
  char *ppb; ppb=perb; ppb+=ln; ppb-=2; if(*(WORD*)ppb==0x0A0D)ln-=2;
 }
 if(allocBuf){
  int alocsz; alocsz=0;
  if(fOptions&ERRSTRING_BORDER)alocsz=4+4+1;
  if(ln){
   alocsz+=ln; alocsz++;
   *(char**)strBuffer=(char*)HeapAlloc(pHeap,0,alocsz);
   if(*(char**)strBuffer==0){LocalFree(perb); return;}
   char *bfs; bfs=*(char**)strBuffer;
   if(fOptions&ERRSTRING_BORDER)bfs+=4;
   CopyMemory(bfs,perb,ln);
   LocalFree(perb);
  }
  else{
   alocsz+=8; *(char**)strBuffer=(char*)HeapAlloc(pHeap,0,alocsz);
   if(**(char**)strBuffer==0)return;
  }
  perb=*(char**)strBuffer;
 }
 if(fOptions&ERRSTRING_BORDER){*(DWORD*)perb=0x203D2D20; perb+=4;}
 if(!ln && fOptions&ERRSTRING_ERRCODE_IF_NOSTRING){HexToString(DosError,perb); ln=8;} 
 if(fOptions&ERRSTRING_BORDER){perb+=ln; *(DWORD*)perb=0x202D3D20; ln=4;}
 perb+=ln; *perb=0;
}

HANDLE hFailLog;

void FailMessage(char* msgTxt,DWORD ErrCode,DWORD dwOptions){DWORD ln,m,v,error_code; char *ErrBuf; BOOL bSmartCode;
 if(dwOptions&FMSG_CallGLE)error_code=GetLastError();
 else error_code=ErrCode;
 ln=0; m=0;
 if((ErrBuf=(char*)LocalAlloc(LPTR,300))==0)return;
 if(msgTxt){
  ln=lstrlen(msgTxt); if(ln>250){msgTxt[249]=0;ln=250;}
  lstrcpy(ErrBuf,msgTxt);
 }
 if(error_code==0 && dwOptions&FMSG_NO_SUCCESSINFO)goto skipFormatMsg;
 if(dwOptions&FMSG_HIDECODE){msgTxt=ErrBuf+ln; goto skip_code;}
 copystring(ErrBuf+ln,ErrCode_txt);
 ln=getstrlen(ErrBuf);
 msgTxt=ErrBuf+ln;
 if(dwOptions&FMSG_SMART_SHOWCODE && ErrCode<15000){bSmartCode=1; msgTxt-=2; goto err_skiphex_dec;}
 else bSmartCode=0;
 ltoa(error_code,msgTxt,16);
 m=getstrlen(msgTxt); if(m<8){copystring(msgTxt+9,msgTxt); copybytes(msgTxt,"00000000",9-m,1); strappend(msgTxt,msgTxt+9);}
 if((int)error_code<0)goto skip_code;
 strappend(msgTxt," (");
 ln=getstrlen(ErrBuf);
 msgTxt=ErrBuf+ln;
err_skiphex_dec:
 ltoa(error_code,msgTxt,10);
 if(!bSmartCode){strappend(msgTxt,skobka); ln++;}
skip_code:
 if(dwOptions&FMSG_NO_INFO)goto skipFormatMsg;
 strappend(msgTxt,"\r\n");
 if(dwOptions&FMSG_NTSTATUS_CONVERT && ntdllFunctions.RtlNtStatusToDosError)error_code=ntdllFunctions.RtlNtStatusToDosError(error_code);
 ln=getstrlen(ErrBuf); m=ln;
 ln=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,error_code,0,ErrBuf+ln,250-ln,0);
 skipFormatMsg:
 if((dwOptions&FMSG_WRITE_LOG)==0)goto skipWriteLog;
 if(hFailLog==NULL && ProgramDirPath.pathLen>0){
  if(buf0==0)buf0=(char*)LocalAlloc(LMEM_FIXED,256);
  if(ProgramDirPath.pathLen>0)lstrcpynW((LPWSTR)buf0,ProgramDirPath.fPath,ProgramDirPath.CharCount+1);
  else *buf0=0;
  lstrcatW((LPWSTR)buf0,L"Errors.log");
  hFailLog=CreateFileW((LPWSTR)buf0,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  if(hFailLog==INVALID_HANDLE_VALUE)hFailLog=0;
  SetFilePointer(hFailLog,0,NULL,FILE_END);
 }
 if(hFailLog!=NULL){
  m+=ln; if(m>250)m=250; ln=m; DWORD endpos;
  if(m<148){ErrBuf[m]='\r'; ErrBuf[m+1]='\n'; m+=2;}
  endpos=SetFilePointer(hFailLog,0,NULL,FILE_END);
  if((endpos+m)>=32*1024){//32 Kilobytes max_size of ErrorsLog File
   SetFilePointer(hFailLog,0,NULL,FILE_BEGIN);
   SetEndOfFile(hFailLog);
  }
  if(WriteFile(hFailLog,ErrBuf,m,&v,NULL) && (dwOptions&FMSG_SHOW_MSGBOX)==0){
   /*char *ppt; mainwin_title=(char*)LocalAlloc(LPTR,getstrlen(progTitle)+getstrlen(smemTable->ViewErrorsLog)+4);
   ppt=mainwin_title;
   ppt+=copystring(ppt,progTitle);
   copystring(ppt,smemTable->ViewErrorsLog);
   SetWindowText(main_win,mainwin_title);*/
   SendMessage(hstatus,SB_SETICON,(WPARAM)STATUSBAR_PANEINFO_ID,(LPARAM)LoadImage(gInst,(char*)18,IMAGE_ICON,16,16,LR_DEFAULTCOLOR));
   bWarnSign=1;
  }
  ErrBuf[ln]=0;
 }
 skipWriteLog:
 if(dwOptions&FMSG_SHOW_MSGBOX){
  MessageBox(GetActiveWin(),ErrBuf,ERROR_txt,MB_OK|MB_SETFOREGROUND|(((dwOptions&FMSG_ICONSTOP)==0)?MB_ICONWARNING:MB_ICONSTOP));
 }
 if(ErrBuf)LocalFree(ErrBuf);
 SetLastError(0);
}

#ifdef LOG_STEPS
HANDLE hStepsLog;
void LogStep(char* pStr){DWORD len,n,endpos; len=getstrlen(pStr);
 if(hStepsLog==NULL && ProgramDirPath.pathLen>0){
  wchar_t *wbuf;
  wbuf=(wchar_t*)LocalAlloc(LPTR,512);
  if(wbuf){
   if(ProgramDirPath.pathLen>0)lstrcpynW(wbuf,ProgramDirPath.fPath,ProgramDirPath.CharCount+1);
   else *wbuf=0;
   lstrcatW(wbuf,L"Steps.log");
   hStepsLog=CreateFileW(wbuf,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
   LocalFree(wbuf);
   if(hStepsLog==INVALID_HANDLE_VALUE)hStepsLog=0;
   SetFilePointer(hStepsLog,0,NULL,FILE_END);
   WriteFile(hStepsLog,"====================\r\n",22,&n,NULL);
  }
 }
 if(hStepsLog!=NULL){
  endpos=SetFilePointer(hStepsLog,0,NULL,FILE_END);
  if((endpos+len)>=32*1024){//32 Kilobytes max_size of StepsLog File
   SetFilePointer(hStepsLog,0,NULL,FILE_BEGIN);
   SetEndOfFile(hStepsLog);
  }
  if(WriteFile(hStepsLog,pStr,len,&n,NULL)){
   WriteFile(hStepsLog,"\r\n",2,&n,NULL);
  }
 }
}
#endif

bool MessageIntErr(WORD err_code){char *s1,*s2,*msg; int ln;
 s1="Internal error #";
 s2=".\r\rContinue anyway ?";
 ln=getstrlen(s1); ln+=getstrlen(s2); ln+=7; ln++;
 msg=(char*)LocalAlloc(LPTR,ln);
 //copybytes(,sNum,5,1);
 ltoa(err_code,msg+copystring(msg,s1),10);
 strappend(msg,s2);
 if(MessageBox(main_win,msg,progTitle,MB_YESNO|MB_ICONSTOP|MB_DEFBUTTON2)!=IDYES)return 1;
 LocalFree(msg);
 return 0;
}

void FailNtFunction(char *Name,DWORD ErrCode,DWORD nn){
 copystring(t1buf1,Name);
 if(ErrCode==0xC0000004){strappend(t1buf1,size_); ltoa(nn,t1buf1+getstrlen(t1buf1),10);}
 FailMessage(t1buf1,ErrCode,FMSG_WRITE_LOG|FMSG_NTSTATUS_CONVERT);
}