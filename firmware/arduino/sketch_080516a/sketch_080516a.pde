void setup(){
  Serial.begin(9600);
}

void loop(){
  byte val;
  if(Serial.available()){
    val = Serial.read();
    Serial.print("Read: ");
    Serial.println(val);
  }
}
