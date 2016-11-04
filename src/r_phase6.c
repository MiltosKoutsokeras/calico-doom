/*
  CALICO

  Renderer phase 6 - Seg Loop 
*/

#include "r_local.h"

static void R_FindPlane(void)
{
   /*
;
; parms
;
fp_check		.equr	r15
fp_height		.equr	r7
fp_picnum		.equr	r6
fp_lightlevel	.equr	r5
fp_start		.equr	r21			; common with sl_x
fp_stop			.equr	r4

;
; locals
;
fp_set			.equr	r10
fp_lastvisplane	.equr	r9
fp_jump			.equr	r8

	movei	#_lastvisplane,fp_lastvisplane
	load	(fp_lastvisplane),fp_lastvisplane

	movei	#L105,fp_jump
	
	movei	#L104,r0
	jump	T,(r0)
	nop

L101:

	load	(fp_check),r0
	cmp		fp_height,r0
	jump	NE,(fp_jump)
	nop
	load	(fp_check+1),r0
	cmp		fp_picnum,r0
	jump	NE,(fp_jump)
	nop
	load	(fp_check+2),r0
	cmp		fp_lightlevel,r0
	jump	NE,(fp_jump)
	nop
 
; if check->open[start]
	move	fp_start,r0
	shlq	#1,r0
	add		fp_check,r0
	addq	#24,r0
	loadw	(r0),r0	
	movei	#65280,r1
	cmp		r0,r1
	jump	NE,(fp_jump)
	nop

; found a plane, so adjust bounds and return it
	load	(fp_check+3),r0
	cmp		r0,fp_start
	jr		U_LE,startok
	nop
	store	fp_start,(fp_check+3)
startok:
	load	(fp_check+4),r0
	cmp		r0,fp_stop
	jr		U_GT,stopok
	nop
	store	fp_stop,(fp_check+4)
stopok:
; return check
	jump	T,(RETURNPOINT)
	move	fp_check,RETURNVALUE


;
; next plane
;
L105:
	movei	#348,r0
	add		r0,fp_check

L104:
	cmp		fp_check,fp_lastvisplane
	movei	#L101,scratch
	jump	U_LT,(scratch)
	nop

;
; make a new plane
;
	move	fp_lastvisplane,fp_check
	movei	#348,r3
	add		r3,fp_lastvisplane
	movei	#_lastvisplane,scratch
	store	fp_lastvisplane,(scratch)			; visplane++
 
	store	fp_height,(fp_check)
	store	fp_picnum,(fp_check+1)
	store	fp_lightlevel,(fp_check+2)
	store	fp_start,(fp_check+3)
	store	fp_stop,(fp_check+4)

;
; for i=0 ; i<80 ; i++ 
; (int)check->open[i] = 0xff00ff00
;
	move	fp_check,fp_set
	addq	#24,fp_set
	
	movei	#$ff00ff00,r0
	movei	#40,r1
		
fillloop:
	store	r0,(fp_set)
	addq	#4,fp_set
	store	r0,(fp_set)
	subq	#1,r1
	addqt	#4,fp_set
	jr		NE,fillloop
	nop

	jump T,(RETURNPOINT)
	move fp_check,RETURNVALUE
   */
}

