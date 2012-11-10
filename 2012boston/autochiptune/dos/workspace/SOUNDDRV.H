;-------------------------------------------------------------------------------
;初期化ルーチン
sound_init:
	lda	#$0f
	sta	$4015
	lda	#$00
	sta	channel_sel

	sta	$4080
	sta	$4082
	sta	$4083
	sta	$4084
	sta	$4085
	sta	$4086
	sta	$4087
	sta	$4088
	sta	$4089
	lda	#$e8
	sta	$408a

	ldx	#$00
	lda	#$80
	sta	$f800
	lda	#$00
soresore:
	sta	$4800
	inx
	cpx	#$00
	beq	owari
	jmp	soresore
owari:
	lda	#$7f
	sta	$f800
	lda	n106_channel
	sec
	sbc	#$01
	asl	a
	asl	a
	asl	a
	asl	a
	sta	$4800
	sta	n106_7f

sound_channel_set:
	lda	channel_sel

	sec
	sbc	#$06
	cmp	n106_channel		;n106は終わりか？
	beq	sound_channel_reset
	lda	channel_sel

	asl	a
	tax
	lda	#$00
;	sta	lfo_st_counter_1,x
	sta	effect_flag,x
	lda	#$01
	sta	sound_counter,x

	lda	sound_data_table,x
	sta	<sound_add_low,x	;データ開始位置書き込み
	inx
	lda	sound_data_table,x
	sta	<sound_add_low,x	;データ開始位置書き込み
	inc	channel_sel
	jmp	sound_channel_set

;-------------------------------------------------------------------------------
sound_channel_reset:			;init も play もココが最後
	lda	#$00
	sta	channel_sel
	rts				;これでお終い
;--------------------------
sound_expansion_part:
	jsr	sound_fds		;FDS行ってこい
	inc	channel_sel		;次のチャンネルを設定して
n106_return:
	jsr	sound_n106		;n106行ってこい
	inc	channel_sel		;次のチャンネルを設定して
	lda	channel_sel
	sec
	sbc	#$06
	cmp	n106_channel		;n106は終わりか？
	bne	n106_return		;まだなら戻れ

	jmp	sound_channel_reset

;--------------------------
sound_driver_start:

	lda	channel_sel
;	cmp	#$06
;	beq	sound_channel_reset	;channel_selが05ならお終い
	cmp	#$05
	beq	sound_expansion_part
	cmp	#$04
	beq	sound_dpcm_part

	asl	a
	tax
	dec	sound_counter,x		;カウンタいっこ減らし
	bne	return1			;ゼロ以外ならカウンタ減らして

	jsr	sound_data_read		;ゼロならサウンド読み込み

	lda	rest_flag,x
	and	#%00000001
	bne	channel_end		;休符ならお終い

duty_write:
	lda	effect_flag,x
	and	#%00000100
;	cmp	#%00000100
	beq	enve_write
	jsr	sound_duty_enverope	;自作（？）音色処理ぃ～

enve_write:
	lda	effect_flag,x
	and	#%00000001
;	cmp	#%00000001
	beq	lfo_write
	jsr	sound_software_enverope	;エンベロープ処理ぃ～

lfo_write:
	lda	effect_flag,x
	and	#%00010000
;	cmp	#%00010000
	beq	pitchenve_write
	jsr	sound_lfo
;	jmp	channel_end

pitchenve_write:
	lda	effect_flag,x
	and	#%00000010
;	cmp	#%00000010
	beq	arpeggio_write
	jsr	sound_pitch_enverope
;	jmp	channel_end

arpeggio_write:
	lda	effect_flag,x
	and	#%00001000
;	cmp	#%00001000
	beq	freq_data_write
	jsr	sound_high_speed_arpeggio
	jmp	channel_end

freq_data_write:
	jsr	sound_data_write	;そしてデータ書き出し

channel_end:
	inc	channel_sel		;次のチャンネルを設定して
	jmp	sound_driver_start	;はじめに戻す

sound_dpcm_part:
	asl	a
	tax
	dec	sound_counter,x		;カウンタいっこ減らし
	bne	channel_end
	jsr	sound_dpcm_play
	jmp	channel_end
;-------
return1:
	lda	rest_flag,x
	and	#%00000001
	bne	return7			;休符なら飛ばし

duty_write2:
	lda	effect_flag,x
	and	#%00000100
	cmp	#%00000100
	bne	enve_write2
	jsr	sound_duty_enverope

enve_write2:
	lda	effect_flag,x
	and	#%00000001
	cmp	#%00000001
	bne	lfo_write2
	jsr	sound_software_enverope

lfo_write2:
	lda	effect_flag,x
	and	#%00010000
	cmp	#%00010000
	bne	pitchenve_write2
	jsr	sound_lfo

