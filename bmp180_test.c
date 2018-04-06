#include <stdio.h>

#include "config.h"
#include "bmp180.h"

extern bmp180_config config;
extern bmp180_state state;

// values from bmp180 datasheet
void bmp180_test_init(void) {
		config.AC1 = 408;
		config.AC2 = -72;
		config.AC3 = -14383;
		config.AC4 = 32741;
		config.AC5 = 32757;
		config.AC6 = 23153;
		config.B1 = 6190;
		config.B2 = 4;
		config.MB = -32768;
		config.MC = -8711;
		config.MD = 2868;

		state.oss = 0;
}

uint8_t bmp180_test_T(void) {
		int32_t T;

		printf("bmp180_calculate_T ... ");
		T = bmp180_calculate_T(&config, &state, 27898);
		if (T != 150) {
			printf("failed!\n");
			return 0;
		}
		printf("passed\n");
		return 1;
}

uint8_t bmp180_test_P(void) {
		int32_t P;

		printf("bmp180_calculate_P ... ");
		P = bmp180_calculate_P(&config, &state, 23843);
		if (P != 69964) {
			printf("failed!\n");
			return 0;
		}
		printf("passed\n");
		return 1;
}

void bmp180_test(void) {
	int passes = 0, possible = 0;
	printf("testing bmp180 module\n");
	bmp180_test_init();
	passes += bmp180_test_T();
	possible++;
	passes += bmp180_test_P();
	possible++;
	printf("passed %d/%d tests (%.2f%%)\n", passes, possible, (float)passes / (float)possible * 100.);
}
