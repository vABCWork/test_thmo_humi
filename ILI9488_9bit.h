
// 表示色(0:白, 1:緑, 2:赤:, 3:青, 4:黄, 5:シアン, 6:マゼンタ)

#define COLOR_WHITE		0
#define COLOR_GREEN		1
#define COLOR_RED		2
#define COLOR_BLUE		3
#define COLOR_YELLOW		4
#define COLOR_CYAN		5
#define COLOR_MAGENTA           6


extern uint8_t rgb111_data_buf[9800];
	

void ILI9488_Init(void);

void ILI9488_Reset(void);

void lcd_adrs_set( uint16_t col, uint16_t page, uint16_t col2, uint16_t page2);

void spi_cmd_2C_send( void );

void pixel_write_test();

void color_bar(void);
void disp_black(void);


