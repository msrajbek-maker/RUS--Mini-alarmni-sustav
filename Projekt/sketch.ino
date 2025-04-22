/*!
* \file sketch.ino
* \brief Projekt - Mini alarmni sustav
* 
* Ovaj program implementira jednostavan alarmni sustav koji koristi više komponenti:
* - PIR senzor za pokret
* - Plinski MQ senzor
* - LCD zaslon
* - Zvučnik
* - 4x4 tipkovnicu
* 
* Funkcionalnosti uključuju unos PIN-a, simulaciju alarma, upravljanje potrošnjom energije i prekide.
*
* \author Mario Srajbek
* \date 30.4.2025.
*/

 #include <Keypad.h>              /**< Biblioteka za rad s matricama tipkovnica */
 #include <Wire.h>                /**< Biblioteka za I2C komunikaciju */
 #include <LiquidCrystal_I2C.h>   /**< Biblioteka za rad s LCD zaslonom preko I2C sučelja */
 #include <avr/sleep.h>           /**< Biblioteka za upravljanje režimima spavanja mikrokontrolera */
 #include <avr/power.h>           /**< Biblioteka za upravljanje potrošnjom energije mikrokontrolera */
 #include <avr/interrupt.h>       /**< Biblioteka za upravljanje prekidima */
 
 /** @brief LCD zaslon na I2C adresi 0x27, dimenzija 16x2 */
 LiquidCrystal_I2C lcd(0x27, 16, 2);
 
 /** @brief Pin za PIR senzor pokreta */
 int senzorpokretaPin = 10;
 /** @brief Status detekcije kretanja */
 bool detektiranakretnja = false;
 
 const byte ROW_NUM = 4;          /**< Broj redaka tipkovnice */
 const byte COLUMN_NUM = 4;       /**< Broj stupaca tipkovnice */
 
 /** @brief Mapa tipki za matricu tipkovnice */
 char keys[ROW_NUM][COLUMN_NUM] = {
   {'1','2','3','A'},
   {'4','5','6','B'},
   {'7','8','9','C'},
   {'*','0','#','D'}
 };
 
 /** @brief Pinovi za redke tipkovnice */
 byte pin_rows[ROW_NUM] = {9, 8, 7, 6};
 /** @brief Pinovi za stupce tipkovnice */
 byte pin_column[COLUMN_NUM] = {5, 4, 3, 2};
 
 /** @brief Objekt tipkovnice */
 Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
 
 /** @brief Pin za plinski senzor */
 int analogPinSenzorkvalitetezraka = A0;
 /** @brief Pin za zvučnik */
 int zvucnikPIN = 12;
 /** @brief Status alarma za PIR senzor */
 bool alarmAktivan = false;
 /** @brief Status alarma za plinski senzor */
 bool alarmAktivan1 = false;
 /** @brief Status unosa PIN-a */
 bool UnesenPin = false;
 /** @brief Status odabrane opcije */
 bool odabranaOpcija = false;
 /** @brief Status aktivne simulacije */
 bool simulacijaAktivna = false;
 /** @brief Status za ručno pokretanje simulacije */
 bool omogucenaSimulacija = false;
 /** @brief Aktivacija PIR senzora */
 bool upotrebaPIRSenzora = false;
 /** @brief Aktivacija plinskog senzora */
 bool upotrebaPlinSenzora = false;
 /** @brief Vrijeme detekcije pokreta */
 unsigned long vrijemePokreta = 0;
 /** @brief Status za unos PIN-a nakon alarma */
 bool unesiPIN = false;
 /** @brief Status za pogrešan PIN */
 bool pogresanPIN = false;
 /** @brief Početni PIN */
 String pocetniPIN = "";
 /** @brief Korisnički unos PIN-a */
 String unosPIN = "";
 /** @brief Status prikaza opcije */
 static bool prikazopcija = false;
 /** @brief Status procesa resetiranja PIN-a */
 bool resetPIN = false;
 /** @brief Prag vrijednosti plina za aktivaciju alarma */
 int pragPlina = 700;
 /** @brief Status buđenja sustava */
 bool probudioSe = false;
 /** @brief Status prikaza poruke za reset PIN-a */
 static bool prikazaoResetPIN = false;
 
 /** @brief Funkcija za unos PIN-a */
 void funkcijaUnosaPINa();
 /** @brief Funkcija za odabir senzora */
 void odabirSenzora();
 /** @brief Funkcija za resetiranje statusnih varijabli */
 void resetVarijabli();
 /** @brief Funkcija za obradu unesenog PIN-a */
 void FunkcijaobradeunesenogPINa();
 /** @brief Funkcija za simulaciju PIR senzora */
 void prikaziSimulacijuPIR();
 /** @brief Funkcija za resetiranje simulacije */
 void resetirajSimulaciju();
 /** @brief Funkcija za promjenu PPM granice */
 void handleGasThresholdChange();
 /** @brief Funkcija za prijelaz u sleep mod */
 void idiNaSleep4Sekunde();
 
 /**
  * @brief Inicijalna postavka uređaja
  */
 void setup() {
   lcd.begin(16, 2);
   lcd.print("Mini alarmni sustav");
   pinMode(senzorpokretaPin, INPUT);
   pinMode(zvucnikPIN, OUTPUT);
   pinMode(analogPinSenzorkvalitetezraka, INPUT);
   lcd.setCursor(0, 1);
   lcd.print("Spreman za rad");
   delay(3000);
   lcd.clear();
 }
 
