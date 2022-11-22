#include "iodefine.h"
#include "misratypes.h"
#include "riic.h"



uint8_t iic_slave_adrs;  // IIC �X���[�u�A�h���X  00: 7bit�A�h���X( ��:100 0000 = 0x40 )

volatile uint8_t iic_rcv_data[16];   // IIC��M�f�[�^
volatile uint8_t iic_sd_data[32];    // ���M�f�[�^
volatile uint8_t iic_sd_pt;	    // ���M�f�[�^�ʒu
volatile uint8_t iic_rcv_pt;         // ��M�f�[�^�ʒu

volatile uint8_t  dummy_read_fg;    // ��M���荞�݂ŁA�_�~�[���[�h���邽�߃t���O

volatile uint8_t  iic_sd_rcv_fg;	    // 0:���M�݂̂܂��͎�M�݂̂̏ꍇ,  1:�}�X�^����M�̏ꍇ(= ���X�^�[�g������ꍇ)
volatile uint8_t  iic_sd_num;	    // ���M�f�[�^��(�X���[�u�A�h���X���܂�)
volatile uint8_t  iic_rcv_num;      // ��M�f�[�^��
volatile uint8_t  iic_com_over_fg;  // 1:STOP�R���f�B�V�����̌��o��

				// �T�[���p�C���p��CRC-8(SMBus PEC�R�[�h)(x8 + x2 + x1 + 1)
uint8_t smbus_crc_8;	        //  CRC���Z�Ώۃf�[�^: �ŏ��̃X���[�u�A�h���X����A�R�}���h�A�X���[�u�A�h���X(Read�p)�A��M�f�[�^(low���j�A��M�f�[�^(high��)��5�o�C�g��
uint8_t smbus_crc_ng;          // ��L��5byte�ɁA�X���[�u�����瑗�M���ꂽ�APEC�����āACRC�v�Z�����l=0�Ȃ�΁A�ُ�Ȃ��B
			        // ( 32.2.3 CRC �f�[�^�o�̓��W�X�^�iCRCDOR�j�uRX23E-A�O���[�v ���[�U�[�Y�}�j���A���@�n�[�h�E�F�A�ҁv (R01UH0801JJ0120 Rev.1.20) )  
			
uint8_t riic_sensor_status;	   // �Z���T�̃X�e�[�^�X
uint32_t riic_sensor_humidity;	   // �Z���T����̎��x�f�[�^ 10�{�����l (��: 784�Ȃ��78.4%)
uint32_t riic_sensor_temperature;  // �Z���T����̉��x�f�[�^ 10�{�����l (��: 784�Ȃ��78.4%)
	
float	float_sensor_humidity;		// �Z���T����̎��x�f�[�^
float	float_sensor_temperature;	// �Z���T����̉��x�f�[�^

uint8_t  crc_x8_x5_x4_1;	// �����x�Z���T�p�@CRC-8 (x8 + x5 + X4 + 1)
uint8_t  riic_crc_ng;

uint16_t  ta_word_temp;
float  ta_celsius;		// Self temperature (�Z���T�̎��͉��x Ta)[��]


uint16_t  to_word_temp;
float  to_celsius;		// Object temperature (����Ώە��̉��x To)[��]



// RIIC0 EEI0
// �ʐM�G���[/�ʐM�C�x���g����
//( �A�[�r�g���[�V�������X�g���o�ANACK ���o�A�^�C���A�E�g���o�A�X�^�[�g�R���f�B�V�������o�A�X�g�b�v�R���f�B�V�������o)
//
//   �A�[�r�g���[�V�������X�g���o�A�ƃ^�C���A�E�g���o�́A�g�p���Ă��Ȃ��B
//

