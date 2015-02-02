#include <stdint.h>

#define ADC_VAL(voltage) (voltage*1024/5)
#define CUTOFF ADC_VAL(0.837)

#define TOTE1_PIN A0
#define TOTE2_PIN A1
#define TOTE3_PIN A2
#define TOTE4_PIN A3
#define TOTE5_PIN A4
#define BIN_PIN A5

typedef struct {
  uint8_t tote1 : 1;
  uint8_t tote2 : 1;
  uint8_t tote3 : 1;
  uint8_t tote4 : 1;
  uint8_t tote5 : 1;
  uint8_t bin : 1;
  uint8_t checksum : 1;
  uint8_t padding : 1;
} packet_t __attribute__ ((packed));

typedef union {
  packet_t packet;
  uint8_t data;
} pdata_t;

pdata_t pdata;
packet_t *packet;
uint8_t prev_data = 0xff;
int cutoff = (int)CUTOFF;
void setup(){
  Serial.begin(115200);
  packet = &pdata.packet;
}

void loop(){
//  Serial.println((float)analogRead(TOTE2_PIN) / 1024.0 * 5);
  //return;
  packet->tote1 = (analogRead(TOTE1_PIN) > cutoff);
  packet->tote2 = (analogRead(TOTE2_PIN) > cutoff);
  packet->tote3 = (analogRead(TOTE3_PIN) > cutoff);
  packet->tote4 = (analogRead(TOTE4_PIN) > cutoff);
  packet->tote5 = (analogRead(TOTE5_PIN) > cutoff);
  packet->bin = (analogRead(BIN_PIN) > cutoff);
  packet->checksum = packet->tote1 ^ packet->tote2 ^ packet->tote3 ^ packet->tote4 ^ packet->tote5 ^ packet->bin;
  uint8_t data = pdata.data;
  if(data != prev_data || Serial.read() == 0xff){
    Serial.write(data);
    prev_data = data;
  }
}
