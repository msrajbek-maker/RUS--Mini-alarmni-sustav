# Mini alarmni sustav (Arduino + Wokwi)

Ovaj mini alarmni sustav koristi Arduino platformu za upravljanje senzorima i omogućuje korisnicima da podešavaju sigurnosne parametre poput PIN-a i granice PPM za plinski senzor.

## Funkcionalnosti
- Unos PIN-a za deaktivaciju alarma
- Odabir između PIR (pokret) i plinskog senzora
- Resetiranje PIN-a
- Postavljanje granice PPM-a za plinski senzor
- Automatska detekcija pokreta ili plina i aktivacija alarma

## Simulacija (Wokwi)
Projekt je napravljen i testiran u [Wokwi simulatoru](https://wokwi.com/) – Arduino online alat za razvoj i simulaciju.

## Upute za korištenje

| Tipka  | Funkcija                          |
|--------|-----------------------------------|
| `A`    | Aktivacija PIR senzora            |
| `B`    | Aktivacija plinskog senzora       |
| `C`    | Resetiranje PIN-a                 |
| `D`    | Promjena granice PPM-a            |
| `#`    | Pohrana unosa (PIN/PPM)           |
| `*`    | Resetiranje unosa                 |

## Potrebne komponente
- Arduino UNO
- PIR senzor
- MQ-135 plinski senzor
- 16x2 LCD + I2C
- 4x4 tipkovnica
- Buzzer

## Postavljanje projekta
1. Otvori projekt u
2. Pokreni simulaciju
3. Koristi tipkovnicu za upravljanje alarmnim sustavom
4. 

## Autor
Mario Šrajbek  
GitHub: msrajbek-maker(https://github.com/msrajbek-maker)