/**
 * @brief Glavna petlja programa.
 * 
 * Upravljanje stanjima sustava na temelju korisničkih unosa, senzora pokreta i kvalitete zraka.
 * Omogućuje resetiranje PIN-a, unos PIN-a, odabir senzora, simulaciju alarma, te reagira na prekide.
 */
 void loop() {
  if (resetPIN) {
    /** @brief Ako je aktiviran proces resetiranja PIN-a */
    if (!prikazaoResetPIN) {
      lcd.clear();
      lcd.print("Unesite novi PIN:"); /**< Prikaz poruke za novi PIN */
      prikazaoResetPIN = true; /**< Status prikaza za resetiranje PIN-a */
    }

    lcd.setCursor(0, 1);
    lcd.print(unosPIN + "     "); /**< Prikaz unesenog PIN-a */
    funkcijaUnosaPINa(); /**< Poziv funkcije za unos PIN-a */

  } else if (!UnesenPin) {
    /** @brief Ako nije unesen PIN */
    lcd.setCursor(0, 0);
    lcd.print("Unesite PIN:");
    lcd.setCursor(0, 1);
    lcd.print(unosPIN); /**< Prikaz unesenog PIN-a */
    funkcijaUnosaPINa();

  } else if (!odabranaOpcija) {
    /** @brief Ako nije odabrana opcija */
    if (!prikazopcija) {
      /** @brief Ako još nije prikazana poruka za odabir opcije */
      lcd.clear();
      lcd.print("Odaberite opciju:");
      lcd.setCursor(0, 1);
      lcd.print("A:PIR B:Plin");
      prikazopcija = true;
    }
    odabirSenzora();

  } else if (!simulacijaAktivna && upotrebaPIRSenzora && !alarmAktivan) {
    /** @brief Ako simulacija nije aktivna i PIR senzor još nije aktiviran */
    prikaziSimulacijuPIR();

  } else {
    detektiranakretnja = digitalRead(senzorpokretaPin); /**< Očitavanje statusa PIR senzora */
    if (detektiranakretnja && !alarmAktivan && !alarmAktivan1) {
      lcd.clear();
      lcd.print("Pokret detektiran!");
      tone(zvucnikPIN, 783, 30000); /**< Uključivanje zvučnog alarma */
      alarmAktivan = true;
      vrijemePokreta = millis(); /**< Spremanje vremena detekcije */
      unosPIN = ""; /**< Resetiranje unosa PIN-a */
    }

    if (alarmAktivan) {
      /** @brief Ako je alarm aktiviran, traži unos PIN-a */
      lcd.setCursor(0, 1);
      lcd.print("Unesite PIN:");
      lcd.print(unosPIN);
      FunkcijaobradeunesenogPINa();
    } else if (upotrebaPlinSenzora) {
      /** @brief Očitavanje vrijednosti plinskog senzora */
      int analogVrijednostkvalitetezraka = analogRead(analogPinSenzorkvalitetezraka);

      lcd.setCursor(0, 0);
      lcd.print("Kvaliteta zraka:");
      lcd.setCursor(0, 1);
      lcd.print(analogVrijednostkvalitetezraka);
      lcd.print(" PPM   ");

      if (analogVrijednostkvalitetezraka > pragPlina && !alarmAktivan1) {
        /** @brief Aktivacija alarma zbog visoke vrijednosti plina */
        lcd.clear();
        lcd.print("Plin detektiran!");
        tone(zvucnikPIN, 783);
        alarmAktivan1 = true;
      }

      if (analogVrijednostkvalitetezraka <= pragPlina && alarmAktivan1) {
        /** @brief Deaktivacija alarma ako je vrijednost stabilna */
        noTone(zvucnikPIN);
        lcd.clear();
        lcd.print("Plin stabilan.");
        delay(2000);
        alarmAktivan1 = false;
      }
    }
  }

  /** @brief Provjera za reset PIN-a preko tipke 'C' */
  char key = keypad.getKey();
  if (key == 'C') {
    lcd.clear();
    lcd.print("Resetiranje PIN-a");
    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unesite novi PIN:");
    unosPIN = "";
    lcd.setCursor(0, 1);
    lcd.print(unosPIN);
    resetPIN = true;
    delay(1000);

    lcd.clear();
    lcd.print("Odaberite opciju:");
    lcd.setCursor(0, 1);
    lcd.print("A:PIR B:Plin");
    prikazopcija = true;
  }
  odabirSenzora();

  if (key == 'D' && upotrebaPlinSenzora) {
    /** @brief Promjena granice PPM-a preko tipke 'D' */
    handleGasThresholdChange();
  }

  if (probudioSe) {
    /** @brief Poruka i reset statusa nakon buđenja */
    lcd.clear();
    lcd.print("Probudio se :)");
    delay(1000);

    probudioSe = false;
    odabranaOpcija = false;
    prikazopcija = false;
    lcd.clear();
  }
}

