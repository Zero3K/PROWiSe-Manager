HWND backWin,shtd_win,ttip; HCURSOR hCurs; bool overHand,ShtdPrivl,SuspndEnbl,HbrntEnbl;
HBITMAP mdc_shtd_btp,sdbut,grayBackbmp,ckbxBmp; int tbb; HMODULE hpwrLib;
HDC ndc,memdc,mdc,backdc,mbackdc,hdcShtdwn,mdcShtdwn,hdcBn;
COLORADJUSTMENT coloradj; LONG deskWidth,deskHeight;
BYTE grayCntr; PAINTSTRUCT ps3,ps2; LONG ldwn,over;
RECT btnrc[6],btst[4]; MSG lmsg;

//WlxShutdown(0) - Switch User

#ifndef TTS_CLOSE
 #define TTS_CLOSE 0x40//0x840000C3-(0x80000000|0x04000000L|1|2|0x40)
#endif

BOOL IsIntheButt(RECT *rct,int xix,int yiy){if(xix>rct->left && xix<rct->right && yiy>rct->top && yiy<rct->bottom)return 1;else return 0;}

LRESULT CALLBACK BackWinPerc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){
if(Message==WM_PAINT){
 BeginPaint(backWin,&ps3);
 BitBlt(backdc,ps3.rcPaint.left,ps3.rcPaint.top,ps3.rcPaint.right-ps3.rcPaint.left,
 ps3.rcPaint.bottom-ps3.rcPaint.top,mbackdc,ps3.rcPaint.left,ps3.rcPaint.top,SRCCOPY);
 EndPaint(backWin,&ps3);
 return 0;
}
else return DefWindowProc(hwnd, Message, wParam, lParam);
}

void GrayBackg(){
 StretchBlt(mbackdc,0,0,deskWidth,deskHeight,mbackdc,0,0,deskWidth,deskHeight,SRCCOPY);
 BitBlt(backdc,0,0,deskWidth,deskHeight,mbackdc,0,0,SRCCOPY);
 if(grayCntr>40)KillTimer(backWin,1);
 else grayCntr++;
}

bool DrawButton(LONG bNum,BYTE Hilite){
 if(bNum>=2 && !ShtdPrivl)return 0;
 if(bNum==2 && !HbrntEnbl)return 0;
 if(bNum==5 && !SuspndEnbl)return 0;
 HDC ndc; LONG bn_offset; bn_offset=Hilite;
 ndc=CreateCompatibleDC(hdcShtdwn);
 SelectObject(ndc,sdbut);
 switch(bNum){
  case 1: bn_offset+=96; break;
  case 2: bn_offset+=192; break;
  case 3: bn_offset+=288; break;
  case 4: bn_offset+=384; break;
  case 5: bn_offset+=480; break;
 }
 BitBlt(hdcShtdwn,btnrc[bNum].left,btnrc[bNum].top,32,32,ndc,0,bn_offset,SRCCOPY);
 DeleteDC(ndc);
return 1;}

BYTE tShtdCkBx[4]={0,0,0,1};//Normal,force,TurboForce,PowerOff

void rButt(int rbn, int st){int imgofs,y,x; imgofs=0;
 ndc=CreateCompatibleDC(mdcShtdwn);
 SelectObject(ndc,ckbxBmp);
 if(st==1)imgofs=14;
 else if(st==2)imgofs=28;
 if(tShtdCkBx[rbn])imgofs+=42;
 x=3;
 switch(rbn){
  case 0: y=172; break;
  case 1: y=189; break;
  case 2: y=206; break;
  case 3: x=160; y=172; break;
 }
 BitBlt(mdcShtdwn,x,y,13,13,ndc,imgofs,0,SRCCOPY);
 BitBlt(hdcShtdwn,x,y,13,13,ndc,imgofs,0,SRCCOPY);
 DeleteDC(ndc);
}

void drwShtdText(RECT *rText,char *tText){DrawTextEx(mdcShtdwn,tText,lstrlen(tText),rText,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE,NULL);}