pitchenve_write2:
	lda	effect_flag,x
	and	#%00000010
	cmp	#%00000010
	bne	arpeggio_write2
	jsr	sound_pitch_enverope

arpeggio_write2:
	lda	effect_flag,x
	and	#%00001000
	cmp	#%00001000
	bne	return7
	jsr	sound_high_speed_arpeggio
	jmp	return7

sound_dpcm_part2:
	jsr	sound_dpcm_play
return7:
	inc	channel_sel		;次のチャンネルを設定して
	jmp	sound_driver_start	;はじめに戻る
;-------------------------------------------------------------------------------
frequency_set:

	lda	channel_sel
	asl	a
	tax
	lda	sound_sel,x		;音階データ読み出し
	and	#%00001111		;下位4bitを取り出して
	asl	a
	tay

	lda	channel_sel
	cmp	#$03
	beq	noise_frequency_get	;４チャンネル目ならノイズ周波数取得へ

	lda	psg_frequency_table,y	;PSG周波数テーブルからLowを読み出す
	sta	sound_freq_low,x	;書き込み
	iny				;yを１増やして
	lda	psg_frequency_table,y	;PSG周波数テーブルからHighを読み出す
	sta	sound_freq_high,x	;書き込み

oct_set1:

	lda	sound_sel,x		;音階データ読み出し
	lsr	a			;上位4bitを取り出し
	lsr	a			;
	lsr	a			;
	lsr	a			;
	beq	freq_end		;ゼロならそのまま終わり
	tay				;

oct_set2:

	lsr	sound_freq_high,x	;右シフト　末尾はCへ
	ror	sound_freq_low,x	;Cから持ってくるでよ　右ローテイト
	dey				;
	bne	oct_set2		;オクターブ分繰り返す

freq_end:
	jsr	detune_write_sub
	rts

detune_write_sub:
	lda	effect_flag,x
	and	#%10000000
	bne	detune_part
	rts

detune_part:
	lda	detune_dat,x
	and	#%10000000
	bne	detune_minus

detune_plus:
	lda	detune_dat,x
	clc
	adc	sound_freq_low,x
	sta	sound_freq_low,x
	bcs	mid_plus
	rts
mid_plus:
	inc	sound_freq_high,x
	bne	n106_high_through
	inc	sound_freq_n106,x
n106_high_through:
	rts

detune_minus:
	lda	detune_dat,x
	and	#%01111111
	sta	detune_tmp
	lda	sound_freq_low,x
	sec
	sbc	detune_tmp
	sta	sound_freq_low,x
	bcc	mid_minus
	rts
mid_minus:
	dec	sound_freq_high,x
	bne	n106_high_through
	dec	sound_freq_n106,x
	rts

noise_frequency_get:
	lda	noise_frequency_table,y	;周波数テーブルからLowを読み出す
	sta	sound_freq_low,x	;書き込み
	iny				;yを１増やして
	lda	noise_frequency_table,y	;周波数テーブルからHighを読み出す
	sta	sound_freq_high,x	;書き込み
	rts
;-------------------------------------------------------------------------------
sound_data_read:
	jsr	reg400x
	lda	[sound_add_low,x]
;----------
;ループ処理1
loop_program
	cmp	#$a0
	bne	loop_program2
	jsr	loop_sub
	jmp	sound_data_read
;----------
;ループ処理2(分岐)
loop_program2
	cmp	#$a1
	bne	data_end		;duty_set
	jsr	loop_sub2
	jmp	sound_data_read
;----------
;データエンド設定
data_end:
	cmp	#$ff
	bne	duty_set
	jsr	data_end_sub
	jmp	sound_data_read

data_end_sub:
	lda	loop_point_table,x
	sta	<sound_add_low,x	;ループ開始位置書き込み Low
	inx
	lda	loop_point_table,x
	sta	<sound_add_low,x	;ループ開始位置書き込み High
	rts
;----------
;音色設定
duty_set:
	cmp	#$fe
	bne	volume_set
	jsr	sound_data_address
	lda	[sound_add_low,x]	;音色データ読み出し
	pha
	and	#%10000000		;8bit目確認
	cmp	#%10000000
	bne	duty_enverope_part	;ヂューティエンベ処理へ

duty_select_part:
	lda	effect_flag,x
	and	#%11111011
	sta	effect_flag,x		;デューティエンベロープ無効指定
	jsr	reg400x
	pla
	asl	a
	asl	a
	asl	a
	asl	a
	sta	register_high,x		;書き込み
	ora	register_low,x
	sta	$4000,y
	jsr	sound_data_address
	jmp	sound_data_read

