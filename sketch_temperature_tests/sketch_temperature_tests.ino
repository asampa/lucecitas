#include <Arduino_HTS221.h>

void setup() {
  // put your setup code here, to run once:

  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }
  // If you want to exit then HTS.end();
}

void loop() {
  // put your main code here, to run repeatedly:

  float temperature = HTS.readTemperature();

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" °C");
  delay(1000);
}