#pragma interrupt (Excep_RIIC0_EEI0(vect=246))
void Excep_RIIC0_EEI0(void){
	
	if( RIIC0.ICSR2.BIT.START == 1 ) {      // �X�^�[�g(���X�^�[�g)�R���f�B�V�������o
		RIIC0.ICSR2.BIT.START = 0;	// �X�^�[�g�R���f�B�V�������o�t���O�̃N���A
		
	     if ( iic_sd_rcv_fg == 1 ) {	// �}�X�^����M�̏ꍇ(= ���X�^�[�g������ꍇ)
		if ( iic_sd_pt == 2) {      //  	�R�}���h(�ǂݏo�����W�X�^)���M������́A���X�^�[�g�R���f�B�V�������s
			
			RIIC0.ICDRT = iic_sd_data[iic_sd_pt];  // ���M ( �X���[�u�A�h���X(�ǂݏo���p)�̑��M )
			
			iic_sd_pt++;
			
			// �X���[�u�A�h���X(�ǂݏo���p)�̑��M��ɁAICCR2.TRS = 0(��M���[�h)�ƂȂ�A
			// ICSR2.RDRF (��M�f�[�^�t���t���O)�͎����I�Ɂg1�h(ICDRR���W�X�^�Ɏ�M�f�[�^����)�ɂȂ�B
			// �X���[�u�A�h���X(�ǂݏo���p)���M��́A��M���荞�݂ŁA�_�~�[���[�h���邽�߂̃t���O��ݒ�
			 
			 dummy_read_fg = 1;    // �_�~�[���[�h�L��
		  
		 	 RIIC0.ICIER.BIT.TEIE = 0;	// ���M�I�����荞��(TEI)�v���̋֎~
		}
	     }
		
	}
	
	else if ( RIIC0.ICSR2.BIT.STOP == 1 ) {      // STOP ���o
	
	      RIIC0.ICSR2.BIT.STOP = 0;	 //  STOP ���o�t���O�̃N���A	
	      
	     iic_com_over_fg = 1;		// �ʐM����
	      
	}
	
	else if ( RIIC0.ICSR2.BIT.NACKF == 1 ) {      // NACK ���o
	        
		RIIC0.ICSR2.BIT.NACKF = 0;	  // NACK ���o�t���O�̃N���A
	        
		RIIC0.ICCR2.BIT.SP = 1;		   // �X�g�b�v�R���f�B�V�����̔��s�v���̐ݒ�
	}
	
}

// RIIC0 RXI0
// ��M�f�[�^�t���@���荞��
// ICDRR���W�X�^�Ɏ�M�f�[�^����
#pragma interrupt (Excep_RIIC0_RXI0(vect=247))
void Excep_RIIC0_RXI0(void){
	
	uint8_t dummy;
	
	if ( dummy_read_fg == 1 ) {		// �X���[�u�A�h���X(�ǂݏo���p)���M��̃_�~�[���[�h
	
		dummy = RIIC0.ICDRR;		// �_�~�[���[�h�@(SCL�N���b�N���o�͂��āA��M����J�n)
		dummy_read_fg = 0;
	}
	else { 
		
		iic_rcv_data[iic_rcv_pt] = RIIC0.ICDRR;    // ��M�f�[�^�ǂݏo��

		iic_rcv_pt++;
		
		
		if ( iic_sd_rcv_fg == 1 ) {	// �}�X�^����M�̏ꍇ(= ���X�^�[�g������ꍇ)
		
		   RIIC0.ICMR3.BIT.ACKBT = 0;	// ACK ���M
		
		
		  if ( iic_rcv_pt == 3 ) {	// �ŏI�f�[�^�@STOP�R���f�B�V�������s��ɁA�ǂݏo��
		
		     RIIC0.ICCR2.BIT.SP = 1;		      // �X�g�b�v�R���f�B�V�����̔��s�v���̐ݒ�
		     
		  }
		}
		
		
		else {				// �}�X�^��M�̏ꍇ
		
		 if ( iic_rcv_pt < iic_rcv_num ) {
		     RIIC0.ICMR3.BIT.ACKBT = 0;		// ACK ���M	
		 }
		 else {					// �ŏI�o�C�g�̎�M
		     RIIC0.ICMR3.BIT.ACKBT = 1;		// NACK ���M	
		      
		     RIIC0.ICCR2.BIT.SP = 1;		// �X�g�b�v�R���f�B�V�����̔��s�v���̐ݒ�
		   }
		}
		
		
	}
	
	
}

// RIIC0 TXI0
// ���M�f�[�^�G���v�e�B	���荞��
// ICDRT���W�X�^�ɑ��M�f�[�^�Ȃ��̎��ɁA����
//
//    