duty_enverope_part:
	lda	effect_flag,x
	ora	#%00000100
	sta	effect_flag,x		;デューティエンベロープ有効指定
	pla
	sta	duty_sel,x
	asl	a
	tay
	lda	dutyenve_table,y	;デューティエンベロープアドレス設定
	sta	<duty_add_low,x
	iny
	lda	dutyenve_table,y
	sta	<duty_add_high,x
	jsr	sound_data_address
	jmp	sound_data_read

;----------
;音量設定
volume_set:
	cmp	#$fd
	bne	rest_set
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	temporary
	and	#%10000000		;bit7確認
	cmp	#%10000000
	bne	softenve_part		;ソフトエンベ処理へ

volume_part:
	lda	effect_flag,x
	and	#%11111110
	sta	effect_flag,x		;ソフトエンベ無効指定

	lda	temporary
	and	#%00001111
	sta	register_low,x
	ora	register_high,x
	sta	$4000,y			;ボリューム書き込み
	jsr	sound_data_address
	jmp	sound_data_read

softenve_part:
	jsr	volume_sub
	jmp	sound_data_read
volume_sub:
	lda	effect_flag,x
	ora	#%00000001
	sta	effect_flag,x		;ソフトエンベ有効指定

;	lda	#$00
;	sta	register_low,x		;通常ボリュームをゼロに

	lda	temporary
	sta	softenve_sel,x
	asl	a
	tay
	lda	softenve_table,y	;ソフトエンベデータアドレス設定
	sta	<soft_add_low,x
	iny
	lda	softenve_table,y
	sta	<soft_add_high,x
	jsr	sound_data_address
	rts
;----------
rest_set:
	cmp	#$fc
	bne	lfo_set

	lda	rest_flag,x
	ora	#%00000001
	sta	rest_flag,x

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x

	lda	channel_sel
	cmp	#$02
	beq	tri

	lda	register_high,x
	sta	$4000,y
	jsr	sound_data_address
	rts
tri:
	lda	#$00
	sta	$4000,y
	jsr	sound_data_address
	rts
;----------
lfo_set:
	cmp	#$fb
	bne	detune_set
	jsr	lfo_set_sub
	jmp	sound_data_read

lfo_set_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	lfo_data_set

	jsr	reg400x
	lda	effect_flag,x
	and	#%10001111		;LFO無効処理
	sta	effect_flag,x
	jsr	sound_data_address
	rts
lfo_data_set:
	asl	a
	asl	a
	sta	lfo_data873

	tay
	lda	channel_sel
	asl	a
	tax
	lda	lfo_data,y
	sta	lfo_count		;ディレイセット
	sta	lfo_start_count,x
	iny
	lda	lfo_data,y
	sta	lfo_revers		;スピードセット
	sta	lfo_revers_count,x
	iny
	lda	lfo_data,y
	sta	lfo_depth		;デプスセット
	iny
	lda	lfo_data,y
	sta	lfo_harf_time
	sta	lfo_harf_count,x		;1/2カウンタセット

	jsr	warizan_start

	lda	channel_sel		;なぜこの処理を入れているかというと
	sec				;内蔵音源と拡張音源で+-が逆だからである
	sbc	#$05
	bcc	urararara2


	lda	effect_flag,x
	ora	#%00110000
	sta	effect_flag,x
	jmp	ittoke2
urararara2:
	lda	effect_flag,x
	and	#%11011111		;波形－処理
	ora	#%00010000		;LFO有効フラグセット
	sta	effect_flag,x
ittoke2:
	jsr	sound_data_address
	rts
;----------
detune_set:
	cmp	#$fa
	bne	sweep_set
	jsr	detune_sub
	jmp	sound_data_read

detune_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	detune_data_set

	jsr	reg400x
	lda	effect_flag,x
	and	#%01111111		;detune無効処理
	sta	effect_flag,x
	jsr	sound_data_address
	rts
detune_data_set:
	pha
	jsr	reg400x
	pla
	tay
	sta	detune_dat,x
	lda	effect_flag,x
	ora	#%10000000		;detune有効処理
	sta	effect_flag,x
	jsr	sound_data_address
	rts
;----------
sweep_set:
	cmp	#$f9
	bne	pitch_set

	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	$4001,y
	jsr	sound_data_address
	jmp	sound_data_read
;----------
;ピッチエンベロープ設定
pitch_set:
	cmp	#$f8
	bne	arpeggio_set
	jsr	pitch_set_sub
	jmp	sound_data_read

pitch_set_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	pitch_enverope_part
	lda	effect_flag,x
	and	#%11111101
	sta	effect_flag,x
	jsr	sound_data_address
	rts

pitch_enverope_part:
	sta	pitch_sel,x
	asl	a
	tay
	lda	pitchenve_table,y
	sta	<pitch_add_low,x
	iny
	lda	pitchenve_table,y
	sta	<pitch_add_high,x
	lda	effect_flag,x
	ora	#%00000010
	sta	effect_flag,x
	jsr	sound_data_address
	rts
