#include <NewPing.h>
#include <Wire.h>
#include "Decoder.h"
#include <ZumoShield.h>
#include <NeoSWSerial.h>

// This is the auto development
#define LED 13

// This might need to be tuned for different lighting conditions, surfaces, etc.
// 700 is the sweet spot for the white background
#define QTR_THRESHOLD  700 // microseconds

// These might need to be tuned for different motor types
#define REVERSE_SPEED     200 // 0 is stopped, 400 is full speed
#define TURN_SPEED        200 // ms
#define FORWARD_SPEED     200 // ms
#define SEARCH_SPEED      200 // ms
#define REVERSE_DURATION  200 // ms
#define TURN_DURATION     300 // ms
#define SUSTAINED_SPEED   400 // Switches to SUSTAINED_SPEED from FULL_SPEED after FULL_SPEED_DURATION_LIMIT in ms
#define FULL_SPEED        400 // ms
#define STOP_DURATION     100 // ms

#define RIGHT 1
#define LEFT -1

#define MINIMUM_PING      0   // The minimum for ultrasonic sensor
#define RA_SIZE 3             // Number of readings to include in running average of accelerometer readings
#define XY_ACCELERATION_THRESHOLD 2400  // for detection of contact (~16000 = magnitude of acceleration due to gravity)

//ultra-sonic sensor variables
#define TRIGGER_PIN  2  // connected to arduino board pin number
#define ECHO_PIN     6  // Connected to arduino board pin number
#define MAX_DISTANCE 30 // CM

#define STOP_SPEED   0  // Setting engine speed to zero
#define NUM_SENSORS  6  // Number of sensors available on Zumo

unsigned int sensor_values[NUM_SENSORS];
bool border = false;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);     // NewPing setup of pins and maximum distance.
ZumoMotors motors;                                      // Zumo Morts Setup
ZumoReflectanceSensorArray sensors(QTR_NO_EMITTER_PIN); // Reflector Sensor setup
ZumoBuzzer buzzer;                                      // Buzzer Setup
NeoSWSerial NodeMCU(12, 3);                             // Initializing the wifi chip and assigning arduino pin numbers 


enum State_enum {
  STOP,
  FORWARD,
  BACKWARD,
  FORWARD_ROTATE_RIGHT,
  FORWARD_ROTATE_LEFT,
  BACKWARD_ROTATE_RIGHT,
  BACKWARD_ROTATE_LEFT,
  ZUMO_AUTO,
  PLAYER_NAME
};
enum Sensors_enum {NONE, SENSOR_RIGHT, SENSOR_LEFT, BOTH};


enum ForwardSpeed { SearchSpeed, SustainedSpeed, FullSpeed };
ForwardSpeed _forwardSpeed;             // current forward speed setting
unsigned long full_speed_start_time;    // Defining the variable for the time zumo is on full speed
#define FULL_SPEED_DURATION_LIMIT 250   // ms

// Sound Effects

const char sound_effect[] PROGMEM = "O4 T100 V15 L4 MS g12>c12>e12>G6>E12 ML>G2"; // "charge" melody

// Timing declerations
unsigned long loop_start_time;
unsigned long last_turn_time;
unsigned long contact_made_time;
#define MIN_DELAY_AFTER_TURN         400   // ms = min delay before detecting contact event
#define MIN_DELAY_BETWEEN_CONTACTS   1000  // ms = min delay between detecting new contact event

template <typename T>
class RunningAverage
{
  public:
    RunningAverage(void);
    RunningAverage(int);
    ~RunningAverage();
    void clear();
    void addValue(T);
    T getAverage() const;
    void fillValue(T, int);
  protected:
    int _size;
    int _cnt;
    int _idx;
    T _sum;
    T * _ar;
    static T zero;
};

// Accelerometer Class -- extends the LSM303 Library to support reading and averaging the x-y acceleration
//   vectors from the onboard LSM303DLHC accelerometer/magnetometer
class Accelerometer : public LSM303
{
    typedef struct acc_data_xy
    {
      unsigned long timestamp;
      int x;
      int y;
      float dir;
    } acc_data_xy;

  public:
    Accelerometer() : ra_x(RA_SIZE), ra_y(RA_SIZE) {};
    ~Accelerometer() {};
    void enable(void);
    void getLogHeader(void);
    void readAcceleration(unsigned long timestamp);
    float len_xy() const;
    float dir_xy() const;
    int x_avg(void) const;
    int y_avg(void) const;
    long ss_xy_avg(void) const;
    float dir_xy_avg(void) const;
  private:
    acc_data_xy last;
    RunningAverage<int> ra_x;
    RunningAverage<int> ra_y;
};
// Prototypes for all the functions and variable declaration
void state_machine_run(uint8_t sensors);
void motors_stop();
void motors_forward();
void motors_backward();
void motors_forward_right();
void motors_forward_left();
void motors_backward_right();
void motors_backward_left();
void zumo_auto();
uint8_t read_state();

