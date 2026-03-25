# Mailserver_LDR: ESP32 E-Mail Benachrichtigungssystem

Dieses Projekt implementiert ein Benachrichtigungssystem basierend auf einem ESP32-Mikrocontroller. Es überwacht einen lichtabhängigen Widerstand (LDR) und einen Taster. Bei bestimmten Ereignissen (Systemstart, Tastendruck, Lichteinfall über einem Schwellenwert) sendet der ESP32 automatisch E-Mails über einen SMTP-Server. Die E-Mails können sowohl im HTML- als auch im reinen Textformat gesendet werden, wobei die Vorlagen im LittleFS-Dateisystem des ESP32 gespeichert sind.

## Funktionen

*   **WLAN-Verbindung**: Verbindet sich automatisch mit dem konfigurierten Netzwerk.
*   **Zeitsynchronisation**: Holt die aktuelle Zeit via NTP (automatische Sommer-/Winterzeit-Umstellung für Mitteleuropa).
*   **LittleFS Dateisystem**: Nutzt den Flash-Speicher des ESP32, um E-Mail-Templates (`.html` und `.txt`) sicher und getrennt vom Code zu speichern.
*   **Aktivierungs-E-Mail**: Sendet eine Benachrichtigung direkt nach dem erfolgreichen Systemstart und der Zeitsynchronisation.
*   **Taster-Überwachung**: Sendet eine E-Mail, sobald der konfigurierte Taster (GPIO 0, Flash-Button) gedrückt wird. Entprellung (Debounce) ist in Software implementiert.
*   **LDR-Überwachung (Emergency Stop)**: Misst kontinuierlich die Spannung eines LDRs. Überschreitet der Wert einen Schwellenwert (z.B. bei Lichteinfall), wird ein "Emergency Stop" ausgelöst. Die Onboard-LED leuchtet auf und eine E-Mail mit der Laufzeit seit Systemstart wird versendet. Dies geschieht pro Systemstart nur einmal.

## Hardware-Anforderungen

*   ESP32 Entwicklungsboard (im Code konfiguriert als `nodemcu-32s`)
*   LDR (Fotowiderstand) als Spannungsteiler konfiguriert und an **GPIO 34** angeschlossen
*   USB-Kabel zur Stromversorgung und Programmierung

## Software-Anforderungen

