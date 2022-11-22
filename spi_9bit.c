#include "iodefine.h"
#include "misratypes.h"

#include "spi_9bit.h"


volatile uint8_t *spi_send_pt;		// ���M�f�[�^���i�[���Ă���ꏊ

volatile uint8_t spi_cmd_flg;		// �R�}���h���M���@= 1, �\���p�f�[�^�܂��̓R�}���h�̃p�����[�^���M��=0

volatile uint8_t spi_sending_fg;	// ���M�� = 1

volatile uint32_t spi_send_num;	// ���M����o�C�g��



// RSPI0 SPTI0
// ���M�o�b�t�@�G���v�e�B	���荞��
//
#pragma interrupt (Excep_RSPI0_SPTI0(vect=46))
void Excep_RSPI0_SPTI0(void){
	
	uint32_t t_sd;

	if (spi_send_num > 0  ) { 		//  ���M�f�[�^������ꍇ
		t_sd = (uint32_t)(*spi_send_pt);
		
		if ( spi_cmd_flg == 1 ){        // �R�}���h���M��
		    t_sd = ( 0x0000 | t_sd );	// D/C�r�b�g=0
		    spi_cmd_flg = 0;		// �R�}���h���M�t���O�̃N���A (�R�}���h�͍ŏ���1�o�C�g�����̂��߁j
		}
		else {				// �\���f�[�^�܂��̓R�}���h�̃p�����[�^���M��
		     t_sd = ( 0x0100 | t_sd );	// D/C�r�b�g=1 
		}
		RSPI0.SPDR.LONG = t_sd;        // ���M�f�[�^�Z�b�g
	
		spi_send_num = spi_send_num  - 1;	// ���M�񐔂��f�N�������g
		spi_send_pt++;				// ���M�ʒu�̍X�V
		
		if ( spi_send_num == 0 ) {	// �ŏI���M�f�[�^���Z�b�g�ς�
		    RSPI0.SPCR.BIT.SPTIE = 0;   // ���M�o�b�t�@�G���v�e�B���荞�ݗv���̔������֎~
                    RSPI0.SPCR2.BIT.SPIIE = 1;	// �A�C�h�����荞�ݗv���̐���������
		}
		
	}
	
}

// RSPI0 SPII0
// �A�C�h�����荞��
#pragma interrupt (Excep_RSPI0_SPII0(vect=47))
void Excep_RSPI0_SPII0(void){
	
	RSPI0.SPCR.BIT.SPE = 0;         // RSPI�@�\�͖���
	RSPI0.SPCR2.BIT.SPIIE = 0;	// �A�C�h�����荞�ݗv���̐������֎~
	
	spi_sending_fg = 0;		// SPI���M���̃t���O�̃N���A
}



// RSPI �|�[�g�̏�����  (LCD�R���g���[���p)
// �@�}�X�^: �}�C�R��
//   �X���[�u:  LCD�R���g���[��(ILI9488)
//
// PC6 : MOSI     ,SPI_MOSI ,�}�X�^�o�� �X���[�u����   
// PC5 : RSPCKA   ,SPI_CLK  ,�N���b�N�o��  
// PC4 : SSLA0    ,DISP_CS  ,�X���[�u�Z���N�g 
// PH3 : port�o�� ,DISP_RST, LCD�R���g���[��(ILI9488) Reset�p 
//
// ILI9488 DBI Type C Serial Interface Option1 �̏ꍇ�A�R�}���h/�f�[�^���ʂ́A���M�f�[�^�̐擪�r�b�g�Ƃ��Ă���̂ŁA
// �|�[�g�o�͂ɂ��R�}���h/�f�[�^���ʂ͕K�v�Ȃ��B
// (PC7 : port�o�� ,DISP_DC ,LCD�R���g���[��(ILI9488)  �R�}���h/�f�[�^���ʗp�@(�R�}���h=low,�f�[�^=high))  
//

void RSPI_Init_Port(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;  	 // �}���`�t�@���N�V�����s���R���g���[���@�v���e�N�g����
        MPC.PWPR.BIT.PFSWE = 1;  	// PmnPFS ���C�g�v���e�N�g����
    
	MPC.PC6PFS.BYTE = 0x0d;		// PC6 = MOSIA   Master Out slave in
	MPC.PC5PFS.BYTE = 0x0d;	  	// PC5 = RSPCKA
	MPC.PC4PFS.BYTE = 0x0d;		// PC4 = SSLA0
	
        MPC.PWPR.BYTE = 0x80;      	//  PmnPFS ���C�g�v���e�N�g �ݒ�	
	
        PORTC.PMR.BIT.B6 = 1;     	// PC6  ���Ӌ@�\�Ƃ��Ďg�p
	PORTC.PMR.BIT.B5 = 1;     	// PC5  
	PORTC.PMR.BIT.B4 = 1;     	// PC4
	
	PORTC.DSCR.BIT.B6 = 1;     	// PC6 ���쓮�o��
	PORTC.DSCR.BIT.B5 = 1;     	// PC5  
	PORTC.DSCR.BIT.B4 = 1;     	// PC4
	
	
	LCD_RESET_PMR  = 0;     	// PH3:�ėp���o�̓|�[�g ,LED�R���g���[�� Reset�p 
	LCD_RESET_PDR = 1;      	// PH3: �o�̓|�[�g�ɐݒ� LED�R���g���[���ւ�Reset
	
      
}