uint8_t state = STOP;
Decoder* decoder = NULL;
int MOTOR1 = 0;
int MOTOR2 = 0;
 
Accelerometer lsm303;
boolean in_contact;  // set when accelerometer detects contact with opposing robot

int health = 100;
bool isNameInitialised = false;
// forward declaration
void setForwardSpeed(ForwardSpeed speed);

unsigned long last = 0;

void setup() {
  // Initiate the Wire library and join the I2C bus as a master
  Wire.begin();
  // Initiate hardware serial
  Serial.begin(9600);
  // Initiate software serial used for mqtt publishing through node mcu
  NodeMCU.begin(9600);
  decoder = Decoder::Instance();

  // Initiate LSM303
  lsm303.init();
  lsm303.enable();

  randomSeed((unsigned int) millis());

  pinMode(LED, HIGH);

  waitForButtonAndCountDown(false);

  motors.setSpeeds(0,0);
}

void waitForButtonAndCountDown(bool restarting)
{
  #ifdef LOG_SERIAL
    Serial.print(restarting ? "Restarting Countdown" : "Starting Countdown");
    Serial.println();
  #endif

  digitalWrite(LED, HIGH);
  digitalWrite(LED, LOW);

  // play audible countdown
  for (int i = 0; i < 3; i++)
  {
    delay(1000);
    // buzzer.playNote(NOTE_G(3), 50, 12);
  }
  
  delay(1000);
  //buzzer.playFromProgramSpace(sound_effect);
  delay(1000);

  // Reset loop variables
  in_contact = false;  // 1 if contact made; 0 if no contact or contact lost
  contact_made_time = 0;
  last_turn_time = millis();  // prevents false contact detection on initial acceleration
  _forwardSpeed = SearchSpeed;
  full_speed_start_time = 0;
}

// Main loop of the program
void loop() {
  state_machine_run(read_state());
}
//Using state pattern to determine the actions of Zumo
void state_machine_run(uint8_t state)
{
  loop_start_time = millis();
  lsm303.readAcceleration(loop_start_time);

  if(state != ZUMO_AUTO){
    if(health <= 0){
      motors.setSpeeds(STOP_SPEED, STOP_SPEED);
      if((millis() - last) > 1000){
        last = millis();
        NodeMCU.print(decoder->player_name);
        NodeMCU.print("|DEAD=");
        NodeMCU.println(health);
      }
      return;
    }
    
    if(check_for_contact()){
      on_contact_made();
      health = health - 10;
      NodeMCU.print(decoder->player_name);
      NodeMCU.print("|HEALTH=");
      NodeMCU.print(health);
      NodeMCU.println();
    }
  }
  
  switch (state)
  {
    case STOP:
      motors_stop();
      break;
    case FORWARD:
      motors_forward();
      break;
    case BACKWARD:
      motors_backward();
      break;
    case FORWARD_ROTATE_RIGHT:
      motors_forward_right();
      last_turn_time = millis();
      break;
    case FORWARD_ROTATE_LEFT:
      motors_forward_left();
      last_turn_time = millis();
      break;
    case BACKWARD_ROTATE_LEFT:
      motors_backward_left();
      last_turn_time = millis();
      break;
    case BACKWARD_ROTATE_RIGHT:
      motors_backward_right();
      last_turn_time = millis();
      break;
    case ZUMO_AUTO:
      zumo_auto();
    case PLAYER_NAME:
      init_player_name();
      break;
  }
  
}

void init_player_name(){
  if(!isNameInitialised){
    NodeMCU.print(decoder->player_name);
    NodeMCU.print("|HEALTH=");
    NodeMCU.print(health);
    NodeMCU.println();
    
    isNameInitialised = true;
  }
}