void OnInit(){RECT rect; HBITMAP hbmp; int i,xps,yps; HWND htmp;
sdbut=(HBITMAP)LoadImage(gInst,(char*)2,IMAGE_BITMAP,32,704,LR_DEFAULTCOLOR);
SetForegroundWindow(shtd_win);
hdcShtdwn=GetDC(shtd_win);
GetClientRect(GetDesktopWindow(),&rect);
xps=rect.right; yps=rect.bottom; yps-=rect.bottom/2;
WNDCLASSEX wndc;
wndc.cbSize=sizeof(WNDCLASSEX);
wndc.style=0;//CS_BYTEALIGNCLIENT;
wndc.lpfnWndProc=BackWinPerc;
wndc.cbClsExtra=0; wndc.cbWndExtra=0;
wndc.hInstance=gInst;
wndc.hIcon=NULL;
wndc.hCursor=LoadCursor(NULL,IDC_ARROW);
wndc.lpszMenuName=0;
wndc.hIconSm=0;
wndc.hbrBackground=(HBRUSH)GetStockObject(NULL_BRUSH);
wndc.lpszClassName=LockDesktop_class;
RegisterClassEx(&wndc);
backWin=CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,LockDesktop_class,0,WS_POPUP|WS_DISABLED|WS_VISIBLE,0,0,rect.right,rect.bottom,main_win,(HMENU)0,gInst,0);
if(backWin!=NULL){
 backdc=GetDC(backWin);
 GetColorAdjustment(backdc,&coloradj);
 deskWidth=rect.right; deskHeight=rect.bottom;
 coloradj.caBrightness=-1;
 coloradj.caColorfulness=0;
 coloradj.caContrast=-1;
 BITMAPINFO bminf;
 ZeroMemory(&bminf,sizeof(BITMAPINFO));
 bminf.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
 bminf.bmiHeader.biWidth=deskWidth;
 bminf.bmiHeader.biHeight=deskHeight;
 bminf.bmiHeader.biPlanes=1;
 bminf.bmiHeader.biBitCount=32;
 grayBackbmp=CreateDIBSection(backdc,&bminf,DIB_RGB_COLORS,NULL,0,0);
 mbackdc=CreateCompatibleDC(backdc);
 SelectObject(mbackdc,grayBackbmp);
 BitBlt(mbackdc,0,0,deskWidth,deskHeight,backdc,0,0,SRCCOPY);
 SetStretchBltMode(mbackdc,HALFTONE);
 SetColorAdjustment(mbackdc,&coloradj);
 grayCntr=0;
 SetWindowPos(backWin,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
 SetForegroundWindow(backWin);
 if(!isMiniNT)SetTimer(backWin,1,200,(TIMERPROC)GrayBackg);
 else{
  coloradj.caBrightness=-25;
  coloradj.caColorfulness=-100;
  coloradj.caContrast=-25;
  SetColorAdjustment(mbackdc,&coloradj);
  GrayBackg();
 }
}
GetClientRect(shtd_win,&rect);
xps-=rect.right; yps-=rect.bottom; xps/=2; yps/=2;
SetWindowPos(shtd_win,HWND_TOPMOST,xps,yps,0,0,SWP_NOSIZE);
SetForegroundWindow(shtd_win);
mdcShtdwn=CreateCompatibleDC(hdcShtdwn);
mdc_shtd_btp=CreateCompatibleBitmap(hdcShtdwn,rect.right,rect.bottom);
SelectObject(mdcShtdwn,mdc_shtd_btp);
HBRUSH hbrbg; hbrbg=CreateSolidBrush(0x00993300);
FillRect(mdcShtdwn,&rect,hbrbg);
DeleteObject(hbrbg);
ndc=CreateCompatibleDC(mdcShtdwn);
SetStretchBltMode(mdcShtdwn,HALFTONE);
hbmp=(HBITMAP)LoadImage(gInst,(char*)3,IMAGE_BITMAP,313,3,LR_DEFAULTCOLOR);
SelectObject(ndc,hbmp);
BitBlt(mdcShtdwn,0,43,rect.right,2,ndc,0,0,SRCCOPY);
BitBlt(mdcShtdwn,0,165,rect.right,2,ndc,0,0,SRCCOPY);
StretchBlt(mdcShtdwn,0,45,rect.right,120,ndc,0,2,rect.right,1,SRCCOPY);
DeleteObject(hbmp);
hbmp=(HBITMAP)LoadImage(gInst,(char*)11,IMAGE_BITMAP,36,36,LR_DEFAULTCOLOR);
SelectObject(ndc,hbmp);
BitBlt(mdcShtdwn,270,6,36,36,ndc,0,0,SRCCOPY);
DeleteObject(hbmp);
WORD bn_offset; bn_offset=0;
SelectObject(ndc,sdbut);
tbb=progSetts.LastShutdownButn;
// enable privelege
HbrntEnbl=0; SuspndEnbl=0;
ShtdPrivl=EnablePrivelege(SE_SHUTDOWN_NAME,SE_PRIVILEGE_ENABLED);
if(ShtdPrivl){
 if(hpwrLib==0)hpwrLib=LoadLibrary("Powrprof.dll");
 if(hpwrLib){
  //pIsPwrShutdownAllowed=(BOOL(_stdcall*)(VOID))GetProcAddress(hpwrLib,"IsPwrShutdownAllowed");
  //if(pIsPwrShutdownAllowed)ShtdPrivl=pIsPwrShutdownAllowed();
  //if(ShtdPrivl){
  pIsPwrSuspendAllowed=(BOOL(_stdcall*)(VOID))GetProcAddress(hpwrLib,"IsPwrSuspendAllowed");
  if(pIsPwrSuspendAllowed)SuspndEnbl=pIsPwrSuspendAllowed();
  pIsPwrHibernateAllowed=(BOOL(_stdcall*)(VOID))GetProcAddress(hpwrLib,"IsPwrHibernateAllowed");
  if(pIsPwrHibernateAllowed)HbrntEnbl=pIsPwrHibernateAllowed();
  //}
 }
}
LONG dcOfst;
for(i=0;i<6;i++){
 htmp=GetDlgItem(shtd_win,15001+i);
 if(htmp!=NULL){
  GetWindowRect(htmp,&btnrc[i]);
  ScreenToClient(shtd_win,(POINT*)&btnrc[i]);
  if(i==2 && !HbrntEnbl)dcOfst=672;
  else if(i==3 && !ShtdPrivl)dcOfst=608;
  else if(i==4 && !ShtdPrivl)dcOfst=576;
  else if(i==5 && !SuspndEnbl)dcOfst=640;
  else if(i==tbb)dcOfst=bn_offset+64;
  else dcOfst=bn_offset;
  BitBlt(mdcShtdwn,btnrc[i].left,btnrc[i].top,32,32,ndc,0,dcOfst,SRCCOPY);
  bn_offset+=(BYTE)96;
  btnrc[i].right=btnrc[i].left; btnrc[i].right+=32;
  btnrc[i].bottom=btnrc[i].top; btnrc[i].bottom+=32;
  DestroyWindow(htmp);
 }
}
DeleteDC(ndc);
SetTextColor(mdcShtdwn,0x00ffffff); SetBkMode(mdcShtdwn,TRANSPARENT);
LOGFONT ft;
lstrcpy(ft.lfFaceName,MSsansSerif_txt);
ft.lfCharSet=RUSSIAN_CHARSET; ft.lfOrientation=0; ft.lfEscapement=0;
ft.lfItalic=0; ft.lfUnderline=0; ft.lfStrikeOut=0;
ft.lfHeight=24; ft.lfWidth=0; ft.lfWeight=500;
DeleteObject(SelectObject(mdcShtdwn,CreateFontIndirect(&ft)));
TextOut(mdcShtdwn,5,10,smemTable->ShutDown,lstrlen(smemTable->ShutDown));
ft.lfHeight=14; ft.lfWidth=0; ft.lfWeight=800;
DeleteObject(SelectObject(mdcShtdwn,CreateFontIndirect(&ft)));
////////////////// BUTTONS
RECT rText; rText.top=85; rText.bottom=100;
rText.left=13; rText.right=84; drwShtdText(&rText,smemTable->Lock);
rText.left=130; rText.right=179; drwShtdText(&rText,smemTable->LogOff);
if(!HbrntEnbl || !ShtdPrivl)SetTextColor(mdcShtdwn,0x00808080);
rText.left=218; rText.right=307;
if(ActiveLangID==RUS_LANGID)DrawTextEx(mdcShtdwn,smemTable->StandBy,lstrlen(smemTable->Hibernate),&rText,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE|DT_CALCRECT,NULL);
//FillRect(mdcShtdwn,&rText,(HBRUSH)GetStockObject(BLACK_BRUSH));
drwShtdText(&rText,smemTable->Hibernate);
if(ShtdPrivl)SetTextColor(mdcShtdwn,0x00ffffff);
rText.top=148; rText.bottom=166;
rText.left=1; rText.right=100;
rText.left=5; rText.right=92; drwShtdText(&rText,smemTable->Restart);
rText.left=116; rText.right=191; drwShtdText(&rText,smemTable->TurnOff);
if(!SuspndEnbl)SetTextColor(mdcShtdwn,0x00808080);
rText.left=212; rText.right=312;
if(ActiveLangID==RUS_LANGID)DrawTextEx(mdcShtdwn,smemTable->StandBy,lstrlen(smemTable->StandBy),&rText,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE|DT_CALCRECT,NULL);
drwShtdText(&rText,smemTable->StandBy);
///////////////////
SetTextColor(mdcShtdwn,0x00ffffff);
ft.lfHeight=15; ft.lfWidth=0; ft.lfWeight=600;
DeleteObject(SelectObject(mdcShtdwn,CreateFontIndirect(&ft)));
TextOut(mdcShtdwn,18,172,smemTable->NormalWindowsExit,lstrlen(smemTable->NormalWindowsExit));
TextOut(mdcShtdwn,18,189,smemTable->ForcedWindowsExit,lstrlen(smemTable->ForcedWindowsExit));
TextOut(mdcShtdwn,18,206,smemTable->TurboWindowsExit,lstrlen(smemTable->TurboWindowsExit));
TextOut(mdcShtdwn,175,172,smemTable->UsePowerOffFeature,lstrlen(smemTable->UsePowerOffFeature));
ckbxBmp=(HBITMAP)LoadImage(gInst,(LPTSTR)4,IMAGE_BITMAP,83,13,LR_DEFAULTCOLOR);
btst[0].left=4; btst[0].top=173; btst[0].right=137; btst[0].bottom=183;//Nrm
btst[1].left=4; btst[1].top=190; btst[1].right=130; btst[1].bottom=200;//Frc
btst[2].left=4; btst[2].top=207; btst[2].right=131; btst[2].bottom=217;//Tb
btst[3].left=160; btst[3].top=173; btst[3].right=305; btst[3].bottom=183;//PwrOff
if(progSetts.ShtdCkBx[2]){progSetts.ShtdCkBx[0]=0;progSetts.ShtdCkBx[1]=0;}
else if(progSetts.ShtdCkBx[1]){progSetts.ShtdCkBx[0]=0;progSetts.ShtdCkBx[2]=0;}
else if(progSetts.ShtdCkBx[0]){progSetts.ShtdCkBx[1]=0;progSetts.ShtdCkBx[2]=0;}
tShtdCkBx[0]=progSetts.ShtdCkBx[0];
tShtdCkBx[1]=progSetts.ShtdCkBx[1];
tShtdCkBx[2]=progSetts.ShtdCkBx[2];
tShtdCkBx[3]=progSetts.ShtdCkBx[3];
rButt(0,0);rButt(1,0);rButt(2,0);rButt(3,0);
INITCOMMONCONTROLSEX icctrl; TOOLINFO ti;
icctrl.dwSize=sizeof(icctrl); icctrl.dwICC=ICC_BAR_CLASSES;
InitCommonControlsEx(&icctrl);
ttip=CreateWindowEx(0,TOOLTIPS_CLASS,0,TTS_ALWAYSTIP|TTS_BALLOON,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,shtd_win,NULL,gInst,NULL);
ti.cbSize=sizeof(TOOLINFO); ti.uFlags=0; ti.hwnd=shtd_win; ti.hinst=gInst;
SendMessage(ttip,TTM_SETDELAYTIME,TTDT_AUTOPOP,10000);
SendMessage(ttip,TTM_SETMAXTIPWIDTH,0,350);
DWORD *ptrs; ptrs=(DWORD*)&(smemTable->tip_lock);
for(i=0;i<6;i++){
 if(i==2 && !HbrntEnbl)goto nex_tip;
 if((i==3 || i==4) && !ShtdPrivl)goto nex_tip;
 if(i==5 && !SuspndEnbl)goto nex_tip;
 ti.uId=i+1; ti.lpszText=(char*)*ptrs;
 ti.rect.left=btnrc[i].left; ti.rect.top=btnrc[i].top;
 ti.rect.right=btnrc[i].right; ti.rect.bottom=btnrc[i].bottom;
 SendMessage(ttip,TTM_ADDTOOL,(WPARAM)0,(LPARAM)&ti);
 nex_tip:
 ptrs++;
}
}