// RSPI ���W�X�^�̏����ݒ� 
//�r�b�g���[�g:
//  RSPI �R�}���h���W�X�^0 �` 7�iSPCMD0�`7)��BRDV[1:0] �r�b�g��SPBR ���W�X�^�̐ݒ�l(n)�̑g�ݍ��킹�ł��܂�B
// 
//  ���݁ARSPI0.SPCMD0��BRDV=0 ,   PCLKB= 32MHz�Ƃ��Ă���B
//   �r�b�g���[�g = PCLKB / 2*(SPBR+1)
//   
//
//     SPBR = 0 �ŁA32/2 = 16 [MHz]
//     SPBR = 1 �ŁA32/4 = 8 [MHz]
//

void RSPI_Init_Reg(void)
{

	RSPI0.SPCR.BIT.SPMS = 0;        // SPI���� (4����)	
	RSPI0.SPCR.BIT.TXMD = 1;        // ���M����̂ݍs���B��M�Ȃ�
	RSPI0.SPCR.BIT.MODFEN = 0;      // ���[�h�t�H���g�G���[���o���֎~
	
	RSPI0.SPCR.BIT.MSTR = 1;	//  �}�X�^���[�h
	
	
	RSPI0.SPCR.BIT.SPEIE =0;       // �G���[���荞�ݗv���̔������֎~
	RSPI0.SPCR.BIT.SPTIE =0;       // ���M�o�b�t�@�G���v�e�B���荞�ݗv���̔������֎~
	RSPI0.SPCR.BIT.SPE = 0;        // RSPI�@�\�͖���
	RSPI0.SPCR.BIT.SPRIE = 0;      // RSPI��M�o�b�t�@�t�����荞�ݗv���̔������֎~
	
	RSPI0.SSLP.BYTE = 0;                // �X���[�u�Z���N�g�ɐ� SSL0�`SSL3 �A�N�e�B�uLow
	RSPI0.SPPCR.BYTE = 0;               //  �ʏ탂�[�h
	
	RSPI0.SPSCR.BYTE = 0;               // �]���t�H�[�}�b�g�́A���SPCMD0 ���W�X�^���Q��

	
	
	// ILI9488 17.4.2. DBI Type C Option 1 (3-Line SPI System) Timing Characteristics ���ASerial clock cycle (Write) = 66nsec (min)
	// �������ݎ��ɂ́A15MHz���ō������A16[MHz]�������Ă���B
	
	//RSPI0.SPBR = 0x1;		   // 8 MHz
	
	RSPI0.SPBR = 0x0;		   // 16 MHz
	
	RSPI0.SPDCR.BIT.SPFC =0x00;    // SPDR ���W�X�^�Ɋi�[�ł���i1 ��̓]���N���j�t���[���� 1 =(32bit, 4byte)
	RSPI0.SPDCR.BIT.SPRDTD = 0;     // 0�FSPDR�͎�M�o�b�t�@��ǂݏo��
	RSPI0.SPDCR.BIT.SPLW = 1;	// RSPI�f�[�^���W�X�^(SPDR)�̓����O���[�h(32bit)�̃A�N�Z�X

                                       // SPCMD0.BIT.SCKDEN = 1�̏ꍇ�́A
	RSPI0.SPCKD.BYTE = 0;               // �Z���N�g�M������N���b�N�����܂ł̒x�����ԁ@1RSPCK
				       // SPCMD0.BIT.SCKDEN = 0�̏ꍇ�A�Z���N�g�M������N���b�N�����܂ł̒x�����Ԃ́A1RSPCK
			
	RSPI0.SSLND.BYTE = 0;          // SPCMD0.BIT.SLNDEN = 1�̏ꍇ�́A
	                              // �N���b�N�ŏI�G�b�W����A�Z���N�g�����܂ł̒x������
	                              // SPCMD0.BIT.SLNDEN = 0�̏ꍇ�A1RSPCK
				      
	RSPI0.SPND.BYTE = 0;	     // SPCMD0.BIT.SPNDEN = 1�̏ꍇ�́A�V���A���]���I����́A�Z���N�g�M����A�N�e�B�u���ԁi���A�N�Z�X�x��)�@1RSPCK�{2PCLK	      
				     // SPCMD0.BIT.SPNDEN = 0�̏ꍇ�A1RSPCK�{2PCLK
	
        RSPI0.SPCR2.BYTE = 0;        // �p���e�B�Ȃ��@�A�C�h�����荞�ݗv���̔������֎~�@�A�C�h�����荞�ݗv���̔������֎~

					// ���M�f�[�^�G���v�e�B���荞��
	IPR(RSPI0,SPTI0) = 0x06;	// ���荞�݃��x�� = 6�@�@�i15���ō����x��)
	IR(RSPI0,SPTI0) = 0;		// ���荞�ݗv���̃N���A
	IEN(RSPI0,SPTI0) = 1;		// ���荞�݋���	
	
					// �A�C�h�����荞��
	IPR(RSPI0,SPII0) = 0x06;	// ���荞�݃��x�� = 6�@�@�i15���ō����x��)
	IR(RSPI0,SPII0) = 0;		// ���荞�ݗv���̃N���A
	IEN(RSPI0,SPII0) = 1;		// ���荞�݋���				      
}






