#ifndef SECRETS_EXAMPLE_H
#define SECRETS_EXAMPLE_H

// WLAN Credentials
const char *WIFI_SSID = "DEIN_WLAN_NAME";
const char *WIFI_PASSWORD = "DEIN_WLAN_PASSWORT";

// SMTP Server Setup (Beispiel: Gmail)
// Wichtig bei Gmail: App-Passwort generieren und verwenden, NICHT das normale Account-Passwort!
const char *AUTHOR_EMAIL = "deine.absender.email@gmail.com";
const char *AUTHOR_PASSWORD = "DEIN_APP_PASSWORT";
const char *SMTP_HOST = "smtp.gmail.com";
const int SMTP_PORT = 465;

// Empfänger Adresse
const char *RECIPIENT_EMAIL = "empfaenger.email@beispiel.com";

#endif // SECRETS_EXAMPLE_H
