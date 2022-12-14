// 令和4年10月28日（2022年10月28日）
// 上間つよし＠沖縄県西原町＠久米島出身
// ATtiny85 1MHz 工場出荷時
//
//
//                    ---------
//                 --|RESET VCC|--5[v]
//   ( LED回路    )--|PB3   PB2|--( アンテナ入力 )
//   ( ブザー回路 )--|PB4   PB1|--( アンテナ入力 )
//   0[v]ーーー------|GND   PB0|--( アンテナ入力 )
//                    ~~~~~~~~~
//
//                                                          5[v]
//                                                            |
//                                                          [680]
//                                                            |
//                                                           LED
//                                                            |
//                                                    +-------+----------( マイコンポート )
//                                                    |       |
// ------+-((((((((((((-+--------                     |C      |
//       |              |                          B /        |
//       |              +--|1S2076A>--[220]---------|         |
//       |                                           \.       |C
//       |                                             |   B /
//       |                                             +----|
//       |                                                   \.
//       |                                                    |E
//       |                                                    |
//       |                                                    |
//       +----------------------------------------------------+------------( GNG )
//                                                            |
//                                                          0[v]
//
//
//
//
//5[v]-+--Buzer------+------( マイコンポート )
//            |             |
//            +--[10k]------+
//            |             |
//            +--<1S2076A|--+
//
//
//
//
//
//       5[v]----[680]---LED-----( マイコンポート )
//

/*--------------------------------------------------*/
/*        開始                                      */
/*--------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "soft_timer.h"

volatile unsigned char f_1ms = 0; /* フラグ 1[ms] */

#define Buzer       PB4
#define LED         PB3
#define Antena2     PB2
#define Antena1     PB1
#define Antena0     PB0

int main(void);
void initialize( void );
ISR(TIM0_COMPA_vect);

int main(void)
{
	t_ini();
	initialize();
	sei();

	while(1)
	{
		t_main();

		if( !( (PINB & 0b111) == 0b111 )  ) {
			PORTB &= ~( (1 << LED)|(1 << Buzer) );  /* オン */

			t_req( 0 );   /* softTimer=1[s]リクエスト */
		}

		if( t_tup( 0 ) ) {
			t_can( 0 );  /* softTimer=1[s]キャンセル */
			PORTB  |= (1 << LED)|(1 << Buzer);  /* オフ */
		}

		while( ! f_1ms ) ;  /* タイミング調整 */
		f_1ms = 0;          /* フラグ＿リセット */
	}
}

/*--------------------------------------------------*/
/*        初期化                                    */
/*--------------------------------------------------*/
void initialize( void )
{
	TCCR0A = 1 << WGM01;  /* CTC_Timer */
	TCCR0B = 1 << CS01;   /* ck/8 */
	OCR0A  = 1000000 / 8 / 1000 - 1; /* 1[ms] */
	TIMSK  = 1 << OCIE0A;  /* 比較一致A割込み許可 */
	TCNT0=0;
	DDRB   |= (1 << LED)|(1 << Buzer);  /* 出力 */
	PORTB  |= (1 << LED)|(1 << Buzer);  /* オフ */
}

/*--------------------------------------------------*/
/*        タイマー割り込み０CTC動作                 */
/*--------------------------------------------------*/
ISR(TIM0_COMPA_vect)
{
	f_1ms = 1;
}

/*--------------------------------------------------*/
/*        終了                                      */
/*--------------------------------------------------*/