#pragma interrupt (Excep_RIIC0_TXI0(vect=248))
void Excep_RIIC0_TXI0(void){
	
	RIIC0.ICDRT = iic_sd_data[iic_sd_pt];  // ���M
	
	iic_sd_pt++;		// ���M�ʒu�̍X�V
	
	if ( iic_sd_rcv_fg == 1 ) {	// �}�X�^����M�̏ꍇ(= ���X�^�[�g������ꍇ)
	    if ( iic_sd_pt == 2) {      //  �R�}���h(�ǂݏo���A�h���X)���M�J�n��
		
		RIIC0.ICIER.BIT.TIE = 0;	// ���M�f�[�^�G���v�e�B���荞��(TXI)�v���̋֎~
		RIIC0.ICIER.BIT.TEIE = 1;	// ���M�I�����荞��(TEI)�v���̋���
	    }
	}
	else {				// �}�X�^���M�A�}�X�^��M�̏ꍇ
	       if ( (iic_sd_data[0] & 0x01) == 1 ) {  // �}�X�^��M�̏ꍇ
			// �X���[�u�A�h���X(�ǂݏo���p)�̑��M��ɁAICCR2.TRS = 0(��M���[�h)�ƂȂ�A
			// ICSR2.RDRF (��M�f�[�^�t���t���O)�͎����I�Ɂg1�h(ICDRR���W�X�^�Ɏ�M�f�[�^����)�ɂȂ�B
			// �S�f�[�^�̑��M��́A��M���荞�݂ŁA�_�~�[���[�h���邽�߂̃t���O��ݒ�
			 
			 dummy_read_fg = 1;    // �_�~�[���[�h�L��
	       }
	       else {					// �}�X�^���M�̏ꍇ
	         if ( iic_sd_pt == iic_sd_num ) {	// �S�f�[�^�̑��M���� 
	             RIIC0.ICIER.BIT.TIE = 0;	// ���M�f�[�^�G���v�e�B���荞��(TXI)�v���̋֎~
		     RIIC0.ICIER.BIT.TEIE = 1;	// ���M�I�����荞��(TEI)�v���̋���
	         }
	      }
	}
}

// RIIC0 TEI0
// ���M�I�����荞��
//  ICSR2.BIT.TEND = 1�Ŕ��� ( ICSR2.BIT.TDRE = 1 �̏�ԂŁASCL �N���b�N��9 �N���b�N�ڂ̗����オ��Ŕ���)
#pragma interrupt (Excep_RIIC0_TEI0(vect=249))
void Excep_RIIC0_TEI0(void){
	
	
         RIIC0.ICSR2.BIT.TEND = 0;		//  ���M�����t���O�̃N���A
	
	 if ( iic_sd_rcv_fg == 1 ) {		// �}�X�^����M�̏ꍇ(= ���X�^�[�g������ꍇ)
		RIIC0.ICCR2.BIT.RS = 1;		// ���X�^�[�g�R���f�B�V�����̔��s 
	 }
	 
	 else {					// �}�X�^���M�ŁA�S�f�[�^�̑��M������
	  
	 	RIIC0.ICIER.BIT.TEIE = 0;	// ���M�I�����荞��(TEI)�v���̋֎~
		RIIC0.ICCR2.BIT.SP = 1;	       // �X�g�b�v�R���f�B�V�����̔��s�v���̐ݒ�
	
	 }	    
	 

}






//  �����x�Z���T�̃X�e�[�^�X�ǂݏo�� (�}�X�^��M)(���荞�ݎg�p)
// RIIC ���M�o�b�t�@
//   �@iic_sd_data[0] : �X���[�u�A�h���X(7bit) + 1(R/W#�r�b�g=Read)
// RIIC ��M�o�b�t�@
//     iic_rcv_data[0]: �X�e�[�^�X

void rd_sensor_status(void)
{
	iic_sd_data[0] = (( iic_slave_adrs << 1 ) | 0x01 ) ;  // �X���[�u�A�h���X����Ǐo��
	
	riic_master_rcv (1);		//�@�}�X�^��M�@�J�n
	
	while( iic_com_over_fg != 1 ) {		// �ʐM�����҂�(��M�����҂�)
	}
	
	riic_sensor_status = iic_rcv_data[0];  // �Z���T�̃X�e�[�^�X
	
}




