#include <stdint.h>

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

void init_sensor(int n, int pin, uint16_t min, uint16_t max){
  sensor_t *sensor = SENSOR(n);
  sensor->min = min;
  sensor->max = max;
  sensor->sum = 0;
  sensor->pin = pin;
  int i;
  for(i = 0; i < PERIOD; i++){
    sensor->values[i] = 0;
  }
}

void eeprom_read(uint8_t *buf, int addr, int n){
  int i;
  for(i = 0; i < n; i++){
    buf[i] = EEPROM.read(addr + i);
  }
}

void eeprom_write(uint8_t *buf, int addr, int n){
  int i;
  for(i = 0; i < n; i++){
    EEPROM.write(addr + i, buf[i]);
  }
}

void load_sensors(){
  int i;
  eedata_t tmp;
  for(i = 0; i < N_SENSORS; i++){
    eeprom_read((uint8_t *)(&tmp), i * sizeof(eedata_t), sizeof(eedata_t));
    sensors[i].min = tmp.min;
    sensors[i].max = tmp.max;
  }
}

void save_sensor(sensor_t *sensor, int n){
  eedata_t tmp;
  tmp.min = sensor->min;
  tmp.max = sensor->max;
  eeprom_write((uint8_t *)sensor, n * sizeof(eedata_t), sizeof(eedata_t));
}

void init_tote(int n, int pin){
  init_sensor(n, pin, CUTOFF, 1023);
}

int index = 0;
uint8_t first = 1;

uint8_t prev_data = 0xff;
void setup(){
  Serial.begin(115200);
  init_tote(0, A0);
  init_tote(1, A1);
  init_tote(2, A2);
  init_tote(3, A3);
  init_tote(4, A4);
  init_sensor(5, A5, BIN_MIN, BIN_MAX);
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
  if(read != 0xff && read >= 0){
    uint8_t sensor_n = (read >> 2) & 0b11111;
    uint16_t new_adc = ((read & 0b11) << 8) | Serial.read();
    uint8_t is_max = read & 0b10000000;
    sensor_t *sensor = SENSOR(sensor_n);
    if(is_max) sensor->max = new_adc;
    else sensor->min = new_adc;
    save_sensor(sensor, sensor_n);
  }

  if(data != prev_data || read == 0xff){
    Serial.write(data);
    prev_data = data;
  }
}
