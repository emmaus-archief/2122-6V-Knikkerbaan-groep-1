#include <Arduino_JSON.h>

KnikkerPoort poortBoven = KnikkerPoort();
Servo360 servo360 = Servo360();
Valluik valluik = Valluik();
MolenPoort molenPoort = MolenPoort();


WiFiCommunicator wifi = WiFiCommunicator(WIFI_NETWERK, WIFI_WACHTWOORD, SERVER_DOMEINNAAM);
Teller tellerA = Teller(TELLER_A_PIN);
int serverContactInterval = 15;                // 15 seconden
unsigned long tijdVoorContactMetServer = 0;
// twelve servo objects can be created on most boards


int molenSnelheid = 85;    // variable to store the servo position
boolean ledIsAan = false;
unsigned long valluikTimer = 1000;
unsigned long ledTimer = 100;

void setup() {
  Serial.begin(9600);
  poortBoven.begin(BOVEN_POORT_PIN, 0, 90);
  servo360.begin(12);
  valluik.begin(9,10);
  molenPoort.begin(11);

  //wifi.begin();
  //myservo.attach(12);  // attaches the servo on pin 9 to the servo object
  //wifi.stuurVerzoek("/api/set/nieuwerun", "");


}


void loop() {
  // laat de teller detecteren:
  tellerA.update();
  molenPoort.update();
  valluik.update();


  // pauzeer de knikkerbaan als het tijd is voor contact met server
  if (millis() > tijdVoorContactMetServer && poortBoven.getOpen()) {
    poortBoven.sluit();
  }

  // knikkerbaan is leeggelopen, er zijn geen sensors dit iets moeten meten
  // nu is het tijd om contact te leggen met de server:
  if (millis() > tijdVoorContactMetServer + LEEGLOOP_TIJD) {
    servoPoort1.write(0); // nog aanpassen
    servoPoort2.write(180);// poort 2 gaat dicht als er contact met de server is
    servoMolen.write(90); // poort molen gaat dicht als er contact met de server is
    
    
    // contact met server houdt in:
    //   * nieuw totaal aantal knikkers versturen
    //   * instellingen ophalen


    // maak de reeks variabelen voor achter de URL:
    String data = "knikkers=";
    data = tellerA.getAantal();

    // als je meer waarden wilt toevoegen, ziet dat er zo uit:
    //data += "&blabla";
    //data += 5;

    // stuur deze data naar het juiste adres
    wifi.stuurVerzoek("/api/set/sensordata", data.c_str());

    // vraag bij de server de nieuwe instellingen op:
    String serverAntwoord = wifi.stuurVerzoek("/api/get/instellingen", "");

    // om een beeld te geven van het antwoord: print in seriële monitor:
    Serial.println(serverAntwoord);

    // om de instellingen gemakkelijk uit te kunnen lezen
    JSONVar ontvangenInstellingen = JSON.parse(serverAntwoord);                      // deze regel hoef je niet te begrijpen

    // check of de instellingen goed zijn binnengekomen
    // indien ja: pas instellingen op Arduino aan
    // indien nee: geef een foutmelding in de seriële monitor
    if (JSON.typeof(ontvangenInstellingen) != "undefined") {                         // je hoeft deze voorwaarde niet te begrijpen
      // stel de wachttijd (opnieuw) in:
      serverContactInterval = ontvangenInstellingen["wachttijdPoort"];
    }
    else {
      // evt. foutmelding:
      Serial.println("FOUT: serverAntwoord kon niet worden verwerkt");
    }

    
    // servercommunicatie is afgerond
    // bepaal nu op welke tijd de knikkerbaan
    // opnieuw contact moet zoeken
    tijdVoorContactMetServer = millis() + (unsigned long)serverContactInterval * 1000;

    // en zet nu het poortje weer open:
    poortBoven.open();
  }


  


}
