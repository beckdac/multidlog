#include <MPU6050.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <Adafruit_SleepyDog.h>

SFE_BMP180 bmp;
MPU6050 mpu;

typedef enum { 
	MAIN, CONFIG, STREAMING, FREERUNNING, 
	DOWNLOAD, REBOOT, HELP 
} commandState_t;

typedef struct {
	unsigned long time;
	Vector gyro;
	Vector accel;
	float temperature;
	float pressure;
} sample_t;

sample_t sampleLast;

#undef I2C_DISABLE
#define I2C_DISABLE

void printMainMenu(void);

void setup() {
	// global chip configuration

	// init serial and welcome the user
	Serial.begin(115200);
	while (!Serial) ; // wait for Arduino Serial Monitor (native USB boards)
	Serial.println("Hello...");

	// initialize each device
	// BMP180
	Serial.print("BMP180 ... ");
	if (bmp.begin()) {
		Serial.println("ok");
	} else {
INIT_FAIL:
#ifndef I2C_DISABLE
		// show a message and stop until user reboots with a key press
		Serial.println("FAIL");
		Serial.println("\nPress any key to reboot.\n");
		while (Serial.read() == -1);
		Watchdog.enable(1000);
		while(1);
#else
		Serial.println("i2c disabled");
#endif
	}
	// MPU6050
	Serial.print("MPU6050 ... ");
	if (mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
		Serial.println("ok");
	} else
#ifndef I2C_DISABLE
		goto INIT_FAIL;
#else
		Serial.println("i2c disabled");
#endif
	// configuration
#ifndef I2C_DISABLE
	mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
	mpu.setIntFreeFallEnabled(false);  
	mpu.setIntZeroMotionEnabled(false);
	mpu.setIntMotionEnabled(false);
#endif

	// take a measurement from each device to prime the pumps
	sampleAcquire(&sampleLast);
	printMainMenu();
}

// start and get are staggered with MPU reads
void sampleAcquire(sample_t *sample) {
#ifndef I2C_DISABLE
	double tmpT, tmpP;
	unsigned long tmpTime;

	int wait = bmp.startTemperature();
	sample->time = millis();
	sample->gyro = mpu.readNormalizeGyro();
	while (millis() - sample->time < wait);
	bmp.getTemperature(tmpT);
	sample->temperature = (float)tmpT;

	wait = bmp.startPressure(0);
	tmpTime = millis();
	sample->accel = mpu.readNormalizeAccel();
	while (millis() - tmpTime < wait);
	bmp.getPressure(tmpP, tmpT);
	sample->pressure = (float)tmpP;
#else
	memset(sample, 0, sizeof(sample_t));
	sample->time = millis();
#endif
}

void samplePrint(sample_t *sample) {
	Serial.print("Last sample @ ");
	Serial.print(sample->time);
	Serial.print(" ms (");
	Serial.print(millis() - sample->time);
	Serial.println(" ms ago)");
	Serial.print("Gyro : ");
	Serial.print(sample->gyro.XAxis, 1);
	Serial.print("\t");
	Serial.print(sample->gyro.YAxis, 1);
	Serial.print("\t");
	Serial.println(sample->gyro.ZAxis, 1);
	Serial.print("Accel: ");
	Serial.print(sample->accel.XAxis, 1);
	Serial.print("\t");
	Serial.print(sample->accel.YAxis, 1);
	Serial.print("\t");
	Serial.println(sample->accel.ZAxis, 1);
	Serial.print("Temperature: ");
	Serial.print(sample->temperature, 0);
	Serial.print("  -  Pressure: ");
	Serial.print(sample->pressure, 1);
	Serial.println("\n");
}

void sampleDump(sample_t *sample) {
	Serial.print("@\t");
	Serial.print(sample->time);
	Serial.print("\t");
	Serial.print(sample->gyro.XAxis, 1);
	Serial.print("\t");
	Serial.print(sample->gyro.YAxis, 1);
	Serial.print("\t");
	Serial.println(sample->gyro.ZAxis, 1);
	Serial.print("\t");
	Serial.print(sample->accel.XAxis, 1);
	Serial.print("\t");
	Serial.print(sample->accel.YAxis, 1);
	Serial.print("\t");
	Serial.print(sample->accel.ZAxis, 1);
	Serial.print("\t");
	Serial.print(sample->temperature, 1);
	Serial.print("\t");
	Serial.println(sample->pressure, 1);
}

void printMenuHeader(char *string) {
	Serial.print("\n-------- ======== [ ");
	Serial.print(string);
	Serial.println(" ] ======== --------");
}

void printMainMenu(void) {
	printMenuHeader("Main Menu");
	Serial.print("c\tConfiguration menu\ns\tStreaming mode\nf\tFree running capture mode\nd\tDownload most recent capture data\nr\tReboot\nh\tHelp (this menu)\n");
	samplePrint(&sampleLast);
	Serial.println("");
}

void loop() {
	commandState_t cmdState = MAIN;
	int inputCh = 0;
	uint16_t samples = 0;
	unsigned long samples_start_time = 0;

  while(1) {

	//Serial.println(".");
	inputCh = Serial.read();
	switch (cmdState) {
		case MAIN:
			switch (inputCh) {
				case -1:	// no data
					break;
				case 'c':
					cmdState = CONFIG;
					Serial.println("Entering configuration mode.");
					continue;
				case 's':
					cmdState = STREAMING;
					Serial.println("Entering streaming mode. Press any key to stop.");
					continue;
				case 'f':
					cmdState = FREERUNNING;
					Serial.println("Entering free running capture mode.");
					continue;
				case 'd':
					cmdState = DOWNLOAD;
					Serial.println("Entering data download mode.");
					continue;
				case 'r':
					cmdState = REBOOT;
					continue;
				case 'h':
				default:
					printMainMenu();
					continue;
			};
			break;
		case CONFIG:
			printMenuHeader("Config Menu");
			Serial.println("Not yet implemented.");
			cmdState = MAIN;
			break;
		case STREAMING:
		case FREERUNNING:
			// take continued samples
			sampleAcquire(&sampleLast);
			samples++;
			if (cmdState == STREAMING) {
				// print reading
				sampleDump(&sampleLast);
				// if received a key then stop
				if (inputCh != -1) {
					Serial.println("Leaving streaming mode.");
					cmdState = MAIN;
				}
			} else {
				// save reading
				// if buffer full, go back to main and report out
				Serial.println("Leaving free running mode.");
					cmdState = MAIN;
			}
			if (cmdState == MAIN) {
				Serial.print("Aquired ");
				Serial.print(samples);
				Serial.print(" samples in ");
				Serial.print(millis() - samples_start_time);
				Serial.print(" milliseconds.\n\nReturning to Main Menu. 'h' for help.\n");
				samples = 0;
				samples_start_time = 0;
			}
			break;
		case DOWNLOAD:
			// read eeprom and send it
			Serial.print("Leaving data download mode.");
			cmdState = MAIN;
			break;
		case REBOOT:
			// soft reset this device
			Serial.println("Rebooting via watch dog in 1 second.");
			Watchdog.enable(1000);
			while(1);
			break;
		default:
			cmdState = MAIN;
			break;
	};
  }
}
