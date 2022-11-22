#include "iodefine.h"
#include "misratypes.h"

#include "spi_9bit.h"


volatile uint8_t *spi_send_pt;		// 送信データを格納している場所

volatile uint8_t spi_cmd_flg;		// コマンド送信時　= 1, 表示用データまたはコマンドのパラメータ送信時=0

volatile uint8_t spi_sending_fg;	// 送信中 = 1

volatile uint32_t spi_send_num;	// 送信するバイト数



// RSPI0 SPTI0
// 送信バッファエンプティ	割り込み
//
#pragma interrupt (Excep_RSPI0_SPTI0(vect=46))
void Excep_RSPI0_SPTI0(void){
	
	uint32_t t_sd;

	if (spi_send_num > 0  ) { 		//  送信データがある場合
		t_sd = (uint32_t)(*spi_send_pt);
		
		if ( spi_cmd_flg == 1 ){        // コマンド送信時
		    t_sd = ( 0x0000 | t_sd );	// D/Cビット=0
		    spi_cmd_flg = 0;		// コマンド送信フラグのクリア (コマンドは最初の1バイトだけのため）
		}
		else {				// 表示データまたはコマンドのパラメータ送信時
		     t_sd = ( 0x0100 | t_sd );	// D/Cビット=1 
		}
		RSPI0.SPDR.LONG = t_sd;        // 送信データセット
	
		spi_send_num = spi_send_num  - 1;	// 送信回数をデクリメント
		spi_send_pt++;				// 送信位置の更新
		
		if ( spi_send_num == 0 ) {	// 最終送信データをセット済み
		    RSPI0.SPCR.BIT.SPTIE = 0;   // 送信バッファエンプティ割り込み要求の発生を禁止
                    RSPI0.SPCR2.BIT.SPIIE = 1;	// アイドル割り込み要求の生成を許可
		}
		
	}
	
}

// RSPI0 SPII0
// アイドル割り込み
#pragma interrupt (Excep_RSPI0_SPII0(vect=47))
void Excep_RSPI0_SPII0(void){
	
	RSPI0.SPCR.BIT.SPE = 0;         // RSPI機能は無効
	RSPI0.SPCR2.BIT.SPIIE = 0;	// アイドル割り込み要求の生成を禁止
	
	spi_sending_fg = 0;		// SPI送信中のフラグのクリア
}



// RSPI ポートの初期化  (LCDコントローラ用)
// 　マスタ: マイコン
//   スレーブ:  LCDコントローラ(ILI9488)
//
// PC6 : MOSI     ,SPI_MOSI ,マスタ出力 スレーブ入力   
// PC5 : RSPCKA   ,SPI_CLK  ,クロック出力  
// PC4 : SSLA0    ,DISP_CS  ,スレーブセレクト 
// PH3 : port出力 ,DISP_RST, LCDコントローラ(ILI9488) Reset用 
//
// ILI9488 DBI Type C Serial Interface Option1 の場合、コマンド/データ判別は、送信データの先頭ビットとしているので、
// ポート出力によるコマンド/データ判別は必要ない。
// (PC7 : port出力 ,DISP_DC ,LCDコントローラ(ILI9488)  コマンド/データ判別用　(コマンド=low,データ=high))  
//

void RSPI_Init_Port(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;  	 // マルチファンクションピンコントローラ　プロテクト解除
        MPC.PWPR.BIT.PFSWE = 1;  	// PmnPFS ライトプロテクト解除
    
	MPC.PC6PFS.BYTE = 0x0d;		// PC6 = MOSIA   Master Out slave in
	MPC.PC5PFS.BYTE = 0x0d;	  	// PC5 = RSPCKA
	MPC.PC4PFS.BYTE = 0x0d;		// PC4 = SSLA0
	
        MPC.PWPR.BYTE = 0x80;      	//  PmnPFS ライトプロテクト 設定	
	
        PORTC.PMR.BIT.B6 = 1;     	// PC6  周辺機能として使用
	PORTC.PMR.BIT.B5 = 1;     	// PC5  
	PORTC.PMR.BIT.B4 = 1;     	// PC4
	
	PORTC.DSCR.BIT.B6 = 1;     	// PC6 高駆動出力
	PORTC.DSCR.BIT.B5 = 1;     	// PC5  
	PORTC.DSCR.BIT.B4 = 1;     	// PC4
	
	
	LCD_RESET_PMR  = 0;     	// PH3:汎用入出力ポート ,LEDコントローラ Reset用 
	LCD_RESET_PDR = 1;      	// PH3: 出力ポートに設定 LEDコントローラへのReset
	
      
}




