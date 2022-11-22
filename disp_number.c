#include "iodefine.h"
#include  "misratypes.h"

#include "ILI9488_9bit.h"
#include "font_32_64.h"
#include "font_48_96.h"
#include "font_64_64.h"
#include "disp_number.h"


//
// �����̕\���e�X�g
//
// ����: index : �\�������������C���f�b�N�X (0�`21) 
//       size_id:  0= 32(W)x64(H)�ŕ\��, 1= 48(W)x96(H)�ŕ\�� 
//       color:    0:��, 1:��, 2:��:, 3:��, 4:��, 5:�V�A��, 6:�}�[���^
//
//  index  �\������
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
//   10       0.  (0�̃s���I�h�t��)
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
//             �B �B�@       �B �B�@       
//�@�@�@�@�@�@VCC GND          T_IRQ
//
//

void disp_num_test(uint8_t index, uint8_t size_id, uint8_t color)
{
	uint32_t st_col;
	uint32_t st_row;
	
	uint32_t end_col;
	uint32_t end_row;
	
	
	st_col = 0;		// �J�n�J����
	st_row = 0;		// �J�n�s
	
	if ( size_id == 0 ) {	// 32(W)x64(H)
	  end_col = 31;		// �I���J����
	  end_row = 63;		// �I���s
	}
	else {			// 48(W)x96(H)�ŕ\�� 
	  end_col = 47;		// �I���J����
	  end_row = 95;		// �I���s
	}

	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // �������ݔ͈͎w�� 
	
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
	spi_data_send_id(index, size_id, color);  	// �f�[�^���M�@�����_��1�ʂ̐���	
	
}


// ���̕\�� (48x96)
 void disp_celsius(void)
{
	
	disp_index_data ( INDEX_FONT_CELSIUS,  268, 60 , 1 ,COLOR_WHITE );
	
}


// F�̕\�� (48x96)
 void disp_fahrenheit(void)
{
	
	disp_index_data ( INDEX_FONT_FAHRENHEIT,  268, 60 , 1 ,COLOR_WHITE );
	
}


// %�̕\�� (48x96)
 void disp_percent(void)
{
	
	disp_index_data ( INDEX_FONT_PERCENT,  268, 200 , 1 ,COLOR_WHITE );
	
}


// 
//  index�Ŏ������f�[�^��\��

void  disp_index_data ( uint8_t index,  uint32_t start_column, uint32_t start_row, uint8_t size_id , uint8_t color )
{
	
	uint32_t st_col;
	uint32_t st_row;
	
	uint32_t end_col;
	uint32_t end_row;
	
	uint32_t col_offset;
	
	st_col = start_column;
	st_row = start_row;

	if ( size_id == 0 ) {	// �t�H���g�T�C�Y 32(W)x64(H)�̏ꍇ
	    	col_offset = 31;
	        end_col = st_col + col_offset;
		end_row = st_row + 63;				
	}
	else {			// �t�H���g�T�C�Y 48(W)x96(H)�̏ꍇ
	        col_offset = 47;
		end_col = st_col + col_offset;
		end_row = st_row + 95;
	}
	
	
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // �������ݔ͈͎w�� 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������
	
	spi_data_send_id( index , size_id, color);  // �f�[�^���M 10�̈ʂ̐���
	 
}


//
//  index�Ŏ����ꂽ�X�C�b�`�p�̎l�p�`(64x64 pixel)��`��
//  ����: index : �\������X�C�b�`��Index (0:��, 1: F)
//        st_col: �J�n col
//        st_row: �J�n row
//
void disp_switch_square_id( uint8_t index, uint16_t st_col, uint16_t st_row)
{
	uint8_t *pt;
	uint32_t num;
	
	
	num = (64/2) * 64;		// ���M�f�[�^�� (1byte = 2piel�̏��)
	
	
	pt = (uint8_t *)&font_64w_64h_seg[index][0];  // �t�H���g�f�[�^�̊i�[�A�h���X
	     
	unpack_font_data ( 512 , pt, COLOR_WHITE);  // �t�H���g�f�[�^��RGB111�֓W�J �F=�� 
	
	lcd_adrs_set(st_col, st_row, st_col + 63, st_row + 63);     // �������ݔ͈͎w��(�R�}���h 2a�ƃR�}���h 2b) 
	     
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������				   
						   
        spi_data_send( num, (uint8_t *)&rgb111_data_buf[0], 0);           //  �s�N�Z���f�[�^���M
}





//
//  float�^�f�[�^��4���̕����Ƃ��ĕ\������B
//           
//  ����: 
//        t : �\������ float �^�f�[�^
//    start_col: �\������ŏ��̕����̃J�����ʒu  (x)      
//    start_page:�\������ŏ��̕����̃y�[�W�ʒu  (y)
//    size_id:	0= 32(W)x64(H)�ŕ\��, 1= 48(W)x96(H)�ŕ\��
//    color  :�����̕\���F(0:��, 1:��, 2:��:, 3:��, 4:��, 5:�V�A��, 6:�}�[���^)

