
void disp_num_test(uint8_t index, uint8_t size_id, uint8_t color);

void disp_celsius(void);
void disp_fahrenheit(void);
void disp_percnt(void);

void  disp_index_data ( uint8_t index,  uint32_t start_column, uint32_t start_row, uint8_t size_id , uint8_t color );

void disp_float_data(float t, uint32_t start_column, uint32_t start_row, uint8_t size_id , uint8_t color);
void disp_byte_data(uint8_t  val , uint32_t start_column, uint32_t start_page, uint8_t color);

void spi_data_send_id(uint8_t index, uint8_t size_id, uint8_t color  );

void unpack_font_data ( uint32_t len, uint8_t * src_ptr , uint8_t color_index );