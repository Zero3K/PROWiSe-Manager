//#include "SrvcGroups.cpp"
//#include "SrvcProperties.cpp"
void AdditionalServiceInfo(SC_HANDLE hsrv,SERVICES_WIN_PARAMS *svwp);

void SrvcContextMenu(HWND hDlg,POINT *curp){HMENU pmenu; POINT curpos;
 if(curp==NULL)GetCursorPos(&curpos);
 else {curpos.x=curp->x; curpos.y=curp->y;}
 if((pmenu=GetSubMenu(main_menu,2))==0)return;
 SetMenuDefaultItem(pmenu,40017,0);
 TrackPopupMenuEx(pmenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos.x,curpos.y,hDlg,NULL);
 SetMenuDefaultItem(pmenu,-1,0);
}

//////////////////////////////// Remove_Service ////////////////////////////////
void Remove_Service(SERVICE_INFOBUF* srvib, HWND hDlg){
 SC_HANDLE hsrv; SERVICES_WIN_PARAMS *svwp; SERVICE_STATUS svst; bool svstopped;
 if(!IsVALID_SERVICE_INFO(srvib) || (srvib->String[0])==0 || *(srvib->String[0])==0)return;
 if(MessageBox(main_win,smemTable->AreYouSureDelSrvc,progTitle,MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)!=IDYES)return;
 /* //test all messages
 FailMessage(smemTable->UnblGetSrvStatus,FMSG_CallGLE,FMSG_SHOW_MSGBOX);
 MessageBox(main_win,smemTable->SrvMrkForDelete_Continue_,progTitle,MB_YESNO|MB_ICONQUESTION);
 FailMessage(smemTable->UnblDeleteSrvc,FMSG_CallGLE|FMSG_ICONSTOP,FMSG_SHOW_MSGBOX);
 MessageBox(main_win,smemTable->SrvDeletedSuccesfl,progTitle,MB_OK|MB_ICONINFORMATION);
 MessageBox(main_win,smemTable->SrvStopped_DeleteFile_,progTitle,MB_YESNO|MB_ICONQUESTION);
 MessageBox(main_win,smemTable->SrvNotStopped,progTitle,MB_OK|MB_ICONINFORMATION);
 MessageBox(main_win,smemTable->PathIsNotValid,progTitle,MB_OK|MB_ICONSTOP);
 FailMessage(smemTable->UnblDelFile,FMSG_CallGLE|FMSG_ICONSTOP,FMSG_SHOW_MSGBOX);
 MessageBox(main_win,smemTable->FileSucsflDeleted,progTitle,MB_OK|MB_ICONINFORMATION);
 return;*/
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))return;
 if((hsrv=OpenService(svwp->scman,srvib->String[1],SERVICE_INTERROGATE|SERVICE_QUERY_STATUS|SERVICE_STOP|DELETE))==NULL){
  FailMessage(openservice,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
  return;
 }
 svstopped=0;
 if(ControlService(hsrv,SERVICE_CONTROL_INTERROGATE,&svst)==0){
  if(QueryServiceStatus(hsrv,&svst)==0){FailMessage(smemTable->UnblGetSrvStatus,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX); goto DeleteSrv;}
 }
 if((svst.dwCurrentState&SERVICE_STOPPED)==0){
  if((svst.dwCurrentState&SERVICE_STOP_PENDING)==0){
   ControlService(hsrv,SERVICE_CONTROL_STOP,&svst);
  }
 }
DeleteSrv:
 if(!DeleteService(hsrv)){
  DWORD err; err=ntdllFunctions.RtlGetLastWin32Error();
  if(err==ERROR_SERVICE_MARKED_FOR_DELETE){
   if(MessageBox(main_win,smemTable->SrvMrkForDelete_Continue_,progTitle,MB_YESNO|MB_ICONQUESTION)!=IDYES)goto close_hsrv;
  }
  else {FailMessage(smemTable->UnblDeleteSrvc,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX); goto close_hsrv;}
 }
 MessageBox(main_win,smemTable->SrvDeletedSuccesfl,progTitle,MB_OK|MB_ICONINFORMATION);
 if(QueryServiceStatus(hsrv,&svst) && (svst.dwCurrentState&SERVICE_STOPPED))svstopped=1;// check is service stopped
 char *pText; DWORD dw;
 if(svwp->IsSeparateWin==0){//надо еще проверить а не Local ли открыт !!!
  if(svstopped==1){pText=smemTable->SrvStopped_DeleteFile_; dw=MB_YESNO|MB_ICONQUESTION;}
  else {pText=smemTable->SrvNotStopped; dw=MB_OK|MB_ICONINFORMATION;}
 }
 //else {pText=smemTable->CantDeleteRemoteFile; dw=MB_OK|MB_ICONINFORMATION;}
 if(MessageBox(main_win,pText,progTitle,dw)!=IDYES)goto close_hsrv;
 if(svwp->IsSeparateWin)goto close_hsrv;/////// !!!
 //////// Delete File
 if(srvib->String[0]==0 || *(srvib->String[0])==0)goto msg_path_notvalid;
 pText=srvib->String[0];
 if(*(WORD*)pText==0x203F)pText+=2;
 pText=CorrectFilePath(pText,0,t0nbuf2);
 if(DeleteFile(pText)){
  MessageBox(main_win,smemTable->FileSucsflDeleted,progTitle,MB_OK|MB_ICONINFORMATION);
 }
 else FailMessage(smemTable->UnblDelFile,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
 goto close_hsrv;
msg_path_notvalid:
 MessageBox(main_win,smemTable->PathIsNotValid,progTitle,MB_OK|MB_ICONSTOP);
close_hsrv:
 CloseServiceHandle(hsrv);
}

///////////////////////////// SrvListview CustomDraw ///////////////////////////
LRESULT SrvListviewCustomDraw(LPARAM lParam){LPNMLVCUSTOMDRAW lplvcd=(LPNMLVCUSTOMDRAW)lParam;
 switch(lplvcd->nmcd.dwDrawStage){
  case CDDS_PREPAINT: return CDRF_NOTIFYITEMDRAW;
  case CDDS_ITEMPREPAINT:
   if(!IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)lplvcd->nmcd.lItemlParam)))break;
   if(((SERVICE_INFOBUF*)lplvcd->nmcd.lItemlParam)->dwFlags&SRVC_FILE_NOT_FOUND){
    lplvcd->clrText=0; lplvcd->clrTextBk=0x00e0e0e0;
   }
   break;
 }
 return CDRF_DODEFAULT;
}

