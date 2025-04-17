#include <Arduino.h>
#include <WiFi.h>            
#include "SevSeg.h"          // Librairie pour controler un afficheur a 7 segments
#include <vector>            // Librairie pour utiliser des vecteurs (stockage des alertes)
#include "index.h"           // Fichier contenant le code HTML de l'interface web

SevSeg sevseg;               // Objet pour controler l'afficheur a 7 segments

// ========================================= Wi-Fi ===============================================

const char *ssid = "UNIFI_IDO1";             // Nom du reseau Wi-Fi
const char *password = "41Bidules!";         // Mot de passe du reseau Wi-Fi

// paramaters pour fixer l'addresse ip
IPAddress local_IP(192, 168, 10, 217);      // l'addresse ip
IPAddress gateway(192, 168, 10, 1);         // passerell
IPAddress subnet(255, 255, 255, 0);         // masque de réseau

WiFiServer server(80);                      // Serveur web sur le port 80 (HTTP)

// ===============================================================================================

// ========================================= Minuteur ============================================

int minutes = 0;                  // Temps restant en minutes
int seconds = 0;                  // Temps restant en secondes
bool running = false;             // etat du minuteur (true = en cours, false = arrête)
std::vector<int> alerts;          // Vecteur pour stocker les alertes
int currentAlertIndex = 0;        // Index de l'alerte actuelle dans le vecteur alerts

// =================================================================================================

// ========================================= Alerte sonore =========================================

#define SPEAKER_PIN 5         
bool beeping = false;                 // etat du bip (true = en cours, false = arrête)
unsigned long beepStartTime = 0;      // Moment ou le bip a commence (pour mesurer la duree)

// ==================================================================================================

// ============================================ Fonctions ===========================================

void handleWiFiClient();      // Gère les requêtes HTTP des clients connectes
void handleTimer();           // Gère le decompte du minuteur et l'affichage
void handleBeep();            // Gère les alertes sonores (bips)

// ===================================================================================================

void setup() {

  Serial.begin(115200);       
  delay(1000);                        // Delai d'1 seconde pour stabiliser le demarrage

  pinMode(SPEAKER_PIN, OUTPUT);       // Configuration de la broche du haut-parleur comme sortie

  
  //_________ Initialisation de l'afficheur 7 segments __________
  byte numDigits = 3;         
  byte digitPins[] = {25, 26, 27};
  byte segmentPins[] = {13, 12, 14, 32, 33, 15, 4}; 
  bool resistorsOnSegments = true;

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments, false, false, true);
  sevseg.setBrightness(200);

  for (int i = 0; i < 3; i++) pinMode(digitPins[i], OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(segmentPins[i], OUTPUT);

  //_______________ Connexion Wi-Fi ______________________________
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(local_IP, gateway, subnet); // Fixer l'adresse IP
  WiFi.begin(ssid, password); 
  
  unsigned long wifiTimeout = millis() + 10000; // Timeout de 10 secondes pour la connexion
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); Serial.print(".");     
    if (millis() > wifiTimeout) { 
      Serial.println("\nWiFi timeout. Continuing without WiFi.");
      break;                  // Sort de la boucle sans connexion
    }
  }
  // Si la connexion Wi-Fi est etablie
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.println("\nConnected!");
    Serial.print("Web server active at: http://");
    Serial.println(WiFi.localIP()); 
    server.begin();           
  } else {
    Serial.println("\nStarting without WiFi."); // Si pas de connexion, continue sans Wi-Fi
  }

}

void loop() {

  handleWiFiClient();         // Verifie et traite les requêtes HTTP
  handleTimer();              // Met a jour le minuteur et l'afficheur
  handleBeep();               // Gère les alertes sonores

}