//  �����x�Z���T����X�e�[�^�X�Ɖ����x�f�[�^�̓ǂݏo�� (�}�X�^��M)(���荞�ݎg�p)
// RIIC ���M�o�b�t�@
//   �@iic_sd_data[0] : �X���[�u�A�h���X(7bit) + 1(=Read)
// RIIC ��M�o�b�t�@
//     iic_rcv_data[0]: �X�e�[�^�X
//             :   [1]: ���x�f�[�^(b19-b12)
//             :   [2]: ���x�f�[�^(b11-b4)
//             :   [3]��b7-b4: ���x�f�[�^(b3-b0)
//             :   [3]��b3-b0: ���x�f�[�^(b19-b16)
//             :   [4]: ���x�f�[�^(b15-b8)
//             :   [5]: ���x�f�[�^(b7-b0)
//             :   [6]: CRC 
void rd_sensor_humi_temp(void)
{
	
	iic_sd_data[0] = (( iic_slave_adrs << 1 ) | 0x01 ) ;  // �X���[�u�A�h���X����Ǐo��

	riic_master_rcv (7);		//�@�}�X�^��M�@�J�n (750usec������) (SCLK = 100 KHz)
	
	
	
	 
}






// �����x�Z���T�ւ̑���J�n�R�}���h���M�@(�}�X�^���M)(���荞�ݎg�p)
//	 IIC ���M�o�b�t�@
//   �@iic_sd_data[0] : �X���[�u�A�h���X(7bit) + 0(=wite)
//                [1] : Trigger measure(0xAC)
//                [2] : Data0(0x33)
//                [3] : Data1(0x00)
void wr_sensor_cmd(void)
{
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 ) ;  // �X���[�u�A�h���X�֏�������
	iic_sd_data[1] = 0xac;
	iic_sd_data[2] = 0x33;
	iic_sd_data[3] = 0x00;
	 
	riic_master_send (4);	// �}�X�^���M(���荞�ݎg�p)
	
	
}




// 
//  �����x�Z���T���瓾���f�[�^���A
//  ���x�Ɖ��x���v�Z����B
//    CRC�ُ�̏ꍇ�́A0�Ƃ���B
//
void Cal_humidity_temperature(void)
{
	uint32_t dt;
	uint32_t dt_h;
	uint32_t dt_m;
	uint32_t dt_l;
	
	
	crc_x8_x5_x4_1 = Calc_crc_x8_x5_x4_1(&iic_rcv_data[0],6);   // CRC-8(X8+X5+X4+1)�̌v�Z
	riic_crc_ng =  Calc_crc_x8_x5_x4_1(&iic_rcv_data[0],7);     // ���M���ꂽCRC���܂߂Čv�Z
	
	
	if ( riic_crc_ng == 0 ) { // CRC����v�����ꍇ�A�����x�̌v�Z
	
		dt_h = iic_rcv_data[1];		// ���x�f�[�^(b19-b12)
		dt_h = dt_h << 12;
	
        	dt_m = iic_rcv_data[2];		// ���x�f�[�^(b11-b4)
		dt_m = dt_m << 4;
	
		dt_l = iic_rcv_data[3];		// b7-b4: ���x�f�[�^(b3-b0)
		dt_l = dt_l >> 4;
	
		dt = dt_h | dt_m | dt_l;
	
		dt =  dt * 1000;		
		dt = dt >> 10;			// 1/1024 = 1/(2^10)
		dt = dt >> 10;
		riic_sensor_humidity = dt;     // ���x�f�[�^ (784�Ȃ��78.4%)
	
	
		dt_h = iic_rcv_data[3] & 0x0f; // b3-b0: ���x�f�[�^(b19-b16)
		dt_h = dt_h << 16;
	
		dt_m = iic_rcv_data[4];		// ���x�f�[�^(b15-b8)
		dt_m = dt_m << 8;
	
		dt_l = iic_rcv_data[5];		// ���x�f�[�^(b7-b0)
	
		dt = dt_h | dt_m | dt_l;
	
		dt =  dt * 200 *10;		
		dt = dt >> 10;
		dt = dt >> 10;
		dt = dt - 500;
	
		riic_sensor_temperature = dt;		// ���x�f�[�^ (283�Ȃ��28.3��)
	}
        else {
		riic_sensor_humidity = 0;
		riic_sensor_temperature = 0;
	}
	
	
	float_sensor_humidity = riic_sensor_humidity / 10.0;
	
	float_sensor_temperature = riic_sensor_temperature/ 10.0;
	
}