/**
 * @brief Funkcija za unos PIN-a putem tipkovnice.
 * 
 * Omogućuje unos znamenki, resetiranje unosa pomoću '*',
 * te potvrdu i pohranu PIN-a pomoću '#' nakon čega sustav ide u sleep mod.
 */
 void funkcijaUnosaPINa() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      UnesenPin = true;
      pocetniPIN = unosPIN; /**< Spremanje unesenog PIN-a kao početnog */
      lcd.clear();
      lcd.print("PIN pohranjen");
      delay(1000);
      lcd.clear();
      resetPIN = false; /**< Deaktivacija procesa resetiranja PIN-a */

      if (!resetPIN) {
        /** @brief Ako nije u resetiranju, postavlja stanje za odabir opcije */
        idiNaSleep4Sekunde(); /**< Uspavljivanje sustava na 4 sekunde */
        UnesenPin = true;
        odabranaOpcija = false;
        prikazopcija = false;
      }

    } else if (key == '*') {
      /** @brief Resetiranje unosa PIN-a pritiskom na '*' */
      unosPIN = "";
      lcd.clear();
      lcd.print("Unos resetiran");
      delay(1000);
    } else {
      /** @brief Dodavanje znamenke u unos PIN-a */
      unosPIN += key;
    }
  }
}

/**
 * @brief Funkcija za obradu unesenog PIN-a.
 *
 * Provjerava ispravnost unesenog PIN-a i deaktivira alarm ako je PIN točan.
 * Ako je PIN netočan, prikazuje odgovarajuću poruku. Također omogućuje resetiranje unosa pomoću '*' i dodavanje znamenki.
 */
 void FunkcijaobradeunesenogPINa() {
  char key = keypad.getKey();
  if (key) {
    if (key == '#') {
      /** @brief Potvrda unosa PIN-a pritiskom na '#' */
      if (unosPIN == pocetniPIN) {
        lcd.clear();
        lcd.print("Alarm iskljucen!");
        noTone(zvucnikPIN); /**< Isključivanje zvučnog alarma */
        alarmAktivan = false;
        unosPIN = "";
        unesiPIN = false;
        delay(1000);
        lcd.clear();

        delay(3000); /**< Pauza dok se PIR senzor smiri */
        while (digitalRead(senzorpokretaPin) == HIGH) {
          delay(100);
        }
        resetirajSimulaciju();

        if (upotrebaPIRSenzora) {
          prikaziSimulacijuPIR();
        }

      } else {
        /** @brief Ako je uneseni PIN netočan */
        lcd.clear();
        lcd.print("Pogresan PIN");
        pogresanPIN = true;
        unosPIN = "";
        delay(1000);
        lcd.clear();
        lcd.print("Pogresan PIN");
      }

    } else if (key == '*') {
      /** @brief Resetiranje unosa PIN-a pritiskom na '*' */
      unosPIN = "";
      lcd.clear();
      lcd.print("Unos resetiran");
      delay(1000);
      lcd.clear();
      lcd.print("Unesite PIN:");

    } else {
      /** @brief Dodavanje znamenke u unos PIN-a */
      unosPIN += key;
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("PIN: ");
      lcd.print(unosPIN);
    }
  }
}

/**
 * @brief Prikazuje poruku da je simulacija PIR senzora aktivna.
 *
 * Funkcija postavlja zastavice za aktivaciju simulacije i traži od korisnika da pokrene kretanje
 * kako bi se mogla detektirati aktivnost putem PIR senzora.
 */
 void prikaziSimulacijuPIR() {
  lcd.clear();
  lcd.print("Simulacija aktivna");
  lcd.setCursor(0, 1);
  lcd.print("Pokreni kretanje");
  omogucenaSimulacija = true; /**< Omogućavanje ručnog pokretanja simulacije */
  simulacijaAktivna = true;   /**< Označavanje da je simulacija aktivna */
}

