
DLGITEM_TEXT d6[]={
{offsetof(STRINGS_MEM_TABLE,CPUUsageHistory), 1103},
{offsetof(STRINGS_MEM_TABLE,Processes), 1141},
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[7]), 1142},
{offsetof(STRINGS_MEM_TABLE,Handles), 1143},
{offsetof(STRINGS_MEM_TABLE,Total), 1144},
{offsetof(STRINGS_MEM_TABLE,Available), 1145},
{offsetof(STRINGS_MEM_TABLE,SystemCache), 1146},
{offsetof(STRINGS_MEM_TABLE,Current), 1147},
{offsetof(STRINGS_MEM_TABLE,Limit), 1151},
{offsetof(STRINGS_MEM_TABLE,Peak), 1149},
{offsetof(STRINGS_MEM_TABLE,PagedPhysical), 1152},
{offsetof(STRINGS_MEM_TABLE,PagedVirtual), 1153},
{offsetof(STRINGS_MEM_TABLE,Nonpaged), 1155},
{offsetof(STRINGS_MEM_TABLE,PageSize), 1156},
{offsetof(STRINGS_MEM_TABLE,AllocationGranularity), 1154},
{offsetof(STRINGS_MEM_TABLE,CurrentLimit), 1148},
{offsetof(STRINGS_MEM_TABLE,PeakLimit), 1150},
{offsetof(STRINGS_MEM_TABLE,CPUUsage), 1101},
{offsetof(STRINGS_MEM_TABLE,CommitHistory), 1107},
{offsetof(STRINGS_MEM_TABLE,Commit), 1105},
{offsetof(STRINGS_MEM_TABLE,Paging), 1162},
{offsetof(STRINGS_MEM_TABLE,PageFaultDelta), 1163},
{offsetof(STRINGS_MEM_TABLE,PagingFileWriteDelta), 1165},
{offsetof(STRINGS_MEM_TABLE,MappedFileWriteDelta), 1166},
{offsetof(STRINGS_MEM_TABLE,PageReadDelta), 1164},
{offsetof(STRINGS_MEM_TABLE,CPUandIO), 1175},
{offsetof(STRINGS_MEM_TABLE,ContextSwitchDelta), 1167},
{offsetof(STRINGS_MEM_TABLE,IOReadDelta), 1168},
{offsetof(STRINGS_MEM_TABLE,IOWriteDelta), 1169},
{offsetof(STRINGS_MEM_TABLE,IOOtherDelta), 1170},
{offsetof(STRINGS_MEM_TABLE,Totals), 1134},
{offsetof(STRINGS_MEM_TABLE,CommitChargeK), 1136},
{offsetof(STRINGS_MEM_TABLE,PhysicalMemoryK), 1135},
{offsetof(STRINGS_MEM_TABLE,KernelMemoryK), 1137},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d7[]={
{offsetof(STRINGS_MEM_TABLE,TypeNameOfProgram), 6},
{offsetof(STRINGS_MEM_TABLE,File_), 5},
{offsetof(STRINGS_MEM_TABLE,OK), 201},
{offsetof(STRINGS_MEM_TABLE,Cancel), 202},
{offsetof(STRINGS_MEM_TABLE,Browse_), 203},
{offsetof(STRINGS_MEM_TABLE,RunAs_), 204},
{offsetof(STRINGS_MEM_TABLE,Parameters_), 9},
{offsetof(STRINGS_MEM_TABLE,RunAsCurrentUser_), 103},
{offsetof(STRINGS_MEM_TABLE,RunAsAnotherUser_), 104},
{offsetof(STRINGS_MEM_TABLE,RunAsUser_), 10},
{offsetof(STRINGS_MEM_TABLE,RunAsMsgText), 102},
{offsetof(STRINGS_MEM_TABLE,UserName_), 105},
{offsetof(STRINGS_MEM_TABLE,Password_), 107},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d8[]={
{offsetof(STRINGS_MEM_TABLE,Cancel), 2},
{offsetof(STRINGS_MEM_TABLE,Lock), 15001},
{offsetof(STRINGS_MEM_TABLE,LogOff), 15002},
{offsetof(STRINGS_MEM_TABLE,StandBy), 15003},
{offsetof(STRINGS_MEM_TABLE,TurnOff), 15004},
{offsetof(STRINGS_MEM_TABLE,Restart), 15005},
{0,0xFFFFFFFF}
};
/*
DLGITEM_TEXT d9[]={offsetof(STRINGS_MEM_TABLE,
{offsetof(STRINGS_MEM_TABLE,"Select the columns that will appear on the Handle view of Process Scanner.), 811},
{offsetof(STRINGS_MEM_TABLE,"Handle Value), 20},
{offsetof(STRINGS_MEM_TABLE,"Type), 21},
{offsetof(STRINGS_MEM_TABLE,"Access Mask), 39},
{offsetof(STRINGS_MEM_TABLE,"Name), 22},
{offsetof(STRINGS_MEM_TABLE,0,0xFFFFFFFF}
};*/
                    
DLGITEM_TEXT d10[]={
{offsetof(STRINGS_MEM_TABLE,SelProcCol), 811},
{offsetof(STRINGS_MEM_TABLE,ProcessName), 0},
{offsetof(STRINGS_MEM_TABLE,ID_ProcessId), 1},
{offsetof(STRINGS_MEM_TABLE,PID_ParentID), 2},
{offsetof(STRINGS_MEM_TABLE,ImagePath), 3},
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[10]), 10},//CommandLine
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[16]), 16},//Description
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[17]), 17},//Version
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[18]), 18},//CompanyName
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[8]), 8},//StartTime
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[9]), 9},//CPUTime
{offsetof(STRINGS_MEM_TABLE,CPUUsage), 4},
{offsetof(STRINGS_MEM_TABLE,UserName), 5},
{offsetof(STRINGS_MEM_TABLE,MainWindowTitle),19},
{offsetof(STRINGS_MEM_TABLE,ColumnTitle[20]), 20},//Session
{offsetof(STRINGS_MEM_TABLE,ThreadCount), 7},
{offsetof(STRINGS_MEM_TABLE,HandleCount), 11},
{offsetof(STRINGS_MEM_TABLE,BasePriority), 6},
{offsetof(STRINGS_MEM_TABLE,PrivateBytes), 13},
{offsetof(STRINGS_MEM_TABLE,WorkingSetSize), 14},
{offsetof(STRINGS_MEM_TABLE,VirtualSize), 12},
{offsetof(STRINGS_MEM_TABLE,CommitCharge), 15},
{offsetof(STRINGS_MEM_TABLE,ContextSwitches), 21},
{offsetof(STRINGS_MEM_TABLE,ContextSwitchesDelta), 22},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d11[]={
{offsetof(STRINGS_MEM_TABLE,SelDllCol), 811},
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[7]), 7},
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[5]), 5},//ImageSize
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[2]), 2},//Description
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[3]), 3},//CompanyName
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[4]), 4},
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[0]), 0},
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[1]), 1},//Path
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[6]), 6},//Version
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d13[]={
{offsetof(STRINGS_MEM_TABLE,properties), 1005},//File properties
{offsetof(STRINGS_MEM_TABLE,User_), 3001},
{offsetof(STRINGS_MEM_TABLE,n_a), 1190},
{offsetof(STRINGS_MEM_TABLE,ImageFile), 1112},
{offsetof(STRINGS_MEM_TABLE,Path_), 3002},
{offsetof(STRINGS_MEM_TABLE,FileVersion_), 3003},
{offsetof(STRINGS_MEM_TABLE,n_a), 25},
{offsetof(STRINGS_MEM_TABLE,Parent_), 3004},
{offsetof(STRINGS_MEM_TABLE,n_a), 35},
{offsetof(STRINGS_MEM_TABLE,Commandline_), 3005},
{offsetof(STRINGS_MEM_TABLE,Currentdirectory_), 3006},
{offsetof(STRINGS_MEM_TABLE,TimeStamp_), 3007},
{offsetof(STRINGS_MEM_TABLE,n_a), 30},
{offsetof(STRINGS_MEM_TABLE,Desktop_), 3008},
{offsetof(STRINGS_MEM_TABLE,n_a), 1007},
{offsetof(STRINGS_MEM_TABLE,Started_), 3009},
{offsetof(STRINGS_MEM_TABLE,n_a), 19},
{offsetof(STRINGS_MEM_TABLE,Debugged_), 3010},
{offsetof(STRINGS_MEM_TABLE,n_a), 1161},
{offsetof(STRINGS_MEM_TABLE,KillProcess), 1004},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d14[]={
{offsetof(STRINGS_MEM_TABLE,CPU), 3001},
{offsetof(STRINGS_MEM_TABLE,Handles), 3002},
{offsetof(STRINGS_MEM_TABLE,IO), 3003},
{offsetof(STRINGS_MEM_TABLE,MemoryK), 3004},
{offsetof(STRINGS_MEM_TABLE,PrivateBytes_), 3005},
{offsetof(STRINGS_MEM_TABLE,WorkingSet_), 3006},
{offsetof(STRINGS_MEM_TABLE,n_a), 2008},
{offsetof(STRINGS_MEM_TABLE,n_a), 2009},
{offsetof(STRINGS_MEM_TABLE,Reads_), 3007},
{offsetof(STRINGS_MEM_TABLE,Writes_), 3008},
{offsetof(STRINGS_MEM_TABLE,Other_), 3009},
{offsetof(STRINGS_MEM_TABLE,n_a), 2041},
{offsetof(STRINGS_MEM_TABLE,n_a), 2043},
{offsetof(STRINGS_MEM_TABLE,n_a), 2045},
{offsetof(STRINGS_MEM_TABLE,GDIHandles_), 3010},
{offsetof(STRINGS_MEM_TABLE,USERHandles_), 3011},
{offsetof(STRINGS_MEM_TABLE,n_a), 2046},
{offsetof(STRINGS_MEM_TABLE,n_a), 2047},
{offsetof(STRINGS_MEM_TABLE,Handles_), 3012},
{offsetof(STRINGS_MEM_TABLE,n_a), 2048},
{offsetof(STRINGS_MEM_TABLE,PeakPrivateBytes_), 3013},
{offsetof(STRINGS_MEM_TABLE,n_a), 2010},
{offsetof(STRINGS_MEM_TABLE,PeakWorkingSet_), 3014},
{offsetof(STRINGS_MEM_TABLE,n_a), 2064},
{offsetof(STRINGS_MEM_TABLE,PageFaults_), 3015},
{offsetof(STRINGS_MEM_TABLE,PageFaultDelta_), 3016},
{offsetof(STRINGS_MEM_TABLE,n_a), 2065},
{offsetof(STRINGS_MEM_TABLE,n_a), 2066},
{offsetof(STRINGS_MEM_TABLE,ReadDelta_), 3017},
{offsetof(STRINGS_MEM_TABLE,n_a), 2044},
{offsetof(STRINGS_MEM_TABLE,WriteDelta_), 3018},
{offsetof(STRINGS_MEM_TABLE,OtherDelta_), 3019},
{offsetof(STRINGS_MEM_TABLE,n_a), 2049},
{offsetof(STRINGS_MEM_TABLE,n_a), 2050},
{offsetof(STRINGS_MEM_TABLE,KernelTime_), 3020},
{offsetof(STRINGS_MEM_TABLE,UserTime_), 3021},
{offsetof(STRINGS_MEM_TABLE,TotalTime_), 3022},
{offsetof(STRINGS_MEM_TABLE,n_a), 2012},
{offsetof(STRINGS_MEM_TABLE,n_a), 2013},
{offsetof(STRINGS_MEM_TABLE,n_a), 2014},
{offsetof(STRINGS_MEM_TABLE,Priority_), 3023},
{offsetof(STRINGS_MEM_TABLE,n_a), 2042},
{offsetof(STRINGS_MEM_TABLE,VirtualSize_), 3024},
{offsetof(STRINGS_MEM_TABLE,PeakVirtualSize_), 3025},
{offsetof(STRINGS_MEM_TABLE,n_a), 2006},
{offsetof(STRINGS_MEM_TABLE,n_a), 2007},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d15[]={
{offsetof(STRINGS_MEM_TABLE,CPUUsage), 1157},
{offsetof(STRINGS_MEM_TABLE,CPUUsageHistory), 1114},
{offsetof(STRINGS_MEM_TABLE,PrivateBytesHistory), 1115},
{offsetof(STRINGS_MEM_TABLE,PrivateBytes_ProcInfo), 1160},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d3[]={
{offsetof(STRINGS_MEM_TABLE,StartType_), 106},
{offsetof(STRINGS_MEM_TABLE,All), 102},
{offsetof(STRINGS_MEM_TABLE,Applications), 103},
{offsetof(STRINGS_MEM_TABLE,Drivers), 104},
{offsetof(STRINGS_MEM_TABLE,Interactive), 105},
//{offsetof(STRINGS_MEM_TABLE,"ComboStart), 107},
{offsetof(STRINGS_MEM_TABLE,Description), 108},
{offsetof(STRINGS_MEM_TABLE,Pathtofile_), 110},
{offsetof(STRINGS_MEM_TABLE,LoadOrderGroup_), 113},
{offsetof(STRINGS_MEM_TABLE,TaginGroup_), 114},
{offsetof(STRINGS_MEM_TABLE,Dependencies_), 116},
//{offsetof(STRINGS_MEM_TABLE,DependsList), 117},
{offsetof(STRINGS_MEM_TABLE,AcceptShutdown_), 118},
{offsetof(STRINGS_MEM_TABLE,Start), 121},
{offsetof(STRINGS_MEM_TABLE,Stop), 122},
{offsetof(STRINGS_MEM_TABLE,Pause), 123},
{offsetof(STRINGS_MEM_TABLE,ResumeSrv), 124},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d5[]={
{offsetof(STRINGS_MEM_TABLE,All), 102},
{offsetof(STRINGS_MEM_TABLE,Basic), 103},
{offsetof(STRINGS_MEM_TABLE,Others), 104},
{offsetof(STRINGS_MEM_TABLE,ItemParameters_), 110},
{offsetof(STRINGS_MEM_TABLE,ItemLocation_), 113},
{0,0xFFFFFFFF}
};


DLGITEM_TEXT d16[]={
{offsetof(STRINGS_MEM_TABLE,_NULL_), 201},
{offsetof(STRINGS_MEM_TABLE,_NULL_), 202},
{offsetof(STRINGS_MEM_TABLE,_NULL_), 203},
{offsetof(STRINGS_MEM_TABLE,_NULL_), 204},
{offsetof(STRINGS_MEM_TABLE,_NULL_), 205},
{offsetof(STRINGS_MEM_TABLE,_NULL_), 206},
{offsetof(STRINGS_MEM_TABLE,CLSIDnotfound), 500},
{offsetof(STRINGS_MEM_TABLE,JumpToRegistry), 401},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d17[]={
{offsetof(STRINGS_MEM_TABLE,Visibleonly), 103},
{offsetof(STRINGS_MEM_TABLE,Withcaptiononly), 104},
{offsetof(STRINGS_MEM_TABLE,Withnoparentonly), 105},
{offsetof(STRINGS_MEM_TABLE,EndTask), 201},
{offsetof(STRINGS_MEM_TABLE,NewTask_), 202},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d18[]={
{offsetof(STRINGS_MEM_TABLE,SelWinCols), 811},
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[2]), 2},//classname
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[0]), 0}, //Handlevalue
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[1]), 1},//Caption
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[3]), 3},//Rectangle
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[4]), 4},//Stylevalue
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[5]), 5},//ExtendedStylevalue
{offsetof(STRINGS_MEM_TABLE,WinColumnTitle[6]), 6},//Status
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d19[]={
{offsetof(STRINGS_MEM_TABLE,ChooseEndTaskType), 101},
{offsetof(STRINGS_MEM_TABLE,NormalEnd), 102},
{offsetof(STRINGS_MEM_TABLE,Forced), 103},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d20[]={
{offsetof(STRINGS_MEM_TABLE,SelDrvCols), 811},
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[5]), 5},//imgsize
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[2]), 2},//Description
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[3]), 3},//CompanyName
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[4]), 4},
{offsetof(STRINGS_MEM_TABLE,FileName), 0},
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[1]), 1},//Path
{offsetof(STRINGS_MEM_TABLE,DllColumnTitle[6]), 6},//Version
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d21[]={
{offsetof(STRINGS_MEM_TABLE,SelSrvCols), 811},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[2]), 2},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[3]), 3},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[5]), 5},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[0]), 0},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[1]), 1},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[4]), 4},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[6]), 6},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[7]), 7},
{offsetof(STRINGS_MEM_TABLE,SrvcColumnTitle[8]), 8},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d22[]={
{offsetof(STRINGS_MEM_TABLE,ServiceName_), 101},
{offsetof(STRINGS_MEM_TABLE,ServiceType_), 105},
{offsetof(STRINGS_MEM_TABLE,DisplayName_), 104},
{offsetof(STRINGS_MEM_TABLE,Pathtofile_), 108},
{offsetof(STRINGS_MEM_TABLE,Description_), 114},
{offsetof(STRINGS_MEM_TABLE,StartupType_), 109},
{offsetof(STRINGS_MEM_TABLE,ControlsAccepted_), 111},
{offsetof(STRINGS_MEM_TABLE,CurrentState_), 116},
{offsetof(STRINGS_MEM_TABLE,Start), 118},
{offsetof(STRINGS_MEM_TABLE,Stop), 119},
{offsetof(STRINGS_MEM_TABLE,Pause), 120},
{offsetof(STRINGS_MEM_TABLE,ResumeSrv), 121},
{offsetof(STRINGS_MEM_TABLE,Command_line_), 122},
{offsetof(STRINGS_MEM_TABLE,JumpToRegistry), 127},
{offsetof(STRINGS_MEM_TABLE,ErrorsControl_), 128},
{offsetof(STRINGS_MEM_TABLE,Process_), 132},
{offsetof(STRINGS_MEM_TABLE,n_a), 117},
{offsetof(STRINGS_MEM_TABLE,n_a), 107},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d23[]={
{offsetof(STRINGS_MEM_TABLE,Load_OrderGroup), 113},
{offsetof(STRINGS_MEM_TABLE,Name_), 110},
{offsetof(STRINGS_MEM_TABLE,TaginGroup_), 112},
{offsetof(STRINGS_MEM_TABLE,LogOn), 101},
{offsetof(STRINGS_MEM_TABLE,LocalSystemAccount), 102},
{offsetof(STRINGS_MEM_TABLE,ThisAccount_), 103},
{offsetof(STRINGS_MEM_TABLE,AlwSrvcInteract), 104},
{offsetof(STRINGS_MEM_TABLE,Password_), 106},
{offsetof(STRINGS_MEM_TABLE,View_Group), 114},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d24[]={
{offsetof(STRINGS_MEM_TABLE,ThisSrvcDependsOn), 101},
{offsetof(STRINGS_MEM_TABLE,TheFlwngSrvcsDependOnThisSrvc), 102},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d2[]={
{offsetof(STRINGS_MEM_TABLE,Description), 105},
{offsetof(STRINGS_MEM_TABLE,SrvcsRegInThisProcess), 101},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d25[]={
{offsetof(STRINGS_MEM_TABLE,QuickList), 102},
{offsetof(STRINGS_MEM_TABLE,FullList), 103},
{offsetof(STRINGS_MEM_TABLE,Total_), 104},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d27[]={
{offsetof(STRINGS_MEM_TABLE,MenuStyle), 403},
{offsetof(STRINGS_MEM_TABLE,MenuStyleDefault), 501},
{offsetof(STRINGS_MEM_TABLE,MenuStyleModern),502},
{offsetof(STRINGS_MEM_TABLE,OnStartOpenTheTab), 402},
{offsetof(STRINGS_MEM_TABLE,UpdateSpeed), 401},
{offsetof(STRINGS_MEM_TABLE,AlwaysOnTop),100},
{offsetof(STRINGS_MEM_TABLE,Allow1inst),101},
{offsetof(STRINGS_MEM_TABLE,ShowFractnlCPUusage),111},
{offsetof(STRINGS_MEM_TABLE,MinimizeOnSwitch),120},
{offsetof(STRINGS_MEM_TABLE,ReplaceTaskMan),121},
{offsetof(STRINGS_MEM_TABLE,HideWhenMin),122},
{offsetof(STRINGS_MEM_TABLE,HideOnESCinsteadClose),127},
{offsetof(STRINGS_MEM_TABLE,_05second),107},
{offsetof(STRINGS_MEM_TABLE,_1second),108},
{offsetof(STRINGS_MEM_TABLE,_2seconds),109},
{offsetof(STRINGS_MEM_TABLE,_Paused),110},
{offsetof(STRINGS_MEM_TABLE,ReverseSort),137},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d28[]={
{offsetof(STRINGS_MEM_TABLE,BottomPane),401},
{offsetof(STRINGS_MEM_TABLE,ConfirmKill),102},
{offsetof(STRINGS_MEM_TABLE,_DontShow0CPUusage),112},
{offsetof(STRINGS_MEM_TABLE,NumericPriority),113},
{offsetof(STRINGS_MEM_TABLE,Show16bitTasks),123},
{offsetof(STRINGS_MEM_TABLE,UserNameContainComputerName),138},
{offsetof(STRINGS_MEM_TABLE,ScrollTo_),124},
 {offsetof(STRINGS_MEM_TABLE,toTheEnd),131},
 {offsetof(STRINGS_MEM_TABLE,ToLastProcess),132},
 {offsetof(STRINGS_MEM_TABLE,setSelectionToProcess),133},
{offsetof(STRINGS_MEM_TABLE,ScrollToNewProcesses),139},
{offsetof(STRINGS_MEM_TABLE,ShowUnnamedHandlesAndMappings),140},
{offsetof(STRINGS_MEM_TABLE,ShowProcessesFromAllUsers),141},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d29[]={
{offsetof(STRINGS_MEM_TABLE,ShowKernelTimes),126},
{offsetof(STRINGS_MEM_TABLE,MemoryUsage),401},
{offsetof(STRINGS_MEM_TABLE,CommitHistory),114},
{offsetof(STRINGS_MEM_TABLE,PhysMemUsageHist),115},
{offsetof(STRINGS_MEM_TABLE,CPUusage_),402},
{offsetof(STRINGS_MEM_TABLE,ShowUserTimes),129},
{offsetof(STRINGS_MEM_TABLE,CollectKernelTimes),128},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d31[]={
{offsetof(STRINGS_MEM_TABLE,Connect),271},
{offsetof(STRINGS_MEM_TABLE,Refresh),274},
{offsetof(STRINGS_MEM_TABLE,Computer_),273},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d32[]={
{offsetof(STRINGS_MEM_TABLE,WindowStyles_),501},
{offsetof(STRINGS_MEM_TABLE,ExtendedStyles_),502},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d33[]={
{offsetof(STRINGS_MEM_TABLE,UpdateWinProps),119},
{offsetof(STRINGS_MEM_TABLE,DefaultAction),401},
{offsetof(STRINGS_MEM_TABLE,ThisActionWillActiveForDblClick),402},
{offsetof(STRINGS_MEM_TABLE,SwitchTo),135},
{offsetof(STRINGS_MEM_TABLE,WindowProperties),136},
{offsetof(STRINGS_MEM_TABLE,_WindowProperties_),403},
{offsetof(STRINGS_MEM_TABLE,ShowStylesWithZeroValue),134},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d34[]={
{offsetof(STRINGS_MEM_TABLE,Transparency),101},
{offsetof(STRINGS_MEM_TABLE,OK),1},
{offsetof(STRINGS_MEM_TABLE,Cancel),2},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d35[]={
{offsetof(STRINGS_MEM_TABLE,ReplaceTaskMan),121},
{offsetof(STRINGS_MEM_TABLE,NormalRun),503},
{offsetof(STRINGS_MEM_TABLE,RunWithLogonDlg),504},
{offsetof(STRINGS_MEM_TABLE,RunWithLogonDlgIfNonAdmin),505},
{offsetof(STRINGS_MEM_TABLE,TaskManagerReplaceInfoText),700},
{offsetof(STRINGS_MEM_TABLE,TaskManagerCurrentReplacer_),701},
{0,0xFFFFFFFF}
};

DLGITEM_TEXT d50[]={
{offsetof(STRINGS_MEM_TABLE,LogonCurUser),202},
{offsetof(STRINGS_MEM_TABLE,LogonAnotherUser),201},
{offsetof(STRINGS_MEM_TABLE,LogonMsgText),102},
{0,0xFFFFFFFF}
};