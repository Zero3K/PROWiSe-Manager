////////////////////////////////////// VARIABLES DECLARATION /////////////////////////////////
VERTICAL_GRAPH_DATA vdCpu,vdCommit; OSCIL_GRAPH_DATA gdCpu,gdCommit,gdCpuKrnl; OSCILGRAPH_DATA mainperf_ogdCpu,mainperf_ogdMem;
HINSTANCE gInst,hThemeLib=0;

// Global variables for assembly function compatibility
extern DWORD _Enable_SetTop;
HWND run_win,main_win,strtrun_win,hstatus,processes_win,abt_win,services_win,windows_win,sysi_win,drvi_win,hndls_win=0,settings_win=0;
HWND hProcessList=0,hModuleList=0,hHandleList=0,StRunList=0,WindowsList=0,DrvList=0,hdnList=0;
HWND *hWinChain; DWORD WinChain_End;
HKEY hMainKey;
HIMAGELIST hUsageImg,himgl,hObjtpImg,hTbimgl,hMenuImgl=0;
HANDLE pHeap=0;
HBITMAP barrdn,barrup; HMENU lng_menu,main_menu;
HFONT hNormalFont=0; int nmFontLoads=0;
HANDLE spi_mem,lPaneThread=0,hUpdateThread=0; DWORD lPaneThreadId,UpdateThreadId;
DWORD *usrHistStart,*krnHistStart,*cmtHistStart,*nbuf5;
DWORD prevPageFaults,prevPageRead,prevPageWrite,prevMappedWrite,prevContextSwitch,prevIORead,prevIOWrite,prevIOOther;
float commit_prc;
char *tbuf3,*WinDir,*DrvDir,*SysDir,*tbuf8,*tbuf10,*tbuf_gfvdi,*dependsPath=0,*tbExpand;
char *t0buf1,*t1buf1,*t1buf9,*t2buf1,*t0buf2,*t1buf2,*t2buf2,*t1buf4,*t2buf4,*t1buf512;
DWORD *t0nbuf1,*t0nbuf2,*t1nbuf1,*t1nbuf2,*t1nbuf3,*t2nbuf1,*t2nbuf2,*t0nbuf4,*t1nbuf4,*t2nbuf4;
char *fbuf1,*buf0;
char *cpuusg_tout;
wchar_t *objTypesNames;
UNICODE_STRING gNamedPipeW,gMailslotW;
void *stringsMem;
DWORD_PTR ExecuteAddr=0,usrHistEnd,krnHistEnd,cmtHistEnd,hwndsEnd;
DWORD cfgsize1,vm1sz,DlgClassStyl=0,spi_memsz;
int itemSel,prevSel,in,tabtop,WinDirlen,SysDirlen,statusHg;
bool flessw=1,flessh=1,mxzd=0,updPriority=0,updCpu=0,basicInfo=0,LogSteps=0,bWarnSign=0,bHideOnRun=0,isMiniNT;
double cpuIdleTime=1,dbSystemTime=0x004C4B40,cpuUsage,oldKernelTm=0,cpuKernelTime=0;
unsigned __int64 oldIdleTm=0,oldCurTm=0,newIdleTm,newCurTm;
DWORD funcScanSleep=1500,funcUpdTimer=2000,UpdateSleepTm=1000;
DWORD dwLogonType,ActiveLangID,cur_pid,processCount,prevProcCount,processCountUser,prevProcCountUser;
BYTE paneType=0,pfORmem=1,mnProcsDisbl=0,FillHndlTbl=0;
RECT wr,drect,listsz; LONG initHg,initWd;
WORD wID;
LARGE_INTEGER lint;
PROCESS_INFO *fpinfo=0,*plastinf,*intrdpcs;
MODULE_INFOBUF *pFirstModuleInfo=0;
PROGSETTINGS progSetts,tempSetts; MAIN_SETTINGS mainSetts;
OBJTYPES_NAMES_TABLE *ObjTypesNmTable;
//LIST_SORT_DIRECTION ProcsSortd={0xFFFFFFFF,0,0};//,DrvSortd={0xFFFFFFFF,0,0};
SYSTEM_BASIC_INFORMATION *SysBasicInf;
SYSTEM_TIME_OF_DAY_INFORMATION *sysTimeInf;
SYSTEM_PERFORMANCE_INFORMATION *sysPerfInf;
SYSTEM_PROCESSOR_TIMES *sysProcessorTm;
SYSTEM_HANDLE_INFORMATION *handleInfo;
BOOL basicInfoErr,pebLdrData_verErr,processesUpd=0,bReSortList=0,firsttime=1,bShowAllProcesses=0,bIsUserAdmin=0;
WORD cpuusglen=0;
PEB* my_peb;
DLGHDR *pHdr;
STRINGS_MEM_TABLE *smemTable;
DLGITEM_TEXT_TABLE *dlgTAT;
PROGRAM_DIRECTORY_PATH ProgramDirPath={0,0};
LNGITEM_INFO *lngFirstInf=0; char *ActiveLngHelp=0;
NOTIFYICONDATA nid;
HFONT hDlgFont; HACCEL haccel;
BYTE iSel=0xFF;
RECT szTab,szstat,strect,*mainRect;
DWORD newHeight,newWidth,winHeight,winWidth,winClHeight,tabWidth,tabHeight,hgminus;
int cxMargin,cyMargin;
////
bool bTurnOffKernelCalc=1;
////
void *wpbuf1,*wpbuf3;
char *pvbuf1;
bool bPaneDestroy;
///command-line modes
BOOL cmd_Remove=0,cmd_NoRegSetts=0,cmd_LogonAs=0,cmd_LogonAsAdmin=0,cmd_Logon=0;


                                        //{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};
