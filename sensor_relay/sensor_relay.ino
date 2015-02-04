#include <stdint.h>

#define ADC_VAL(voltage) (voltage*1024/5)
#define CUTOFF ADC_VAL(0.837)
#define BIN_MIN ADC_VAL(0.348)
#define BIN_MAX ADC_VAL(0.629)

#define N_VARS 6
#define PERIOD 10
#define N_BIN 5
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

/*typedef struct {
  uint8_t checksum : 1;
  uint8_t bin : 1;
  uint8_t tote5 : 1;
  uint8_t tote4 : 1;
  uint8_t tote3 : 1;
  uint8_t tote2 : 1;
  uint8_t tote1 : 1;
  uint8_t padding : 1;
} packet_t __attribute__ ((packed));*/

uint16_t values[N_VARS][PERIOD] = {0};
int sums[N_VARS] = {0};
uint8_t pins[N_VARS] = {A0, A1, A2, A3, A4, A5};
int index = 0;
uint8_t first = 1;
/*typedef union {
  packet_t packet;
  uint8_t data;
} pdata_t;*/

//pdata_t pdata;
//packet_t *packet;

uint8_t prev_data = 0xff;
int cutoff = (int)CUTOFF;
void setup(){
  Serial.begin(115200);
}

void loop(){
//  Serial.println((float)analogRead(TOTE2_PIN) / 1024.0 * 5);
  //return;

  int i;
  for(i = 0; i < N_VARS; i++){
    sums[i] -= values[i][index];
    int new_val = analogRead(pins[i]);
    sums[i] += new_val;
    values[i][index] = new_val;
  };
  index++;
  if(index == PERIOD){
    index = first = 0;
  }
  
  /*packet->tote1 = (analogRead(TOTE1_PIN) > cutoff);
  packet->tote2 = (analogRead(TOTE2_PIN) > cutoff);
  packet->tote3 = (analogRead(TOTE3_PIN) > cutoff);
  packet->tote4 = (analogRead(TOTE4_PIN) > cutoff);
  packet->tote5 = (analogRead(TOTE5_PIN) > cutoff);
  packet->bin = (analogRead(BIN_PIN) > cutoff);*/
  uint8_t checksum = 0;
  uint8_t data = 0;
  int denominator = first ? index : PERIOD;
  for(i = 0; i < N_VARS; i++){
    int avg = sums[i] / denominator;
    uint8_t tmp = (i == N_BIN) ? (avg > BIN_MIN && avg < BIN_MAX) : avg > cutoff;
    data |= tmp;
    checksum ^= tmp;
    data <<= 1;
  }
  data |= checksum;
  int read = Serial.read();
  if((read & 0b00000011) == 0){
    uint16_t hb = (read & 0b11) << 8;
    cutoff = hb | Serial.read();
  }

  if(data != prev_data || read == 0xff){
    Serial.write(data);
    prev_data = data;
  }
}
