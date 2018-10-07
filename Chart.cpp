#include "Chart.h"

BOOL grfclsreg=0;
HDC ndcVertGraf;
TCHAR GraphMonClass_txt[]="GraphMonitor_Class";

TCHAR txt_bt[4]=" b\0\0";
TCHAR txt_Kb[4]=" KB\0";
TCHAR txt_Mb[4]=" MB\0";
TCHAR txt_Gb[4]=" GB\0";

//////////////////////////////// DrawVertGraph /////////////////////////////////
void Chart_VertGraph_Draw(HWND hwnd){HDC hdc,chdc; RECT clt; LONG GraphHg; TCHAR tbuf[10],lta[10]; VERTICAL_GRAPH_DATA *vgd;
 int lnNum,lnUse,lnKern,drwHg; BYTE lnColorPos,Unit; LPTSTR pstr; DWORD den;
 if(!GETLONGPTR_VertGraphData(hwnd,&vgd))return;
 GetClientRect(hwnd,&clt); GraphHg=clt.bottom;
 chdc=vgd->genInfo.memdc;
 SelectObject(chdc,GetStockObject(DEFAULT_GUI_FONT));
 SetBkColor(chdc,vgd->genInfo.BackgColor);
 FillRect(chdc,&clt,vgd->genInfo.hBackgBrush);
 drwHg=GraphHg; lnKern=0;
 if(drwHg>23)drwHg-=23;
 lnNum=(drwHg>5)?(drwHg-5):drwHg; lnNum/=3;
 if(vgd==NULL)goto drawLines;
 if(vgd->dType&GRAPHTYPE_PERCENT){
  lnUse=lnNum; lnUse*=(vgd->numData1)*0.01;
  if(lnUse==0 && vgd->numData1>=0.01)lnUse=1;
  if(vgd->dType&GRAPHTYPE_DOUBLE){lnKern=lnNum*(vgd->numData2)*0.01;}
  lnColorPos=3; cpuToFractStr(vgd->numData1,tbuf,lta); lstrcat(tbuf," %");
 }
 else if(vgd->dType==GRAPHTYPE_BYTESIZE){
  lnUse=lnNum*(vgd->numData1);
  lnColorPos=9;
  if(vgd->numData3<1024){den=1; Unit=0;}
  else if(vgd->numData3>=1024 && vgd->numData3<1048576){den=1024; Unit=1;}
  else if(vgd->numData3>=1048576 && vgd->numData3<1073741824){den=1048576; Unit=2;}
  else if(vgd->numData3>=1073741824 && vgd->numData3<1099511627776){den=1073741824; Unit=3;}
  n=Math_div(vgd->numData3,den,tbuf,0);
  switch(Unit){
   case 0: pstr=txt_bt; break;
   case 1: pstr=txt_Kb; break;
   case 2: pstr=txt_Mb; break;
   case 3: pstr=txt_Gb; break;
  }
  lstrcpy((LPTSTR)tbuf+n,pstr);
 }
 clt.top=clt.bottom=GraphHg; clt.top-=22;
 SetTextColor(chdc,vgd->drwColor);
 DrawTextEx(chdc,tbuf,lstrlen(tbuf),&clt,DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE,NULL);
 drawLines:
 while(lnNum>0){
  if(lnKern){BitBlt(chdc,11,drwHg,39,3,ndcVertGraf,0,6,SRCCOPY); lnKern--; lnUse--;}
  else if(lnUse){BitBlt(chdc,11,drwHg,39,3,ndcVertGraf,0,lnColorPos,SRCCOPY); lnUse--;}
  else BitBlt(chdc,11,drwHg,39,3,ndcVertGraf,0,0,SRCCOPY);
  drwHg-=3; lnNum--;
 }
 hdc=GetDC(hwnd);
 BitBlt(hdc,0,0,clt.right,GraphHg,chdc,0,0,SRCCOPY);
 ReleaseDC(hwnd,hdc);
}

