# Minuteur d'examen ESP32 avec interface web 🕒📱

Ce projet permet de gérer un **minuteur d'examen** avec un ESP32. Il inclut :

- Une **interface web** pour contrôler le minuteur (démarrer, arrêter, réinitialiser)
- Un affichage **7 segments** pour visualiser le temps
- Un système d'**alertes sonores** quelques minutes avant la fin

---

## 🔧 Matériel utilisé

- ESP32 (WROOM)
- Afficheur 7 segments (3 digits)
- Haut-parleur (buzzer)
- Résistances (220 ohms)
- Connexion Wi-Fi (réseau local)

---

## 🌐 Interface web

Accessible via l'adresse IP locale de l'ESP32  
**Exemple** : `http://192.168.10.217`

Fonctionnalités :
- Définir le temps total de l'examen
- Ajouter des alertes (ex: 5 minutes avant la fin)
- Démarrer / Arrêter / Réinitialiser
- Verrouiller les boutons pour empêcher les modifications

---

## 📁 Organisation des fichiers

- `src/` : Code principal (`main.cpp`)
- `include/index.h` : Page HTML de l'interface web
- `platformio.ini` : Configuration PlatformIO

---

## 📦 Installation

1. Ouvrir le projet dans PlatformIO (VS Code)
2. Flasher l'ESP32
3. Se connecter au Wi-Fi (Changer le nom du réseau Wi-Fi (SSID) et son mot de passe)
4. Ouvrir un navigateur et aller à l'adresse IP affichée dans le moniteur série

---

## ✨ Exemple

![image](https://github.com/user-attachments/assets/0b7c3e64-5f65-415c-91f6-a483d83cc4ff)


