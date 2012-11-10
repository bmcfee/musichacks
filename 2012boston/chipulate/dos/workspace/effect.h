dutyenve_table:
dutyenve_lp_table:


softenve_table:
	dw	softenve_000
	dw	softenve_001
softenve_lp_table:
	dw	softenve_lp_000
	dw	softenve_lp_001

softenve_000:
	db	$0a,$09,$08,$07,$06,$05,$04,$03
softenve_lp_000:
	db	$02,$ff
softenve_001:
	db	$0f,$0f,$0e,$0e,$0d,$0d,$0c,$0c
	db	$0b,$0b,$0a,$0a,$09,$09,$08,$08
	db	$07,$07,$06
softenve_lp_001:
	db	$06,$ff

pitchenve_table:
pitchenve_lp_table:


arpeggio_table:
arpeggio_lp_table:


lfo_data:
fds_data_table:
fds_effect_select:
fds_4088_data:


n106_channel:
	db	1
n106_wave_init:
n106_wave_table:


dpcm_data:


