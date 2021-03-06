/*
  HuSIC ローダー部分
  Please see license in the package to use this code

  History :
  2004-03-04 : created by BouKiCHi
  2005-01-19 : rewritten by BouKiCHi
  2007-04-11 : rewritten by BouKiCHi
  2011-12-11 :
  2015-05-22 : changed to display more detail variables.
*/

#asm

  .data
  .bank CONST_BANK
  .org $4400

 _sound_dat:
	dw	song_000_track_table	 ; 0
	dw	song_000_loop_table	 ; 1
	dw	softenve_table		 ; 2
	dw	softenve_lp_table	 ; 3
	dw	pitchenve_table		 ; 4
	dw	pitchenve_lp_table ; 5
	dw	lfo_data		       ; 6
	dw	song_000_bank_table ; 7
	dw	song_000_loop_bank  ; 8
	dw	arpeggio_table		 ; 9
	dw	arpeggio_lp_table	 ; 10
  dw	dutyenve_table		 ; 11
  dw	dutyenve_lp_table	 ; 12
  dw	multienv_table		 ; 13
  dw	multienv_lp_table  ; 14
  dw	song_addr_table  ; 15

_pcewav:
	dw	pce_data_table

_xpcmdata
	dw	xpcm_data

	.bank CONST_BANK
	.include "effect.h"


;------------------------------
; PCM timer routines
	.code
;	.bank DATA_BANK
	.bank START_BANK

; タイマー割り込みエントリーポイント
;  .proc _timer_pcm

_timer_pcm:
  sei
	pha
	phx
	phy

; タイマーリセット
  	sta   irq_status

; C言語用一時変数などの保存
	__ldw <__temp
	__stw <__tmr_temp

	__ldw <__ptr
	__stw <__tmr_ptr

	__ldw <_bx
	__stw <__tmr_bx

	tma	#page(_pcm_intr)
	pha
	map	_pcm_intr
	jsr	_pcm_intr
	pla
	tam	#page(_pcm_intr)

; 一時変数などの復元


	__ldw <__tmr_temp
	__stw <__temp

	__ldw <__tmr_ptr
	__stw <__ptr

	__ldw <__tmr_bx
	__stw <_bx

	ply
	plx
	pla

	cli
	rti
;  .endp

;  .proc _vsync_drv

; VSYNC割り込み
_vsync_drv:
	__ldw <__temp
	__stw <__vs_temp

	__ldw <__ptr
	__stw <__vs_ptr

	__ldw <_bx
	__stw <__vs_bx


	tma	#page(_drv_intr)
	pha
	map	_drv_intr
	jsr	_drv_intr
	pla
	tam	#page(_drv_intr)

	__ldw <__vs_temp
	__stw <__temp

	__ldw <__vs_ptr
	__stw <__ptr

	__ldw <__vs_bx
	__stw <_bx
    
	rts

;  .endp
           .zp
__tmr_temp   .ds 2
__tmr_ptr   .ds 2
__tmr_bx    .ds 2
__vs_temp   .ds 2
__vs_ptr   .ds 2
__vs_bx    .ds 2


	; restore segment and bank
	.code
	.bank DATA_BANK
;	.bank START_BANK

#endasm

/*  HuC's standard header */
#include "huc.h"
 /* HuSIC's core( main code ) */
#include "hus.c"

/* title */
#include "title.h"

display_title()
{
  cls();
  put_string(PRG_TITLE, 5, 2);  
}

select_song()
{
  char songno;
  char songmax;
  
  songno = 0;
  songmax = get_songmax();
  
  display_title();
  put_string("SELECT SONG", 5, 3);
  
  while(1) {
    vsync();
    put_hex(songno, 2, 5, 4);
    if (joytrg(0) & JOY_RGHT) {
      songno++;
      songno = songno % songmax; 
    } 
    if (joytrg(0) & JOY_LEFT) {
      songno--;
      songno = songno % songmax; 
    } 
    if (joytrg(0) & JOY_STRT) {
      break;
    }
  }
  return songno;
}

main()
{
	int i;
	int count, x, y;
  char songno;
  
  songno = 1;
  
	count = 0;

	disp_off();
	cls();

	/* font foreground color */

	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
	disp_on();

  songno = select_song();
  
  display_title();
  
  drv_init(songno);
  
	while(1)
	{
		vsync();
    
    /* 現在のポインタ、topbank、nowbank */
    /* put_hex(value, digits, x, y) */
		put_hex(seq_ptr, 4, 13, 3);
		put_hex(ch_topbank, 2, 20, 3);
		put_hex(ch_nowbank, 2, 23, 3);

		for(i = 0; i < 6; i++)
		{
			x = 2;

      /* トラック番号 */
			put_hex(i, 2, x, 5 + i);
			x += 2 + 1;

      /* コマンド */
			put_hex(ch_lastcmd[i], 2, x, 5 + i);
			x += 2 + 1;

      /* カウント */
			put_hex(ch_cnt[i], 2, x, 5 + i);
			x += 2 + 1;
      /* ノート番号 */
			put_hex(note_data[i], 2, x, 5 + i);
			x += 2 + 1;

      /* バンク */
			put_hex(ch_bank[i], 2, x, 5 + i);
			x += 2 + 1;
      /* ポインタ */
			put_hex(seq_pos[i], 4, x, 5 + i);
			x += 4 + 1;
      /* 周波数 */
			put_hex(seq_freq[i], 4, x, 5 + i);
			x += 4 + 1;

		}

    /* XPCM */
		for ( i = 0; i < 2; i++)
		{
			x = 1;
			put_hex(xpcm_addr[i], 4, x, 14 + i );
			x += 4 + 1;
			put_hex(xpcm_len[i], 4, x, 14 + i );
		}
	}

}
