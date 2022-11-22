#include "iodefine.h"
#include  "misratypes.h"

#include "ILI9488_9bit.h"
#include "font_32_64.h"
#include "font_48_96.h"
#include "font_64_64.h"
#include "disp_number.h"


//
// 数字の表示テスト
//
// 入力: index : 表示文字を示すインデックス (0～21) 
//       size_id:  0= 32(W)x64(H)で表示, 1= 48(W)x96(H)で表示 
//       color:    0:白, 1:緑, 2:赤:, 3:青, 4:黄, 5:シアン, 6:マゼンタ
//
//  index  表示文字
//    0       0
//    1       1 
//    2       2 
//    3       3
//    4       4
//    5       5
//    6       6
//    7       7
//    8       8
//    9       9
//   10       0.  (0のピリオド付き)
//   11       1.
//   12       2.
//   13       3.
//   14       4.
//   15       5.
//   16       6.
//   17       7.
//   18       8.
//   19       9.
//   20      blank
//   21       -
//
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
//             。 。　       。 。　       
//　　　　　　VCC GND          T_IRQ
//
//

void disp_num_test(uint8_t index, uint8_t size_id, uint8_t color)
{
	uint32_t st_col;
	uint32_t st_row;
	
	uint32_t end_col;
	uint32_t end_row;
	
	
	st_col = 0;		// 開始カラム
	st_row = 0;		// 開始行
	
	if ( size_id == 0 ) {	// 32(W)x64(H)
	  end_col = 31;		// 終了カラム
	  end_row = 63;		// 終了行
	}
	else {			// 48(W)x96(H)で表示 
	  end_col = 47;		// 終了カラム
	  end_row = 95;		// 終了行
	}

	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // 書き込み範囲指定 
	
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
	spi_data_send_id(index, size_id, color);  	// データ送信　小数点第1位の数字	
	
}


// ℃の表示 (48x96)
 void disp_celsius(void)
{
	
	disp_index_data ( INDEX_FONT_CELSIUS,  268, 60 , 1 ,COLOR_WHITE );
	
}


// Fの表示 (48x96)
 void disp_fahrenheit(void)
{
	
	disp_index_data ( INDEX_FONT_FAHRENHEIT,  268, 60 , 1 ,COLOR_WHITE );
	
}


// %の表示 (48x96)
 void disp_percent(void)
{
	
	disp_index_data ( INDEX_FONT_PERCENT,  268, 200 , 1 ,COLOR_WHITE );
	
}


// 
//  indexで示されるデータを表示

void  disp_index_data ( uint8_t index,  uint32_t start_column, uint32_t start_row, uint8_t size_id , uint8_t color )
{
	
	uint32_t st_col;
	uint32_t st_row;
	
	uint32_t end_col;
	uint32_t end_row;
	
	uint32_t col_offset;
	
	st_col = start_column;
	st_row = start_row;

	if ( size_id == 0 ) {	// フォントサイズ 32(W)x64(H)の場合
	    	col_offset = 31;
	        end_col = st_col + col_offset;
		end_row = st_row + 63;				
	}
	else {			// フォントサイズ 48(W)x96(H)の場合
	        col_offset = 47;
		end_col = st_col + col_offset;
		end_row = st_row + 95;
	}
	
	
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // 書き込み範囲指定 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み
	
	spi_data_send_id( index , size_id, color);  // データ送信 10の位の数字
	 
}


//
//  indexで示されたスイッチ用の四角形(64x64 pixel)を描く
//  入力: index : 表示するスイッチのIndex (0:℃, 1: F)
//        st_col: 開始 col
//        st_row: 開始 row
//
void disp_switch_square_id( uint8_t index, uint16_t st_col, uint16_t st_row)
{
	uint8_t *pt;
	uint32_t num;
	
	
	num = (64/2) * 64;		// 送信データ数 (1byte = 2pielの情報)
	
	
	pt = (uint8_t *)&font_64w_64h_seg[index][0];  // フォントデータの格納アドレス
	     
	unpack_font_data ( 512 , pt, COLOR_WHITE);  // フォントデータをRGB111へ展開 色=白 
	
	lcd_adrs_set(st_col, st_row, st_col + 63, st_row + 63);     // 書き込み範囲指定(コマンド 2aとコマンド 2b) 
	     
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み				   
						   
        spi_data_send( num, (uint8_t *)&rgb111_data_buf[0], 0);           //  ピクセルデータ送信
}





//
//  float型データを4桁の文字として表示する。
//           
//  入力: 
//        t : 表示する float 型データ
//    start_col: 表示する最初の文字のカラム位置  (x)      
//    start_page:表示する最初の文字のページ位置  (y)
//    size_id:	0= 32(W)x64(H)で表示, 1= 48(W)x96(H)で表示
//    color  :文字の表示色(0:白, 1:緑, 2:赤:, 3:青, 4:黄, 5:シアン, 6:マゼンタ)

// 出力: 
//       表示範囲 -99.9 ～ 999.9　  , 4桁目（数値またはマイナス),3桁目,2桁目+小数点, 1桁目
//　     表示文字フォント 32(W)x64H
//
//   
// 例1:  温度 0.9℃
//   4桁目  3桁目  2桁目  1桁目
//    空白　　空白    0.    9 
//
// 例2:  温度 12.3℃ 
//  4桁目  3桁目  2桁目  1桁目
//　  空白  　1     2.    3 
//
// 例3:  温度 213.4℃ 
// 4桁目  3桁目  2桁目  1桁目
//    2　  　1     3.      4 
//
// 例4:  温度 -5.2℃
//  4桁目  3桁目  2桁目  1桁目
//    - 　　空白    5.    2 
//
// 例5:  温度 -12.3℃ 
// 4桁目  3桁目  2桁目  1桁目
//  -　   　1     2.    3 
//

void disp_float_data(float t, uint32_t start_column, uint32_t start_row, uint8_t size_id , uint8_t color)
{
	uint8_t dig1;    // 1桁目 小数点第1位
	uint8_t dig2;    // 2桁目 1の位　
	uint8_t dig3;    // 3桁目 10の位
	uint8_t dig4;    // 4桁目 100の位
	
	uint8_t para[1];
	
	int16_t  temp;
	
	uint32_t st_col;
	uint32_t st_row;
	
	uint32_t end_col;
	uint32_t end_row;
	
	uint32_t col_offset;
	
	temp = t * 10.0;	// 温度を10倍する

	
	st_col = start_column;
	st_row = start_row;

	if ( size_id == 0 ) {	// フォントサイズ 32(W)x64(H)の場合
	    	col_offset = 31;
	        end_col = st_col + col_offset;
		end_row = st_row + 63;				
	}
	else {			// フォントサイズ 48(W)x96(H)の場合
	        col_offset = 47;
		end_col = st_col + col_offset;
		end_row = st_row + 95;
	}
	
	
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // 書き込み範囲指定 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
		
	if ( temp >= 0 ) {            // 温度が正の場合  0～999.9 表示					   
		dig4 = temp / 1000;
		
		if ( dig4 == 0 ) {
			 spi_data_send_id(INDEX_FONT_BLANK,size_id, color);  // データ送信 " "(空白)
		}
		
		else{
			spi_data_send_id(dig4, size_id, color);  // データ送信 100の位の数字
		}
	}
	else {				// 温度が負の場合 -99.9～ - 0.1
		spi_data_send_id(INDEX_FONT_MINUS, size_id, color);  // データ送信 "-"(マイナス符号)
		
		temp = - temp;		// 正の値として表示処理	
		
		dig4 = 0;
	}
	
	
					// 10の位 表示
	st_col = end_col + 1;		// 前の文字の最終カラム + 1					
	end_col = st_col + col_offset;		
	
						// 10の位 表示
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // 書き込み範囲指定 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
	dig3  = ( temp - (dig4 * 1000)) / 100;
	
	if (( dig4 == 0 ) && ( dig3  == 0 )) { // 100の位が空白で、10の位も空白の場合 (例:　5.1) 
	  	spi_data_send_id(INDEX_FONT_BLANK, size_id, color);  // データ送信 " "(空白)	
	}
	else{
	       spi_data_send_id(dig3, size_id, color);  // データ送信 10の位の数字
	}
	

	
					// 1の位の表示(小数点付き)
	st_col = end_col + 1;		// 前の文字の最終カラム + 1					
	end_col = st_col + col_offset;
	
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // 書き込み範囲指定 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
					   
	dig2 = ( temp - (dig4 * 1000) - (dig3 * 100) ) / 10;
	
	spi_data_send_id( dig2 + 10 , size_id, color);		  // データ送信 1の位の数字(小数点付き) (小数点付きのデータは、数字 + 10 )

	
							// 小数点第1位の表示
	st_col = end_col + 1;		// 前の文字の最終カラム + 1					
	end_col = st_col + col_offset;						
					
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // 書き込み範囲指定 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
	dig1 = temp - (dig4 * 1000) - (dig3 * 100) - (dig2 * 10);
	
	spi_data_send_id(dig1,size_id,color);  	// データ送信　小数点第1位の数字	
	
	
	
}






