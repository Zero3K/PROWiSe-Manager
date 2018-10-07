
typedef struct {
 BYTE sp2_head_plus[2];
 BYTE Pushes_Mov[45];
 BYTE Ret[4];
}CREATE_WINDOWS_EX_W_ORIGINAL_DATA;

CREATE_WINDOWS_EX_W_ORIGINAL_DATA CreateWindowExW_orig={
 {0x8B,0xFF},
 {0x55,0x8B,0xEC,0x68,0x00,0x00,0x00,0x40, 0xFF,0x75,0x34,0xFF,0x75,0x30,0xFF,0x75,0x2C,0xFF,0x75,0x28,0xFF,0x75,0x24,0xFF,0x75,0x20,0xFF,0x75,0x1C,0xFF,0x75,0x18,0xFF,0x75,0x14,0xFF,0x75,0x10,0xFF,0x75,0x0C,0xFF,0x75,0x08, 0xE8},
 {0x5D,0xC2,0x30,0x00}
};

BYTE CreateWindowExW_mycode_preEnd[3]={
 0xFF,0xD0,//call eax
 0x5D//pop ebp
};

BYTE memJmp_inj[7]={0xB8,00,00,00,00,0xFF,0xE0};//mov eax,DWORD; jmp eax;

void TryHook_CreateWindowExW(){DWORD n; BYTE *cwexw_spy,*lmem,*pbt,*pcmp; int i,in,len,svpack;
 //check compatability
 svpack=0; n=0;
 pcmp=CreateWindowExW_orig.Pushes_Mov; len=sizeof(CreateWindowExW_orig.Pushes_Mov);
 pbt=(BYTE*)(user32Functions.CreateWindowExW);
 if(*(DWORD*)pbt==0x8B55FF8B){svpack=2; pbt+=2;}
 for(i=0;i<len;i++){
  if(*pbt!=*pcmp)break;
  pcmp++; pbt++;
 }
 if(i!=len)return;
 pbt+=4;
 if(*(DWORD*)pbt!=*(DWORD*)(CreateWindowExW_orig.Ret))return;
 //calc asmSetTop() size
 pbt=(BYTE*)asmMyCreateWindowExW_EndCode; i=0;
 while(i<512){
  if(*pbt==0xC2 && *(pbt+1)==0x30 && *(pbt+2)==0x00){i+=3; goto inject_mem;}
  i++; pbt++;
 }
 return;
inject_mem:
 len=i; len+=100;
 if((cwexw_spy=(BYTE*)VirtualAllocEx((HANDLE)-1,0,len,MEM_COMMIT,PAGE_EXECUTE_READWRITE))==0)return;
 if((lmem=(BYTE*)LocalAlloc(LMEM_FIXED,len))==0)goto freemem;
 pbt=lmem;
 if(svpack==2){
  *pbt=CreateWindowExW_orig.sp2_head_plus[0]; pbt++;
  *pbt=CreateWindowExW_orig.sp2_head_plus[1]; pbt++;
 }
 pbt+=copybytes((char*)pbt,(char*)(CreateWindowExW_orig.Pushes_Mov),sizeof(CreateWindowExW_orig.Pushes_Mov)-1,0);
 *pbt=0xB8;//call
 pbt++;
 pcmp=(BYTE*)(user32Functions.CreateWindowExW);
 if(svpack==2)pcmp+=2;
 pcmp+=45;//+4
 n=*(DWORD*)pcmp;// call-offset
 n+=(DWORD)pcmp; n+=4;//plus addr of next operand
 *(DWORD*)pbt=n;//write call-address
 pbt+=4;
 *pbt=CreateWindowExW_mycode_preEnd[0]; pbt++;
 *pbt=CreateWindowExW_mycode_preEnd[1]; pbt++;
 *pbt=CreateWindowExW_mycode_preEnd[2]; pbt++;
 // установка call SetWindowPos()
 SetWindowPos_addr=(DWORD)(user32Functions.SetWindowPos);
 in=0; pcmp=(BYTE*)asmMyCreateWindowExW_EndCode;
 while(in<i){
  if(*(DWORD*)pcmp==0xB850FF6A && *(DWORD*)(pcmp+4)==0xBAADDEAD){
   pcmp+=4;
   if(WriteProcessMemory((HANDLE)-1,pcmp,&SetWindowPos_addr,4,&n)==0 || n!=4)goto freemem;
   goto continue_inject;
  }
  in++; pcmp++;
 }
 goto freemem;
continue_inject:
 copybytes((char*)pbt,(char*)asmMyCreateWindowExW_EndCode,i,0);
 *(DWORD*)(memJmp_inj+1)=(DWORD)cwexw_spy;
 if(WriteProcessMemory((HANDLE)-1,cwexw_spy,lmem,len,&n)==0 || n!=(DWORD)len)goto freemem;
 LocalFree(lmem); lmem=0;
 if(WriteProcessMemory((HANDLE)-1,user32Functions.CreateWindowExW,memJmp_inj,7,&n)==0 || n!=7)goto freemem;
 VirtualProtectEx((HANDLE)-1,cwexw_spy,len,PAGE_EXECUTE_WRITECOPY,&n);
 return;
freemem:
 LocalFree(lmem);
 VirtualFreeEx((HANDLE)-1,cwexw_spy,0,MEM_RELEASE);
}