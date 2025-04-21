/** Ukljucivanje potrebnih biblioteka za rad s komponentama i funkcionalnostima */
#include <Keypad.h> /** Biblioteka za rad s matricama tipkovnica */
#include <Wire.h> /** Biblioteka za I2C komunikaciju */
#include <LiquidCrystal_I2C.h> /** Biblioteka za rad s LCD zaslonom preko I2C sučelja */
#include <avr/sleep.h> /** Biblioteka za upravljanje režimima spavanja mikrokontrolera */
#include <avr/power.h> /** Biblioteka za upravljanje potrošnjom energije mikrokontrolera */
#include <avr/interrupt.h> /** Biblioteka za upravljanje prekidima */
/**
 * @file
 * @brief Projekt - Mini alarmni sustav
 * @author Mario Srajbek
 * @date 30.4.2025.
 */

/** Postavke LCD zaslona */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/** Postavke za PIR senzor i tipkovnicu */
int senzorpokretaPin = 10;
bool detektiranakretnja = false;

const byte ROW_NUM = 4; /** 4 redka */
const byte COLUMN_NUM = 4; /** 4 stupca */

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7, 6};  /** Pinovi za redke */
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};  /** Pinovi za stupce */

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

/** Postavke alarma i simulacije */
int analogPinSenzorkvalitetezraka = A0; /** Pin za plinski senzor */
int zvucnikPIN = 12; /** Pin za zvucnik */
bool alarmAktivan = false;  /** Status alarma za PIR senzor */
bool alarmAktivan1 = false;  /** Status alarma za plinski senzor */
bool UnesenPin = false;  /** Status da li je PIN uspješno unesen */
bool odabranaOpcija = false;  /** Status odabrane opcije */
bool simulacijaAktivna = false; /** Status simulacije */
bool omogucenaSimulacija = false; /** Status za ručno pokretanje simulacije */
bool upotrebaPIRSenzora = false;  /** Koristi PIR senzor */
bool upotrebaPlinSenzora = false;  /** Koristi senzor plina */
unsigned long vrijemePokreta = 0;  /** Vrijeme kada je detektiran pokret */
bool unesiPIN = false; /** Za unos password-a nakon kad se upali alarm */
bool pogresanPIN = false; /** Status za pogresan PIN */
String pocetniPIN = "";  /** Postavljanje početnog PIN-a */
String unosPIN = "";  /** Unos korisničkog PIN-a */
static bool prikazopcija = false; /** Status prikazane opcije */
bool resetPIN = false;  /** Dodana varijabla koja kontrolira je li u procesu resetiranja PIN-a */
int pragPlina = 700;  /** Zadana granica PPM-a za alarm (može biti promijenjena od strane korisnika) */
bool probudioSe = false; /** Za ispis nakon buđenja */
static bool prikazaoResetPIN = false; /** Status prikaza za resetiranje PIN-a */

/** Funkcije za unos PIN-a, odabir opcija i resetiranje statusa */
void funkcijaUnosaPINa();
void odabirSenzora();
void resetVarijabli();
void FunkcijaobradeunesenogPINa();

/** Pocetna funkcija */
void setup() {
  lcd.begin(16, 2); /** Inicijalizacija LCD zaslona */
  lcd.print("Mini alarmni sustav"); /** Prikazivanje početne poruke */
  pinMode(senzorpokretaPin, INPUT); /** Postavka PIR senzora kao ulaz */
  pinMode(zvucnikPIN, OUTPUT);  /** Postavka zvučnika kao izlaz */
  pinMode(analogPinSenzorkvalitetezraka, INPUT);   /** AOUT kao ulaz */

  lcd.setCursor(0, 1); 
  lcd.print("Spreman za rad"); /** Prikazivanje statusa da je sustav spreman */
  delay(3000);
  lcd.clear();
}