// �o��: 
//       �\���͈� -99.9 �` 999.9�@  , 4���ځi���l�܂��̓}�C�i�X),3����,2����+�����_, 1����
//�@     �\�������t�H���g 32(W)x64H
//
//   
// ��1:  ���x 0.9��
//   4����  3����  2����  1����
//    �󔒁@�@��    0.    9 
//
// ��2:  ���x 12.3�� 
//  4����  3����  2����  1����
//�@  ��  �@1     2.    3 
//
// ��3:  ���x 213.4�� 
// 4����  3����  2����  1����
//    2�@  �@1     3.      4 
//
// ��4:  ���x -5.2��
//  4����  3����  2����  1����
//    - �@�@��    5.    2 
//
// ��5:  ���x -12.3�� 
// 4����  3����  2����  1����
//  -�@   �@1     2.    3 
//

void disp_float_data(float t, uint32_t start_column, uint32_t start_row, uint8_t size_id , uint8_t color)
{
	uint8_t dig1;    // 1���� �����_��1��
	uint8_t dig2;    // 2���� 1�̈ʁ@
	uint8_t dig3;    // 3���� 10�̈�
	uint8_t dig4;    // 4���� 100�̈�
	
	uint8_t para[1];
	
	int16_t  temp;
	
	uint32_t st_col;
	uint32_t st_row;
	
	uint32_t end_col;
	uint32_t end_row;
	
	uint32_t col_offset;
	
	temp = t * 10.0;	// ���x��10�{����

	
	st_col = start_column;
	st_row = start_row;

	if ( size_id == 0 ) {	// �t�H���g�T�C�Y 32(W)x64(H)�̏ꍇ
	    	col_offset = 31;
	        end_col = st_col + col_offset;
		end_row = st_row + 63;				
	}
	else {			// �t�H���g�T�C�Y 48(W)x96(H)�̏ꍇ
	        col_offset = 47;
		end_col = st_col + col_offset;
		end_row = st_row + 95;
	}
	
	
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // �������ݔ͈͎w�� 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
		
	if ( temp >= 0 ) {            // ���x�����̏ꍇ  0�`999.9 �\��					   
		dig4 = temp / 1000;
		
		if ( dig4 == 0 ) {
			 spi_data_send_id(INDEX_FONT_BLANK,size_id, color);  // �f�[�^���M " "(��)
		}
		
		else{
			spi_data_send_id(dig4, size_id, color);  // �f�[�^���M 100�̈ʂ̐���
		}
	}
	else {				// ���x�����̏ꍇ -99.9�` - 0.1
		spi_data_send_id(INDEX_FONT_MINUS, size_id, color);  // �f�[�^���M "-"(�}�C�i�X����)
		
		temp = - temp;		// ���̒l�Ƃ��ĕ\������	
		
		dig4 = 0;
	}
	
	
					// 10�̈� �\��
	st_col = end_col + 1;		// �O�̕����̍ŏI�J���� + 1					
	end_col = st_col + col_offset;		
	
						// 10�̈� �\��
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // �������ݔ͈͎w�� 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
	dig3  = ( temp - (dig4 * 1000)) / 100;
	
	if (( dig4 == 0 ) && ( dig3  == 0 )) { // 100�̈ʂ��󔒂ŁA10�̈ʂ��󔒂̏ꍇ (��:�@5.1) 
	  	spi_data_send_id(INDEX_FONT_BLANK, size_id, color);  // �f�[�^���M " "(��)	
	}
	else{
	       spi_data_send_id(dig3, size_id, color);  // �f�[�^���M 10�̈ʂ̐���
	}
	

	
					// 1�̈ʂ̕\��(�����_�t��)
	st_col = end_col + 1;		// �O�̕����̍ŏI�J���� + 1					
	end_col = st_col + col_offset;
	
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // �������ݔ͈͎w�� 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
					   
	dig2 = ( temp - (dig4 * 1000) - (dig3 * 100) ) / 10;
	
	spi_data_send_id( dig2 + 10 , size_id, color);		  // �f�[�^���M 1�̈ʂ̐���(�����_�t��) (�����_�t���̃f�[�^�́A���� + 10 )

	
							// �����_��1�ʂ̕\��
	st_col = end_col + 1;		// �O�̕����̍ŏI�J���� + 1					
	end_col = st_col + col_offset;						
					
	lcd_adrs_set(st_col, st_row, end_col, end_row );	 // �������ݔ͈͎w�� 
	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
	dig1 = temp - (dig4 * 1000) - (dig3 * 100) - (dig2 * 10);
	
	spi_data_send_id(dig1,size_id,color);  	// �f�[�^���M�@�����_��1�ʂ̐���	
	
	
	
}






