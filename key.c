#include "iodefine.h"
#include  "misratypes.h"
#include  "timer.h"
#include  "key.h"

#include "sci_spi.h"


volatile struct  SW_info  Key_sw[KEY_SW_NUM];	// スイッチ　6個分の情報格納領域


// X軸、Y軸kの測定値の平均処理用
// 測定値は12bitデータだが、上位 8bitだけ見ている

uint8_t   touch_x_val;		// X軸データ
uint8_t   touch_y_val;          // Y軸データ

uint8_t   touch_z1_val;		// Z方向　(タッチ圧測定用)
uint8_t   touch_z2_val;

uint8_t   tc_val_pt;		// 測定値の格納位置

uint8_t   touch_x[8];		// 8回分の測定値
uint8_t   touch_y[8];


uint16_t   touch_x_average;	// 平均値
uint16_t   touch_y_average;


float	touch_resistance;	// タッチ抵抗

float   touch_z1_z2;		// z2/z1


//  ℃、F　判別用
uint8_t  flg_disp_fahrenheit;   // 1:(F 華氏表示)

// 
// タッチ状態の初期化 (  電源ON直後に1回実施 )
//  
//   全て非タッチ(SW OFF: High = 1 )とする
//   測定値の格納位置の初期化
//
void touch_pre_status_ini(void)
{
	uint32_t i;
		
	for ( i = 0 ; i < KEY_SW_NUM; i++ ) {		//  全て非タッチ(SW OFF: High = 1 )とする
	
	     Key_sw[i].pre_status = 1;
	
	}
	
 	tc_val_pt = 0;		// 格納位置のクリア	
}


// X軸、Y軸kの測定結果の平均値を得る (プログラム作成用)
//　押されたキーを判定するプログラム作成時に、必要なデータを得る。
//(実際のキー入力判定処理には平均値は使用していない)
// 12bitデータの上位 8bitを使用 tc_val_pt
void xpt2046_cal_average(void)
{
	uint32_t  i;
	uint32_t  x_avg;
	uint32_t  y_avg;
	

	touch_x[tc_val_pt] =  xpt_rcv_data[1];   // X軸測定データの (b11-b4)
	touch_y[tc_val_pt] =  xpt_rcv_data[4];   // Y軸測定データの (b11-b4)
	
	x_avg = 0;			// 平均値の計算
	y_avg = 0;
	
	for ( i = 0 ; i < 8 ; i++ ) {	// 8回分の総和を得る 
	      x_avg = x_avg + touch_x[i];
	      y_avg = y_avg + touch_y[i];
	 }
	   
	touch_x_average =  x_avg >> 3;   // 割る 8
	touch_y_average =  y_avg >> 3;   // 割る 8
	

	if ( tc_val_pt < 7 ) {		// 
	
	   tc_val_pt = tc_val_pt + 1;	// 格納位置のインクリメント
	}
	else  {				
	  
	   tc_val_pt = 0;		// 格納位置のクリア	
	}
	 
}



//
//   X,Y,Z1,Z2を得るまた、抵抗値を計算する。
//
// タッチパネルのZ方向(フィルムとその下にあるLCDガラスとの上下方向)の抵抗値(Rz)を得る
//  タッチされていない状態では、大きい値になる。
//
//     Rz = Rx_plate * ( X軸測定値 /4096 )* ( ( Z2 / Z1 ) - 1 )
//
//    Rx_plate: タッチスクリーンのXプレート(フィルム側)の抵抗値 270[Ω] (実測)

void xpt2046_xyz_press(void)
{
	touch_x_val = xpt_rcv_data[1];	// X軸測定データの b11-b4
	touch_y_val = xpt_rcv_data[4];  // Y軸測定データの b11-b4
	
	touch_z1_val =  xpt_rcv_data[7];  // Z1 測定データの b11-b4
	
	touch_z2_val = xpt_rcv_data[10];  // Z2 測定データの b11-b4
	  
	if ( touch_z1_val > 0 ) {
	  //   touch_z1_z2 = ( touch_z2_val / touch_z1_val ); //  Z2/Z1 integer/integer 
	  touch_z1_z2 = (float)touch_z2_val / (float)touch_z1_val;  // modified. 2022-Dec-27
	   
	  touch_resistance = 270.0 * ( touch_x_val / 4096.0 ) * ( touch_z1_z2 - 1.0 );
        }
	else{
	   touch_resistance = 999.9;
	}		
}