;----------
;ノートエンベロープ設定
arpeggio_set:
	cmp	#$f7
	bne	freq_direct_set
	jsr	arpeggio_set_sub
	jmp	sound_data_read

arpeggio_set_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	cmp	#$ff
	bne	arpeggio_part

	lda	effect_flag,x
	and	#%11110111
	sta	effect_flag,x
	jsr	sound_data_address
	rts

arpeggio_part:
	sta	arpeggio_sel,x
	asl	a
	tay
	lda	arpeggio_table,y
	sta	<arpe_add_low,x
	iny
	lda	arpeggio_table,y
	sta	<arpe_add_high,x

	lda	effect_flag,x
	ora	#%00001000
	sta	effect_flag,x
	jsr	sound_data_address
	rts
;----------
;再生周波数直接設定
freq_direct_set:
	cmp	#$f6
	bne	y_command_set
	jsr	direct_freq_sub
	rts

direct_freq_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_freq_low,x		;Low
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_freq_high,x		;High
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x			;Counter
	jsr	sound_data_address
	jsr	effect_init
	rts
;----------
;ｙコマンド設定
y_command_set:
	cmp	#$f5
	bne	wait_set
	jsr	y_sub
	jmp	sound_data_read

y_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	<reg_add_low
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	<reg_add_high
	jsr	sound_data_address
	lda	[sound_add_low,x]
	ldx	#$00
	sta	[reg_add_low,x]
	jsr	reg400x
	jsr	sound_data_address
	rts
;----------
;ウェイト設定
wait_set:
	cmp	#$f4
	bne	oto_set

wait_sub:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x
	jsr	sound_data_address

	rts
	rts
	rts
;----------
oto_set:
	sta	sound_sel,x		;処理はまた後で

	jsr	sound_data_address
	lda	[sound_add_low,x]	;音長読み出し
	sta	sound_counter,x		;実際のカウント値となります
	jsr	sound_data_address

	jsr	frequency_set		;周波数セットへ
	jsr	effect_init
	rts
;-----------------
sound_data_address:
	inc	<sound_add_low,x	;データアドレス＋１
	bne	return2			;位が上がったら
	inc	<sound_add_high,x	;データアドレス百の位（違）＋１
return2:
	rts
;-------------------------------------------------------------------------------
sound_data_write:
	jsr	reg400x
	lda	register_low,x		;音量保持
	ora	register_high,x
	sta	$4000,y
write:
	lda	sound_freq_low,x	;Low Write
	sta	$4002,y
	lda	sound_freq_high,x	;High Write
	sta	$4003,y
	rts
;-------------------------------------------------------------------------------
sound_software_enverope:
	jsr	volume_enve_sub
	sta	register_low,x
	ora	register_high,x		;音色データ（上位4bit）と下位4bitで足し算
	sta	$4000,y			;書き込み～
	jsr	enverope_address	;アドレス一個増やして
	rts				;おしまい

volume_enve_sub:
	jsr	reg400x

	lda	[soft_add_low,x]	;エンベロープデータ読み込み
	cmp	#$ff			;最後かどーか
	beq	return3			;最後ならループ処理へ
	rts

return3:
	lda	softenve_sel,x
	asl	a
	tay
	lda	softenve_lp_table,y
	sta	<soft_add_low,x
	iny
	lda	softenve_lp_table,y
	sta	<soft_add_high,x
	jmp	volume_enve_sub
;-------------------------------------------------------------------------------
enverope_address:
	inc	<soft_add_low,x
	bne	return5
	inc	<soft_add_high,x
return5:
	rts
;--------------------------------------	
reg400x:
	lda	channel_sel
	asl	a
	tax
	asl	a
	tay
	rts
;-------------------------------------------------------------------------------
sound_duty_enverope:
	jsr	reg400x

	lda	channel_sel
	cmp	#$02
	beq	return21		;三角波なら飛ばし～

	lda	[duty_add_low,x]	;エンベロープデータ読み込み
	cmp	#$ff			;最後かどーか
	beq	return22		;最後ならそのままおしまい
	asl	a
	asl	a
	asl	a
	asl	a
	sta	register_high,x
	ora	register_low,x		;音色データ（上位4bit）と下位4bitで足し算
	sta	$4000,y			;書き込み～
	jsr	duty_enverope_address	;アドレス一個増やして
return21:
	rts				;おしまい

return22:
	lda	duty_sel,x
	asl	a
	tay
	lda	dutyenve_lp_table,y
	sta	<duty_add_low,x
	iny
	lda	dutyenve_lp_table,y
	sta	<duty_add_high,x
	jmp	sound_duty_enverope

