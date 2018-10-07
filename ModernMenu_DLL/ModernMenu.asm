    .486
    .model flat, stdcall
    option casemap :none
    
    include XXCtrl.inc
    include XXControls.inc

    include XXGrafics.asm  
    include XXMenu.asm

     
.code

;===========================================================================;
; Name:     DllEntry                                                        ;
; Function: Performs an undo action (if any)                                ;
;                                                                           ;
; Params:   lpEditData = Address of the RS Edit control's data structure    ;
;                                                                           ;
; Returns:  Nothing                                                         ;
;===========================================================================;

DllEntry proc hInstance:HINSTANCE, reason:DWORD, reserved1:DWORD

    .if reason==DLL_PROCESS_ATTACH                 ; DLL загружена
                 
    .elseif reason==DLL_PROCESS_DETACH             ; DLL выгружается

 
    .elseif reason==DLL_THREAD_ATTACH

    .else        ; DLL_THREAD_DETACH

    .endif
    mov  eax,1
    ret
DllEntry endp

end DllEntry