// RSPI レジスタの初期設定 
//ビットレート:
//  RSPI コマンドレジスタ0 ～ 7（SPCMD0～7)のBRDV[1:0] ビットとSPBR レジスタの設定値(n)の組み合わせできまる。
// 
//  現在、RSPI0.SPCMD0のBRDV=0 ,   PCLKB= 32MHzとしている。
//   ビットレート = PCLKB / 2*(SPBR+1)
//   
//
//     SPBR = 0 で、32/2 = 16 [MHz]
//     SPBR = 1 で、32/4 = 8 [MHz]
//

void RSPI_Init_Reg(void)
{

	RSPI0.SPCR.BIT.SPMS = 0;        // SPI動作 (4線式)	
	RSPI0.SPCR.BIT.TXMD = 1;        // 送信動作のみ行う。受信なし
	RSPI0.SPCR.BIT.MODFEN = 0;      // モードフォルトエラー検出を禁止
	
	RSPI0.SPCR.BIT.MSTR = 1;	//  マスタモード
	
	
	RSPI0.SPCR.BIT.SPEIE =0;       // エラー割り込み要求の発生を禁止
	RSPI0.SPCR.BIT.SPTIE =0;       // 送信バッファエンプティ割り込み要求の発生を禁止
	RSPI0.SPCR.BIT.SPE = 0;        // RSPI機能は無効
	RSPI0.SPCR.BIT.SPRIE = 0;      // RSPI受信バッファフル割り込み要求の発生を禁止
	
	RSPI0.SSLP.BYTE = 0;                // スレーブセレクト極性 SSL0～SSL3 アクティブLow
	RSPI0.SPPCR.BYTE = 0;               //  通常モード
	
	RSPI0.SPSCR.BYTE = 0;               // 転送フォーマットは、常にSPCMD0 レジスタを参照

	
	
	// ILI9488 17.4.2. DBI Type C Option 1 (3-Line SPI System) Timing Characteristics より、Serial clock cycle (Write) = 66nsec (min)
	// 書き込み時には、15MHzが最高だが、16[MHz]を試している。
	
	//RSPI0.SPBR = 0x1;		   // 8 MHz
	
	RSPI0.SPBR = 0x0;		   // 16 MHz
	
	RSPI0.SPDCR.BIT.SPFC =0x00;    // SPDR レジスタに格納できる（1 回の転送起動）フレーム数 1 =(32bit, 4byte)
	RSPI0.SPDCR.BIT.SPRDTD = 0;     // 0：SPDRは受信バッファを読み出す
	RSPI0.SPDCR.BIT.SPLW = 1;	// RSPIデータレジスタ(SPDR)はロングワード(32bit)のアクセス

                                       // SPCMD0.BIT.SCKDEN = 1の場合の、
	RSPI0.SPCKD.BYTE = 0;               // セレクト信号からクロック発生までの遅延時間　1RSPCK
				       // SPCMD0.BIT.SCKDEN = 0の場合、セレクト信号からクロック発生までの遅延時間は、1RSPCK
			
	RSPI0.SSLND.BYTE = 0;          // SPCMD0.BIT.SLNDEN = 1の場合の、
	                              // クロック最終エッジから、セレクト解除までの遅延時間
	                              // SPCMD0.BIT.SLNDEN = 0の場合、1RSPCK
				      
	RSPI0.SPND.BYTE = 0;	     // SPCMD0.BIT.SPNDEN = 1の場合の、シリアル転送終了後の、セレクト信号非アクティブ期間（次アクセス遅延)　1RSPCK＋2PCLK	      
				     // SPCMD0.BIT.SPNDEN = 0の場合、1RSPCK＋2PCLK
	
        RSPI0.SPCR2.BYTE = 0;        // パリティなし　アイドル割り込み要求の発生を禁止　アイドル割り込み要求の発生を禁止

					// 送信データエンプティ割り込み
	IPR(RSPI0,SPTI0) = 0x06;	// 割り込みレベル = 6　　（15が最高レベル)
	IR(RSPI0,SPTI0) = 0;		// 割り込み要求のクリア
	IEN(RSPI0,SPTI0) = 1;		// 割り込み許可	
	
					// アイドル割り込み
	IPR(RSPI0,SPII0) = 0x06;	// 割り込みレベル = 6　　（15が最高レベル)
	IR(RSPI0,SPII0) = 0;		// 割り込み要求のクリア
	IEN(RSPI0,SPII0) = 1;		// 割り込み許可				      
}