/////////////////////////// OscilGraph Find Max_Value //////////////////////////
VOID Chart_OscilGraph_FindMaxValue(OSCILGRAPH_DATA *ogd, OSCIL_GRAPH_DATA *gd, LONG GraphWd){float *curHist,*histLast; int xpos;
go_graph:
 histLast=gd->histLastAddr;
 curHist=gd->histCurPos;
 if(curHist==0 || histLast==0)goto next_graph;
 histLast--;
 xpos=GraphWd;
 // Determine a scale value
 for(;(DWORD_PTR)curHist>(gd->histStartAddr) && xpos>=0;curHist--){
  if(*(DWORD32*)curHist==0x03030303)continue;
  if(ogd->MaxValue<*curHist)ogd->MaxValue=*curHist;
  xpos-=2;
 }
 for(curHist=histLast;(DWORD_PTR)curHist>(DWORD_PTR)(gd->histCurPos) && xpos>=0;curHist--){
  if(*(DWORD32*)curHist==0x03030303)break;
  if(ogd->MaxValue<*curHist)ogd->MaxValue=*curHist;
  xpos-=2;
 }
next_graph:
 if(gd->NextGraphData){
  gd=(OSCIL_GRAPH_DATA*)(gd->NextGraphData);
  goto go_graph;
 }
}

//////////////////////////// Chart_OscilGraph_Draw /////////////////////////////
VOID Chart_OscilGraph_Draw(HWND hwnd){HDC hdc,chdc; RECT clt; LONG GraphWd,GraphHg,oscWpls,HorzSilent; HPEN prevPen;
 OSCILGRAPH_DATA *ogd; OSCIL_GRAPH_DATA *gd; BOOL bInverse,bScale;
 int DrawHg,dpoint,xpos; float *curHist,*histLast,scale;
 if(!GETLONGPTR_OscilGraph_Data(hwnd,&ogd))return;
 GetClientRect(hwnd,&clt); GraphWd=clt.right; GraphHg=clt.bottom;
 chdc=ogd->genInfo.memdc;
 SetGraphicsMode(chdc,GM_ADVANCED);
 SetBkColor(chdc,ogd->genInfo.BackgColor);
 FillRect(chdc,&clt,ogd->genInfo.hBackgBrush);
 //Draw Grid
 prevPen=(HPEN)SelectObject(chdc,ogd->hPenGrid);
 HorzSilent=ogd->oscGridHorzMove;
 if(!ogd->oscGridStatic){
  if((ogd->oscGridHorzDelta+HorzSilent)<12)ogd->oscGridHorzDelta+=HorzSilent;
  else ogd->oscGridHorzDelta=0;
 }
 oscWpls=ogd->oscGridHorzDelta;
 for(DrawHg=GraphHg;DrawHg>0;DrawHg-=12){MoveToEx(chdc,0,DrawHg,0); LineTo(chdc,GraphWd,DrawHg);}
 for(DrawHg=GraphWd;DrawHg>0;DrawHg-=12){MoveToEx(chdc,DrawHg-oscWpls,0,0); LineTo(chdc,DrawHg-oscWpls,GraphHg);}
 // Prepare to Draw the graphic
 gd=ogd->FirstGraphData;
 if(!IsVALID_OscilGraphData(gd))goto exit;
 bScale=((gd->dwFlags&CHART_FLAG_SCALE)==CHART_FLAG_SCALE);
 if(bScale)Chart_OscilGraph_FindMaxValue(ogd,gd,GraphWd);
 #ifdef MY_CHART_OSCILGRAPH_HORZLINES
  if(ogd->HorzLines.bUse[0] && ogd->MaxValue<ogd->HorzLines.Value[0])ogd->MaxValue=ogd->HorzLines.Value[0];
  if(ogd->HorzLines.bUse[1] && ogd->MaxValue<ogd->HorzLines.Value[1])ogd->MaxValue=ogd->HorzLines.Value[1];
 #endif
draw_graph:
 DrawHg=GraphHg; DrawHg-=3;
 if(bScale){
  if(ogd->MaxValue!=0)scale=DrawHg/ogd->MaxValue;
  else scale=0.0f;
 }
 else scale=DrawHg;
 // Draw Horizontal Lines
 #ifdef MY_CHART_OSCILGRAPH_HORZLINES
  if(ogd->HorzLines.bUse[0]){
   SelectObject(chdc,ogd->HorzLines.hPen[0]);
   dpoint=DrawHg-(scale*ogd->HorzLines.Value[0])+1.5;
   MoveToEx(chdc,0,dpoint,0);
   LineTo(chdc,GraphWd,dpoint);
  }
  if(ogd->HorzLines.bUse[1]){
   SelectObject(chdc,ogd->HorzLines.hPen[1]);
   dpoint=DrawHg-(scale*ogd->HorzLines.Value[1])+1.5;
   MoveToEx(chdc,0,dpoint,0);
   LineTo(chdc,GraphWd,dpoint);
  }
 #endif
 // Draw the graphic
 if(gd->hPen!=0)SelectObject(chdc,gd->hPen);
 histLast=gd->histLastAddr;
 curHist=gd->histCurPos;
 if(curHist==0 || histLast==0)goto next_graph;
 if(gd->dwFlags&CHART_FLAG_HIDDEN)goto next_graph;
 histLast--;
 xpos=GraphWd;
 bInverse=((gd->dwFlags&CHART_FLAG_INVERSE_HG)==CHART_FLAG_INVERSE_HG);
 dpoint=(scale*(*curHist))+1.5;
 MoveToEx(chdc,xpos,dpoint,0);
 for(;(DWORD_PTR)curHist>(gd->histStartAddr) && xpos>=0;curHist--){
  if(*(DWORD32*)curHist==0x03030303)continue;
  if(!bInverse)dpoint=DrawHg-(scale*(*curHist))+1.5;
  else dpoint=(scale*(*curHist))+1.5;
  LineTo(chdc,xpos,dpoint);
  xpos-=HorzSilent;//oscWpls
 }
 for(curHist=histLast;(DWORD_PTR)curHist>(DWORD_PTR)(gd->histCurPos) && xpos>=0;curHist--){
  if(*(DWORD32*)curHist==0x03030303)break;
  if(!bInverse)dpoint=DrawHg-(scale*(*curHist))+1.5;
  else dpoint=(scale*(*curHist))+1.5;
  LineTo(chdc,xpos,dpoint);
  xpos-=HorzSilent;
 }
next_graph:
 if(gd->NextGraphData){
  gd=(OSCIL_GRAPH_DATA*)(gd->NextGraphData);
  if(IsVALID_OscilGraphData(gd))goto draw_graph;
 }
exit:
 SelectObject(chdc,prevPen);
 hdc=GetDC(hwnd);
 BitBlt(hdc,0,0,GraphWd,GraphHg,chdc,0,0,SRCCOPY);
 ReleaseDC(hwnd,hdc);
}

