#include "iodefine.h"
#include "misratypes.h"
#include "riic.h"



uint8_t iic_slave_adrs;  // IIC スレーブアドレス  00: 7bitアドレス( 例:100 0000 = 0x40 )

volatile uint8_t iic_rcv_data[16];   // IIC受信データ
volatile uint8_t iic_sd_data[32];    // 送信データ
volatile uint8_t iic_sd_pt;	    // 送信データ位置
volatile uint8_t iic_rcv_pt;         // 受信データ位置

volatile uint8_t  dummy_read_fg;    // 受信割り込みで、ダミーリードするためフラグ

volatile uint8_t  iic_sd_rcv_fg;	    // 0:送信のみまたは受信のみの場合,  1:マスタ送受信の場合(= リスタートがある場合)
volatile uint8_t  iic_sd_num;	    // 送信データ数(スレーブアドレスを含む)
volatile uint8_t  iic_rcv_num;      // 受信データ数
volatile uint8_t  iic_com_over_fg;  // 1:STOPコンディションの検出時

				// サーモパイル用のCRC-8(SMBus PECコード)(x8 + x2 + x1 + 1)
uint8_t smbus_crc_8;	        //  CRC演算対象データ: 最初のスレーブアドレスから、コマンド、スレーブアドレス(Read用)、受信データ(low側）、受信データ(high側)の5バイト文
uint8_t smbus_crc_ng;          // 上記の5byteに、スレーブ側から送信された、PECを入れて、CRC計算した値=0ならば、異常なし。
			        // ( 32.2.3 CRC データ出力レジスタ（CRCDOR）「RX23E-Aグループ ユーザーズマニュアル　ハードウェア編」 (R01UH0801JJ0120 Rev.1.20) )  
			
uint8_t riic_sensor_status;	   // センサのステータス
uint32_t riic_sensor_humidity;	   // センサからの湿度データ 10倍した値 (例: 784ならば78.4%)
uint32_t riic_sensor_temperature;  // センサからの温度データ 10倍した値 (例: 784ならば78.4%)
	
float	float_sensor_humidity;		// センサからの湿度データ
float	float_sensor_temperature;	// センサからの温度データ

uint8_t  crc_x8_x5_x4_1;	// 温湿度センサ用　CRC-8 (x8 + x5 + X4 + 1)
uint8_t  riic_crc_ng;

uint16_t  ta_word_temp;
float  ta_celsius;		// Self temperature (センサの周囲温度 Ta)[℃]


uint16_t  to_word_temp;
float  to_celsius;		// Object temperature (測定対象物の温度 To)[℃]



// RIIC0 EEI0
// 通信エラー/通信イベント発生
//( アービトレーションロスト検出、NACK 検出、タイムアウト検出、スタートコンディション検出、ストップコンディション検出)
//
//   アービトレーションロスト検出、とタイムアウト検出は、使用していない。
//

#pragma interrupt (Excep_RIIC0_EEI0(vect=246))
void Excep_RIIC0_EEI0(void){
	
	if( RIIC0.ICSR2.BIT.START == 1 ) {      // スタート(リスタート)コンディション検出
		RIIC0.ICSR2.BIT.START = 0;	// スタートコンディション検出フラグのクリア
		
	     if ( iic_sd_rcv_fg == 1 ) {	// マスタ送受信の場合(= リスタートがある場合)
		if ( iic_sd_pt == 2) {      //  	コマンド(読み出しレジスタ)送信完了後の、リスタートコンディション発行
			
			RIIC0.ICDRT = iic_sd_data[iic_sd_pt];  // 送信 ( スレーブアドレス(読み出し用)の送信 )
			
			iic_sd_pt++;
			
			// スレーブアドレス(読み出し用)の送信後に、ICCR2.TRS = 0(受信モード)となり、
			// ICSR2.RDRF (受信データフルフラグ)は自動的に“1”(ICDRRレジスタに受信データあり)になる。
			// スレーブアドレス(読み出し用)送信後の、受信割り込みで、ダミーリードするためのフラグを設定
			 
			 dummy_read_fg = 1;    // ダミーリード有効
		  
		 	 RIIC0.ICIER.BIT.TEIE = 0;	// 送信終了割り込み(TEI)要求の禁止
		}
	     }
		
	}
	
	else if ( RIIC0.ICSR2.BIT.STOP == 1 ) {      // STOP 検出
	
	      RIIC0.ICSR2.BIT.STOP = 0;	 //  STOP 検出フラグのクリア	
	      
	     iic_com_over_fg = 1;		// 通信完了
	      
	}
	
	else if ( RIIC0.ICSR2.BIT.NACKF == 1 ) {      // NACK 検出
	        
		RIIC0.ICSR2.BIT.NACKF = 0;	  // NACK 検出フラグのクリア
	        
		RIIC0.ICCR2.BIT.SP = 1;		   // ストップコンディションの発行要求の設定
	}
	
}