//     SPCMD 0 の設定
//   
//     SPI モード3: クロックアイドル時 High
//                :  (クロック立ち上がり時にデータサンプル)

//　　　SSL信号   : SSLA0

void RSPI_SPCMD_0(void)
{

					// SPI モード 3 : 表示 OK
	RSPI0.SPCMD0.BIT.CPOL = 1;	// CPOL= 1 負パルス(クロックアイドル時 High)				
	RSPI0.SPCMD0.BIT.CPHA = 1;	// CPHA= 1 奇数エッジでデータ変化、偶数エッジでデータサンプル (エッジは1から数える）

	
	RSPI0.SPCMD0.BIT.BRDV = 0;	// BRDV= 0 ベースのビットレート使用 (分周なし)
	RSPI0.SPCMD0.BIT.SSLA = 0;	// SSLA= 0 SSL0を使用
	
	RSPI0.SPCMD0.BIT.SSLKP = 0;	// 転送終了後(このプログラムの場合、1byte転送)全SSL信号をネゲート
	
	RSPI0.SPCMD0.BIT.SPB = 0x08;	//  SPB= 8  送信 データは 9 bit
	RSPI0.SPCMD0.BIT.LSBF = 0;	// LSBF= 0  MSB first
	
	RSPI0.SPCMD0.BIT.SPNDEN = 0;	// SPNDEN=0 次アクセス遅延は、1RSPCK + 2PCLK
	RSPI0.SPCMD0.BIT.SLNDEN = 0;	// SLNDEN=0 SSLネゲート遅延は 1RSPCK
	RSPI0.SPCMD0.BIT.SCKDEN = 0;	//  SCKDEN=0 RSPCK遅延は 1RSPCK
	
}





// LCDコントローラ(ILI9488)へのデータ送信 　(送信割り込み開始)
//  
//  入力:    sd_num :送信データバイト数 
//          *data_pt: 送信データの格納位置
//　　　　　cmd_flg : 1=コマンド、パラメータ送信の場合
//                    0=表示データ送信の場合
//


void spi_data_send( uint32_t sd_num, uint8_t *data_pt, uint8_t cmd_flg)
{
	
	spi_send_pt = data_pt;   		// 送信 割り込みで送信するデータの先頭位置
	spi_send_num = sd_num;			// 送信割り込みでの送信数
	
	if ( cmd_flg == 1 ) {			// コマンド,パラメータ送信の場合
		spi_cmd_flg = 1;	
	}
	else {					// 表示データ送信の場合
		spi_cmd_flg = 0;
	}
	
	spi_sending_fg = 1;			// SPI送信中のフラグセット
	
        RSPI0.SPCR2.BIT.SPIIE = 0;		// アイドル割り込み(SPII)の禁止
	RSPI0.SPCR.BIT.SPTIE = 1;       	// 送信バッファエンプティ割り込み要求の発生を許可
						// (送信開始時の送信バッファエンプティ割り込み要求は、SPTIE ビットと同時または後に、SPE ビットを“1” にすることで発生します。(page 1010))
	RSPI0.SPCR.BIT.SPE = 1;        		// RSPI 機能有効

	spi_data_send_wait();			// 全データの送信完了待ち
	
}




//
// 全データの送信完了待ち
//
void  spi_data_send_wait(void)
{
	while( spi_sending_fg == 1) {		// 連続して送信する場合の、送信完了待ち
	
	}
	
}
