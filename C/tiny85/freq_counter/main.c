/* このファイルはDoxygen自動資料生成用に準備されています。*/
/*! \file *********************************************************************
 *
 * \brief
 *		tiny x61/mega x8周波数計数器
 *
 *		2つのタイマ/カウンタと2つの割り込み源を用いた周波数計数器: FREQ_CNTRはPB6上の上昇入力を数えます。
 *		PB6のピン変化割り込みが入力の上昇端遷移の計数を開始します。
 *		GATE_CNTRは入力信号が計数される時間長を決める0.1秒基準時間を生成します。
 *		GATE_CNTRはそれが溢れる時にそのような値で予め設定され、OVF割り込みが生成されて計数器が停止されます。
 *		これは16ビットのFREQ_CNTRの結果に従ってfreq_cntr_freq_divd_by_10と記されたSRAM位置に書かれます。
 *
 *		出力ピンPB5は0.1秒計数周期の始めにHigh、後でLowに設定されます。
 *		コード性能での変化なしを望む場合、PB5に関連する命令が取り去られるかもしれません。
 *
 *		この周波数測定算法の精度は99%またはそれ以上で、AVRの8MHzクロック精度の関数です。
 *		各応用はこの指示経由でPB5のHighとPB5のLowの測定によってより高い精度に調整することができます。
 *
 *		#define DELAY_VAL_100_MS  0xFFFF - (5560 * AVR_Clk_freq) // 0.1秒開門時間の大小に対しては5560の数を調整してください。
 *
 * \par Documentation
 *		包括的なコード資料、支援コンパイラ、コンパイラ設定、支援デバイスについてはreadme.htmlをご覧ください。
 *
 * \author
 *		Atmel Corporation.
 *		支援Eメール: avr@atmel.com
 *
 *
 * 著作権 (c) 2010, Atmel Corporation 全権利予約済み。
 *
 * 変更の有りまたはなしでソースと2進の形式での使用と再配布は以下の条件に合っていれ
 * ば許されます。
 *
 * 1. ソース コードの再配布は上の著作権通知、この条件一覧、それと以下のお断りを維持し
 *	  なければなりません。
 *
 * 2. 2進形式での再配布はこの配布で提供された資料や他の素材で、上の著作権通知、
 *	  この条件一覧、それと以下のお断りを再現しなければなりません。
 *
 * 3. ATMELの名称は先に書かれた許諾の指定なしにこのソフトウェアから派生した販促製品や
 *	  裏書(保証)に使用できないかもしれません。
 *
 * このソフトウェアはどんな明示的または暗示的な保証、包含物の制限なしの"現状そのまま"
 * でATMELによって提供され、特定目的のための市場性と適合性の暗黙的な保証は明白
 * 且つ明確に放棄されます。たとえATMELがそのような損害賠償の可能性を進言された
 * としても、本資料を使用できない、または使用以外で発生する(情報の損失、事業中
 * 断、または利益の損失に関する制限なしの損害賠償を含み)、直接、間接、必然、偶
 * 然、特別、または付随して起こる如何なる損害賠償に対しても決してATMELに責任が
 * ないでしょう。
 *****************************************************************************/

/*! \page doc_page1 AVRへの信号源接続と結果観測の方法\n
 * tiny x61が望まれたAVRなら、このAVR Studioプロジェクトが開かれた後で、
 *	- Project⇒Configuration Options⇒Device attiny 261, tiny461またはattiny861をクリックしてください。
 *	- 測定されるべきデジタル信号をAVRのPB6ピンに接続してください。
 *	- 信号の振幅はAVRのVihとVilの電圧に適合させるべきです。
 *	- JTAGICEmkⅡデバッガを使用し、この.cコードで各々の結果が決められた後に中断点を設定することが示唆されます。\n
 *
 * MEGAが望まれたAVRなら、このAVR Studioプロジェクトが開かれた後で、
 *	- Project⇒Configuration Options⇒Device mega48, mega88またはmega168をクリックしてください。
 *	- 測定されるべきデジタル信号をAVRのPD7ピンに接続してください。
 *	- 信号の振幅はAVRのVihとVilの電圧に適合させるべきです。
 *	- JTAGICEmkⅡデバッガを使用し、この.cコードで各々の結果が決められた後に中断点を設定することが示唆されます。\n
*/

#include <avr/io.h>
#include <compat/ina90.h>		// ここでは_NOP, _CLI, _SEIが定義されます。
#include <avr/interrupt.h>
#include "freq_meter.h"

unsigned int	freq_cntr_result;
//unsigned int	freq_cntr_freq_divd_by_10;
// 原型
void users_init(void);

int main() {
  freq_cntr_init();
  users_init();						//!< 使用者は以下で定義されるこの関数内にそれら自身のコードを挿入してください。
  _SEI();							//! 全体割り込み許可

  freq_cntr_start_measurement();	// 周波数を測定するために単純にmeasure_frequency_via_interrupts()を実行
  while (1)
  {
	// 使用者のコードがここに挿入されるべきです。
	if(freq_cntr_get_result() !=0) {

	freq_cntr_result = freq_cntr_get_result();
	freq_cntr_clear_result();

	_NOP();							// 使用者のコードがここに挿入されるべきです。

	freq_cntr_start_measurement();	// 別の測定開始

	}
	_NOP();							// 使用者のコードがここに挿入されるべきです。
  }
} // main終了


/*! \brief 使用者の初期化関数
 *
 * この関数は接続された周辺機能に命令バイトを送り、状態符号が返されるのを待ちます。
 *
 *  \note 周辺機能は先に初期化されなければなりません。
 *
 */
void users_init(void)
{
  _NOP();							// 使用者はここに初期化コードを挿入します。
}