// RIIC0 RXI0
// 受信データフル　割り込み
// ICDRRレジスタに受信データあり
#pragma interrupt (Excep_RIIC0_RXI0(vect=247))
void Excep_RIIC0_RXI0(void){
	
	uint8_t dummy;
	
	if ( dummy_read_fg == 1 ) {		// スレーブアドレス(読み出し用)送信後のダミーリード
	
		dummy = RIIC0.ICDRR;		// ダミーリード　(SCLクロックを出力して、受信動作開始)
		dummy_read_fg = 0;
	}
	else { 
		
		iic_rcv_data[iic_rcv_pt] = RIIC0.ICDRR;    // 受信データ読み出し

		iic_rcv_pt++;
		
		
		if ( iic_sd_rcv_fg == 1 ) {	// マスタ送受信の場合(= リスタートがある場合)
		
		   RIIC0.ICMR3.BIT.ACKBT = 0;	// ACK 送信
		
		
		  if ( iic_rcv_pt == 3 ) {	// 最終データ　STOPコンディション発行後に、読み出し
		
		     RIIC0.ICCR2.BIT.SP = 1;		      // ストップコンディションの発行要求の設定
		     
		  }
		}
		
		
		else {				// マスタ受信の場合
		
		 if ( iic_rcv_pt < iic_rcv_num ) {
		     RIIC0.ICMR3.BIT.ACKBT = 0;		// ACK 送信	
		 }
		 else {					// 最終バイトの受信
		     RIIC0.ICMR3.BIT.ACKBT = 1;		// NACK 送信	
		      
		     RIIC0.ICCR2.BIT.SP = 1;		// ストップコンディションの発行要求の設定
		   }
		}
		
		
	}
	
	
}

// RIIC0 TXI0
// 送信データエンプティ	割り込み
// ICDRTレジスタに送信データなしの時に、発生
//
//    

#pragma interrupt (Excep_RIIC0_TXI0(vect=248))
void Excep_RIIC0_TXI0(void){
	
	RIIC0.ICDRT = iic_sd_data[iic_sd_pt];  // 送信
	
	iic_sd_pt++;		// 送信位置の更新
	
	if ( iic_sd_rcv_fg == 1 ) {	// マスタ送受信の場合(= リスタートがある場合)
	    if ( iic_sd_pt == 2) {      //  コマンド(読み出しアドレス)送信開始後
		
		RIIC0.ICIER.BIT.TIE = 0;	// 送信データエンプティ割り込み(TXI)要求の禁止
		RIIC0.ICIER.BIT.TEIE = 1;	// 送信終了割り込み(TEI)要求の許可
	    }
	}
	else {				// マスタ送信、マスタ受信の場合
	       if ( (iic_sd_data[0] & 0x01) == 1 ) {  // マスタ受信の場合
			// スレーブアドレス(読み出し用)の送信後に、ICCR2.TRS = 0(受信モード)となり、
			// ICSR2.RDRF (受信データフルフラグ)は自動的に“1”(ICDRRレジスタに受信データあり)になる。
			// 全データの送信後の、受信割り込みで、ダミーリードするためのフラグを設定
			 
			 dummy_read_fg = 1;    // ダミーリード有効
	       }
	       else {					// マスタ送信の場合
	         if ( iic_sd_pt == iic_sd_num ) {	// 全データの送信完了 
	             RIIC0.ICIER.BIT.TIE = 0;	// 送信データエンプティ割り込み(TXI)要求の禁止
		     RIIC0.ICIER.BIT.TEIE = 1;	// 送信終了割り込み(TEI)要求の許可
	         }
	      }
	}
}