////////////////////////////// SrvcList CompareFunc ////////////////////////////
int CALLBACK SrvcListCompareFunc(LPARAM lParam1,LPARAM lParam2,LIST_SORT_DIRECTION *lParamSort){
 SERVICE_INFOBUF *sif1,*sif2; int ret; char *ptr1,*ptr2;
 ptr1=ptr2=0;
 sif1=(SERVICE_INFOBUF*)lParam1; sif2=(SERVICE_INFOBUF*)lParam2;
 if(!IsVALID_SERVICE_INFO(sif1) || !IsVALID_SERVICE_INFO(sif2))return 0;
 if(lParamSort->SortDirection==2){
  SERVICE_INFOBUF *psvinf; SERVICES_WIN_PARAMS *psvwp; BYTE n;
  psvwp=(SERVICES_WIN_PARAMS*)(sif1->pMainStruct);
  if(!IsVALID_SERVICES_WIN_PARAMS(psvwp))return 0;
  psvinf=psvwp->pfsinf; n=0;
  while(IsVALID_SERVICE_INFO(psvinf)){
   if((DWORD_PTR)psvinf==(DWORD_PTR)lParam1){n=1;break;}
   else if((DWORD_PTR)psvinf==(DWORD_PTR)lParam2){n=2;break;}
   psvinf=psvinf->NextEntry;
  }
  if(n==1)ret=-1; else if(n==2)ret=1;
  return ret;
 }
 if(lParamSort->iSub==Srvclnumber[0]){
cmp_name:
  ptr1=(char*)(sif1->String[1]);
  ptr2=(char*)(sif2->String[1]);
 }
 else if(lParamSort->iSub==Srvclnumber[1]){ptr1=(char*)(sif1->String[2]);ptr2=(char*)(sif2->String[2]);}
 else if(lParamSort->iSub==Srvclnumber[2]){
  getServiceStatus(sif1->dwStatus,&ptr1);
  getServiceStatus(sif2->dwStatus,&ptr2);
 }
 else if(lParamSort->iSub==Srvclnumber[3]){ret=sif1->BootType; ret-=sif2->BootType; goto exit;}
 else if(lParamSort->iSub==Srvclnumber[4]){ptr1=(char*)(sif1->String[3]);ptr2=(char*)(sif2->String[3]);}
 else if(lParamSort->iSub==Srvclnumber[5]){ptr1=(char*)(sif1->String[4]);ptr2=(char*)(sif2->String[4]);}
 else if(lParamSort->iSub==Srvclnumber[6]){
  ret=CompareFileTime(&sif1->ftCreationTime,&sif2->ftCreationTime);
  if(ret==0)goto cmp_name;
  goto exit;
 }
 else if(lParamSort->iSub==Srvclnumber[7]){
  ret=CompareFileTime(&sif1->ftLastWriteTime,&sif2->ftLastWriteTime);
  if(ret==0)goto cmp_name;
  goto exit;
 }
 else if(lParamSort->iSub==Srvclnumber[8]){
  ret=CompareFileTime(&sif1->ftRegLastWriteTime,&sif2->ftRegLastWriteTime);
  if(ret==0)goto cmp_name;
  goto exit;
 }
 if(ptr1 && ptr2)ret=lstrcmpi(ptr1,ptr2);
 else if(ptr1==0){if(ptr2)ret=-1; else ret=0;}
 else ret=1;
exit:
 if(lParamSort->SortDirection==1){if(ret<0)ret=1; else if(ret>0)ret=-1;}
 return ret;
}

////////////////////////////// Update Service Info /////////////////////////////
BYTE UpdSrvFails=0;

void UpdateSrvInfo(SC_HANDLE hsrv, SERVICE_INFOBUF* svif, SERVICES_WIN_PARAMS *svwp){SERVICE_STATUS *svst; DWORD n;
 if(hsrv==NULL)return;
 svst=(SERVICE_STATUS*)(svwp->nbuf3);
 if(QueryServiceStatus(hsrv,svst)==0){
  if(UpdSrvFails<5){
   UpdSrvFails++;
   FailMessage(QSSfailedOnUpdate_txt,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
  }
  return;
 }
 svif->CntrlAccept=svst->dwControlsAccepted;
 svif->dwStatus=svst->dwCurrentState;
 if(QueryServiceConfig(hsrv,(QUERY_SERVICE_CONFIG*)(svwp->nbuf3),1024,&n)==0){
  FailMessage(queryserviceconfig,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX);
  return;
 }
 svif->BootType=((QUERY_SERVICE_CONFIG*)(svwp->nbuf3))->dwStartType;
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))return;
 CompareAndReAllocateHeapA(svwp->pHeap,&svif->String[3],((QUERY_SERVICE_CONFIG*)(svwp->nbuf3))->lpServiceStartName);
 SendMessage(svwp->hServList,LVM_UPDATE,itemSel,0);
 AdditionalServiceInfo(hsrv,svwp);
 UpdSrvFails=0;
}

void ClearAdditionalInfo(HWND hDlg){HWND hwn;
 EnableWindow(GetDlgItem(hDlg,122),0);
 EnableWindow(GetDlgItem(hDlg,123),0);
 EnableWindow(GetDlgItem(hDlg,124),0);
 EnableWindow(GetDlgItem(hDlg,121),0);
 SendMessage(GetDlgItem(hDlg,119),WM_SETTEXT,0,(LPARAM)xxx_txt);
 SendMessage(GetDlgItem(hDlg,109),WM_SETTEXT,0,(LPARAM)0);
 SendMessage(GetDlgItem(hDlg,115),WM_SETTEXT,0,(LPARAM)xxx_txt);
 SendMessage(GetDlgItem(hDlg,111),WM_SETTEXT,0,0);
 SendMessage(GetDlgItem(hDlg,112),WM_SETTEXT,0,0);
 hwn=GetDlgItem(hDlg,107); SendMessage(hwn,CB_SETCURSEL,(WPARAM)-1,0); EnableWindow(hwn,0);
 SendMessage(GetDlgItem(hDlg,117),LB_RESETCONTENT,0,0);
 EnableMainMenuItem(47201,MF_BYCOMMAND|MF_GRAYED);
}

/////////////////////////////// ShowServicesInfo ///////////////////////////////
#include "Services_Info.cpp"
void ShowServicesInfoWoClear(HWND hDlg){DWORD svTypeShow; SERVICES_WIN_PARAMS *svwp;
 if(IsDlgButtonChecked(hDlg,102)==BST_CHECKED)svTypeShow=SERVICE_TYPE_ALL;
 else if(IsDlgButtonChecked(hDlg,103)==BST_CHECKED)svTypeShow=SERVICE_WIN32;
 else if(IsDlgButtonChecked(hDlg,104)==BST_CHECKED)svTypeShow=SERVICE_DRIVER;
 else if(IsDlgButtonChecked(hDlg,105)==BST_CHECKED)svTypeShow=SERVICE_INTERACTIVE_PROCESS;
 else svTypeShow=SERVICE_TYPE_ALL;
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))return;
 if(svwp->scman!=NULL)SendMessage(svwp->hstatus,SB_SETTEXT,(WPARAM)((svwp->IsSeparateWin)?2:STATUSBAR_PANEINFO_ID),(LPARAM)smemTable->Wait_);
