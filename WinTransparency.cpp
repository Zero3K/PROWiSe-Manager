typedef struct _WINDOW_OPACITY_INFO{
 HWND hWnd;
 BYTE bLayered;
 BYTE FirstOpacity;
 char *TransparencyCaption;
 DWORD TranspCapLen;
 HWND hTrackBar;
}WINDOW_OPACITY_INFO;

void WinCustomTransparency_OnInit(HWND hDlg,WINDOW_OPACITY_INFO *woi){HWND htbar; DWORD exstyle,n; BYTE opacity; bool bl;
 SetWindowText(hDlg,smemTable->CustomTransparency);
 WindowFillText(hDlg,dlgTAT->dlg34);
 if((htbar=GetDlgItem(hDlg,102))==0)return;
 woi->hTrackBar=htbar;
 exstyle=GetWindowLong(woi->hWnd,GWL_EXSTYLE);
 bl=((exstyle&WS_EX_LAYERED)==WS_EX_LAYERED);
 if(woi->bLayered==0){
  if(bl)woi->bLayered=1;
  else woi->bLayered=2;
 }
 if(bl){
  if(user32Functions.GetLayeredWindowAttributes(woi->hWnd,0,&opacity,&n)!=0){
   if((n&LWA_ALPHA)==LWA_ALPHA){
    woi->FirstOpacity=opacity;
    goto skipset255;
   }
  }
 }
 woi->FirstOpacity=opacity=255;
 skipset255:
 SendMessage(htbar,TBM_SETRANGE,(WPARAM)0,(LPARAM)0x00ff0000);//MAKELONG(lMinimum, lMaximum);
 n=255; n-=opacity;
 SendMessage(htbar,TBM_SETPOS,(WPARAM)1,(LPARAM)n);
 SendMessage(htbar,TBM_SETTIC,(WPARAM)0,(LPARAM)128);
 char *ptr;
 n=getstrlen(smemTable->Transparency); n+=7;
 if((ptr=(char*)LocalAlloc(LPTR,n))!=0){
  copystring(ptr,smemTable->Transparency); n-=7;
  copystring(ptr+n,": "); n+=2;
  woi->TranspCapLen=n;
  float flt;
  flt=100; flt/=255; flt*=(255-opacity);
  DWORD mm; mm=(DWORD)flt;
  flt-=mm; if(flt>0.5)mm++;
  ltoa(mm,ptr+n,10);
  strappend(ptr,"%");
  SetDlgItemText(hDlg,101,ptr);
  woi->TransparencyCaption=ptr;
 }
}