//
//  byte型データを1桁の文字として表示する。
//     空白 空白  空白　表示文字　(最初の3桁は空白、最後の1桁で表示)
//  入力: 
//        val : 表示する byte 型データ
//    start_col: 表示する最初の文字のカラム位置          
//    start_page:表示する最初の文字のページ位置
//    color  :文字の表示色(0:白, 1:緑, 2:赤:, 3:青, 4:黄, 5:シアン, 6:マゼンタ)
//
// 出力: 
//       表示範囲 0 ～ 9　  
//　     表示文字フォント 32(W)x64H

void disp_byte_data(uint8_t  val , uint32_t start_column, uint32_t start_page, uint8_t color)
{
	uint8_t	i;
	uint8_t para[1];
	
	uint32_t st_col;
	uint32_t st_page;
	
	uint32_t end_col;
	uint32_t end_page;
	
	
	st_col = start_column;
	st_page = start_page;
	
	end_col = st_col + 31;	// 1文字の書き込み終了カラム
	end_page = st_page + 63;      // 　　　　　　:    　ページ
	
	for ( i = 0 ; i < 4;	i++ ) {
		   
	   lcd_adrs_set(st_col, st_page, end_col, end_page );	 // 書き込み範囲指定 
	  
	   spi_cmd_2C_send();	  // Memory Write (2Ch)  先頭位置(コマンド2a,2bで指定した位置)からデータ書き込み		
	 
						   
	   if ( i < 3 ) {
	   	spi_data_send_id(INDEX_FONT_BLANK, 0, color);  // データ送信 " "(空白)					   
	   }
	   else {
	   	 spi_data_send_id(val,  0, color );		// データ送信　表示数字
	   }
	   
	   
	   st_col = end_col + 1;		// 前の文字の最終カラム + 1					
	   end_col = st_col + 31;		
	}
	

	
}




//  32x64 または48 x 96 pixel のデータ送信
//  
//  入力: index:  送信するデータ
//     size_id:	0= 32(W)x64(H)で表示, 1= 48(W)x96(H)で表示    
//    color  :文字の表示色(0:白, 1:緑, 2:赤:, 3:青, 4:黄, 5:シアン, 6:マゼンタ)
//
//  a ) size_id  = 0 の場合:
//  フォントデータのサイズ(byte):  256 = 32/8 * 64
//  送信データ数 (byte):　 1024 = 32/2 * 64    (1pixel = 2byte) 
//
//  b) size_id  = 1 の場合:
//  フォントデータのサイズ(byte):  576 = 48/8 * 96
//  送信データ数 (byte)　2304 = 48/2 *96
//    (1pixel = 2byte) 
//
// ・indexと表示文字の関係 (32x64 と 48x96 で共通)
//  index  表示文字
//    0       0
//    1       1 
//    2       2 
//    3       3
//    4       4
//    5       5
//    6       6
//    7       7
//    8       8
//    9       9
//   10       0.  (0のピリオド付き)
//   11       1.
//   12       2.
//   13       3.
//   14       4.
//   15       5.
//   16       6.
//   17       7.
//   18       8.
//   19       9.
//   20      blank
//   21       -
//


void spi_data_send_id(uint8_t index, uint8_t size_id ,uint8_t color ) {
	
	uint8_t *pt;
 	
	uint32_t len;
	uint32_t num;

	if ( size_id == 0 ) {			// フォントサイズ 32(W)x64(H)の場合				
		pt = (uint8_t *)&font_32w_64h_seg[index][0];  // フォントデータの格納アドレス
	        len = 256;
		num = 1024;
		
	}
	else {					// フォントサイズ 48(W)x96(H)の場合						
		pt = (uint8_t *)&font_48w_96h_seg[index][0];  // フォントデータの格納アドレス
	   	len = 576;
		num = 2304;
	}

	unpack_font_data( len ,pt , color );  // フォントデータをRGB111(rgb111_data_bur)へ展開 
	
	spi_data_send( num, (uint8_t *)&rgb111_data_buf[0],0 );   //  データ送信
	

}




// ビットマップのフォントデータ(1byte単位)を、RGB111に変換して、
// 表示用のバッファ にセット
//  入力: * ptt: フォントデータの先頭アドレス
//         len:  フォントデータのバイト数
//               (8Wx16H ならば 16byte)
//      color_index:文字の表示色(0:白, 1:緑, 2:赤:, 3:青, 4:黄, 5:シアン, 6:マゼンタ)
//
//        
// ・　隣あったビットのON/OFF 情報と文字色の組合わせで、出力するデータは異なる。
//　　(pn:最初のビット, pn1:次のビット) 色(index)
//    pn  pn1　  White (0)    Green(1)      Red(2)       Blue(3)       Yellow(4)    Cyan(5)         Magenta(6)
//     0  0      0x00          0x00         0x00         0x00           0x00         0x00            0x00
//     1  0      0x38          0x10         0x20         0x08           0x30         0x18            0x28
//     0  1      0x07          0x02         0x04         0x01           0x06         0x03            0x05
//     1  1      0x3f          0x12         0x24         0x09           0x36         0x1b            0x2d
  