static void R_DrawTexture(void)
{
   // CALICO_TODO
   /*

dt_tex		.equr	r15
dt_blitter	.equr	r15

dt_top		.equr	r16
dt_bottom	.equr	r17
dt_colnum	.equr	r18
dt_frac		.equr	r5
dt_centery	.equr	r7

dt_scratch	.equr	r10
dt_scratch2	.equr	r11
dt_MATHA	.equr	r0
dt_MATHB	.equr	r1

dtb_base	.equr	r0
dtb_a1pix	.equr	r1
dtb_a1frac	.equr	r2
dtb_a1inc	.equr	r3
dtb_a1incfrac .equr r4

dtb_count	.equr	r7
dtb_command	.equr	r8

;
; common to draw texture
;
dt_floorclipx	.equr	r19
dt_ceilingclipx	.equr	r20
dt_x			.equr	r21
dt_scale		.equr	r22

dt_iscale		.equr	r23
dt_texturecol	.equr	r24
dt_texturelight	.equr	r25


	load	(dt_tex+3),dt_scratch		; tex->topheight
	imult	dt_scale,dt_scratch
	sharq	#15,dt_scratch
	movei	#90,dt_centery
	move	dt_centery,dt_top
	sub		dt_scratch,dt_top		;top=CENTERY-((scale*tex->topheight)>>(15))
	
	cmp		dt_top,dt_ceilingclipx
	jr		S_GT,dt_topok		; if (top > ceilingclipx)
	nop
	move	dt_ceilingclipx,dt_top
	addq	#1,dt_top
dt_topok:

	load	(dt_tex+4),dt_scratch		; tex->bottomheight
	imult	dt_scale,dt_scratch
	sharq	#15,dt_scratch
	move	dt_centery,dt_bottom
	subq	#1,dt_bottom
	sub		dt_scratch,dt_bottom	;bottom=CENTERY-1-((scale*tex->botheight)>>15)
	
	cmp		dt_bottom,dt_floorclipx
	jr		S_LT,dt_bottomok		; if (bottom < clipbottom)
	nop
	move	dt_floorclipx,dt_bottom
	subqt	#1,dt_bottom
 dt_bottomok:

	cmp		dt_top,dt_bottom
	jump	S_GT,(RETURNPOINT)	; if (top>bottom) return
	nop

;===========

	move	dt_texturecol,dt_colnum
	
;
; frac = tex->texturemid - (CENTERY-top)*tx_iscale
;
	move	dt_centery,dt_MATHB
	sub		dt_top,dt_MATHB
	move	dt_MATHB,dt_scratch
	move	dt_iscale,dt_MATHA
	abs		dt_MATHB
	move	dt_MATHA,dt_scratch2
	shrq	#16,dt_scratch2
	mult	dt_MATHB,dt_MATHA
	mult	dt_MATHB,dt_scratch2
	shlq	#16,dt_scratch2
	btst	#31,dt_scratch
	jr		EQ,dt_notneg
	add		dt_scratch2,dt_MATHA			; delay slot
	neg		dt_MATHA
dt_notneg:

	load	(dt_tex+5),dt_frac		; tex->texturemid
	sub		dt_MATHA,dt_frac

; while frac < 0 colnum--,frac += tex->height<<16
	jr		PL,fracpos
	nop
subagain:
	load	(dt_tex+2),dt_scratch		; tex->height
	shlq	#16,dt_scratch
	add		dt_scratch,dt_frac
	jr		MI,subagain
	subq	#1,dt_colnum
fracpos:

; colnum = colnum - tex->width*(colnum/tex->width)
	load	(dt_tex+1),dt_scratch		; tex->width
	subq	#1,scratch
	and		scratch,dt_colnum
	
; a1inc
	move	dt_iscale,dtb_a1inc
	shrq	#16,dtb_a1inc
	
; a1incfrac
	movei	#$ffff,dtb_a1incfrac
	and		dt_iscale,dtb_a1incfrac

; count
	move	dt_bottom,dtb_count
	sub		dt_top,dtb_count
	addq	#1,dtb_count
	shlq	#16,dtb_count
	addq	#1,dtb_count
	
; a2pix
	shlq	#16,dt_top
	or		dt_x,dt_top				; screen x

;
;frac += (colnum*tex->height)<<16
;
	load	(dt_tex+2),dt_scratch		; tex->height
	mult	dt_colnum,dt_scratch
	shlq	#16,dt_scratch
	add		dt_scratch,dt_frac

; a1pix
	move	dt_frac,dtb_a1pix
	shrq	#16,dtb_a1pix
	
; a1pix frac
	movei	#$ffff,dtb_a1frac
	and		dt_frac,dtb_a1frac
	
;===========
	
; base
	load	(dt_tex),dtb_base		; dt->data
			
; command
	movei	#1+(1<<8)+(1<<9)+(1<<10)+(1<<11)+(1<<13)+(1<<30)+(12<<21),dtb_command
	
	movei	#$f02200,dt_blitter
	movei	#$f02270,dt_scratch2		; iinc blitter register
	 
dt_wait:
	load	(dt_blitter+14),dt_scratch
	btst	#0,dt_scratch
	jr		EQ,dt_wait
	nop
		
	store	dtb_base,(dt_blitter)
	store	dtb_a1pix,(dt_blitter+3)
	store	dtb_a1frac,(dt_blitter+6)
	store	dtb_a1inc,(dt_blitter+4)
	store	dtb_a1incfrac,(dt_blitter+5)
	store	dt_top,(dt_blitter+12)
	store	dt_texturelight,(dt_scratch2)
	store	dtb_count,(dt_blitter+15)
	jump	T,(RETURNPOINT)
	store	dtb_command,(dt_blitter+14)		; delay slot
   */
}

