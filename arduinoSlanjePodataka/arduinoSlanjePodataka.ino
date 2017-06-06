void setup() {
  
  Serial.begin(9600);

}

void loop() {

//String data;
//String data = "Vrijednost livog senzora: " ;
//  data = data + analogRead(2);
//  data = data + "/";
  //Serial.write(data);
  //Serial.write("\0");

//for (int i = 0; i< data.length(); i++)
//{
//  Serial.write(data[i]);
//}

//  Serial.flush();

  Serial.print("Vrijednost livog senzora: ");
  Serial.println( analogRead(1));

delay(500);
}