//     SPCMD 0 �̐ݒ�
//   
//     SPI ���[�h3: �N���b�N�A�C�h���� High
//                :  (�N���b�N�����オ�莞�Ƀf�[�^�T���v��)

//�@�@�@SSL�M��   : SSLA0

void RSPI_SPCMD_0(void)
{

					// SPI ���[�h 3 : �\�� OK
	RSPI0.SPCMD0.BIT.CPOL = 1;	// CPOL= 1 ���p���X(�N���b�N�A�C�h���� High)				
	RSPI0.SPCMD0.BIT.CPHA = 1;	// CPHA= 1 ��G�b�W�Ńf�[�^�ω��A�����G�b�W�Ńf�[�^�T���v�� (�G�b�W��1���琔����j

	
	RSPI0.SPCMD0.BIT.BRDV = 0;	// BRDV= 0 �x�[�X�̃r�b�g���[�g�g�p (�����Ȃ�)
	RSPI0.SPCMD0.BIT.SSLA = 0;	// SSLA= 0 SSL0���g�p
	
	RSPI0.SPCMD0.BIT.SSLKP = 0;	// �]���I����(���̃v���O�����̏ꍇ�A1byte�]��)�SSSL�M�����l�Q�[�g
	
	RSPI0.SPCMD0.BIT.SPB = 0x08;	//  SPB= 8  ���M �f�[�^�� 9 bit
	RSPI0.SPCMD0.BIT.LSBF = 0;	// LSBF= 0  MSB first
	
	RSPI0.SPCMD0.BIT.SPNDEN = 0;	// SPNDEN=0 ���A�N�Z�X�x���́A1RSPCK + 2PCLK
	RSPI0.SPCMD0.BIT.SLNDEN = 0;	// SLNDEN=0 SSL�l�Q�[�g�x���� 1RSPCK
	RSPI0.SPCMD0.BIT.SCKDEN = 0;	//  SCKDEN=0 RSPCK�x���� 1RSPCK
	
}





// LCD�R���g���[��(ILI9488)�ւ̃f�[�^���M �@(���M���荞�݊J�n)
//  
//  ����:    sd_num :���M�f�[�^�o�C�g�� 
//          *data_pt: ���M�f�[�^�̊i�[�ʒu
//�@�@�@�@�@cmd_flg : 1=�R�}���h�A�p�����[�^���M�̏ꍇ
//                    0=�\���f�[�^���M�̏ꍇ
//


void spi_data_send( uint32_t sd_num, uint8_t *data_pt, uint8_t cmd_flg)
{
	
	spi_send_pt = data_pt;   		// ���M ���荞�݂ő��M����f�[�^�̐擪�ʒu
	spi_send_num = sd_num;			// ���M���荞�݂ł̑��M��
	
	if ( cmd_flg == 1 ) {			// �R�}���h,�p�����[�^���M�̏ꍇ
		spi_cmd_flg = 1;	
	}
	else {					// �\���f�[�^���M�̏ꍇ
		spi_cmd_flg = 0;
	}
	
	spi_sending_fg = 1;			// SPI���M���̃t���O�Z�b�g
	
        RSPI0.SPCR2.BIT.SPIIE = 0;		// �A�C�h�����荞��(SPII)�̋֎~
	RSPI0.SPCR.BIT.SPTIE = 1;       	// ���M�o�b�t�@�G���v�e�B���荞�ݗv���̔���������
						// (���M�J�n���̑��M�o�b�t�@�G���v�e�B���荞�ݗv���́ASPTIE �r�b�g�Ɠ����܂��͌�ɁASPE �r�b�g���g1�h �ɂ��邱�ƂŔ������܂��B(page 1010))
	RSPI0.SPCR.BIT.SPE = 1;        		// RSPI �@�\�L��

	spi_data_send_wait();			// �S�f�[�^�̑��M�����҂�
	
}




//
// �S�f�[�^�̑��M�����҂�
//
void  spi_data_send_wait(void)
{
	while( spi_sending_fg == 1) {		// �A�����đ��M����ꍇ�́A���M�����҂�
	
	}
	
}
