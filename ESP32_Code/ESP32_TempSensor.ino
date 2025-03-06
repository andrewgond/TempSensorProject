#include <M2M_LM75A.h>

M2M_LM75A Temp_Sensor;


int LED_BUILTIN = 2;
double temp = 0; 
double temps[10];
int Buzzer_Pin = 10;
int i = 0;

void setup() {
  //while (!Serial); // Leonardo: wait for serial monitor  
  Serial.begin(115200);
  Serial.println("");
  Temp_Sensor.begin();
  temp = Temp_Sensor.getTemperature();
  for(int c = 0 ; c < 10 ; c++){
    temps[c]=temp;
  }
}


void loop() {
  Serial.print(Temp_Sensor.getTemperature());
  Serial.println(F(" *C"));
  Serial.println(F(""));  
  Serial.println(F("==========================================="));
  Serial.println(F(""));

  temp = Temp_Sensor.getTemperature(); 
  temps[i] = temp;
  if (i == 9){
    i = 0;
  }
  else {
    i++;
  }
  delay(100);

  if ( check(temps, 10)){

    for (int b = 0 ; b < 10 ; b++){
      Serial.print(temps[b]);
      Serial.print((","));
      temp =  Temp_Sensor.getTemperature();
      temps[b] = temp;
 
    }
    delay(5000);
  }
  else {

  }

}

bool check(double* arr, int size){
 int temp_max = arr[0];
 int temp_min = arr[0];
 for (int a = 0 ; a < size ; a++){
  if (arr[a] > temp_max) temp_max=arr[a];
    else if (arr[a] < temp_min) temp_min=arr[a];
  }
  return((temp_max-temp_min) > 5);

}
