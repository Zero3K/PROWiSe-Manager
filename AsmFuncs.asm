	.586p
	model flat
_TEXT	segment dword public use32 'CODE'

asmIsProcessInJob proc near
 push [esp+8]
 push [esp+8]
 call	 dword ptr [esp+20]
 test	 eax, eax
 jl	 short quit
 checkisjob_:
  xor	 ecx, ecx
  cmp	 eax, 0123h
  setnz	 cl
  mov	 eax, ecx
 quit:
  retn 0Ch
asmIsProcessInJob endp

asmGetCurrentProcessId proc near
 mov eax, fs:18h
 mov eax, [eax+20h]
 retn
asmGetCurrentProcessId endp

asmGetCurrentPeb proc near
 mov     eax, fs:18h
 mov     eax, [eax+30h]
 retn
asmGetCurrentPeb endp

getstrlen proc near ; ������ ����� ������ � ������
 mov eax,[esp+04h]
 test eax,eax
 jz short _quitA
 dec eax
 _loopstrA:
 inc eax
 mov cl, [eax]
 test cl,cl
 jnz _loopstrA
 sub eax,[esp+04h]      ;len=end_str-start_str
 _quitA:
 ret 4
getstrlen endp

getstrlenW proc near ; ������ ����� ������ � ������
 mov eax,[esp+04h]
 test eax,eax
 jz short _quitW
 sub eax,2
 _loopstrW:
 add eax,2
 cmp word ptr [eax],0
 jnz _loopstrW
 sub eax,[esp+04h]      ;len_bytes=end_str-start_str
 ;shr eax,1              ;len_characters=len_bytes/2
 _quitW:
 ret 4
getstrlenW endp

asmMyCreateWindowExW_EndCode proc near     ; ��� ��������� ���� �������������� ������ � openProperies()
 push eax
 push ebx
 mov ebx,_Enable_SetTop
 test ebx,ebx
 jz _quitAsmSetTop
 push 3			; SWP_NOSIZE|SWP_NOMOVE
 xor ebx,ebx
 push ebx
 push ebx
 push ebx
 push ebx
 push 0FFFFFFFFh	; HWND_TOPMOST
 push eax
 mov eax,0baaddeadh
 call eax
 _quitAsmSetTop:
 pop ebx
 pop eax
 ret 030h
asmMyCreateWindowExW_EndCode endp

_TEXT	ends

extrn	_Enable_SetTop:dword
public asmIsProcessInJob
public asmGetCurrentProcessId
public asmGetCurrentPeb
public getstrlen
public getstrlenW
public asmMyCreateWindowExW_EndCode

end