// Waiting for the enum state sent from the app
uint8_t read_state()
{
  if (Serial.available() > 0){
    String received = Serial.readStringUntil(decoder->ENDING_CHAR);
    
    decoder->DecodeNewSerial(received);
    
    if(decoder->GetState() == "PLAYER-NAME"){
      return PLAYER_NAME;
    }
    
    MOTOR1 = decoder->GetSpeedForMotor1();
    MOTOR2 = decoder->GetSpeedForMotor2();
      
    if (decoder->GetState() == "FORWARD")
    {
      return FORWARD;
    }
    else if (decoder->GetState() == "BACKWARD")
    {
      return BACKWARD;
    }
    else if (decoder->GetState() == "FORWARD|LEFT")
    {
      return FORWARD_ROTATE_LEFT;
    }
    else if (decoder->GetState() == "FORWARD|RIGHT")
    {
      return FORWARD_ROTATE_RIGHT;
    }
    else if (decoder->GetState() == "BACKWARD|LEFT")
    {
      return BACKWARD_ROTATE_LEFT;
    }
    else if (decoder->GetState() == "BACKWARD|RIGHT")
    {
      return BACKWARD_ROTATE_RIGHT;
    }
    else if (decoder->GetState() == "ZUMO|AUTO")
    {
      return ZUMO_AUTO; // return enum
    }
    else if(decoder->GetState() == "STOP")
    {
      return STOP;
    }
  }else if(decoder->isAutoMode){
    return ZUMO_AUTO; // return enum
  }

}

// Speed cases fro the zumo motors
void motors_stop()
{
  motors.setSpeeds(STOP_SPEED, STOP_SPEED);
}

void motors_forward()
{
  motors.setSpeeds(MOTOR1, MOTOR2);
}

void motors_backward()
{
  motors.setSpeeds(MOTOR1, MOTOR2);
}

void motors_forward_right()
{
  motors.setSpeeds(MOTOR1, MOTOR2);
}

void motors_forward_left()
{
  motors.setSpeeds(MOTOR1, MOTOR2);
}

void motors_backward_right()
{
  motors.setSpeeds(MOTOR1, MOTOR2);
}

void motors_backward_left()
{
  motors.setSpeeds(MOTOR1, MOTOR2);
}


// Autonomous function for zumo 
void zumo_auto() {
  loop_start_time = millis();
  lsm303.readAcceleration(loop_start_time);
  sensors.read(sensor_values);

  if(health <= 0){
    motors.setSpeeds(STOP_SPEED, STOP_SPEED);
    if((millis() - last) > 1000){
      NodeMCU.print(decoder->player_name);
      NodeMCU.print("|DEAD=");
      NodeMCU.println(health);
    }
    return;
  }
  
  if ((_forwardSpeed == FullSpeed) && (loop_start_time - full_speed_start_time > FULL_SPEED_DURATION_LIMIT))
  {
    setForwardSpeed(SustainedSpeed);
  }

  if (sensor_values[0] > QTR_THRESHOLD)
  {
    // If leftmost sensor detects line, reverse and turn to the right
    turn(RIGHT, true);
  }
  else if (sensor_values[5] > QTR_THRESHOLD)
  {
    // Of rightmost sensor detects line, reverse and turn to the left
    turn(LEFT, true);
  }
  else  // Otherwise, go straight
  {

    if(check_for_contact()){
      on_contact_made();
      health = health - 10;
      NodeMCU.print(decoder->player_name);
      NodeMCU.print("|HEALTH=");
      NodeMCU.print(health);
      NodeMCU.println();
    }
    
    if (sonar.ping_cm() > MINIMUM_PING) {
        on_contact_made();
    }

    int speed = getForwardSpeed();
    motors.setSpeeds(speed, speed);


  }
}

void turn(char direction, bool randomize)
{
  #ifdef LOG_SERIAL
    Serial.print("turning ...");
    Serial.println();
  #endif

  // Assume contact lost
  on_contact_lost();

  static unsigned int duration_increment = TURN_DURATION / 4;

  // motors.setSpeeds(0,0);
  // delay(STOP_DURATION);
  motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
  delay(REVERSE_DURATION);
  motors.setSpeeds(TURN_SPEED * direction, -TURN_SPEED * direction);
  delay(randomize ? TURN_DURATION + (random(8) - 2) * duration_increment : TURN_DURATION);
  int speed = getForwardSpeed();
  motors.setSpeeds(speed, speed);
  last_turn_time = millis();
}

void setForwardSpeed(ForwardSpeed speed)
{
  _forwardSpeed = speed;
  if (speed == FullSpeed) full_speed_start_time = loop_start_time;
}

int getForwardSpeed()
{
  int speed;
  switch (_forwardSpeed)
  {
    case FullSpeed:
      speed = FULL_SPEED;
      break;
    case SustainedSpeed:
      speed = SUSTAINED_SPEED;
      break;
    default:
      speed = SEARCH_SPEED;
      break;
  }
  return speed;
}

// Check for contact, but ignore readings immediately after turning or losing contact
bool check_for_contact()
{
  static long threshold_squared = (long) XY_ACCELERATION_THRESHOLD * (long) XY_ACCELERATION_THRESHOLD;
  return (lsm303.ss_xy_avg() >  threshold_squared) && \
         (loop_start_time - last_turn_time > MIN_DELAY_AFTER_TURN) && \
         (loop_start_time - contact_made_time > MIN_DELAY_BETWEEN_CONTACTS);
}

