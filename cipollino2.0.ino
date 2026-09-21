#include <Servo.h>
#include <RTClib.h>

Servo myServo;
RTC_DS3231 rtc;

// Sensori Hall (con INPUT_PULLUP: HIGH = libero, LOW = magnete rilevato)
const int hallPosition0 = 4; // Posizione di stop finale
const int hallPosition1 = 2; // Posizione intermedia di controllo

String ORE_PASTO[] = { "2202", "2204", "2206", "2208", "2159", "2151"}; 

// Variabili per il controllo del Servo 360°

const int VELOCITA_AVANTI_PIANO = 105;  
const int VELOCITA_INDIETRO_PIANO = 80;
const int VELOCITA_FERMO = 87;
const int ATTESA_MILLIS = 3000;
const int TEMPO_INDIETRO = 750 ; 
const long ONE_MIN_MILLIS = 60000; 
const int ONE_SEC_MILLIS = 1000 ; 

void setup() {
  Serial.begin(9600);
  
  myServo.attach(9);
  ferma(); // Assicurati che sia fermo all'avvio

  if (!rtc.begin()) {
    Serial.println("Errore: RTC non trovato!");
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(hallPosition0, INPUT_PULLUP);
  pinMode(hallPosition1, INPUT_PULLUP);
}

void loop() {
  String oraAttuale = get_time_rtc();
  Serial.print("Ora attuale: ");
  Serial.println(oraAttuale);

  if (isOraPasto()) {
    Serial.println("--- ORA DEL PASTO INIZIATA ---");
    
    // 1. Inizia a girare in avanti
    giraAvanti();
    unsigned long t0 = millis();
    
    // 2. Continua a girare FINCHÉ NON rileva il sensore 1 (cioè finché resta HIGH)
    while (digitalRead(hallPosition1) == HIGH) {
      // Se sono passati più di ATTESA_MILLIS/1000 secondi senza vedere il sensore 1
      if ((millis() - t0) > ATTESA_MILLIS) {
        Serial.println("ATTENZIONE: Bloccato! Tento sblocco all'indietro...");
        giraIndietro();
        delay(TEMPO_INDIETRO); // Fa mezzo giro indietro (regola il tempo se serve)
        
        // Ripristina la marcia avanti e resetta il timer t0
        giraAvanti();
        t0 = millis(); 
      }
      delay(20); // Piccolo delay per non sovraccaricare la CPU
    }
    
    Serial.println("Sensore 1 Superato con successo!");

    // 3. Continua a girare in avanti FINCHÉ NON rileva il sensore 0 di stop
    while (digitalRead(hallPosition0) == HIGH) {
      delay(20); 
    }
    
    // 4. Fine ciclo erogazione
    ferma();
    Serial.println("Posizione 0 raggiunta. Erogazione completata.");
    
    // Evita che la stessa ora riattivi il loop immediatamente nel prossimo minuto
    delay(ONE_MIN_MILLIS); 
  }
  
  delay(ONE_SEC_MILLIS);
}

// FUNZIONI DI MOVIMENTO (Tarate per Servo a rotazione continua)
void giraAvanti() {
  myServo.write(VELOCITA_AVANTI_PIANO);
}

void giraIndietro() {
  myServo.write(VELOCITA_INDIETRO_PIANO);
}

void ferma() {
  myServo.write(VELOCITA_FERMO);
}

// VERIFICA ORARIO
bool isOraPasto() {
  String now = get_time_rtc();
  for (String t : ORE_PASTO) {
    if (t == now) return true;
  }
  return false;
}

// LETTURA RTC
String get_time_rtc() {
  char timeBuffer[] = "hhmm";
  return rtc.now().toString(timeBuffer);
}