VOID Chart_Initialize(HWND hwnd,void* cginf, BYTE chartType){HDC hdc; RECT rect; CHART_GENERAL_INFO *ctgi;
 if(cginf==0)return;
 ctgi=(CHART_GENERAL_INFO*)cginf;
 VALIDATE_ChartGenInfo(ctgi);
 hdc=GetDC(hwnd); ctgi->memdc=CreateCompatibleDC(hdc);
 GetClientRect(hwnd,&rect);
 ctgi->hbmp=CreateCompatibleBitmap(hdc,rect.right,rect.bottom);
 DeleteObject(SelectObject(ctgi->memdc,ctgi->hbmp));
 SetBkColor(ctgi->memdc,0); FillRect(ctgi->memdc,&rect,ctgi->hBackgBrush);
 ctgi->ChartType=chartType;
 SetWindowLongPtr(hwnd,GWLP_CHART_INTERNAL_STRUCT_PTR,(DWORD_PTR)ctgi);
 ReleaseDC(hwnd,hdc);
 if(ctgi->ChartType==CHART_DIAGRAMM)Chart_OscilGraph_Draw(hwnd);
 else if(ctgi->ChartType==CHART_VERTICAL)Chart_VertGraph_Draw(hwnd);
}

#ifdef MY_CHART_OSCILGRAPH_POPUPMENU
VOID Chart_PopupMenu(HWND hwnd, CHART_GENERAL_INFO *pcgi){HMENU pmenu;  POINT curpos;
 if((pmenu=CreatePopupMenu())==0)return;
 GetCursorPos(&curpos);
 if(pcgi->PopupMenuInit!=0)pcgi->PopupMenuInit(pmenu,pcgi);
 #ifdef MY_CHART_OSCILGRAPH_HORZLINES
 BOOL *pbl; DWORD dw; char *ptr; MENUITEMINFO mii;
 mii.cbSize=sizeof(MENUITEMINFO);
 mii.fMask=MIIM_TYPE|MIIM_STATE|MIIM_ID;
 mii.fType=MFT_STRING;
 if(ogd->HorzLines.Value[0]>ogd->HorzLines.Value[1]){pbl=&ogd->HorzLines.bUse[0]; dw=ogd->HorzLines.Value[0];}
 else {pbl=&ogd->HorzLines.bUse[1]; dw=ogd->HorzLines.Value[1];}
 if(*pbl)mii.fState=MFS_ENABLED|MFS_CHECKED;
 else mii.fState=MFS_ENABLED;
 mii.dwTypeData="Верхняя граница (%d)"; mii.wID=30001;
 if((ptr=(char*)LocalAlloc(LPTR,getstrlen(mii.dwTypeData)+1+100))!=0){wsprintf(ptr,mii.dwTypeData,dw); mii.dwTypeData=ptr;}
 InsertMenuItem(pmenu,0,1,&mii);
 LocalFree(ptr);
 if(ogd->HorzLines.Value[0]<ogd->HorzLines.Value[1]){pbl=&ogd->HorzLines.bUse[0]; dw=ogd->HorzLines.Value[0];}
 else {pbl=&ogd->HorzLines.bUse[1]; dw=ogd->HorzLines.Value[1];}
 if(*pbl)mii.fState=MFS_ENABLED|MFS_CHECKED;
 else mii.fState=MFS_ENABLED;
 mii.dwTypeData="Нижняя граница (%d)"; mii.wID=30002;
 if((ptr=(char*)LocalAlloc(LPTR,getstrlen(mii.dwTypeData)+1+100))!=0){wsprintf(ptr,mii.dwTypeData,dw); mii.dwTypeData=ptr;}
 InsertMenuItem(pmenu,1,1,&mii);
 LocalFree(ptr);
 #endif
 TrackPopupMenuEx(pmenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos.x,curpos.y,hwnd,NULL);
 DestroyMenu(pmenu);
}
#endif

