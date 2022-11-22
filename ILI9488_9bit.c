#include "iodefine.h"
#include  "misratypes.h"
#include "spi_9bit.h"

#include "ILI9488_9bit.h"

uint8_t spi_cmd_buf[8];	// 送信するコマンド、パラメータを格納

uint8_t rgb111_data_buf[9800];	// 表示用バッファ RGB-111のデータを格納 (2pixelで1バイト)
				//　カラーバー表示テストの場合は、8色を　480Wx320Hの範囲へ表示　1色=60Wx320H
                                //  (2dot = 1byte, 1col=320dot= 160byte,  160x60col=9600byte ) 
	
// ILI9488の初期化
//
// 1) 8色で使用 : Idle Mode ON (39h). Interface Pixel Format (3Ah)
// 2) 縦型, →方向へ書き込み: Memory Access Control (36h) ,パラメータ=48H
//
//                  Clolumn
//            0                 319
//          0 +------------------+
// Page(Row)  |                  |
//            |                  |
//            |                  |
//            |                  |
//            |                  |
//            |                  |
//            |                  |    
//            |                  |
//            |                  |
//            |                  |
//            |                  |
//       479  |                  |
//            +------------------+
//             。 。　       。 。　       
//　　　　　　VCC GND          T_IRQ
//
// 
// Memory Access Control (36h)
// パラメータ (MADCTL)
//  b7 : MY (Row address order)    0=Top to bottomm, 1= Bottom to top	
//  b6 : MX (Column address order) 0=Left to right,  1= Right to Left
//  b5 : MV (Row/Column exchange)
//  b4 : Vertical Refresh Order
//  b3 : RGB-BGR Order 0=RGB color filter panel, 1=BGR color filter panel
//  b2 : Horizontal Refresh ORDER
//  b1 : Reserved
//  b0 : Reserved
//	

void ILI9488_Init(void)
{
	
	
	spi_cmd_buf[0] = 0x13;		      // Normal Display ON (13h), パラメータ数=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	
	spi_cmd_buf[0] = 0x39;		      // Idle Mode ON (39h), パラメータ数=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	
	spi_cmd_buf[0] = 0x3A;		      //  Interface Pixel Format (3Ah)のパラメータ , 8色  (3 bits/pixel) (MCU interface format (SPI))  パラメータ数=1
	spi_cmd_buf[1] = 0x01;
	spi_data_send(2, &spi_cmd_buf[0],1);  // コマンド送信 
	
	
	spi_cmd_buf[0] = 0x36;		      // Memory Access Control (36h)
	spi_cmd_buf[1] = 0x48;		      // コマンドレジスタ 0x36用パラメータ (MY=0,MX=1,MV=0)
	spi_data_send(2, &spi_cmd_buf[0],1);  // コマンド送信
	
	
	spi_cmd_buf[0] = 0xB2;		      //  Frame Rate Control (In Idle Mode/8 Colors) (B2h)
	spi_cmd_buf[1] = 0x00;
	spi_cmd_buf[2] = 0x10;
	spi_data_send(3, &spi_cmd_buf[0],1);  // コマンド送信
	
	
	spi_cmd_buf[0] = 0xB6;        // / (Extend Command)Display Function Control (B6h) 
	spi_cmd_buf[1] = 0x02;
	spi_cmd_buf[2] = 0x02;
	spi_cmd_buf[3] = 0x3b;
	spi_data_send(4, &spi_cmd_buf[0],1);  // コマンド送信
	
	 
	spi_cmd_buf[0] = 0x29;		      // Display ON (29h), パラメータ数=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	 
	spi_cmd_buf[0] = 0x11;		      // Sleep OUT (11h),パラメータ数=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	
	delay_msec(5);	    	    	   //  5msec待ち
	  
}



// LCD のリセット
// 
void ILI9488_Reset(void)
{

     	
	LCD_RESET_PODR = 0;              // LCD リセット状態にする
	delay_msec(1);		        // 1[msec] 待つ 
	
	LCD_RESET_PODR = 1;             // LCD 非リセット状態
	delay_msec(2);	        	// 2[msec] 待つ 
	
	
}





// 　ピクセルデータの書き込みテスト
//    Memory Access Control (36h) の設定により、データの表示位置が変わる事を確認
//
//   1byte に 2pixel分のRGB情報  
//   1byte = **RG BRGB
//
// 色       RGB     2pixel情報
// 白       111 ,  0011 1111(=0x3f)
// 黄色     110 ,  0011 0110(=0x36)
// シアン   011 ,  0001 1011(=0x1b)
// 緑       010 ,  0001 0010(=0x12)
// マゼンタ 101 ,  0010 1101(=0x2d)
// 赤       100 ,  0010 0100(=0x24)
// 青       001 ,  0000 1001(=0x09)
// 黒       000 ,  0000 0000(=0x00)

void pixel_write_test()
{
		
	uint16_t pix_num;
	uint16_t wr_num;
	
	uint32_t i;
	
	pix_num = 4;		// 書き込みピクセル数
	wr_num = pix_num / 2;	// 送信バイト数 ( 1 byteに 2ピクセル分の情報　)
	

	for ( i = 0; i < wr_num; i++ )    // 書き込みデータのセット 
	{
		rgb111_data_buf[i] = 0x12;
	}
	
	lcd_adrs_set(0,0, pix_num, 0);	  // Column Address Set(2Ah), Page Address Set(2Bh) (開始カラム=0, 開始ページ=0, 終了カラム=pix_num, 終了ページ=0)
	
	 
	 spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
	
	 spi_data_send(wr_num, &rgb111_data_buf[0],0);  // ピクセルデータ送信

	
}



