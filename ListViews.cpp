
void SetSortArrowIcon(HWND hListView,int prevClmn,int newClmn,BYTE SortDir){HDITEM hdi; HWND hHeader;
 if((hHeader=(HWND)SendMessage(hListView,LVM_GETHEADER,0,0))==0)return;
 if(barrdn==0)barrdn=(HBITMAP)LoadImage(gInst,(char*)9,IMAGE_BITMAP,9,5,LR_LOADMAP3DCOLORS|LR_SHARED);
 if(barrup==0)barrup=(HBITMAP)LoadImage(gInst,(char*)8,IMAGE_BITMAP,9,5,LR_LOADMAP3DCOLORS|LR_SHARED);
 hdi.mask=HDI_FORMAT;
 if(prevClmn!=0xFFFFFFFF){
  if(SendMessage(hHeader,HDM_GETITEM,prevClmn,(LPARAM)&hdi)){
   hdi.fmt&=~(HDF_BITMAP_ON_RIGHT|HDF_BITMAP);
   SendMessage(hHeader,HDM_SETITEM,prevClmn,(LPARAM)&hdi);
  }
 }
 if(newClmn!=0xFFFFFFFF && SortDir<2){
  if(SendMessage(hHeader,HDM_GETITEM,newClmn,(LPARAM)&hdi)){
   hdi.mask=HDI_BITMAP|HDI_FORMAT;
   hdi.fmt|=HDF_BITMAP_ON_RIGHT|HDF_BITMAP;
   if(progSetts.CheckState[37]==0){
    if(SortDir==0)hdi.hbm=barrdn;
    else if(SortDir==1)hdi.hbm=barrup;
   }
   else{
    if(SortDir==1)hdi.hbm=barrdn;
    else if(SortDir==0)hdi.hbm=barrup;
   }
   SendMessage(hHeader,HDM_SETITEM,newClmn,(LPARAM)&hdi);
  }
 }
}

//int GetSelectedItem(HWND hListView){return SendMessage(hListView,LVM_GETNEXTITEM,-1,MAKELPARAM(LVNI_SELECTED,0));}//0x00020000

bool CheckHeaderClick(HWND hLVFrom,HWND hFrom,POINT *curpos){HWND hHdr; HMENU pmenu;
 if((hHdr=(HWND)SendMessage(hLVFrom,LVM_GETHEADER,0,0))==0 || hFrom!=hHdr)return 0;
 pmenu=CreatePopupMenu();
 InsertMenu(pmenu,0,MF_BYPOSITION|MF_STRING,40030,smemTable->selectcolumns);
 TrackPopupMenuEx(pmenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,curpos->x,curpos->y,GetParent(hLVFrom),NULL);
 DestroyMenu(pmenu);
 return 1;
}

void ListViewColumnSortClick(NM_LISTVIEW *nmlv, LIST_SORT_DIRECTION *lsd,DWORD_PTR SortFunc){DWORD itmClick;
 itmClick=(DWORD)nmlv->iSubItem;
 if(lsd->CurSubItem==itmClick || lsd->SortDirection==2){
  if(lsd->SortDirection>1)lsd->SortDirection=0;
  else lsd->SortDirection++;
 }
 SetSortArrowIcon(nmlv->hdr.hwndFrom,lsd->CurSubItem,itmClick,lsd->SortDirection);
 lsd->CurSubItem=itmClick;
 if(lsd->SortDirection<2)goto send_sortmsg;
 if(lsd->bRestoreSort){
 //if(nmlv->hdr.hwndFrom==hModuleList || nmlv->hdr.hwndFrom==hHandleList || StRunList || hdnList || DrvList){
send_sortmsg:
  SendMessage(nmlv->hdr.hwndFrom,LVM_SORTITEMS,(WPARAM)lsd,SortFunc);//0=\/ ; 1=/\; 2=off
 }
}

void LV_ColumnClickSort(NM_LISTVIEW *nmlv, LIST_SORT_DIRECTION *lsd,DWORD_PTR SortFunc){DWORD itmClick;
 itmClick=(DWORD)nmlv->iSubItem;
 if(lsd->CurSubItem==itmClick || lsd->SortDirection==2){
  if(lsd->SortDirection>1)lsd->SortDirection=0;
  else lsd->SortDirection++;
 }
 SetSortArrowIcon(nmlv->hdr.hwndFrom,lsd->CurSubItem,itmClick,lsd->SortDirection);
 lsd->CurSubItem=itmClick;
 SendMessage(nmlv->hdr.hwndFrom,LVM_SORTITEMS,(WPARAM)lsd,SortFunc);//0=\/ ; 1=/\; 2=def
}