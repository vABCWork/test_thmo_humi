#include	<machine.h>
#include	 "iodefine.h"
#include	 "misratypes.h"
#include	"delay.h"
#include	 "spi_9bit.h"
#include	 "ILI9488_9bit.h"
#include	"timer.h"
#include	"riic.h"
#include	 "disp_number.h"
#include	"key.h"
#include	"debug_port.h"

void clear_module_stop(void);
void debug_port_ini(void);

float	temp_fahrenheit;

void main(void)
{
	
	clear_module_stop();	//  モジュールストップの解除
	
	initSCI_5();		// SCI5(簡易SPI)  初期設定
	
	RSPI_Init_Port();	// RSPI ポートの初期化  (LCDコントローラ用)   
     	RSPI_Init_Reg();        // SPI レジスタの設定  

     	RSPI_SPCMD_0();	        // SPI 転送フォーマットを設定, SSLA0使用	
	
	ILI9488_Reset();	// LCD のリセット	
	 
	ILI9488_Init();		// LCDの初期化
	
	delay_msec(10);		// LCD(ILI9488)初期化完了待ち
	
	disp_black();		// 画面　黒  ( 106 [msec] at 16[MHz] )

	RIIC0_Port_Set();	//  I2C(SMBus)インターフェイス用のポート設定	
	RIIC0_Init();		//  I2C(SMBus)インターフェイス の初期化
					
	RIIC0_Init_interrupt();	// RIIC0 割り込み許可 
	
	delay_msec(100);	// センサ安定待ち (100msec 待つ) 
	
	Timer10msec_Set();      // タイマ(10msec)作成(CMT0)
     	Timer10msec_Start();    // タイマ(10msec)開始　
	
	
	disp_percent();		// %の表示(48x96)

	disp_switch_square_id( 0, 128, 330);	// ℃ のボタン(64x64)
	disp_switch_square_id( 1, 128, 416);	// F のボタン(64x64)
	
	xpt2046_cmd_set();             // タッチコントローラ XPT2046用 コマンド設定　

	iic_slave_adrs = 0x38;          //  スレーブアドレス = 0x3B (温湿度センサ AHT25)
	wr_sensor_cmd();	        //  マスタ送信の開始(トリガコマンドの発行)(最初)
	
	touch_pre_status_ini();		// 全て非タッチ(SW OFF: High = 1 )とする
	
	debug_port_ini();		// debug用
	
	while(1){
           
	   if ( flg_100msec_interval == 1 ) {  // 100msec経過
               
		 
	       flg_100msec_interval = 0;	       // 100msec経過フラグのクリア
	   
	       
	       if ( timer_10msec_cnt == 0 ) {  		
		   
	           wr_sensor_cmd();	     //　マスタ送信の開始(トリガコマンドの発行)
	       	   
	        }
	        else if ( timer_10msec_cnt == 10 ) { 
		      
	           rd_sensor_humi_temp();    // マスタ受信の開始(温湿度データの読み出し)  
	      
	        }
		else if ( timer_10msec_cnt == 20 ) {
		
		    Cal_humidity_temperature();	//  湿度と温度を計算
				
		}
	       
	                                         // 100mse毎の共通処理
	       key_input();			
	   
	       if ( flg_disp_fahrenheit == 0 ) {      //　℃(摂氏)表示
	       
	           disp_float_data(float_sensor_temperature, 60, 60, 1, COLOR_WHITE );   // 温度表示 サイズ(48x96) 
		   disp_celsius();		     // ℃の表示(48x96)
	       }
	       else {					// F(華氏)表示
		   temp_fahrenheit = float_sensor_temperature * 1.8 + 32.0; 
		   
	           disp_float_data(temp_fahrenheit, 60, 60, 1, COLOR_WHITE );   // 温度表示 サイズ(48x96) 
	           disp_fahrenheit();			// Fの表示 (48x96)
	       }
      
	       disp_float_data(float_sensor_humidity, 100, 200, 0, COLOR_WHITE);   // 湿度表示 サイズ(32x64) 
	       
	   }
	   
	   
	}  // while(1)
	
	
	
}




// モジュールストップの解除
// コンペアマッチタイマ(CMT) ユニット0(CMT0, CMT1) 
//  シリアルペリフェラルインタフェース0(RSPI) (LCDコントローラ ILI9488との通信用)(SPI通信)
//  シリアルコミュニケーションインタフェース5(SCI5)(タッチコントローラ XPT2046 との通信用)(簡易SPI通信)
//  I2C バスインタフェース(RIICa)
//  CRC 演算器（CRC）(RIIC I2C通信用)
//
void clear_module_stop(void)
{
	SYSTEM.PRCR.WORD = 0xA50F;	// クロック発生、消費電力低減機能関連レジスタの書き込み許可	
	
	MSTP(CMT0) = 0;			// コンペアマッチタイマ(CMT) ユニット0(CMT0, CMT1) モジュールストップの解除
	
	MSTP(RSPI0) = 0;		// シリアルペリフェラルインタフェース0 モジュールストップの解除
	
	MSTP(SCI5) = 0;			// SCI5    モジュールストップの解除:
	
	MSTP(RIIC0) = 0;                //  RIIC0モジュールストップ解除 (I2C通信)

	MSTP(CRC) = 0;			// CRC モジュールストップの解除
	
	SYSTEM.PRCR.WORD = 0xA500;	// クロック発生、消費電力低減機能関連レジスタ書き込み禁止
}




//   デバックポートの設定 
//   (debug_port.h)

void debug_port_ini(void)
{	
        DEBUG_0_PMR = 0;    //  P15 汎用入出力ポート
	DEBUG_0_PDR = 1;    //  出力ポートに指定
	
}
