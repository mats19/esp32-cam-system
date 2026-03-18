# ESP32-Cam Autonomous MQTT Camera

## Konzept

Diese Schaltung realisiert eine autonom über Batterie betriebene Überwachungskamera basierend auf dem ESP32-Cam AI-Thinker Modul.

**Funktionsweise:**
1.  **Autonomer Betrieb:** Das System ist darauf ausgelegt, über längere Zeiträume batteriebetrieben zu laufen.
2.  **Intervall-Aufnahmen:** In definierten, größeren Zeitabständen wacht der ESP32 auf, nimmt ein Foto auf und sendet dieses über WLAN per MQTT an eine Home Assistant Instanz.
3.  **Energieeffizienz:** Zwischen den Aufnahmezyklen wird das Modul in den Deep-Sleep-Modus (Ruhemodus) versetzt, um den Energieverbrauch zu minimieren.
4.  **Manuelle Auslösung:** Über einen Taster kann jederzeit manuell eine Aufnahme getriggert werden.
5.  **Statusanzeige:** Zwei LEDs geben visuelles Feedback über den aktuellen Status oder Fehler (z.B. fehlende Internetverbindung).

## Hardware-Beschreibung

### Bausteine

#### Hauptplatine
*   **U1 (ESP32-Cam AI-Thinker Model):** Das Kernmodul, welches den ESP32-S SoC, die Kamera und den SD-Kartenslot (optional zur Pufferung) enthält. **Footprint:** `Spezifischer ESP32-Cam Footprint`
*   **J1 (Stromversorgung):** 2-Pin Anschluss für die externe Spannungsversorgung (5V). **Footprint:** `Connector_JST:JST_XH_S2B-XH-A_1x02_P2.50mm_Horizontal`
*   **J2 (Programmieranschluss):** USB Breakout Board (aufgelötet) mit Pins zum Flashen der Firmware und für Debug-Ausgaben via UART. **Footprint:** `Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical`
*   **J3 (Verbindung Frontpanel):** 4-Pin Buchse zum Anschluss der Tochterplatine. **Footprint:** `Connector_JST:JST_XH_S4B-XH-A_1x04_P2.50mm_Horizontal`

#### Tochterplatine (Frontpanel)
*   **J4 (Verbindung Hauptplatine):** 4-Pin Anschluss an die Hauptplatine. **Footprint:** `Connector_JST:JST_XH_S4B-XH-A_1x04_P2.50mm_Horizontal`
*   **SW1 (Taster):** Dient zum manuellen Auslösen eines Bildes (zieht IO12 auf GND). **Footprint:** `Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical`
*   **D1 (LED Rot) & R1:** Fehlerindikator. Leuchtet beispielsweise, wenn keine Verbindung zum MQTT-Broker oder WLAN hergestellt werden kann. **Footprint:** `LED_THT:LED_D5.0mm` & `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal`
*   **D2 (LED Status) & R2:** Statusindikator. Signalisiert aktive Vorgänge wie Bildaufnahme oder Speichervorgang. **Footprint:** `LED_THT:LED_D5.0mm` & `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal`

### Anschlüsse und Belegung (Labels)

#### J1 - Power Supply
*   **Pin 1:** `5V` (Eingangsspannung)
*   **Pin 2:** `GND` (Masse)

#### J2 - Programming Interface (FTDI)
Dieser Anschluss entspricht der Belegung gängiger FTDI-Adapter, wobei Pin 5 zum Aktivieren des Flash-Modus genutzt wird.
*   **Pin 1:** `5V` (Versorgung durch Programmer, falls Batterie getrennt)
*   **Pin 2:** `GND`
*   **Pin 3:** `U0T` (TXD0 des ESP32 -> RXD des Programmers)
*   **Pin 4:** `U0R` (RXD0 des ESP32 -> TXD des Programmers)
*   **Pin 5:** `IO0` (Boot Mode Selection). Wird dieser Pin während des Resets auf GND gezogen, startet der ESP32 im Download-Modus.

#### J3 / J4 - Board-to-Board Verbindung
Verbindung zwischen Haupt- und Tochterplatine.
*   **Pin 1:** `GND`
*   **Pin 2:** `TRIGGER` (IO12)
*   **Pin 3:** `LED_ERROR` (IO13)
*   **Pin 4:** `LED_STATUS` (IO15)

#### Interne Signale & GPIOs

| Label | GPIO | Beschreibung |
| :--- | :--- | :--- |
| **TRIGGER** | IO12 | Verbunden mit **SW1**. Active-Low. Dient als Wake-Up Source aus dem Deep Sleep oder als manueller Auslöser. |
| **LED_ERROR** | IO13 | Verbunden mit **D1** (Rot). High-Active (Anode an GPIO). Zeigt Fehler an. |
| **LED_STATUS**| IO15 | Verbunden mit **D2**. High-Active. Zeigt Aktivität an. |
| **U0T** | IO1 | Serial Transmit (UART0). |
| **U0R** | IO3 | Serial Receive (UART0). |
| **IO0** | IO0 | Boot-Konfiguration. Muss zum Flashen auf GND liegen. |

---
*Erstellt mit KiCad.*