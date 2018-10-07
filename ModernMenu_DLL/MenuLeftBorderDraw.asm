		.586
    .model flat, stdcall
    option casemap :none

;     include files
;     ~~~~~~~~~~~~~
    include C:\Assemblers\MASM\masm32\include\windows.inc
    include C:\Assemblers\MASM\masm32\include\masm32.inc
    include C:\Assemblers\MASM\masm32\include\gdi32.inc
    include C:\Assemblers\MASM\masm32\include\user32.inc
    include C:\Assemblers\MASM\masm32\include\kernel32.inc
    include C:\Assemblers\MASM\masm32\include\comctl32.inc
    include C:\Assemblers\MASM\masm32\include\debug.inc
    include C:\Assemblers\MASM\masm32\include\Msimg32.inc
    include C:\Assemblers\MASM\masm32\include\winmm.inc
      
;     macros
;     ~~~~~~~~~
    include   C:\Assemblers\MASM\masm32\macros\macros.asm
    include   XXMacros.inc

_DATA	SEGMENT PARA PUBLIC 'DATA'

 hMenu  HMENU ?
 fAccel BYTE  TRUE
 ; флаги
 fPopup BYTE  ?
 
_DATA ends


_TEXT	segment public use32 'CODE'

;##########################################################################
; Процедура обработки сообщения WM_DrawItem.                              ;
;##########################################################################
DrawItem_MenuDefault proc uses ebx
	LOCAL RctLeftBorder:RECT
	LOCAL mii         :MENUITEMINFO

	assume ebx: ptr DRAWITEMSTRUCT
		; выберем информацию о пункте меню    
    mov mii.cbSize, sizeof(MENUITEMINFO)
    mov mii.fMask, MIIM_DATA or MIIM_BITMAP or MIIM_FTYPE ;or MIIM_STRING
    invoke GetMenuItemInfo, [ebx].hwndItem, [ebx].itemID, FALSE, ADDR mii
    mov eax, mii.fType
    and eax, MFT_SEPARATOR
   .if eax == MFT_SEPARATOR && mii.dwItemData          ; Пункт меню SEPARATOR
    
            invoke GetClipBox, [ebx].hdc, ADDR RctLeftBorder
            m2m RctLeftBorder.left, [ebx].rcItem.left
            m2m RctLeftBorder.right, [ebx].rcItem.right

;            mov eax, offset RctLeftBorder
            lea eax, RctLeftBorder
            push eax
            push [ebx].hdc
            push [ebx].hwndItem
            call mii.dwItemData    
    
    .endif
    
    ret
DrawItem_MenuDefault endp

DrawItem_Menu proc uses ebx lParam:LPARAM
		mov ebx, lParam
	assume ebx: ptr DRAWITEMSTRUCT
    invoke IsMenu, hMenu
    test eax,eax
    jz e_t
		.if [ebx].CtlType == ODT_MENU
			invoke SaveDC, [ebx].hdc
			invoke SetBkMode, [ebx].hdc, TRANSPARENT
      call DrawItem_MenuDefault        
			invoke RestoreDC, [ebx].hdc, -1
		.endif
e_t:
	assume ebx: nothing
		mov eax,1
		ret
DrawItem_Menu endp

; #########################################################################
; Процедура обработки сообщения WM_MeasureItem_Menu.                      ;
; #########################################################################
MeasureItem_Menu proc uses ebx ecx lParam:LPARAM
	LOCAL mii         :MENUITEMINFO
	LOCAL hDC         :HDC

    mov ebx, lParam
	assume ebx: ptr MEASUREITEMSTRUCT
		.if [ebx].CtlType == ODT_MENU

          mov mii.cbSize, SIZEOF mii
          mov mii.fMask, MIIM_TYPE or MIIM_DATA or MIIM_STATE
          invoke GetMenuItemInfo,hMenu,[ebx].itemID,FALSE,ADDR mii        
                
          mov eax, mii.fType
          and eax, MFT_SEPARATOR or MFT_STRING or MFT_BITMAP
                
          .if eax == MFT_SEPARATOR
                
                .if fPopup  ; popup
                
                    .if mii.dwItemData
                        mov [ebx].itemHeight, 5 ; не принципиально

                        push 0
                        push 0
                        push hMenu
                        call mii.dwItemData
                        .if eax
                            mov [ebx].itemWidth, eax
                        .else
                            m2m [ebx].itemWidth, 8 ; LeftBmpWidth
                        .endif                       
                    .else
                        mov [ebx].itemHeight, 6
                        mov [ebx].itemWidth, 8
                    .endif

                .else       ; main                
                    mov [ebx].itemHeight, 8
                    mov [ebx].itemWidth, 8                     
                .endif

          .endif
                                
        .endif   

    assume ebx: nothing
    ret
MeasureItem_Menu endp

; #########################################################################
; Процедура субкласирования для окна использующего меню                   ;
; #########################################################################    
MenuDrawItemProc proc hWin:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
	LOCAL OldhMenu  :HMENU

		.if uMsg == WM_INITMENUPOPUP    
			.if !hMenu && !fAccel
				m2m hMenu, wParam             
				mov fPopup, TRUE    
			.endif    

		.elseif uMsg == WM_UNINITMENUPOPUP
			mov eax, wParam
			.if hMenu == eax && !fAccel
				mov hMenu, 0            
			.endif

		.elseif uMsg == WM_MEASUREITEM
			.if !wParam    ; MENU? (wParam == 0 - only menu)     
				m2m OldhMenu, hMenu             ; Это нужно что бы потом, распознать хендл при оброботке "WM_UNINITMENUPOPUP"
				.if !hMenu                      ; установим флаги в соответствии с тем какое меню (main - popup)
					invoke GetMenu, hWin
					mov hMenu, eax
					mov fPopup, FALSE
				.else
					mov fPopup, TRUE    
				.endif
				invoke IsMenu, hMenu            ; если не хендл, зачем прорисовывать
				test eax,eax                
				jz no_measure
				invoke MeasureItem_Menu, lParam

no_measure:
				m2m hMenu, OldhMenu
				mov fPopup, TRUE            
			.endif
 
		.elseif uMsg == WM_DRAWITEM
			.if !wParam    ; MENU? (wParam == 0 - only menu)     
				m2m OldhMenu, hMenu         ; Это нужно что бы потом, распознать хендл при оброботке "WM_UNINITMENUPOPUP"
				.if !hMenu                  ; установим флаги в соответствии с тем какое меню (main - popup)
@@:
					invoke GetMenu, hWin
					mov hMenu, eax
					mov fPopup, FALSE
				.else
					.if fAccel
						jmp @b
					.else
						mov fPopup, TRUE
					.endif
				.endif
				invoke IsMenu, hMenu        ; если не хендл, зачем прорисовывать
				test eax,eax                
				jz no_DrawItem_Menu
				invoke DrawItem_Menu, lParam
        
no_DrawItem_Menu:
				m2m hMenu, OldhMenu
				mov fPopup, TRUE
			.endif

		.elseif uMsg == WM_ENTERMENULOOP
			mov fAccel, FALSE

		.elseif uMsg == WM_EXITMENULOOP
			mov fAccel, TRUE

	.endif           
	xor eax,eax
	ret
MenuDrawItemProc endp

_TEXT ends

PUBLIC MenuDrawItemProc

end