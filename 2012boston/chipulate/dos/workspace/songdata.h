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
	db	$f9,$08,$fe,$8b,$fd,$00,$20,$0c
	db	$22,$0c,$24,$0c,$25,$0c,$fd,$01
	db	$27,$18,$fd,$00,$29,$06,$2b,$06
	db	$30,$0c,$30,$0c,$32,$0c,$34,$0c
	db	$35,$0c,$fd,$01,$37,$18,$fd,$00
	db	$39,$06,$3b,$06,$40,$0c,$20,$0c
	db	$22,$0c,$24,$0c,$25,$0c,$fd,$01
	db	$27,$18,$fd,$00,$29,$06,$2b,$06
	db	$30,$0c,$30,$0c,$32,$0c,$34,$0c
	db	$35,$0c,$fd,$01,$37,$18,$fd,$00
	db	$39,$06,$3b,$06,$40,$0c
songdata_00_lp:
	db	$fc,$ff,$ff

songdata_01:
	db	$f9,$08,$fe,$87,$fd,$00,$10,$0c
	db	$12,$0c,$14,$0c,$15,$0c,$fd,$01
	db	$17,$18,$fd,$00,$19,$06,$1b,$06
	db	$20,$0c,$20,$0c,$22,$0c,$24,$0c
	db	$25,$0c,$fd,$01,$27,$18,$fd,$00
	db	$29,$06,$2b,$06,$30,$0c,$10,$0c
	db	$12,$0c,$14,$0c,$15,$0c,$fd,$01
	db	$17,$18,$fd,$00,$19,$06,$1b,$06
	db	$20,$0c,$20,$0c,$22,$0c,$24,$0c
	db	$25,$0c,$fd,$01,$27,$18,$fd,$00
	db	$29,$06,$2b,$06,$30,$0c
songdata_01_lp:
	db	$fc,$ff,$ff

songdata_02:
	db	$fe,$8f,$00,$12,$fc,$06,$04,$12
	db	$fc,$06,$07,$09,$fc,$03,$07,$09
	db	$fc,$03,$09,$04,$fc,$02,$0b,$04
	db	$fc,$02,$10,$09,$fc,$03,$10,$12
	db	$fc,$06,$14,$12,$fc,$06,$17,$09
	db	$fc,$03,$17,$09,$fc,$03,$19,$04
	db	$fc,$02,$1b,$04,$fc,$02,$20,$09
	db	$fc,$03,$00,$12,$fc,$06,$04,$12
	db	$fc,$06,$07,$09,$fc,$03,$07,$09
	db	$fc,$03,$09,$04,$fc,$02,$0b,$04
	db	$fc,$02,$10,$09,$fc,$03,$10,$12
	db	$fc,$06,$14,$12,$fc,$06,$17,$09
	db	$fc,$03,$17,$09,$fc,$03,$19,$04
	db	$fc,$02,$1b,$04,$fc,$02,$20,$09
	db	$fc,$03,$00,$12,$fc,$06,$04,$12
	db	$fc,$06,$07,$09,$fc,$03,$07,$09
	db	$fc,$03,$09,$04,$fc,$02,$0b,$04
	db	$fc,$02,$10,$09,$fc,$03,$10,$12
	db	$fc,$06,$14,$12,$fc,$06,$17,$09
	db	$fc,$03,$17,$09,$fc,$03,$19,$04
	db	$fc,$02,$1b,$04,$fc,$02,$20,$09
	db	$fc,$03,$00,$12,$fc,$06,$04,$12
	db	$fc,$06,$07,$09,$fc,$03,$07,$09
	db	$fc,$03,$09,$04,$fc,$02,$0b,$04
	db	$fc,$02,$10,$09,$fc,$03,$10,$12
	db	$fc,$06,$14,$12,$fc,$06,$17,$09
	db	$fc,$03,$17,$09,$fc,$03,$19,$04
	db	$fc,$02,$1b,$04,$fc,$02,$20,$09
	db	$fc,$03
songdata_02_lp:
	db	$fc,$ff,$ff

songdata_03:
	db	$fe,$8f
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