void doJob(int jobNm){DWORD flags;
progSetts.ShtdCkBx[0]=tShtdCkBx[0];
progSetts.ShtdCkBx[1]=tShtdCkBx[1];
progSetts.ShtdCkBx[2]=tShtdCkBx[2];
progSetts.ShtdCkBx[3]=tShtdCkBx[3];
SaveSettings();
if(!ShtdPrivl && (jobNm>=3))return;
switch(jobNm){
 case 1: if(user32Functions.LockWorkStation!=NULL)user32Functions.LockWorkStation(); break;
 case 2://Log Off
  flags=EWX_LOGOFF|EWX_FORCEIFHUNG; if(tShtdCkBx[1] || tShtdCkBx[2])flags|=EWX_FORCE;
  ExitWindowsEx(flags,0);
  break;
 case 3://Hibernate
  if(!HbrntEnbl)break;
  ntdllFunctions.pNtInitiatePowerAction(3,2,3,0);
  break;
 case 4://Restart
  if(!tShtdCkBx[2] || ntdllFunctions.pNtShutdownSystem==NULL){
   flags=EWX_REBOOT|EWX_FORCEIFHUNG;
   if(tShtdCkBx[1])flags|=EWX_FORCE;
   ExitWindowsEx(flags,0);
  }
  else {ntdllFunctions.pNtShutdownSystem(ShutdownReboot);}
  break;
 case 5://Turn Off
  if(!tShtdCkBx[2] || ntdllFunctions.pNtShutdownSystem==NULL){
   flags=(tShtdCkBx[3])?(EWX_POWEROFF|EWX_FORCEIFHUNG):(EWX_SHUTDOWN|EWX_FORCEIFHUNG);
   if(tShtdCkBx[1])flags|=EWX_FORCE;
   ExitWindowsEx(flags,0);
  }
  else {ntdllFunctions.pNtShutdownSystem(((tShtdCkBx[3])?ShutdownPowerOff:ShutdownNoReboot));}
  break;
 case 6://StandBy
  if(!SuspndEnbl)break;
  ntdllFunctions.pNtInitiatePowerAction(2,2,3,0);
  break;
}
SendMessage(shtd_win,WM_CLOSE,0,0);
}