static void R_SegLoop(void)
{
   // CALICO_TODO
   /*
BIT_ADDFLOOR	.equ	0
BIT_ADDCEILING	.equ	1
BIT_TOPTEXTURE	.equ	2
BIT_BOTTOMTEXTURE .equ	3
BIT_NEWCEILING	.equ	4
BIT_NEWFLOOR	.equ	5
BIT_ADDSKY		.equ	6
BIT_CALCTEXTURE	.equ	7
BIT_TOPSIL		.equ	8
BIT_BOTTOMSIL	.equ	9
BIT_SOLIDSIL	.equ	10

; r15 stays free for subfunctions to use

sl_colnum		.equr	r16
sl_top			.equr	r17
sl_bottom		.equr	r18
sl_actionbits	.equr	r9

sl_high			.equr	r30
sl_low			.equr	r31

;
; common to draw texture
;
sl_floorclipx	.equr	r19
sl_ceilingclipx	.equr	r20
sl_x			.equr	r21		; also common to find plane
sl_scale		.equr	r22

sl_iscale		.equr	r23
sl_texturecol	.equr	r24
sl_texturelight	.equr	r25


	movei	#96,scratch
	sub		scratch,FP

	store	RETURNPOINT,(FP+10) ;	only store once
	movefa	VR_actionbits,sl_actionbits
	movefa	VR_scalefrac,r1
	movefa	VR_start,sl_x
	btst	#0,r1				; scoreboard bug
	store	r1,(FP+6)			; scalefrac
	
;
; force R_FindPlane for both planes
;	
	movei	#_visplanes,r1
	store	r1,(FP+8)	; &ceiling
	store	r1,(FP+7)	; &floor

	movei	#L121,r0
	jump	T,(r0)
	nop

L118:

	load	(FP+6),r1	; scalefrac
	move	r1,sl_scale
	sharq	#7,sl_scale
 	movefa	VR_scalestep,r2
	add		r2,r1
	movei	#$7fff,scratch
	store	r1,(FP+6)
	cmp		scratch,sl_scale
	jr		U_GT,scaleok			; if scale>0x7fff scale = 0x7fff
	nop
	move	scratch,sl_scale	
scaleok:

L125:
;
; get ceilingclipx and floorclipx from clipbounds
;
	move	sl_x,r0 ;(x)
	shlq	#2,r0
	movei	#_clipbounds,r1
	add		r1,r0
	load	(r0),sl_floorclipx
	move	sl_floorclipx,sl_ceilingclipx
	shrq	#8,sl_ceilingclipx
	shlq	#24,sl_floorclipx
	subq	#1,sl_ceilingclipx
	shrq	#24,sl_floorclipx		; mask off top 24 bits

;
; texture only stuff
;
	btst	#BIT_CALCTEXTURE,sl_actionbits
	movei	#L129,scratch
	jump	EQ,(scratch)
	nop

;
; calculate texture offset
;
	
 	movefa	VR_centerangle,r1
	move	sl_x,r3 ;(x)
	shlq	#2,r3

	movei	#_xtoviewangle,r4
	add		r4,r3
	load	(r3),r3
	add		r3,r1
	shrq	#19,r1
	shlq	#2,r1
	movei	#_finetangent,r0
	add		r1,r0
	
	load	(r0),MATH_A
 	movefa	VR_distance,MATH_B
;---------------------------------------
;========== FixedMul r0,
	move    MATH_A,MATH_SIGN
	xor     MATH_B,MATH_SIGN
	abs     MATH_A
	abs     MATH_B
	move    MATH_A,RETURNVALUE
	mult    MATH_B,RETURNVALUE              ; al*bl
	shrq    #16,RETURNVALUE
	move    MATH_B,scratch2
	shrq    #16,scratch2
	mult    MATH_A,scratch2                 ; al*bh
	add     scratch2,RETURNVALUE
	move    MATH_A,scratch2
	shrq    #16,scratch2
	mult    MATH_B,scratch2                 ; bl*ah
	add     scratch2, RETURNVALUE
	move    MATH_A,scratch2
	shrq    #16,scratch2
	move    MATH_B,scratch
	shrq    #16,scratch
	mult    scratch,scratch2                ; bh*ah
	shlq    #16,scratch2
	add     scratch2, RETURNVALUE
	btst    #31,MATH_SIGN
	jr   	EQ,notneg
	nop
	neg     RETURNVALUE
notneg:
;---------------------------------------
 	movefa	VR_offset,sl_texturecol
	sub RETURNVALUE,sl_texturecol
	shrq #16,sl_texturecol

;
; other texture drawing info
;
	movei #33554432,sl_iscale
	div	sl_scale,sl_iscale			; let this div complete in background

;
; calc light level
;
	movei	#_lightcoef,r1
	load	(r1),r1
	mult	sl_scale,r1
	shrq	#16,r1
	movei	#_lightsub,r2
	load	(r2),r2
	sub		r2,r1
 
	movei	#_lightmin,r0
	load	(r0),r0
	cmp		r0,r1
	jr		S_LT,lightovermin
	nop
	move	r0,r1 
lightovermin:
	movei	#_lightmax,r0
	load	(r0),r0
	cmp		r0,r1
	jr		S_GT,lightundermax
	nop
	move	r0,r1 
lightundermax:

; convert to a hardware value
	movei	#255,r0
	sub		r1,r0
	shlq	#14,r0
	neg		r0
	movei	#$ffffff,sl_texturelight
	and		r0,sl_texturelight

;
; draw textures
;
	btst	#BIT_TOPTEXTURE,sl_actionbits
	jr		EQ,L137
	btst	#BIT_BOTTOMTEXTURE,sl_actionbits	; harmless delay slot

	movei	#_toptex,dt_tex	; parameter
	movei	#_R_DrawTexture,r0
	move	PC,RETURNPOINT
	jump	T,(r0)
	addq	#6,RETURNPOINT
	movefa	VR_actionbits,sl_actionbits

	btst	#BIT_BOTTOMTEXTURE,sl_actionbits
L137:
	jr		EQ,L139
	nop

	movei	#_bottomtex,dt_tex	; parameter
	movei	#_R_DrawTexture,r0
	move	PC,RETURNPOINT
	jump	T,(r0)
	addq	#6,RETURNPOINT
	movefa	VR_actionbits,sl_actionbits

L139:

L129:
;-----------------------
;
; floor
;
;-----------------------

	btst	#BIT_ADDFLOOR,sl_actionbits
	movei	#L143,scratch
	jump	EQ,(scratch)
	nop
	
 	movefa	VR_floorheight,r0
	imult	sl_scale,r0
	sharq	#15,r0
	movei 	#90,sl_top
	sub 	r0,sl_top
	cmp 	sl_top,sl_ceilingclipx ;(top)(ceilingclipx)
	jr		S_GT,nocliptop
	nop
	move	sl_ceilingclipx,sl_top
	addq	#1,sl_top
nocliptop:
	move	sl_floorclipx,sl_bottom
	subq	#1,sl_bottom

	cmp 	sl_top,sl_bottom ;(top)(bottom)
	movei 	#L147,scratch
	jump 	S_GT,(scratch)
	nop

	move	sl_x,r0 ;(x)
	load	(FP+7),r3 ; local floor
	shlq	#1,r0
	addq	#24,r0
	add		r3,r0
	loadw	(r0),r0		; floor->open[x]
	movei	#65280,r1
	cmp		r0,r1
	movei	#L149,scratch
	jump	EQ,(scratch)
	nop

 	movefa	VR_floorheight,fp_height
 	movefa	VR_floorpic,fp_picnum
 	movefa	VR_seglightlevel,fp_lightlevel
 	movefa	VR_stop,fp_stop
	movei	#348,fp_check
	add		r3,fp_check			; fp_check = floor+1

	movei	#_R_FindPlane,r1
 	move	PC,RETURNPOINT
	jump	T,(r1)
	addq	#6,RETURNPOINT
	movefa	VR_actionbits,sl_actionbits
 
	store	RETURNVALUE,(FP+7)	; floor

L149:

	move	sl_x,r0 ;(x)
	shlq	#1,r0
	load	(FP+7),r1 ; local floor
	addq	#24,r1
	add		r1,r0
	move	sl_top,r1 ;(top)
	shlq	#8,r1
	add		sl_bottom,r1 ;(bottom)
	storew	r1,(r0)				; floor->open[x] = (top<<8)+bottom

L147:

L143:

;-----------------------
;
; ceiling
;
;-----------------------
	btst	#BIT_ADDCEILING,sl_actionbits
	movei	#L157,scratch
	jump	EQ,(scratch)
	nop
	
 	move	sl_ceilingclipx,sl_top
	addq	#1,sl_top			; top = ceilingclipx+1
	
	movei	#89,sl_bottom
 	movefa	VR_ceilingheight,r1
	imult	sl_scale,r1
	sharq	#15,r1
	sub		r1,sl_bottom
	
	cmp		sl_bottom,sl_floorclipx ;(bottom)(floorclipx)
	jr		S_LT,noclipbottom
	nop
	move	sl_floorclipx,sl_bottom
	subq	#1,sl_bottom
noclipbottom:		

	cmp		sl_top,sl_bottom ;(top)(bottom)
	movei	#L161,scratch
	jump	S_GT,(scratch)
	nop

	move	sl_x,r0 ;(x)
	shlq	#1,r0
	load	(FP+8),r3 ; local ceiling
	addq	#24,r0
	add		r3,r0
	loadw	(r0),r0			; ceiling->open[x]
	movei	#65280,r1
	cmp		r0,r1
	movei	#L163,scratch
	jump	EQ,(scratch)
	nop

 	movefa	VR_ceilingheight,fp_height
 	movefa	VR_ceilingpic,fp_picnum
 	movefa	VR_seglightlevel,fp_lightlevel
 	movefa	VR_stop,fp_stop
	movei	#348,fp_check
	add		r3,fp_check			; fp_check = ceiling+1

	movei	#_R_FindPlane,r1
 	move	PC,RETURNPOINT
	jump	T,(r1)
 	addq	#6,RETURNPOINT
	movefa	VR_actionbits,sl_actionbits
 
	store	RETURNVALUE,(FP+8)	; ceiling

L163:

	move	sl_x,r0 ;(x)
	shlq	#1,r0
	load	(FP+8),r1 ; local ceiling
	addq	#24,r1
	add		r1,r0
	move	sl_top,r1 ;(top)
	shlq	#8,r1
	add		sl_bottom,r1 ;(bottom)
	storew	r1,(r0)

L161:

L157:

;------------------------
;
; calc high and low
;
;------------------------
 	movefa	VR_floornewheight,r0
	imult	sl_scale,r0
	sharq	#15,r0
	movei	#90,sl_low
	sub		r0,sl_low			; low = CENTERY-(scale*wl.floornewheight)>>15
	jr		PL,lownotneg
	nop
	moveq	#0,sl_low
lownotneg:
	cmp		sl_low,sl_floorclipx
	jr		S_LT,lowless
	nop
	move	sl_floorclipx,sl_low
lowless:

 	movefa	VR_ceilingnewheight,r0
	imult	sl_scale,r0
	sharq	#15,r0
	movei	#89,sl_high
	sub		r0,sl_high			; high = CENTERY-(scale*wl.ceilinewheight)>>15
	movei	#179,r0
	cmp		r0,sl_high
	jr		S_GT,highabove
	nop
	move	r0,sl_high
highabove:
	cmp		sl_high,sl_ceilingclipx
	jr		S_GT,highcheck2
	nop
	move	sl_ceilingclipx,sl_high
highcheck2:

;
; bottom sprite clip sil
;
	btst #BIT_BOTTOMSIL,sl_actionbits
	jr	EQ,nobottomsil
	nop	

 	movefa	VR_bottomsil,r1
	add		sl_x,r1
	storeb sl_low,(r1)

nobottomsil:

;
; top sprite clip sil
;
	btst	#BIT_TOPSIL,sl_actionbits
	jr		EQ,notopsil
	nop	

 	movefa 	VR_topsil,r1
	add		sl_x,r1
	move	sl_high,r0
	addq	#1,r0
	storeb	r0,(r1)

notopsil:
	
;--------------------------------------------------------------
; sky mapping
	
	btst #BIT_ADDSKY,sl_actionbits
	movei #L190,scratch
	jump EQ,(scratch)
	nop

L187:

	move	sl_ceilingclipx,sl_top
	addq	#1,sl_top
	movei	#90,r0
 	movefa	VR_ceilingheight,r1
	move sl_scale,r2
	imult	r1,r2
L228:

 move r2,r1
 sharq #15,r1
 sub r1,r0
 subq #1,r0
 move r0,sl_bottom ;(bottom)
 cmp sl_bottom,sl_floorclipx ;(bottom)(floorclipx)
 movei #L192,scratch
 jump S_LT,(scratch)
 nop

 move sl_floorclipx,r0 ;(floorclipx)
 subq #1,r0
 move r0,sl_bottom ;(bottom)

L192:

 cmp sl_top,sl_bottom ;(top)(bottom)
 movei #L194,scratch
 jump PL,(scratch)
 nop

 movei #L190,r0
 jump T,(r0)
 nop

L194:

 movei #_viewangle,r0
 load (r0),r0
 move sl_x,r1 ;(x)
 shlq #2,r1
 movei #_xtoviewangle,r2
 add r2,r1
 load (r1),r1
 add r1,r0
 shrq #22,r0
 move r0,sl_colnum ;(colnum)
 movei #255,r0
 move sl_colnum,r1 ;(colnum)
 and r0,r1
 move r1,sl_colnum ;(colnum)


	shlq	#21,sl_colnum
 
	movei	#18204,r1
	imult	sl_top,r1
	add		r1,sl_colnum
 
	shlq	#2,sl_colnum
 
 
 movei #36,r0
 add FP,r0 ; &count
 move sl_bottom,r1 ;(bottom)
 sub sl_top,r1 ;(top)
 moveq #1,r2
 add r2,r1
 shlq #16,r1
 add r2,r1
 store r1,(r0)

L196:

L197:

 movei #15737400,r0
 load (r0),r0
 moveq #1,r1
 and r1,r0
 moveq #0,r1
 cmp r0,r1
 movei #L196,scratch
 jump EQ,(scratch)
 nop

 movei #15737344,r0
 movei #_skytexturep,r1
 load (r1),r1
 addq #16,r1
 load (r1),r1
 store r1,(r0)

 movei #15737356,r0
 move sl_colnum,r1 ;(colnum)
 shrq #16,r1
 store r1,(r0)

 movei #15737368,r0
 movei #65535,r1
 move sl_colnum,r2 ;(colnum)
 and r1,r2
 move r2,r1
 store r1,(r0)

 movei #15737360,r0
 moveq #1,r1
 store r1,(r0)

 movei #15737364,r0
 movei #7281,r1
 store r1,(r0)

 movei #15737392,r0
 move sl_top,r1 ;(top)
 shlq #16,r1
 add sl_x,r1 ;(x)
 store r1,(r0)

 movei #15737456,r0
 moveq #0,r1
 store r1,(r0)

 movei #15737404,r0
 load (FP+9),r1 ; local count
 store r1,(r0)

 movei #15737400,r0
 movei #1098919681,r1
 store r1,(r0)
	
L190:
;--------------------------------------------------------------

;if (!(actionbits & (AC_NEWFLOOR|AC_NEWCEILING)))
;	continue;	// don't bother rewriting clipbounds[x]

	movei	#48,r0
	and		sl_actionbits,r0
	movei	#L119,scratch
	jump	EQ,(scratch)
	nop

	btst	#BIT_NEWFLOOR,sl_actionbits
	jr		EQ,nonewfloor
	nop	
	move	sl_low,sl_floorclipx
	
nonewfloor:

	btst	#BIT_NEWCEILING,sl_actionbits
	jr		EQ,nonewceiling
	nop
	move	sl_high,sl_ceilingclipx
	
nonewceiling:

;clipbounds[x] = ((ceilingclipx+1)<<8) + floorclipx;

	move	sl_x,r0 ;(x)
	shlq	#2,r0
	movei	#_clipbounds,r1
	add		r1,r0
	addq	#1,sl_ceilingclipx
	shlq	#8,sl_ceilingclipx
	add		sl_floorclipx,sl_ceilingclipx
	store	sl_ceilingclipx,(r0)
L119:
	
;--------------
;
; next
;
;--------------
	addq	#1,sl_x

L121:

 	movefa	VR_stop,r0
	cmp		sl_x,r0 ;(x)
	movei	#L118,scratch
	jump	S_LE,(scratch)
	nop

	load	(FP+10),RETURNPOINT
	movei	#96,scratch
	jump	T,(RETURNPOINT)
	add		scratch,FP ; delay slot
   */
}