;-------------------------------------------------------------------------------
duty_enverope_address:
	inc	<duty_add_low,x
	bne	return23
	inc	<duty_add_high,x
return23:
	rts
;--------------------------------------	
sound_dpcm_play:
	jsr	reg400x
	lda	[sound_add_low,x]
;----------
;ループ処理1
	cmp	#$a0
	bne	dmc_loop_program2
	jsr	loop_sub
	jmp	sound_dpcm_play
;----------
;ループ処理2(分岐)
dmc_loop_program2
	cmp	#$a1
	bne	dmc_data_end
	jsr	loop_sub2
	jmp	sound_dpcm_play
;----------
dmc_data_end:
	cmp	#$ff
	bne	no_dpcm
	jsr	data_end_sub
	jmp	sound_dpcm_play
;----------
no_dpcm:
	cmp	#$fc
	bne	wait_set2
	lda	#$00
	sta	$4011
	jmp	ontyou2

wait_set2:
	cmp	#$f4
	bne	dpcm_set
	jsr	wait_sub
	rts

dpcm_set:
	asl	a
	asl	a
	tax

	pha
	lda	#$0F
	sta	$4015
;;;;;;;;;; 無理矢理改造 "MCK virtual keyboard" by Norix
	lda	#$FF		; 鳴らすぜ！
	sta	$07A0+4*2	; 休符フラグ使っちゃうよー
;;;;;;;;;;
	pla

	lda	dpcm_data,x		;DPCM control
	sta	$4010
	inx	
	lda	dpcm_data,x		;DPCM wave control
	sta	$4011
	inx	
	lda	dpcm_data,x		;DPCM address set
	sta	$4012
	inx	
	lda	dpcm_data,x		;DPCM length set
	sta	$4013

	lda	#$1F
	sta	$4015

ontyou2:
	jsr	reg400x
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	sound_counter,x
	jsr	sound_data_address
	rts
;-------------------------------------------------------------------------------
sound_lfo:
	lda	sound_freq_high,x
	sta	temporary

	jsr	lfo_sub

	lda	sound_freq_low,x
	sta	$4002,y			;　　現在値をレジスタにセット
	lda	sound_freq_high,x
	cmp	temporary
	beq	end4
	sta	$4003,y
end4:
	rts				;ここまで
;-------------------------------------------------------------------------------
lfo_sub:
	jsr	reg400x
	lda	lfo_start_count,x
	beq	lfo_start
	dec	lfo_start_count,x
	rts
lfo_start:

	lda	effect_flag,x
	and	#%01000000
	bne	hanbun

	lda	lfo_harf_count,x
	beq	lfo_start_2

	dec	lfo_harf_count,x
	bne	lfo_start_2

	lda	effect_flag,x
	ora	#%01000000
	sta	effect_flag,x
hanbun:
	jsr	lfo_set_2
	lsr	lfo_revers
	lsr	lfo_revers_count,x
	jsr	warizan

lfo_start_2:
	asl	lfo_revers
	lda	lfo_revers_count,x	;現在の値を読み込んで
	cmp	lfo_revers
	beq	lfo_revers_set		;規定数に達していたら反転セット
	jmp	lfo_depth_set		;達していなければデプス処理へ

lfo_revers_set:
	lda	#$00			;
	sta	lfo_revers_count,x	;反転カウンタ初期化
	lda	effect_flag,x		;エフェクト読み込んで
	pha				;一時退避
	and	#%00100000
	bne	lfo_revers_p

	pla
	ora	#%00100000
	sta	effect_flag,x
	jmp	lfo_depth_set
lfo_revers_p:
	pla
	and	#%11011111
	sta	effect_flag,x

lfo_depth_set:
	lsr	lfo_revers
	lda	lfo_depth_count,x	;時間読み込み
	cmp	lfo_adc_sbc_time	;現在のカウンタと比較
	beq	lfo_depth_work		;一致していればデプス処理へ
	jmp	lfo_count_plus		;まだならカウンタプラスへ

lfo_depth_work:
	lda	#$00			;
	sta	lfo_depth_count,x	;デプスカウンタ初期化
	lda	effect_flag,x		;＋か－か
	and	#%00100000
	bne	lfo_depth_plus

lfo_depth_minus:
	lda	sound_freq_low,x	;デプス読み込み
	sec
	sbc	lfo_depth		;引く
	sta	sound_freq_low,x
	bcc	lfo_high_minus		;桁処理へ
	jmp	lfo_count_plus

lfo_high_minus:
	dec	sound_freq_high,x	;桁処理
	bne	lfo_count_plus
	dec	sound_freq_n106,x
	jmp	lfo_count_plus
lfo_depth_plus:
	lda	sound_freq_low,x	;デプス読み込み
	clc
	adc	lfo_depth		;足す
	sta	sound_freq_low,x
	bcs	lfo_high_plus		;桁処理へ
	jmp	lfo_count_plus