// RIIC0 TEI0
// 送信終了割り込み
//  ICSR2.BIT.TEND = 1で発生 ( ICSR2.BIT.TDRE = 1 の状態で、SCL クロックの9 クロック目の立ち上がりで発生)
#pragma interrupt (Excep_RIIC0_TEI0(vect=249))
void Excep_RIIC0_TEI0(void){
	
	
         RIIC0.ICSR2.BIT.TEND = 0;		//  送信完了フラグのクリア
	
	 if ( iic_sd_rcv_fg == 1 ) {		// マスタ送受信の場合(= リスタートがある場合)
		RIIC0.ICCR2.BIT.RS = 1;		// リスタートコンディションの発行 
	 }
	 
	 else {					// マスタ送信で、全データの送信完了時
	  
	 	RIIC0.ICIER.BIT.TEIE = 0;	// 送信終了割り込み(TEI)要求の禁止
		RIIC0.ICCR2.BIT.SP = 1;	       // ストップコンディションの発行要求の設定
	
	 }	    
	 

}






//  温湿度センサのステータス読み出し (マスタ受信)(割り込み使用)
// RIIC 送信バッファ
//   　iic_sd_data[0] : スレーブアドレス(7bit) + 1(R/W#ビット=Read)
// RIIC 受信バッファ
//     iic_rcv_data[0]: ステータス

void rd_sensor_status(void)
{
	iic_sd_data[0] = (( iic_slave_adrs << 1 ) | 0x01 ) ;  // スレーブアドレスから読出し
	
	riic_master_rcv (1);		//　マスタ受信　開始
	
	while( iic_com_over_fg != 1 ) {		// 通信完了待ち(受信完了待ち)
	}
	
	riic_sensor_status = iic_rcv_data[0];  // センサのステータス
	
}




//  温湿度センサからステータスと温湿度データの読み出し (マスタ受信)(割り込み使用)
// RIIC 送信バッファ
//   　iic_sd_data[0] : スレーブアドレス(7bit) + 1(=Read)
// RIIC 受信バッファ
//     iic_rcv_data[0]: ステータス
//             :   [1]: 湿度データ(b19-b12)
//             :   [2]: 湿度データ(b11-b4)
//             :   [3]のb7-b4: 湿度データ(b3-b0)
//             :   [3]のb3-b0: 温度データ(b19-b16)
//             :   [4]: 温度データ(b15-b8)
//             :   [5]: 温度データ(b7-b0)
//             :   [6]: CRC 
void rd_sensor_humi_temp(void)
{
	
	iic_sd_data[0] = (( iic_slave_adrs << 1 ) | 0x01 ) ;  // スレーブアドレスから読出し

	riic_master_rcv (7);		//　マスタ受信　開始 (750usecかかる) (SCLK = 100 KHz)
	
	
	
	 
}






// 温湿度センサへの測定開始コマンド送信　(マスタ送信)(割り込み使用)
//	 IIC 送信バッファ
//   　iic_sd_data[0] : スレーブアドレス(7bit) + 0(=wite)
//                [1] : Trigger measure(0xAC)
//                [2] : Data0(0x33)
//                [3] : Data1(0x00)
void wr_sensor_cmd(void)
{
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // スレーブアドレスへ書き込み
	iic_sd_data[1] = 0xac;
	iic_sd_data[2] = 0x33;
	iic_sd_data[3] = 0x00;
	 
	riic_master_send (4);	// マスタ送信(割り込み使用)
	
	
}