/** Glavna petlja programa */
void loop() {
 if (resetPIN) {
  /** Ako je aktiviran proces resetiranja PIN-a */
  if (!prikazaoResetPIN) {
    lcd.clear();
    lcd.print("Unesite novi PIN:"); /** Prikazujemo poruku za unos novog PIN-a */
    prikazaoResetPIN = true; /** Označavamo da je resetiranje PIN-a prikazano */
  }

  lcd.setCursor(0, 1);
  lcd.print(unosPIN + "     ");  /** Prikazujemo uneseni PIN-a */
  funkcijaUnosaPINa(); /** Funkcija za unos novog PIN-a */

} else if (!UnesenPin) {
    /** Ako nije unesen PIN */
    lcd.setCursor(0, 0);
    lcd.print("Unesite PIN:");
    lcd.setCursor(0, 1);
    lcd.print(unosPIN);  /** Prikazuje uneseni PIN */
    funkcijaUnosaPINa(); /** Funkcija za unos PIN-a */
  } else if (!odabranaOpcija) {
    /** Ako nije odabrana opcija */
    if (!prikazopcija) {
      /**Ako nije prikazana opcija */
      lcd.clear();
      lcd.print("Odaberite opciju:");
      lcd.setCursor(0, 1);
      lcd.print("A:PIR B:Plin");
      prikazopcija = true;
    }
    odabirSenzora(); /** Funkcija za odabir senzora */

  } else if (!simulacijaAktivna && upotrebaPIRSenzora && !alarmAktivan) {
    /** Ako simulacija nije aktivna i PIR senzor nije aktiviran */
    prikaziSimulacijuPIR(); /** Pozivamo funkciju i pokrecemo simulaciju PIR senzora */
    
  } else {
    detektiranakretnja = digitalRead(senzorpokretaPin);  /** Provjeramo pokret */
    if (detektiranakretnja && !alarmAktivan && !alarmAktivan1) {
      lcd.clear();
      lcd.print("Pokret detektiran!");
      tone(zvucnikPIN, 783, 30000);  /** Uključujemo buzzer (zvuk traje 30 sekundi) */
      alarmAktivan = true;  /** Alarm za PIR senzor je sada aktivan */
      vrijemePokreta = millis();  /** Spremmamo vrijeme kada je pokret detektiran */
      unosPIN = "";  /** Resetiramo unos PIN-a kad alarm krene */
    }

    /** Ako je alarm aktiviran, traži unos PIN-a za gašenje alarma */
    if (alarmAktivan) {
      lcd.setCursor(0, 1);
      lcd.print("Unesite PIN:");
      lcd.print(unosPIN);
      FunkcijaobradeunesenogPINa(); /** Funkija za obradu unesenog PIN-a */
    } else if (upotrebaPlinSenzora) {
      /** Čitanje sa analognog pina za kvalitetu zraka (plinski senzor) */
      int analogVrijednostkvalitetezraka = analogRead(analogPinSenzorkvalitetezraka);

      /** Prikaz trenutne vrijednosti PPM */
      lcd.setCursor(0, 0);
      lcd.print("Kvaliteta zraka:");
      lcd.setCursor(0, 1);
      lcd.print(analogVrijednostkvalitetezraka);
      lcd.print(" PPM   ");

      /** Aktivacija alarma za plin ako je PPM > 700 */
      if (analogVrijednostkvalitetezraka > pragPlina && !alarmAktivan1) {
        lcd.clear();
        lcd.print("Plin detektiran!");
        tone(zvucnikPIN, 783);  /** Aktiviramo alarm (buzzer) */
        alarmAktivan1 = true;  /** Alarm za plin je sada aktivan */
      }

      /** Isključenje alarma za plin ako je PPM <= 700 */
      if (analogVrijednostkvalitetezraka <= pragPlina && alarmAktivan1) {
        noTone(zvucnikPIN);  /** Isključujemo alarm */
        lcd.clear();
        lcd.print("Plin stabilan.");
        delay(2000);  /** Pričekamo prije ponovnog očitavanja */
        alarmAktivan1 = false;  /** Alarm za plin je sada isključen */
      }
    }
  }
   /** Provjera za tipku C - resetiranje PIN-a */
  char key = keypad.getKey();
  
  if (key == 'C') {
    lcd.clear();
    lcd.print("Resetiranje PIN-a");
    delay(1000);
    
    /** Resetiranje početnog PIN-a */
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Unesite novi PIN:");
    unosPIN = "";  /** Prazni unos PIN-a kako bi korisnik mogao unijeti novi */
    lcd.setCursor(0,1);
    lcd.print(unosPIN);
    resetPIN = true;  /** Označava da je proces resetiranja PIN-a aktivan */
    delay(1000);
  
      lcd.clear();
      lcd.print("Odaberite opciju:");
      lcd.setCursor(0, 1);
      lcd.print("A:PIR B:Plin");
      prikazopcija = true;
    }
    odabirSenzora(); /** Pozivanje funkcije za odabir senzora */

  if (key == 'D' && upotrebaPlinSenzora) {  /** Provjeravamo da li tipka 'D' pritisnuto nakon odabira opcije B (Plin) */
    handleGasThresholdChange();  /** Pozivamo funkciju za promjenu granice PPM-a */
  }

 if (probudioSe) {
  lcd.clear();
  lcd.print("Probudio se :)"); /** Prikazivanje poruke nakon buđenja */
  delay(1000);

  /** Resetiranje varijabli kako bi korisnik vidio ponovno meni */
  probudioSe = false;
  odabranaOpcija = false;
  prikazopcija = false;
  lcd.clear();
}

}

