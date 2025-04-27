# Mini alarmni sustav (Arduino + Wokwi)

Ovaj mini alarmni sustav koristi Arduino platformu za upravljanje senzorima i omogućuje korisnicima da podešavaju sigurnosne parametre poput PIN-a i granice PPM za plinski senzor.

## Funkcionalnosti
- Unos PIN-a za deaktivaciju alarma
- Odabir između PIR (pokret) i plinskog senzora
- Resetiranje PIN-a
- Postavljanje granice PPM-a za plinski senzor
- Automatska detekcija pokreta ili plina i aktivacija alarma

## Dokumentacija  

GitHub Wiki - https://github.com/msrajbek-maker/RUS--Mini-alarmni-sustav/wiki  
Doxygen - https://msrajbek-maker.github.io/RUS--Mini-alarmni-sustav/


## Simulacija (Wokwi)
Projekt je napravljen i testiran u [Wokwi simulatoru](https://wokwi.com/) – Arduino online alat za razvoj i simulaciju.
1. Otvori projekt u Wokwi simulatoru (https://wokwi.com/projects/428416281736408065)
2. Pokreni simulaciju
3. Koristi tipkovnicu za upravljanje alarmnim sustavom


## Upute za korištenje

| Tipka  | Funkcija                          |
|--------|-----------------------------------|
| `A`    | Aktivacija PIR senzora            |
| `B`    | Aktivacija plinskog senzora       |
| `C`    | Resetiranje PIN-a                 |
| `D`    | Promjena granice PPM-a            |
| `#`    | Pohrana unosa (PIN/PPM)           |
| `*`    | Resetiranje unosa                 |

## Shema
![slikazawiki](https://github.com/user-attachments/assets/dd00760c-540e-47ca-a1d7-483467ba0626)

## Potrebne komponente
- Arduino UNO
- PIR senzor
- MQ-135 plinski senzor
- 16x2 LCD + I2C
- 4x4 tipkovnica
- Buzzer

## Testiranje i ispitivanje Mini alarmnog sustava
Testiranje i ispitivanje Mini alarmnog sustava, koje bi korisniku moglo pružiti uslugu da lakše razumije sustav i da se jednostavnije služi sa koracima rada navedenog sustava, nalazi se u Wiki dokumentaciji pod kategorijom "Opis projektnog zadatka" na samome dnu.

## Autor
Mario Šrajbek  
GitHub: msrajbek-maker(https://github.com/msrajbek-maker)