//  表示範囲の設定
// 入力:
//  col: 開始カラム(x), page(row):開始ページ(y)
//  col2:終了カラム, page2(row2): 終了ページ
//
void lcd_adrs_set( uint16_t col, uint16_t page, uint16_t col2, uint16_t page2)
{
	
	 spi_cmd_buf[0] = 0x2a;  			       // Column Address Set コマンドレジスタ 0x2a , パラメータ数=4
	 spi_cmd_buf[1] = (uint8_t) ((0xff00 & col) >> 8);     //  SC[15:8]　スタートカラム(16bit)の上位バイト
	 spi_cmd_buf[2] = (uint8_t) (0x00ff & col);            //  SC[7:0]         :　　　　　　　　下位バイト 
	 spi_cmd_buf[3] = (uint8_t) ((0xff00 & col2) >> 8);    //  EC[15:8]　終了カラム(16bit)の上位バイト
	 spi_cmd_buf[4] = (uint8_t) (0x00ff & col2);           //  EC[7:0]         :　　　　　　　　下位バイト 
	 	 
	 spi_data_send(5, &spi_cmd_buf[0],1);  // コマンド送信
	 
	
	 spi_cmd_buf[0] = 0x2b;				       //  Page Address Set コマンドレジスタ 0x2b , パラメータ数=4
	 spi_cmd_buf[1] = (uint8_t) ((0xff00 & page ) >> 8);    //  SP[15:8]　スタートページ(16bit)の上位バイト
	 spi_cmd_buf[2] = (uint8_t) (0x00ff & page);            //  SP[7:0]         :　　　　　　　　下位バイト 
	 spi_cmd_buf[3] = (uint8_t) ((0xff00 & page2 ) >> 8);    // EP[15:8]　終了ページ(16bit)の上位バイト
	 spi_cmd_buf[4] = (uint8_t) (0x00ff & page2);            // EP[7:0]         :　　　　　　　　下位バイト 
	 
	spi_data_send(5, &spi_cmd_buf[0],1);  // コマンド送信
		
		 
}


//
//  Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み
// 
void spi_cmd_2C_send( void )
{
	 spi_cmd_buf[0] = 0x2c;		       // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み
	
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	
	
}

	
//  ILI9488  LCD カラーバー(8色) 
//   (320x480)
//
//   1byte に 2pixel分のRGB情報  
//   1byte = **RG BRGB

// 色       RGB     2pixel情報
// 白       111 ,  0011 1111(=0x3f)
// 黄色     110 ,  0011 0110(=0x36)
// シアン   011 ,  0001 1011(=0x1b)
// 緑       010 ,  0001 0010(=0x12)
// マゼンタ 101 ,  0010 1101(=0x2d)
// 赤       100 ,  0010 0100(=0x24)
// 青       001 ,  0000 1001(=0x09)
// 黒       000 ,  0000 0000(=0x00)
//
//
//                  Clolumn
//            0                 319
//          0 +------------------+
// Page(Row)  |                  |
//            |                  |
//            |                  |
//            |                  |
//            |                  |
//            |                  |
//            |                  |    
//            |                  |
//            |                  |
//            |                  |
//            |                  |
//       479  |                  |
//            +------------------+
//                          Z400IT002
//

void color_bar(void)
{
	uint32_t i;
	uint32_t num;
	
	
	lcd_adrs_set(0,0, 319,479);	  // 書き込み範囲指定(コマンド 2aとコマンド 2b) (開始カラム=0, 開始ページ=0, 終了カラム=319, 終了ページ=479)
        
	num = (320 / 2 ) * 60; 		// 2pixel/1byte, 480/8色=60 page
	
         for ( i = 0; i < num ; i++)	// ピクセルデータを流し込む (60ページ分) 	
         {
	     rgb111_data_buf[i] = 0x3f;        //  白 (2pixel 分)
         }

	 
	 spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // ピクセルデータ送信
	 
	 
	 for ( i = 0; i < num ; i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x36;        // 黄色 (2pixel 分)
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
	
	
		
         for ( i = 0; i < num ; i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x1b;        // シアン  (2pixel 分)
         }
 	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
	 

	  
         for ( i = 0; i < num ;i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x12;        // 緑
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
 
	
	 
         for ( i = 0; i < num; i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x2d;        // マゼンダ
         }
	 
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
	 
	
	 
	 for ( i = 0; i < num; i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x24;        // 赤
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	  
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
	
	
         for ( i = 0; i < num; i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x09;        // 青
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	  
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
	  	 
	
	   
         for ( i = 0; i < num; i++)	// ピクセルデータを流し込む	
         {
	     rgb111_data_buf[i] = 0x00;        // 黒
         }
	 
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
	 
}


//
// 画面を黒にする
//
void disp_black(void)
{
	
	uint32_t i;
	uint32_t num;
	
	lcd_adrs_set(0,0, 319, 479);	  // 書き込み範囲指定(コマンド 2aとコマンド 2b) (開始カラム=0, 開始ページ=0, 終了カラム=319, 終了ページ=479)
        
	num = 9600;			// 9600 = (320/2) * 60行 , 2pixel=1byte       
        for ( i = 0; i < num ; i++)	// ピクセルデータを流し込む (60ページ分)  (1page書くのに 320 / 2 = 160byte必要 , (1 pixel=2 byte) ) 	
        {
	     rgb111_data_buf[i] = 0x00;        //  黒 (2pixel 分)
        }
	
 	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
	spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
 	
	
	for ( i = 0; i < 7 ; i++)	// 残りの部分に黒を転送	
        {
		spi_cmd_buf[0] = 0x3c;		      // Memory Write Continue (3Ch)	
		spi_data_send(1, &spi_cmd_buf[0],1);  // コマンド送信
 		
		spi_data_send(num, &rgb111_data_buf[0],0);  // データ送信
 		
        }
	
	
	 
}



