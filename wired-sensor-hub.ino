#define MY_RF24_CE_PIN 41
#define MY_RF24_CS_PIN 38
#define MY_RF24_IRQ_PIN 2
#define MY_BAUD_RATE 38400
#define MY_RADIO_RF24
#define MY_NODE_ID 24
#define MY_RX_MESSAGE_BUFFER_FEATURE
//#define MY_DEBUG
#define MY_REPEATER_FEATURE

#include <MySensors.h> 

#define CUT_THRESHOLD 900
#define SECURE_THRESHOLD 125
#define ALARM_THRESHOLD 250
#define REPEAT_STATE_WINDOW 30000

int motionSensorPins[] = {A0, A1, A2, A3, A4, A5, A6, A7, A8};
int sensorValue[16];
char sensorState[16];
unsigned long lastTimeSensorUpdated[16];
MyMessage* msgs[16];
MyMessage* valueMessages[16];

void presentation() { 
  // Send the sketch version information to the gateway
  sendSketchInfo("wired-sensor-hub", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  for (int i = 0; i < (sizeof(motionSensorPins)/sizeof(int)); i++) {
    msgs[i]= new MyMessage(i, V_TRIPPED);
    valueMessages[i] = new MyMessage(i, V_VAR1);
    present(i, S_MOTION);
  }
}

void setup() {
  Serial.begin(38400);
  Serial.println("Setup started");
  
  for (int i = 0; i < (sizeof(motionSensorPins)/sizeof(int)); i++) { 
    initSensorPin(i);
  }
}

void loop() {
  for (int i = 0; i < (sizeof(motionSensorPins)/sizeof(int)); i++) { 
    checkSensor(i);
  }
  
  wait(100);
}

void initSensorPin(int sensor) {
  pinMode(motionSensorPins[sensor], INPUT);
  digitalWrite(motionSensorPins[sensor], HIGH);  // set pullup on analog pin
}

void sendSensorState(int sensor, const char *state) {
  if (sensorState[sensor] != *state || millis() - lastTimeSensorUpdated[sensor] >= REPEAT_STATE_WINDOW) {

//    Serial.print(sensor);
//    Serial.print("[");
//    Serial.print(sensorValue[sensor]);
//    Serial.print("]");
//    Serial.println(*state);
    MyMessage msg = *msgs[sensor];
    MyMessage valMsg = *valueMessages[sensor];
    send(msg.set(state));
    send(valMsg.set(sensorValue[sensor]));
    sensorState[sensor] = *state;
    lastTimeSensorUpdated[sensor] = millis();
  }
}

void checkSensor(int sensor) {
  sensorValue[sensor] = analogRead(motionSensorPins[sensor]);

  //Serial.print("Sensor index=");
  //Serial.print(sensor);
  //Serial.print(", value=");
  //Serial.println(sensorValue[sensor]);

  if (sensorValue[sensor] > CUT_THRESHOLD) {
    //sensorState[sensor] = CUT;
    sendSensorState(sensor, "1");
  } else if (sensorValue[sensor] > ALARM_THRESHOLD && sensorValue[sensor] <= CUT_THRESHOLD) {
    //alarmCooldownStart[sensor] = millis();  
    //sensorState[sensor] = ALARM;
    sendSensorState(sensor, "1");
  } else if (sensorValue[sensor] > SECURE_THRESHOLD && sensorValue[sensor] <= ALARM_THRESHOLD) {
    sendSensorState(sensor, "0");
  } else {
    //sensorState[sensor] = SHORT_CISRCUIT;
    sendSensorState(sensor, "1");
  }
}
