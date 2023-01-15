// buzzer.h
const int buzzerPin = 8; // pin for the buzzer
void Buzzer() {
  tone(buzzerPin, 2000); // play a tone at 2000 Hz
  delay(1000); // wait for 1 second
  noTone(buzzerPin); // stop the tone
  delay(1000); // wait for 1 second
}