//
//  byte�^�f�[�^��1���̕����Ƃ��ĕ\������B
//     �� ��  �󔒁@�\�������@(�ŏ���3���͋󔒁A�Ō��1���ŕ\��)
//  ����: 
//        val : �\������ byte �^�f�[�^
//    start_col: �\������ŏ��̕����̃J�����ʒu          
//    start_page:�\������ŏ��̕����̃y�[�W�ʒu
//    color  :�����̕\���F(0:��, 1:��, 2:��:, 3:��, 4:��, 5:�V�A��, 6:�}�[���^)
//
// �o��: 
//       �\���͈� 0 �` 9�@  
//�@     �\�������t�H���g 32(W)x64H

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
	
	end_col = st_col + 31;	// 1�����̏������ݏI���J����
	end_page = st_page + 63;      // �@�@�@�@�@�@:    �@�y�[�W
	
	for ( i = 0 ; i < 4;	i++ ) {
		   
	   lcd_adrs_set(st_col, st_page, end_col, end_page );	 // �������ݔ͈͎w�� 
	  
	   spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
						   
	   if ( i < 3 ) {
	   	spi_data_send_id(INDEX_FONT_BLANK, 0, color);  // �f�[�^���M " "(��)					   
	   }
	   else {
	   	 spi_data_send_id(val,  0, color );		// �f�[�^���M�@�\������
	   }
	   
	   
	   st_col = end_col + 1;		// �O�̕����̍ŏI�J���� + 1					
	   end_col = st_col + 31;		
	}
	

	
}




//  32x64 �܂���48 x 96 pixel �̃f�[�^���M
//  
//  ����: index:  ���M����f�[�^
//     size_id:	0= 32(W)x64(H)�ŕ\��, 1= 48(W)x96(H)�ŕ\��    
//    color  :�����̕\���F(0:��, 1:��, 2:��:, 3:��, 4:��, 5:�V�A��, 6:�}�[���^)
//
//  a ) size_id  = 0 �̏ꍇ:
//  �t�H���g�f�[�^�̃T�C�Y(byte):  256 = 32/8 * 64
//  ���M�f�[�^�� (byte):�@ 1024 = 32/2 * 64    (1pixel = 2byte) 
//
//  b) size_id  = 1 �̏ꍇ:
//  �t�H���g�f�[�^�̃T�C�Y(byte):  576 = 48/8 * 96
//  ���M�f�[�^�� (byte)�@2304 = 48/2 *96
//    (1pixel = 2byte) 
//
// �Eindex�ƕ\�������̊֌W (32x64 �� 48x96 �ŋ���)
//  index  �\������
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
//   10       0.  (0�̃s���I�h�t��)
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

	if ( size_id == 0 ) {			// �t�H���g�T�C�Y 32(W)x64(H)�̏ꍇ				
		pt = (uint8_t *)&font_32w_64h_seg[index][0];  // �t�H���g�f�[�^�̊i�[�A�h���X
	        len = 256;
		num = 1024;
		
	}
	else {					// �t�H���g�T�C�Y 48(W)x96(H)�̏ꍇ						
		pt = (uint8_t *)&font_48w_96h_seg[index][0];  // �t�H���g�f�[�^�̊i�[�A�h���X
	   	len = 576;
		num = 2304;
	}

	unpack_font_data( len ,pt , color );  // �t�H���g�f�[�^��RGB111(rgb111_data_bur)�֓W�J 
	
	spi_data_send( num, (uint8_t *)&rgb111_data_buf[0],0 );   //  �f�[�^���M
	

}




// �r�b�g�}�b�v�̃t�H���g�f�[�^(1byte�P��)���ARGB111�ɕϊ����āA
// �\���p�̃o�b�t�@ �ɃZ�b�g
//  ����: * ptt: �t�H���g�f�[�^�̐擪�A�h���X
//         len:  �t�H���g�f�[�^�̃o�C�g��
//               (8Wx16H �Ȃ�� 16byte)
//      color_index:�����̕\���F(0:��, 1:��, 2:��:, 3:��, 4:��, 5:�V�A��, 6:�}�[���^)
//
//        
// �E�@�ׂ������r�b�g��ON/OFF ���ƕ����F�̑g���킹�ŁA�o�͂���f�[�^�͈قȂ�B
//�@�@(pn:�ŏ��̃r�b�g, pn1:���̃r�b�g) �F(index)
//    pn  pn1�@  White (0)    Green(1)      Red(2)       Blue(3)       Yellow(4)    Cyan(5)         Magenta(6)
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
		
		bd = *src_ptr;		// �r�b�g�}�b�v�f�[�^���o��
					// b7,b6�̏���
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
	
		rgb111_data_buf[pt] = cd;	// rgb111�f�[�^���i�[
		pt = pt + 1;		        // �i�[�ʒu�̃C���N�������g
		
		
						// b5,b4�̏���
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
	
		rgb111_data_buf[pt] = cd;	// rgb111�f�[�^���i�[
		pt = pt + 1;		        // �i�[�ʒu�̃C���N�������g
		
		
						// b3,b2�̏���
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
	
		rgb111_data_buf[pt] = cd;	// rgb111�f�[�^���i�[
		pt = pt + 1;		        // �i�[�ʒu�̃C���N�������g
	
		
						// b1,b0�̏���
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
	
		rgb111_data_buf[pt] = cd;	// rgb111�f�[�^���i�[
		pt = pt + 1;		        // �i�[�ʒu�̃C���N�������g
		
		src_ptr++;				// ���o���ʒu�C���N�������g
		
	}
	
}




