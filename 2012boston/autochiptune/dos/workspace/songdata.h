; Title: Song Name
; Composer: Artist
; Maker: Maker
SOUND_GENERATOR	equ	$00
sound_data_table:
	dw	songdata_00
	dw	songdata_01
	dw	songdata_02
	dw	songdata_03
	dw	songdata_04
	dw	songdata_05
	dw	songdata_06
loop_point_table:
	dw	songdata_00_lp
	dw	songdata_01_lp
	dw	songdata_02_lp
	dw	songdata_03_lp
	dw	songdata_04_lp
	dw	songdata_05_lp
	dw	songdata_06_lp

songdata_00:
	db	$f9,$08,$fe,$8b,$fd,$0f,$fd,$01
	db	$2b,$32,$fd,$0f,$21,$1a,$fd,$0e
	db	$23,$19,$fd,$0f,$25,$19,$fd,$10
	db	$25,$13,$fd,$0f,$25,$1a,$fd,$0e
	db	$25,$26,$fd,$0e,$25,$19,$fd,$0d
	db	$25,$13,$fd,$0f,$25,$33,$fd,$0e
	db	$25,$19,$fd,$0e,$25,$26,$fd,$10
	db	$25,$33,$fd,$0f,$2a,$0c,$fd,$0f
	db	$26,$27,$fd,$0f,$26,$32,$fd,$0e
	db	$26,$1a,$fd,$0e,$26,$13,$fd,$0e
	db	$26,$19,$fd,$0e,$21,$19
songdata_00_lp:
	db	$fc,$ff,$ff

songdata_01:
	db	$f9,$08,$fe,$87,$fd,$0a,$fd,$00
	db	$2a,$32,$fd,$00,$23,$1a,$fd,$00
	db	$25,$19,$fd,$00,$23,$19,$fd,$00
	db	$20,$13,$fd,$00,$23,$1a,$fd,$00
	db	$26,$26,$fd,$00,$23,$19,$fd,$00
	db	$26,$13,$fd,$00,$23,$33,$fd,$00
	db	$2a,$19,$fd,$00,$2a,$26,$fd,$00
	db	$2a,$33,$fd,$00,$21,$0c,$fd,$00
	db	$21,$27,$fd,$00,$21,$32,$fd,$00
	db	$2a,$1a,$fd,$00,$21,$13,$fd,$00
	db	$2a,$19,$fd,$00,$26,$19
songdata_01_lp:
	db	$fc,$ff,$ff

songdata_02:
	db	$fe,$8f,$1b,$12,$fc,$07,$11,$12
	db	$fc,$07,$13,$13,$fc,$07,$15,$12
	db	$fc,$07,$15,$12,$fc,$07,$15,$13
	db	$fc,$07,$15,$12,$fc,$07,$15,$12
	db	$fc,$07,$15,$13,$fc,$07,$15,$12
	db	$fc,$07,$15,$12,$fc,$07,$15,$13
	db	$fc,$07,$15,$12,$fc,$07,$1a,$12
	db	$fc,$07,$16,$13,$fc,$07,$16,$12
	db	$fc,$07,$16,$12,$fc,$07,$16,$13
	db	$fc,$07,$16,$12,$fc,$07,$11,$13
	db	$fc,$07
songdata_02_lp:
	db	$fc,$ff,$ff

songdata_03:
	db	$fe,$8f,$fd,$00,$00,$19,$fd,$0f
	db	$00,$19,$fd,$00,$fc,$1a,$fd,$00
	db	$fc,$19,$fd,$00,$fc,$19,$fd,$00
	db	$fc,$1a,$fd,$00,$fc,$19,$fd,$00
	db	$fc,$19,$fd,$00,$fc,$1a,$fd,$00
	db	$fc,$19,$fd,$00,$fc,$19,$fd,$00
	db	$fc,$1a,$fd,$00,$fc,$19,$fd,$00
	db	$fc,$19,$fd,$00,$fc,$1a,$fd,$00
	db	$fc,$19,$fd,$00,$fc,$19,$fd,$00
	db	$fc,$1a,$fd,$00,$fc,$19,$fd,$00
	db	$fc,$1a
songdata_03_lp:
	db	$fc,$ff,$ff

songdata_04:

songdata_04_lp:
	db	$fc,$ff,$ff

songdata_05:

songdata_05_lp:
	db	$fc,$ff,$ff

songdata_06:

songdata_06_lp:
	db	$fc,$ff,$ff
