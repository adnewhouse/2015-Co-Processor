#include <stdint.h>
#include <avr/eeprom.h>

#define ADC_VAL(voltage) (voltage*1024/5)
#define CUTOFF ADC_VAL(0.837)
#define BIN_MIN ADC_VAL(0.348)
#define BIN_MAX ADC_VAL(0.629)

#define N_SENSORS 6
#define SENSOR(n) (sensors + n)
#define PERIOD 10
/*
0: tote1
1: tote2
2: tote3
3: tote4
4: tote5
5: bin
6: checksum
7: zero
*/

typedef struct {
  int pin;
  uint16_t min;
  uint16_t max;
  unsigned int sum;
  uint16_t values[PERIOD];
} sensor_t;

typedef struct {
  uint16_t min;
  uint16_t max;
} eedata_t;

sensor_t sensors[N_SENSORS];
eedata_t EEMEM eedata[N_SENSORS];
eedata_t ram_eedata[N_SENSORS];

void init_sensor(int n, int pin){
  sensor_t *sensor = SENSOR(n);
//  sensor->min = min;
//  sensor->max = max;
  sensor->sum = 0;
  sensor->pin = pin;
  int i;
  for(i = 0; i < PERIOD; i++){
    sensor->values[i] = 0;
  }
}

void load_sensors(){
  eeprom_read_block(ram_eedata, eedata, sizeof(eedata_t) * N_SENSORS);
  int i;
  for(i = 0; i < N_SENSORS; i++){
    sensors[i].min = ram_eedata[i].min;
    sensors[i].max = ram_eedata[i].max;
  }
}
void store_sensor(int n){
  ram_eedata[n].min = sensors[n].min;
  ram_eedata[n].max = sensors[n].max;
  eeprom_write_block((ram_eedata + n), (eedata + n), sizeof(eedata_t));
}

int index = 0;
uint8_t first = 1;

uint8_t prev_data = 0xff;
void setup(){
  Serial.begin(115200);
  init_sensor(0, A0);
  init_sensor(1, A1);
  init_sensor(2, A2);
  init_sensor(3, A3);
  init_sensor(4, A4);
  init_sensor(5, A5);
  load_sensors();
  int i;
/*  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);*/
}

void loop(){
  int i;
  for(i = 0; i < N_SENSORS; i++){
    sensor_t *sensor = SENSOR(i);
    sensor->sum -= sensor->values[index];
    int new_val = analogRead(sensor->pin);
    sensor->sum += new_val;
    sensor->values[index] = new_val;
  };
  index++;
  if(index == PERIOD){
    index = first = 0;
  }

  uint8_t checksum = 0;
  uint8_t data = 0;
  int denominator = first ? index : PERIOD;
  for(i = 0; i < N_SENSORS; i++){
    sensor_t *sensor = SENSOR(i);
    int avg = sensor->sum / denominator;
    uint8_t tmp = (avg >= sensor->min) && (avg <= sensor->max);
    data |= tmp << i;
    checksum ^= tmp;
  }
  data |= checksum << N_SENSORS;
  int read = Serial.read();
  if(read == 0xff){}
  else if(read == 0xfe){
    Serial.write((char *)ram_eedata, sizeof(eedata_t) * N_SENSORS);
  }else if(read >= 0){
    uint8_t sensor_n = (read >> 2) & 0b11111;
    delay(10);
    uint8_t b2 = Serial.read();
    uint16_t new_adc = ((read & 0b11) << 8) | b2;
    uint8_t is_max = read >> 7;
    sensor_t *sensor = SENSOR(sensor_n);
    if(is_max) sensor->max = ram_eedata[sensor_n].max = new_adc;
    else sensor->min = ram_eedata[sensor_n].min = new_adc;
    store_sensor(sensor_n);
  }
  
  if(data != prev_data || read == 0xff){
    Serial.write(data);
    prev_data = data;
  }
}
