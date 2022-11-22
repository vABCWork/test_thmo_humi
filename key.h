
// SW�̌�
#define KEY_SW_NUM	2	// SW��2��  ���̃L�[(SW0),F�̃L�[(SW1)


// �X�C�b�`���͊֌W�̍\����
struct SW_info
{
	uint8_t status;		// ����� �^�b�`(Low=0),��^�b�`(High=1) ��� (10msec��)
	uint8_t pre_status;	// �O���   :
	uint8_t low_cnt;	// �^�b�`(ON)�̉�
	uint8_t one_push;	// 0:�L�[���͏����v���Ȃ� 1:�L�[���͏����v��(1�x����)�@�i�L�[���͏����A�\�������I�����0�N���A�j
        uint8_t long_push;      // 0:�L�[���͏����v���Ȃ� 1:�L�[���͏����v��(������)�@ ( Low��High �̗��オ�茟�o���� 0�N���A)
	
};



void touch_pre_status_ini(void);

void xpt2046_xyz_press(void);
void touch_key_status_check(void);
void switch_input_check( uint8_t id ); 

void key_input(void);	


extern volatile struct  SW_info  Key_sw[KEY_SW_NUM];	// �X�C�b�`�@5���̏��i�[�̈�
extern uint8_t  flg_disp_fahrenheit; 