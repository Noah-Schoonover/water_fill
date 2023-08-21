#define RED_LED 10
#define WATER_SENSOR_POWER_PIN 15
#define VALVE_OUTPUT_PIN 9
#define WATER_SENSOR_SIGNAL_PIN A0

static const int WATER_START_FILL_THRESHOLD = 150;
static const int WATER_STOP_FILL_THRESHOLD = 300;

static const int SERIAL_INIT_MILLIS = 200;
static const int WATER_SENSOR_INIT_MILLIS = 200;

static const int SLEEP_CHECK_SENSOR_INTERVAL_MILLIS = 20000;  // how often to check sensor while not filling (sleeping)
static const int FILL_CHECK_SENSOR_INTERVAL_MILLIS = 1000;    // how often to check sensor while filling
static const int MAX_FILL_ERROR_MILLIS = 30000;               // how long to fill before going to error mode

//--------------------------------------------------------------------------------------------------------------------------
// this method is called automatically by arduino before running loop()
//
void setup() {

  Serial.begin(9600);
  delay(SERIAL_INIT_MILLIS);
  Serial.println("started");
  pinMode(RED_LED, OUTPUT);
  pinMode(VALVE_OUTPUT_PIN, OUTPUT);
  pinMode(WATER_SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(WATER_SENSOR_POWER_PIN, LOW);

}

//--------------------------------------------------------------------------------------------------------------------------
//
void enableWaterSensor() {
  digitalWrite(WATER_SENSOR_POWER_PIN, HIGH);
  delay(WATER_SENSOR_INIT_MILLIS); // wait for sensor to initialize/equalize
}

//--------------------------------------------------------------------------------------------------------------------------
//
void disableWaterSensor() {
  digitalWrite(WATER_SENSOR_POWER_PIN, LOW);
}

//--------------------------------------------------------------------------------------------------------------------------
// enables an LED along with the valve
//
void openWaterValve() {
  digitalWrite(RED_LED, HIGH); // turn LED on
  digitalWrite(VALVE_OUTPUT_PIN, HIGH); // open valve
}

//--------------------------------------------------------------------------------------------------------------------------
// turns off the LED along with the valve
//
void closeWaterValve() {
  digitalWrite(RED_LED, LOW); // turn LED off
  digitalWrite(VALVE_OUTPUT_PIN, LOW); // close valve
}

//--------------------------------------------------------------------------------------------------------------------------
// this infinite loop will get called if the FILL_TIME_MAX_ERROR_MILLIS is reached during fill
// reset/power cycle required to exit this loop
// the loop quickly cycles the power on the sensor to flash the sensor LED as an ERROR indicator
//
void doSystemErrorLoop() {
  closeWaterValve();
  while(true) {
    delay(500);
    Serial.println("ERROR");
    digitalWrite(RED_LED, HIGH);
    enableWaterSensor();

    digitalWrite(RED_LED, LOW);
    disableWaterSensor();

    digitalWrite(RED_LED, HIGH);
    enableWaterSensor();

    digitalWrite(RED_LED, LOW);
    disableWaterSensor();
  }
}

//--------------------------------------------------------------------------------------------------------------------------
// helper method to write the sensor value after reading it
//
int readWaterSensor() {
  int value = analogRead(WATER_SENSOR_SIGNAL_PIN); // read the analog value from sensor
  Serial.print("Sensor value: "); // output sensor value
  Serial.println(value);
  return value;
}

//--------------------------------------------------------------------------------------------------------------------------
// continuously checks the water sensor value while filling
// if the MAX_FILL_ERROR_MILLIS is reached during fill, the doSystemErrorLoop() method will be called
//
void doFill() {

  unsigned long startFillTime = millis();
  int waterSensorValue = 0;
  openWaterValve();

  while (waterSensorValue < WATER_STOP_FILL_THRESHOLD) {

    delay(FILL_CHECK_SENSOR_INTERVAL_MILLIS);
    unsigned long time = millis();
    Serial.print("fill time: ");
    Serial.println(time - startFillTime);

    if (time > startFillTime + MAX_FILL_ERROR_MILLIS) {
      Serial.println("ERROR - MAX FILL TIME REACHED.");
      closeWaterValve();
      doSystemErrorLoop();
    }

    Serial.println("filling...");
    waterSensorValue = readWaterSensor();
  }

  closeWaterValve();
  Serial.println("done filling.");

}

//--------------------------------------------------------------------------------------------------------------------------
// this method is called automatically by arduino repeatedly after setup()
//
void loop() {

  unsigned long time = millis();
  Serial.print("Time (s): ");
  Serial.println(time / 1000);

  delay(SLEEP_CHECK_SENSOR_INTERVAL_MILLIS);

  enableWaterSensor();

  int waterSensorValue = readWaterSensor();
  
  if (waterSensorValue < WATER_START_FILL_THRESHOLD) {
    Serial.println("start fill");
    doFill();
  }
  
  disableWaterSensor();
  
}
