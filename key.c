#include "iodefine.h"
#include  "misratypes.h"
#include  "timer.h"
#include  "key.h"

#include "sci_spi.h"


volatile struct  SW_info  Key_sw[KEY_SW_NUM];	// �X�C�b�`�@6���̏��i�[�̈�


// X���AY��k�̑���l�̕��Ϗ����p
// ����l��12bit�f�[�^�����A��� 8bit�������Ă���

uint8_t   touch_x_val;		// X���f�[�^
uint8_t   touch_y_val;          // Y���f�[�^

uint8_t   touch_z1_val;		// Z�����@(�^�b�`������p)
uint8_t   touch_z2_val;

uint8_t   tc_val_pt;		// ����l�̊i�[�ʒu

uint8_t   touch_x[8];		// 8�񕪂̑���l
uint8_t   touch_y[8];


uint16_t   touch_x_average;	// ���ϒl
uint16_t   touch_y_average;


float	touch_resistance;	// �^�b�`��R

float   touch_z1_z2;		// z2/z1


//  ���AF�@���ʗp
uint8_t  flg_disp_fahrenheit;   // 1:(F �؎��\��)

// 
// �^�b�`��Ԃ̏����� (  �d��ON�����1����{ )
//  
//   �S�Ĕ�^�b�`(SW OFF: High = 1 )�Ƃ���
//   ����l�̊i�[�ʒu�̏�����
//
void touch_pre_status_ini(void)
{
	uint32_t i;
		
	for ( i = 0 ; i < KEY_SW_NUM; i++ ) {		//  �S�Ĕ�^�b�`(SW OFF: High = 1 )�Ƃ���
	
	     Key_sw[i].pre_status = 1;
	
	}
	
 	tc_val_pt = 0;		// �i�[�ʒu�̃N���A	
}


// X���AY��k�̑��茋�ʂ̕��ϒl�𓾂� (�v���O�����쐬�p)
//�@�����ꂽ�L�[�𔻒肷��v���O�����쐬���ɁA�K�v�ȃf�[�^�𓾂�B
//(���ۂ̃L�[���͔��菈���ɂ͕��ϒl�͎g�p���Ă��Ȃ�)
// 12bit�f�[�^�̏�� 8bit���g�p tc_val_pt
void xpt2046_cal_average(void)
{
	uint32_t  i;
	uint32_t  x_avg;
	uint32_t  y_avg;
	

	touch_x[tc_val_pt] =  xpt_rcv_data[1];   // X������f�[�^�� (b11-b4)
	touch_y[tc_val_pt] =  xpt_rcv_data[4];   // Y������f�[�^�� (b11-b4)
	
	x_avg = 0;			// ���ϒl�̌v�Z
	y_avg = 0;
	
	for ( i = 0 ; i < 8 ; i++ ) {	// 8�񕪂̑��a�𓾂� 
	      x_avg = x_avg + touch_x[i];
	      y_avg = y_avg + touch_y[i];
	 }
	   
	touch_x_average =  x_avg >> 3;   // ���� 8
	touch_y_average =  y_avg >> 3;   // ���� 8
	

	if ( tc_val_pt < 7 ) {		// 
	
	   tc_val_pt = tc_val_pt + 1;	// �i�[�ʒu�̃C���N�������g
	}
	else  {				
	  
	   tc_val_pt = 0;		// �i�[�ʒu�̃N���A	
	}
	 
}



//
//   X,Y,Z1,Z2�𓾂�܂��A��R�l���v�Z����B
//
// �^�b�`�p�l����Z����(�t�B�����Ƃ��̉��ɂ���LCD�K���X�Ƃ̏㉺����)�̒�R�l(Rz)�𓾂�
//  �^�b�`����Ă��Ȃ���Ԃł́A�傫���l�ɂȂ�B
//
//     Rz = Rx_plate * ( X������l /4096 )* ( ( Z2 / Z1 ) - 1 )
//
//    Rx_plate: �^�b�`�X�N���[����X�v���[�g(�t�B������)�̒�R�l 270[��] (����)

void xpt2046_xyz_press(void)
{
	touch_x_val = xpt_rcv_data[1];	// X������f�[�^�� b11-b4
	touch_y_val = xpt_rcv_data[4];  // Y������f�[�^�� b11-b4
	
	touch_z1_val =  xpt_rcv_data[7];  // Z1 ����f�[�^�� b11-b4
	
	touch_z2_val = xpt_rcv_data[10];  // Z2 ����f�[�^�� b11-b4
	  
	if ( touch_z1_val > 0 ) {
	   touch_z1_z2 = ( touch_z2_val / touch_z1_val ); //  Z2/Z1 
	
	   touch_resistance = 270.0 * ( touch_x_val / 4096.0 ) * ( touch_z1_z2 - 1.0 );
        }
	else{
	   touch_resistance = 999.9;
	}		
}



