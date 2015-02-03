#include <stdint.h>

#define ADC_VAL(voltage) (voltage*1024/5)
#define CUTOFF ADC_VAL(0.837)

#define TOTE1_PIN A0
#define TOTE2_PIN A1
#define TOTE3_PIN A2
#define TOTE4_PIN A3
#define TOTE5_PIN A4
#define BIN_PIN A5

#define N_VARS 6
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

/*typedef struct {
  uint8_t tote1 : 1;
  uint8_t tote2 : 1;
  uint8_t tote3 : 1;
  uint8_t tote4 : 1;
  uint8_t tote5 : 1;
  uint8_t bin : 1;
  uint8_t checksum : 1;
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
  int new_idx = index + 1;
  if(new_idx == PERIOD){
    new_idx = first = 0;
  }
  int i;
  for(i = 0; i < N_VARS; i++){
    sums[i] -= values[i][index];
    int new_val = analogRead(pins[i]);
    sums[i] += new_val;
    values[i][new_idx] = new_val;
  };
  index = new_idx;
  
  /*packet->tote1 = (analogRead(TOTE1_PIN) > cutoff);
  packet->tote2 = (analogRead(TOTE2_PIN) > cutoff);
  packet->tote3 = (analogRead(TOTE3_PIN) > cutoff);
  packet->tote4 = (analogRead(TOTE4_PIN) > cutoff);
  packet->tote5 = (analogRead(TOTE5_PIN) > cutoff);
  packet->bin = (analogRead(BIN_PIN) > cutoff);*/
  uint8_t checksum = 0;
  uint8_t data = 0;
  int denominator = first ? index : cutoff;
  for(i = 0; i < N_VARS; i++){
    uint8_t tmp = (sums[i] / (float)denominator) > cutoff;
    data |= tmp;
    checksum ^= tmp;
    data <<= 1;
  }
  data |= checksum;

  if(data != prev_data || Serial.read() == 0xff){
    Serial.write(data);
    prev_data = data;
  }
}