void R_SegCommands(void)
{
   // CALICO_TODO: R_SegCommands
   /*
 subq #32,FP

	movei	#_clipbounds,r0
	movei	#180,r1		; SCREENHEIGHT
	movei	#160,r2		; SCREENWIDTH
	movei	#clearclipbounds,r3
	
clearclipbounds:
	store	r1,(r0)
	addq	#4,r0
	store	r1,(r0)
	addq	#4,r0
	store	r1,(r0)
	addq	#4,r0
	subq	#4,r2
	store	r1,(r0)
	jump	NE,(r3)
	addq	#4,r0
	


;
; setup blitter
;

 movei #15737348,r0
 movei #145440,r1
 store r1,(r0)

 movei #15737384,r0
 movei #145952,r1
 store r1,(r0)

 movei #_viswalls,r0
 move r0,r16 ;(segl)

 movei #L61,r0
 jump T,(r0)
 nop

L58:

;
; copy viswall to local memory
;
	move	r16,r15
	
	load	(r15+VS_start),r0
	moveta	r0,VR_start
	load	(r15+VS_stop),r0
	moveta	r0,VR_stop
	load	(r15+VS_floorpic),r0
	moveta	r0,VR_floorpic
	load	(r15+VS_ceilingpic),r0
	moveta	r0,VR_ceilingpic
	load	(r15+VS_actionbits),r0
	moveta	r0,VR_actionbits
	load	(r15+VS_floorheight),r0
	moveta	r0,VR_floorheight
	load	(r15+VS_floornewheight),r0
	moveta	r0,VR_floornewheight
	load	(r15+VS_ceilingheight),r0
	moveta	r0,VR_ceilingheight
	load	(r15+VS_ceilingnewheight),r0
	moveta	r0,VR_ceilingnewheight
	load	(r15+VS_topsil),r0
	moveta	r0,VR_topsil
	load	(r15+VS_bottomsil),r0
	moveta	r0,VR_bottomsil
	load	(r15+VS_scalefrac),r0
	moveta	r0,VR_scalefrac
	load	(r15+VS_scalestep),r0
	moveta	r0,VR_scalestep
	load	(r15+VS_centerangle),r0
	moveta	r0,VR_centerangle
	load	(r15+VS_offset),r0
	moveta	r0,VR_offset
	load	(r15+VS_distance),r0
	moveta	r0,VR_distance
	load	(r15+VS_seglightlevel),r0
	moveta	r0,VR_seglightlevel
	

;		lightmin = wl.seglightlevel - (255-wl.seglightlevel)*2;
;		if (lightmin < 0)
;			lightmin = 0;
sc_lightmin		.equr	r5
sc_lightmax		.equr	r6

	movefa	VR_seglightlevel,sc_lightmin
	movei	#255,r2
	movefa	VR_seglightlevel,r3
	sub		r3,r2
	shlq	#1,r2
	sub		r2,sc_lightmin
	jr		PL,minnotneg
	nop
	moveq	#0,sc_lightmin
minnotneg:
	movei	#_lightmin,r0
	store	sc_lightmin,(r0)
	
;	lightmax = wl.seglightlevel;

	movei	#_lightmax,r0
	movefa	VR_seglightlevel,sc_lightmax
	store	sc_lightmax,(r0)

; lightsub = 160*(lightmax-lightmin)/(800-160);
; lightcoef = ((lightmax-lightmin)<<16)/(800-160);

	sub		sc_lightmin,sc_lightmax
	move	sc_lightmax,r1
	movei	#160,r2
	mult	r1,r2
	movei	#640,r4
	div		r4,r2
	movei	#_lightsub,r0

	move	sc_lightmax,r3
	shlq	#16,r3
	store	r2,(r0)				; div hit
	div		r4,r3
	
	movei	#_lightcoef,r0
	store	r3,(r0)				; div hit
	

 	movefa	VR_actionbits,r0
	btst	#2,r0
	movei	#L71,scratch
	jump	EQ,(scratch)
	nop

 movei #_toptex+12,r0
 load (r15+7),r1
 store r1,(r0)

 movei #_toptex+16,r0
 load (r15+8),r1
 store r1,(r0)

 movei #_toptex+20,r0
 load (r15+9),r1
 store r1,(r0)

 load (r15+10),r0
 move r0,r17 ;(tex)
 movei #_toptex+4,r0
 move r17,r1 ;(tex)
 addq #8,r1
 load (r1),r1
 store r1,(r0)

 movei #_toptex+8,r0
 move r17,r1 ;(tex)
 addq #12,r1
 load (r1),r1
 store r1,(r0)

 movei #_toptex,r0
 move r17,r1 ;(tex)
 addq #16,r1
 load (r1),r1
 store r1,(r0)

L71:

	movefa	VR_actionbits,r0
	btst	#3,r0
	movei	#L83,scratch
	jump	EQ,(scratch)
	nop

 movei #_bottomtex+12,r0
 load (r15+11),r1
 store r1,(r0)

 movei #_bottomtex+16,r0
 load (r15+12),r1
 store r1,(r0)

 movei #_bottomtex+20,r0
 load (r15+13),r1
 store r1,(r0)

 load (r15+14),r0
 move r0,r17 ;(tex)
 movei #_bottomtex+4,r0
 move r17,r1 ;(tex)
 addq #8,r1
 load (r1),r1
 store r1,(r0)

 movei #_bottomtex+8,r0
 move r17,r1 ;(tex)
 addq #12,r1
 load (r1),r1
 store r1,(r0)

 movei #_bottomtex,r0
 move r17,r1 ;(tex)
 addq #16,r1
 load (r1),r1
 store r1,(r0)

L83:

 movei #_R_SegLoop,r0
 store r28,(FP) ; psuh ;(RETURNPOINT)
 store r17,(FP+1) ; push ;(tex)
 store r16,(FP+2) ; push ;(segl)
 movei #L99,RETURNPOINT
 jump T,(r0)
 store r15,(FP+3) ; delay slot push ;(i)
L99:
 load (FP+1),r17 ; pop ;(tex)
 load (FP+2),r16 ; pop ;(segl)
 load (FP+3),r15 ; pop ;(i)
 load (FP),RETURNPOINT ; pop

L59:

 movei #112,r0
 move r16,r1 ;(segl)
 add r0,r1
 move r1,r16 ;(segl)

L61:

 move r16,r0 ;(segl)
 movei #_lastwallcmd,r1
 load (r1),r1
 cmp r0,r1
 movei #L58,scratch
 jump U_LT,(scratch)
 nop

 movei #_phasetime+24,r0
 movei #_samplecount,r1
 load (r1),r1
 store r1,(r0)

 movei #_gpucodestart,r0
 movei #_ref7_start,r1
 store r1,(r0)


L53:
 jump T,(RETURNPOINT)
 addq #32,FP ; delay slot
   */
}