/**
 * @brief Resetira statusne varijable nakon završetka simulacije.
 *
 * Funkcija se koristi kada simulacija PIR senzora završi ili je ručno isključena,
 * čime se deaktivira aktivna i omogućena simulacija.
 */
 void resetirajSimulaciju() {
  simulacijaAktivna = false;      /**< Isključivanje aktivne simulacije */
  omogucenaSimulacija = false;    /**< Isključivanje mogućnosti pokretanja simulacije */
}

/**
 * @brief Funkcija za odabir senzora putem tipkovnice.
 *
 * Korisnik bira između PIR senzora (tipka 'A') i plinskog senzora (tipka 'B').
 * Ovisno o izboru, postavljaju se odgovarajuće zastavice i prikazuje poruka na LCD zaslonu.
 */
 void odabirSenzora() {
  char key = keypad.getKey();
  if (key == 'A') {
    /** @brief Ako je odabran PIR senzor pritiskom na 'A' */
    upotrebaPIRSenzora = true;
    odabranaOpcija = true;
    lcd.clear();
    lcd.print("Odabran PIR senzor");
    delay(1000);

  } else if (key == 'B') {
    /** @brief Ako je odabran plinski senzor pritiskom na 'B' */
    upotrebaPlinSenzora = true;
    lcd.clear();
    lcd.print("Odabran plinski senzor");
    delay(1000);
    odabranaOpcija = true;
  }
}

/**
 * @brief Resetira varijable vezane uz alarm i simulaciju.
 *
 * Funkcija isključuje trenutno aktivirani alarm i aktivnu simulaciju,
 * te omogućuje korisniku ponovno pokretanje simulacije detekcije pokreta.
 */
 void resetVarijabli() {
  alarmAktivan = false;           /**< Deaktivacija PIR alarma */
  simulacijaAktivna = false;      /**< Deaktivacija statusa aktivne simulacije */
  omogucenaSimulacija = true;     /**< Ponovno omogućavanje simulacije */
}

/**
 * @brief Funkcija za unos nove granice PPM-a plinskog senzora.
 *
 * Korisnik unosi novu granicu PPM-a pomoću tipkovnice.
 * Unos se potvrđuje pritiskom na '#', nakon čega se nova vrijednost pohranjuje
 * i prikazuje na LCD zaslonu.
 */
 void handleGasThresholdChange() {
  lcd.clear();
  lcd.print("Unesite novu granicu:");
  lcd.setCursor(0, 1);
  String thresholdInput = "";
  char key = keypad.getKey();

  while (key != '#') {
    /** @brief Unos brojeva dok se ne pritisne '#' */
    key = keypad.getKey();
    if (key) {
      thresholdInput += key; /**< Dodavanje brojke u unos */
      lcd.clear();
      lcd.print("Nova granica:");
      lcd.setCursor(0, 1);
      lcd.print(thresholdInput); /**< Prikaz trenutnog unosa */
    }
  }

  pragPlina = thresholdInput.toInt(); /**< Pohrana nove granice kao integer */
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

/**
 * @brief Uspavljuje mikrokontroler na otprilike 4 sekunde koristeći Timer2.
 *
 * Postavlja mikrokontroler u sleep mod "power down" i koristi Timer2 za buđenje nakon 4 sekunde.
 * Nakon buđenja, omogućuje nastavak rada sustava i označava buđenje zastavicom.
 */
 void idiNaSleep4Sekunde() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); /**< Postavljanje sleep moda */
  sleep_enable();

  /** @brief Podešavanje Timer2 za buđenje */
  noInterrupts(); /**< Onemogućavanje prekida pri konfiguraciji */

  TCCR2A = 0;               /**< Normalni rad Timer2 */
  TCCR2B = 0b00000111;      /**< Prescaler za usporavanje brojenja */
  TCNT2  = 0;               /**< Resetiranje brojača */
  OCR2A  = 255;             /**< Izlazna vrijednost za buđenje */

  TIMSK2 |= (1 << TOIE2);   /**< Omogućavanje prekida Timer2 overflow */

  interrupts();             /**< Ponovno omogućavanje prekida */

  sleep_cpu();              /**< Ulazak u sleep */

  sleep_disable();          /**< Onemogućavanje sleep moda nakon buđenja */
  probudioSe = true;        /**< Zastavica da se sustav probudio */
}

/**
 * @brief Interrupt rutina koja se koristi za buđenje iz sleep moda.
 *
 * Prekid se aktivira kada Timer2 overflowa. Ne izvodi dodatne operacije, već samo služi za buđenje.
 */
ISR(TIMER2_OVF_vect) {
  // Prazan ISR samo za prekidanje sleep-a
}
