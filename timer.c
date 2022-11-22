
#include "typedefine.h"
#include  "iodefine.h"
#include "misratypes.h"
#include "timer.h"
#include "key.h"


// Timer 
volatile uint8_t flg_100msec_interval;	// 100msec����ON

volatile uint8_t timer_10msec_cnt;      //�@(10msec���ɃJ�E���g�A�b�v)


// �^�b�`�L�[
volatile uint8_t flg_touch_cmd;		// �^�b�`�ǂݏo���R�}���h���s�t���O (0=�R�}���h�����s, 1=�R�}���h���s�ς�)

//  �R���y�A�}�b�`�^�C�} CMT0
//   10msec���̊��荞��
//

#pragma interrupt (Excep_CMT0_CMI0(vect=28))

void Excep_CMT0_CMI0(void){
	
	uint32_t i;

	timer_10msec_cnt++;	       // �J�E���g�̃C���N�������g
	
	if ( timer_10msec_cnt > 199 ) {    // 2�b�o��
		
		timer_10msec_cnt = 0;	  //  �J�E���^�[�̃N���A
	}
	
	
	if ( (timer_10msec_cnt % 10 ) ==  0 ) {	   // 100msec�o�� (10�Ŋ������]�� = 0�̏ꍇ)
		
		flg_100msec_interval = 1;  // 100msec�t���O ON
	}
	
	if ( flg_touch_cmd == 0 ) {	// �R�}���h�����s�̏ꍇ
		
		xpt2046_comm_start();		//����M�J�n  (2Mbps�Ŗ�50[usec]������ )
		
		flg_touch_cmd = 1;	// �R�}���h���s�ς݃t���O�̃Z�b�g	
	}
	else if ( flg_touch_cmd == 1 ){
	  	
		xpt2046_xyz_press();		// X,Y,Z�̒l�𓾂�B�^�b�`�����v�Z�B
	 
	        touch_key_status_check();   // �^�b�`�L�[�̏�Ԃ𓾂�
	 
		for( i = 0; i < KEY_SW_NUM; i++ ) {   // �^�b�`����^�b�`��ԂƂȂ����L�[�������āA�L�[���͏����v���t���O���Z�b�g����B
		     switch_input_check(i);           //  (�O�񉟂��ꂽ�L�[���������Ă���)
	        }
		
		flg_touch_cmd = 0;		// �R�}���h���s�ς݃t���O�̃N���A	
	}
	
	
 	// xpt2046_cal_average();			// X���AY��k�̑��茋�ʂ̕��ϒl�𓾂�  (�v���O�����쐬�p)
	
}





//
//    10msec �^�C�}(CMT0)
//    CMT���j�b�g0��CMT0���g�p�B 
//
//  PCLKB(=32MHz)��128�����ŃJ�E���g 32/128 = 1/4 MHz
//      1�J�E���g = 4/1 = 4 [usec]  
//    1*10,000 usec =  N * 4 usec 
//      N = 2500


void Timer10msec_Set(void)
{	
	IPR(CMT0,CMI0) = 3;		// ���荞�݃��x�� = 3�@�@�i15���ō����x��)
	IEN(CMT0,CMI0) = 1;		// CMT0 �����݋���
		
	CMT0.CMCR.BIT.CKS = 0x2;        // PCLKB/128       
	CMT0.CMCOR = 2499;		// CMCNT��0����J�E���g 	


}


//   CMT0 �^�C�}�J�n�@
//  ���荞�݋����ăJ�E���g�J�n

void Timer10msec_Start(void)
{	
	CMT0.CMCR.BIT.CMIE = 1;		// �R���y�A�}�b�`�����݁@����
		
	CMT.CMSTR0.BIT.STR0 = 1;	// CMT0 �J�E���g�J�n

	timer_10msec_cnt = 0;	  //  �J�E���^�[�̃N���A

}