lfo_high_plus:
	inc	sound_freq_high,x	;桁処理
	bne	lfo_count_plus
	inc	sound_freq_n106,x

lfo_count_plus:
	inc	lfo_revers_count,x	;カウンタ足してお終い
	inc	lfo_depth_count,x
	lda	effect_flag,x
	and	#%01000000
	beq	endend
	jsr	lfo_set_2
	jsr	warizan
endend:
	rts

;-------------------------------------------------------------------------------
lfo_set_2:
	lda	lfo_data873
	tay
	lda	channel_sel
	asl	a
	tax
	iny
	lda	lfo_data,y
	sta	lfo_revers		;スピードセット
	sta	lfo_revers_count,x
	iny
	lda	lfo_data,y
	sta	lfo_depth		;デプスセット
	rts
;-------------------------------------------------------------------------------
sound_pitch_enverope:
	lda	sound_freq_high,x
	sta	temporary
	jsr	pitch_sub
pitch_write:
	lda	sound_freq_low,x
	sta	$4002,y
	lda	sound_freq_high,x
	cmp	temporary
	beq	end3
	sta	$4003,y
end3:
	jsr	pitch_enverope_address
	rts
;-------------------------------------------------------------------------------
pitch_sub:
	jsr	reg400x
	lda	[pitch_add_low,x]
	cmp	#$ff
	beq	return62

	and	#%10000000
;	cmp	#%10000000
	bne	pitch_plus

	lda	[pitch_add_low,x]
	clc
	adc	sound_freq_low,x
	sta	sound_freq_low,x	;low側書き込み
	bcs	freq_high_plus_2
	rts
freq_high_plus_2:
	inc	sound_freq_high,x	;high側＋１
	bne	rreturn
	inc	sound_freq_n106,x
rreturn
	rts				;レジスタ書き込みへGO!
pitch_plus:
	lda	[pitch_add_low,x]
	and	#%01111111
	sta	pitch_tmp
	sec
	lda	sound_freq_low,x
	sbc	pitch_tmp
	sta	sound_freq_low,x
	bcc	freq_high_minus_2
	rts
freq_high_minus_2:
	dec	sound_freq_high,x
	bne	rreturn2
	dec	sound_freq_n106,x
rreturn2
	rts
;-------------------------------------------------------------------------------
return62:
	lda	[pitch_add_low,x]
	lda	pitch_sel,x
	asl	a
	tay
	lda	pitchenve_lp_table,y
	sta	<pitch_add_low,x
	iny
	lda	pitchenve_lp_table,y
	sta	<pitch_add_high,x
	jmp	pitch_sub
;-------------------------------------------------------------------------------
pitch_enverope_address:
	inc	<pitch_add_low,x
	bne	return63
	inc	<pitch_add_high,x
return63:
	rts
;-------------------------------------------------------------------------------
sound_high_speed_arpeggio:		;note enverope
	lda	sound_freq_high,x
	sta	temporary2
	jsr	note_enve_sub
	jsr	frequency_set
note_freq_write:
	jsr	reg400x
	lda	sound_freq_low,x
	sta	$4002,y
	lda	sound_freq_high,x
	cmp	temporary2,y
	beq	end2
	sta	$4003,y
end2:
	jsr	arpeggio_address
	rts
;-------------------------------------------------------------------------------
note_add_set:
	lda	arpeggio_sel,x
	asl	a
	tay
	lda	arpeggio_lp_table,y
	sta	<arpe_add_low,x
	iny
	lda	arpeggio_lp_table,y
	sta	<arpe_add_high,x
	jmp	note_enve_sub
;-------------------------------------------------------------------------------
arpeggio_address:
	inc	<arpe_add_low,x
	bne	return83
	inc	<arpe_add_high,x
return83:
	rts
;-------------------------------------------------------------------------------
note_enve_sub:

	jsr	reg400x
	lda	[arpe_add_low,x]	;ノートエンベデータ読み出し
	cmp	#$ff			;$ff（お終い）か？
	beq	note_add_set
	cmp	#$00			;ゼロか？
	beq	return83		;ゼロなら何もせずお終い
	cmp	#$80
	beq	return83		;ゼロなら何もせずお終い
	and	#%10000000
;	cmp	#%10000000		;－か？
	bne	arpeggio_minus		;－処理へ

arpeggio_plus:
	lda	[arpe_add_low,x]	;ノートエンベデータを読み出して
	sta	arpeggio_tmp		;テンポラリに置く（ループ回数）
arpeggio_plus2:
	lda	sound_sel,x		;音階データ読み出し
	and	#$0f			;下位4bit抽出
	cmp	#$0b			;もしbなら
	beq	oct_plus		;オクターブ＋処理へ
	inc	sound_sel,x		;でなければ音階＋１
	jmp	loop_1			;ループ処理１へ
