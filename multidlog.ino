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
		// show a message and stop until user reboots with a key press
		Serial.println("FAIL");
		Serial.println("\nPress any key to reboot.\n");
		while (Serial.read() == -1);
		Watchdog.enable(1000);
		while(1);
	}
	// MPU6050
	Serial.print("MPU6050 ... ");
	if (mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G)) {
		Serial.println("ok");
	} else
		goto INIT_FAIL;
	// configuration
	mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
	mpu.setIntFreeFallEnabled(false);  
	mpu.setIntZeroMotionEnabled(false);
	mpu.setIntMotionEnabled(false);

	// take a measurement from each device to prime the pumps
	sampleAcquire(&sampleLast);
}

void sampleAcquire(sample_t *sample) {
	double tmpT, tmpP;
	unsigned long tmpTime;
	int wait = bmp.startTemperature();
	sample->time = millis();
	sample->gyro = mpu.readNormalizeGyro();
	sample->accel = mpu.readNormalizeAccel();
	while (millis() - sample->time < wait);
	bmp.getTemperature(tmpT);
	sample->temperature = (float)tmpT;
	tmpTime = millis();
	wait = bmp.startPressure(0);
	while (millis() - tmpTime < wait);
	bmp.getPressure(tmpP, tmpT);
	sample->pressure = (float)tmpP;
}

void samplePrint(sample_t *sample) {
	Serial.print("Last sample @ ");
	Serial.print(sample->time);
	Serial.print(" ms (");
	Serial.print(millis() - sample->time);
	Serial.println(" ms ago)");
	Serial.print("Gyro : ");
	Serial.print(sample->gyro.XAxis, 1);
	Serial.print(sample->gyro.YAxis, 1);
	Serial.println(sample->gyro.ZAxis, 1);
	Serial.print("Accel: ");
	Serial.print(sample->accel.XAxis, 1);
	Serial.print(sample->accel.YAxis, 1);
	Serial.println(sample->accel.ZAxis, 1);
	Serial.print("Temperature: ");
	Serial.print(sample->temperature, 0);
	Serial.print("  -  Pressure: ");
	Serial.println(sample->pressure, 1);
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
	Serial.print("-------- ======== [ ");
	Serial.print(string);
	Serial.println(" ] ======== --------");
}

void loop() {
	commandState_t cmdState = MAIN;
	int inputCh = 0;
	uint16_t samples = 0;
	unsigned long samples_start_time = 0;

	inputCh = Serial.read();
	switch (cmdState) {
		MAIN:
			switch (inputCh) {
				case -1:	// no data
					break;
				case 'c':
					cmdState = CONFIG;
					Serial.println("Entering configuration mode.");
					return;
				case 's':
					cmdState = STREAMING;
					Serial.println("Entering streaming mode. Press any key to stop.");
					return;
				case 'f':
					cmdState = FREERUNNING;
					Serial.println("Entering free running capture mode.");
					return;
				case 'd':
					cmdState = DOWNLOAD;
					Serial.println("Entering data download mode.");
					return;
				case 'h':
				default:
					printMenuHeader("Main Menu");
					Serial.print("c\tConfiguration menu\ns\tStreaming mode\nf\tFree running capture mode\nd\tDownload most recent capture data\nr\tReboot\nh\tHelp (this menu)\n");
					samplePrint(&sampleLast);
					return;
					break;
			};
			break;
		CONFIG:
			printMenuHeader("Config Menu");
			Serial.println("Not yet implemented.");
			cmdState = MAIN;
			break;
		STREAMING:
		FREE:
			// take continued samples
			sampleAcquire(&sampleLast);
			samples++;
			if (cmdState == STREAMING) {
				// print reading
				sampleDump(&sampleLast);
				// if received a key then stop
				if (inputCh != -1)
					cmdState = MAIN;
			} else {
				// save reading
				// if buffer full, go back to main and report out
					cmdState = MAIN;
			}
			if (cmdState == MAIN) {
				Serial.print("Aquired ");
				Serial.print(samples);
				Serial.print(" samples in ");
				Serial.print(millis() - samples_start_time);
				Serial.print(" d milliseconds.\n\nReturning to Main Menu. 'H' for help.\n");
				samples = 0;
				samples_start_time = 0;
			}
			break;
		DOWNLOAD:
			// read eeprom and send it
			cmdState = MAIN;
			break;
		REBOOT:
			// soft reset this device
			Watchdog.enable(1000);
			while(1);
			break;
		default:
			cmdState = MAIN;
			break;
	};
}
