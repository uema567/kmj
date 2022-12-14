#include "soft_timer.h"

/* 60秒程度のソフトタイマ４本 */


unsigned int t_ini_val[ 4 ];         /* タイマ初期値      */
unsigned int t_cnt_val[ 4 ];         /* タイマカウンタ    */
const unsigned int t_const_val[ 4 ] = { 900,1000*60*60*60,700,60000 };

                                     /* ROM:固定値 */
/**********************************************************/
/*            初期化                                      */
/**********************************************************/
void t_ini( void )
{
	for( unsigned char i=0; i<4; ++i ) {
		t_ini_val[ i ] = t_const_val[ i ];
		t_cnt_val[ i ] = 0xffff;
	}
}

/**********************************************************/
/*            メイン                                      */
/**********************************************************/
void t_main( void )
{
	for( unsigned char i=0; i<4; ++i ) {
		if( ( t_cnt_val[ i ] != 0xffff ) && 
		 ( t_cnt_val[ i ] != 0x0000 ) )
			t_cnt_val[ i ]--; /* デクリメント */
	}
}

/**********************************************************/
/*            リクエスト                                  */
/**********************************************************/
void t_req( unsigned char i )
{
	t_cnt_val[ i ] = t_ini_val[ i ];
}

/**********************************************************/
/*            キャンセル                                  */
/**********************************************************/
void t_can( unsigned char i )
{
	t_cnt_val[ i ] = 0xffff;
}

/**********************************************************/
/*            タイムアップ                                */
/**********************************************************/
unsigned char t_tup( unsigned char i )
{
	return ( t_cnt_val[ i ] == 0x0000 )  ? 1 : 0;
}

/**********************************************************/
/*            休止中                                      */
/**********************************************************/
unsigned char t_off( unsigned char i )
{
	return ( t_cnt_val[ i ] == 0xffff )  ? 1 : 0;
}

/**********************************************************/
/*            カウントダウン中                            */
/**********************************************************/
unsigned char t_run( unsigned char i )
{
	return ( 
			( t_cnt_val[ i ] != 0xffff ) 
			&& 
			( t_cnt_val[ i ] != 0x0000 ) )  ? 1 : 0;
}

/**********************************************************/
/*             強制設定                                   */
/**********************************************************/
void t_set( unsigned char i, unsigned int cnt )
{
	t_cnt_val[ i ] = cnt;
}

