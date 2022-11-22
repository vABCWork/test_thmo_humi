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
	
	clear_module_stop();	//  ���W���[���X�g�b�v�̉���
	
	initSCI_5();		// SCI5(�Ȉ�SPI)  �����ݒ�
	
	RSPI_Init_Port();	// RSPI �|�[�g�̏�����  (LCD�R���g���[���p)   
     	RSPI_Init_Reg();        // SPI ���W�X�^�̐ݒ�  

     	RSPI_SPCMD_0();	        // SPI �]���t�H�[�}�b�g��ݒ�, SSLA0�g�p	
	
	ILI9488_Reset();	// LCD �̃��Z�b�g	
	 
	ILI9488_Init();		// LCD�̏�����
	
	delay_msec(10);		// LCD(ILI9488)�����������҂�
	
	disp_black();		// ��ʁ@��  ( 106 [msec] at 16[MHz] )

	RIIC0_Port_Set();	//  I2C(SMBus)�C���^�[�t�F�C�X�p�̃|�[�g�ݒ�	
	RIIC0_Init();		//  I2C(SMBus)�C���^�[�t�F�C�X �̏�����
					
	RIIC0_Init_interrupt();	// RIIC0 ���荞�݋��� 
	
	delay_msec(100);	// �Z���T����҂� (100msec �҂�) 
	
	Timer10msec_Set();      // �^�C�}(10msec)�쐬(CMT0)
     	Timer10msec_Start();    // �^�C�}(10msec)�J�n�@
	
	
	disp_percent();		// %�̕\��(48x96)

	disp_switch_square_id( 0, 128, 330);	// �� �̃{�^��(64x64)
	disp_switch_square_id( 1, 128, 416);	// F �̃{�^��(64x64)
	
	xpt2046_cmd_set();             // �^�b�`�R���g���[�� XPT2046�p �R�}���h�ݒ�@

	iic_slave_adrs = 0x38;          //  �X���[�u�A�h���X = 0x3B (�����x�Z���T AHT25)
	wr_sensor_cmd();	        //  �}�X�^���M�̊J�n(�g���K�R�}���h�̔��s)(�ŏ�)
	
	touch_pre_status_ini();		// �S�Ĕ�^�b�`(SW OFF: High = 1 )�Ƃ���
	
	debug_port_ini();		// debug�p
	
	while(1){
           
	   if ( flg_100msec_interval == 1 ) {  // 100msec�o��
               
		 
	       flg_100msec_interval = 0;	       // 100msec�o�߃t���O�̃N���A
	   
	       
	       if ( timer_10msec_cnt == 0 ) {  		
		   
	           wr_sensor_cmd();	     //�@�}�X�^���M�̊J�n(�g���K�R�}���h�̔��s)
	       	   
	        }
	        else if ( timer_10msec_cnt == 10 ) { 
		      
	           rd_sensor_humi_temp();    // �}�X�^��M�̊J�n(�����x�f�[�^�̓ǂݏo��)  
	      
	        }
		else if ( timer_10msec_cnt == 20 ) {
		
		    Cal_humidity_temperature();	//  ���x�Ɖ��x���v�Z
				
		}
	       
	                                         // 100mse���̋��ʏ���
	       key_input();			
	   
	       if ( flg_disp_fahrenheit == 0 ) {      //�@��(�ێ�)�\��
	       
	           disp_float_data(float_sensor_temperature, 60, 60, 1, COLOR_WHITE );   // ���x�\�� �T�C�Y(48x96) 
		   disp_celsius();		     // ���̕\��(48x96)
	       }
	       else {					// F(�؎�)�\��
		   temp_fahrenheit = float_sensor_temperature * 1.8 + 32.0; 
		   
	           disp_float_data(temp_fahrenheit, 60, 60, 1, COLOR_WHITE );   // ���x�\�� �T�C�Y(48x96) 
	           disp_fahrenheit();			// F�̕\�� (48x96)
	       }
      
	       disp_float_data(float_sensor_humidity, 100, 200, 0, COLOR_WHITE);   // ���x�\�� �T�C�Y(32x64) 
	       
	   }
	   
	   
	}  // while(1)
	
	
	
}




// ���W���[���X�g�b�v�̉���
// �R���y�A�}�b�`�^�C�}(CMT) ���j�b�g0(CMT0, CMT1) 
//  �V���A���y���t�F�����C���^�t�F�[�X0(RSPI) (LCD�R���g���[�� ILI9488�Ƃ̒ʐM�p)(SPI�ʐM)
//  �V���A���R�~���j�P�[�V�����C���^�t�F�[�X5(SCI5)(�^�b�`�R���g���[�� XPT2046 �Ƃ̒ʐM�p)(�Ȉ�SPI�ʐM)
//  I2C �o�X�C���^�t�F�[�X(RIICa)
//  CRC ���Z��iCRC�j(RIIC I2C�ʐM�p)
//
void clear_module_stop(void)
{
	SYSTEM.PRCR.WORD = 0xA50F;	// �N���b�N�����A����d�͒ጸ�@�\�֘A���W�X�^�̏������݋���	
	
	MSTP(CMT0) = 0;			// �R���y�A�}�b�`�^�C�}(CMT) ���j�b�g0(CMT0, CMT1) ���W���[���X�g�b�v�̉���
	
	MSTP(RSPI0) = 0;		// �V���A���y���t�F�����C���^�t�F�[�X0 ���W���[���X�g�b�v�̉���
	
	MSTP(SCI5) = 0;			// SCI5    ���W���[���X�g�b�v�̉���:
	
	MSTP(RIIC0) = 0;                //  RIIC0���W���[���X�g�b�v���� (I2C�ʐM)

	MSTP(CRC) = 0;			// CRC ���W���[���X�g�b�v�̉���
	
	SYSTEM.PRCR.WORD = 0xA500;	// �N���b�N�����A����d�͒ጸ�@�\�֘A���W�X�^�������݋֎~
}




//   �f�o�b�N�|�[�g�̐ݒ� 
//   (debug_port.h)

void debug_port_ini(void)
{	
        DEBUG_0_PMR = 0;    //  P15 �ėp���o�̓|�[�g
	DEBUG_0_PDR = 1;    //  �o�̓|�[�g�Ɏw��
	
}
