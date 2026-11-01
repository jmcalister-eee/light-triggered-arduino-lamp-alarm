// Ohm resistance = float | ADC value = int (0-1023) | Time = unsigned long

const int LDR_PIN = A0;

//const int RELAY_PIN = 7;
const int RELAY_PIN = LED_BUILTIN; //change after testing

const unsigned long R1_OHMS = 150000UL; // UL = Unsigned Long Type
const float LDR_OHMS_SCREEN_ON = 190000.0;
const float LDR_OHMS_SCREEN_OFF = 360000.0;

const float ADC_FULL_SCALE = 1024.0;
int screenOnADC = 1023;  
int screenOffADC = 0;  
int baselineADC = 0;

bool lampIsOn = false;
unsigned long lampOnTime = 0;  //millis() returns an unsigned long, so these variables need to be the same type

bool phoneScreenIsOn = false;
bool phoneScreenWasOn = false;
unsigned long phoneScreenOnSince = 0;
unsigned long lastDebugPrintTime = 0;

const unsigned long LAMP_DURATION_MS = 60UL * 1000UL; //add after testing: 15m * 60s * 1000ms
const unsigned long PHONE_SCREEN_DURATION_MS = 10UL * 1000UL;
const unsigned long LOOP_PRINT_INTERVAL_MS = 500UL;
const int temporaryLoopIncrement = 0;


//Convert V_AO in Ohms to an ADC Value
int LDROhmsToADC (float assumedOhms){
  float predictedADC = (ADC_FULL_SCALE * R1_OHMS) / (R1_OHMS + assumedOhms);
  return constrain((int)(predictedADC + 0.5), 0, 1023);
}

//R_LDR in terms of ADC
float ADCToLDROhms (int ADCReading_VA0) {
  if (ADCReading_VA0 <= 0){
    return 1000000000.0;
  }
  return R1_OHMS * ( (ADC_FULL_SCALE - ADCReading_VA0) / ADCReading_VA0 );
}

//int R1_ADC = LDROhmsToADC(R1_OHMS);


void setup() {
  pinMode(RELAY_PIN, OUTPUT); //Tells arduino RELAY_PIN will SEND a signal out
  digitalWrite(RELAY_PIN, LOW);  //Sends 0V to the Relay, keeps the lamp OFF
  
  Serial.begin(9600);

  analogRead(LDR_PIN);  //throwaway read
  delay(10); //in ms
  baselineADC = analogRead(LDR_PIN);

  Serial.print("Sanity check, R1_ADC value (expect ~512): ");
  Serial.println(LDROhmsToADC(R1_OHMS));

  Serial.print("ADC Value for the screen to switch on (expect ~452): ");
  screenOnADC = LDROhmsToADC(LDR_OHMS_SCREEN_ON);
  Serial.println(screenOnADC);

  Serial.print("ADC Value for the screen to switch off (expect ~301): ");
  screenOffADC = LDROhmsToADC(LDR_OHMS_SCREEN_OFF);
  Serial.println(screenOffADC); 

  //information blob
  Serial.print("Pins - LDR (A0=14): "); Serial.println(LDR_PIN);
  Serial.print("Pins - Relay: "); Serial.println(RELAY_PIN);
  Serial.print("baseline ADC: "); Serial.println(baselineADC);
  Serial.print("Lamp ON above ADC value: "); Serial.println(screenOnADC);
  Serial.print("Lamp OFF below ADC value: "); Serial.println(screenOffADC);


  if (baselineADC >= screenOffADC){
    Serial.println("Warning: baselineADC is already above the screenOff Threshold. Please lower light level.");
  }

}

void loop() {
  //Take intial sensor read:
  analogRead(LDR_PIN); //throwaway read
  delay(10);

  int ADCReading_VA0 = analogRead(LDR_PIN);
  //save time here
  unsigned long currentTime = millis();


  //update screen state
  phoneScreenWasOn = phoneScreenIsOn;

  if (screenOffADC > ADCReading_VA0) {
    phoneScreenIsOn = false;
  } else if (screenOnADC <= ADCReading_VA0) {
    phoneScreenIsOn = true;
    }

  if ((phoneScreenWasOn == false) && (phoneScreenIsOn == true)) {
    phoneScreenOnSince = currentTime;
  }

  //lamp OFF --> ON
  if ((lampIsOn == false) && (phoneScreenIsOn == true) && (currentTime - phoneScreenOnSince) > PHONE_SCREEN_DURATION_MS) {
    digitalWrite(RELAY_PIN, HIGH);
    lampOnTime = currentTime;
    lampIsOn = true;
  }

  // Lamp ON --> OFF, reset phoneScreenOnSince, toggle phoneScreenIsOn
  if ((lampIsOn == true) && (phoneScreenIsOn == false) && ((currentTime - lampOnTime) >= LAMP_DURATION_MS)) {
    digitalWrite(RELAY_PIN, LOW);
    lampIsOn = false;
  }

  //debug info
  if ((currentTime - lastDebugPrintTime) > LOOP_PRINT_INTERVAL_MS) {
    Serial.print("Current Time (ms): ");
    Serial.println(currentTime);
    float LDROhms = ADCToLDROhms(ADCReading_VA0);
    Serial.print("ADCReading_VA0 in ADC: ");
    Serial.println(ADCReading_VA0);
    Serial.print("ADCReading_VA0 in Ohms (Should be LDR Ohms Value): ");
    Serial.println(LDROhms);
  }


}


// To Do: 
//Light check and Sound check simultaneously, Light check for 10s, Sound check for 1s