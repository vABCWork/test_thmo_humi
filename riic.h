
extern	uint8_t iic_slave_adrs; 
extern  volatile uint8_t iic_rcv_data[16];
extern  volatile uint8_t iic_sd_data[32];

extern  volatile uint8_t iic_com_over_fg;

extern  uint8_t smbus_crc_8;
extern  uint8_t smbus_crc_ng; 

extern uint8_t riic_sensor_status;
extern float	float_sensor_humidity;	
extern float	float_sensor_temperature;

extern	uint8_t  crc_x8_x5_x4_1;
extern	uint8_t  riic_crc_ng;

void rd_sensor_status(void);
void rd_sensor_humi_temp(void);
void wr_sensor_cmd(void);


void riic_master_rcv ( uint8_t rcv_num);
void riic_master_send ( uint8_t sd_num);

void riic_sd_start(void);

void Cal_humidity_temperature(void);
uint8_t Calc_crc_x8_x5_x4_1(volatile uint8_t *data, uint8_t num);

void rd_thermo_pile(uint32_t rd_obj);
void Cal_crc_thermo_pile(void);
void Cal_Ta_To_temperature(void);


void RIIC0_Init(void);
void RIIC0_Port_Set(void);
void RIIC0_Init_interrupt(void);