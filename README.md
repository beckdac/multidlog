# sensor logger for trinket m0

Narrative
1. Boot
  1. Initialize devices
    1. BMP180
    2. MPU6050
	3. AT24C256
 2. Prepare devices
    1. take reading of temperature and pressure BMP180
	2. get accel and gyro *and* temperature mpu6050
	3. compare mpu6050 T w/ BMP180 T
	4. check version / magic in EEPROM zero byte, mark as init or not init
	5. if not EEPROM init, write MPU6050 configuration to EEPROM after zero byte; write version / magic to zero byte; mark as init
2. Main loop, "in parallel"
  * Sample accelerometer
    1. save data in ring buffer
	2. on "big change" set EEPROM write enabled to true
  * EEPROM write enabled? 
  	1. write next data from ring buffer
  	2. free slot in ring buffer
  	3. wait until write finished
  * Observe keystroke (state dependent)
  	* state Main
		1. <enter> - display menu
		2. <c> - state = Setup
		3. <f> - state = Free Running
		4. <d> - state = Download
  	* state Free Running
		1. set EEPROM write enabled to true
  	* state Setup
		1. unimplemented
  	* state Download
		1. send begin download code
		2. read EEPROM data stream length
		3. read EEPROM from data start for length bytes and puke raw bytes to serial
		4. return to state Main
