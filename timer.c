
#include "typedefine.h"
#include  "iodefine.h"
#include "misratypes.h"
#include "timer.h"
#include "key.h"


// Timer 
volatile uint8_t flg_100msec_interval;	// 100msec毎にON

volatile uint8_t timer_10msec_cnt;      //　(10msec毎にカウントアップ)


// タッチキー
volatile uint8_t flg_touch_cmd;		// タッチ読み出しコマンド発行フラグ (0=コマンド未発行, 1=コマンド発行済み)

//  コンペアマッチタイマ CMT0
//   10msec毎の割り込み
//

#pragma interrupt (Excep_CMT0_CMI0(vect=28))

void Excep_CMT0_CMI0(void){
	
	uint32_t i;

	timer_10msec_cnt++;	       // カウントのインクリメント
	
	if ( timer_10msec_cnt > 199 ) {    // 2秒経過
		
		timer_10msec_cnt = 0;	  //  カウンターのクリア
	}
	
	
	if ( (timer_10msec_cnt % 10 ) ==  0 ) {	   // 100msec経過 (10で割った余り = 0の場合)
		
		flg_100msec_interval = 1;  // 100msecフラグ ON
	}
	
	if ( flg_touch_cmd == 0 ) {	// コマンド未発行の場合
		
		xpt2046_comm_start();		//送受信開始  (2Mbpsで約50[usec]かかる )
		
		flg_touch_cmd = 1;	// コマンド発行済みフラグのセット	
	}
	else if ( flg_touch_cmd == 1 ){
	  	
		xpt2046_xyz_press();		// X,Y,Zの値を得る。タッチ圧を計算。
	 
	        touch_key_status_check();   // タッチキーの状態を得る
	 
		for( i = 0; i < KEY_SW_NUM; i++ ) {   // タッチ→非タッチ状態となったキーを見つけて、キー入力処理要求フラグをセットする。
		     switch_input_check(i);           //  (前回押されたキーを検索している)
	        }
		
		flg_touch_cmd = 0;		// コマンド発行済みフラグのクリア	
	}
	
	
 	// xpt2046_cal_average();			// X軸、Y軸kの測定結果の平均値を得る  (プログラム作成用)
	
}





//
//    10msec タイマ(CMT0)
//    CMTユニット0のCMT0を使用。 
//
//  PCLKB(=32MHz)の128分周でカウント 32/128 = 1/4 MHz
//      1カウント = 4/1 = 4 [usec]  
//    1*10,000 usec =  N * 4 usec 
//      N = 2500


void Timer10msec_Set(void)
{	
	IPR(CMT0,CMI0) = 3;		// 割り込みレベル = 3　　（15が最高レベル)
	IEN(CMT0,CMI0) = 1;		// CMT0 割込み許可
		
	CMT0.CMCR.BIT.CKS = 0x2;        // PCLKB/128       
	CMT0.CMCOR = 2499;		// CMCNTは0からカウント 	


}


//   CMT0 タイマ開始　
//  割り込み許可してカウント開始

void Timer10msec_Start(void)
{	
	CMT0.CMCR.BIT.CMIE = 1;		// コンペアマッチ割込み　許可
		
	CMT.CMSTR0.BIT.STR0 = 1;	// CMT0 カウント開始

	timer_10msec_cnt = 0;	  //  カウンターのクリア

}