list_again:
 Services_GetInfoAll(svTypeShow,svwp);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp) || svwp->bDestroy)return;
 UpdateServicesView(hDlg,svwp);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp) || svwp->bDestroy)return;
 if(svwp->bFirstTimeSrvTab){
  svwp->bFirstTimeSrvTab=0;
  svwp->SrvUpd[0]=svwp->SrvUpd[1]=svwp->SrvUpd[2]=svwp->SrvUpd[3]=svwp->SrvUpd[4]=0;
  svwp->SrvUpd[5]=svwp->SrvUpd[6]=svwp->SrvUpd[7]=svwp->SrvUpd[8]=1; 
  goto list_again;
 }
}

void ShowServicesInfo(HWND hDlg){SERVICES_WIN_PARAMS *svwp; SERVICE_INFOBUF *svif,*svif2; LVITEM lvi;
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))return;
 if((lvi.iItem=SendMessage(svwp->hServList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))!=-1){
  lvi.mask=LVIF_PARAM;
  if(SendMessage(svwp->hServList,LVM_GETITEM,0,(LPARAM)&lvi)!=0 && IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)lvi.lParam))){
   svwp->szLastSelSrvcName=LocalAllocAndCopy(((SERVICE_INFOBUF*)lvi.lParam)->String[1]);
  }
  else svwp->szLastSelSrvcName=0;
 }
 else svwp->szLastSelSrvcName=0;
 ClearAdditionalInfo(hDlg);
 svif=(SERVICE_INFOBUF*)(svwp->pfsinf);
 while(IsVALID_SERVICE_INFO(svif)){
  int i;
  for(i=0;i<(1+4);i++){
   if(svif->String[i])HeapFree(svwp->pHeap,0,svif->String[i]);
  }
  svif->cbSize=0;
  if(svwp->pfsinf==svif)svwp->pfsinf=0;
  // next
  svif2=svif;
  svif=svif->NextEntry;
  HeapFree(svwp->pHeap,0,svif2);
 }
 //
 SendMessage(svwp->hServList,LVM_DELETEALLITEMS,0,0);
 //
 ShowServicesInfoWoClear(hDlg);
 LocalFree(svwp->szLastSelSrvcName);
 svwp->szLastSelSrvcName=0;
}

/////////////////////////// AdditionalServiceInfo //////////////////////////////
void AdditionalServiceInfo(SC_HANDLE hsrv, SERVICES_WIN_PARAMS *svwp){HWND ServList,hDlg,htmp; DWORD ErrCode,n,dw; void *vmem; char *tb;
 LV_ITEM dlvi; SERVICE_STATUS svst; BOOL ret,ret2,bCloseHsrv,bUpdateItem; QUERY_SERVICE_CONFIG* qsc; SERVICE_INFOBUF* svif;
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))return;
 bUpdateItem=0;
 dlvi.iSubItem=0; dlvi.iItem=itemSel; dlvi.mask=LVIF_PARAM;
 ServList=svwp->hServList; hDlg=GetParent(ServList);
 if(SendMessage(ServList,LVM_GETITEM,0,(LPARAM)&dlvi)==0 || !IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)dlvi.lParam))){
  ClearAdditionalInfo(hDlg);
  return;
 }
 svif=(SERVICE_INFOBUF*)dlvi.lParam;
 if(hsrv==0){
  if((hsrv=OpenService(svwp->scman,svif->String[1],SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS))==0){
   ErrCode=ntdllFunctions.RtlGetLastWin32Error();
   ClearAdditionalInfo(hDlg);
   if(ErrCode==ERROR_SERVICE_DOES_NOT_EXIST){//1060L
    if(svif->String[0])SendMessage(GetDlgItem(hDlg,111),WM_SETTEXT,0,(LPARAM)(svif->String[0]));
    SendMessage(GetDlgItem(hDlg,112),WM_SETTEXT,0,(LPARAM)smemTable->SvcNotInstalled);
   }
   else FailMessage(openservice,ErrCode,FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);
   return;
  }
  bCloseHsrv=1;
 }
 else bCloseHsrv=0;
 if(QueryServiceStatus(hsrv,&svst)){
  svif->CntrlAccept=svst.dwControlsAccepted;
  if(svif->dwStatus!=svst.dwCurrentState){svif->dwStatus=svst.dwCurrentState; bUpdateItem=1;}
 }
 qsc=(QUERY_SERVICE_CONFIG*)(svwp->nbuf3);
 ret=QueryServiceConfig(hsrv,qsc,1024,&n);
 ret2=QueryServiceConfig2(hsrv,SERVICE_CONFIG_DESCRIPTION,(BYTE*)t1nbuf4,2048,&n);
 if(bCloseHsrv)CloseServiceHandle(hsrv);
 if(ret==0)FailMessage(queryserviceconfig,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX);
 if(svif->String[0])qsc->lpBinaryPathName=(char*)svif->String[0];
 SendMessage(GetDlgItem(hDlg,111),WM_SETTEXT,0,(LPARAM)qsc->lpBinaryPathName);
 if(ret==0)return;
 SendMessage(GetDlgItem(hDlg,112),WM_SETTEXT,0,(LPARAM)qsc->lpLoadOrderGroup);
 // set Group Tag Id
 if(*(qsc->lpLoadOrderGroup)!=0){
  *(WORD*)(svwp->tbuf)=0x7830;// x0
  HexToString(qsc->dwTagId,svwp->tbuf+2);
 }
 else *(svwp->tbuf)=0;
 SendMessage(GetDlgItem(hDlg,115),WM_SETTEXT,0,(LPARAM)(svwp->tbuf));
 // set Description
 vmem=0;
 if((!ret2 || *t1nbuf4==0) && svwp->szComputerName==0){
  if(svif->ValidPathLen && svif->String[0] && (svif->dwFlags&SRVC_FILE_NOT_FOUND)==0){
   char *ppath; ppath=svif->String[0];
   if(*(WORD*)ppath==0x203F)ppath+=2;
   lstrcpy((char*)t1nbuf4,ppath);
   ppath=CorrectFilePath((char*)t1nbuf4,0,t0nbuf2);
   *(ppath+svif->ValidPathLen)=0;
   DWORD wfdsc; wfdsc=1;
   vmem=FileModule_GetVerDescInfo(ppath,0,&wfdsc,0,0);
   if(wfdsc)*(LPTSTR*)t1nbuf4=(LPTSTR)wfdsc;
   else *t1nbuf4=0;
  }
 }
 else if(!ret2){
  *(LPTSTR*)t1nbuf4=smemTable->n_a;
 }
 SendMessage(GetDlgItem(hDlg,109),WM_SETTEXT,0,(LPARAM)(*(LPTSTR*)t1nbuf4));
 if(vmem)LocalFree(vmem);
 // set Startup Type
 setCmbBoxSvcStartType(svwp->ComboStrtType,qsc->dwStartType);
 if(qsc->dwStartType!=svif->BootType){svif->BootType=qsc->dwStartType; bUpdateItem=1;}
 // set Dependencies
 htmp=GetDlgItem(hDlg,117);
 SendMessage(htmp,LB_RESETCONTENT,0,0);
 tb=qsc->lpDependencies;
 if(tb==NULL || *tb==0){SendMessage(htmp,LB_ADDSTRING,0,(LPARAM)smemTable->NoDependencies); goto skipDepends;}
 do{
  n=lstrlen(tb);
  SendMessage(htmp,LB_ADDSTRING,0,(LPARAM)tb); n++; tb+=n;
 }
 while(*tb!=0);
