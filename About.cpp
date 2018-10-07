HDC hdc; HBITMAP tempbmp,mdc_btp; HRGN hrgn1; LOGFONT ft; int cclr[6],cprtWd; bool abtmove;
POINT curpos; RECT mpos,URLrect; LONG cmx,cmy,Dwd,Dht; HWND DeskWin;
char cprits[58];
unsigned const char devlpr[]={237,90,160,193,153,154,148,147,198,229,160,148,159,162,155,148,0,0};
unsigned const char devlprCity[]={152,179,158,144,172,154,237,217,164,183,165,139,148,151,185,229,170,162,161,154,0};
unsigned const char Rdevlpr[]={237,90,33,65,31,29,37,21,237,26,35,59,45,25,35,21,13,35,0};
unsigned const char RdevlprCity[]={35,53,28,18,18,29,237,217,37,54,32,31,36,29,63,229,46,37,38,29,0};

char web_address[]="http://DMoNsoft.jino-net.ru\0";
char web_address_name[]="[ DMoNsoft ]\0";
#define webAdr_len 24
#define webAdrName_len 12
inline void OffsetRect2(RECT *r,long x,long y){r->left+=x;r->right+=x;r->top+=y;r->bottom+=y;}

BOOL LeaveInTheRect(RECT *texti,long *xix,long *yiy){BOOL ret=0;
if(xix!=NULL){if(*xix<texti->left)*xix=texti->left,ret=1; else{if(*xix>texti->right)*xix=texti->right,ret=1;}}
if(yiy!=NULL){if(*yiy<texti->top)*yiy=texti->top,ret=1; else{if(*yiy>texti->bottom)*yiy=texti->bottom,ret=1;}}
return ret;}

LRESULT CALLBACK AbtProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
switch(message){
case WM_PAINT: BeginPaint(abt_win,&ps2);
 BitBlt(hdc,ps2.rcPaint.left,ps2.rcPaint.top,ps2.rcPaint.right-ps2.rcPaint.left,
 ps2.rcPaint.bottom-ps2.rcPaint.top,memdc,ps2.rcPaint.left,ps2.rcPaint.top,SRCCOPY);
 EndPaint(abt_win,&ps2);
 return 0;
case WM_DESTROY: KillTimer(abt_win,2); DeleteDC(mdc);DeleteDC(memdc);DeleteDC(ndc);
 DeleteObject(tempbmp); DeleteObject(mdc_btp); UnregisterClass(about_class,gInst);
 DeleteObject(hrgn1);
 abt_win=0; PostQuitMessage(0); return 0;
case WM_TIMER: if(wParam==2){
 ft.lfHeight=20; ft.lfWidth=0; ft.lfWeight=700;
 DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
 SetTextColor(mdc,RGB(cclr[0],cclr[1],cclr[2]));
 TextOut(mdc,25,113,(char*)cprits,1);
 ft.lfHeight=18; ft.lfWidth=0; ft.lfWeight=700;
 DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
 TextOut(mdc,37,115,(char*)cprits+1,cprits[57]-1);
 if(cclr[0]>=255)cclr[3]=1; else if(cclr[0]<150)cclr[3]=0;
 if(!cclr[3])cclr[0]++;else cclr[0]--;
 if(cclr[1]>=253)cclr[4]=1; else if(cclr[1]<180)cclr[4]=0;
 if(!cclr[4])cclr[1]+=2;else cclr[1]-=2;
 if(cclr[2]>=255)cclr[5]=1; else if(cclr[2]<60)cclr[5]=0;
 if(!cclr[5])cclr[2]++;else cclr[2]--;
 BitBlt(hdc,25,115,cprtWd,20,mdc,25,115,SRCCOPY);
 return 0;
 }
 return 1;
case WM_LBUTTONDOWN:
 curpos.x=(short)LOWORD(lParam); curpos.y=(short)HIWORD(lParam);
 if(curpos.y<19){
  abtmove=1;
  ClientToScreen(hwnd,&curpos);
  cmx=curpos.x; cmy=curpos.y;
  SetCapture(hwnd);
  GetClientRect(DeskWin,&mpos);
  curpos.x=curpos.y=0;
  ClientToScreen(DeskWin,&curpos);
  OffsetRect2(&mpos,curpos.x,curpos.y);
  mpos.left+=curpos.x;mpos.right+=curpos.x;mpos.top+=curpos.y;mpos.bottom+=curpos.y;
  ClipCursor(&mpos);
  return 0;
 }
// #ifndef IS_HM_VERSION
 if(over==91){
  //int i; for(i=0;i<webAdr_len;i++){t0buf1[i]=(char)(web_address[i]^0x8D);} t0buf1[i]=0;
  ShellExecute(abt_win,open_txt,web_address,0,0,SW_SHOW);
  return 0;
 }
// #endif
case WM_RBUTTONDOWN:
 curpos.x=(short)LOWORD(lParam); curpos.y=(short)HIWORD(lParam);
 if(curpos.y<19)goto sendclose;
 return 0;
case WM_MOUSEMOVE:
 curpos.x=(short)LOWORD(lParam);curpos.y=(short)HIWORD(lParam);
 if(wParam&MK_LBUTTON){
  if(abtmove){
   ClientToScreen(hwnd,&curpos);
   rect2.left+=curpos.x-cmx;rect2.top+=curpos.y-cmy;
   SetRect(&mpos,0,0,Dwd-rect2.right,Dht-rect2.bottom);
   LeaveInTheRect(&mpos,&rect2.left,&rect2.top); cmx=curpos.x;cmy=curpos.y;
   MoveWindow(abt_win,rect2.left,rect2.top,rect2.right,rect2.bottom,100);
  }
  break;
 }
// #ifndef IS_HM_VERSION
 if(IsIntheButt(&URLrect,curpos.x,curpos.y)){over=91; SetCursor(hCurs);}
 else over=0;
// #endif
 break;
case WM_LBUTTONUP: case WM_NCLBUTTONUP: ReleaseCapture(); ClipCursor(NULL); abtmove=0; return 0;
 case WM_COMMAND: if(LOWORD(wParam)==50004){
 sendclose:
 SendMessage(abt_win,WM_CLOSE,0,0);
 }
 return 0;
}return DefWindowProc(hwnd, message, wParam, lParam);}

