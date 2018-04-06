#include "config.h"
#include "bmp180.h"

bmp180_config config;
bmp180_state  state;

// Compute the temperature in steps of 0.1 deg C
// This formula is taken from the BMP180 data sheet
int32_t bmp180_calculate_T(bmp180_config *config, bmp180_state *state, int32_t UT) {
	int32_t X1, X2, B5, T;

	X1 = (((int32_t)UT - (int32_t)config->AC6) * (int32_t)config->AC5)
			>> 15; // shift 15 bit = 2^15

	X2 = ((int32_t)config->MC << 11) / (X1 + (int32_t)config->MD);
	B5 = X1 + X2;
	T = (B5 + 8) >> 4;

	state->B5 = B5;
	return T;
}

// Compute the pressure in steps of 1.0 Pa
// This formula is taken from the BMP180 data sheet
int32_t bmp180_calculate_P(bmp180_config *config, bmp180_state *state, int32_t UP) {
	int32_t B6, X1, X2, X3, B3, P;
	uint32_t B4, B7;

	B6 = state->B5 - 4000;
	X1 = ((int32_t)config->B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = ((int32_t)config->AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((int32_t)config->AC1 * 4 + X3) << state->oss) + 2) / 4;
	X1 = ((int32_t)config->AC3 * B6) >> 13;
	X2 = ((int32_t)config->B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) / 4;
	B4 = ((uint32_t)config->AC4 * (uint32_t)(X3 + 32768)) >> 15;
	B7 = ((uint32_t)UP - B3) * (50000 >> state->oss);
	if (B7 < 0x80000000)
		P = (B7 * 2) / B4;
	else
		P = (B7 / B4) * 2;
	X1 = P >> 8;
	X1 *= X1;
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * P) >> 16;
	P = P + ((X1 + X2 + 3791) >> 4);

	return P;
}
