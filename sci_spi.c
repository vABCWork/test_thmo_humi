
#include "iodefine.h"
#include "misratypes.h"
#include "sci_spi.h"


//
//  SCI5 タッチコントローラ (XPT2046)との簡易SPI通信
//

// タッチコントローラ XPT2046との通信　受信用
volatile uint8_t  xpt_rcv_data[16];
volatile uint8_t  xpt_rcv_cnt;

volatile uint8_t  xpt_ovr_run_err;

//  タッチコントローラ XPT2046との通信　送信用
volatile uint8_t xpt_sd_data[16];
volatile uint8_t xpt_send_num;
volatile uint8_t xpt_send_pt;

volatile uint8_t sci5_rxi_cnt;	// デバック用
volatile uint8_t sci5_tei_cnt;  // デバック用




//
// 受信割り込み(SCI5) (タッチコントローラ XPT2046 通信用)
// 28.5.6 シリアルデータの送受信同時動作( クロック同期式モード)
//  受信から同時送受信へ切り替えるときには、SCI が受信完了状態であることを確認した後、SCR レジスタのRIE、RE ビットを“0” にしてから、
//  エラーフラグ(SSR.ORER, FER, PER) が“0” であることを確認した後、SCR レジスタのTIE、RIE、TE、RE ビットを1 命令で同時に“1” にしてください。
//
//
#pragma interrupt (Excep_SCI5_RXI5(vect=223))
void Excep_SCI5_RXI5(void)
{
      xpt_rcv_data[xpt_rcv_cnt] =  SCI5.RDR;	// 受信データ読み出し

      xpt_ovr_run_err = SCI5.SSR.BIT.ORER;  // オーバランエラーフラグ読み出し
      
      sci5_rxi_cnt++;		// デバック用
      
      if ( xpt_rcv_cnt < (xpt_send_num - 1) ){  
	     xpt_rcv_cnt++;      // 受信データ格納位置のインクリメント
      }
      else {                     // 最終データ受信済み
	   
      	    xpt_rcv_cnt = 0;
	    
	    
      }
}


//
// 送信割り込み(SCI5) (タッチコントローラ XPT2046 通信用)
// 28.5.6 シリアルデータの送受信同時動作( クロック同期式モード)
//   送信から同時送受信へ切り替えるときには、SCI が送信終了状態であることをSSR.TEND フラグが“1” に なっていることで確認してください。
//    その後、SCR レジスタを初期化してからSCR レジスタのTIE,RIE, TE,RE ビットを1 命令で同時に“1” にしてください。
//
#pragma interrupt (Excep_SCI5_TXI5(vect=224))
void Excep_SCI5_TXI5(void){
		
	SCI5.TDR = xpt_sd_data[xpt_send_pt];     // 送信データをライト
	
	xpt_send_pt++;		// 送信データの格納場所インクリメント
	
	if ( xpt_send_pt == xpt_send_num ) {  // 最終データの送信完了
	
	     IEN(SCI5,TXI5) = 0;		// 送信割り込み禁止
	     SCI5.SCR.BIT.TEIE = 1;  		// TEI 送信完了割り込み要求の許可
	
	}				
	
}


//
// 送信終了割り込み(SCI5) (タッチコントローラ XPT2046 通信用)
//
#pragma interrupt (Excep_SCI5_TEI5(vect=225))
void Excep_SCI5_TEI5(void){
       
	SCI5.SCR.BIT.TEIE = 0;  	// TEI割り込み(送信終了割り込み)禁止
	
	sci5_tei_cnt++;			// デバック用
}