skipDepends:
 dw=svif->CntrlAccept; n=svif->dwStatus;
 if(dw&SERVICE_ACCEPT_STOP && n!=SERVICE_STOPPED)ret=1; else ret=0;
  EnableWindow(GetDlgItem(hDlg,122),ret);//stop
 if(dw&SERVICE_ACCEPT_PAUSE_CONTINUE && n!=SERVICE_PAUSED)ret=1; else ret=0;
  EnableWindow(GetDlgItem(hDlg,123),ret);//pause
 if(dw&SERVICE_ACCEPT_PAUSE_CONTINUE && n==SERVICE_PAUSED)ret=1; else ret=0;
  EnableWindow(GetDlgItem(hDlg,124),ret);//continue
 if(dw&SERVICE_ACCEPT_SHUTDOWN)ret=1; else ret=0;
  SendMessage(GetDlgItem(hDlg,119),WM_SETTEXT,0,(LPARAM)((!ret)?smemTable->No:smemTable->Yes));
 if(n==SERVICE_STOPPED)ret=1; else ret=0;
  EnableWindow(GetDlgItem(hDlg,121),ret);//start
 if(bUpdateItem)SendMessage(ServList,LVM_UPDATE,dlvi.iItem,0);
 EnableMainMenuItem(47201,MF_BYCOMMAND|MF_ENABLED);
 return;
}

//////////////////////////// Reconfig Service //////////////////////////////////
void ReconfigService(HWND hDlg){SC_HANDLE schman=0,hsrv=0; SC_LOCK scl=0; LV_ITEM dlvi; int in; DWORD n; LPTSTR tbuf1;
 if((tbuf1=(LPTSTR)LocalAlloc(LMEM_FIXED,300))==0)return;
 //Вы уверены, что хотите измененить настройки данной службы?
 lstrcpy(tbuf1,smemTable->AreYouSureYouWanna); lstrcat(tbuf1,smemTable->ChngStUpTypeSvc);
 if(MessageBox(hDlg,tbuf1,progTitle,MB_YESNO|MB_ICONQUESTION)!=IDYES)goto exit;
 SERVICES_WIN_PARAMS *svwp;
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))goto exit;
 dlvi.iSubItem=0; dlvi.iItem=itemSel; dlvi.mask=LVIF_PARAM;
 if(SendMessage(svwp->hServList,LVM_GETITEM,0,(LPARAM)&dlvi)==0 || !IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)dlvi.lParam)))goto exit;
 if((schman=OpenSCManager(svwp->szComputerName,NULL,SC_MANAGER_LOCK))==0){
  FailMessage(openscmanager,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX);
  goto exit;
 }
 scl=LockServiceDatabase(schman);
 if((hsrv=OpenService(schman,((SERVICE_INFOBUF*)dlvi.lParam)->String[1],SERVICE_CHANGE_CONFIG|SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS))==0){
  FailMessage(openservice,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
  goto exit;
 }
 QUERY_SERVICE_CONFIG *qsc; qsc=(QUERY_SERVICE_CONFIG*)(svwp->nbuf3);
 if(QueryServiceConfig(hsrv,qsc,1024,&n)==0){
  FailMessage(queryserviceconfig,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX);
  goto exit;
 }
 in=SendMessage(svwp->ComboStrtType,CB_GETCURSEL,0,0);
 if(in!=CB_ERR)n=SendMessage(svwp->ComboStrtType,CB_GETITEMDATA,in,0);
 else n=SERVICE_NO_CHANGE;
 if(ChangeServiceConfig(hsrv,SERVICE_NO_CHANGE,n,SERVICE_NO_CHANGE,NULL,NULL,NULL,NULL,NULL,NULL,qsc->lpDisplayName)==0){
  FailMessage(changeserviceconfig,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX|FMSG_WRITE_LOG);
  goto exit;
 }
 UpdateSrvInfo(hsrv,(SERVICE_INFOBUF*)(dlvi.lParam),svwp);
 CloseServiceHandle(hsrv); hsrv=0;
exit:
 if(hsrv!=NULL)CloseServiceHandle(hsrv);
 if(scl!=NULL)UnlockServiceDatabase(scl);
 if(schman!=NULL)CloseServiceHandle(schman);
 if(tbuf1)LocalFree(tbuf1);
}

///////////////////////////// Control Selected Service /////////////////////////
void ControlSelService(WORD btn, HWND hDlg){
 LV_ITEM dlvi; SC_HANDLE hsrv; DWORD cntrl; SERVICE_STATUS *svs; bool retvl; retvl=0;
 char *tbuf1; tbuf1=(char*)LocalAlloc(LMEM_FIXED,300); if(tbuf1==0)return;
 lstrcpy(tbuf1,smemTable->AreYouSureYouWanna); lstrcat(tbuf1,smemTable->ChngStatusSvc);
 if(MessageBox(hDlg,tbuf1,progTitle,MB_YESNO|MB_ICONQUESTION)!=IDYES)goto free_mem;
 SERVICES_WIN_PARAMS *svwp;
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))goto free_mem;
 dlvi.iSubItem=0; dlvi.iItem=itemSel; dlvi.mask=LVIF_PARAM;
 if(SendMessage(svwp->hServList,LVM_GETITEM,0,(LPARAM)&dlvi)==0 || !IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)dlvi.lParam)))goto free_mem;
 hsrv=OpenService(svwp->scman,((SERVICE_INFOBUF*)dlvi.lParam)->String[1],SERVICE_PAUSE_CONTINUE|SERVICE_START|SERVICE_STOP|SERVICE_QUERY_CONFIG|SERVICE_QUERY_STATUS);
 if(hsrv==NULL){FailMessage(openservice,0,FMSG_CallGLE|FMSG_ICONSTOP|FMSG_SHOW_MSGBOX);goto free_mem;}
 svs=(SERVICE_STATUS*)tbuf1;
 switch(btn){
  case 121: retvl=StartService(hsrv,0,0); goto exit;
  case 122: cntrl=SERVICE_CONTROL_STOP; break;
  case 123: cntrl=SERVICE_CONTROL_PAUSE; break;
  case 124: cntrl=SERVICE_CONTROL_CONTINUE; break;
  default: goto exit;
 }
 retvl=ControlService(hsrv,cntrl,svs);
