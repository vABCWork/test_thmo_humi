

extern volatile uint8_t  xpt_rcv_data[16];
extern volatile uint8_t  xpt_rcv_cnt;

extern volatile uint8_t xpt_sd_data[16];
extern volatile uint8_t xpt_send_num;
extern volatile uint8_t xpt_send_pt;

extern volatile uint8_t sci5_rxi_cnt;
extern volatile uint8_t sci5_tei_cnt;


void initSCI_5(void);


void xpt2046_comm_start(void);
void xpt2046_cmd_set(void);