/*

DWORD chart_settsdlg_items1[]={106,206,107,207,0xFFFF};
DWORD chart_settsdlg_items2[]={109,209,110,210,0xFFFF};

typedef struct _CHART_SETTINGS_DATA{
 CHART_GENERAL_INFO* ctgi;
 COLORREF clrGraph;
 COLORREF clrBackg;
 COLORREF clrGrid;
 struct {
  BOOL bUse[2];
  LONG Value[2];
  COLORREF Color[2];
 }HorzLines;
}CHART_SETTINGS_DATA;

BOOL CALLBACK Chart_SettingsProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){CHART_GENERAL_INFO* ctgi; CHART_SETTINGS_DATA *csdt;
switch(Message){
 case WM_INITDIALOG:
  RECT rect; int i; char buf[20];
  SetWindowInCenter(hDlg,&rect,GetParent(hDlg));
  if(lParam){
   SetWindowLongPtr(hDlg,GWLP_CHART_INTERNAL_STRUCT_PTR,(LONG_PTR)ctgi);
   ctgi=(CHART_GENERAL_INFO*)lParam;
   if(ctgi->ChartType!=CHART_DIAGRAMM)break;
   if((csdt=(CHART_SETTINGS_DATA*)LocalAlloc(LPTR,sizeof(CHART_SETTINGS_DATA)))==0)break;
   csdt->ctgi=ctgi;
   // copy graph data
   csdt->HorzLines.bUse[0]=((OSCILGRAPH_DATA*)ctgi)->HorzLines.bUse[0];
   csdt->HorzLines.bUse[1]=((OSCILGRAPH_DATA*)ctgi)->HorzLines.bUse[1];
   csdt->HorzLines.Value[0]=((OSCILGRAPH_DATA*)ctgi)->HorzLines.Value[0];
   csdt->HorzLines.Value[1]=((OSCILGRAPH_DATA*)ctgi)->HorzLines.Value[1];
   //
   Message=csdt->HorzLines.bUse[0];
   while(chart_settsdlg_items1[i]!=0xFFFF){EnableWindow(GetDlgItem(hDlg,chart_settsdlg_items1[i]),Message); i++;}
   CheckDlgButton(hDlg,105,(Message)?BST_CHECKED:BST_UNCHECKED);
   Message=csdt->HorzLines.bUse[1];
   while(chart_settsdlg_items1[i]!=0xFFFF){EnableWindow(GetDlgItem(hDlg,chart_settsdlg_items2[i]),Message); i++;}
   CheckDlgButton(hDlg,108,(Message)?BST_CHECKED:BST_UNCHECKED);
   ltoa(csdt->HorzLines.Value[0],buf,10); SetDlgItemText(hDlg,206,buf);
   ltoa(csdt->HorzLines.Value[1],buf,10); SetDlgItemText(hDlg,209,buf);
  }
  break;
 case WM_DRAWITEM:
  if(((DRAWITEMSTRUCT*)lParam)->CtlType==ODT_BUTTON){
   if(((DRAWITEMSTRUCT*)lParam)->CtlID==202){
    Draw_ColoredButton((DRAWITEMSTRUCT*)lParam,);
   }
  }
  break;
 case WM_COMMAND:
  break;
 case WM_CLOSE:
  DestroyWindow(hDlg);
  return 1;
 case WM_DESTROY:
  break;
} return 0;}

//CreateDialogParam(gInst,(char*)8,hwnd,(DLGPROC)Chart_SettingsProc,(LPARAM)ctgi);
*/