// �@
// �^�b�`�L�[�̏�Ԃ𓾂�
//  ��R�l�� 20���𒴂���ꍇ�́A��^�b�`�Ƃ���B
//  20���ȉ��̏ꍇ�A�^�b�`����Ă���Ɣ��f����B
// 
// X����Y���̓ǂݏo���f�[�^�ɂ��A�����ꂽ�L�[�𔻒f�B
//
//  �c�ɔz�u:  ��
//              F
//
// �@�@�@�@�@�@    �^�b�`����   ��(SW0)         F (SW1)        �@    
//  touch_x_val:�@�@ 0x00      0x32�`0x48     0x32�`0x48   
//  touch_y_val:     0x7f      0x1b�`0x29     0x06�`0x15     
//
///
//


void touch_key_status_check(void)
{
	uint32_t i;
	

	if ( touch_resistance > 20.0 ) {		// ��R�l��20���𒴂���ꍇ�A
	  for ( i = 0 ; i < KEY_SW_NUM; i++ ) {		//  �S�Ĕ�^�b�`(SW OFF: High = 1 )�Ƃ���
	     Key_sw[i].status = 1;
	  }
	  
	  return;  
	}
							// �����ꂽ�L�[�̔���
	if (( touch_x_val >= 0x32 ) && (  touch_x_val <= 0x48 )){        //  
	
	   if (( touch_y_val >= 0x1b ) && (  touch_y_val <= 0x29 )){   //   ���L�[(SW0) �^�b�`
	  	Key_sw[0].status = 0;				       // �����ꂽ�L�[���^�b�`����(SW ON: Low = 0 )�Ƃ���
	    }
	    else if (( touch_y_val >= 0x06 ) && (  touch_y_val <= 0x15 )){     //   F�L�[(SW1) �^�b�`
	  	Key_sw[1].status = 0;						
	    }
	}
	
}

//
// �@�X�C�b�`�̓��͔��� (20msec��1��@���s)
//
// �T�v:
//   �^�b�`��Ԃ�4��p����A��^�b�`�ɂ�΃^�b�`���ꂽ�Ƃ���B
//
//    ���肷��X�C�b�`:
//                  0 = ���̃L�[ (SW0)  
//                  1 =  F�̃L�[ (SW1) 
//                
//             
//  
void switch_input_check( uint8_t id ) 
{
	
         if ( Key_sw[id].status == 0 ) {     // ���� �^�b�`���
	
	     if ( Key_sw[id].pre_status == 1 ) {  // �O�� ��^�b�`��ԁ@�@(�������茟�o)
	           Key_sw[id].low_cnt =  1;       // Low�J�E���g = 1 �Z�b�g
	     
	     }
	     else {				 // �O�� �^�b�`��ԁ@
	          Key_sw[id].low_cnt = Key_sw[id].low_cnt + 1; // Low�J�E���g�̃C���N�������g  
	     }
	     
	  } 

	  else{				// ����@��^�b�`���
	   
	      if ( Key_sw[id].pre_status == 0 ) {  // �O�� �^�b�`��� (���オ�茟�o)
	     
	          if ( Key_sw[id].low_cnt > 3 ) {   // 4��ȏ�@�^�b�`���o�̏ꍇ
	             
		      Key_sw[id].one_push = 1;	 // �L�[���͏����v��(1�񉟂�)�Z�b�g
		      
		      Key_sw[id].low_cnt = 0;	//  Low�J�E���g�̃N���A  
	          }
	      }
	  }	  
	  	
	  Key_sw[id].pre_status = Key_sw[id].status;   // ���݂̏�Ԃ��A��O�̏�ԂփR�s�[
	  
}
   


//
// �L�[���͏���
//   0 = SW0 , ���x�̒P�ʂ́A��(�ێ�)�ŕ\��
//   1 = SW1 , ���x�̒P�ʂ́AF(�؎�)�ŕ\��
//
void key_input(void)	
{
	uint32_t    i;
	
	if (  Key_sw[0].one_push == 1 ) {	    // �� key
		
		flg_disp_fahrenheit = 0;
	       
	}
	
	else if ( Key_sw[1].one_push == 1 ) {	   // F key
		
		flg_disp_fahrenheit = 1;
		
	}
	
	
	for ( i = 0 ; i < KEY_SW_NUM ; i++ ) {	// �X�C�b�` ��x�����̏����N���A
		Key_sw[i].one_push = 0;
	}
	
	
}


