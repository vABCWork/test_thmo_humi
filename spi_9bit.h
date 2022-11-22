

// LCDコントローラ Reset用 出力ポート(PH3)の定義
#define LCD_RESET_PMR     (PORTH.PMR.BIT.B3)   //  汎用入出力ポート
#define LCD_RESET_PDR     (PORTH.PDR.BIT.B3)   //  出力ポートに指定
#define LCD_RESET_PODR    (PORTH.PODR.BIT.B3)  //  出力データ



extern	volatile uint8_t *spi_send_pt;		// 送信データを格納している場所
extern  volatile uint8_t spi_sending_fg;	// 送信中 = 1
extern  volatile uint32_t spi_send_num;		// 送信するバイト数



void Excep_RSPI0_SPTI0(void);

void RSPI_Init_Port(void);

void RSPI_Init_Reg(void);

void RSPI_SPCMD_0(void);

void spi_data_send( uint32_t sd_num, uint8_t *data_pt, uint8_t cmd_flg);

void  spi_data_send_wait(void);