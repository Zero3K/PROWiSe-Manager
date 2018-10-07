#ifndef __MY_CHART_H
#define __MY_CHART_H

// ------------------------- CHART's Functionality -----------------------------
#undef MY_CHART_OSCILGRAPH_HORZLINES// disable/enable use of HorzLines

#define MY_CHART_OSCILGRAPH_WNDEVENTS// disable/enable use of window's events

#ifdef MY_CHART_OSCILGRAPH_WNDEVENTS
 #define MY_CHART_OSCILGRAPH_POPUPMENU// disable/enable use of popup menu
 #define MY_CHART_OSCILGRAPH_DBLCLICK// disable/enable use of double click
#endif
// -----------------------------------------------------------------------------

#define GRAPHTYPE_PERCENT 2
#define GRAPHTYPE_BYTESIZE 4
#define GRAPHTYPE_DOUBLE 8

#define CHART_DIAGRAMM 0x01
#define CHART_VERTICAL 0x02

#define CHART_FLAG_INVERSE_HG 0x01// Mirror graph by vertical
#define CHART_FLAG_SCALE      0x02// Enable scaling
#define CHART_FLAG_HIDDEN     0x04// Hide this graph

#define GWLP_CHART_INTERNAL_STRUCT_PTR 0

#define WND_EVENT_MOUSE_RBTNDOWN 1

typedef struct _CHART_GENERAL_INFO{
 DWORD cbSize;
 BYTE ChartType;
 HDC memdc;
 HBITMAP hbmp;
 HBRUSH hBackgBrush;
 COLORREF BackgColor;
 #ifdef MY_CHART_OSCILGRAPH_POPUPMENU
  VOID (WINAPI *PopupMenuInit)(IN HMENU hmenu, IN LPVOID lpGraphData);
  BOOL (WINAPI *PopupMenuWmCommand)(IN HWND hwnd, IN DWORD dwItemId, IN LPVOID lpGraphData, OUT LPBOOL lpbUpdate);// If an application processes this message, it should return zero.
 #endif //MY_CHART_OSCILGRAPH_POPUPMENU
 #ifdef MY_CHART_OSCILGRAPH_DBLCLICK
  BOOL (WINAPI *OnDblClick)(IN HWND hwnd, IN WPARAM wParam, IN LPVOID lpGraphData, OUT LPBOOL lpbUpdate);// If an application processes this message, it should return zero.
 #endif //MY_CHART_OSCILGRAPH_DBLCLICK
 #ifdef MY_CHART_OSCILGRAPH_WNDEVENTS
  DWORD WindowEvents;// Internal use.
 #endif //MY_CHART_OSCILGRAPH_WNDEVENTS
}CHART_GENERAL_INFO;

typedef struct _VERTICAL_GRAPH_DATA{// Data of the entire VerticalGraph and each of it graphs
 CHART_GENERAL_INFO genInfo;
 COLORREF drwColor;
 double  numData1;
 double  numData2;
 __int64 numData3;
 BYTE dType;
}VERTICAL_GRAPH_DATA;

typedef struct _OSCIL_GRAPH_DATA{// Data of each graph in the OscilGraph
 DWORD dwFlags;
 HPEN hPen;
 DWORD_PTR histStartAddr;
 float *histCurPos;
 float *histLastAddr;
 _OSCIL_GRAPH_DATA *NextGraphData;
}OSCIL_GRAPH_DATA;

typedef struct _OSCILGRAPH_DATA{// Data of the entire OscilGraph
 CHART_GENERAL_INFO genInfo;
 HPEN hPenGrid;
 BOOL oscGridStatic;// Static mean Dont move grid on Draw() call.
 LONG oscGridHorzDelta;// Internal.
 LONG oscGridHorzMove;// a value by wich increase the oscGridHorzDelta at every update
 #ifdef MY_CHART_OSCILGRAPH_HORZLINES
  struct {
   BOOL bUse[2];
   LONG Value[2];
   HPEN hPen[2];
  }HorzLines;
 #endif
 float MaxValue;
 OSCIL_GRAPH_DATA *FirstGraphData;
}OSCILGRAPH_DATA;

#define IsVALID_VertGraphData(pVar) ((BOOL)(pVar!=0 && pVar->genInfo.cbSize==sizeof(CHART_GENERAL_INFO)))
#define GETLONGPTR_VertGraphData(hDlg,ppVar) ((BOOL)((*ppVar=(VERTICAL_GRAPH_DATA*)GetWindowLongPtr(hDlg,GWLP_CHART_INTERNAL_STRUCT_PTR))!=0 && *ppVar->genInfo.cbSize==sizeof(CHART_GENERAL_INFO)))

#define IsVALID_OscilGraph_Data(pVar) ((BOOL)(pVar!=0 && pVar->genInfo.cbSize==sizeof(CHART_GENERAL_INFO)))
#define GETLONGPTR_OscilGraph_Data(hDlg,ppVar) ((BOOL)((*ppVar=(OSCILGRAPH_DATA*)GetWindowLongPtr(hDlg,GWLP_CHART_INTERNAL_STRUCT_PTR))!=0 && *ppVar->genInfo.cbSize==sizeof(CHART_GENERAL_INFO)))

//#define VALIDATE_OscilGraphData(pVar) (pVar->cbSize=sizeof(OSCIL_GRAPH_DATA))
//#define INVALIDATE_OscilGraphData(pVar) (pVar->cbSize=0)
#define IsVALID_OscilGraphData(pVar) ((BOOL)(pVar!=0))

#define VALIDATE_ChartGenInfo(pVar) (pVar->cbSize=sizeof(CHART_GENERAL_INFO))
#define INVALIDATE_ChartGenInfo(pVar) (pVar->cbSize=0)
#define IsVALID_ChartGenInfo(pVar) ((BOOL)(pVar!=0 && pVar->cbSize==sizeof(CHART_GENERAL_INFO)))
#define GETLONGPTR_ChartGenInfo(hDlg,ppVar) ((BOOL)((*ppVar=(CHART_GENERAL_INFO*)GetWindowLongPtr(hDlg,GWLP_CHART_INTERNAL_STRUCT_PTR))!=0 && *ppVar->cbSize==sizeof(CHART_GENERAL_INFO)))

#endif