// Sound horn and accelerate on contact -- fight or flight
void on_contact_made()
{

  #ifdef LOG_SERIAL
    Serial.print("contact made");
    Serial.println();
  #endif
  
  in_contact = true;
  contact_made_time = loop_start_time;
  setForwardSpeed(FullSpeed);
  //buzzer.playFromProgramSpace(sound_effect);
}

// reset forward speed
void on_contact_lost()
{
  #ifdef LOG_SERIAL
    Serial.print("contact lost");
    Serial.println();
  #endif
  in_contact = false;
  setForwardSpeed(SearchSpeed);
}

// class Accelerometer -- member function definitions

// enable accelerometer only
// to enable both accelerometer and magnetometer, call enableDefault() instead
void Accelerometer::enable(void)
{
  // Enable Accelerometer
  // 0x27 = 0b00100111
  // Normal power mode, all axes enabled
  writeAccReg(LSM303::CTRL_REG1_A, 0x27);

  if (getDeviceType() == LSM303::device_DLHC)
    writeAccReg(LSM303::CTRL_REG4_A, 0x08); // DLHC: enable high resolution mode
}

void Accelerometer::getLogHeader(void)
{
  Serial.print("millis    x      y     len     dir  | len_avg  dir_avg  |  avg_len");
  Serial.println();
}

void Accelerometer::readAcceleration(unsigned long timestamp)
{
  readAcc();
  if (a.x == last.x && a.y == last.y) return;

  last.timestamp = timestamp;
  last.x = a.x;
  last.y = a.y;

  ra_x.addValue(last.x);
  ra_y.addValue(last.y);

  #ifdef LOG_SERIAL
    Serial.print(last.timestamp);
    Serial.print("  ");
    Serial.print(last.x);
    Serial.print("  ");
    Serial.print(last.y);
    Serial.print("  ");
    Serial.print(len_xy());
    Serial.print("  ");
    Serial.print(dir_xy());
    Serial.print("  |  ");
    Serial.print(sqrt(static_cast<float>(ss_xy_avg())));
    Serial.print("  ");
    Serial.print(dir_xy_avg());
    Serial.println();
  #endif
}

float Accelerometer::len_xy() const
{
  return sqrt(last.x * a.x + last.y * a.y);
}

float Accelerometer::dir_xy() const
{
  return atan2(last.x, last.y) * 180.0 / M_PI;
}

int Accelerometer::x_avg(void) const
{
  return ra_x.getAverage();
}

int Accelerometer::y_avg(void) const
{
  return ra_y.getAverage();
}

long Accelerometer::ss_xy_avg(void) const
{
  long x_avg_long = static_cast<long>(x_avg());
  long y_avg_long = static_cast<long>(y_avg());
  return x_avg_long * x_avg_long + y_avg_long * y_avg_long;
}

float Accelerometer::dir_xy_avg(void) const
{
  return atan2(static_cast<float>(x_avg()), static_cast<float>(y_avg())) * 180.0 / M_PI;
}



// RunningAverage class
// based on RunningAverage library for Arduino
// source:  https://playground.arduino.cc/Main/RunningAverage
// author:  Rob.Tillart@gmail.com
// Released to the public domain

template <typename T>
T RunningAverage<T>::zero = static_cast<T>(0);

template <typename T>
RunningAverage<T>::RunningAverage(int n)
{
  _size = n;
  _ar = (T*) malloc(_size * sizeof(T));
  clear();
}

template <typename T>
RunningAverage<T>::~RunningAverage()
{
  free(_ar);
}

// resets all counters
template <typename T>
void RunningAverage<T>::clear()
{
  _cnt = 0;
  _idx = 0;
  _sum = zero;
  for (int i = 0; i < _size; i++) _ar[i] = zero; // needed to keep addValue simple
}

// adds a new value to the data-set
template <typename T>
void RunningAverage<T>::addValue(T f)
{
  _sum -= _ar[_idx];
  _ar[_idx] = f;
  _sum += _ar[_idx];
  _idx++;
  if (_idx == _size) _idx = 0;  // faster than %
  if (_cnt < _size) _cnt++;
}

// returns the average of the data-set added so far
template <typename T>
T RunningAverage<T>::getAverage() const
{
  if (_cnt == 0) return zero; // NaN ?  math.h
  return _sum / _cnt;
}

// fill the average with a value
// the param number determines how often value is added (weight)
// number should preferably be between 1 and size
template <typename T>
void RunningAverage<T>::fillValue(T value, int number)
{
  clear();
  for (int i = 0; i < number; i++)
  {
    addValue(value);
  }
}