*   [Visual Studio Code](https://code.visualstudio.com/)
*   [PlatformIO IDE](https://platformio.org/) Erweiterung für VS Code

## Setup-Anleitung

Befolgen Sie diese Schritte, um das Projekt erfolgreich auf Ihrem ESP32 zu installieren und auszuführen.

### 1. Projekt klonen oder öffnen

Öffnen Sie den Projektordner `Mailserver_LDR` in Visual Studio Code mit installierter PlatformIO-Erweiterung. PlatformIO sollte die benötigten Bibliotheken (wie `ESP Mail Client`) beim ersten Öffnen oder Kompilieren automatisch herunterladen.

### 2. Zugangsdaten konfigurieren (Secrets)

Ihre WLAN- und E-Mail-Zugangsdaten dürfen nicht öffentlich geteilt werden. Daher verwendet dieses Projekt eine separate `secrets.h` Datei. Diese muss zu erst noch erstellt werden. Als Beispiel dient die Datei `secrets.example.h`.

1.  Navigieren Sie in den Ordner `src/`.
2.  Erstellen Sie eine Kopie der Datei `secrets.example.h` und benennen Sie diese in `secrets.h` um.
3.  Öffnen Sie `secrets.h` und füllen Sie Ihre eigenen Daten ein:
    *   `WIFI_SSID`: Der Name Ihres WLAN-Netzwerks.
    *   `WIFI_PASSWORD`: Ihr WLAN-Passwort.
    *   `AUTHOR_EMAIL`: Die E-Mail-Adresse, von der gesendet werden soll (z.B. Ihr Gmail-Konto).
    *   `AUTHOR_PASSWORD`: Ihr E-Mail-Passwort. **Wichtig:** Wenn Sie Gmail verwenden, müssen Sie in Ihrem Google-Konto ein *App-Passwort* generieren (Zwei-Faktor-Authentifizierung muss aktiv sein). Verwenden Sie nicht Ihr normales Login-Passwort!
    *   `SMTP_HOST`: Der SMTP-Server Ihres Anbieters (für Gmail z.B. `smtp.gmail.com`).
    *   `SMTP_PORT`: Der Port für SSL (meist `465`).
    *   `RECIPIENT_EMAIL`: Die E-Mail-Adresse, die die Benachrichtigungen erhalten soll.

### 3. Dateisystem (LittleFS) hochladen

Die E-Mail-Vorlagen (Texte und HTML) befinden sich im Unterordner `data/`. Diese müssen zuerst in den Flash-Speicher des ESP32 geladen werden.

1.  Schließen Sie Ihren ESP32 über USB an den Computer an.
2.  Klicken Sie in der linken Seitenleiste von VS Code auf das **PlatformIO-Symbol** (Alien-Kopf).
3.  Erweitern Sie im Menüpunkt "Project Tasks" Ihren konfigurierten Environment-Namen (z.B. `env:nodemcu-32s`).
4.  Erweitern Sie den Ordner **Platform**.
5.  Klicken Sie auf **Build Filesystem Image**. Warten Sie auf die Erfolgsmeldung im Terminal.
6.  Klicken Sie auf **Upload Filesystem Image**. Die Dateien aus dem `data/` Verzeichnis werden nun auf den ESP32 geflasht.

### 4. Code kompilieren und hochladen

Nachdem das Dateisystem geflasht wurde, können Sie den eigentlichen C++ Programmcode hochladen.

1.  Klicken Sie in der unteren blauen PlatformIO-Leiste auf das Häkchen-Symbol (**Build**), um zu überprüfen, ob der Code fehlerfrei kompiliert.
2.  Klicken Sie anschließend auf das Pfeil-Symbol (**Upload**), um das Programm auf den ESP32 zu übertragen.
3.  (Optional) Klicken Sie auf das Stecker-Symbol (**Serial Monitor**), um die Ausgaben des ESP32 zu beobachten. Die Baudrate ist im Code auf 115200 konfiguriert.

## Anpassung des Codes

*   **Schwellenwert (Threshold):** In `src/main.cpp` ist `#define THRESHOLD 0.8` definiert. Passen Sie diesen Wert an die realen Licht-Bedingungen und Ihren LDR-Spannungsteiler an.
*   **Zeitzone:** Die NTP-Zeitzone ist in `main.cpp` unter `time_zone = "CET-1CEST,M3.5.0,M10.5.0/3"` konfiguriert (gültig für Mitteleuropa).
*   **E-Mail-Vorlagen:** Die HTML- und Text-Vorlagen können in den entsprechenden Dateien im `data/` Ordner angepasst werden. Belassen Sie Platzhalter wie `{{timestamp}}` oder `{{extra_info}}` intakt, damit diese im Code zur Laufzeit dynamisch ersetzt werden können.
*   **Email-Empfänger:** In `secrets.h` wird ein Hauptempfänger definiert. Für eine Weiterleitung ist der Weg über das Outlookkontos des Hauptempfängers zu empfehlen.
## Fehlerbehebung

*   **Verbindungsprobleme:** Überprüfen Sie die Ausgabe im Serial Monitor. Bei SMTP-Fehlern prüfen Sie Ihre E-Mail-Zugangsdaten (insbesondere das generierte App-Passwort).
*   **LittleFS Fehler:** Wenn das Auslesen der Dateien fehlschlägt ("Failed to mount LittleFS" im Log) oder leere E-Mails ankommen, stellen Sie sicher, dass Schritt 3 (Upload Filesystem Image) erfolgreich durchgeführt wurde.
*   **E-Mails im Spam-Ordner:** Falls keine E-Mails ankommen, prüfen Sie Ihren Spam-Ordner. Konfigurieren Sie gegebenenfalls Filterregeln, um die Absenderadresse zuzulassen.