exit:
 if(!retvl)FailMessage(smemTable->UnblStartnControlSvc,0,FMSG_CallGLE|FMSG_SHOW_MSGBOX);
 UpdateSrvInfo(hsrv,(SERVICE_INFOBUF*)dlvi.lParam,svwp);
 CloseServiceHandle(hsrv);
free_mem:
 if(tbuf1)LocalFree(tbuf1);
}

//////////////////////////////// Resize Window /////////////////////////////////
void resizeServicesWin(LONG newsz,LONG newwd,HWND hDlg){
 HWND htmp; POINT pt; HDWP hdwp,hdwpcur; int hplus,wplus,wd,hg,i; SERVICES_WIN_PARAMS *svwp;
 if((svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWL_USERDATA))==0 || svwp->cbsize!=sizeof(SERVICES_WIN_PARAMS))return;
 hplus=newsz; hplus-=svwp->initHg;
 wplus=newwd; wplus-=svwp->initWd;
 hdwp=BeginDeferWindowPos(23);
 if(hdwp==0)return;
 hdwpcur=hdwp;
DeferWindowPos(hdwp,svwp->hServList,0,0,0,svwp->listsz.right+wplus,svwp->listsz.bottom+hplus,SWP_NOZORDER|SWP_NOMOVE|SWP_NOOWNERZORDER);
for(i=0;i<22;i++){//106...127
 if(i==2 || i==3 || i==19)continue;
 htmp=GetDlgItem(hDlg,i+106);
 if(htmp!=NULL){
  pt.x=svwp->szr[i].x;
  pt.y=svwp->szr[i].y; pt.y+=hplus;
  hdwpcur=DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,0,0,SWP_NOZORDER|SWP_NOSIZE);
 }
}
if((htmp=GetDlgItem(hDlg,108))!=0){
 pt.x=svwp->szr[2].x;
 pt.y=svwp->szr[2].y; pt.y+=hplus;
 wd=newwd; wd-=pt.x; wd-=3; if(wd<164)wd=164;
 hg=newsz; hg-=pt.y; hg-=3; if(svwp->IsSeparateWin)hg-=statusHg;
 hdwpcur=DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,wd,hg,SWP_NOZORDER);
}
if((htmp=GetDlgItem(hDlg,109))!=0){
 pt.x=svwp->szr[3].x;
 pt.y=svwp->szr[3].y; pt.y+=hplus;
 wd=newwd; wd-=pt.x; wd-=6; if(wd<157)wd=157;
 hg=newsz; hg-=pt.y; hg-=6; if(svwp->IsSeparateWin)hg-=statusHg;
 DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,wd,hg,SWP_NOZORDER);
}
if((htmp=GetDlgItem(hDlg,125))!=0){
 pt.x=svwp->szr[2].x;
 pt.y=svwp->szr[2].y; pt.y+=hplus;
 wd=newwd; wd-=pt.x; wd-=3; if(wd<164)wd=164;
 DeferWindowPos(hdwpcur,htmp,0,pt.x,pt.y,wd,statusHg,SWP_NOZORDER);
}
EndDeferWindowPos(hdwpcur);
}

//////////////////////////////////// On Init ///////////////////////////////////
void OnInit_Services(SERVICES_WIN_PARAMS *svwp,HWND hDlg){HIMAGELIST himgl; DWORD_PTR *ptrs; int ni,i; DWORD cnt,n,dw[3]; RECT listsz; LV_COLUMN lvcol;
 himgl=ImageList_Create(16,16,ILC_COLORDDB,6,1); svwp->himgl=himgl;
 ImageList_SetBkColor(himgl,0x00ffffff);
 for(i=4;i<10;i++)ImageList_AddIcon(himgl,LoadIcon(gInst,(char*)i));
 GetWindowRect(hDlg,&listsz);
 listsz.left=3;
 listsz.top=(svwp->IsSeparateWin==0)?20:58;
 listsz.right=svwp->initWd-7;
 listsz.bottom=svwp->initHg-listsz.top-117;
 if(svwp->IsSeparateWin){
  listsz.bottom-=statusHg; listsz.bottom-=4;
  svwp->hstatus=GetDlgItem(hDlg,125);
  dw[0]=132; dw[1]=360; dw[2]=(DWORD)-1;
  SendMessage(svwp->hstatus,SB_SETPARTS,(WPARAM)3,(LPARAM)&dw);
  SendMessage(svwp->hstatus,SB_SETTEXT,(WPARAM)0,(LPARAM)smemTable->RemoteComputer);
 }
 svwp->listsz.left=3;
 svwp->listsz.top=listsz.top;
 svwp->listsz.right=listsz.right;
 svwp->listsz.bottom=listsz.bottom;
 svwp->hServList=GetDlgItem(hDlg,101);
 SetWindowPos(svwp->hServList,0,listsz.left,listsz.top,listsz.right,listsz.bottom,SWP_NOZORDER);
 lvcol.mask=LVCF_FMT|LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;
 lvcol.fmt=LVCFMT_LEFT;
 cnt=0;
 for(i=0;i<SRVCOLUMNS_COUNT;i++){
  for(int t=0;t<SRVCOLUMNS_COUNT;t++){
   if(SrvColumnsCreateOrder[t]==i+1 && smemTable->SrvcColumnTitle[t][0]){
    if(t==SRVCOLUMNS_COUNT-1 && svwp->IsSeparateWin)break;
    lvcol.pszText=smemTable->SrvcColumnTitle[t];
    lvcol.iSubItem=i;
    lvcol.cx=ServColWidth[t];
    ni=SendMessage(svwp->hServList,LVM_INSERTCOLUMN,(WPARAM)i,(LPARAM)&lvcol); ni++;
    if(svwp->IsSeparateWin==0)Srvclnumber[t]=ni;
    else Srvclnumber[t]=ni;
    if(ni)cnt++;
    break;
   }
  }
 }
 n=LVS_MY_EXTENDED_STYLE; if(progSetts.CheckState[16]==MFS_CHECKED)n|=LVS_EX_GRIDLINES;
 SendMessage(svwp->hServList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
 if(bValidSrvOrderArray)SendMessage(svwp->hServList,LVM_SETCOLUMNORDERARRAY,(WPARAM)cnt,(LPARAM)&SrvOrderArray);
 ListView_SetImageList(svwp->hServList,himgl,LVSIL_SMALL);
 svwp->ComboStrtType=GetDlgItem(hDlg,107);
 ptrs=(DWORD_PTR*)&(smemTable->Boot);
 for(i=0;i<5;i++){
  in=SendMessage(svwp->ComboStrtType,CB_ADDSTRING,0,(LPARAM)(char*)*ptrs);
  if(in!=CB_ERR)SendMessage(svwp->ComboStrtType,CB_SETITEMDATA,in,(LPARAM)i);
  ptrs++;
 }
 if(svwp->IsSeparateWin)goto skip_drwmenubar;
 CheckMarkMenuItems();
skip_drwmenubar:
 itemSel=-1;
 if(!svwp->IsSeparateWin)SetFocus(svwp->hServList);
 else SetFocus(GetDlgItem(hDlg,272));
 //
 svwp->bFirstTimeSrvTab=1;
}

LPTSTR SrvcTimeToStr(HWND hDlg,FILETIME *ftm){SERVICES_WIN_PARAMS *svwp; SYSTEMTIME stm; LPTSTR lpOutputBuf;
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))return 0;
 lpOutputBuf=svwp->tbuf2;
 if(lpOutputBuf==0)return 0;
 if(FileTimeToSystemTime(ftm,&stm)){
  if(GetDateFormat(LOCALE_USER_DEFAULT,0,&stm,"dd.MM.yyyy",lpOutputBuf,12)){
   lstrcat(lpOutputBuf," ");
   GetTimeFormat(LOCALE_USER_DEFAULT,0,&stm,"HH:mm",lpOutputBuf+lstrlen(lpOutputBuf),7);
  }
 }
 return lpOutputBuf;
}