LRESULT CALLBACK Chart_WinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam){CHART_GENERAL_INFO* ctgi;
switch(Message){
 case WM_PAINT:
  PAINTSTRUCT ps1; HDC hdc;
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  BeginPaint(hwnd,&ps1);
  hdc=GetDC(hwnd);
  BitBlt(hdc,ps1.rcPaint.left,ps1.rcPaint.top,ps1.rcPaint.right-ps1.rcPaint.left,
  ps1.rcPaint.bottom-ps1.rcPaint.top,ctgi->memdc,ps1.rcPaint.left,ps1.rcPaint.top,SRCCOPY);
  ReleaseDC(hwnd,hdc);
  EndPaint(hwnd,&ps1);
  return 0;
#ifdef MY_CHART_OSCILGRAPH_DBLCLICK
 case WM_LBUTTONDBLCLK:
   if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
   if(ctgi->OnDblClick!=0){
    BOOL bUpdate; bUpdate=0;
    if(ctgi->OnDblClick(hwnd,wParam,ctgi,&bUpdate)==0){//i.e. function processes this message
     if(bUpdate){
      if(ctgi->ChartType==CHART_DIAGRAMM){
       ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=1;
       Chart_OscilGraph_Draw(hwnd);
       ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=0;
      }
      else if(ctgi->ChartType==CHART_VERTICAL){
       Chart_VertGraph_Draw(hwnd);
      }
     }
     return 0;
    }
   }
   break;
#endif //MY_CHART_OSCILGRAPH_DBLCLICK
#ifdef MY_CHART_OSCILGRAPH_POPUPMENU
 case WM_RBUTTONDOWN:
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  ctgi->WindowEvents|=WND_EVENT_MOUSE_RBTNDOWN;
  return 0;
 case WM_RBUTTONUP:
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  if(ctgi->PopupMenuInit!=0){
   if((ctgi->WindowEvents&WND_EVENT_MOUSE_RBTNDOWN)==WND_EVENT_MOUSE_RBTNDOWN){
    ctgi->WindowEvents&=~WND_EVENT_MOUSE_RBTNDOWN;
    Chart_PopupMenu(hwnd,ctgi);
   }
  }
  return 0;
 case WM_NCMOUSEMOVE:
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  ctgi->WindowEvents&=~WND_EVENT_MOUSE_RBTNDOWN;
  return 0;
 case WM_COMMAND:
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  if(HIWORD(wParam)==0 && lParam==0){//i.e. the message is from a menu
   if(ctgi->PopupMenuWmCommand!=0){
    BOOL bUpdate; bUpdate=0;
    if(ctgi->PopupMenuWmCommand(hwnd,LOWORD(wParam),ctgi,&bUpdate)==0){//i.e. function processes this message
     if(bUpdate){
      if(ctgi->ChartType==CHART_DIAGRAMM){
       ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=1;
       Chart_OscilGraph_Draw(hwnd);
       ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=0;
      }
      else if(ctgi->ChartType==CHART_VERTICAL){
       Chart_VertGraph_Draw(hwnd);
      }
     }
     return 0;
    }
   }
  }
  #ifdef MY_CHART_OSCILGRAPH_HORZLINES
   if(ctgi->ChartType==CHART_DIAGRAMM){
    if(LOWORD(wParam)==30001 || LOWORD(wParam)==30002){
     BOOL *pbl;
     if(LOWORD(wParam)==30001){
      if(((OSCILGRAPH_DATA*)ctgi)->HorzLines.Value[0]>((OSCILGRAPH_DATA*)ctgi)->HorzLines.Value[1]){pbl=&((OSCILGRAPH_DATA*)ctgi)->HorzLines.bUse[0];}
      else {pbl=&((OSCILGRAPH_DATA*)ctgi)->HorzLines.bUse[1];}
      *pbl=(*pbl)?0:1;
      if(!*pbl){((OSCILGRAPH_DATA*)ctgi)->MaxValue=0;}
     }
     else{
      if(((OSCILGRAPH_DATA*)ctgi)->HorzLines.Value[0]<((OSCILGRAPH_DATA*)ctgi)->HorzLines.Value[1]){pbl=&((OSCILGRAPH_DATA*)ctgi)->HorzLines.bUse[0];}
      else {pbl=&((OSCILGRAPH_DATA*)ctgi)->HorzLines.bUse[1];}
      *pbl=(*pbl)?0:1;
      if(!*pbl){((OSCILGRAPH_DATA*)ctgi)->MaxValue=0;}
     }
     ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=1;
     Chart_OscilGraph_Draw(hwnd);
     ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=0;
     return 0;
    }
   }
  #endif //MY_CHART_OSCILGRAPH_HORZLINES
  break;
#endif //MY_CHART_OSCILGRAPH_POPUPMENU
 case WM_SIZE:
  if(wParam==SIZE_MAXHIDE || wParam==SIZE_MAXSHOW || wParam==SIZE_MINIMIZED)break;
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  RECT rect;
  GetClientRect(hwnd,&rect);
  if((hdc=GetDC(hwnd))!=0){
   ctgi->hbmp=CreateCompatibleBitmap(hdc,rect.right,rect.bottom);
   DeleteObject(SelectObject(ctgi->memdc,ctgi->hbmp));
   if(ctgi->ChartType==CHART_DIAGRAMM){
    ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=1;
    Chart_OscilGraph_Draw(hwnd);
    ((OSCILGRAPH_DATA*)ctgi)->oscGridStatic=0;
   }
   else if(ctgi->ChartType==CHART_VERTICAL)Chart_VertGraph_Draw(hwnd);
   InvalidateRect(hwnd,&rect,0);
   ReleaseDC(hwnd,hdc);
  }
  return 0;
 case WM_DESTROY:
  if(!GETLONGPTR_ChartGenInfo(hwnd,&ctgi))break;
  DeleteObject(ctgi->hbmp);
  DeleteDC(ctgi->memdc);
  return 0;
 }
 return DefWindowProc(hwnd, Message, wParam, lParam);
}