// 
//  温湿度センサから得たデータより、
//  湿度と温度を計算する。
//    CRC異常の場合は、0とする。
//
void Cal_humidity_temperature(void)
{
	uint32_t dt;
	uint32_t dt_h;
	uint32_t dt_m;
	uint32_t dt_l;
	
	
	crc_x8_x5_x4_1 = Calc_crc_x8_x5_x4_1(&iic_rcv_data[0],6);   // CRC-8(X8+X5+X4+1)の計算
	riic_crc_ng =  Calc_crc_x8_x5_x4_1(&iic_rcv_data[0],7);     // 送信されたCRCを含めて計算
	
	
	if ( riic_crc_ng == 0 ) { // CRCが一致した場合、温湿度の計算
	
		dt_h = iic_rcv_data[1];		// 湿度データ(b19-b12)
		dt_h = dt_h << 12;
	
        	dt_m = iic_rcv_data[2];		// 湿度データ(b11-b4)
		dt_m = dt_m << 4;
	
		dt_l = iic_rcv_data[3];		// b7-b4: 湿度データ(b3-b0)
		dt_l = dt_l >> 4;
	
		dt = dt_h | dt_m | dt_l;
	
		dt =  dt * 1000;		
		dt = dt >> 10;			// 1/1024 = 1/(2^10)
		dt = dt >> 10;
		riic_sensor_humidity = dt;     // 湿度データ (784ならば78.4%)
	
	
		dt_h = iic_rcv_data[3] & 0x0f; // b3-b0: 温度データ(b19-b16)
		dt_h = dt_h << 16;
	
		dt_m = iic_rcv_data[4];		// 温度データ(b15-b8)
		dt_m = dt_m << 8;
	
		dt_l = iic_rcv_data[5];		// 温度データ(b7-b0)
	
		dt = dt_h | dt_m | dt_l;
	
		dt =  dt * 200 *10;		
		dt = dt >> 10;
		dt = dt >> 10;
		dt = dt - 500;
	
		riic_sensor_temperature = dt;		// 温度データ (283ならば28.3℃)
	}
        else {
		riic_sensor_humidity = 0;
		riic_sensor_temperature = 0;
	}
	
	
	float_sensor_humidity = riic_sensor_humidity / 10.0;
	
	float_sensor_temperature = riic_sensor_temperature/ 10.0;
	
}



// CRC-8の計算 (AHT25用)
// CRC-8-Maxim: X8+X5+X4+1 (0x31) 初期値=0xff
//
// 下記サンプルプログラムより引用
// STM32 の AHT20 ルーチン (aht20_stm32 demo v1_4)」 (http://www.aosong.com/class-36.html)
// 
//
uint8_t Calc_crc_x8_x5_x4_1(volatile uint8_t *data, uint8_t num)
{
        uint8_t i;
        uint8_t pt;
        uint8_t crc;
	
	crc = 0xff;

	for ( pt = 0; pt < num; pt++ ) {
  
         crc ^= data[pt];
    
	 for ( i = 8 ;i >0 ; --i)  {
    
           if ( crc & 0x80 ) {
               crc = ( crc << 1 ) ^ 0x31;
	   }
           else{
	       crc = ( crc << 1 );
	   }
	 }
       }
 
       return crc;
}


// PECによるデータチェック (サーモパイル用)
// CRC-8-ATM: X8+X2+X1+1 (0x07) 初期値=0x00
//
// 送受信データから、CPUのCRC演算器を使用して求める。
// 例:
//　　iic_sd_data[0] = 0x7a;   (スレーブアドレス=3D + R/W#(Write=0))
//    iic_sd_data[1] = 0x71;   (コマンド Object temperature read)
//    iic_sd_data[2] = 0x7b;   (スレーブアドレス=3D + R/W#(Read=1))
//
//    iic_rcv_data[0] = 0xdd;  (対象物の温度 下位バイト)
//    iic_rcv_data[1] = 0x01;  (対象物の温度 上位バイト)
//    iic_rcv_data[1] = 0xb8;  PEC(Packet error code)
//
//   全データ(0x7a,0x71,0x7b,0xdd,0x01,0xb8)を、CRC.CRCDIRに入れる。
//   CRC.CRCDOR = 0であれば、データに誤り無し。
// 
// 参考: 「RX23E-Aグループ ユーザーズマニュアル　ハードウェア編 (R01UH0801JJ0120 Rev.1.20)」
//　　　　32.2.3 CRC データ出力レジスタ（CRCDOR）
//    
void Cal_crc_thermo_pile(void)
{
	uint32_t i;
	
	CRC.CRCCR.BYTE = 0x85;		     // CRCDORレジスタをクリア, MSBファースト通信用にCRCを生成, 8ビットCRC（X8 + X2 + X + 1）

	for ( i = 0 ; i < 3 ; i++ ) {	     // CRC-8の計算(送信データ)
	   CRC.CRCDIR = iic_sd_data[i];
	}
	
	CRC.CRCDIR = iic_rcv_data[0];	    // CRC-8の計算(受信データ)
	CRC.CRCDIR = iic_rcv_data[1];
		     
	smbus_crc_8 = CRC.CRCDOR;	   // CRC計算結果(PEC)
 
	CRC.CRCDIR = iic_rcv_data[2];     // PEC　
	       
	smbus_crc_ng = CRC.CRCDOR;        // 受信したPECまでCRC計算。0ならばデータ正常
}