///////////////////////////////// Services_Proc ////////////////////////////////
BOOL CALLBACK Services_Proc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam){SERVICES_WIN_PARAMS *svwp; WORD wID; LV_ITEM lvi;
if(progSetts.MenuStyle!=MENU_STYLE_DEFAULT){
 if(Message==WM_INITMENUPOPUP || Message==WM_UNINITMENUPOPUP || Message==WM_MEASUREITEM || Message==WM_DRAWITEM || Message==WM_ENTERMENULOOP || Message==WM_EXITMENULOOP){
  ModernMenu_DrawMenuProc(hDlg,Message,wParam,lParam);
 }
}
switch(Message){
case WM_INITDIALOG:
 if(lParam==0){
  svwp=(SERVICES_WIN_PARAMS*)LocalAlloc(LPTR,sizeof(SERVICES_WIN_PARAMS));
 }
 else svwp=(SERVICES_WIN_PARAMS*)lParam;
 if(svwp==0)break;
 HWND htmp; POINT pt; int hpls,i;
 svwp->cbsize=sizeof(SERVICES_WIN_PARAMS);
 SetWindowLongPtr(hDlg,GWLP_USERDATA,(DWORD_PTR)svwp);
 if(svwp->IsSeparateWin==0){
  svwp->initHg=pHdr->rcDisplay.bottom;
  svwp->initWd=pHdr->rcDisplay.right; svwp->initWd-=4;
  hpls=svwp->initHg-195;
 }
 else {RECT rect;
  GetClientRect(hDlg,&rect);
  svwp->initHg=rect.bottom;// svwp->initHg-=rect.top;
  svwp->initWd=rect.right;// Svwp->initWd-=rect.left;
  svwp->initWd-=4;
  hpls=0;
 }
 for(i=0;i<22;i++){//106...127
  htmp=GetDlgItem(hDlg,i+106);
  if(htmp!=NULL){
   GetWindowRect(htmp,&clt); pt.y=clt.top; pt.x=clt.left;
   ScreenToClient(hDlg,&pt);
   svwp->szr[i].x=pt.x;
   if(i!=19)pt.y+=hpls;
   else{pt.y+=statusHg; pt.y+=4;}
   svwp->szr[i].y=pt.y;
  }
 }
 WindowFillText(hDlg,dlgTAT->dlg3);
 if(svwp->IsSeparateWin==0){
  svwp->hstatus=hstatus;
  services_win=hDlg;
  MainTab_OnChildDialogInit(hDlg); pHdr->hwndDisplay=hDlg;
  svwp->rcDisplay.left=pHdr->rcDisplay.left;
  svwp->rcDisplay.right=pHdr->rcDisplay.right;
  svwp->rcDisplay.top=pHdr->rcDisplay.top;
  svwp->rcDisplay.bottom=pHdr->rcDisplay.bottom;
 }
 OnInit_Services(svwp,hDlg);
 if(progSetts.LastServcEnumButn==0)progSetts.LastServcEnumButn=102;
 CheckRadioButton(hDlg,102,105,progSetts.LastServcEnumButn);
 svwp->pHeap=HeapCreate(0,1024,0);
 svwp->bDestroy=0;
 if((DWORD_PTR)(svwp->szComputerName)!=(DWORD_PTR)-1){
  svwp->scman=OpenSCManager(svwp->szComputerName,0,SC_MANAGER_ENUMERATE_SERVICE);
  if(svwp->IsSeparateWin==0){
   ExecuteAddr=(DWORD_PTR)ShowServicesInfo;
   ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0x87ABFE);
  }
  else ShowServicesInfo(hDlg);
 }
 else ClearAdditionalInfo(hDlg);
 if(progSetts.SrvSortd.CurSubItem!=(DWORD)-1)SetSortArrowIcon(svwp->hServList,(DWORD)-1,progSetts.SrvSortd.CurSubItem,progSetts.SrvSortd.SortDirection);
 break;
case WM_DESTROY:
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
 svwp->bDestroy=1;
 SaveColumns_Services(svwp);
 if(svwp->hServList!=NULL)DestroyWindow(svwp->hServList);
 if(svwp->scman)CloseServiceHandle(svwp->scman);
 if(svwp->pHeap)HeapDestroy(svwp->pHeap);
 if(svwp->IsSeparateWin==0){
  DeleteMenu(main_menu,47201,MF_BYCOMMAND);
  DeleteMenu(main_menu,47202,MF_BYCOMMAND);
  DeleteMenu(main_menu,30016,MF_BYCOMMAND);
  services_win=NULL;
 }
 SendMessage(svwp->hstatus,SB_SETTEXT,(WPARAM)((svwp->IsSeparateWin)?2:STATUSBAR_PANEINFO_ID),(LPARAM)0);
 ImageList_Destroy(svwp->himgl);
 svwp->cbsize=0; bool bl; bl=svwp->IsSeparateWin;
 LocalFree(svwp);
 if(bl)PostQuitMessage(0);
 break;
case WM_COMMAND: wID=LOWORD(wParam);
 switch(wID){
  case 102: case 103: case 104: case 105: progSetts.LastServcEnumButn=wID;
  case 50010:
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
   if(svwp->IsSeparateWin==0){
    ExecuteAddr=(DWORD_PTR)ShowServicesInfo;
    ControlUpdateThread(UPDATETHREAD_EXECUTE,ExecuteAddr,0x87ABFE);
   }
   else ShowServicesInfo(hDlg);
   break;
  case 107:
   if(HIWORD(wParam)==CBN_SELCHANGE)ReconfigService(hDlg);
   break;
  case 121: case 122: case 123: case 124: ControlSelService(wID,hDlg); break;
  case 271: case 272:
   DWORD ErrCode;
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp) || svwp->IsSeparateWin==0)break;
   if(wID==272 && HIWORD(wParam)==EN_CHANGE){
    EnableWindow(GetDlgItem(hDlg,271),1);
    break;
   }
   if(wID!=271)break;
   if(GetDlgItemText(hDlg,272,svwp->tbuf,200)==0)break;
   if(svwp->szComputerName!=0 && (DWORD_PTR)svwp->szComputerName!=(DWORD_PTR)-1){
    if(lstrcmp(svwp->szComputerName,svwp->tbuf)==0)goto connect_comp;
    LocalFree(svwp->szComputerName);
   }
   svwp->szComputerName=AllocateAndCopy(0,svwp->tbuf);
   connect_comp:
   if(svwp->scman!=0){CloseServiceHandle(svwp->scman); svwp->scman=0;}
   svwp->scman=OpenSCManager(svwp->szComputerName,NULL,SC_MANAGER_ENUMERATE_SERVICE);
   if(svwp->scman==0){
    ErrCode=GetLastError();
    SetWindowText(hDlg,smemTable->ServicesOnRemoteComp);
    EnableWindow(GetDlgItem(hDlg,274),0);
   }
   else EnableWindow(GetDlgItem(hDlg,271),0);
   ShowServicesInfo(hDlg);
   if(svwp->scman==0){
    SendMessage(svwp->hstatus,SB_SETTEXT,(WPARAM)1,(LPARAM)0);
    SendMessage(svwp->hstatus,SB_SETTEXT,(WPARAM)2,(LPARAM)0);
    FailMessage(smemTable->UnblOpenSCManager,ErrCode,FMSG_SHOW_MSGBOX);
    break;
   }
   int ln; ln=copystring(svwp->tbuf,smemTable->Services_0x20);
   copystring(svwp->tbuf+ln,svwp->szComputerName);
   SendMessage(svwp->hstatus,SB_SETTEXT,(WPARAM)1,(LPARAM)svwp->szComputerName);
   EnableWindow(GetDlgItem(hDlg,274),1);
   SetWindowText(hDlg,svwp->tbuf);
   SetFocus(svwp->hServList);
   break;
  case 274: ShowServicesInfo(hDlg); break;
  case 30016:
   DWORD n;
   n=LVS_MY_EXTENDED_STYLE2;
   if(progSetts.CheckState[16]==MFS_CHECKED)progSetts.CheckState[16]=MFS_UNCHECKED;
   else {progSetts.CheckState[16]=MFS_CHECKED; n|=LVS_EX_GRIDLINES;}
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
   CheckMenuItem(main_menu,30016,progSetts.CheckState[16]);
   SendMessage(svwp->hServList,LVM_SETEXTENDEDLISTVIEWSTYLE,(WPARAM)0,(LPARAM)n);
   break;
  case 40018: case 40020: case 40017:
contextmenu_click:
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
   if((itemSel=SendMessage(svwp->hServList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.mask=LVIF_PARAM; lvi.iItem=itemSel;
   if(SendMessage(svwp->hServList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || lvi.lParam==0)break;
   SERVICE_INFOBUF *svif; svif=(SERVICE_INFOBUF*)lvi.lParam;
   if(wID==40017){
    if(svif->String[1]){
     DATA_TO_SVCPROPS_THREAD *svdata;
     svdata=(DATA_TO_SVCPROPS_THREAD*)LocalAlloc(LMEM_FIXED,sizeof(DATA_TO_SVCPROPS_THREAD));
     svdata->SrvcName=svif->String[1];
     svdata->szComputerName=svwp->szComputerName;
     svdata->hMain=hDlg;
     CloseHandle(CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)SrvcPropsThread,(void*)svdata,0,&n));
    }
    break;
   }
   if(svif->String[0]){
    char *pptr; pptr=svif->String[0];
    if(*(WORD*)(svif->String[0])==0x203F)pptr+=2;
    pptr=CorrectFilePath(pptr,0,t0nbuf2);
    if(*pptr){
     if(wID==40018)File_OpenProperies(pptr);
     else{
      char *pttr;
      if(ParseFile(pptr,0,&pttr,t1buf9)==0)pttr=pptr;
      *((char*)t0nbuf4)='\"';
      int lnn; lnn=copybytes((char*)t0nbuf4+1,pttr,0,2);
      lnn++; *(((BYTE*)t0nbuf4)+lnn)='\"';
      lnn++; *(((BYTE*)t0nbuf4)+lnn)=0;
      if((DWORD)ShellExecute(NULL,open_txt,dependsPath,(char*)t0nbuf4,0,SW_SHOWNORMAL)>32){
       if(progSetts.CheckState[0] && progSetts.CheckState[20])ShowWindow(main_win,SW_MINIMIZE);
      }
     }
    }
   }
   break;
  case 40019: goto delete_service;
  case 40022:
   CreateRemoteThread((HANDLE)-1,0,0,(LPTHREAD_START_ROUTINE)RemoteServicesWinThread,(void*)0,0,0);
   break;
  case 40030: CreateColumnSelDlg(3); break;
  case 50004:
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp) || svwp->IsSeparateWin==0)break;
   goto close_win;
 }
 break;
case WM_SIZING:
 svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
 if(!IsVALID_SERVICES_WIN_PARAMS(svwp) || !svwp->IsSeparateWin)break;
 RECT *mRect; LONG newWidth,newHeight;
 mRect=(RECT*)lParam;
 newWidth=mRect->right; newWidth-=mRect->left;
 newHeight=mRect->bottom; newHeight-=mRect->top;
 if(newWidth<=600){newWidth=600; mRect->right=mRect->left; mRect->right+=600;}
 if(newHeight<=280){newHeight=280; mRect->bottom=mRect->top; mRect->bottom+=280;}
 svwp->win_hg=newHeight; svwp->win_wd=newWidth;
 break;
