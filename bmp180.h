#ifndef _BMP180_H_
#define _BMP180_H_

typedef struct {
	int16_t	AC1;
	int16_t	AC2;
	int16_t	AC3;
	uint16_t	AC4;
	uint16_t	AC5;
	uint16_t	AC6;
	int16_t	B1;
	int16_t	B2;
	int16_t	MB;
	int16_t	MC;
	int16_t	MD;
} bmp180_config;

typedef struct {
	uint8_t oss;
	// uint32_t B5_last_update;
	int32_t B5;
} bmp180_state;

int32_t bmp180_calculate_T(bmp180_config *config, bmp180_state *state, int32_t UT);
int32_t bmp180_calculate_P(bmp180_config *config, bmp180_state *state, int32_t UP);
void bmp180_test_init(void);
uint8_t bmp180_test_T(void);
uint8_t bmp180_test_P(void);
void bmp180_test(void);

#endif /* _BMP180_H_ */