oct_plus:
	lda	sound_sel,x		;音階データ読み出し
	and	#$f0			;上位4bit取り出し＆下位4bitゼロ
	clc
	adc	#$10			;オクターブ＋１
	sta	sound_sel,x		;音階データ書き出し
loop_1:
	dec	arpeggio_tmp		;ループ回数－１
	lda	arpeggio_tmp		;んで読み出し
;	cmp	#$00			;ゼロか？
	beq	note_enve_end		;ならループ処理終わり
	jmp	arpeggio_plus2		;でなければまだ続く

arpeggio_minus:
	lda	[arpe_add_low,x]
	and	#%01111111
	sta	arpeggio_tmp
arpeggio_minus2:
	lda	sound_sel,x		;音階データ読み出し
	and	#$0f			;下位4bit抽出
;	cmp	#$00			;ゼロか？
	beq	oct_minus		;ゼロなら－処理へ
	dec	sound_sel,x		;でなければ音階－１
	jmp	loop_2			;ループ処理２へ
oct_minus:
	lda	sound_sel,x		;音階データ読み出し
	clc
	adc	#$0b			;+b
	sec
	sbc	#$10			;-10
	sta	sound_sel,x		;音階データ書き出し
loop_2:
	dec	arpeggio_tmp		;ループ回数－１
	lda	arpeggio_tmp		;んで読み出し
	cmp	#$00			;ゼロか？
	bne	arpeggio_minus2		;ならループ処理終わり
note_enve_end:
	rts				;でなければまだ続く
;-------------------------------------------------------------------------------
effect_init:
;ソフトウェアエンベロープ読み込みアドレス初期化
	lda	softenve_sel,x
	asl	a
	tay
	lda	softenve_table,y
	sta	<soft_add_low,x
	iny
	lda	softenve_table,y
	sta	<soft_add_high,x

;ピッチエンベロープ読み込みアドレス初期化
	lda	pitch_sel,x
	asl	a
	tay
	lda	pitchenve_table,y
	sta	<pitch_add_low,x
	iny
	lda	pitchenve_table,y
	sta	<pitch_add_high,x

;デューティエンベロープ読み込みアドレス初期化
	lda	duty_sel,x
	asl	a
	tay
	lda	dutyenve_table,y
	sta	<duty_add_low,x
	iny
	lda	dutyenve_table,y
	sta	<duty_add_high,x

;ノートエンベロープ読み込みアドレス初期化
	lda	arpeggio_sel,x
	asl	a
	tay
	lda	arpeggio_table,y
	sta	<arpe_add_low,x
	iny
	lda	arpeggio_table,y
	sta	<arpe_add_high,x
;ソフトウェアLFO初期化
	lda	lfo_count
	sta	lfo_start_count,x
	lda	lfo_adc_sbc_time
	sta	lfo_depth_count,x
	lda	lfo_harf_time
	sta	lfo_harf_count,x
	lda	lfo_revers
	sta	lfo_revers_count,x

	lda	channel_sel
	sec
	sbc	#$04
	bmi	urararara

	lda	effect_flag,x
	and	#%10111111
	ora	#%00100000
	sta	effect_flag,x
	jmp	ittoke
urararara:
	lda	effect_flag,x
	and	#%10011111
	sta	effect_flag,x
	
ittoke:
;休符フラグクリア
	lda	#$00
	sta	rest_flag,x


	rts
;-------------------------------------------------------------------------------
loop_sub:
	jsr	sound_data_address
	lda	channel_loop,x
	bne	loop_dec

	lda	[sound_add_low,x]
	sta	channel_loop,x
	jmp	loop_start
loop_dec:
	dec	channel_loop,x
	lda	#$01
	cmp	channel_loop,x
	beq	loop_end
loop_start:
	jsr	sound_data_address
	lda	[sound_add_low,x]
	pha
	jsr	sound_data_address
	lda	[sound_add_low,x]
	sta	<sound_add_high,x
	pla
	sta	<sound_add_low,x	;ループアドレス書き込み
	rts
loop_end:
	jsr	sound_data_address	;データアドレス進めてx3
	jsr	sound_data_address
	jsr	sound_data_address
	rts				;おちまい
;-------------------------------------------------------------------------------
loop_sub2:
	lda	channel_loop,x
	bne	loop_sbc2
loop_through:
	jsr	sound_data_address
	jsr	sound_data_address
	jsr	sound_data_address
	rts