DWORD RingClr[]={
 RGB(204,0,150),RGB(204,0,150),RGB(127,0,94),RGB(127,0,94),RGB(255,0,188),0,
 RGB(150,204,0),RGB(150,204,0),RGB(93,127,0),RGB(93,127,0),RGB(186,255,0)
};
BYTE RingCrd[]={1,3,0,4,2,0xFF,1,3,0,4,2,0xFF};

void CreateAboutWin(){
WNDCLASS wndc;
wndc.style=CS_BYTEALIGNCLIENT|CS_SAVEBITS;
wndc.lpfnWndProc=AbtProc;
wndc.cbClsExtra=0; wndc.cbWndExtra=0;
wndc.hInstance=gInst;
wndc.hIcon=0;
wndc.hCursor=LoadCursor(NULL,IDC_ARROW);
wndc.hbrBackground=0;
wndc.lpszMenuName=0;
wndc.lpszClassName=about_class;
RegisterClass(&wndc);
DeskWin=GetDesktopWindow();
GetClientRect(DeskWin,&rect1);
Dwd=rect1.right; Dht=rect1.bottom;
RECT rctr; rctr.right=335; rctr.bottom=266;
rect2.left=Dwd; rect2.left-=rctr.right; rect2.left*=0.5f;
rect2.top=Dht; rect2.top-=rctr.bottom; rect2.top*=0.5f;
rect2.right=rctr.right; rect2.bottom=rctr.bottom;
abt_win=CreateWindowEx(WS_EX_TOOLWINDOW,about_class,NULL,WS_CLIPSIBLINGS|WS_POPUP,rect2.left,rect2.top,rect2.right,rect2.bottom,main_win,NULL,gInst,NULL);
// Set Region
HRGN hrgn2,hrgn3;
rctr.left=0; rctr.top=rctr.left;
hrgn3=CreateRectRgnIndirect(&rctr);
hrgn1=CreateEllipticRgnIndirect(&rctr);
rctr.left+=5; rctr.top=rctr.left; rctr.right-=5; rctr.bottom-=5;
hrgn2=CreateEllipticRgnIndirect(&rctr);
CombineRgn(hrgn3,hrgn1,hrgn2,RGN_DIFF);
DeleteObject(hrgn1);
rctr.left+=2; rctr.top=rctr.left; rctr.right-=2; rctr.bottom-=2;
hrgn1=CreateEllipticRgnIndirect(&rctr);
CombineRgn(hrgn2,hrgn3,hrgn1,RGN_OR);
DeleteObject(hrgn1);
rctr.left+=5; rctr.top=rctr.left; rctr.right-=5; rctr.bottom-=5;
hrgn1=CreateEllipticRgnIndirect(&rctr);
CombineRgn(hrgn3,hrgn2,hrgn1,RGN_DIFF);
DeleteObject(hrgn1);
rctr.left+=2; rctr.top=rctr.left; rctr.right-=2; rctr.bottom-=2;
hrgn1=CreateEllipticRgnIndirect(&rctr);
CombineRgn(hrgn2,hrgn3,hrgn1,RGN_OR);
DeleteObject(hrgn1);DeleteObject(hrgn3);
rctr.left=3; rctr.top=rctr.left; rctr.right=335; rctr.bottom=266;
hrgn1=CreateEllipticRgnIndirect(&rctr);
HRGN hrgn4;
hrgn4=CreateEllipticRgnIndirect(&rctr);
rctr.left=0; rctr.top=21; rctr.right=337; rctr.bottom=266;
hrgn3=CreateRectRgnIndirect(&rctr);
CombineRgn(hrgn4,hrgn1,hrgn3,RGN_DIFF);
DeleteObject(hrgn3);
CombineRgn(hrgn1,hrgn2,hrgn4,RGN_OR);
DeleteObject(hrgn2);DeleteObject(hrgn4);
SetWindowRgn(abt_win,hrgn1,1);
///
SetWindowText(abt_win,smemTable->About);
if(hCurs==0)hCurs=LoadCursor(0,IDC_HAND);
GetClientRect(abt_win,&rect1);
float ClWidth,ClHeight,Clxc,Clyc,kf;
DWORD clhg,clwd; clhg=rect1.bottom; clwd=rect1.right;
ClWidth=rect1.right; ClHeight=rect1.bottom; cprtWd=(int)ClWidth;
hdc=GetDC(abt_win);memdc=mdc=CreateCompatibleDC(hdc);
ndc=CreateCompatibleDC(mdc);
tempbmp=CreateCompatibleBitmap(hdc,200,20);
mdc_btp=CreateCompatibleBitmap(hdc,rect1.right,rect1.bottom);
SelectObject(mdc,mdc_btp);
SelectObject(ndc,tempbmp);
//BitBlt(mdc,0,0,270,180,hdc,0,0,SRCCOPY);
SetStretchBltMode(mdc,HALFTONE); SetBkMode(mdc,TRANSPARENT);
HBRUSH transparent=(HBRUSH)GetStockObject(NULL_BRUSH);
SelectObject(mdc,transparent);
Clxc=ClWidth/2; Clyc=ClHeight/2; kf=Clxc+45;
int ss,ws;
for(ss=0;ss<=kf;ss++){
 int scolor=225*(1-ss/kf);
 DeleteObject(SelectObject(mdc,CreatePen(PS_SOLID,2,RGB(scolor,0,scolor))));
 Ellipse(mdc,Clxc-ss,Clyc-ss,Clxc+ss,(Clyc+ss)*0.85f);
}
clwd--; clhg--; ws=0; ss=0;
againRing:
for(;RingCrd[ss]!=0xFF;ss++){
DeleteObject(SelectObject(mdc,CreatePen(PS_SOLID,(RingCrd[ss]==2)?1:2,RingClr[ss])));
Ellipse(mdc,ws+RingCrd[ss],ws+RingCrd[ss],clwd-RingCrd[ss],clhg-RingCrd[ss]);
}
if(ws==0){ss++; ws=7; clwd-=7; clhg-=7; goto againRing;}
kf=10;
for(ss=0;ss<=kf;ss++){
 int scolor=225*(ss/kf);
 DeleteObject(SelectObject(mdc,CreatePen(PS_SOLID,1,RGB(26,scolor*0.89f,scolor))));
 MoveToEx(mdc,0,ss,NULL); LineTo(mdc,ClWidth,ss);
 scolor=225*(1-ss/kf);
 DeleteObject(SelectObject(mdc,CreatePen(PS_SOLID,1,RGB(26,scolor*0.89f,scolor))));
 MoveToEx(mdc,0,10+ss,0); LineTo(mdc,ClWidth,10+ss);
}
RECT rect11;
rect11.top=170+10; rect11.left=0; rect11.bottom=ClHeight; rect11.right=ClWidth;
HBITMAP hbmp;
hbmp=(HBITMAP)LoadImage(gInst,(char*)5,IMAGE_BITMAP,72+8,72+8,LR_DEFAULTCOLOR);
SelectObject(ndc,hbmp);
BitBlt(mdc,167-2,97-2,72+8,72+8,ndc,0,0,SRCCOPY);
DeleteObject(hbmp);
///// coding start
//"\xA9 Dmitry Komarov\x0ARussia, Nizhny Tagil\x0\x0"
/*char cpo[]="\xA9 Дмитрий Комаров\x0\x0"; BYTE cod1,cod2; DWORD n;
char towr[10], bfx[4];
HANDLE File=CreateFile("code.txt",GENERIC_WRITE,0,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
SetFilePointer(File,0,NULL,FILE_BEGIN);
for(int yyu=0; cpo[yyu]!=0; yyu+=2){cod1=cpo[yyu+1]; cod2=cpo[yyu]; cod1-=0x10; cod2-=5;
cod1^=0xFD; cod2^=0xFE;
itoa(cod1,bfx,10); lstrcpy(towr,bfx); lstrcat(towr,",");
itoa(cod2,bfx,10); lstrcat(towr,bfx); lstrcat(towr,",");
WriteFile(File,&towr,lstrlen(towr),&n,NULL);
}
WriteFile(File,"0",1,&n,NULL);
CloseHandle(File); */
///// coding end
//if(tOptions.lng==0){for(i=0;i<sizeof(devlpr)-1;i+=2){cprits[i]=(char)((devlpr[i+1]^0xFE)+5); cprits[i+1]=(char)((devlpr[i]^0xFD)+0x10);}cprits[56]=0;cprits[57]=(char)(sizeof(devlpr)-2);}
//else {for(i=0;i<sizeof(devlpr2)-1;i+=2){cprits[i]=(char)((devlpr2[i+1]^0xFE)+5); cprits[i+1]=(char)((devlpr2[i]^0xFD)+0x10);} cprits[sizeof(devlpr2)-2]=0; cprits[57]=(char)(sizeof(devlpr2)-2);}
int i;
LONG enctxtlen; unsigned char *penctxt;
if(ActiveLangID==RUS_LANGID){enctxtlen=sizeof(Rdevlpr); penctxt=(unsigned char*)Rdevlpr;}
else {enctxtlen=sizeof(devlpr);penctxt=(unsigned char*)devlpr;}
for(i=0;i<enctxtlen-1;i+=2){cprits[i]=(char)((penctxt[i+1]^0xFE)+5); cprits[i+1]=(char)((penctxt[i]^0xFD)+0x10);}cprits[56]=0;cprits[57]=(char)(enctxtlen-2);
ft.lfCharSet=RUSSIAN_CHARSET; ft.lfOrientation=0; ft.lfEscapement=0;
lstrcpy(ft.lfFaceName,TimesNewRoman_txt);
ft.lfHeight=37; ft.lfWidth=16; ft.lfWeight=500;
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
for(ss=10;ss>=0;ss--){int scolor=255*(1-ss/10.0f);SetTextColor(mdc,RGB(scolor,scolor,scolor/3));TextOut(mdc,88+ss*0.5f,31+ss*0.4f,"PROWiSe",7);}
SetTextColor(mdc,0x0000ffff);
ft.lfHeight=19; ft.lfWidth=8;
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
for(ss=10;ss>=0;ss--){int scolor=255*(1-ss/10.0f);SetTextColor(mdc,RGB(0,scolor,0));TextOut(mdc,88+ss*0.4f,62+ss*0.3f,"MANAGER",7);}
cclr[0]=30; cclr[1]=60; cclr[2]=40; cclr[3]=0; cclr[4]=0; cclr[5]=0;
ft.lfHeight=37; ft.lfWidth=16;
//lstrcpy(ft.lfFaceName,MSsansSerif_txt);
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
//TextOut(mdc,228,115,"v",1);
for(ss=10;ss>=0;ss--){int scolor=255*(1-ss/10.0f);SetTextColor(mdc,RGB(0,100+scolor*0.5f,scolor));TextOut(mdc,232+ss*0.4f,110-16+ss*0.3f,"v1.8",4);}
#ifdef BETA_RELEASE
 ft.lfHeight=18; ft.lfWidth=9;
 DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
 TextOut(mdc,260,142-15,"beta",4);
#endif
SetStretchBltMode(ndc,HALFTONE); SetBkMode(ndc,TRANSPARENT);
lstrcpy(ft.lfFaceName,Arial_txt);
ft.lfHeight=16; ft.lfWeight=100; ft.lfWidth=0;
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
if(ActiveLangID==RUS_LANGID){enctxtlen=sizeof(RdevlprCity); penctxt=(unsigned char*)RdevlprCity;}
else {enctxtlen=sizeof(devlprCity);penctxt=(unsigned char*)devlprCity;}
enctxtlen--;
for(i=0;i<enctxtlen;i+=2){
 t2buf4[i]=(char)((penctxt[i+1]^0xFE)+5);
 t2buf4[i+1]=(char)((penctxt[i]^0xFD)+0x10);
}
t2buf4[i]=0;
int x,y,color; x=25; y=132; color=0x00404000;
paint_city:
 SetTextColor(mdc,color);
 TextOut(mdc,x,y,t2buf4,i);
 if(color!=0x00ffff00){color=0x00ffff00; x--; y--; goto paint_city;}
color=0x00957400; 
//get build date
#ifdef BETA_RELEASE
 y=143;
#else
 y=130;
#endif
x=241;
LONGLONG ll; FILETIME ftm; SYSTEMTIME stm; char tBuf[15]; DWORD dw; BYTE *pbt;
tBuf[0]=0;
dw=(DWORD)(my_peb)->ImageBaseAddress; pbt=(BYTE*)dw;
dw+=0x3C; pbt+=*(DWORD*)dw;
if(*(DWORD*)pbt==0x00004550){
 dw=*((DWORD*)pbt+2);
 ll=Int32x32To64(dw,10000000)+116444736000000000;
 ftm.dwLowDateTime=(DWORD)ll; ll>>=32;
 ftm.dwHighDateTime=(DWORD)ll;
 FileTimeToSystemTime(&ftm,&stm);
 getDate(&stm,tBuf);
}
if((i=getstrlen(tBuf))!=0){
 paint_date:
 SetTextColor(mdc,color); TextOut(mdc,x,y,tBuf,i);
 if(color!=0x007bff00){color=0x007bff00; x--; y--; goto paint_date;}
}
ft.lfHeight=15; ft.lfWeight=700; ft.lfWidth=0;
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
SetTextColor(mdc,0x0000ffff);
RECT lrt;
lrt.left=110; lrt.right=110+108;
lrt.top=3; lrt.bottom=20;
DrawTextEx(mdc,smemTable->About,lstrlen(smemTable->About),&lrt,DT_CENTER|DT_VCENTER|DT_SINGLELINE,0);
ft.lfHeight=15; ft.lfWeight=100; ft.lfWidth=0;
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
///////WEB-SITE
SetTextColor(mdc,0x0000ff00);
lstrcpy(ft.lfFaceName,MSsansSerif_txt);
ft.lfHeight=18; ft.lfWeight=700; ft.lfWidth=0;
DeleteObject(SelectObject(mdc,CreateFontIndirect(&ft)));
//#ifndef IS_HM_VERSION
//URLrect.left=68; URLrect.right=266; URLrect.top=202; URLrect.bottom=URLrect.top+ft.lfHeight;
URLrect.left=125; URLrect.right=215; URLrect.top=202; URLrect.bottom=URLrect.top+ft.lfHeight;
//for(i=0;i<webAdr_len;i++){t2buf1[i]=(char)(web_address[i]^0x8D);} t2buf1[i]=0;
DrawTextEx(mdc,web_address_name,webAdrName_len,&URLrect,DT_CENTER|DT_VCENTER|DT_SINGLELINE,0);
//#endif
ShowWindow(abt_win,SW_SHOW);
SetTimer(abt_win,2,20,(TIMERPROC)NULL);
}

DWORD AboutThread(LPVOID){MSG msg; HACCEL haccel2;
CreateAboutWin();
if(abt_win==NULL)goto exit;
haccel2=LoadAccelerators(gInst,(char*)2);
while(GetMessage(&msg,NULL,0,0)){
 if(haccel2!=NULL && TranslateAccelerator(abt_win,haccel2,&msg))continue;
 TranslateMessage(&msg);DispatchMessage(&msg);
}
exit:
ExitThread(0);
return 0;}