// 
//  放射温度計(サーモパイル)から得たデータより、
//  Self temperatureとObject temperatureを計算する。
//    CRC異常の場合は、0とする。
//
void Cal_Ta_To_temperature(void)
{
	if ( smbus_crc_ng == 0 ) {   // CRC 正常の場合
		     
	    if( iic_sd_data[1] == 0x70 ) {					// Ta(Self temperature)の読み出の場合
			  ta_word_temp =  iic_rcv_data[1];
		    	  ta_word_temp =  ( ta_word_temp << 8 );
		          ta_word_temp =  (ta_word_temp | iic_rcv_data[0]);
		          ta_celsius = ( ta_word_temp * 0.125) - 20.0;  
	     }
	     else if ( iic_sd_data[1] == 0x71 ){				// To(Object temperature)の読み出の場合
			  to_word_temp =  iic_rcv_data[1];
		          to_word_temp =  ( to_word_temp << 8 );
		          to_word_temp =  ( to_word_temp | iic_rcv_data[0]);
		          to_celsius = ( to_word_temp * 0.125) - 30.0;  
	      	        }
         }

	 else{			// PEC 異常
		ta_celsius = 0.0;
		to_celsius = 0.0;
	 }

	
}



//
//  放射温度計(サーモパイル)からのデータ読み出し　(マスタ送受信) （割り込み使用)
//
// 入力: rd_obj= 0: センサの周囲温度(Self temperature)(TA)を読み出す
///            = 1: 測定対象対象物の温度(TO)を読み出す
//
//   IIC 送信バッファ
//   　iic_sd_data[0] : スレーブアドレス(7bit) + 0(=Write)
//     iic_sd_data[1] : コマンド(読み出しアドレス)  0x70=周囲温度読み出し, 0x71=測定対象温度読み出し
//     iic_sd_data[2] : スレーブアドレス(7bit) + 1(=Read)
//
void rd_thermo_pile(uint32_t rd_obj)
{
	
	if ( rd_obj == 0 ) {	// センサの周囲温度(Self temperature)(TA)を読み出す
		iic_sd_data[1] = 0x70;
		
	}
	else {				// 測定対象対象物の温度(TO)を読み出す
		iic_sd_data[1] = 0x71;
	
	}
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 );    // 書き込み用 スレーブアドレス
	iic_sd_data[2] = (iic_sd_data[0] | 0x01);   // 読み出し用　スレーブアドレス 
	

	iic_sd_rcv_fg = 1;		// マスタ送受信処理
	
	riic_sd_start();		   //  RIIC 送信開始
	
	 
}


// RIIC マスタ受信
//   スレーブから、rcv_numバイト受信して、受信バッファ　iic_rcv_data[]へ格納する。
// 入力: rcv_num  受信バイト数
// 

void riic_master_rcv ( uint8_t rcv_num)
{

	iic_sd_num = 1;			// 送信データ数
	iic_rcv_num = rcv_num;		// 受信データ数
	
	iic_sd_rcv_fg = 0;		// マスタ送信またはマスタ受信
	riic_sd_start();		// RIIC 送信開始
		
}