loop_sbc2
	and	#%00000001
	bne	loop_through		;ゼロでなければスルー
	dec	channel_loop,x
	jsr	sound_data_address	;ゼロならループアドレス書き込み
	lda	[sound_add_low,x]
	pha
	jsr	sound_data_address	;ゼロならループアドレス書き込み
	lda	[sound_add_low,x]
	sta	<sound_add_high,x
	pla
	sta	<sound_add_low,x	;ループ開始位置書き込み High
	rts
;-------------------------------------------------------------------------------
warizan_start:
	lda	#$00
	sta	kotae
	lda	lfo_revers
	cmp	lfo_depth
	beq	plus_one
	bmi	depth_wari
revers_wari:
	lda	lfo_depth
	sta	waru
	lda	lfo_revers
	jsr	warizan
	lda	kotae
	sta	lfo_adc_sbc_time
	sta	lfo_depth_count,x
	lda	#$01
	sta	lfo_depth
	rts
depth_wari:
	lda	lfo_revers
	sta	waru
	lda	lfo_depth
	jsr	warizan
	lda	kotae
	sta	lfo_depth
	lda	#$01
	sta	lfo_adc_sbc_time
	sta	lfo_depth_count,x
	rts
plus_one:
	lda	#$01
	sta	lfo_depth
	sta	lfo_adc_sbc_time
	sta	lfo_depth_count,x
	rts
warizan:
	inc	kotae
	sec
	sbc	waru
	beq	warizan_end
	bcc	warizan_end
	jmp	warizan
warizan_end:
	rts
;-------------------------------------------------------------------------------

;ゼロページに必要なメモリ
;14ch sq1 sq2 tri noiz dpcm fds 106 106 106 106 106 106 106 106 28bytes
soft_add_low	=	$0040	;6bytes $c0,$c1 ch1 - $ca,$cb ch6 *ch5は未使用
soft_add_high	=	$0041	;6bytes
pitch_add_low	=	$0060	;8bytes $d0,$d1 ch1 - $da,$db ch6 *ch4-5は未使用
pitch_add_high	=	$0061	;8bytes
duty_add_low	=	$0080	;4bytes $e6,$e7 ch1 - $e8,$e9 ch2 *ch3-6は未使用
duty_add_high	=	$0081	;4bytes
arpe_add_low	=	$00a0	;6bytes $e0,$e1 ch1 - $ea,$eb ch6 *ch4-5は未使用
arpe_add_high	=	$00a1	;6bytes
sound_add_low	=	$00c0	;6bytes $f0,$f1 ch1 - $fa,$fb ch6
sound_add_high	=	$00c1	;6bytes

reg_add_low	=	$00de	;temp
reg_add_high	=	$00df
fds_wave_address=	$00de
fds_wave_addhigh=	$00df
n106_wave_add	=	$00de
n106_wave_add_hi=	$00df
waru		=	$00de
kotae		=	$00df
;それ以外に必要なメモリ

lfo_revers_count=	$0700
lfo_depth_count	=	$0701
lfo_start_count	=	$0720

lfo_count	=	$0721
lfo_adc_sbc_time=	$0723
lfo_depth	=	$0725
lfo_revers	=	$0727
lfo_harf_time	=	$0729
lfo_data873	=	$072b

lfo_harf_count	=	$0740
detune_dat	=	$0500	;6bytes(6

register_high	=	$0620	;2bytes(8
register_low	=	$0630	;2bytes(8
duty_sel	=	$0640	;2bytes(4

channel_loop	=	$06a0

softenve_sel	=	$0760	;10bytes(20
pitch_sel	=	$0761	;10bytes(20
arpeggio_sel	=	$0780	;10bytes(20
effect_flag	=	$0781	;10bytes(20

rest_flag	=	$07a0	;10bytes(20
detune_tmp	=	$07a1	;1byte
pitch_tmp	=	$07a3	;1byte
arpeggio_tmp	=	$07a5	;1byte
temporary	=	$07a7	;1byte
channel_sel	=	$07a9	;1byte
temporary2	=	$07ab
fds_hard_count_1=	$0300
fds_hard_count_2=	$0350

sound_sel	=	$07c0	;14bytes(20
sound_counter	=	$07c1	;14bytes(20
sound_freq_low	=	$07e0	;14bytes(20
sound_freq_n106	=	$0741
sound_freq_high	=	$07e1	;10bytes(20
fds_hard_select	=	$07af
fds_volume	=	$07b1
n106_volume	=	$0660
n106_7f		=	$07b5

n106_7c		=	$0680

n106_temp	=	$07b9
n106_temp_2	=	$07bb

;+------ detune flag
;l+----- software LFOスピード可変フラグ（予約）
;ll+---- software LFO方向フラグ0=- 1=+
;lll+--- software LFO flag
;llll+---- note enverope flag
;lllll+--- duty enverope flag / FDS hardware effect flag
;llllll+-- pitch enverope flag
;lllllll+- software enverope flag
;llllllll
;DLLLadpv
