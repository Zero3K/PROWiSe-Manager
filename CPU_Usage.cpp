
bool getCPUusage(){BYTE bquit; DWORD nn,ErrCode; double cpu_idletime,cpu_usage;
 bquit=0; nn=0;
 if(!basicInfo){
  if((ErrCode=ntdllFunctions.pNtQuerySystemInformation(SystemBasicInformation,SysBasicInf,44,&nn))==0){
   basicInfo=1;
   if(SysBasicInf->NumberProcessors==1)bTurnOffKernelCalc=0;
   else if(SysBasicInf->NumberProcessors==0)bquit++;
  }
  else{
   if(basicInfoErr)return 0;
   FailNtFunction("NtQSI_SBI",ErrCode,nn); basicInfoErr=1; bquit++;
  }
 }
 if((ErrCode=ntdllFunctions.pNtQuerySystemInformation(SystemTimeOfDayInformation,sysTimeInf,28,&nn))!=0){//get new system time
  bquit++;
  FailNtFunction("NtQSI_STODI",ErrCode,nn);
 }
 if((ErrCode=ntdllFunctions.pNtQuerySystemInformation(SystemPerformanceInformation,sysPerfInf,400,&nn))!=0){
  bquit++;
  FailNtFunction("NtQSI_SPI",ErrCode,nn);
 }
 if((ErrCode=ntdllFunctions.pNtQuerySystemInformation(SystemProcessorTimes,sysProcessorTm,48*4,&nn))!=0){//get new CPU's time
  bquit++;
  FailNtFunction("NtQSI_SPT",ErrCode,nn);
 }
 if(bquit)return 0;
 newIdleTm=sysPerfInf->IdleTime.QuadPart;
 newCurTm=sysTimeInf->CurrentTime.QuadPart;
 if(oldIdleTm!=0){
  dbSystemTime=newCurTm; dbSystemTime-=oldCurTm;
  cpu_idletime=newIdleTm; cpu_idletime-=oldIdleTm;
  cpu_idletime/=SysBasicInf->NumberProcessors;
  if(dbSystemTime){
   cpu_idletime/=dbSystemTime;// CurrentCpuIdle = IdleTime / SystemTime
   cpu_usage=1.0-cpu_idletime; cpu_usage*=100.0;//CurrentCpuUsage,% = 100 - (CurrentSystemIdle * 100)
   cpuIdleTime=cpu_idletime;
   cpuUsage=cpu_usage;
  }
 }
 oldIdleTm=newIdleTm; oldCurTm=newCurTm;
 return 1;
}

void UpdateCPUusage(){char lta[10];
 if(getCPUusage()==0)return;
 if(cpuusg_tout==0){
  cpuusglen=(WORD)getstrlen(smemTable->CPUusage_);
  cpuusg_tout=(char*)LocalAlloc(LMEM_FIXED,cpuusglen+10);
  copystring(cpuusg_tout,smemTable->CPUusage_);
 }
 if(progSetts.CheckState[11]){cpuToFractStr(cpuUsage,cpuusg_tout+cpuusglen,lta);}
 else{
  if((cpuUsage-(DWORD)cpuUsage)>=0.5)cpuUsage+=0.5;
  ltoa((DWORD)cpuUsage,cpuusg_tout+cpuusglen,10);
 }
 strappend(cpuusg_tout," %");
 if(pHdr!=0){
  if(pHdr->StatusCPUUsgType!=progSetts.CheckState[11]){
   pHdr->StatusCPUUsgType=progSetts.CheckState[11];
   DWORD nn;
   if((nn=CalcStringAvgWidth(hstatus,cpuusg_tout))!=0){
    nn-=5;
    pHdr->StatusBarParts[STATUSBAR_CPUUSAGE_ID]=nn;
    SetStatusBarParts();
   }
  }
  SendMessage(hstatus,SB_SETTEXT,(WPARAM)1,(LPARAM)cpuusg_tout);
 }
 ModifyTrayIcon(cpuusg_tout);
 //// calc Kernel Time
 if(progSetts.CheckState[26] && !bTurnOffKernelCalc){
  double newKernelTm,dkernel; LARGE_INTEGER lint; int i;
  SYSTEM_PROCESSOR_TIMES *pspt; pspt=sysProcessorTm;
  lint.QuadPart=sysPerfInf->IdleTime.QuadPart;
  lint.QuadPart/=SysBasicInf->NumberProcessors;
  for(i=0;i<SysBasicInf->NumberProcessors;i++){
   lint.QuadPart+=pspt->UserTime.QuadPart;
   pspt++;
  }
  lint.QuadPart=sysTimeInf->CurrentTime.QuadPart-lint.QuadPart;
  newKernelTm=(double)lint.QuadPart;
  if(oldKernelTm!=0){
   dkernel=newKernelTm; dkernel-=oldKernelTm;
   cpuKernelTime=dkernel/dbSystemTime;
   *gdCpuKrnl.histCurPos=(float)(1.0-cpuKernelTime);
   cpuKernelTime*=100.0;
  }
  oldKernelTm=newKernelTm;
 }
}