// RIIC マスタ送信
//   スレーブへ　送信バッファ　iic_sd_data[]のデータを sd_numバイト送信する。
// 入力: sd_num  送信バイト数　
// 
//   
void riic_master_send ( uint8_t sd_num)
{
	iic_sd_num = sd_num;		// 送信データ数
	iic_rcv_num = 0;		// 受信データ数
	
	iic_sd_rcv_fg = 0;		// マスタ送信またはマスタ受信
	
	riic_sd_start();		// RIIC 送信開始
}


//  RIIC 送信開始
void riic_sd_start(void)
{
	iic_sd_pt = 0;				 // 送信データ位置　クリア
	iic_rcv_pt = 0;                          // 受信データ位置

	iic_com_over_fg = 0;			// 通信完了フラグのクリア
	
	RIIC0.ICIER.BIT.TIE = 1;		// 送信データエンプティ割り込み(TXI)要求の許可
	
	while(RIIC0.ICCR2.BIT.BBSY == 1){ 	// I2Cバスビジー状態の場合、ループ
	}
	
	RIIC0.ICCR2.BIT.ST = 1;		// スタートコンディションの発行  (マスタ送信の開始)
					// スタートコンディション発行後、ICSR2.TDRE(送信データエンプティフラグ)=1となり、
					//  TXI(送信データエンプティ)割り込み、発生
}


//  I2C(SMBus)インターフェイス の初期化 
// 
//   	PORT16 = SCL
//      PORT17 = SDA
//
//      PCLKB = 32MHz:
//
//      転送速度= 1 / { ( (ICBRH + 1) + (ICBRL + 1) ) / (IIC Phy) + SCLn ライン立ち上がり時間(tr) + SCLn ライン立ち下がり時間(tf) }
//
//       (資料の  29.2.14 I2C バスビットレートHigh レジスタ(ICBRH)　より)
//
//     ( 資料:「 RX23E-Aグループ ユーザーズマニュアル　ハードウェア編」 (R01UH0801JJ0120 Rev.1.20)） 
//


void RIIC0_Init(void)
{
	RIIC0.ICCR1.BIT.ICE = 0;    // RIICは機能停止(SCL,SDA端子非駆動状態)
	RIIC0.ICCR1.BIT.IICRST = 1; // RIICリセット、
	RIIC0.ICCR1.BIT.ICE = 1;    // 内部リセット状態 、SCL0、SDA0端子駆動状態
		
	RIIC0.ICSER.BYTE = 0x00;    // I2Cバスステータス許可レジスタ （マスタ動作のためスレーブ設定は無効)	
	
				    //  通信速度 = 100 kbps (オシロ測定値 102.6 kbps)
//	RIIC0.ICMR1.BIT.CKS = 3;    // RIICの内部基準クロック = 32/8 = 4 MHz　
//	RIIC0.ICBRH.BIT.BRH = 0xEF; // 資料の　「表29.5 転送速度に対するICBRH、ICBRLレジスタの設定例」より (PCLK=PCLKB=32[MHz])
//	RIIC0.ICBRL.BIT.BRL = 0xF2;
	
				    // Fast mode 専用 通信速度 = 400 kbps
				    // オシロで 390 kbos
//	RIIC0.ICMR1.BIT.CKS = 1;    // RIICの内部基準クロック = 32/2 = 16 MHz　
//	RIIC0.ICBRH.BIT.BRH = 0xEE; // 
//	RIIC0.ICBRL.BIT.BRL = 0xF4;
	
	
				    //  通信速度 = 100 kbps (オシロ測定値 97.6 kbps)
	RIIC0.ICMR1.BIT.CKS = 3;    // RIICの内部基準クロック = 32/8 = 4 MHz　
	RIIC0.ICBRH.BIT.BRH = 0xF0; // 
	RIIC0.ICBRL.BIT.BRL = 0xF3;
	
	
	
	RIIC0.ICMR3.BIT.ACKWP = 1;	// ACKBTビットへの書き込み許可		
						
					
					
	RIIC0.ICMR3.BIT.RDRFS = 1;	// RDRFフラグ(受信データフル)セットタイミング
					// 1：RDRF フラグは8 クロック目の立ち上がりで“1” にし、8 クロック目の立ち下がりでSCL0 ラインをLow にホールドします。
					// このSCL0 ラインのLow ホールドはACKBT ビットへの書き込みにより解除されます。
					//この設定のとき、データ受信後アクノリッジビット送出前にSCL0 ラインを自動的にLow にホールドするため、
					// 受信データの内容に応じてACK (ACKBT ビットが“0”) またはNACK (ACKBT ビットが“1”) を送出する処理が可能です。
			
					
	RIIC0.ICMR3.BIT.WAIT = 0;	// WAITなし (9クロック目と1クロック目の間をLowにホールドしない)	
	
	RIIC0.ICMR3.BIT.SMBS = 1;       // SMBus選択 				
	
	 
	RIIC0.ICCR1.BIT.IICRST = 0;	 // RIICリセット解除
}




