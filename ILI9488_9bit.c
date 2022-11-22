#include "iodefine.h"
#include  "misratypes.h"
#include "spi_9bit.h"

#include "ILI9488_9bit.h"

uint8_t spi_cmd_buf[8];	// ���M����R�}���h�A�p�����[�^���i�[

uint8_t rgb111_data_buf[9800];	// �\���p�o�b�t�@ RGB-111�̃f�[�^���i�[ (2pixel��1�o�C�g)
				//�@�J���[�o�[�\���e�X�g�̏ꍇ�́A8�F���@480Wx320H�͈̔͂֕\���@1�F=60Wx320H
                                //  (2dot = 1byte, 1col=320dot= 160byte,  160x60col=9600byte ) 
	
// ILI9488�̏�����
//
// 1) 8�F�Ŏg�p : Idle Mode ON (39h). Interface Pixel Format (3Ah)
// 2) �c�^, �������֏�������: Memory Access Control (36h) ,�p�����[�^=48H
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
// Memory Access Control (36h)
// �p�����[�^ (MADCTL)
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
	
	
	spi_cmd_buf[0] = 0x13;		      // Normal Display ON (13h), �p�����[�^��=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	spi_cmd_buf[0] = 0x39;		      // Idle Mode ON (39h), �p�����[�^��=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	spi_cmd_buf[0] = 0x3A;		      //  Interface Pixel Format (3Ah)�̃p�����[�^ , 8�F  (3 bits/pixel) (MCU interface format (SPI))  �p�����[�^��=1
	spi_cmd_buf[1] = 0x01;
	spi_data_send(2, &spi_cmd_buf[0],1);  // �R�}���h���M 
	
	
	spi_cmd_buf[0] = 0x36;		      // Memory Access Control (36h)
	spi_cmd_buf[1] = 0x48;		      // �R�}���h���W�X�^ 0x36�p�p�����[�^ (MY=0,MX=1,MV=0)
	spi_data_send(2, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	
	spi_cmd_buf[0] = 0xB2;		      //  Frame Rate Control (In Idle Mode/8 Colors) (B2h)
	spi_cmd_buf[1] = 0x00;
	spi_cmd_buf[2] = 0x10;
	spi_data_send(3, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	
	spi_cmd_buf[0] = 0xB6;        // / (Extend Command)Display Function Control (B6h) 
	spi_cmd_buf[1] = 0x02;
	spi_cmd_buf[2] = 0x02;
	spi_cmd_buf[3] = 0x3b;
	spi_data_send(4, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	 
	spi_cmd_buf[0] = 0x29;		      // Display ON (29h), �p�����[�^��=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	spi_cmd_buf[0] = 0x11;		      // Sleep OUT (11h),�p�����[�^��=0		
	spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	delay_msec(5);	    	    	   //  5msec�҂�
	  
}



// LCD �̃��Z�b�g
// 
void ILI9488_Reset(void)
{

     	
	LCD_RESET_PODR = 0;              // LCD ���Z�b�g��Ԃɂ���
	delay_msec(1);		        // 1[msec] �҂� 
	
	LCD_RESET_PODR = 1;             // LCD �񃊃Z�b�g���
	delay_msec(2);	        	// 2[msec] �҂� 
	
	
}





// �@�s�N�Z���f�[�^�̏������݃e�X�g
//    Memory Access Control (36h) �̐ݒ�ɂ��A�f�[�^�̕\���ʒu���ς�鎖���m�F
//
//   1byte �� 2pixel����RGB���  
//   1byte = **RG BRGB
//
// �F       RGB     2pixel���
// ��       111 ,  0011 1111(=0x3f)
// ���F     110 ,  0011 0110(=0x36)
// �V�A��   011 ,  0001 1011(=0x1b)
// ��       010 ,  0001 0010(=0x12)
// �}�[���^ 101 ,  0010 1101(=0x2d)
// ��       100 ,  0010 0100(=0x24)
// ��       001 ,  0000 1001(=0x09)
// ��       000 ,  0000 0000(=0x00)

void pixel_write_test()
{
		
	uint16_t pix_num;
	uint16_t wr_num;
	
	uint32_t i;
	
	pix_num = 4;		// �������݃s�N�Z����
	wr_num = pix_num / 2;	// ���M�o�C�g�� ( 1 byte�� 2�s�N�Z�����̏��@)
	

	for ( i = 0; i < wr_num; i++ )    // �������݃f�[�^�̃Z�b�g 
	{
		rgb111_data_buf[i] = 0x12;
	}
	
	lcd_adrs_set(0,0, pix_num, 0);	  // Column Address Set(2Ah), Page Address Set(2Bh) (�J�n�J����=0, �J�n�y�[�W=0, �I���J����=pix_num, �I���y�[�W=0)
	
	 
	 spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
	
	 spi_data_send(wr_num, &rgb111_data_buf[0],0);  // �s�N�Z���f�[�^���M

	
}



//  �\���͈͂̐ݒ�
// ����:
//  col: �J�n�J����(x), page(row):�J�n�y�[�W(y)
//  col2:�I���J����, page2(row2): �I���y�[�W
//
void lcd_adrs_set( uint16_t col, uint16_t page, uint16_t col2, uint16_t page2)
{
	
	 spi_cmd_buf[0] = 0x2a;  			       // Column Address Set �R�}���h���W�X�^ 0x2a , �p�����[�^��=4
	 spi_cmd_buf[1] = (uint8_t) ((0xff00 & col) >> 8);     //  SC[15:8]�@�X�^�[�g�J����(16bit)�̏�ʃo�C�g
	 spi_cmd_buf[2] = (uint8_t) (0x00ff & col);            //  SC[7:0]         :�@�@�@�@�@�@�@�@���ʃo�C�g 
	 spi_cmd_buf[3] = (uint8_t) ((0xff00 & col2) >> 8);    //  EC[15:8]�@�I���J����(16bit)�̏�ʃo�C�g
	 spi_cmd_buf[4] = (uint8_t) (0x00ff & col2);           //  EC[7:0]         :�@�@�@�@�@�@�@�@���ʃo�C�g 
	 	 
	 spi_data_send(5, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	
	 spi_cmd_buf[0] = 0x2b;				       //  Page Address Set �R�}���h���W�X�^ 0x2b , �p�����[�^��=4
	 spi_cmd_buf[1] = (uint8_t) ((0xff00 & page ) >> 8);    //  SP[15:8]�@�X�^�[�g�y�[�W(16bit)�̏�ʃo�C�g
	 spi_cmd_buf[2] = (uint8_t) (0x00ff & page);            //  SP[7:0]         :�@�@�@�@�@�@�@�@���ʃo�C�g 
	 spi_cmd_buf[3] = (uint8_t) ((0xff00 & page2 ) >> 8);    // EP[15:8]�@�I���y�[�W(16bit)�̏�ʃo�C�g
	 spi_cmd_buf[4] = (uint8_t) (0x00ff & page2);            // EP[7:0]         :�@�@�@�@�@�@�@�@���ʃo�C�g 
	 
	spi_data_send(5, &spi_cmd_buf[0],1);  // �R�}���h���M
		
		 
}


//
//  Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������
// 
void spi_cmd_2C_send( void )
{
	 spi_cmd_buf[0] = 0x2c;		       // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������
	
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	
	
}

	
//  ILI9488  LCD �J���[�o�[(8�F) 
//   (320x480)
//
//   1byte �� 2pixel����RGB���  
//   1byte = **RG BRGB

// �F       RGB     2pixel���
// ��       111 ,  0011 1111(=0x3f)
// ���F     110 ,  0011 0110(=0x36)
// �V�A��   011 ,  0001 1011(=0x1b)
// ��       010 ,  0001 0010(=0x12)
// �}�[���^ 101 ,  0010 1101(=0x2d)
// ��       100 ,  0010 0100(=0x24)
// ��       001 ,  0000 1001(=0x09)
// ��       000 ,  0000 0000(=0x00)
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
	
	
	lcd_adrs_set(0,0, 319,479);	  // �������ݔ͈͎w��(�R�}���h 2a�ƃR�}���h 2b) (�J�n�J����=0, �J�n�y�[�W=0, �I���J����=319, �I���y�[�W=479)
        
	num = (320 / 2 ) * 60; 		// 2pixel/1byte, 480/8�F=60 page
	
         for ( i = 0; i < num ; i++)	// �s�N�Z���f�[�^�𗬂����� (60�y�[�W��) 	
         {
	     rgb111_data_buf[i] = 0x3f;        //  �� (2pixel ��)
         }

	 
	 spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �s�N�Z���f�[�^���M
	 
	 
	 for ( i = 0; i < num ; i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x36;        // ���F (2pixel ��)
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
	
	
		
         for ( i = 0; i < num ; i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x1b;        // �V�A��  (2pixel ��)
         }
 	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
	 

	  
         for ( i = 0; i < num ;i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x12;        // ��
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
 
	
	 
         for ( i = 0; i < num; i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x2d;        // �}�[���_
         }
	 
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
	 
	
	 
	 for ( i = 0; i < num; i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x24;        // ��
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	  
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
	
	
         for ( i = 0; i < num; i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x09;        // ��
         }
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	  
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
	  	 
	
	   
         for ( i = 0; i < num; i++)	// �s�N�Z���f�[�^�𗬂�����	
         {
	     rgb111_data_buf[i] = 0x00;        // ��
         }
	 
	 spi_cmd_buf[0] = 0x3c;		       // Memory Write Continue (3Ch)
	 spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
	 
	 spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
	 
}


//
// ��ʂ����ɂ���
//
void disp_black(void)
{
	
	uint32_t i;
	uint32_t num;
	
	lcd_adrs_set(0,0, 319, 479);	  // �������ݔ͈͎w��(�R�}���h 2a�ƃR�}���h 2b) (�J�n�J����=0, �J�n�y�[�W=0, �I���J����=319, �I���y�[�W=479)
        
	num = 9600;			// 9600 = (320/2) * 60�s , 2pixel=1byte       
        for ( i = 0; i < num ; i++)	// �s�N�Z���f�[�^�𗬂����� (60�y�[�W��)  (1page�����̂� 320 / 2 = 160byte�K�v , (1 pixel=2 byte) ) 	
        {
	     rgb111_data_buf[i] = 0x00;        //  �� (2pixel ��)
        }
	
 	spi_cmd_2C_send();	  // Memory Write (2Ch)  �擪�ʒu(�R�}���h2a,2b�Ŏw�肵���ʒu)����f�[�^��������		
	 
	spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
 	
	
	for ( i = 0; i < 7 ; i++)	// �c��̕����ɍ���]��	
        {
		spi_cmd_buf[0] = 0x3c;		      // Memory Write Continue (3Ch)	
		spi_data_send(1, &spi_cmd_buf[0],1);  // �R�}���h���M
 		
		spi_data_send(num, &rgb111_data_buf[0],0);  // �f�[�^���M
 		
        }
	
	
	 
}