const uint8_t b2_on_off[7] = {0x38, 0x10, 0x20, 0x08, 0x30, 0x18, 0x28 };  
const uint8_t b2_off_on[7] = {0x07, 0x02, 0x04, 0x01, 0x06, 0x03, 0x05 };  
const uint8_t b2_on_on[7]  = {0x3f, 0x12, 0x24, 0x09, 0x36, 0x1b, 0x2d };


void unpack_font_data ( uint32_t len, uint8_t * src_ptr , uint8_t color_index )
{
	uint8_t  bd;
	uint8_t  bdn;
	uint8_t  cd;
	uint32_t i;
	uint32_t pt;
	
	pt = 0;
	
	for ( i = 0; i < len  ; i++ ){
		
		bd = *src_ptr;		// ビットマップデータ取り出し
					// b7,b6の処理
		if (( bd & 0xc0 ) == 0xc0 ) {    //  ON,ON (b7=1,b6=1)
		    cd = b2_on_on[color_index];
		}
		else if (( bd & 0xc0 ) == 0x80 ) {  // ON,OFF (b7=1,b6=0)
		    cd = b2_on_off[color_index];
		}
		else if (( bd & 0xc0) == 0x40 ){	  // OFF,ON (b7=0,b7=1)
		    cd = b2_off_on[color_index];
		}
		else if (( bd & 0xc0) == 0x00 ){	  // OFF,OFF (b7=0,b7=0)
		    cd = 0x00;
		}
	
		rgb111_data_buf[pt] = cd;	// rgb111データを格納
		pt = pt + 1;		        // 格納位置のインクリメント
		
		
						// b5,b4の処理
		if ((bd & 0x30 ) == 0x30 ) {    //  ON,ON (b5=1,b4=1)
		    cd = b2_on_on[color_index];
		}
		else if (( bd & 0x30 ) == 0x20 ) {  // ON,OFF (b5=1,b4=0)
		    cd = b2_on_off[color_index];
		}
		else if (( bd & 0x30) == 0x10 ){	  // OFF,ON (b5=0,b4=1)
		    cd = b2_off_on[color_index];
		}
		else if (( bd & 0x30) == 0x00 ){	  // OFF,OFF (b5=0,b4=0)
		    cd = 0x00;
		}
	
		rgb111_data_buf[pt] = cd;	// rgb111データを格納
		pt = pt + 1;		        // 格納位置のインクリメント
		
		
						// b3,b2の処理
		if ((bd & 0x0c ) == 0x0c ) {    //  ON,ON (b3=1,b2=1)
		    cd = b2_on_on[color_index];
		}
		else if (( bd & 0x0c ) == 0x08 ) {  // ON,OFF (b3=1,b2=0)
		    cd = b2_on_off[color_index];
		}
		else if ((bd & 0x0c) == 0x04 ){	  // OFF,ON (b3=0,b2=1)
		    cd = b2_off_on[color_index];
		}
		else if ((bd & 0x0c) == 0x00 ){	  // OFF,OFF (b3=0,b2=0)
		    cd = 0x00;
		}
	
		rgb111_data_buf[pt] = cd;	// rgb111データを格納
		pt = pt + 1;		        // 格納位置のインクリメント
	
		
						// b1,b0の処理
		if ((bd & 0x03 ) == 0x03 ) {    //  ON,ON (b1=1,b0=1)
		    cd = b2_on_on[color_index];
		}
		else if ((bd & 0x03 ) == 0x02 ) {  // ON,OFF (b1=1,b0=0)
		    cd = b2_on_off[color_index];
		}
		else if ((bd & 0x03) == 0x01 ){	  // OFF,ON (b1=0,b0=1)
		    cd = b2_off_on[color_index];
		}
		else if ((bd & 0x03) == 0x00 ){	  // OFF,OFF (b1=0,b0=0)
		    cd = 0x00;
		}
	
		rgb111_data_buf[pt] = cd;	// rgb111データを格納
		pt = pt + 1;		        // 格納位置のインクリメント
		
		src_ptr++;				// 取り出し位置インクリメント
		
	}
	
}