// CRC-8�̌v�Z (AHT25�p)
// CRC-8-Maxim: X8+X5+X4+1 (0x31) �����l=0xff
//
// ���L�T���v���v���O���������p
// STM32 �� AHT20 ���[�`�� (aht20_stm32 demo v1_4)�v (http://www.aosong.com/class-36.html)
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


// PEC�ɂ��f�[�^�`�F�b�N (�T�[���p�C���p)
// CRC-8-ATM: X8+X2+X1+1 (0x07) �����l=0x00
//
// ����M�f�[�^����ACPU��CRC���Z����g�p���ċ��߂�B
// ��:
//�@�@iic_sd_data[0] = 0x7a;   (�X���[�u�A�h���X=3D + R/W#(Write=0))
//    iic_sd_data[1] = 0x71;   (�R�}���h Object temperature read)
//    iic_sd_data[2] = 0x7b;   (�X���[�u�A�h���X=3D + R/W#(Read=1))
//
//    iic_rcv_data[0] = 0xdd;  (�Ώە��̉��x ���ʃo�C�g)
//    iic_rcv_data[1] = 0x01;  (�Ώە��̉��x ��ʃo�C�g)
//    iic_rcv_data[1] = 0xb8;  PEC(Packet error code)
//
//   �S�f�[�^(0x7a,0x71,0x7b,0xdd,0x01,0xb8)���ACRC.CRCDIR�ɓ����B
//   CRC.CRCDOR = 0�ł���΁A�f�[�^�Ɍ�薳���B
// 
// �Q�l: �uRX23E-A�O���[�v ���[�U�[�Y�}�j���A���@�n�[�h�E�F�A�� (R01UH0801JJ0120 Rev.1.20)�v
//�@�@�@�@32.2.3 CRC �f�[�^�o�̓��W�X�^�iCRCDOR�j
//    
void Cal_crc_thermo_pile(void)
{
	uint32_t i;
	
	CRC.CRCCR.BYTE = 0x85;		     // CRCDOR���W�X�^���N���A, MSB�t�@�[�X�g�ʐM�p��CRC�𐶐�, 8�r�b�gCRC�iX8 + X2 + X + 1�j

	for ( i = 0 ; i < 3 ; i++ ) {	     // CRC-8�̌v�Z(���M�f�[�^)
	   CRC.CRCDIR = iic_sd_data[i];
	}
	
	CRC.CRCDIR = iic_rcv_data[0];	    // CRC-8�̌v�Z(��M�f�[�^)
	CRC.CRCDIR = iic_rcv_data[1];
		     
	smbus_crc_8 = CRC.CRCDOR;	   // CRC�v�Z����(PEC)
 
	CRC.CRCDIR = iic_rcv_data[2];     // PEC�@
	       
	smbus_crc_ng = CRC.CRCDOR;        // ��M����PEC�܂�CRC�v�Z�B0�Ȃ�΃f�[�^����
}





// 
//  ���ˉ��x�v(�T�[���p�C��)���瓾���f�[�^���A
//  Self temperature��Object temperature���v�Z����B
//    CRC�ُ�̏ꍇ�́A0�Ƃ���B
//
void Cal_Ta_To_temperature(void)
{
	if ( smbus_crc_ng == 0 ) {   // CRC ����̏ꍇ
		     
	    if( iic_sd_data[1] == 0x70 ) {					// Ta(Self temperature)�̓ǂݏo�̏ꍇ
			  ta_word_temp =  iic_rcv_data[1];
		    	  ta_word_temp =  ( ta_word_temp << 8 );
		          ta_word_temp =  (ta_word_temp | iic_rcv_data[0]);
		          ta_celsius = ( ta_word_temp * 0.125) - 20.0;  
	     }
	     else if ( iic_sd_data[1] == 0x71 ){				// To(Object temperature)�̓ǂݏo�̏ꍇ
			  to_word_temp =  iic_rcv_data[1];
		          to_word_temp =  ( to_word_temp << 8 );
		          to_word_temp =  ( to_word_temp | iic_rcv_data[0]);
		          to_celsius = ( to_word_temp * 0.125) - 30.0;  
	      	        }
         }

	 else{			// PEC �ُ�
		ta_celsius = 0.0;
		to_celsius = 0.0;
	 }

	
}



