;          MotionDetHelpers.asm
;
;
; Notes on calling conventions for WIN32
;
; Scratch registers (do not have to be saved inside procedure)
; EAX, ECX, EDX, ST(0)-ST(7) same as MM0-MM7, XMM0-XMM7
;
; Callee-save registers (have to be saved inside procedure with push and pop)
; EBX, ESI, EDI, EBP
;
; Registers for return
; EAX, EDX, ST(0)

.586
.xmm
.model flat

PublicAlias MACRO MangledName ; macro for giving a function alias public names
       MangledName label near
       public MangledName
ENDM

.code

; ********** Mix Functions **********
; C++ prototype:
; extern "C" void Mix7To1MMX(void* pBackgnd, void* pSrc, int nSize);
;
; average_round_up(a, b)                               = pavgb(a, b)
; average_round_down(a, b) = ~average_round_up(~a, ~b) = ~pavgb(~a, ~b)
; pavgb is only available on processors with SSE!
;
; round((a*7+b)/8)
; = (a*7 + b + 4) >> 3
; = (a*4 + a*2 + a + b + 4) >> 3
; = (((((a + b) >> 1) + a) >> 1) + a + 1) >> 1
; = average_round_up(average_round_down(average_round_down(a, b), a), a)
Mix7To1MMX PROC NEAR
PUBLIC Mix7To1MMX
PublicAlias _Mix7To1MMX	; Underscore needed when called from Windows
	mov     edx, [esp+4]    ; pBackgnd
	mov     eax, [esp+8]    ; pSrc
	mov     ecx, [esp+12]   ; nSize
	pcmpeqb	mm7,	mm7		; mm7 is 0xFFFFFFFFFFFFFFFF
ALIGN 16
MainLoop:
	movq	mm0,	[edx]	; Backgnd
	movq	mm1,	[eax]	; Src
	movq	mm2,	mm0		; mm2 is Backgnd
	pxor	mm0,	mm7		; mm0 is ~Backgnd
	pxor	mm1,	mm7		; mm1 is ~Src
	pavgb	mm1,	mm0
	pavgb	mm1,	mm0
	pxor	mm1,	mm7
	pavgb	mm1,	mm2
	
	movq	[edx],	mm1
	
	add		eax,	8
	add		edx,	8
	
	; Dec. Loop Counter
	dec		ecx
	jnz		MainLoop
	
	emms					; Empty MMX State
	ret
Mix7To1MMX ENDP

; extern "C" void Mix3To1MMX(void* pBackgnd, void* pSrc, int nSize);
;
; average_round_up(a, b)                               = pavgb(a, b)
; average_round_down(a, b) = ~average_round_up(~a, ~b) = ~pavgb(~a, ~b)
; pavgb is only available on processors with SSE!
;
; round((a*3+b)/4)
; = ((a*3 + b) + 2) >> 2
; = (a*2 + a + b + 2) >> 2
; = (((a + b) >> 1) + a + 1) >> 1
; = average_round_up(average_round_down(a, b), a) 
Mix3To1MMX PROC NEAR
PUBLIC Mix3To1MMX
PublicAlias _Mix3To1MMX	; Underscore needed when called from Windows
	mov     edx, [esp+4]    ; pBackgnd
	mov     eax, [esp+8]    ; pSrc
	mov     ecx, [esp+12]   ; nSize
	pcmpeqb	mm7,	mm7		; mm7 is 0xFFFFFFFFFFFFFFFF
ALIGN 16
MainLoop:
	movq	mm0,	[edx]	; Backgnd
	movq	mm1,	[eax]	; Src
	movq	mm2,	mm0		; mm2 is Backgnd
	pxor	mm0,	mm7		; mm0 is ~Backgnd
	pxor	mm1,	mm7		; mm1 is ~Src
	pavgb	mm1,	mm0
	pxor	mm1,	mm7
	pavgb	mm1,	mm2
	
	movq	[edx],	mm1
	
	add		eax,	8
	add		edx,	8
	
	; Dec. Loop Counter
	dec		ecx
	jnz		MainLoop
	
	emms					; Empty MMX State
	ret
