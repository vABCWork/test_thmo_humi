
// SWの個数
#define KEY_SW_NUM	2	// SWは2個  ℃のキー(SW0),Fのキー(SW1)


// スイッチ入力関係の構造体
struct SW_info
{
	uint8_t status;		// 今回の タッチ(Low=0),非タッチ(High=1) 状態 (10msec毎)
	uint8_t pre_status;	// 前回の   :
	uint8_t low_cnt;	// タッチ(ON)の回数
	uint8_t one_push;	// 0:キー入力処理要求なし 1:キー入力処理要求(1度押し)　（キー入力処理、表示処理終了後に0クリア）
        uint8_t long_push;      // 0:キー入力処理要求なし 1:キー入力処理要求(長押し)　 ( Low→High の立上がり検出時に 0クリア)
	
};



void touch_pre_status_ini(void);

void xpt2046_xyz_press(void);
void touch_key_status_check(void);
void switch_input_check( uint8_t id ); 

void key_input(void);	


extern volatile struct  SW_info  Key_sw[KEY_SW_NUM];	// スイッチ　5個分の情報格納領域
extern uint8_t  flg_disp_fahrenheit; 