//
//  ���ˉ��x�v(�T�[���p�C��)����̃f�[�^�ǂݏo���@(�}�X�^����M) �i���荞�ݎg�p)
//
// ����: rd_obj= 0: �Z���T�̎��͉��x(Self temperature)(TA)��ǂݏo��
///            = 1: ����ΏۑΏە��̉��x(TO)��ǂݏo��
//
//   IIC ���M�o�b�t�@
//   �@iic_sd_data[0] : �X���[�u�A�h���X(7bit) + 0(=Write)
//     iic_sd_data[1] : �R�}���h(�ǂݏo���A�h���X)  0x70=���͉��x�ǂݏo��, 0x71=����Ώۉ��x�ǂݏo��
//     iic_sd_data[2] : �X���[�u�A�h���X(7bit) + 1(=Read)
//
void rd_thermo_pile(uint32_t rd_obj)
{
	
	if ( rd_obj == 0 ) {	// �Z���T�̎��͉��x(Self temperature)(TA)��ǂݏo��
		iic_sd_data[1] = 0x70;
		
	}
	else {				// ����ΏۑΏە��̉��x(TO)��ǂݏo��
		iic_sd_data[1] = 0x71;
	
	}
	
	iic_sd_data[0] = ( iic_slave_adrs << 1 );    // �������ݗp �X���[�u�A�h���X
	iic_sd_data[2] = (iic_sd_data[0] | 0x01);   // �ǂݏo���p�@�X���[�u�A�h���X 
	

	iic_sd_rcv_fg = 1;		// �}�X�^����M����
	
	riic_sd_start();		   //  RIIC ���M�J�n
	
	 
}


// RIIC �}�X�^��M
//   �X���[�u����Arcv_num�o�C�g��M���āA��M�o�b�t�@�@iic_rcv_data[]�֊i�[����B
// ����: rcv_num  ��M�o�C�g��
// 

void riic_master_rcv ( uint8_t rcv_num)
{

	iic_sd_num = 1;			// ���M�f�[�^��
	iic_rcv_num = rcv_num;		// ��M�f�[�^��
	
	iic_sd_rcv_fg = 0;		// �}�X�^���M�܂��̓}�X�^��M
	riic_sd_start();		// RIIC ���M�J�n
		
}



// RIIC �}�X�^���M
//   �X���[�u�ց@���M�o�b�t�@�@iic_sd_data[]�̃f�[�^�� sd_num�o�C�g���M����B
// ����: sd_num  ���M�o�C�g���@
// 
//   
void riic_master_send ( uint8_t sd_num)
{
	iic_sd_num = sd_num;		// ���M�f�[�^��
	iic_rcv_num = 0;		// ��M�f�[�^��
	
	iic_sd_rcv_fg = 0;		// �}�X�^���M�܂��̓}�X�^��M
	
	riic_sd_start();		// RIIC ���M�J�n
}


//  RIIC ���M�J�n
void riic_sd_start(void)
{
	iic_sd_pt = 0;				 // ���M�f�[�^�ʒu�@�N���A
	iic_rcv_pt = 0;                          // ��M�f�[�^�ʒu

	iic_com_over_fg = 0;			// �ʐM�����t���O�̃N���A
	
	RIIC0.ICIER.BIT.TIE = 1;		// ���M�f�[�^�G���v�e�B���荞��(TXI)�v���̋���
	
	while(RIIC0.ICCR2.BIT.BBSY == 1){ 	// I2C�o�X�r�W�[��Ԃ̏ꍇ�A���[�v
	}
	
	RIIC0.ICCR2.BIT.ST = 1;		// �X�^�[�g�R���f�B�V�����̔��s  (�}�X�^���M�̊J�n)
					// �X�^�[�g�R���f�B�V�������s��AICSR2.TDRE(���M�f�[�^�G���v�e�B�t���O)=1�ƂȂ�A
					//  TXI(���M�f�[�^�G���v�e�B)���荞�݁A����
}