Mix3To1MMX ENDP

; ********** Diff Function **********
; C++ prototype:
; extern "C" void DiffMMX2(void* pDst, void* pSrc1, void* pSrc2, int nSize, void* pMinDiffs);
;
; Note:
; MMX2 (= processors with SSE2) is not a performance gain in this case
; because the 128 bits instructions are split into two 64 bits instructions.
; Unrolling the loop should gain something compared to the same unrolled
; 64 bits version, I tested it there is no gain!
;
;DiffMMX2 PROC NEAR
;PUBLIC DiffMMX2
;PublicAlias _DiffMMX2			; Underscore needed when called from Windows
;	push    ebx
;	mov     ebx,	[esp+4]		; pDst
;	mov     edx,	[esp+12]	; pSrc2
;	mov     ecx,	[esp+16]	; nSize
;	mov     eax,	[esp+20]	; pMinDiffs
;	movapd	xmm3,	[eax]		; xmm3 contains 16 x MinDiff
;	pxor	xmm4,	xmm4		; xmm4 is 0
;	mov     eax,	[esp+8]		; pSrc1
;ALIGN 16
;MainLoop:
;	movapd	xmm0,	[edx]		; Src2
;	movapd	xmm1,	[eax]		; Src1
;	movapd	xmm2,	xmm1		; Duplicate Src1
;	psubusb	xmm1,	xmm0		; Src1 - Src2
;	psubusb xmm0,	xmm2		; Src2 - Src1
;	por		xmm0,	xmm1		; One of both two is 0 -> res = xmm0 = |Src1-Src2|
;
;	movapd	xmm1,	xmm0		; xmm1 contains also the res
;	psubusb	xmm1,	xmm3		; xmm1 = res - nMinDiff
;	pcmpeqb	xmm1,	xmm4		; xmm4 is 0: xmm1 values are 1 if xmm1 are 0
;	pandn	xmm1,	xmm0		; xmm1 = ~mask & xmm0
;
;	movapd	[ebx],	xmm1
;
;	add		eax,	16
;	add		edx,	16
;	add		ebx,	16
;	dec		ecx
;	jnz		MainLoop
;
;	emms						; Empty MMX State
;	pop     ebx
;	ret
;DiffMMX2 ENDP
;
DiffMMX PROC NEAR
PUBLIC DiffMMX
PublicAlias _DiffMMX			; Underscore needed when called from Windows
	push    ebx
	mov     ebx,	[esp+8]		; pDst
	mov     edx,	[esp+16]	; pSrc2
	mov     ecx,	[esp+20]	; nSize
	mov     eax,	[esp+24]	; pMinDiffs
	movq	mm3,	[eax]		; mm3 contains 8 x MinDiff
	pxor	mm4,	mm4			; mm4 is 0
	mov     eax,	[esp+12]	; pSrc1
ALIGN 16
MainLoop:
	movq	mm0,	[edx]		; Src2
	movq	mm1,	[eax]		; Src1
	movq	mm2,	mm1			; Duplicate Src1
	psubusb	mm1,	mm0			; Src1 - Src2
	psubusb mm0,	mm2			; Src2 - Src1
	por		mm0,	mm1			; One of both two is 0 -> res = mm0 = |Src1-Src2|

	movq	mm1,	mm0			; mm1 contains also the res
	psubusb	mm1,	mm3			; mm1 = res - nMinDiff
	pcmpeqb	mm1,	mm4			; mm4 is 0: mm1 values are 1 if mm1 are 0
	pandn	mm1,	mm0			; mm1 = ~mask & mm0
	movq	[ebx],	mm1

	add		eax,	8
	add		edx,	8
	add		ebx,	8
	dec		ecx
	jnz		MainLoop
	
	emms						; Empty MMX State
	pop     ebx
	ret
DiffMMX ENDP


END