// 
// SCI5 初期設定 （簡易SPI通信) (タッチコントローラ XPT2046用)
// 
//ポート: 用途      :XPT2046信号名
//  P14: 入力ポート ; T_IRQ (Lowでタッチあり)
//
//  PH2; SCK5       : T_CLK (クロック　max:約2MHz)
//  PH1: SMOSI5     : T_DIN (マイコンからのコマンド入力)
//  PH0: SMISO5     : T_DO (A/D変換データ出力)
//
//・SPIモード
//  CKPH = 1 , CKPOL = 0
//  アイドル時 Clock = Low, クロック立ち上がり時にデータサンプル
//　
// ・ボーレート
// クロックセレクトビット=0 (PCKLB=32MHz)　
//  SCI5.SMR.BYTE = 0x80;	// PCLKB(=32MHz)  n = 0
//
//  Nの計算式:	(B=ボーレート bps)			
//    N= (32 x 1000000/(8/2)xB)-1
//     = (32 x 1000000/(4xB)) - 1
//　　
//   B=250Kbps とすると、N= ( 32x 1000000 / 1000000 ) - 1 = 31
//   B=500Kbps とすると、N= ( 32x 1000000 / 2000000 )  -1 = 15
//   B=1000Kbps とすると、N= ( 32x 1000000 / 4000000 )  -1 = 7
//   B=2000kbps  とすると、N = ( 32x1000000 / 8000000 ) - 1 = 3
// (タッチコントローラ XPT2046は Max 2.5MHz)
//
// ・送受信のビットの順番
//   MSBファーストで送受信 (b7から送信、受信)
//  SCI5.SCMR.BIT.SDIR = 1 : MSBファーストで送受信
//
void initSCI_5(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;   // マルチファンクションピンコントローラ　プロテクト解除
	MPC.PWPR.BIT.PFSWE = 1;  // PmnPFS ライトプロテクト解除
	
	
	MPC.PH2PFS.BYTE = 0x0A;  // PH2 = SCK5
	MPC.PH1PFS.BYTE = 0x0A;  // PH1 = SMOSI5
	MPC.PH0PFS.BYTE = 0x0A;  // PH0 = SMISO5
	
	MPC.PWPR.BYTE = 0x80;    //  PmnPFS ライトプロテクト 設定
			
	PORTH.PMR.BIT.B2 = 1;   // PH2  周辺機能として使用
	PORTH.PMR.BIT.B1 = 1;   // PH1   :
	PORTH.PMR.BIT.B0 = 1;   // PH0   :
	
	
	
	SCI5.SCR.BYTE = 0;	// 送受信禁止 
	
	SCI5.SIMR1.BIT.IICM =0; // 調歩同期式、クロック同期式モードまたは簡易SPIモード

	SCI5.SPMR.BIT.SSE = 0;	// SSn#端子機能禁止
	SCI5.SPMR.BIT.CTSE =0;  // CTS機能禁止
	SCI5.SPMR.BIT.MSS = 0;  // SMOSIn端子：送信、SMISOn端子：受信(マスタモード)
	SCI5.SPMR.BIT.MFF = 0;  // モードフォルトエラーなし
		
      	SCI5.SPMR.BIT.CKPH = 1;  // クロック遅れあり
	SCI5.SPMR.BIT.CKPOL = 0; // クロック極性反転なし
	
	SCI5.SMR.BIT.CM = 1;	//クロック同期式モード、または簡易SPIモードで動作
	SCI5.SMR.BIT.CKS = 0;	// クロックセレクトビット PCLKB(=32MHz)(n=0)
	
	SCI5.SCMR.BIT.SMIF = 0;	// 非スマートカードインタフェースモード(調歩同期式モード、クロック同期式モード、簡易SPIモード、簡易I2Cモード)
	SCI5.SCMR.BIT.SDIR = 1;  //  MSBファーストで送受信
	
	//SCI5.BRR = 15;	 // 500Kbps  
	SCI5.BRR = 3;		 // 2000Kbps= 2Mbps

	
	
	IPR(SCI5,RXI5) = 14;		// 受信 割り込みレベル = 14（15が最高レベル)
	IEN(SCI5,RXI5) = 1;		// 受信割り込み許可
	
	
	IPR(SCI5,TXI5) = 13;		// 送信 割り込みレベル = 13 （15が最高レベル)  
	IEN(SCI5,TXI5) = 1;		// 送信割り込み許可
	
	IPR(SCI5,TEI5) = 13;		// 送信完了 割り込みレベル = 13 （15が最高レベル)
	IEN(SCI5,TEI5) = 1;		// 送信完了割り込み許可
	
	
	xpt_rcv_cnt = 0;		// 受信データ数
	

}



//  XPT2046と送信開始
void xpt2046_comm_start(void)
{
	xpt_send_num = 12;	// 送信データ数 (XPT2046)
	xpt_send_pt = 0;	// 送信データの位置の初期化
  
	sci5_rxi_cnt = 0;	// デバック用
        sci5_tei_cnt = 0;	// デバック用
	
	IEN(SCI5,TXI5) = 1;    // 送信割り込み許可
	  
	SCI5.SCR.BYTE = 0xf0;   // 送受信動作(TE,RE)と送受信割り込み(TIE,RIE)を許可
	  			// 最初の送信割り込みが発生する。
}


//
//  X軸、Y軸とZ1,Z2 の読み出しコマンド設定と最初の送信開始
// 
// 12bit,ディファレンシャル,オートパワーダウン
//
void xpt2046_cmd_set(void)
{
	xpt_sd_data[0] = 0xd4;	// X軸読み出し,12bit,ディファレンシャル,オートパワーダウン
	xpt_sd_data[1] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	xpt_sd_data[2] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	
	xpt_sd_data[3] = 0x94;	// Y軸読み出し,12bit,ディファレンシャル,オートパワーダウン
	xpt_sd_data[4] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	xpt_sd_data[5] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	
	xpt_sd_data[6] = 0xb4;	// Z1読み出し,12bit,ディファレンシャル,オートパワーダウン
	xpt_sd_data[7] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	xpt_sd_data[8] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	
	xpt_sd_data[9] = 0xc4;	// Z2読み出し,12bit,ディファレンシャル,オートパワーダウン
	xpt_sd_data[10] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	xpt_sd_data[11] = 0x00;	// ダミーデータ　(読出しのためのクロック発生用)
	
	
	
	
}