/** Funkcija unosa PIN-a */
void funkcijaUnosaPINa() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      UnesenPin = true;
      pocetniPIN = unosPIN;  /** Spremamo uneseni PIN */
      lcd.clear();
      lcd.print("PIN pohranjen");
      delay(1000);
      lcd.clear();
      resetPIN = false;  /** Izvršavamo resetiranje PIN-a */
      if (!resetPIN) {
    /** Ako resetiranje nije aktivno, postavljamo sljedeće varijable */
      idiNaSleep4Sekunde();  /** Idemo u sleep nakon unosa PIN-a */
      UnesenPin = true;  /** Postavljamo UnesenPin na true */
      odabranaOpcija = false;  /**Postavljamo odabranu opciju na false, čekanje odabira opcije */
      prikazopcija = false;  /** Resetiramo opcije prikaza */
      }
      
      
    } else if (key == '*') {
      /**Ako pritisnemo tipku * */
      unosPIN = ""; /** Resitiramo unos PIN-a */
      lcd.clear();
      lcd.print("Unos resetiran");
      delay(1000);
    } else {
      unosPIN += key;
    }
  }
}

/** Funkcija obrade unesenog PIN-a gdje usporedujemo */
void FunkcijaobradeunesenogPINa() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      /** Kada korisnik pritisne #, PIN se pohranjuje i omogućava simulaciju */
      if (unosPIN == pocetniPIN) {
        lcd.clear();
        lcd.print("Alarm iskljucen!");
        noTone(zvucnikPIN);  /** Isključujemo buzzer */
        alarmAktivan = false;  /** Alarm više nije aktivan */
        unosPIN = "";  /** Resetiramo unos PIN-a */
        unesiPIN = false;  /** Resetiramo status da nije unesen PIN */
        delay(1000);  /** Prikazujemo poruku 1 sekundu */
        lcd.clear();

      delay(3000);  /** Čekaj da se PIR smiri */
      while (digitalRead(senzorpokretaPin) == HIGH) {
      delay(100);
      }
      resetirajSimulaciju(); /** Funkcija za reset simulacije */

      if (upotrebaPIRSenzora) {
      prikaziSimulacijuPIR();  /** Funkcija za simulaciju PIR senzora */
        }


      } else {
        /** Ako je PIN pogrešan */
        lcd.clear();
        lcd.print("Pogresan PIN");
        pogresanPIN = true;  /** Označujemo da je uneseni PIN pogrešan */
        unosPIN = "";  /** Resetiramo unos PIN-a */
        delay(1000);  /** Prikazujemo poruku 1 sekundu */
        lcd.clear();  /** Brišemo zaslon */
        lcd.print("Pogresan PIN");  /** Ponovno prikazujemo poruku "Pogresan PIN" */
      }
   }else if (key == '*') {
      /** Ako je pritisnuta tipka *, resetiraj unos PIN-a */
      unosPIN = ""; /** resitiramo unosPIN-a */
      lcd.clear();
      lcd.print("Unos resetiran");
      delay(1000);  /** Prikazujemo poruku 1 sekundu */
      lcd.clear();
      lcd.print("Unesite PIN:");  /** Ponovno prikazujemo poruku */
    } else {
      /** Ako je pritisnuta bilo koja druga tipka, dodaj je u unos PIN-a */
      unosPIN += key;
      lcd.clear();  /** Očisti zaslon */
      lcd.setCursor(0, 1);
      lcd.print("PIN: ");  /** Prikazujemo poruku za unos PIN-a */
      lcd.print(unosPIN); /** Prikazujemo unesene brojke */
    
    }
    
  }
}

