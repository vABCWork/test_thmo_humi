
extern volatile uint8_t flg_100msec_interval;
extern volatile uint8_t timer_10msec_cnt;   


void  touch_key_one_push_check(void);
void  touch_long_key_check(void);
void  touch_long_key_clear(void);

void Timer10msec_Set(void);
void Timer10msec_Start(void);