// 　
// タッチキーの状態を得る
//  抵抗値が 20Ωを超える場合は、非タッチとする。
//  20Ω以下の場合、タッチされていると判断する。
// 
// X軸とY軸の読み出しデータにより、押されたキーを判断。
//
//  縦に配置:  ℃
//              F
//
// 　　　　　　    タッチ無し   ℃(SW0)         F (SW1)        　    
//  touch_x_val:　　 0x00      0x32～0x48     0x32～0x48   
//  touch_y_val:     0x7f      0x1b～0x29     0x06～0x15     
//
///
//


void touch_key_status_check(void)
{
	uint32_t i;
	

	if ( touch_resistance > 20.0 ) {		// 抵抗値が20Ωを超える場合、
	  for ( i = 0 ; i < KEY_SW_NUM; i++ ) {		//  全て非タッチ(SW OFF: High = 1 )とする
	     Key_sw[i].status = 1;
	  }
	  
	  return;  
	}
							// 押されたキーの判定
	if (( touch_x_val >= 0x32 ) && (  touch_x_val <= 0x48 )){        //  
	
	   if (( touch_y_val >= 0x1b ) && (  touch_y_val <= 0x29 )){   //   ℃キー(SW0) タッチ
	  	Key_sw[0].status = 0;				       // 押されたキーをタッチあり(SW ON: Low = 0 )とする
	    }
	    else if (( touch_y_val >= 0x06 ) && (  touch_y_val <= 0x15 )){     //   Fキー(SW1) タッチ
	  	Key_sw[1].status = 0;						
	    }
	}
	
}

//
// 　スイッチの入力判定 (20msecに1回　実行)
//
// 概要:
//   タッチ状態が4回継続後、非タッチにればタッチされたとする。
//
//    判定するスイッチ:
//                  0 = ℃のキー (SW0)  
//                  1 =  Fのキー (SW1) 
//                
//             
//  
void switch_input_check( uint8_t id ) 
{
	
         if ( Key_sw[id].status == 0 ) {     // 今回 タッチ状態
	
	     if ( Key_sw[id].pre_status == 1 ) {  // 前回 非タッチ状態　　(立下がり検出)
	           Key_sw[id].low_cnt =  1;       // Lowカウント = 1 セット
	     
	     }
	     else {				 // 前回 タッチ状態　
	          Key_sw[id].low_cnt = Key_sw[id].low_cnt + 1; // Lowカウントのインクリメント  
	     }
	     
	  } 

	  else{				// 今回　非タッチ状態
	   
	      if ( Key_sw[id].pre_status == 0 ) {  // 前回 タッチ状態 (立上がり検出)
	     
	          if ( Key_sw[id].low_cnt > 3 ) {   // 4回以上　タッチ検出の場合
	             
		      Key_sw[id].one_push = 1;	 // キー入力処理要求(1回押し)セット
		      
		      Key_sw[id].low_cnt = 0;	//  Lowカウントのクリア  
	          }
	      }
	  }	  
	  	
	  Key_sw[id].pre_status = Key_sw[id].status;   // 現在の状態を、一つ前の状態へコピー
	  
}
   


//
// キー入力処理
//   0 = SW0 , 温度の単位は、℃(摂氏)で表示
//   1 = SW1 , 温度の単位は、F(華氏)で表示
//
void key_input(void)	
{
	uint32_t    i;
	
	if (  Key_sw[0].one_push == 1 ) {	    // ℃ key
		
		flg_disp_fahrenheit = 0;
	       
	}
	
	else if ( Key_sw[1].one_push == 1 ) {	   // F key
		
		flg_disp_fahrenheit = 1;
		
	}
	
	
	for ( i = 0 ; i < KEY_SW_NUM ; i++ ) {	// スイッチ 一度押しの情報をクリア
		Key_sw[i].one_push = 0;
	}
	
	
}