//  I2C(SMBus)�C���^�[�t�F�C�X �̏����� 
// 
//   	PORT16 = SCL
//      PORT17 = SDA
//
//      PCLKB = 32MHz:
//
//      �]�����x= 1 / { ( (ICBRH + 1) + (ICBRL + 1) ) / (IIC Phy) + SCLn ���C�������オ�莞��(tr) + SCLn ���C�����������莞��(tf) }
//
//       (������  29.2.14 I2C �o�X�r�b�g���[�gHigh ���W�X�^(ICBRH)�@���)
//
//     ( ����:�u RX23E-A�O���[�v ���[�U�[�Y�}�j���A���@�n�[�h�E�F�A�ҁv (R01UH0801JJ0120 Rev.1.20)�j 
//


void RIIC0_Init(void)
{
	RIIC0.ICCR1.BIT.ICE = 0;    // RIIC�͋@�\��~(SCL,SDA�[�q��쓮���)
	RIIC0.ICCR1.BIT.IICRST = 1; // RIIC���Z�b�g�A
	RIIC0.ICCR1.BIT.ICE = 1;    // �������Z�b�g��� �ASCL0�ASDA0�[�q�쓮���
		
	RIIC0.ICSER.BYTE = 0x00;    // I2C�o�X�X�e�[�^�X�����W�X�^ �i�}�X�^����̂��߃X���[�u�ݒ�͖���)	
	
				    //  �ʐM���x = 100 kbps (�I�V������l 102.6 kbps)
//	RIIC0.ICMR1.BIT.CKS = 3;    // RIIC�̓�����N���b�N = 32/8 = 4 MHz�@
//	RIIC0.ICBRH.BIT.BRH = 0xEF; // �����́@�u�\29.5 �]�����x�ɑ΂���ICBRH�AICBRL���W�X�^�̐ݒ��v��� (PCLK=PCLKB=32[MHz])
//	RIIC0.ICBRL.BIT.BRL = 0xF2;
	
				    // Fast mode ��p �ʐM���x = 400 kbps
				    // �I�V���� 390 kbos
//	RIIC0.ICMR1.BIT.CKS = 1;    // RIIC�̓�����N���b�N = 32/2 = 16 MHz�@
//	RIIC0.ICBRH.BIT.BRH = 0xEE; // 
//	RIIC0.ICBRL.BIT.BRL = 0xF4;
	
	
				    //  �ʐM���x = 100 kbps (�I�V������l 97.6 kbps)
	RIIC0.ICMR1.BIT.CKS = 3;    // RIIC�̓�����N���b�N = 32/8 = 4 MHz�@
	RIIC0.ICBRH.BIT.BRH = 0xF0; // 
	RIIC0.ICBRL.BIT.BRL = 0xF3;
	
	
	
	RIIC0.ICMR3.BIT.ACKWP = 1;	// ACKBT�r�b�g�ւ̏������݋���		
						
					
					
	RIIC0.ICMR3.BIT.RDRFS = 1;	// RDRF�t���O(��M�f�[�^�t��)�Z�b�g�^�C�~���O
					// 1�FRDRF �t���O��8 �N���b�N�ڂ̗����オ��Łg1�h �ɂ��A8 �N���b�N�ڂ̗����������SCL0 ���C����Low �Ƀz�[���h���܂��B
					// ����SCL0 ���C����Low �z�[���h��ACKBT �r�b�g�ւ̏������݂ɂ���������܂��B
					//���̐ݒ�̂Ƃ��A�f�[�^��M��A�N�m���b�W�r�b�g���o�O��SCL0 ���C���������I��Low �Ƀz�[���h���邽�߁A
					// ��M�f�[�^�̓��e�ɉ�����ACK (ACKBT �r�b�g���g0�h) �܂���NACK (ACKBT �r�b�g���g1�h) �𑗏o���鏈�����\�ł��B
			
					
	RIIC0.ICMR3.BIT.WAIT = 0;	// WAIT�Ȃ� (9�N���b�N�ڂ�1�N���b�N�ڂ̊Ԃ�Low�Ƀz�[���h���Ȃ�)	
	
	RIIC0.ICMR3.BIT.SMBS = 1;       // SMBus�I�� 				
	
	 
	RIIC0.ICCR1.BIT.IICRST = 0;	 // RIIC���Z�b�g����
}




