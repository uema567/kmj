/////////////////////////////////////////////////////////////////////////////
//
//   防犯システム
//
//   人体検知　→　　ブザー鳴動  
//   * 音がなるまで１０秒の遅延がある                        *
//   * １分鳴る。鳴り終わったら再度、監視状態になる。        *
//   * センサー反応回数を１個のLEDで表示する。               *
//
//
/////////////////////////////////////////////////////////////////////////////
//
//   使い方：開始: ボタンを２回押す  -> ６０秒以内に退避
//   使い方：終了: １０秒以内に、ボタンを２回押す。
//   使い方：ボタンを１回押すと反応回数を表示
//   使い方：ボタンを３回押すと反応回数をリセット
//
//
//
/////////////////////////////////////////////////////////////////////////////
//
//                         Tiny13A
//                        ---------
//                     --|Reset VCC|-----------------------------------5[v]
//  0[v]---[680]---LED---|PB3   PB2|--                     +--------------+
//  0[v]-----------SW----|PB4   PB1|----------ーー---------| 動体センサー |
//  0[v]-----------------|GND   PB0|-----以下ブザー回路    +--------------+
//                        ---------
//
/////////////////////////////////////////////////////////////////////////////
//
//
//
//   令和4年10月19日（2022年10月19日） 
//
//   上間つよし＠沖縄県西原町＠久米島出身
//
//   tiny13A  クロック内蔵1.2[MHz]
//
//   
/////////////////////////////////////////////////////////////////////////////
//
//  動体センサー AKE-1
//
//  LM358N
//  1: OUT1    8: VCC
//  2: IN1(-)  7: OUT2
//  3: IN1(+)  6: IN2(-)
//  4: GND     5: IN2(+)
//
//                         ------
//   9[v]----+-----+------| 7805 |-----+---------+---A
//           | +   |       ------      |         | + 
//   100uF  ---   ---0.1uF   |        ---0.1uF  --- 10uF
//          ---   ---        |        ---       ---  
//           |     |         |         |         |   
//           |     |         |         |         |   
//    0[v]---+-----+---------+---------+---------+---GND
//
//                     -----
//     A----[10K]-+---|AKE-1|---+----------------LM358N:IN1+  LM358N:OUT2 -100K-+-----B
//                |    -----    |    +-----------LM358N:IN1-  LM358N:IN2- ------+
//               ---0.1uF|      |    +----470K-+-LM358N:OUT2  LM358N:IN2+ --+ [4.7K]
//               ---     |    [47K][4.7K]      |                            |   |
//                |      |      |    |         +----------------------------+  ---47uF
//                |      |      |   ---47uF                                    ---
//                |      |      |   ---                                         |
//                |      |      |    |                                          |
//      GND-------+------+------+-----------------------------------------------+-----GND
//
//
//       B------+--------->ATtiny13A:PB1
//              |
//              |
//              |
//             10M
//              | 
//              | 
//              | 
//       GND----+----------GND
//
//
/////////////////////////////////////////////////////////////////////////////
//
//  ブザー回路
//
//                         C     E
//   5[v]--+--Sounder--+----     --------------+--------0[v]
//         +--[10k]----+    \   /              |
//         +--Diode----+     --- 2SC1815       |
//            (1S2076A)       |B               |
//                            +-----[56k]------+
//                            |
//                            +-----[4.7k]---------------ATtiny13A:PB0
//
/////////////////////////////////////////////////////////////////////////////
//
//    上と同じ
//
//    GND-680-LED---ATtiny13A:PB3
//    GND-----SW----ATtiny13A:PB4
//
//
/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////


/****************************************************************/
/*		START						*/
/****************************************************************/

/* ヘッダファイル */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "sw.h"
#include "soft_timer.h"

/* グローバル変数 など */
#define  Buzer   0          /* OUT */
#define  Sensor  1          /* IN  */
#define  LED     3          /* OUT */
#define  SW      4          /* IN  */


//static unsigned char EEMEM EEPROM_V1;          /* EEPROM_V1 　を宣言 */
//static unsigned char EEMEM EEPROM_V2;          /* EEPROM_V2 　を宣言 */

volatile unsigned char f_1ms    = 0;
         unsigned char Bouhan   = 0;   /* 1: 動作中 */
         unsigned char sw_click = 0;   /* スイッチ押した回数*/


/* プロトタイプ宣言 */
int main( void );
void initialize( void );
ISR(TIM0_COMPA_vect);

/* Main関数 */
int main( void )
{

	initialize();  /* 初期化： 一般                 */
	t_ini();       /* 初期化： ソフトタイマー       */
	sw_ini();      /* 初期化： ３回照合スイッチ入力 */

	sei();

	while( 1 ) {
		sw_main();
		t_main();

		if( ( PINB & ( 1<<Sensor )) && ( Bouhan ) ) {
			t_req( 0 );   /* タイマー０：10[s]リクエスト */
		}

		if( t_tup( 0 ) ) {   /* センサーが反応して１０秒経過 */
			t_can( 0 );  /* タイマー０キャンセル */
			t_req( 1 );  /* タイマー１: 60[s]リクエスト */
			PORTB |= 1<<Buzer;    /* ブザー鳴動 */
		}	

		if( t_tup( 1 ) ) {   /* ブザーがなってから60[s]経過 */
			t_can( 1 );  /* タイマー１キャンセル */
			PORTB &= ~( 1<<Buzer );    /* ブザー停止鳴動 */
		}

		////////////////  SW  ////////////////
		if( sw_dn & ( 1<<SW ) ) {
			sw_click++;
			t_req( 2 );
		}

		if( t_tup( 2 ) ) {
			if( sw_click == 1 ) {
				asm( "nop" );
			} else if( sw_click == 2 ) {

			} else if( sw_click == 3 ) {

			}

			t_can( 2 );
			sw_click = 0;
		}

		while( ! f_1ms ) ;    /* 1[ms] 待機     */
		f_1ms = 0;            /* フラグリセット */
	}
	return 0;
}

void initialize( void )
{
	TCCR0A = 1 << WGM01;          /* Timer:CTC:1[ms] */
	TCCR0B = 0b010 << CS00;       /* CK/8            */
	OCR0A  = 1000000/8/1000-1;    /* 1/1000=1[ms]    */
	TCNT0  = 0;
	TIMSK0  = 1 << OCIE0A;
	asm( "nop" );

	DDRB    = ( 1<<Buzer  ) | ( 1<<LED );    /* OUT     */
	PORTB   = ( 1<<Sensor ) | ( 1<<SW  );    /* Pull UP */

//	eeprom_busy_wait();
//	briteT1=eeprom_read_byte( &EEPROM_V1 );
//	eeprom_busy_wait();
//	briteT2=eeprom_read_byte( &EEPROM_V2 );
//	eeprom_write_byte(&EEPROM_V1, briteT1 );
//	eeprom_write_byte(&EEPROM_V2, briteT1 );
}

ISR(TIM0_COMPA_vect)
{
	f_1ms = 1;
}


/****************************************************************/
/*		END						*/
/****************************************************************/
