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

void setup() {

  Serial.begin(9600);
  delay(SERIAL_INIT_MILLIS);
  Serial.println("started");
  pinMode(RED_LED, OUTPUT);
  pinMode(VALVE_OUTPUT_PIN, OUTPUT);
  pinMode(WATER_SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(WATER_SENSOR_POWER_PIN, LOW);

}

// this infinite loop will get called if the FILL_TIME_MAX_ERROR_MILLIS is reached
// before the sensor detects water. reset/power cycle required to exit this loop
// the loop quickly cycles the power on the sensor to flash the sensor LED as an ERROR indicator
void doSystemErrorLoop() {
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

void enableWaterSensor() {
  digitalWrite(WATER_SENSOR_POWER_PIN, HIGH);  // turn the sensor ON
  delay(WATER_SENSOR_INIT_MILLIS); // wait for sensor
}

void disableWaterSensor() {
  digitalWrite(WATER_SENSOR_POWER_PIN, LOW);  // turn the sensor OFF
}

int readWaterSensor() {
  int value = analogRead(WATER_SENSOR_SIGNAL_PIN); // read the analog value from sensor
  Serial.print("Sensor value: "); // output sensor value
  Serial.println(value);
  return value;
}

void openWaterValve() {
  digitalWrite(RED_LED, HIGH); // turn LED on when filling 
  digitalWrite(VALVE_OUTPUT_PIN, HIGH); // turn valve ON
}

void closeWaterValve() {
  digitalWrite(RED_LED, LOW); // turn LED off
  digitalWrite(VALVE_OUTPUT_PIN, LOW); // turn valve OFF
}

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
      Serial.println(millis());
      Serial.println(MAX_FILL_ERROR_MILLIS);
      Serial.println(startFillTime + MAX_FILL_ERROR_MILLIS);
      closeWaterValve();
      doSystemErrorLoop();
    }

    Serial.println("filling...");
    waterSensorValue = readWaterSensor();
  }

  closeWaterValve();
  Serial.println("done filling.");

}

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