//
//
//  I2C(SMBus)�C���^�[�t�F�C�X�p�̃|�[�g��ݒ�
// 
//   	PORT16 = SCL
//      PORT17 = SDA
//

void RIIC0_Port_Set(void)
{
	
	MPC.PWPR.BIT.B0WI = 0;      // �}���`�t�@���N�V�����s���R���g���[���@�v���e�N�g����
    	MPC.PWPR.BIT.PFSWE = 1;     // PmnPFS ���C�g�v���e�N�g����
    
    	MPC.P16PFS.BYTE = 0x0f;     // PORT16 = SCL0
    	MPC.P17PFS.BYTE = 0x0f;     // PORT17 = SDA0
          
    	MPC.PWPR.BYTE = 0x80;      //  PmnPFS ���C�g�v���e�N�g �ݒ�
  
    	PORT1.PMR.BIT.B6 = 1;     // PORT16:���Ӄ��W���[���Ƃ��Ďg�p
    	PORT1.PMR.BIT.B7 = 1;     // PORT17:      :
}



// RIIC �̊��荞�ݗp�A���荞�݃R���g���[���̐ݒ�
// �ȉ����A���荞�ݏ����ōs��
//   EEI: �ʐM�G���[/�ʐM�C�x���g (NACK ���o�A�X�^�[�g�R���f�B�V�������o�A�X�g�b�v�R���f�B�V�������o)
//�@ RXI:�@��M�f�[�^�t��
//   TXI:  ���M�f�[�^�G���v�e�B
//   TEI:  ���M�I��

void RIIC0_Init_interrupt(void)
{
					// �ʐM�G���[/�ʐM�C�x���g ���荞��
	IPR(RIIC0,EEI0) = 0x04;		// ���荞�݃��x�� = 4�@�@�i15���ō����x��)
	IR(RIIC0,EEI0) = 0;		// ���荞�ݗv���̃N���A
	IEN(RIIC0,EEI0) = 1;		// ���荞�݋���	
	
					// ��M�f�[�^�t��
	IPR(RIIC0,RXI0) = 0x04;		// ���荞�݃��x�� = 4�@�@�i15���ō����x��)
	IR(RIIC0,RXI0) = 0;		// ���荞�ݗv���̃N���A
	IEN(RIIC0,RXI0) = 1;		// ���荞�݋���	
	
					// ���M�f�[�^�G���v�e�B
	IPR(RIIC0,TXI0) = 0x04;		// ���荞�݃��x�� = 4�@�@�i15���ō����x��)
	IR(RIIC0,TXI0) = 0;		// ���荞�ݗv���̃N���A
	IEN(RIIC0,TXI0) = 1;		// ���荞�݋���	
	
					// ���M�I��
	IPR(RIIC0,TEI0) = 0x04;		// ���荞�݃��x�� = 4�@�@�i15���ō����x��)
	IR(RIIC0,TEI0) = 0;		// ���荞�ݗv���̃N���A
	IEN(RIIC0,TEI0) = 1;		// ���荞�݋���	
	
	
	
	RIIC0.ICIER.BIT.TMOIE = 0;	// �^�C���A�E�g���荞��(TMOI)�v���̋֎~
	RIIC0.ICIER.BIT.ALIE  = 0;   	// �A�[�r�g���[�V�������X�g���荞��(ALI)�v���̋֎~
	
	RIIC0.ICIER.BIT.STIE  = 1;	// �X�^�[�g�R���f�B�V�������o���荞��(STI)�v���̋���
	RIIC0.ICIER.BIT.SPIE  = 1;      // �X�g�b�v�R���f�B�V�������o���荞��(SPI)�v���̋���
	RIIC0.ICIER.BIT.NAKIE  = 1;	// NACK��M���荞��(NAKI)�v���̋���

	RIIC0.ICIER.BIT.RIE = 1;	// ��M�f�[�^�t�����荞��(RXI)�v���̋���
	RIIC0.ICIER.BIT.TIE = 0;	// ���M�f�[�^�G���v�e�B���荞��(TXI)�v���̋֎~
	RIIC0.ICIER.BIT.TEIE = 0;	// ���M�I�����荞��(TEI)�v���̋֎~
	
}
