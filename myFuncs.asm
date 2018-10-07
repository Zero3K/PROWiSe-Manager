	.586p
	model flat
_TEXT	segment dword public use32 'CODE'

asmCalcHash32 proc near
	  push ebx
	  push edi
	  mov esi,dword ptr [esp+4+8] ;address of string
	  xor eax,eax
	  xor edi,edi
	  push eax                      ;make a place to store the hash_by_bytes
	  mov eax,[esi]                 ;add first 4 bytes to hash
_CalcHash32:
	  ror  eax,7
	  xor  [esp],eax
	  lodsb
	  inc edi           ;calc string size
	  test al,al
	  jnz _CalcHash32
	  mov esi,dword ptr [esp+4+8+4] ;address of string
	  mov eax,edi                   ;add string length to hash
	  sub edi,5                     ;get address of last 4 bytes
	  add edi,esi
_AddDword32:                            ;More Hash by dwords
	  add eax,[esi]                 ;eax=*(DWORD*)ptr
	  add esi,4                     ;ptr+=4
	  cmp edi,esi                   ;ptr<end ?
	  ja _AddDword32
	  add eax,[edi]                 ;add last 4 bytes to hash
	  pop edi                       ;edi=hash_by_bytes
	  xor eax,edi                   ;hash_by_bytes XOR hash_by_dwords
	  pop edi
	  pop ebx
	  ret
asmCalcHash32 endp

_TEXT	ends

public asmCalcHash32

end