BYTE ColumnsCreateOrder[PRCCOLUMNS_COUNT]={1,2,0,0,4,5,0,6,0,0, 0, 0, 7, 0, 8, 0, 3, 0, 0, 0, 0, 0, 0};
BYTE ColumnsAlign[PRCCOLUMNS_COUNT]={DT_LEFT,DT_RIGHT,DT_RIGHT,DT_LEFT,DT_RIGHT,DT_LEFT,DT_RIGHT,DT_RIGHT,DT_RIGHT,DT_LEFT,DT_LEFT,DT_RIGHT,DT_RIGHT,DT_RIGHT,DT_RIGHT,DT_RIGHT,DT_LEFT,DT_LEFT,DT_LEFT,DT_LEFT,DT_RIGHT,DT_RIGHT,DT_RIGHT};
int Prcclnumber[PRCCOLUMNS_COUNT];
/*char ColumnTitle[COLUMNS_COUNT][17]={
"Process",//0
"ID",//1
"P.ID",//2
"Path",//3
"CPU",//4
"User",//5
"Priority",//6
"Threads",//7
"Start time",//8
"CPU Time",//9
"Command Line",//10
"Handles",//11
"VM Size",//12 //VM Size //����. ������
"Private bytes",//13 //Pagefile Usage
"Working Set",//14 //Working Set //���. ������
"Commit Charge",//15
"Description",//16
"Version",//17
"Company Name",//18
"Window Title",//19
"Session",//20
"Context Sw",//21
"CSw Delta"//22
};*/
WORD ColumnWidth[PRCCOLUMNS_COUNT]={125,37,37,290,35,137,58,55,155,79,290,55,60,79,75,70,225,110,145,150,65,75,65};
int ProcsOrderArray[PRCCOLUMNS_COUNT]; BOOL bValidProcsOrderArray; BYTE Upd[PRCCOLUMNS_COUNT],bLoadIcons;

			                               //{0,1,2,3,4,5,6,7,8};
BYTE ProcsDllColumnsCreateOrder[DLLCOLUMNS_COUNT]={1,2,5,0,3,4,0,0};
BYTE ProcsDllColumnsAlign[DLLCOLUMNS_COUNT]={DT_LEFT,DT_LEFT,DT_LEFT,DT_LEFT,DT_RIGHT,DT_RIGHT,DT_LEFT,DT_RIGHT};
int Dllclnumber[DLLCOLUMNS_COUNT];
/*char DllColumnTitle[DLLCOLUMNS_COUNT][13]={
"File",//0
"Path",//1
"Description",//2
"Company Name",//3
"Image Base",//4
"Image Size",//5
"Version"//6
"Image Base Address",//7
};*/
WORD ProcsDllColumnWidth[DLLCOLUMNS_COUNT]={120,300,100,200,77,77,87,77};
int ProcsDllOrderArray[COLUMNS_COUNT]; BOOL bValidProcsDllOrderArray;

					                       //{0,1,2,3,4,5,6};
BYTE WinColumnsCreateOrder[WINCOLUMNS_COUNT]={1,2,0,0,0,0,3}; BYTE UpdWin[WINCOLUMNS_COUNT];
int Winclnumber[WINCOLUMNS_COUNT];
/*char WinColumnTitle[WINCOLUMNS_COUNT][13]={
"Handle",//0
"Caption",//1
"Class Name",//2
"Rectangle",//3
"Style, hex",//4
"ExStyle",//5
"Status"//6
};*/
WORD WinColumnWidth[WINCOLUMNS_COUNT]={78,350,130,100,65,65,95};
int WinOrderArray[COLUMNS_COUNT]; BOOL bValidWinOrderArray;

WORD HndlColumnWidth[3]={80,100,500};
WORD StartupColWidth[3]={140,220,300};

BYTE SrvColumnsCreateOrder[SRVCOLUMNS_COUNT]={1,2,3,4,5,6,0,0,0};
int Srvclnumber[SRVCOLUMNS_COUNT];
/*
Name //0 str [1]
FriendName //1 str [2]
Status //2
BootType //3
Start LoginName //4 str [3]
FileCompany //5 str [4]
FileCreateDate //6
FileModifDate //7
RegCreateDate //8
*/
WORD ServColWidth[SRVCOLUMNS_COUNT]={140,220,70,60,150,150,100,100,100};
int SrvOrderArray[COLUMNS_COUNT]; BOOL bValidSrvOrderArray;

BYTE DrvColumnsCreateOrder[DRVCOLUMNS_COUNT]={1,2,3,0,0,0,0};
BYTE DrvColumnsAlign[DRVCOLUMNS_COUNT]={DT_LEFT,DT_LEFT,DT_LEFT,DT_LEFT,DT_RIGHT,DT_RIGHT,DT_LEFT};
int Drvclnumber[DRVCOLUMNS_COUNT];
WORD DrvColumnWidth[DRVCOLUMNS_COUNT]={95,255,200,150,77,77,87};
int DrvOrderArray[COLUMNS_COUNT]; BOOL bValidDrvOrderArray;

WORD SrvInProccessColumnWidth[2]={120,0};