// EOF

/*
#define	AC_ADDFLOOR      1       000:00000001
#define	AC_ADDCEILING    2       000:00000010
#define	AC_TOPTEXTURE    4       000:00000100
#define	AC_BOTTOMTEXTURE 8       000:00001000
#define	AC_NEWCEILING    16      000:00010000
#define	AC_NEWFLOOR      32      000:00100000
#define	AC_ADDSKY        64      000:01000000
#define	AC_CALCTEXTURE   128     000:10000000
#define	AC_TOPSIL        256     001:00000000
#define	AC_BOTTOMSIL     512     010:00000000
#define	AC_SOLIDSIL      1024    100:00000000

typedef struct
{
0:  int  filepos; // also texture_t * for comp lumps
4:  int  size;
8:  char name[8];
} lumpinfo_t;

typedef struct
{
   // filled in by bsp
  0:   seg_t        *seg;
  4:   int           start;
  8:   int           stop;   // inclusive x coordinates
 12:   int           angle1; // polar angle to start

   // filled in by late prep
 16:   pixel_t      *floorpic;
 20:   pixel_t      *ceilingpic;

   // filled in by early prep
 24:   unsigned int  actionbits;
 28:   int           t_topheight;
 32:   int           t_bottomheight;
 36:   int           t_texturemid;
 40:   texture_t    *t_texture;
 44:   int           b_topheight;
 48:   int           b_bottomheight;
 52:   int           b_texturemid;
 56:   texture_t    *b_texture;
 60:   int           floorheight;
 64:   int           floornewheight;
 68:   int           ceilingheight;
 72:   int           ceilingnewheight;
 76:   byte         *topsil;
 80:   byte         *bottomsil;
 84:   unsigned int  scalefrac;
 88:   unsigned int  scale2;
 92:   int           scalestep;
 96:   unsigned int  centerangle;
100:   unsigned int  offset;
104:   unsigned int  distance;
108:   unsigned int  seglightlevel;
} viswall_t;

typedef struct seg_s
{
 0:   vertex_t *v1;
 4:   vertex_t *v2;
 8:   fixed_t   offset;
12:   angle_t   angle;       // this is not used (keep for padding)
16:   side_t   *sidedef;
20:   line_t   *linedef;
24:   sector_t *frontsector;
28:   sector_t *backsector;  // NULL for one sided lines
} seg_t;

typedef struct line_s
{
 0: vertex_t     *v1;
 4: vertex_t     *v2;
 8: fixed_t      dx;
12: fixed_t      dy;                    // v2 - v1 for side checking
16: VINT         flags;
20: VINT         special;
24: VINT         tag;
28: VINT         sidenum[2];               // sidenum[1] will be -1 if one sided
36: fixed_t      bbox[4];
52: slopetype_t  slopetype;                // to aid move clipping
56: sector_t    *frontsector;
60: sector_t    *backsector;
64: int          validcount;               // if == validcount, already checked
68: void        *specialdata;              // thinker_t for reversable actions
72: int          fineangle;                // to get sine / cosine for sliding
} line_t;

typedef struct
{
 0:  fixed_t   textureoffset; // add this to the calculated texture col
 4:  fixed_t   rowoffset;     // add this to the calculated texture top
 8:  VINT      toptexture;
12:  VINT      bottomtexture;
16:  VINT      midtexture;
20:  sector_t *sector;
} side_t;

typedef	struct
{
 0:   fixed_t floorheight;
 4:   fixed_t ceilingheight;
 8:   VINT    floorpic;
12:   VINT    ceilingpic;        // if ceilingpic == -1,draw sky
16:   VINT    lightlevel;
20:   VINT    special;
24:   VINT    tag;
28:   VINT    soundtraversed;              // 0 = untraversed, 1,2 = sndlines -1
32:   mobj_t *soundtarget;                 // thing that made a sound (or null)
36:   VINT        blockbox[4];             // mapblock bounding box for height changes
52:   degenmobj_t soundorg;                // for any sounds played by the sector
76:   int     validcount;                  // if == validcount, already checked
80:   mobj_t *thinglist;                   // list of mobjs in sector
84:   void   *specialdata;                 // thinker_t for reversable actions
88:   VINT    linecount;
92:   struct line_s **lines;               // [linecount] size
} sector_t;

typedef struct subsector_s
{
0:   sector_t *sector;
4:   VINT      numlines;
8:   VINT      firstline;
} subsector_t;

typedef struct
{
 0:  char     name[8];  // for switch changing, etc
 8:  int      width;
12:  int      height;
16:  pixel_t *data;     // cached data to draw from
20:  int      lumpnum;
24:  int      usecount; // for precaching
28:  int      pad;
} texture_t;

typedef struct mobj_s
{
  0: struct mobj_s *prev;
  4: struct mobj_s *next;
  8: latecall_t     latecall;
 12: fixed_t        x;
 16: fixed_t        y;
 20: fixed_t        z;
 24: struct mobj_s *snext;
 28: struct mobj_s *sprev;
 32: angle_t        angle;
 36: VINT           sprite;
 40: VINT           frame;
 44: struct mobj_s *bnext;
 48: struct mobj_s *bprev;
 52: struct subsector_s *subsector;
 56: fixed_t        floorz;
 60: fixed_t        ceilingz;
 64: fixed_t        radius;
 68: fixed_t        height;
 72: fixed_t        momx;
 76: fixed_t        momy;
 80: fixed_t        momz;
 84: mobjtype_t     type;
 88: mobjinfo_t    *info;
 92: VINT           tics;
 96: state_t       *state;
100: int            flags;
104: VINT           health;
108: VINT           movedir;
112: VINT           movecount;
116: struct mobj_s *target;
120: VINT           reactiontime;
124: VINT           threshold;
132: struct player_s *player;
136: struct line_s *extradata;
140: short spawnx;
142: short spawny;
144: short spawntype;
146: short spawnangle;
} mobj_t;

typedef struct vissprite_s
{
 0:  int     x1;
 4:  int     x2;
 8:  fixed_t startfrac;
12:  fixed_t xscale;
16:  fixed_t xiscale;
20:  fixed_t yscale;
24:  fixed_t yiscale;
28:  fixed_t texturemid;
32:  patch_t *patch;
36:  int     colormap;
40:  fixed_t gx;
44:  fixed_t gy;
48:  fixed_t gz;
52:  fixed_t gzt;
56:  pixel_t *pixels;
} vissprite_t;

typedef struct 
{ 
0:  short width;
2:  short height;
4:  short leftoffset;
6:  short topoffset;
8:  unsigned short columnofs[8];
} patch_t; 

typedef struct
{
 0:  state_t *state;
 4:  int      tics;
 8:  fixed_t  sx;
12:  fixed_t  sy;
} pspdef_t;

typedef struct
{
 0:  spritenum_t sprite;
 4:  long        frame;
 8:  long        tics;
12:  void        (*action)();
16:  statenum_t  nextstate;
20:  long        misc1;
24:  long        misc2;
} state_t;

typedef struct
{
 0:  boolean rotate;  // if false use 0 for any position
 4:  int     lump[8]; // lump to use for view angles 0-7
36:  byte    flip[8]; // flip (1 = flip) to use for view angles 0-7
} spriteframe_t;

typedef struct
{
0: int            numframes;
4: spriteframe_t *spriteframes;
} spritedef_t;

typedef struct memblock_s
{
 0: int     size; // including the header and possibly tiny fragments
 4: void  **user; // NULL if a free block
 8: short   tag;  // purgelevel
10: short   id;   // should be ZONEID
12: int     lockframe; // don't purge on the same frame
16: struct memblock_s *next;
20: struct memblock_s *prev;
} memblock_t; // sizeof() == 24

typedef struct
{
 0:  int         size;      // total bytes malloced, including header
 4:  memblock_t *rover;
 8:  memblock_t  blocklist; // start / end cap for linked list
      0: int     size; // including the header and possibly tiny fragments
12:   4: void  **user; // NULL if a free block
16:   8: short   tag;  // purgelevel
18:  10: short   id;   // should be ZONEID
20:  12: int     lockframe; // don't purge on the same frame
24:  16: struct memblock_s *next;
28:  20: struct memblock_s *prev;
} memzone_t; // sizeof() == 32

*/