BOOL CALLBACK WinCustomTransparency_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){WINDOW_OPACITY_INFO *woi;
switch(Message){
case WM_INITDIALOG:
 SetWindowLong(hDlg,GWL_USERDATA,(DWORD)lParam);
 SetWinPos(hDlg,1);
 if(pEnableThemeDialogTexture>0)pEnableThemeDialogTexture(hDlg,0x00000006);//ETDT_ENABLETAB or ETDT_USETABTEXTURE
 WinCustomTransparency_OnInit(hDlg,(WINDOW_OPACITY_INFO*)lParam);
 return 0;
case WM_COMMAND:
 WORD wID; wID=LOWORD(wParam);
 if(wID==IDOK)goto closeWin;
 else if(wID==50004 || wID==IDCANCEL){
  cmd_cancel:
  if((woi=(WINDOW_OPACITY_INFO*)GetWindowLong(hDlg,GWL_USERDATA))==0)goto closeWin;
  if(!IsWindow(woi->hWnd))goto closeWin;
  DWORD exstyle;
  exstyle=GetWindowLong(woi->hWnd,GWL_EXSTYLE);
  if((BYTE)(woi->FirstOpacity)<255 && (exstyle&WS_EX_LAYERED)==0){
   SetWindowLong(woi->hWnd,GWL_EXSTYLE,exstyle|WS_EX_LAYERED);
  }
  user32Functions.SetLayeredWindowAttributes(woi->hWnd,0,woi->FirstOpacity,LWA_ALPHA);
  if(woi->FirstOpacity==255 && (exstyle&WS_EX_LAYERED)==WS_EX_LAYERED){
   exstyle&=~WS_EX_LAYERED;
   SetWindowLong(woi->hWnd,GWL_EXSTYLE,exstyle);
   RedrawWindow(woi->hWnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN);
  }
  goto closeWin;
 }
 break;
case WM_HSCROLL:
 if((woi=(WINDOW_OPACITY_INFO*)GetWindowLong(hDlg,GWL_USERDATA))==0)break;
 if((HWND)lParam==woi->hTrackBar){
  DWORD pp,nn; char *ptr; float flt;
  pp=SendMessage(woi->hTrackBar,TBM_GETPOS,0,(LPARAM)0);
  if(pp>255)pp=255;
  if((ptr=woi->TransparencyCaption)!=0){
   ptr+=woi->TranspCapLen;
   flt=100; flt/=255; flt*=pp;
   DWORD mm; mm=(DWORD)flt;
   flt-=mm; if(flt>0.5)mm++;
   ltoa(mm,ptr,10);
   strappend(ptr,"%");
   SetDlgItemText(hDlg,101,woi->TransparencyCaption);
  }
  if((nn=GetWindowLong(woi->hWnd,GWL_EXSTYLE))!=0){
   if((nn&WS_EX_LAYERED)!=WS_EX_LAYERED){
    if(pp!=0)SetWindowLong(woi->hWnd,GWL_EXSTYLE,nn|WS_EX_LAYERED);
   }
   user32Functions.SetLayeredWindowAttributes(woi->hWnd,0,(BYTE)(255-pp),LWA_ALPHA);
   if(pp==0 && woi->bLayered==2){
     nn&=~WS_EX_LAYERED;
     SetWindowLong(woi->hWnd,GWL_EXSTYLE,nn);
     //RedrawWindow(woi->hWnd,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_FRAME|RDW_ALLCHILDREN);
    }
  }
 }
 break;
case WM_CLOSE:
 goto cmd_cancel;
 closeWin: DestroyWindow(hDlg);
 break;
case WM_DESTROY: PostQuitMessage(0); break;
} return 0;}

DWORD WinCustomTransparency_Thread(WINDOW_INFO *wif){HWND custom_win; MSG msg; WINDOW_OPACITY_INFO *woi; HACCEL haccel;
 woi=0;
 if(wif==0 || wif->cbSize!=sizeof(WINDOW_INFO))goto exit;
 if(!IsWindow(wif->hWnd)){
  MessageBox(main_win,smemTable->HandleDoesntIdentifyExistingWindow,ERROR_txt,MB_OK|MB_ICONSTOP);
  goto exit;
 }
 if((woi=(WINDOW_OPACITY_INFO*)LocalAlloc(LMEM_FIXED,sizeof(WINDOW_OPACITY_INFO)))==0)goto exit;
 woi->hWnd=wif->hWnd;
 woi->bLayered=wif->bLayered;
 woi->TransparencyCaption=0;
 if((custom_win=CreateDialogParam(gInst,(char*)34,main_win,(DLGPROC)WinCustomTransparency_Proc,(DWORD)woi))==0)goto exit;
 ShowWindow(custom_win,SW_SHOW);
 AddHwndToWinChain(custom_win);
 haccel=LoadAccelerators(gInst,(char*)2);
 while(GetMessage(&msg,NULL,0,0)){
  if(haccel!=0 && TranslateAccelerator(custom_win,haccel,&msg))continue;
  if(!IsDialogMessage(custom_win,&msg)){
   TranslateMessage(&msg);DispatchMessage(&msg);
  }
 }
 exit:
 if(woi){
  LocalFree(woi->TransparencyCaption);
  LocalFree(woi);
 }
 LeaveWindowFocus(custom_win);
 ExitThread(0);
 return 0;
}