// =============================== Gestion des requêtes HTTP reçues via Wi-Fi ===============================
void handleWiFiClient() {

  WiFiClient client = server.available();             // Verifie si un client est connecte
  if (!client) return;                                // Si aucun client, sort de la fonction

  String request = client.readStringUntil('\r');     // Lit la requête HTTP jusqu'a la fin de ligne
  client.flush();                                    // Vide le buffer du client

  // Commandes simples via URL
  if (request.indexOf("/start") != -1) running = true;      // Demarre le minuteur
  if (request.indexOf("/stop") != -1) running = false;      // Arrête le minuteur
  if (request.indexOf("/reset") != -1) {                    // Reinitialise le minuteur
    minutes = 0;
    seconds = 0;
    running = false;
    alerts.clear();                       // Supprime toutes les alertes
    currentAlertIndex = 0;               // Reinitialise l'index des alertes
  }

  // Requête pour obtenir les donnees actuelles
  if (request.indexOf("/data") != -1) {
    client.println("HTTP/1.1 200 OK");                  // Reponse HTTP reussie
    client.println("Content-Type: application/json");   // Type de contenu JSON
    client.println();                     
    client.print("{\"minutes\":");        // Envoie les minutes
    client.print(minutes);
    client.print(", \"seconds\": ");     // Envoie les secondes
    client.print(seconds);
    client.print("}");                   
    client.stop();                       // Ferme la connexion
    return;
  }

  // Initialise la durée du minuteur
  if (request.indexOf("/settime?minutes=") != -1) {
    int startIndex = request.indexOf("minutes=") + 8;         // Position du nombre
    int newMinutes = request.substring(startIndex).toInt();   // Extrait et convertit en entier
    minutes = newMinutes;                                     // Met a jour les minutes
    currentAlertIndex = 0;                                    // Reinitialise l'index des alertes

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.print("{\"minutes\": ");
    client.print(minutes);          // Confirme la nouvelle valeur
    client.print("}");                   
    client.stop();
    return;
  }

  //  Ajoute une alerte dans le vecteur "alerts"
  if (request.indexOf("/settime?alerts=") != -1) {
    int startIndex = request.indexOf("alerts=") + 7;        // Position du nombre
    int newAlert = request.substring(startIndex).toInt();   // Extrait l'alerte
    alerts.push_back(newAlert);                             // Ajoute l'alerte au vecteur

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.print("{\"alerts\": ");
    client.print(newAlert);           // Confirme l'alerte ajoutee
    client.print("}");                   
    client.stop();
    return;
  }

  // Envoi de la page HTML par defaut
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println(htmlPage);              // Envoie le contenu de la page web
  client.stop();                         // Ferme la connexion
}
// ================================================================================================================

// ========================================== Controle du minuteur ================================================
void handleTimer() {
  static unsigned long previousMillis = 0;  
  unsigned long currentMillis = millis();  // Temps actuel

  // Si le minuteur est en cours et qu'une seconde s'est ecoulee
  if (running && currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;        // Met a jour le temps de reference

    if (seconds == 0) {                   // Si les secondes sont a 0
      if (minutes > 0) {                  // S'il reste des minutes
        minutes--;                        // Decremente les minutes
        seconds = 59;                     // Reinitialise les secondes
      
      } else {
        running = false;                  // Arrête le minuteur si temps ecoule
      }
    } else {
      seconds--;                          // Decremente les secondes
    }
  }

  // Mise a jour de l'afficheur
    sevseg.setNumber(minutes);            // Affiche les minutes
    sevseg.refreshDisplay();
}



// =================================================================================================================

// ====================================== Gestion du bip sonore pour les alertes ===================================
void handleBeep() {
  // Declenche un bip si une alerte est atteinte (a la minute specifiee, 1 seconde avant)
  if (!beeping && currentAlertIndex < alerts.size() && minutes == alerts[currentAlertIndex] && seconds == 1) {
    tone(SPEAKER_PIN, 1000);              
    beepStartTime = millis();             // Enregistre le debut du bip
    beeping = true;                       // Indique que le bip est en cours
  }

  // Arrête le bip après 3 secondes
  if (beeping && (millis() - beepStartTime >= 3000)) {
    noTone(SPEAKER_PIN);                  
    beeping = false;                      
    currentAlertIndex++;                 
  }
}
// ====================================================================================================================