//
//
//  I2C(SMBus)インターフェイス用のポートを設定
// 
//   	PORT16 = SCL
//      PORT17 = SDA
//

void RIIC0_Port_Set(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;      // マルチファンクションピンコントローラ　プロテクト解除
    	MPC.PWPR.BIT.PFSWE = 1;     // PmnPFS ライトプロテクト解除
    
    	MPC.P16PFS.BYTE = 0x0f;     // PORT16 = SCL0
    	MPC.P17PFS.BYTE = 0x0f;     // PORT17 = SDA0
          
    	MPC.PWPR.BYTE = 0x80;      //  PmnPFS ライトプロテクト 設定
  
    	PORT1.PMR.BIT.B6 = 1;     // PORT16:周辺モジュールとして使用
    	PORT1.PMR.BIT.B7 = 1;     // PORT17:      :
}



// RIIC の割り込み用、割り込みコントローラの設定
// 以下を、割り込み処理で行う
//   EEI: 通信エラー/通信イベント (NACK 検出、スタートコンディション検出、ストップコンディション検出)
//　 RXI:　受信データフル
//   TXI:  送信データエンプティ
//   TEI:  送信終了

void RIIC0_Init_interrupt(void)
{
					// 通信エラー/通信イベント 割り込み
	IPR(RIIC0,EEI0) = 0x04;		// 割り込みレベル = 4　　（15が最高レベル)
	IR(RIIC0,EEI0) = 0;		// 割り込み要求のクリア
	IEN(RIIC0,EEI0) = 1;		// 割り込み許可	
	
					// 受信データフル
	IPR(RIIC0,RXI0) = 0x04;		// 割り込みレベル = 4　　（15が最高レベル)
	IR(RIIC0,RXI0) = 0;		// 割り込み要求のクリア
	IEN(RIIC0,RXI0) = 1;		// 割り込み許可	
	
					// 送信データエンプティ
	IPR(RIIC0,TXI0) = 0x04;		// 割り込みレベル = 4　　（15が最高レベル)
	IR(RIIC0,TXI0) = 0;		// 割り込み要求のクリア
	IEN(RIIC0,TXI0) = 1;		// 割り込み許可	
	
					// 送信終了
	IPR(RIIC0,TEI0) = 0x04;		// 割り込みレベル = 4　　（15が最高レベル)
	IR(RIIC0,TEI0) = 0;		// 割り込み要求のクリア
	IEN(RIIC0,TEI0) = 1;		// 割り込み許可	
	
	
	
	RIIC0.ICIER.BIT.TMOIE = 0;	// タイムアウト割り込み(TMOI)要求の禁止
	RIIC0.ICIER.BIT.ALIE  = 0;   	// アービトレーションロスト割り込み(ALI)要求の禁止
	
	RIIC0.ICIER.BIT.STIE  = 1;	// スタートコンディション検出割り込み(STI)要求の許可
	RIIC0.ICIER.BIT.SPIE  = 1;      // ストップコンディション検出割り込み(SPI)要求の許可
	RIIC0.ICIER.BIT.NAKIE  = 1;	// NACK受信割り込み(NAKI)要求の許可

	RIIC0.ICIER.BIT.RIE = 1;	// 受信データフル割り込み(RXI)要求の許可
	RIIC0.ICIER.BIT.TIE = 0;	// 送信データエンプティ割り込み(TXI)要求の禁止
	RIIC0.ICIER.BIT.TEIE = 0;	// 送信終了割り込み(TEI)要求の禁止
	
}