VOID Chart_RegisterClass(){WNDCLASSEX wndcex;
 wndcex.cbSize=sizeof(WNDCLASSEX);
 if(GetClassInfoEx(gInst,GraphMonClass_txt,&wndcex))return;
 #ifdef MY_CHART_OSCILGRAPH_DBLCLICK
  wndcex.style=CS_DBLCLKS;
 #else
  wndcex.style=0;
 #endif
 wndcex.lpfnWndProc=Chart_WinProc;
 wndcex.cbClsExtra=0;
 wndcex.cbWndExtra=4;
 wndcex.hInstance=gInst;
 wndcex.hIcon=0;
 wndcex.hCursor=LoadCursor(NULL,IDC_ARROW);
 wndcex.lpszMenuName=0;
 wndcex.hIconSm=0;
 wndcex.hbrBackground=0;//blackBr;//GetStockObject(LTGRAY_BRUSH);
 wndcex.lpszClassName=GraphMonClass_txt;
 HBITMAP hbtp;
 if((hbtp=(HBITMAP)LoadImage(gInst,(char*)1,IMAGE_BITMAP,39,12,LR_DEFAULTCOLOR))!=0){
  ndcVertGraf=CreateCompatibleDC(0);
  SelectObject(ndcVertGraf,hbtp);
  DeleteObject(hbtp);
 }
 if(RegisterClassEx(&wndcex)!=0)grfclsreg=1;
}

VOID Chart_UnRegisterClass(){
 if(grfclsreg)UnregisterClass(GraphMonClass_txt,gInst);
 grfclsreg=0;
}