case WM_SIZE: resizeServicesWin(HIWORD(lParam),LOWORD(lParam),hDlg); break;
case WM_CLOSE: close_win: DestroyWindow(hDlg); break;
case WM_NOTIFY:
 DWORD dCode; dCode=((NMHDR*)lParam)->code;
 if(dCode==NM_RCLICK || dCode==NM_CLICK){
  svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
  if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
  itemSel=SendMessage(svwp->hServList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED);
  if(itemSel!=prevSel && itemSel==-1){
showAddtnlInfo:
   if(prevSel==-1)EnableWindow(GetDlgItem(hDlg,107),1);
   prevSel=itemSel;
   AdditionalServiceInfo(0,svwp);
  }
  if(dCode==NM_RCLICK){
   POINT curpos; GetCursorPos(&curpos);
   if(CheckHeaderClick(svwp->hServList,((NMHDR*)lParam)->hwndFrom,&curpos)==1 || itemSel==-1)break;
   SrvcContextMenu(hDlg,0);
  }
  break;
 }
 if(((NMHDR*)lParam)->idFrom!=101)break;
 if(dCode==NM_CUSTOMDRAW){
  SetWindowLong(hDlg,DWL_MSGRESULT,(LONG)SrvListviewCustomDraw(lParam));
  return 1;
 }
 else if(dCode==LVN_GETDISPINFO){
  NMLVDISPINFO *nvd; nvd=(NMLVDISPINFO*)lParam;
  SERVICE_INFOBUF *svif;
  svif=(SERVICE_INFOBUF*)(nvd->item.lParam);
  if(!IsVALID_SERVICE_INFO(svif))break;
  if(nvd->item.mask&LVIF_TEXT){
   int iw; iw=nvd->item.iSubItem; iw++;
   if(iw==Srvclnumber[0])nvd->item.pszText=svif->String[1];
   else if(iw==Srvclnumber[1])nvd->item.pszText=svif->String[2];
   else if(iw==Srvclnumber[2])getServiceStatus(svif->dwStatus,&nvd->item.pszText);
   else if(iw==Srvclnumber[3]){
    LPTSTR *ptrs; ptrs=(LPTSTR*)&(smemTable->Boot);
    ptrs+=svif->BootType; nvd->item.pszText=*ptrs;
   }
   else if(iw==Srvclnumber[4])nvd->item.pszText=svif->String[3];
   else if(iw==Srvclnumber[5])nvd->item.pszText=svif->String[4];
   else if(iw==Srvclnumber[6] && svif->dwFlags&SRVC_CREATEMODIFDATE_VALID){
    if((svif->dwFlags&SRVC_FILE_NOT_FOUND)!=SRVC_FILE_NOT_FOUND && svif->String[0]!=0)nvd->item.pszText=SrvcTimeToStr(hDlg,&svif->ftCreationTime);
   }
   else if(iw==Srvclnumber[7] && svif->dwFlags&SRVC_CREATEMODIFDATE_VALID){
    if((svif->dwFlags&SRVC_FILE_NOT_FOUND)!=SRVC_FILE_NOT_FOUND && svif->String[0]!=0)nvd->item.pszText=SrvcTimeToStr(hDlg,&svif->ftLastWriteTime);
   }
   else if(iw==Srvclnumber[8] && svif->dwFlags&SRVC_REGWRITEDATE_VALID)nvd->item.pszText=SrvcTimeToStr(hDlg,&svif->ftRegLastWriteTime);
   if(nvd->item.pszText==0)nvd->item.pszText="";
  }
 }
 else if(dCode==LVN_COLUMNCLICK){
  progSetts.SrvSortd.hFrom=((NMHDR*)lParam)->hwndFrom;
  progSetts.SrvSortd.iSub=(((NM_LISTVIEW*)lParam)->iSubItem); progSetts.SrvSortd.iSub++;
  progSetts.SrvSortd.bRestoreSort=1;
  ListViewColumnSortClick((NM_LISTVIEW*)lParam,&progSetts.SrvSortd,(DWORD_PTR)SrvcListCompareFunc);
 }
 else if(dCode==LVN_KEYDOWN){
  if(((LPNMLVKEYDOWN)lParam)->wVKey==VK_DELETE){
delete_service:
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
   if((lvi.iItem=SendMessage(svwp->hServList,LVM_GETNEXTITEM,-1,(LPARAM)LVNI_SELECTED))==-1)break;
   lvi.mask=LVIF_PARAM; lvi.lParam=0;
   if(SendMessage(svwp->hServList,LVM_GETITEM,0,(LPARAM)&lvi)==0 || !IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)lvi.lParam)))break;
   Remove_Service((SERVICE_INFOBUF*)lvi.lParam,hDlg);
  }
  else if(((LPNMLVKEYDOWN)lParam)->wVKey==VK_RETURN){
   wID=40017; goto contextmenu_click;
  }
 }
 else if(dCode==NM_DBLCLK){wID=40017; goto contextmenu_click;}
 else if(dCode==LVN_ITEMCHANGED){
  NMLISTVIEW *nmlv; nmlv=(LPNMLISTVIEW)lParam; if(nmlv==0)break;
  if(nmlv->uChanged&LVIF_STATE && nmlv->uNewState&LVIS_SELECTED){
   svwp=(SERVICES_WIN_PARAMS*)GetWindowLongPtr(hDlg,GWLP_USERDATA);
   if(!IsVALID_SERVICES_WIN_PARAMS(svwp))break;
   itemSel=nmlv->iItem;
   if(itemSel!=prevSel){
    if(itemSel!=-1){DWORD dwFlags;
     lvi.mask=LVIF_PARAM; lvi.lParam=0; lvi.iItem=itemSel;
     dwFlags=MF_BYCOMMAND|MF_ENABLED;
     if(SendMessage(svwp->hServList,LVM_GETITEM,0,(LPARAM)&lvi)!=0 && IsVALID_SERVICE_INFO(((SERVICE_INFOBUF*)lvi.lParam))){
      char *svnm; svnm=((SERVICE_INFOBUF*)lvi.lParam)->String[0];
      if(svnm==0 || *svnm==0)dwFlags=MF_BYCOMMAND|MF_GRAYED;
      else if(svwp->IsSeparateWin && svwp->szComputerName!=0)dwFlags=MF_BYCOMMAND|MF_GRAYED;
     }
     EnableMenuItem(main_menu,40018,dwFlags);
     if(dwFlags!=(MF_BYCOMMAND|MF_GRAYED)){
      if(dependsPath && *dependsPath)dwFlags=MF_BYCOMMAND|MF_ENABLED;
      else dwFlags=MF_BYCOMMAND|MF_GRAYED;
     }
     EnableMenuItem(main_menu,40020,dwFlags);
    }
    goto showAddtnlInfo;
   }
  }
 }
 break;
} return 0;}