/** Funkcija koja prikazuje poruku da je simulacija aktivna, traži od korisnika da pokrene kretanje kako bi simulacija mogla poceti */
void prikaziSimulacijuPIR() {
  lcd.clear();
  lcd.print("Simulacija aktivna");
  lcd.setCursor(0, 1);
  lcd.print("Pokreni kretanje");
  omogucenaSimulacija = true;
  simulacijaAktivna = true;
}

/** Funkcija koja se koristi za resetiranje statusa nakon što simulacija završi ili je isključena */
void resetirajSimulaciju() {
  simulacijaAktivna = false; /** Iskljucujemo status aktivne simulacije */
  omogucenaSimulacija = false; /** Isključujemo omogućenu simulaciju */
}


/** Funkcija za odabir senzora */
void odabirSenzora() {
  char key = keypad.getKey();
  if (key == 'A') {
    /** Ako je pritisnuta tipka A */
    upotrebaPIRSenzora = true;
    odabranaOpcija = true;
    lcd.clear();
    lcd.print("Odabran PIR senzor");
    delay(1000);
  } else if (key == 'B') {
    /** Ako je pritisnuta tipka B */
    upotrebaPlinSenzora = true;
    lcd.clear();
    lcd.print("Odabran plinski senzor");
    delay(1000);
    odabranaOpcija = true;
  }

}

/** Funkcija koja isključuje status aktivnog alarma i deaktivira aktivnu simulaciju te omogućuje ponovno pokretanje simulacije detekcije pokreta   */
void resetVarijabli() {
  alarmAktivan = false; /** Isključuje aktivni alarm */
  simulacijaAktivna = false; /** Iskljucuje aktivnu simulaciju */
  omogucenaSimulacija = true;  /** Ponovno omogućuje simulaciju pokreta */
}

/** Funkcija za unos granice PPM-a */
void handleGasThresholdChange() {
  lcd.clear();
  lcd.print("Unesite novu granicu:");
  lcd.setCursor(0, 1);
  String thresholdInput = "";
  char key = keypad.getKey();
  while (key != '#') {  /** Korisnik unosi broj dok ne pritisne '#' */
    key = keypad.getKey();
    if (key) {
      thresholdInput += key;  /** Dodaj broj u unos */
      lcd.clear();
      lcd.print("Nova granica:");
      lcd.setCursor(0, 1);
      lcd.print(thresholdInput);  /** Prikaz unesenog broja */
    }
  }
  
  /** Pretvoramo unos u integer i pohranjujemo u pragPlina */
  pragPlina = thresholdInput.toInt();
  lcd.clear();
  lcd.print("Granica postavljena:");
  lcd.setCursor(0, 1);
  lcd.print(pragPlina);
  delay(1000); 
  lcd.clear();
  lcd.print("Odaberite opciju:");
  lcd.setCursor(0, 1);
  lcd.print("A:PIR B:Plin");
}

void idiNaSleep4Sekunde() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); /** sleep mode */
  sleep_enable();

  /** Podešavanje Timer2 za buđenje nakon 4 sekunde */
  noInterrupts();           /** Isključi prekide dok podešavamo */

  TCCR2A = 0;               /** Normalni mode */
  TCCR2B = 0b00000111;      /** omogucujemo usporavanje brojenja timera */
  TCNT2  = 0;               /** Resetiramo brojac */
  OCR2A  = 255;             /** Postavljamo vrijednost za izlazak iz sleep-a */

  TIMSK2 |= (1 << TOIE2);   /** Omogućujemo prekid */

  interrupts();             /** Ponovno ukljucujemo prekide */

  sleep_cpu();              /** Spavamo */

  sleep_disable();          /** Nakon budenja, onemogucujemo sleep */
  probudioSe = true;        /** Oznacujemo da se Arduino probudio */
}

/** Interrupt rutina koja se poziva pri buđenju */
ISR(TIMER2_OVF_vect) {
  // Prekid koji samo služi da prekinemo sleep
}