BOOL CALLBACK Shutdown_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){
POINT curpos;
switch(Message){
case WM_INITDIALOG: shtd_win=hDlg;
 WindowFillText(hDlg,dlgTAT->dlg8);
 OnInit();
 ShowWindow(shtd_win,SW_SHOWNA);
 hCurs=LoadCursor(0,IDC_HAND);
 break;
case WM_PAINT:
 BeginPaint(hDlg,&ps2);
 BitBlt(hdcShtdwn,ps2.rcPaint.left,ps2.rcPaint.top,ps2.rcPaint.right-ps2.rcPaint.left,
 ps2.rcPaint.bottom-ps2.rcPaint.top,mdcShtdwn,ps2.rcPaint.left,ps2.rcPaint.top,SRCCOPY);
 EndPaint(hDlg,&ps2);
 return 1;
case WM_CLOSE: closeWin: EndDialog(hDlg,1);
case WM_DESTROY:
 UnregisterClass(LockDesktop_class,gInst);
 DeleteObject(grayBackbmp);
 DeleteObject(sdbut);
 ReleaseDC(backWin,backdc);
 ReleaseDC(shtd_win,hdcShtdwn);
 DeleteDC(mbackdc);
 DeleteDC(mdcShtdwn);
 DeleteObject(ckbxBmp);
 DestroyWindow(backWin);
 shtd_win=NULL; PostQuitMessage(0);
 break;
case WM_MOUSEMOVE:
 curpos.x=LOWORD(lParam); curpos.y=HIWORD(lParam);
 if(curpos.y<46)break;
 lmsg.hwnd=hDlg; lmsg.message=Message; lmsg.wParam=wParam; lmsg.lParam=lParam;
 lmsg.pt.x=curpos.x; lmsg.pt.y=curpos.y;
 SendMessage(ttip,TTM_RELAYEVENT,(WPARAM)0,(LPARAM)&lmsg);
 overHand=0; int i;
 if(curpos.y>170)goto nShtdType;
 if(wParam&MK_LBUTTON){overHand=1;
  if(over>49){for(i=0;i<6;i++){if(over==50+i && !IsIntheButt(&btnrc[i],curpos.x,curpos.y)){if(DrawButton(i,0))over=40+i; break;}}}
  else{for(i=0;i<6;i++){if(over==40+i && IsIntheButt(&btnrc[i],curpos.x,curpos.y)){if(DrawButton(i,32))over=50+i; break;}}}
  goto next1;
 }
 if(over>49){for(i=0;i<6;i++){if(over==50+i && !IsIntheButt(&btnrc[i],curpos.x,curpos.y)){if(DrawButton(i,0))over=40+i; break;}}}
 else{for(i=0;i<6;i++){if(IsIntheButt(&btnrc[i],curpos.x,curpos.y)){if(DrawButton(i,64))ldwn=over=50+i; break;}}}
 if(over>49 && i<6){
  if(tbb<6 && tbb!=i)DrawButton(tbb,0);
 }
 goto next1;
 nShtdType:
 if(wParam&MK_LBUTTON){
  if(over<10){for(i=0;i<6;i++){if(over==i+1 && !IsIntheButt(&btst[i],curpos.x,curpos.y)){rButt(i,0);over=11+i; break;}}}
  else{for(i=0;i<6;i++){if(over==11+i && IsIntheButt(&btst[i],curpos.x,curpos.y)){rButt(i,2); over=i+1; break;}}}
 }
 else{
  for(i=0;i<4;i++){if(IsIntheButt(&btst[i],curpos.x,curpos.y)){rButt(i,1);ldwn=over=i+1; break;}}
  if(over<10){for(i=0;i<4;i++){if(over==1+i && !IsIntheButt(&btst[i],curpos.x,curpos.y)){rButt(i,0); over=11+i; break;}}}
 }
 next1:
 if(over>49 || (over>0 && over<6))overHand=1;
 if(overHand)SetCursor(hCurs);
 break;
case WM_LBUTTONUP:
 curpos.x=LOWORD(lParam); curpos.y=HIWORD(lParam);
 if(ldwn!=over)break; ldwn=0;
 if(curpos.y>170){
  for(i=0;i<3;i++){if(IsIntheButt(&btst[i],curpos.x,curpos.y)){
   tShtdCkBx[0]=0;tShtdCkBx[1]=0;tShtdCkBx[2]=0;
   tShtdCkBx[i]=1; rButt(0,0); rButt(1,0);rButt(2,0);
   rButt(i,1); over=i+11;
   goto exxit;
  }}
  if(IsIntheButt(&btst[3],curpos.x,curpos.y)){
   if(tShtdCkBx[3])tShtdCkBx[3]=0;
   else tShtdCkBx[3]=1; over=14;
   rButt(3,tShtdCkBx[3]);
  }
 }
 else if(curpos.y>46){for(i=0;i<6;i++){if(IsIntheButt(&btnrc[i],curpos.x,curpos.y)){if(DrawButton(i,64)){over=40+i; doJob(i+1);} break;}}}
 exxit:
 break;
case WM_LBUTTONDOWN:
 curpos.x=LOWORD(lParam); curpos.y=HIWORD(lParam);
 if(curpos.y<46)break;
 if(curpos.y>170){for(i=0;i<4;i++){if(IsIntheButt(&btst[i],curpos.x,curpos.y)){rButt(i,2); over=i+1; break;}}}
 else{for(i=0;i<6;i++){if(IsIntheButt(&btnrc[i],curpos.x,curpos.y)){if(DrawButton(i,32))over=50+i; break;}}}
 if(overHand)SetCursor(hCurs);
 break;
case WM_COMMAND:
 wID=LOWORD(wParam);
 switch(wID){
  case 50004: case IDCANCEL: goto closeWin;
  case 50005: case 50007: case 50008: case 50009: case 50010:
   if(tbb<6)DrawButton(tbb,0);
   else rButt(tbb-6,0);
   if(wID==50005 || wID==50007 || wID==50008){//TAB,RIGHT,DOWN
    if(tbb<9){
     tbb++;
     if(tbb==2 && !HbrntEnbl)tbb++;
     if(tbb==3 && !ShtdPrivl)tbb++;
     if(tbb==4 && !ShtdPrivl)tbb++;
     if(tbb==5 && !SuspndEnbl)tbb++;
    }
    else tbb=0;
   }
   else{
    if(tbb>0){
     tbb--;
     if(tbb==5 && !SuspndEnbl)tbb--;
     if(tbb==4 && !ShtdPrivl)tbb--;
     if(tbb==3 && !ShtdPrivl)tbb--;
     if(tbb==2 && !HbrntEnbl)tbb--;
    }
    else tbb=9;
   }
   if(tbb<6){DrawButton(tbb,64); over=50+tbb;}
   else rButt(tbb-6,1);
   break;
  case 50006:
   if(tbb<6){DrawButton(tbb,32); over=40+tbb; doJob(tbb+1);}
   else{
    if(tbb!=9){
     tShtdCkBx[0]=0;tShtdCkBx[1]=0;tShtdCkBx[2]=0;
     tShtdCkBx[tbb-6]=1; rButt(0,0); rButt(1,0);rButt(2,0);
     rButt(tbb-6,1); over=tbb+11-6;
    }
    else{
     if(tShtdCkBx[3])tShtdCkBx[3]=0;
     else tShtdCkBx[3]=1; over=14;
     rButt(3,tShtdCkBx[3]);
    }
   }
   break;
 }
 break;
} return 0;}

DWORD ShutdownThread(LPVOID){MSG msg; HACCEL haccel3;
CreateDialog(gInst,(char*)8,main_win,(DLGPROC)Shutdown_Proc);
if(shtd_win==NULL)goto exit;
haccel3=LoadAccelerators(gInst,(char*)3);
while(GetMessage(&msg,NULL,0,0)){
 if(haccel3!=NULL && TranslateAccelerator(shtd_win,haccel3,&msg))continue;
 TranslateMessage(&msg);DispatchMessage(&msg);
}
exit:
EnablePrivelege(SE_SHUTDOWN_NAME,0);
if(hpwrLib){if(FreeLibrary(hpwrLib))hpwrLib=0;}
ExitThread(0);
return 0;}
