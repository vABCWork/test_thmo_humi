

// LCD�R���g���[�� Reset�p �o�̓|�[�g(PH3)�̒�`
#define LCD_RESET_PMR     (PORTH.PMR.BIT.B3)   //  �ėp���o�̓|�[�g
#define LCD_RESET_PDR     (PORTH.PDR.BIT.B3)   //  �o�̓|�[�g�Ɏw��
#define LCD_RESET_PODR    (PORTH.PODR.BIT.B3)  //  �o�̓f�[�^



extern	volatile uint8_t *spi_send_pt;		// ���M�f�[�^���i�[���Ă���ꏊ
extern  volatile uint8_t spi_sending_fg;	// ���M�� = 1
extern  volatile uint32_t spi_send_num;		// ���M����o�C�g��



void Excep_RSPI0_SPTI0(void);

void RSPI_Init_Port(void);

void RSPI_Init_Reg(void);

void RSPI_SPCMD_0(void);

void spi_data_send( uint32_t sd_num, uint8_t *data_pt, uint8_t cmd_flg);

void  spi_data_send_wait(void);