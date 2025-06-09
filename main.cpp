// ############################################################################################### //
// ESP32 Radio Evo3 - Interent Radio Player                                                        //
// ############################################################################################### //
// Robgold 2025                                                                                    //
// Source -> https://github.com/dzikakuna/ESP32_radio_evo3/tree/main/src/ESP32_radio_v2_evo3.18    //
// ############################################################################################### //

#include "Arduino.h"           // Standardowy nagłówek Arduino, który dostarcza podstawowe funkcje i definicje
#include <WiFiManager.h>       // Biblioteka do zarządzania konfiguracją sieci WiFi, opis jak ustawić połączenie WiFi przy pierwszym uruchomieniu jest opisany tu: https://github.com/tzapu/WiFiManager
#include "Audio.h"             // Biblioteka do obsługi funkcji związanych z dźwiękiem i audio
#include "SPI.h"               // Biblioteka do obsługi komunikacji SPI
#include "SPIFFS.h"            // Biblioteka do obsługi kart SPIFFS
#include "FS.h"                // Biblioteka do obsługi systemu plików
#include <U8g2lib.h>           // Biblioteka do obsługi wyświetlaczy
#include <ezButton.h>          // Biblioteka do obsługi enkodera z przyciskiem
#include <HTTPClient.h>        // Biblioteka do wykonywania żądań HTTP, umożliwia komunikację z serwerami przez protokół HTTP
#include <Ticker.h>            // Mechanizm tickera do odświeżania timera 1s, pomocny do cyklicznych akcji w pętli głównej
#include <EEPROM.h>            // Bilbioteka obsługi emulacji pamięci EEPROM
#include <Time.h>              // Biblioteka do obsługi funkcji związanych z czasem, np. odczytu daty i godziny
#include <ESPAsyncWebServer.h> // Bliblioteka asyncrhionicznego serwera web
#include <AsyncTCP.h>          // Bliblioteka TCP dla serwera web
#include <Update.h>            // Blibioteka dla aktulizacji OTA
#include <ESPmDNS.h>           // Blibioteka mDNS dla ESP
#include "font.h"              // Plik nagłówkowy z czcionką

// Deklaracja wersji oprogramowania i nazwy hosta widocznego w routerze oraz na ekranie OLED i stronie www
#define softwareRev "v3.18.06"  // Wersja oprogramowania radia
#define hostname "esp32radio"   // Definicja nazwy hosta widoczna na zewnątrz


// Definicja pinow dla wyswietlacza OLED 
#define SPI_MOSI_OLED 39  // Pin MOSI (Master Out Slave In) dla interfejsu SPI OLED
#define SPI_MISO_OLED 0   // Pin MISO (Master In Slave Out) brak dla wyswietlacza OLED
#define SPI_SCK_OLED 38   // Pin SCK (Serial Clock) dla interfejsu SPI OLED
#define CS_OLED 42        // Pin CS (Chip Select) dla interfejsu OLED
#define DC_OLED 40        // Pin DC (Data/Command) dla interfejsu OLED
#define RESET_OLED 41     // Pin Reset dla interfejsu OLED

// Rozmiar wysweitlacz OLED (potrzebny do wyświetlania grafiki)
#define SCREEN_WIDTH 256        // Szerokość ekranu w pikselach
#define SCREEN_HEIGHT 64        // Wysokość ekranu w pikselach

// Definicja pinow dla przetwornika PCM5102A
#define I2S_DOUT 13             // Podłączenie do pinu DIN na DAC
#define I2S_BCLK 12             // Podłączenie po pinu BCK na DAC
#define I2S_LRC 14              // Podłączenie do pinu LCK na DAC

// Enkoder 2 - uzywany dla radia
#define CLK_PIN2 10             // Podłączenie z pinu 10 do CLK na enkoderze
#define DT_PIN2 11              // Podłączenie z pinu 11 do DT na enkoderze lewym
#define SW_PIN2 9               // Podłączenie z pinu 1 do SW na enkoderze lewym (przycisk)

// IR odbiornik podczerwieni 
#define recv_pin 6

// definicja dlugosci ilosci stacji w banku, dlugosci nazwy stacji w PSRAM/EEPROM, maksymalnej ilosci plikow audio (odtwarzacz)
#define MAX_STATIONS 99          // Maksymalna liczba stacji radiowych, które mogą być przechowywane w jednym banku
#define STATION_NAME_LENGTH 220  // Nazwa stacji wraz z bankiem i numerem stacji do wyświetlenia w pierwszej linii na ekranie
#define MAX_FILES 100            // Maksymalna liczba plików lub katalogów w tablicy directoriesz


#define STATIONS_URL1 "https://raw.githubusercontent.com/hevet/radio-player-stream/main/bank01.txt"       // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL2 "https://raw.githubusercontent.com/hevet/radio-player-stream/main/bank02.txt"       // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL3 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank03.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL4 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank04.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL5 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank05.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL6 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank06.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL7 "https://raw.githubusercontent.com/hevet/radio-player-stream/main/bank07.txt"       // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL8 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank08.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL9 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank09.txt"   // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL10 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank10.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL11 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank11.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL12 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank12.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL13 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank13.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL14 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank14.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL15 "https://raw.githubusercontent.com/dzikakuna/ESP32_radio_streams/main/bank15.txt"  // Adres URL do pliku z listą stacji radiowych
#define STATIONS_URL16 "https://raw.githubusercontent.com/hevet/radio-player-stream/main/bank16.txt"       // Adres URL do pliku z listą stacji radiowych


// ############### DEFINICJA DLA PILOTa IR w standardzie NEC - przeniesiona do pliku txt na karcie ############### //
uint16_t rcCmdVolumeUp = 0;   // Głosnosc +
uint16_t rcCmdVolumeDown = 0; // Głośnosc -
uint16_t rcCmdArrowRight = 0; // strzałka w prawo - nastepna stacja
uint16_t rcCmdArrowLeft = 0;  // strzałka w lewo - poprzednia stacja  
uint16_t rcCmdArrowUp = 0;    // strzałka w góre - lista stacji krok do gory
uint16_t rcCmdArrowDown = 0;  // strzałka w dół - lista stacj krok na dół
uint16_t rcCmdBack = 0;	      // Przycisk powrotu
uint16_t rcCmdOk = 0;         // Przycisk Ent - zatwierdzenie stacji
uint16_t rcCmdSrc = 0;        // Przełączanie źródła radio, odtwarzacz
uint16_t rcCmdMute = 0;       // Wyciszenie dzwieku
uint16_t rcCmdAud = 0;        // Equalizer dzwieku
uint16_t rcCmdDirect = 0;     // Janość ekranu, dwa tryby 1/16 lub pełna janość     
uint16_t rcCmdBankMinus = 0;  // Wysweitla wybór banku
uint16_t rcCmdBankPlus = 0;   // Wysweitla wybór banku
uint16_t rcCmdRed = 0;        // Przełącza ładowanie banku kartaSPIFFS - serwer GitHub w menu bank
uint16_t rcCmdGreen = 0;      // VU wyłaczony, VU tryb 1, VU tryb 2, zegar
uint16_t rcCmdKey0 = 0;       // Przycisk "0"
uint16_t rcCmdKey1 = 0;       // Przycisk "1"
uint16_t rcCmdKey2 = 0;       // Przycisk "2"
uint16_t rcCmdKey3 = 0;       // Przycisk "3"
uint16_t rcCmdKey4 = 0;       // Przycisk "4"
uint16_t rcCmdKey5 = 0;       // Przycisk "5"
uint16_t rcCmdKey6 = 0;       // Przycisk "6"
uint16_t rcCmdKey7 = 0;       // Przycisk "7"
uint16_t rcCmdKey8 = 0;       // Przycisk "8"
uint16_t rcCmdKey9 = 0;       // Przycisk "9"
// Koniec definicji pilota IR

int currentSelection = 0;       // Numer aktualnego wyboru na ekranie OLED
int firstVisibleLine = 0;       // Numer pierwszej widocznej linii na ekranie OLED
uint8_t station_nr = 0;         // Numer aktualnie wybranej stacji radiowej z listy
int stationFromBuffer = 0;      // Numer stacji radiowej przechowywanej w buforze do przywrocenia na ekran po bezczynności
uint8_t bank_nr;                // Numer aktualnie wybranego banku stacji z listy
uint8_t previous_bank_nr = 0;   // Numer banku przed wejsciem do menu zmiany banku
int bankFromBuffer = 0;         // Numer aktualnie wybranego banku stacji z listy do przywrócenia na ekran po bezczynności
int CLK_state2;                        // Aktualny stan CLK enkodera lewego
int prev_CLK_state2;                   // Poprzedni stan CLK enkodera lewego
int stationsCount = 0;                 // Aktualna liczba przechowywanych stacji w tablicy
uint8_t volumeValue = 10;                  // Wartość głośności, domyślnie ustawiona na 10
uint8_t maxVolume = 21;
bool maxVolumeExt =  false;                 // 0(false) -  zakres standardowy Volume 1-21 , 1 (true) - zakres rozszerzony 0-42
uint8_t volumeBufferValue = 0;             // Wartość głośności, domyślnie ustawiona na 10
int maxVisibleLines = 4;               // Maksymalna liczba widocznych linii na ekranie OLED
int bitrateStringInt = 0;              // Deklaracja zmiennej do konwersji Bitrate string na wartosc Int aby podzelic bitrate przez 1000
int buttonLongPressTime2 = 2000;       // Czas reakcji na długie nacisniecie enkoder 2
int buttonShortPressTime2 = 500;       // Czas rekacjinna krótkie nacisniecie enkodera 2
int buttonSuperLongPressTime2 = 4000;  // Czas reakcji na super długie nacisniecie enkoder 2
uint8_t stationNameLenghtCut = 24;    // 24-> 25 znakow, 25-> 26 znaków, zmienna określająca jak długa nazwę ma nazwa stacji w plikach Bankow liczone od 0- wartosci ustalonej
uint8_t yPositionDisplayScrollerMode0 = 33;
uint8_t yPositionDisplayScrollerMode1 = 61;
uint8_t yPositionDisplayScrollerMode2 = 25;
uint16_t stationStringScrollLength = 0;
uint8_t maxStationVisibleStringScrollLength = 46;
// ---- Głosowe odtwarzanie czasu co godzinę ---- //
bool voiceTimePlay = false; 
bool voiceTimePlayActionTaken = false;
bool timeVoiceInfoEveryHour = true;


// ---- Auto dimmer / auto przyciemnianie wyswietlacza ---- //
uint8_t displayDimmerTimeCounter = 0;  // Zmienna inkrementowana w przerwaniu timera2 do przycimniania wyswietlacz
uint8_t dimmerDisplayBrightness = 10;   // Wartość przyciemnienia wyswietlacza po czasie niekatywnosci
uint8_t displayBrightness = 180;        // Domyślna maksymalna janość wyswietlacza
uint16_t displayAutoDimmerTime = 10;   // Czas po jakim nastąpi przyciemninie wyswietlacza, liczony w sekundach
bool displayAutoDimmerOn = false;       // Automatyczne przyciemnianie wyswietlacza, domyślnie włączone
bool displayDimmerActive = false;       // Aktywny tryb przyciemnienia
uint16_t displayPowerSaveTime = 30;    // Czas po jakim zostanie wyłączony wyswietlacz OLED (tryb power save)
uint16_t displayPowerSaveTimeCounter = 0;    // Timer trybu Power Save wyswietlacza OLED
bool displayPowerSaveEnabled = false;   // Flaga okreslajaca czy tryb wyłączania wyswietlacza OLED jest właczony
uint8_t displayMode = 0;               // Tryb wyswietlacza 0-displayRadio z przewijaniem "scroller" / 1-Zegar / 2- tryb 3 stałych linijek tekstu stacji

// ---- Equalzier ---- //
int8_t toneLowValue = 0;               // Wartosc filtra dla tonow niskich
int8_t toneMidValue = 0;               // Wartosc flitra dla tonow srednich
int8_t toneHiValue = 0;                // Wartość filtra dla tonow wysokich
uint8_t toneSelect = 1;                // Zmienna okreslająca, który filtr equalizera regulujemy, startujemy od pierwszego
bool equalizerMenuEnable = false;      // Flaga wyswietlania menu Equalizera


uint8_t rcInputDigit1 = 0xFF;      // Pierwsza cyfra w przy wprowadzaniu numeru stacji z pilota
uint8_t rcInputDigit2 = 0xFF;      // Druga cyfra w przy wprowadzaniu numeru stacji z pilota


// ---- Zmienne konfiguracji ---- //
uint16_t configArray[16] = { 0 };
uint8_t rcPage = 0;
uint16_t configRemoteArray[30] = { 0 };   // Tablica przechowująca kody pilota podczas odczytu z pliku
uint16_t configAdcArray[20] = { 0 };      // Tablica przechowująca wartosci ADC dla przyciskow klawiatury
bool configExist = true;                  // Flaga okreslajaca czy istnieje plik konfiguracji


//const int maxVisibleLines = 5;  // Maksymalna liczba widocznych linii na ekranie OLED
bool encoderButton2 = false;      // Flaga określająca, czy przycisk enkodera 2 został wciśnięty
bool encoderFunctionOrder = true; // Flaga okreslająca kolejność funkcji enkodera 2
bool displayActive = false;       // Flaga określająca, czy wyświetlacz jest aktywny
bool mp3 = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie MP3
bool flac = false;                // Flaga określająca, czy aktualny plik audio jest w formacie FLAC
bool aac = false;                 // Flaga określająca, czy aktualny plik audio jest w formacie AAC
bool vorbis = false;              // Flaga określająca, czy aktualny plik audio jest w formacie VORBIS
bool opus = false;                // Flaga określająca, czy aktualny plik audio jest w formacie OPUS
bool id3tag = false;              // Flaga określająca, czy plik audio posiada dane ID3
bool timeDisplay = true;          // Flaga określająca kiedy pokazać czas na wyświetlaczu, domyślnie od razu po starcie
bool listedStations = false;      // Flaga określająca czy na ekranie jest pokazana lista stacji do wyboru
bool bankMenuEnable = false;      // Flaga określająca czy na ekranie jest wyświetlone menu wyboru banku
bool bitratePresent = false;      // Flaga określająca, czy na serial terminalu pojawiła się informacja o bitrate - jako ostatnia dana spływajaca z info
bool bankNetworkUpdate = false;   // Flaga wyboru aktualizacji banku z sieci lub karty SPIFFS - True aktulizacja z NETu
bool volumeMute = false;          // Flaga okreslająca stan funkcji wyciszczenia - Mute
bool volumeSet = false;           // Flaga wejscia menu regulacji głosnosci na enkoderze 2
bool vuMeterOn = true;            // Flaga właczajaca wskazniki VU
bool vuMeterMode = false;         // tryb rysowania vuMeter
bool action3Taken = false;        // Flaga Akcji 3 - załaczenia VU
bool ActionNeedUpdateTime = false;// Zmiena okresaljaca dla displayRadio potrzebe odczytu aktulizacji czasu
bool debugAudioBuffor = false;    // Wyswietlanie bufora Audio
bool audioInfoRefresh = false;    // Flaga wymuszjąca wymagane odsiwezenie ze względu na zmianę info stream - stationstring, title, 
bool audioInfoRefresh2 = false;   // Flaga wymuszjąca wymagane odsiwezenie ze względu na zmianę info stream - linia kbps, khz, format streamu
bool noSPIFFScard = false;        // flaga ustawiana przy braku wykrycia karty SPIFFS
bool resumePlay = false;          // Flaga wymaganego uruchomienia odtwarzania po zakonczeniu komunikatu głosowego
bool fwupd = false;               // Flaga blokujaca main loop podczas aktualizacji oprogramowania
bool configIrExist = false;       // Flaga informująca o istnieniu poprawnej konfiguracji pilota IR
bool wsAudioRefresh = false;      // Flaga informujaca o potrzebe odswiezeninia Station Text za pomoca Web Sokcet


unsigned long debounceDelay = 300;    // Czas trwania debouncingu w milisekundach
unsigned long displayTimeout = 5000;  // Czas wyświetlania komunikatu na ekranie w milisekundach
unsigned long displayStartTime = 0;   // Czas rozpoczęcia wyświetlania komunikatu
unsigned long seconds = 0;            // Licznik sekund timera
unsigned int PSRAM_lenght = MAX_STATIONS * (STATION_NAME_LENGTH) + MAX_STATIONS; // deklaracjia długości pamięci PSRAM
unsigned long lastCheckTime = 0;      // No stream audio blink
uint8_t stationNameStreamWidth = 0;   // Test pełnej nazwy stacji
uint8_t x = 0;                             // Globalna zmienna pomocnicza 

// ---- VU Meter ---- //
unsigned long vuMeterTime;                 // Czas opznienia odswiezania wskaznikow VU w milisekundach
uint8_t vuMeterL;                          // Wartosc VU dla L kanału zakres 0-255
uint8_t vuMeterR;                          // Wartosc VU dla R kanału zakres 0-255
uint8_t peakL = 0;                         // Wartosc peakHold dla kanalu lewego
uint8_t peakR = 0;                         // Wartosc peakHold dla kanalu prawego
uint8_t peakHoldTimeL = 0;                 // Licznik peakHold dla kanalu lewego
uint8_t peakHoldTimeR = 0;                 // Licznik peakHold dla kanalu prawegoAdd commentMore actions
const uint8_t peakHoldThreshold = 5;       // Liczba cykli zanim peak opadnie
const uint8_t vuLy = 41;                   // Koordynata Y wskaznika VU L-lewego (wyzej)
const uint8_t vuRy = 47;                   // Koordynata Y wskaznika VU R-prawego (nizej)


unsigned long scrollingStationStringTime;  // Czas do odswiezania scorllingu
uint8_t scrollingRefresh = 50;              // Czas w ms przewijania tekstu funkcji Scroller

uint8_t vuMeterRefreshCounterSet = 0;      // Mnoznik co ile petli loopRefreshTime ma byc odswiezony VU Meter
uint8_t scrollerRefreshCounterSet = 0;     // Mnoznik co ile petli loopRefreshTime ma byc odswiezony i przewiniety o 1px Scroller

uint16_t stationStringScrollWidth;         // szerokosc Stringu nazwy stacji w funkcji Scrollera
uint16_t xPositionStationString = 0;       // Pozycja początkowa dla przewijania tekstu StationString
uint16_t offset;                           // Zminnna offsetu dla funkcji Scrollera - przewijania streamtitle na ekranie OLED
unsigned char * psramData;                 // zmienna do trzymania danych stacji w pamieci PSRAM

unsigned long vuMeterMilisTimeUpdate;           // Zmienna przechowujaca czas dla funkci millis VU Meter refresh
uint8_t vuMeterRefreshTime = 65;                // Czas w ms odswiezania VUmetera

// ---- Serwer Web ---- //
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
bool urlToPlay = false;

// ---- Sprawdzenie funkcji pilota, zminnne do pomiaru róznicy czasów ---- //
unsigned long runTime = 0;
unsigned long runTime1 = 0;
unsigned long runTime2 = 0;


String stationStringScroll = "";     // Zmienna przechowująca tekst do przewijania na ekranie
String stationName;                  // Nazwa aktualnie wybranej stacji radiowej
String stationString;                // Dodatkowe dane stacji radiowej (jeśli istnieją)
String stationStringWeb;             // Dodatkowe dane stacji radiowej (jeśli istnieją)
String stationNameStreamWeb;         // Dodatkowe dane stacji radiowej (jeśli istnieją)
String bitrateString;                // Zmienna przechowująca informację o bitrate
String sampleRateString;             // Zmienna przechowująca informację o sample rate
String bitsPerSampleString;          // Zmienna przechowująca informację o liczbie bitów na próbkę
String currentIP;
String stationNameStream;           // Nazwa stacji wyciągnieta z danych wysylanych przez stream

String header;                      // Zmienna dla serwera www
String sliderValue = "0";
String url2play = "";


File myFile;  // Uchwyt pliku

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R2, /* cs=*/CS_OLED, /* dc=*/DC_OLED, /* reset=*/RESET_OLED);  // Hardware SPI 3.12inch OLED
//U8G2_SH1122_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ CS_OLED, /* dc=*/ DC_OLED, /* reset=*/ RESET_OLED);		// Hardware SPI  2.08inch OLED
//U8G2_SSD1363_256X128_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/CS_OLED, /* dc=*/DC_OLED, /* reset=*/RESET_OLED);  // Hardware SPI 3.12inch OLED

// Przypisujemy port serwera www
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); //Start obsługi Websocketów


// Inicjalizacja WiFiManagera
WiFiManager wifiManager;

// Konfiguracja nowego SPI z wybranymi pinami dla czytnika kart SPIFFS
SPIClass customSPI = SPIClass(HSPI);  // Używamy HSPI, ale z własnymi pinami

ezButton button2(SW_PIN2);  // Utworzenie obiektu przycisku z enkodera 1 ezButton, podłączonego do pinu 1
Audio audio;                // Obiekt do obsługi funkcji związanych z dźwiękiem i audio
AudioBuffer audioBuffer;
Ticker timer1;             // Timer do updateTimer co 1s
Ticker timer2;             // Timer do getWeatherData co 60s
//Ticker timer3;           // Timer do przełączania wyświetlania danych pogodoych w ostatniej linii co 10s
WiFiClient client;         // Obiekt do obsługi połączenia WiFi dla klienta HTTP

const char stylehead_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <link rel='icon' href='/favicon.ico' type='image/x-icon'>
  <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
  <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
  <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Web Radio</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 1.3rem;}
    p {font-size: 0.95rem;}
    table {border: 1px solid black; border-collapse: collapse; margin: 0px 0px;}
    td, th {font-size: 0.8rem; border: 1px solid gray; border-collapse: collapse;}
    td:hover {font-weight:bold;}
    a {color: black; text-decoration: none;}
    body {max-width: 1380px; margin:0px auto; padding-bottom: 25px; background: #D0D0D0;}
    .slider {-webkit-appearance: none; margin: 14px; width: 330px; height: 10px; background: #4CAF50; outline: none; -webkit-transition: .2s; transition: opacity .2s; border-radius: 5px;}
    .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 25px; background: #4a4a4a; cursor: pointer; border-radius: 5px;}
    .slider::-moz-range-thumb { width: 35px; height: 35px; background: #4a4a4a; cursor: pointer; border-radius: 5px;} 
    .button { background-color: #4CAF50; border: 1; color: white; padding: 10px 20px; border-radius: 5px;}
    .buttonBank { background-color: #4CAF50; border: 1; color: white; padding: 8px 8px; border-radius: 5px; width: 35px; height: 35px; margin: 0 1.5px;}
    .buttonBankSelected { background-color: #505050; border: 1; color: white; padding: 8px 8px; border-radius: 5px; width: 35px; height: 35px; margin: 0 1.5px;}
    .buttonBank:active {background-color: #4a4a4a box-shadow: 0 4px #666; transform: translateY(2px);}
    .buttonBank:hover {background-color: #4a4a4a;}
    .button:hover {background-color: #4a4a4a;}
    .button:active {background-color: #4a4a4a; box-shadow: 0 4px #666; transform: translateY(2px);}
    .column { align: center; padding: 5px; display: flex; justify-content: space-between;}
    .columnlist { align: center; padding: 10px; display: flex; justify-content: center;}
    .stationList {text-align:left; margin-top: 0px; width: 280px; margin-bottom:0px;cursor: pointer;}
	  .stationNumberList {text-align:center; margin-top: 0px; width: 35px; margin-bottom:0px;}
	  .stationListSelected {text-align:left; margin-top: 0px; width: 280px; margin-bottom:0px;cursor: pointer; background-color: #4CAF50;}
	  .stationNumberListSelected {text-align:center; margin-top: 0px; width: 35px; margin-bottom:0px; background-color: #4CAF50;}
    .station-name   
  </style>
</head>
)rawliteral";


const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <link rel='icon' href='/favicon.ico' type='image/x-icon'>
    <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
    <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
    <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Web Radio</title>
    <style>
      html {font-family: Arial; display: inline-block; text-align: center;}
      h2 {font-size: 1.3rem;}
      p {font-size: 0.95rem;}
      table {border: 1px solid black; border-collapse: collapse; margin: 0px 0px;}
      td, th {font-size: 0.8rem; border: 1px solid gray; border-collapse: collapse;}
      td:hover {font-weight:bold;}
      a {color: black; text-decoration: none;}
      body {max-width: 1380px; margin:0px auto; padding-bottom: 25px; background: #D0D0D0;}
      .slider {-webkit-appearance: none; margin: 14px; width: 330px; height: 10px; background: #4CAF50; outline: none; -webkit-transition: .2s; transition: opacity .2s; border-radius: 5px;}
      .slider::-webkit-slider-thumb {-webkit-appearance: none; appearance: none; width: 35px; height: 25px; background: #4a4a4a; cursor: pointer; border-radius: 5px;}
      .slider::-moz-range-thumb { width: 35px; height: 35px; background: #4a4a4a; cursor: pointer; border-radius: 5px;} 
      .button { background-color: #4CAF50; border: 1; color: white; padding: 10px 20px; border-radius: 5px;}
      .buttonBank { background-color: #4CAF50; border: 1; color: white; padding: 8px 8px; border-radius: 5px; width: 35px; height: 35px; margin: 0 1.5px;}
      .buttonBankSelected { background-color: #505050; border: 1; color: white; padding: 8px 8px; border-radius: 5px; width: 35px; height: 35px; margin: 0 1.5px;}
      .buttonBank:active {background-color: #4a4a4a box-shadow: 0 4px #666; transform: translateY(2px);}
      .buttonBank:hover {background-color: #4a4a4a;}
      .button:hover {background-color: #4a4a4a;}
      .button:active {background-color: #4a4a4a; box-shadow: 0 4px #666; transform: translateY(2px);}
      .column { align: center; padding: 5px; display: flex; justify-content: space-between;}
      .columnlist { align: center; padding: 10px; display: flex; justify-content: center;}
      .stationList {text-align:left; margin-top: 0px; width: 280px; margin-bottom:0px;cursor: pointer;}
      .stationNumberList {text-align:center; margin-top: 0px; width: 35px; margin-bottom:0px;}
      .stationListSelected {text-align:left; margin-top: 0px; width: 280px; margin-bottom:0px;cursor: pointer; background-color: #4CAF50;}
      .stationNumberListSelected {text-align:center; margin-top: 0px; width: 35px; margin-bottom:0px; background-color: #4CAF50;}
      .station-name   
    </style>
  </head>

  <body>
    <h2>ESP32 Web Radio</h2>
  
    <div id="display" style="display: inline-block; padding: 5px; border: 2px solid #4CAF50; border-radius: 15px; background-color: #4a4a4a; font-size: 1.45rem; 
      color: #AAA; width: 345px; text-align: center; white-space: nowrap; box-shadow: 0 0 20px #4CAF50;" onClick="flashBackground(); displayMode()">
      
      <div style="margin-bottom: 10px; font-weight: bold; overflow: hidden; text-overflow: ellipsis; -webkit-text-stroke: 0.3px black; text-stroke: 0.3px black;">
        <span id="textStationName"><b>STATIONNAME</b></span>
      </div>
      
      <div style="width: 345px; margin-bottom: 10px;">
        <div id="stationTextDiv" style="display: block; text-overflow: ellipsis; white-space: normal; font-size: 1.0rem; color: #999; margin-bottom: 10px; text-align: center; align-items: center; height: 4.2em; justify-content: center; overflow: hidden; line-height: 1.4em;">
          <span id="stationText">STATIONTEXT</span>
        </div>
      </div>
      
      <div style="height: 1px; background-color: #4CAF50; margin: 5px 0;"></div>

      
      <div style="display: flex; justify-content: center; gap: 25px; font-size: 1.0rem; color: #999;">
        <div><span id="bankValue">Bank: --</span></div>
      
        <div style="display: flex; justify-content: center; gap: 10px; font-size: 0.65rem; color: #999;margin: 3px 0;">
          <div><span id="samplerate">---.-kHz</span></div>
          <div><span id="bitrate">---bit</span></div>
          <div><span id="bitpersample">---kbps</span></div>
          <div><span id="streamformat">----</span></div>
        </div>
        
        <div><span id="stationNumber">Station: --</span></div>
      </div>

  </div>
  <br>
  <br>
  <script>

  var websocket;

  function updateSliderVolume(element) 
  {
    var sliderValue = document.getElementById("volumeSlider").value;
    document.getElementById("textSliderValue").innerText = sliderValue;

    if (websocket && websocket.readyState === WebSocket.OPEN) 
    {
      websocket.send("volume:" + sliderValue);
    } 
    else 
    {
      console.warn("WebSocket niepołączony");
    }
  }

  function changeStation(number) 
  {
    if (websocket.readyState === WebSocket.OPEN) 
    {
      websocket.send("station:" + number);
    } 
    else 
    {
      console.log("WebSocket nie jest otwarty");
    }
  }

  function changeBank(number) 
  {
    if (websocket.readyState === WebSocket.OPEN) 
    {
      websocket.send("bank:" + number);
    } 
    else 
    {
      console.log("WebSocket nie jest otwarty");
    }
  }


  function displayMode() 
  {
    //fetch("/displayMode")
    if (websocket.readyState === WebSocket.OPEN) 
    {
      websocket.send("displayMode");
    } 
    else 
    {
      console.log("WebSocket nie jest otwarty");
    }
  }
  
  
  function connectWebSocket() 
  {
    websocket = new WebSocket('ws://' + window.location.hostname + '/ws');
    websocket.onopen = function () 
    {
      console.log("WebSocket polaczony");
    };
    
    websocket.onclose = function (event) 
    {
      console.log("WebSocket zamkniety. Proba ponownego polaczenia za 3 sekundy...");
      setTimeout(connectWebSocket, 3000); // próba ponownego połączenia
    };

    websocket.onerror = function (error) 
    {
      console.error("Blad WebSocket: ", error);
      websocket.close(); // zamyka połączenie, by wywołać reconnect
    };
    
    websocket.onmessage = function(event) 
    {
      if (event.data === "reload") 
      {
        location.reload();
      }  
      
      if (event.data.startsWith("volume:")) 
      {
        var vol = parseInt(event.data.split(":")[1]);
        document.getElementById("volumeSlider").value = vol;
        document.getElementById("textSliderValue").innerText = vol;
      }
      
      if (event.data.startsWith("station:")) 
      {
        var station = parseInt(event.data.split(":")[1]);
        highlightStation(station);
        //document.getElementById('stationNumber').innerText = event.data.split(':')[1];
        document.getElementById('stationNumber').innerText ='Station: ' + station; 
      }
      
      if (event.data.startsWith("stationname:")) 
      {
        var value = event.data.split(":")[1];
        document.getElementById("textStationName").innerHTML = `<b>${value}</b>`;
        //checkStationTextLength();
      }  

      if (event.data.startsWith("stationtext$")) 
      {
        var stationtext = event.data.split("$")[1];
        document.getElementById("stationText").innerHTML = `${stationtext}`;
        //checkStationTextLength();
      }  

      if (event.data.startsWith("bank:")) 
      {
        var bankValue = parseInt(event.data.split(":")[1]);
        document.getElementById('bankValue').innerText = 'Bank: ' + bankValue;
      } 

      if (event.data.startsWith("samplerate:")) 
      {
        var samplerate = parseInt(event.data.split(":")[1]);
        var formattedRate = (samplerate / 1000).toFixed(1) + "kHz";
        document.getElementById('samplerate').innerText = formattedRate;
      }
      
      if (event.data.startsWith("bitrate:")) 
      {	
        var bitrate = parseInt(event.data.split(":")[1]);
        document.getElementById('bitrate').innerText = bitrate + 'kbps';
      }  
      
      if (event.data.startsWith("bitpersample:")) 
      {	
        var bitpersample = parseInt(event.data.split(":")[1]);
        document.getElementById('bitpersample').innerText = bitpersample + 'bit';
      }  
      
      if (event.data.startsWith("streamformat:")) 
      {	
        var streamformat = event.data.split(":")[1];
        document.getElementById('streamformat').innerText = streamformat;
      }  

    }

  };
  
  function highlightStation(stationId) 
  {
    // Usuń poprzednie zaznaczenia
    document.querySelectorAll(".stationList").forEach(el => {
    el.classList.remove("stationListSelected");
    el.innerHTML = el.dataset.stationName || el.innerText; // przywróć oryginalny numer
    });

    document.querySelectorAll(".stationNumberList").forEach(el => {
      el.classList.remove("stationNumberListSelected");
      el.innerHTML = el.dataset.stationNumber || el.innerText; // przywróć oryginalny numer
    });

    // Zaznacz nową stację
    const numCells = document.querySelectorAll(".stationNumberList");
    const stationCells = document.querySelectorAll(".stationList");

    const numCell = numCells[stationId - 1];
    const stationCell = stationCells[stationId - 1];

    if (numCell && stationCell) 
    {
      numCell.classList.add("stationNumberListSelected");
      stationCell.classList.add("stationListSelected");
      
      // Pogrub nazwę stacji
      stationCell.dataset.stationName = stationCell.innerText; // zapisz oryginalny tekst
      stationCell.innerHTML = `<b>${stationCell.innerText}</b>`; // pogrubienie nazwy stacji
        
      // Pogrub numer
      numCell.dataset.stationNumber = numCell.innerText; // zapisz oryginalny numer
      numCell.innerHTML = `<b>${numCell.innerText}</b>`; 
    }
  }
  
  function flashBackground() 
	{
	  const div = document.getElementById('display');
	  const originalColor = div.style.backgroundColor;

	  const textSpan = document.getElementById('textStationName');
    const originalText = textSpan.innerText;


	  div.style.backgroundColor = '#115522'; // kolor flash
	  //textSpan.innerText = 'OLED Display Mode Changed';
    textSpan.innerHTML = '<b>Display Mode Changed</b>';  

	  setTimeout(() => 
	  {
		  div.style.backgroundColor = originalColor;
		  //textSpan.innerText = originalText;
      textSpan.innerHTML = `<b>${originalText}</b>`;
	  }, 150); // czas w ms flasha
	}


  document.addEventListener("DOMContentLoaded", function () 
  {
    connectWebSocket(); // podlaczamy websockety

    const slider = document.getElementById("volumeSlider");
    slider.addEventListener("wheel", function (event) 
    {
      event.preventDefault(); // zapobiega przewijaniu strony

      let currentValue = parseInt(slider.value);
      const step = parseInt(slider.step) || 1;
      const max = parseInt(slider.max);
      const min = parseInt(slider.min);

      if (event.deltaY < 0) {
        // przewijanie w górę (zwiększ)
        slider.value = Math.min(currentValue + step, max);
      } else {
        // przewijanie w dół (zmniejsz)
        slider.value = Math.max(currentValue - step, min);
      }

      updateSliderVolume(slider); // wywołaj aktualizację
    });
  });

  window.onpageshow = function(event) 
  {
    if (event.persisted) 
    {
      window.location.reload();
    }
  };

 </script>

)rawliteral";

const char list_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <link rel='icon' href='/favicon.ico' type='image/x-icon'>
  <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
  <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
  <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Web Radio</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem;}
    p {font-size: 1.1rem;}
    table {border: 1px solid black; border-collapse: collapse; margin: 0px 0px;}
    td, th {font-size: 0.8rem; border: 1px solid gray; border-collapse: collapse;}
    td:hover {font-weight:bold;}
    a {color: black; text-decoration: none;}
    body {max-width: 1380px; margin:0px auto; padding-bottom: 25px;}
    .columnlist { align: center; padding: 10px; display: flex; justify-content: center;}
  </style>
</head>
)rawliteral";

const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <link rel='icon' href='/favicon.ico' type='image/x-icon'>
  <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
  <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
  <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Web Radio</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h1 {font-size: 2.3rem;}
    table {border: 1px solid black; border-collapse: collapse; margin: 20px auto; width: 80%;}
    th, td {font-size: 1rem; border: 1px solid gray; padding: 8px; text-align: left;}
    td:hover {font-weight: bold;}
    a {color: black; text-decoration: none;}
    body {max-width: 1380px; margin:0 auto; padding-bottom: 25px;}
    .tableSettings {border: 2px solid #4CAF50; border-collapse: collapse; margin: 10px auto; width: 60%;}
  </style>
  </head>

<body>
<h2>ESP32 Radio - Settings</h2>
<form action="/configupdate" method="POST">
<table class="tableSettings">
<tr><th>Setting</th><th>Value</th></tr>
<tr><td>Normal Display Brightness (0-255)</td><td><input type="number" name="displayBrightness" min="1" max="255" value="%D1"></td></tr>
<tr><td>Dimmed Display Brightness (0-255)</td><td><input type="number" name="dimmerDisplayBrightness" min="1" max="255" value="%D2"></td></tr>
<tr><td>Auto Dimmer Delay Time (1-255 sec.)</td><td><input type="number" name="displayAutoDimmerTime" min="1" max="255" value="%D3"></td></tr>
<tr><td>Auto Dimmer</td><td><select name="displayAutoDimmerOn"><option value="1"%S1>On</option><option value="0"%S2>Off</option></select></td></tr>
<tr><td>Time Voice Info Every Hour</td><td><select name="timeVoiceInfoEveryHour"><option value="1"%S3>On</option><option value="0"%S4>Off</option></select></td></tr>
<tr><td>VU Meter Mode (0-1),    0-dashed lines, 1-continuous lines</td><td><input type="number" name="vuMeterMode" min="0" max="1" value="%D4"></td></tr>
<tr><td>Encoder Function Order (0-1),   0-Volume, click for station list, 1-Station list, click for Volume</td><td><input type="number" name="encoderFunctionOrder" min="0" max="1" value="%D5"></td></tr>
<tr><td>Display Mode (0-2),   0-Radio scroller, 1-Clock, 2-Three lines without scroll</td><td><input type="number" name="displayMode" min="0" max="2" value="%D6"></td></tr>

<tr><td>VU Meter Visible (Mode 0 only)</td><td><select name="vuMeterOn"><option value="1"%S5>On</option><option value="0"%S6>Off</option></select></td></tr>

<!-- <tr><td>VU Meter Refresh Time (20-100ms)</td><td><input type="number" name="vuMeterRefreshTime" min="15" max="100" value="%D7"></td></tr> -->

<tr><td>Radio Scroller & VU Meter Refresh Time (15-100ms)</td><td><input type="number" name="scrollingRefresh" min="15" max="100" value="%D8"></td></tr>
<tr><td>ADC Keyboard Enabled</td><td><select name="adcKeyboardEnabled"><option value="1"%S7>On</option><option value="0"%S8>Off</option></select></td></tr>

<tr><td>OLED Power Save Mode</td><td><select name="displayPowerSaveEnabled"><option value="1"%S9>On</option><option value="0"%S10>Off</option></select></td></tr>
<tr><td>OLED Power Save Time (1-600sek.)</td><td><input type="number" name="displayPowerSaveTime" min="1" max="600" value="%D9"></td></tr>
<tr><td>Max Volume Extended range -> 1-21 [Off], 1-42 [On]</td><td><select name="maxVolumeExt"><option value="1"%11>On</option><option value="0"%S12>Off</option></select></td></tr>
</table>
<input type="submit" value="Update">
</form>
<p style='font-size: 0.8rem;'><a href='/menu'>Go Back</a></p>
</body>
</html>
)rawliteral";

const char adc_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <link rel='icon' href='/favicon.ico' type='image/x-icon'>
  <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
  <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
  <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Web Radio</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h1 {font-size: 2.3rem;}
    table {border: 1px solid black; border-collapse: collapse; margin: 10px auto; width: 40%;}
    th, td {font-size: 1rem; border: 1px solid gray; padding: 8px; text-align: left;}
    td:hover {font-weight: bold;}
    a {color: black; text-decoration: none;}
    body {max-width: 1380px; margin:0 auto; padding-bottom: 15px;}
    .tableSettings {border: 2px solid #4CAF50; border-collapse: collapse; margin: 10px auto; width: 40%;}
  </style>
  </head>

<body>
<h2>ESP32 Radio - ADC Settings</h2>
<form action="/configadc" method="POST">
<table class="tableSettings">
<tr><th>Button</th><th>Value</th></tr>
<tr><td>keyboardButtonThreshold_0</td><td><input type="number" name="keyboardButtonThreshold_0" min="0" max="4095" value="%D0"></td></tr>
<tr><td>keyboardButtonThreshold_1</td><td><input type="number" name="keyboardButtonThreshold_1" min="0" max="4095" value="%D1"></td></tr>
<tr><td>keyboardButtonThreshold_2</td><td><input type="number" name="keyboardButtonThreshold_2" min="0" max="4095" value="%D2"></td></tr>
<tr><td>keyboardButtonThreshold_3</td><td><input type="number" name="keyboardButtonThreshold_3" min="0" max="4095" value="%D3"></td></tr>
<tr><td>keyboardButtonThreshold_4</td><td><input type="number" name="keyboardButtonThreshold_4" min="0" max="4095" value="%D4"></td></tr>
<tr><td>keyboardButtonThreshold_5</td><td><input type="number" name="keyboardButtonThreshold_5" min="0" max="4095" value="%D5"></td></tr>
<tr><td>keyboardButtonThreshold_6</td><td><input type="number" name="keyboardButtonThreshold_6" min="0" max="4095" value="%D6"></td></tr>
<tr><td>keyboardButtonThreshold_7</td><td><input type="number" name="keyboardButtonThreshold_7" min="0" max="4095" value="%D7"></td></tr>
<tr><td>keyboardButtonThreshold_8</td><td><input type="number" name="keyboardButtonThreshold_8" min="0" max="4095" value="%D8"></td></tr>
<tr><td>keyboardButtonThreshold_9</td><td><input type="number" name="keyboardButtonThreshold_9" min="0" max="4095" value="%D9"></td></tr>
<tr><td>keyboardButtonThreshold_Shift - Ok/Enter</td><td><input type="number" name="keyboardButtonThreshold_Shift" min="0" max="4095" value="%D10"></td></tr>
<tr><td>keyboardButtonThreshold_Memory - Bank Menu</td><td><input type="number" name="keyboardButtonThreshold_Memory" min="0" max="4095" value="%D11"></td></tr>
<tr><td>keyboardButtonThreshold_Band -  Back</td><td><input type="number" name="keyboardButtonThreshold_Band" min="0" max="4095" value="%D12"></td></tr>
<tr><td>keyboardButtonThreshold_Auto -  Display Mode</td><td><input type="number" name="keyboardButtonThreshold_Auto" min="0" max="4095" value="%D13"></td></tr>
<tr><td>keyboardButtonThreshold_Scan - Dimmer/Network Bank Update</td><td><input type="number" name="keyboardButtonThreshold_Scan" min="0" max="4095" value="%D14"></td></tr>
<tr><td>keyboardButtonThreshold_Mute - Mute</td><td><input type="number" name="keyboardButtonThreshold_Mute" min="0" max="4095" value="%D15"></td></tr>
<tr><td>keyboardButtonThresholdTolerance</td><td><input type="number" name="keyboardButtonThresholdTolerance" min="0" max="50" value="%D16"></td></tr>
<tr><td>keyboardButtonNeutral</td><td><input type="number" name="keyboardButtonNeutral" min="0" max="4095" value="%D17"></td></tr>
<tr><td>keyboardSampleDelay (30-300ms)</td><td><input type="number" name="keyboardSampleDelay" min="30" max="300" value="%D18"></td></tr>
</table>
<input type="submit" value="ADC Thresholds Update">
</form>
<br>
<button onclick="toggleAdcDebug()">ADC Debug ON/OFF</button>
<script>
function toggleAdcDebug() {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/toggleAdcDebug", true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
    xhr.onload = function() {
        if (xhr.status === 200) {
            alert("ADC Debug is now " + (xhr.responseText === "1" ? "ON" : "OFF"));
        } else {
            alert("Error: " + xhr.statusText);
        }
    };
    xhr.send(); // Wysyłanie pustego zapytania POST
}
</script>


<p style='font-size: 0.8rem;'><a href='/menu'>Go Back</a></p>
</body>
</html>
)rawliteral";

const char menu_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
  <link rel='icon' href='/favicon.ico' type='image/x-icon'>
  <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
  <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
  <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Web Radio</title>
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 1.3rem;}
    p {font-size: 1.1rem;}
    a {color: black; text-decoration: none;}
    body {max-width: 1380px; margin:0px auto; padding-bottom: 25px;}
    .button { background-color: #4CAF50; border: 1; color: white; padding: 10px 20px; border-radius: 5px; width:200px;}
    .button:hover {background-color: #4a4a4a;}
    .button:active {background-color: #4a4a4a; box-shadow: 0 4px #666; transform: translateY(2px);}
    
  </style>
  </head>
  <body>
  <h2>ESP32 Web Radio - Menu</h2>
  <!-- <br><button class="button" onclick="location.href='/fwupdate'">OTA Update (Old)</button><br> -->
  <br><button class="button" onclick="location.href='/info'">Info</button><br>
  <br><button class="button" onclick="location.href='/ota'">OTA Update</button><br>
  <br><button class="button" onclick="location.href='/adc'">ADC Keyboard Settings</button><br>
  <br><button class="button" onclick="location.href='/list'">SPIFFS Explorer</button><br>
  <br><button class="button" onclick="location.href='/editor'">Memory Bank Editor</button><br>
  <br><button class="button" onclick="location.href='/config'">Configuration</button><br>
  <br><p style='font-size: 0.8rem;'><a href="#" onclick="window.location.replace('/')">Go Back</a></p>
  </body></html>

)rawliteral";

const char info_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
  <html>
  <head>
    <link rel='icon' href='/favicon.ico' type='image/x-icon'>
    <link rel="shortcut icon" href="/favicon.ico" type="image/x-icon">
    <link rel="apple-touch-icon" sizes="180x180" href="/icon.png">
    <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">

    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Web Radio</title>
    <style>
      html {font-family: Arial; display: inline-block; text-align: center;}
      h2 {font-size: 1.7rem;}
      table {border: 1px solid black; border-collapse: collapse; margin: 10px auto; width: 40%;}
      th, td {font-size: 1rem; border: 1px solid gray; padding: 8px; text-align: left;}
      td:hover {font-weight: bold;}
      a {color: black; text-decoration: none;}
	  
      body {max-width: 1380px; margin:0 auto; padding-bottom: 15px;}
      .tableSettings {border: 2px solid #4CAF50; border-collapse: collapse; margin: 10px auto; width: 40%;} 
	  .signal-bars {display: inline-block; vertical-align: middle; margin-left: 10px;}
	  .bar {display: inline-block; width: 5px; margin-right: 2px; background-color: #F2F2F2; height: 10px;}
      .bar.active { background-color: #4CAF50;}
	  
    </style>
    </head>

  <body>
  <h2>ESP32 Radio - Info</h2>
  <form action="/configadc" method="POST">
  <table class="tableSettings">

  <tr><td>ESP Serial Number:</td><td><input name="espSerial" value="%D0"></td></tr>
  <tr><td>Firmware version:</td><td><input name="espFw" value="%D1"></td></tr>
  <tr><td>Hostname:</td><td><input name="hostnameValue" value="%D2"></td></tr>
  <tr><td>WiFi Signal Strength:</td><td><input id="wifiSignal" value="%D3"> dBm
  <div class="signal-bars" id="signalBars">
      <div class="bar" style="height:2px;"></div>
      <div class="bar" style="height:6px;"></div>
      <div class="bar" style="height:10px;"></div>
      <div class="bar" style="height:14px;"></div>
      <div class="bar" style="height:18px;"></div>
      <div class="bar" style="height:22px;"></div>
    </div>


  </td></tr>
  <tr><td>WiFi SSID:</td><td><input name="wifiSsid" value="%D4"></td></tr>
  <tr><td>IP number:</td><td><input name="ipValue" value="%D5"></td></tr>
  <tr><td>MAC Address:</td><td><input name="macValue" value="%D6"></td></tr>
  
  </table>
  </form>
  <br>
  <p style='font-size: 0.8rem;'><a href='/menu'>Go Back</a></p>
  
<script>
  function updateSignalBars(signal) {
    const bars = document.querySelectorAll('#signalBars .bar');
    let level = 0;

    signal = parseInt(signal);

    if (signal >= -50) level = 6;
    else if (signal >= -57) level = 5;
    else if (signal >= -66) level = 4;
    else if (signal >= -74) level = 3;
    else if (signal >= -81) level = 2;
    else if (signal >= -88) level = 1;
    else level = 0;

    bars.forEach((bar, index) => {
      if (index < level) {
        bar.classList.add('active');
      } else {
        bar.classList.remove('active');
      }
    });
  }

  const signalInput = document.getElementById('wifiSignal');
  updateSignalBars(signalInput.value);

 </script>
  </body>
  </html>
)rawliteral";



char stations[MAX_STATIONS][STATION_NAME_LENGTH + 1];  // Tablica przechowująca linki do stacji radiowych (jedna na stację) +1 dla terminatora null

const char *ntpServer1 = "pool.ntp.org";  // Adres serwera NTP używany do synchronizacji czasu
const char *ntpServer2 = "time.nist.gov"; // Adres serwera NTP używany do synchronizacji czasu
const long gmtOffset_sec = 3600;          // Przesunięcie czasu UTC w sekundach
const int daylightOffset_sec = 3600;      // Przesunięcie czasu letniego w sekundach, dla Polski to 1 godzina


// Ikona karty SPIFFS wyswietlana przy braku karty podczas startu
static unsigned char sdcard[] PROGMEM = {
  0xf0, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x1f, 0xf8, 0xcf, 0xf3, 0x3f,
  0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c,
  0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c, 0x38, 0x49, 0x92, 0x3c,
  0x38, 0x49, 0x92, 0x3c, 0xf8, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f,
  0xf8, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x3f,
  0xfc, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f,
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f,
  0xfc, 0xff, 0xff, 0x3f, 0xfc, 0xc0, 0x80, 0x3f, 0x7c, 0xc0, 0x00, 0x3f,
  0x3c, 0xc0, 0x00, 0x3e, 0x1c, 0xfc, 0x38, 0x3e, 0x1e, 0xfc, 0x78, 0x3c,
  0x1f, 0xe0, 0x78, 0x3c, 0x3f, 0xc0, 0x78, 0x3c, 0x7f, 0x80, 0x78, 0x3c,
  0xff, 0x87, 0x78, 0x3c, 0xff, 0x87, 0x38, 0x3e, 0x3f, 0x80, 0x00, 0x3e,
  0x1f, 0xc0, 0x00, 0x3f, 0x3f, 0xe0, 0x80, 0x3f, 0xff, 0xff, 0xff, 0x3f,
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x1f,
  0xfc, 0xff, 0xff, 0x0f 
  };

// Obrazek nutek wyswietlany przy starcie
#define notes_width 256
#define notes_height 46
static unsigned char notes[] PROGMEM = {
  0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x40, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0e, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x20, 0x07, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x07, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x80, 0x05, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
  0x00, 0xf0, 0x01, 0xc0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x40, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x40,
  0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x08,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x40, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x5e, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
  0x60, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x5e, 0x00, 0x80, 0x08, 0x00, 0x00, 0x07, 0x00, 0xe0, 0x01, 0x00,
  0xc0, 0x47, 0x00, 0x08, 0x00, 0x00, 0x07, 0x00, 0xe0, 0x01, 0x00, 0x00,
  0x40, 0x00, 0x18, 0x00, 0x80, 0x00, 0x00, 0x0e, 0x00, 0x4e, 0x00, 0x00,
  0x19, 0x00, 0xf0, 0x07, 0x00, 0x20, 0x01, 0x00, 0xf8, 0x41, 0x00, 0x18,
  0x00, 0xf0, 0x07, 0x00, 0x20, 0x01, 0x00, 0x18, 0x40, 0x00, 0x78, 0x00,
  0x80, 0x00, 0xe0, 0x0f, 0x00, 0x47, 0x00, 0x00, 0x10, 0x00, 0xf0, 0x07,
  0x00, 0x20, 0x03, 0x00, 0x3c, 0x40, 0x00, 0x10, 0x00, 0xf0, 0x07, 0x00,
  0x20, 0x03, 0x00, 0x1c, 0x40, 0x00, 0xe8, 0x00, 0x80, 0x00, 0xe0, 0x0f,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x41, 0x00, 0x00,
  0x10, 0x00, 0x3f, 0x04, 0x00, 0xe0, 0x04, 0xe0, 0x11, 0x40, 0x00, 0x10,
  0x00, 0x3f, 0x04, 0x00, 0xe0, 0x04, 0xe0, 0x11, 0x40, 0x00, 0x38, 0x01,
  0x40, 0x00, 0x7e, 0x08, 0xc0, 0xe0, 0x0f, 0x00, 0x10, 0x00, 0x0f, 0x04,
  0x00, 0xb0, 0x01, 0x78, 0x10, 0x40, 0x00, 0x10, 0x00, 0x0f, 0x04, 0x00,
  0xb0, 0x01, 0x78, 0x10, 0x40, 0x00, 0x68, 0x01, 0x40, 0x00, 0x1e, 0x08,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x70, 0x3e, 0x00,
  0x1c, 0x00, 0x01, 0x04, 0x00, 0x10, 0x00, 0x0e, 0x10, 0x40, 0x00, 0x18,
  0x00, 0x01, 0x04, 0x00, 0x10, 0x00, 0x0e, 0x10, 0x40, 0x00, 0xa8, 0x00,
  0x2f, 0x00, 0x02, 0x08, 0x40, 0x98, 0x78, 0x00, 0x1f, 0x00, 0x01, 0x04,
  0x00, 0x10, 0x00, 0x06, 0x10, 0x40, 0x00, 0x1f, 0x00, 0x01, 0x04, 0x00,
  0x10, 0x00, 0x06, 0x10, 0x40, 0x00, 0x48, 0xc0, 0x1f, 0x00, 0x02, 0x08,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x40, 0x88, 0x70, 0x80,
  0x0f, 0x00, 0x01, 0x04, 0x80, 0x17, 0x00, 0x02, 0x10, 0x7c, 0x80, 0x0f,
  0x00, 0x01, 0x04, 0x80, 0x17, 0x00, 0x02, 0x10, 0x7c, 0x00, 0x08, 0x80,
  0x0f, 0x00, 0x02, 0x08, 0x40, 0x88, 0x70, 0x00, 0x07, 0x00, 0x01, 0x04,
  0xc0, 0x1f, 0x00, 0x02, 0x10, 0x7e, 0x00, 0x07, 0x00, 0x01, 0x04, 0xc0,
  0x1f, 0x00, 0x02, 0x10, 0x7e, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x02, 0x08,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x71, 0x00,
  0x00, 0x00, 0xc1, 0x07, 0xe0, 0x07, 0x00, 0x02, 0x1e, 0x00, 0x00, 0x00,
  0x00, 0xc1, 0x07, 0xe0, 0x07, 0x00, 0x02, 0x1e, 0x00, 0xc0, 0x07, 0x00,
  0x00, 0x00, 0x82, 0x0f, 0x80, 0x01, 0x39, 0x00, 0x00, 0x00, 0xe1, 0x07,
  0xc0, 0x03, 0x00, 0x02, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xe1, 0x07, 0xc0,
  0x03, 0x00, 0x02, 0x1f, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0xc2, 0x0f,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0x1e, 0x00,
  0x00, 0xf0, 0x81, 0x03, 0x00, 0x00, 0x80, 0x03, 0x0e, 0x00, 0x00, 0x00,
  0xf0, 0x81, 0x03, 0x00, 0x00, 0x80, 0x03, 0x0e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x03, 0x07, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0xf8, 0x01, 0x00,
  0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x03, 0x00,
  0x00, 0xf0, 0x03, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03,
  0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00,
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00,
  0x70, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x02, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x0c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x01, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Funkcja odwracania bitów MSL-LSB <-> LSB-MSB
uint32_t reverse_bits(uint32_t inval, int bits)
{
if ( bits > 0 )
{
bits--;
return reverse_bits(inval >> 1, bits) | ((inval & 1) << bits);
}
return 0;
}

/*---------- Definicja portu i deklaracje zmiennych do obsługi odbiornika IR ----------*/

// Zmienne obsługi  pilota IR
bool pulse_ready = false;                // Flaga sygnału gotowości
unsigned long pulse_start_high = 0;      // Czas początkowy impulsu
unsigned long pulse_end_high = 0;        // Czas końcowy impulsu
unsigned long pulse_duration = 0;        // Czas trwania impulsu
unsigned long pulse_duration_9ms = 0;    // Tylko do analizy - Czas trwania impulsu
unsigned long pulse_duration_4_5ms = 0;  // Tylko do analizy - Czas trwania impulsu
unsigned long pulse_duration_560us = 0;  // Tylko do analizy - Czas trwania impulsu
unsigned long pulse_duration_1690us = 0; // Tylko do analizy - Czas trwania impulsu

bool pulse_ready9ms = false;            // Flaga sygnału gotowości puls 9ms
bool pulse_ready_low = false;           // Flaga sygnału gotowości 
unsigned long pulse_start_low = 0;      // Czas początkowy impulsu
unsigned long pulse_end_low = 0;        // Czas końcowy impulsu
unsigned long pulse_duration_low = 0;   // Czas trwania impulsu


unsigned long ir_code = 0;  // Zmienna do przechowywania kodu IR
int bit_count = 0;          // Licznik bitów w odebranym kodzie

// Progi przełączeń dla sygnałów czasowych pilota IR
const int LEAD_HIGH = 9050;         // 9 ms sygnał wysoki (początkowy)
const int LEAD_LOW = 4500;          // 4,5 ms sygnał niski (początkowy)
const int TOLERANCE = 150;          // Tolerancja (w mikrosekundach)
const int HIGH_THRESHOLD = 1690;    // Sygnał "1"
const int LOW_THRESHOLD = 600;      // Sygnał "0"

bool data_start_detected = false;  // Flaga dla sygnału wstępnego
bool rcInputDigitsMenuEnable = false;

//================ Definicja portów i pinów dla klaiwatury numerycznej ===========================//

const int keyboardPin = 9;  // wejscie klawiatury (ADC)

unsigned long keyboardValue = 0;
unsigned long keyboardLastSampleTime = 0;
unsigned long keyboardSampleDelay = 50;


// ---- Progi przełaczania ADC dla klawiatury matrycowej 5x3 w tunrze Sony ST-120 ---- //
int keyboardButtonThresholdTolerance = 20; // Tolerancja dla pomiaru ADC
int keyboardButtonNeutral = 4095;          // Pozycja neutralna
int keyboardButtonThreshold_0 = 2375;      // Przycisk 0
int keyboardButtonThreshold_1 = 10;        // Przycisk 1
int keyboardButtonThreshold_2 = 545;       // Przycisk 2
int keyboardButtonThreshold_3 = 1390;      // Przycisk 3
int keyboardButtonThreshold_4 = 1925;      // Przycisk 4
int keyboardButtonThreshold_5 = 2285;      // Przycisk 5
int keyboardButtonThreshold_6 = 385;       // Przycisk 6
int keyboardButtonThreshold_7 = 875;       // Przycisk 7
int keyboardButtonThreshold_8 = 1585;      // Przycisk 8
int keyboardButtonThreshold_9 = 2055;      // Przycisk 9
int keyboardButtonThreshold_Shift = 2455;  // Shift - funkcja Enter/OK
int keyboardButtonThreshold_Memory = 2170; // Memory - funkcja Bank menu
int keyboardButtonThreshold_Band = 1640;   // Przycisk Band - funkcja Back
int keyboardButtonThreshold_Auto = 730;    // Przycisk Auto - przelacza Radio/Zegar
int keyboardButtonThreshold_Scan = 1760;   // Przycisk Scan - funkcja Dimmer ekranu OLED
int keyboardButtonThreshold_Mute = 1130;   // Przycisk Mute - funkcja MUTE

// Flagi do monitorowania stanu klawiatury
bool keyboardButtonPressed = false; // Wcisnięcie klawisza
bool debugKeyboard = false;         // Wyłącza wywoływanie funkcji i zostawia tylko wydruk pomiaru ADC
bool adcKeyboardEnabled = false;    // Flaga właczajaca działanie klawiatury ADC


String checkUTF8(const String& input) 
{
  String output = "";
  const char* str = input.c_str();
  int len = input.length();
  int i = 0;

  while (i < len) {
    unsigned char c = str[i];

    // 1-byte ASCII
    if (c <= 0x7F) {
      output += (char)c;
      i++;
    }
    // 2-byte UTF-8
    else if ((c & 0xE0) == 0xC0 && i + 1 < len &&
             (str[i + 1] & 0xC0) == 0x80) {
      output += str[i];
      output += str[i + 1];
      i += 2;
    }
    // 3-byte UTF-8
    else if ((c & 0xF0) == 0xE0 && i + 2 < len &&
             (str[i + 1] & 0xC0) == 0x80 &&
             (str[i + 2] & 0xC0) == 0x80) {
      output += str[i];
      output += str[i + 1];
      output += str[i + 2];
      i += 3;
    }
    // 4-byte UTF-8
    else if ((c & 0xF8) == 0xF0 && i + 3 < len &&
             (str[i + 1] & 0xC0) == 0x80 &&
             (str[i + 2] & 0xC0) == 0x80 &&
             (str[i + 3] & 0xC0) == 0x80) {
      output += str[i];
      output += str[i + 1];
      output += str[i + 2];
      output += str[i + 3];
      i += 4;
    }
    // Invalid byte or incomplete sequence
    else {
      output += '?';
      i++;
    }
  }

  return output;
}

bool isValidUtf8(const String& str) {
  const unsigned char* bytes = (const unsigned char*)str.c_str();

  while (*bytes) {
    if (*bytes <= 0x7F) {
      // ASCII (0xxxxxxx)
      bytes++;
    } else if ((*bytes & 0xE0) == 0xC0) {
      // 2-byte sequence (110xxxxx 10xxxxxx)
      if ((bytes[1] & 0xC0) != 0x80) return false;
      bytes += 2;
    } else if ((*bytes & 0xF0) == 0xE0) {
      // 3-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
      if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) return false;
      bytes += 3;
    } else if ((*bytes & 0xF8) == 0xF0) {
      // 4-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
      if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) return false;
      bytes += 4;
    } else {
      return false; // Invalid leading byte
    }
  }

  return true;
}

void removeUtf8Bom(String &text) {
  if (text.length() >= 3 &&
      (uint8_t)text[0] == 0xEF &&
      (uint8_t)text[1] == 0xBB &&
      (uint8_t)text[2] == 0xBF) {
    text.remove(0, 3);  // usuń bajty EF BB BF
  }
}

// Zamiana niepolskich znaków (typu ê -> ę)
void transliterateToAscii(String &text) {

  removeUtf8Bom(text);  // usuń BOM, jeśli jest obecny

  text.replace("á", "a"); text.replace("à", "a"); text.replace("â", "a"); text.replace("ä", "a"); text.replace("ã", "a"); text.replace("å", "a");
  text.replace("Á", "A"); text.replace("À", "A"); text.replace("Â", "A"); text.replace("Ä", "A"); text.replace("Ã", "A"); text.replace("Å", "A");

  text.replace("é", "e"); text.replace("è", "e"); text.replace("ê", "e"); text.replace("ë", "e");
  text.replace("É", "E"); text.replace("È", "E"); text.replace("Ê", "E"); text.replace("Ë", "E");

  text.replace("í", "i"); text.replace("ì", "i"); text.replace("î", "i"); text.replace("ï", "i");
  text.replace("Í", "I"); text.replace("Ì", "I"); text.replace("Î", "I"); text.replace("Ï", "I");

  text.replace("ò", "o"); text.replace("ô", "o"); text.replace("ö", "o"); text.replace("õ", "o"); text.replace("ø", "o");
  text.replace("Ò", "O"); text.replace("Ô", "O"); text.replace("Ö", "O"); text.replace("Õ", "O"); text.replace("Ø", "O");

  text.replace("ú", "u"); text.replace("ù", "u"); text.replace("û", "u"); text.replace("ü", "u");
  text.replace("Ú", "U"); text.replace("Ù", "U"); text.replace("Û", "U"); text.replace("Ü", "U");

  text.replace("ñ", "n"); text.replace("Ñ", "N");
  text.replace("ç", "c"); text.replace("Ç", "C");

  text.replace("ß", "ss");
  text.replace("ÿ", "y"); text.replace("Ÿ", "Y");

  text.replace("g̃", "g"); text.replace("G̃", "G");

  // Można też usunąć znaki nie-ASCII całkowicie, np.:
  // for (int i = 0; i < text.length(); ++i) {
  //   if ((unsigned char)text[i] > 127) text.remove(i--, 1);
  // }
}

// Funkcja przetwarza tekst, zamieniając polskie znaki diakrytyczne
void processText(String &text) 
{
  for (int i = 0; i < text.length() - 1; i++) {
    if (i + 1 >= text.length()) break; // zabezpieczenie przed wyjściem poza bufor

    switch ((uint8_t)text[i]) {
      case 0xC3:
        switch ((uint8_t)text[i + 1]) {
          case 0xB3: text.setCharAt(i, 0xF3); break;  // ó
          case 0x93: text.setCharAt(i, 0xD3); break;  // Ó
        }
        text.remove(i + 1, 1);
        break;

      case 0xC4:
        switch ((uint8_t)text[i + 1]) {
          case 0x85: text.setCharAt(i, 0xB9); break;  // ą
          case 0x84: text.setCharAt(i, 0xA5); break;  // Ą
          case 0x87: text.setCharAt(i, 0xE6); break;  // ć
          case 0x86: text.setCharAt(i, 0xC6); break;  // Ć
          case 0x99: text.setCharAt(i, 0xEA); break;  // ę
          case 0x98: text.setCharAt(i, 0xCA); break;  // Ę
        }
        text.remove(i + 1, 1);
        break;

      case 0xC5:
        switch ((uint8_t)text[i + 1]) {
          case 0x82: text.setCharAt(i, 0xB3); break;  // ł
          case 0x81: text.setCharAt(i, 0xA3); break;  // Ł
          case 0x83: text.setCharAt(i, 0xD1); break;  // Ń
          case 0x84: text.setCharAt(i, 0xF1); break;  // ń
          case 0x9B: text.setCharAt(i, 0x9C); break;  // ś
          case 0x9A: text.setCharAt(i, 0x8C); break;  // Ś
          case 0xBA: text.setCharAt(i, 0x9F); break;  // ź
          case 0xB9: text.setCharAt(i, 0x8F); break;  // Ź
          case 0xBC: text.setCharAt(i, 0xBF); break;  // ż
          case 0xBB: text.setCharAt(i, 0xAF); break;  // Ż
        }
        text.remove(i + 1, 1);
        break;
    }
  }
}

void win1250ToUtf8(String& input) {
  String output = "";
  const uint8_t* bytes = (const uint8_t*)input.c_str();

  while (*bytes) {
    uint8_t c = *bytes;
    switch (c) {
      case 0xA5: output += "Ą"; break;
      case 0xB9: output += "ą"; break;
      case 0xC6: output += "Ć"; break;
      case 0xE6: output += "ć"; break;
      case 0xCA: output += "Ę"; break;
      case 0xEA: output += "ę"; break;
      case 0xA3: output += "Ł"; break;
      case 0xB3: output += "ł"; break;
      case 0xD1: output += "Ń"; break;
      case 0xF1: output += "ń"; break;
      case 0xD3: output += "Ó"; break;
      case 0xF3: output += "ó"; break;
      case 0xA6: output += "Ś"; break;
      case 0xB6: output += "ś"; break;
      case 0xAF: output += "Ż"; break;
      case 0xBF: output += "ż"; break;
      case 0xAC: output += "Ź"; break;
      case 0xBC: output += "ź"; break;
      case 0x8C: output += "Ś"; break;
      case 0x9C: output += "ś"; break; 
      default:
        if (c < 0x80) {
          output += (char)c;
        } else {
          output += '?';
        }
        break;
    }
    bytes++;
  }

  input = output; // <-- modyfikujemy oryginalny String
}


void wsRefreshPage() // Funckja odswiezania strony za pomocą WebSocket
{
  ws.textAll("reload");  // wyślij komunikat do wszystkich klientów
}

void wsStationChange(uint8_t stationId) 
{
  ws.textAll("stationname:" + String(stationName.substring(0, stationNameLenghtCut)));
  ws.textAll("station:" + String(stationId));
  ws.textAll("bank:" + String(bank_nr));
  ws.textAll("volume:" + String(volumeValue)); 
  
  ws.textAll("bitrate:" + String(bitrateString)); 
  ws.textAll("samplerate:" + String(sampleRateString)); 
  ws.textAll("bitpersample:" + String(bitsPerSampleString)); 

  if (mp3 == true) {ws.textAll("streamformat:MP3"); }
  if (flac == true) {ws.textAll("streamformat:FLAC"); }
  if (aac == true) {ws.textAll("streamformat:AAC"); }
  if (vorbis == true) {ws.textAll("streamformat:VRB"); }
  if (opus == true) {ws.textAll("streamformat:OPUS"); }
}

void wsStreamInfoRefresh()
{ 
  if (audio.isRunning() == true)
  {  
    Serial.print("web debug-- ws.stationtext$: ");
    Serial.println(stationStringWeb);
    
    //checkUTF8(stationStringWeb);
   
    if (isValidUtf8(stationStringWeb)) 
    {
      Serial.println("stationStringWeb jest w UTF-8 - OK");
    } 
    else 
    {
      Serial.println("stationStringWeb NIE jest w UTF-8 — potrzebna konwersja!");
      win1250ToUtf8(stationStringWeb);    // Konwersja
    }
  

    Serial.print("web debug-- after check ws.stationtext$: ");
    Serial.println(stationStringWeb);
    
    ws.textAll("stationtext$" + stationStringWeb);  // znak podziału to $ aby uniknac problemow z adresami http: separatorem |. Jako znak separacji uzyty $
  }
  else
  {
    ws.textAll("stationtext$... No audio stream ! ...");
  }

  ws.textAll("bitrate:" + String(bitrateString)); 
  ws.textAll("samplerate:" + String(sampleRateString)); 
  ws.textAll("bitpersample:" + String(bitsPerSampleString)); 

  if (mp3 == true) {ws.textAll("streamformat:MP3"); }
  if (flac == true) {ws.textAll("streamformat:FLAC"); }
  if (aac == true) {ws.textAll("streamformat:AAC"); }
  if (vorbis == true) {ws.textAll("streamformat:VRB"); }
  if (opus == true) {ws.textAll("streamformat:OPUS"); }
}

void wsVolumeChange(int newVolume) 
{
  ws.textAll("volume:" + String(volumeValue)); // wysyła wartosc volume do wszystkich połączonych klientów
}


//Funkcja odpowiedzialna za zapisywanie informacji o stacji do pamięci EEPROM.
void saveStationToPSRAM(const char *station) 
{
  // Sprawdź, czy istnieje jeszcze miejsce na kolejną stację w pamięci EEPROM.
  if (stationsCount < MAX_STATIONS) {
    int length = strlen(station);

    // Sprawdź, czy długość linku nie przekracza ustalonego maksimum.
    if (length <= STATION_NAME_LENGTH) {
      // Zapisz długość linku jako pierwszy bajt.
      psramData[stationsCount * (STATION_NAME_LENGTH + 1)] = length;
      // Zapisz link jako kolejne bajty w pamięci EEPROM.
      for (int i = 0; i < length; i++) 
      {
        psramData[stationsCount * (STATION_NAME_LENGTH + 1) + 1 + i] = station[i];
      }

      // Potwierdź zapis do pamięci EEPROM.
      //EEPROM.commit();

      // Wydrukuj informację o zapisanej stacji na Serialu.
      Serial.println(String(stationsCount + 1) + "   " + String(station));  // Drukowanie na serialu od nr 1 jak w banku na serwerze

      // Zwiększ licznik zapisanych stacji.
      stationsCount++;

      u8g2.setFont(spleen6x12PL);  // progress bar pobieranych stacji
      u8g2.drawStr(21, 36, "Progress:");
      u8g2.drawStr(75, 36, String(stationsCount).c_str());  // Napisz licznik pobranych stacji

      u8g2.drawRFrame(21, 42, 212, 12, 3);  // Ramka paska postępu ladowania stacji stacji w>8 h>8
      x = (stationsCount * 2) + 8;          // Dodajemy gdy stationCount=1 + 8 aby utrzymac warunek dla zaokrąglonego drawRBox - szerokość W>6 h>6 ma byc W>=2*(r+1), h >= 2*(r+1)
      u8g2.drawRBox(23, 44, x, 8, 2);       // Pasek postepu ladowania stacji z serwera lub karty SPIFFS
      u8g2.sendBuffer();
    } else {
      // Informacja o błędzie w przypadku zbyt długiego linku do stacji.
      Serial.println("Błąd: Link do stacji jest zbyt długi");
    }
  } else {
    // Informacja o błędzie w przypadku osiągnięcia maksymalnej liczby stacji.
    Serial.println("Błąd: Osiągnięto maksymalną liczbę zapisanych stacji");
  }
}

// Funkcja przetwarza i zapisuje stację do pamięci EEPROM
void sanitizeAndSaveStation(const char *station) {
  // Bufor na przetworzoną stację - o jeden znak dłuższy niż maksymalna długość linku
  char sanitizedStation[STATION_NAME_LENGTH + 1];

  // Indeks pomocniczy dla przetwarzania
  int j = 0;

  // Przeglądaj każdy znak stacji i sprawdź czy jest to drukowalny znak ASCII
  for (int i = 0; i < STATION_NAME_LENGTH && station[i] != '\0'; i++) {
    // Sprawdź, czy znak jest drukowalnym znakiem ASCII
    if (isprint(station[i])) {
      // Jeśli tak, dodaj do przetworzonej stacji
      sanitizedStation[j++] = station[i];
    }
  }

  // Dodaj znak końca ciągu do przetworzonej stacji
  sanitizedStation[j] = '\0';

  // Zapisz przetworzoną stację do pamięci EEPROM
  saveStationToPSRAM(sanitizedStation);
}

// Jesli dany bank istnieje juz na karcie SPIFFS to odczytujemy tylko dany Bank z karty
void readSPIFFSStations() {
  stationsCount = 0;
  Serial.println("Plik Banku istnieje na karcie SPIFFS, czytamy TYLKO z karty");
  //mp3 = flac = aac = false;
   mp3 = flac = aac = vorbis = opus = false;
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString

  // Tworzymy nazwę pliku banku
  String fileName = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzamy, czy plik istnieje
  if (!SPIFFS.exists(fileName)) {
    Serial.println("Błąd: Plik banku nie istnieje.");
    return;
  }

  // Otwieramy plik w trybie do odczytu
  File bankFile = SPIFFS.open(fileName, FILE_READ);
  if (!bankFile)  // jesli brak pliku to...
  {
    Serial.println("Błąd: Nie można otworzyć pliku banku.");
    return;
  }

  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String stationUrl = "";

  while (bankFile.available())  // & currentLine <= MAX_STATIONS)
  {
    //if (currentLine < MAX_STATIONS)
    //{
    String line = bankFile.readStringUntil('\n');
    currentLine++;

    //currentLine == station_nr
    stationName = line.substring(0, 42);
    int urlStart = line.indexOf("http");  // Szukamy miejsca, gdzie zaczyna się URL
    if (urlStart != -1) {
      stationUrl = line.substring(urlStart);  // Wyciągamy URL od "http"
      stationUrl.trim();                      // Usuwamy białe znaki na początku i końcu
      ////Serial.print(" URL stacji:");
      ///Serial.println(stationUrl);
      //String station = currentLine + "   " + stationName + "  " + stationUrl;
      String station = stationName + "  " + stationUrl;
      sanitizeAndSaveStation(station.c_str());  // przepisanie stacji do EEPROMu  (RAMU)
    }
    //}
  }
  Serial.print("Zamykamy plik bankFile na wartosci currentLine:");
  Serial.println(currentLine);
  bankFile.close();  // Zamykamy plik po odczycie
}


// Funkcja do pobierania listy stacji radiowych z serwera
void fetchStationsFromServer() 
{
  displayActive = true;
  u8g2.setFont(spleen6x12PL);
  u8g2.clearBuffer();
  u8g2.setCursor(15, 23);
  u8g2.print("Loading BANK:" + String(bank_nr) + " stations from:");
  u8g2.sendBuffer();
  
  currentSelection = 0;
  firstVisibleLine = 0;
  station_nr = 1;
  previous_bank_nr = bank_nr; // jesli ładujemy stacje to ustawiamy zmienna previous_bank

  // Utwórz obiekt klienta HTTP
  HTTPClient http;

  // URL stacji dla danego banku
  String url;
 
  

  // Wybierz URL na podstawie bank_nr za pomocą switch
  switch (bank_nr) {
    case 1:
      url = STATIONS_URL1;
      break;
    case 2:
      url = STATIONS_URL2;
      break;
    case 3:
      url = STATIONS_URL3;
      break;
    case 4:
      url = STATIONS_URL4;
      break;
    case 5:
      url = STATIONS_URL5;
      break;
    case 6:
      url = STATIONS_URL6;
      break;
    case 7:
      url = STATIONS_URL7;
      break;
    case 8:
      url = STATIONS_URL8;
      break;
    case 9:
      url = STATIONS_URL9;
      break;
    case 10:
      url = STATIONS_URL10;
      break;
    case 11:
      url = STATIONS_URL11;
      break;
    case 12:
      url = STATIONS_URL12;
      break;
    case 13:
      url = STATIONS_URL13;
      break;
    case 14:
      url = STATIONS_URL14;
      break;
    case 15:
      url = STATIONS_URL15;
      break;
    case 16:
      url = STATIONS_URL16;
      break;
    default:
      Serial.println("Nieprawidłowy numer banku");
      return;
  }
  

  // Tworzenie nazwy pliku dla danego banku
  String fileName = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzenie, czy plik istnieje
  if (SPIFFS.exists(fileName) && bankNetworkUpdate == false) 
  {
    Serial.println("Plik banku " + fileName + " już istnieje.");
    u8g2.setFont(spleen6x12PL);
    u8g2.print("SPIFFS CARD");
    u8g2.sendBuffer();
    readSPIFFSStations();  // Jesli plik istnieje to odczytujemy go tylko z karty
  } 
  else
  //if (bankNetworkUpdate = true)
  {
    bankNetworkUpdate = false;
    // stworz plik na karcie tylko jesli on nie istnieje GR
    //u8g2.drawStr(205, 23, "GitHub server");
    u8g2.print("GitHub");
    u8g2.sendBuffer();
    {
      // Próba utworzenia pliku, jeśli nie istnieje
      File bankFile = SPIFFS.open(fileName, FILE_WRITE);

      if (bankFile) 
      {
        Serial.println("Utworzono plik banku: " + fileName);
        bankFile.close();  // Zamykanie pliku po utworzeniu
      } else 
      {
        Serial.println("Błąd: Nie można utworzyć pliku banku: " + fileName);
      //  return;  // Przerwij dalsze działanie, jeśli nie udało się utworzyć pliku
      }
    }
    // Inicjalizuj żądanie HTTP do podanego adresu URL
    http.begin(url);

    // Wykonaj żądanie GET i zapisz kod odpowiedzi HTTP
    int httpCode = http.GET();

    // Wydrukuj dodatkowe informacje diagnostyczne
    Serial.print("Kod odpowiedzi HTTP: ");
    Serial.println(httpCode);

    // Sprawdź, czy żądanie było udane (HTTP_CODE_OK)
    if (httpCode == HTTP_CODE_OK)
    {
      // Pobierz zawartość odpowiedzi HTTP w postaci tekstu
      String payload = http.getString();
      //Serial.println("Stacje pobrane z serwera:");
      //Serial.println(payload);  // Wyświetlenie pobranych danych (payload)
      // Otwórz plik w trybie zapisu, aby zapisać payload
      File bankFile = SPIFFS.open(fileName, FILE_WRITE);
      if (bankFile) {
        bankFile.println(payload);  // Zapisz dane do pliku
        bankFile.close();           // Zamknij plik po zapisaniu
        Serial.println("Dane zapisane do pliku: " + fileName);
      } else {
        Serial.println("Błąd: Nie można otworzyć pliku do zapisu: " + fileName);
      }
      // Zapisz każdą niepustą stację do pamięci EEPROM z indeksem
      int startIndex = 0;
      int endIndex;
      stationsCount = 0;
      // Przeszukuj otrzymaną zawartość w poszukiwaniu nowych linii
      while ((endIndex = payload.indexOf('\n', startIndex)) != -1 && stationsCount < MAX_STATIONS) {
        // Wyodrębnij pojedynczą stację z otrzymanego tekstu
        String station = payload.substring(startIndex, endIndex);

        // Sprawdź, czy stacja nie jest pusta, a następnie przetwórz i zapisz
        if (!station.isEmpty()) {
          // Zapisz stację do pliku na karcie SPIFFS
          sanitizeAndSaveStation(station.c_str());
        }
        // Przesuń indeks początkowy do kolejnej linii
        startIndex = endIndex + 1;
      }
    } else {
      // W przypadku nieudanego żądania wydrukuj informację o błędzie z kodem HTTP
      Serial.printf("Błąd podczas pobierania stacji. Kod HTTP: %d\n", httpCode);
    }
    // Zakończ połączenie HTTP
    http.end();
    
  }
  wsRefreshPage();
  
}

void readEEPROM() // Funkcja kontrolna-debug, nie uzywan przez inne funkcje
{
  EEPROM.get(0, station_nr);
  EEPROM.get(1, bank_nr);
  EEPROM.get(2, volumeValue); 
}



void calcNec() // Funkcja umozliwajaca przeliczanie odwrotne aby "udawac" przyciskami klawiatury komendy piltoa w standardzie NEC
{
  //składamy kod pilota do postaci ADDR/CMD/CMD/ADDR aby miec 4 bajty
  uint8_t CMD = (ir_code >> 8) & 0xFF; 
  uint8_t ADDR = ir_code & 0xFF;        
  ir_code =  ADDR;
  ir_code = (ir_code << 8) | CMD;
  ir_code = (ir_code << 8) | CMD;
  ir_code = (ir_code << 8) | ADDR;
  ADDR = (ir_code >> 24) & 0xFF;           // Pierwszy bajt
  uint8_t IADDR = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
  CMD = (ir_code >> 8) & 0xFF;            // Trzeci bajt (komenda)
  uint8_t ICMD = ir_code & 0xFF;          // Czwarty bajt (inwersja komendy)
  
  // Dorabiamy brakujące odwórcone bajty 
  IADDR = IADDR ^ 0xFF;
  ICMD = ICMD ^ 0xFF;

  // Składamy bajty w jeden ciąg          
  ir_code =  ICMD;
  ir_code = (ir_code << 8) | ADDR;
  ir_code = (ir_code << 8) | IADDR;
  ir_code = (ir_code << 8) | CMD;
  ir_code = reverse_bits(ir_code,32);     // rotacja bitów do porządku LSB-MSB jak w NEC        
}

void stationStringFormatting() // Funkcja aktualizujaca dla scorllera stationString/stationName  **** stationStringScroll ****
{
  if (displayMode == 0)
  {   
    if (stationString == "") // Jeżeli stationString jest pusty i stacja go nie nadaje to podmieniamy pusty stationString na nazwę staji - stationNameStream
    {    
      if (stationNameStream == "") // jezeli nie ma równiez stationName to wstawiamy 3 kreseczki
      { 
        stationStringScroll = "---" ;
        stationStringWeb = "---" ;
      } 
      else // jezeli jest station name to prawiamy w "-- NAZWA --" i wysylamy do scrollera
      { 
        transliterateToAscii(stationNameStream);
        processText(stationNameStream);
        stationStringScroll = ("-- " + stationNameStream + " --");
        stationStringWeb = ("-- " + stationNameStreamWeb + " --");
      }  // Zmienna stationStringScroller przyjmuje wartość stationNameStream
    }
    else // Jezeli stationString zawiera dane to przypisujemy go do stationStringScroll do funkcji scrollera
    {
      stationStringWeb = stationString;
      transliterateToAscii(stationString);
      processText(stationString);  // przetwarzamy polskie znaki
      stationStringScroll = stationString + "    "; // dodajemy separator do przewijanego tekstu jesli się nie miesci na ekranie
    }             
        //Liczymy długość napisu stationStringScroll 
    Serial.print("### StationStringScroll lenght [chars]:");  Serial.println(stationStringScroll.length());
    stationStringScrollWidth = stationStringScroll.length() * 6;
    Serial.print("### Station String Scroll Width (lenght * 6) [px]:"); Serial.println(stationStringScrollWidth);
	  
    Serial.print("debug -> Display0 (VU) stationStringScroll: ");
    Serial.println(stationStringScroll);



  }
  else if (displayMode == 1) // Tryb wświetlania zegara z 1 linijką radia na dole
  {
    char StationNrStr[3];
    snprintf(StationNrStr, sizeof(StationNrStr), "%02d", station_nr);  //Formatowanie informacji o stacji i banku do postaci 00

    int StationNameEnd = stationName.indexOf("  "); // Wycinamy nazwe stacji tylko do miejsca podwojnej spacji 
    stationName = stationName.substring(0, StationNameEnd);
 
    if (stationString == "")                // Jeżeli stationString jest pusty i stacja go nie nadaje
    {   
      if (stationNameStream == "")          // jezeli nie ma równiez stationName
      {
        stationStringScroll = String(StationNrStr) + "." + stationName + ", ---" ;
        stationStringWeb = "---" ;
      }      // wstawiamy trzy kreseczki do wyswietlenia
      else                                  // jezeli jest brak "stationString" ale jest "stationName" to składamy NR.Nazwa stacji z pliku, nadawany stationNameStream + separator przerwy
      { 
        transliterateToAscii(stationNameStream);
        processText(stationNameStream);
        stationStringScroll = String(StationNrStr) + "." + stationName + ", " + stationNameStream + "      ";
        stationStringWeb = stationNameStreamWeb;
      }
    }
    else //stationString != "" -> ma wartość
    {
      stationStringWeb = stationString;
      transliterateToAscii(stationString);
      processText(stationString);  // przetwarzamy polsie znaki
      stationStringScroll = String(StationNrStr) + "." + stationName + ", " + stationString + "      ";
      Serial.println(stationStringScroll);
    }
	  Serial.print("debug -> Display1 (zegar) stationStringScroll: ");
    Serial.println(stationStringScroll);

    //Liczymy długość napisu stationStringScrollWidth 
    stationStringScrollWidth = stationStringScroll.length() * 6;
  }
  else if (displayMode == 2) // Tryb wświetlania mode 2 - 3 linijki tekstu
  {             
    // Jesli stacja nie nadaje stationString to podmieniamy pusty stationString na nazwę staji - stationNameStream
    if (stationString == "") // Jeżeli stationString jest pusty i stacja go nie nadaje
    {    
      if (stationNameStream == "") // jezeli nie ma równiez stationName
      { 
        stationStringScroll = "---" ;
        stationStringWeb = "---" ;
      } // wstawiamy trzy kreseczki do wyswietlenia
      else // jezeli jest station name to oprawiamy w "-- NAZWA --" i wysylamy do scrollera
      { 
        transliterateToAscii(stationNameStream);
        processText(stationNameStream);
        stationStringScroll = ("-- " + stationNameStream + " --");
        stationStringWeb = stationNameStreamWeb;
      }  // Zmienna stationStringScroller przyjmuje wartość stationNameStream
    }
    else // Jezeli stationString zawiera dane to przypisujemy go do stationStringScroll do funkcji scrollera
    {
      stationStringWeb = stationString;
      transliterateToAscii(stationString);
      processText(stationString);  // przetwarzamy polskie znaki
      stationStringScroll = stationString;
    }

   
  }
}

// Obsługa wyświetlacza dla odtwarzanego strumienia radia internetowego
void displayRadio() 
{
  if (displayMode == 0)
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_10x20_tf);
    u8g2.drawStr(24, 16, stationName.substring(0, stationNameLenghtCut - 1).c_str());
    u8g2.drawRBox(1, 1, 21, 16, 4);  // Rbox pod numerem stacji
    
    // Funkcja wyswietlania numeru Banku na dole ekranu
    u8g2.setFont(spleen6x12PL);
    char BankStr[8];  
    snprintf(BankStr, sizeof(BankStr), "Bank%02d", bank_nr); // Formatujemy numer banku do postacji 00

    // Wyswietlamy numer Banku w dolnej linijce
    u8g2.drawBox(154, 54, 1, 12);  // dorysowujemy 1px pasek przed napisem "Bank" dla symetrii
    u8g2.setDrawColor(0);
    u8g2.setCursor(155, 63);  // Pozycja napisu Bank0x na dole ekranu
    u8g2.print(BankStr);


    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_spleen8x16_mr);
    char StationNrStr[3];
    snprintf(StationNrStr, sizeof(StationNrStr), "%02d", station_nr);  //Formatowanie informacji o stacji i banku do postaci 00
    u8g2.setCursor(4, 14);                                            // Pozycja numeru stacji na gorze po lewej ekranu
    u8g2.print(StationNrStr);
    u8g2.setDrawColor(1);
    
    u8g2.setFont(spleen6x12PL);
        
    stationStringFormatting(); //Formatujemy stationString wyswietlany przez funkcję Scrollera

    u8g2.drawLine(0, 52, 255, 52);
    
    // Przeliczamy Hz na kHz
    int SampleRate = sampleRateString.toInt();
    int SampleRateRest = SampleRate % 1000;
    SampleRateRest = SampleRateRest / 100;
    SampleRate = SampleRate / 1000;
    
    String displayString = String(SampleRate) + "." + String(SampleRateRest) + "kHz " + bitsPerSampleString + "bit " + bitrateString + "kbps";
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(0, 63, displayString.c_str());
  }
  else if (displayMode == 1) // Tryb wświetlania zegara z 1 linijką radia na dole
  {
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFont(spleen6x12PL);
    u8g2.drawLine(0, 50, 255, 50); // Linia separacyjna zegar, dolna linijka radia

    stationStringFormatting(); //Formatujemy stationString wyswietlany przez funkcję Scrollera

  }
  else if (displayMode == 2) // Tryb wświetlania mode 2 - 3 linijki tekstu
  {
    u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(24, 11, stationName.substring(0, stationNameLenghtCut).c_str()); // Przyciecie i wyswietlenie dzieki temu nie zmieniamy zawartosci zmiennej stationName
    u8g2.drawRBox(1, 1, 18, 13, 4);  // Rbox pod numerem stacji
    
    // Funkcja wyswietlania numeru Banku na dole ekranu
    char BankStr[8];  
    snprintf(BankStr, sizeof(BankStr), "Bank%02d", bank_nr); // Formatujemy numer banku do postacji 00

    // Wyswietlamy numer Banku w dolnej linijce
    u8g2.drawBox(154, 54, 1, 12);  // dorysowujemy 1px pasek przed napisem "Bank" dla symetrii
    u8g2.setDrawColor(0);
    u8g2.setCursor(155, 63);  // Pozycja napisu Bank0x na dole ekranu
    u8g2.print(BankStr);


    u8g2.setDrawColor(0);
    char StationNrStr[3];
    snprintf(StationNrStr, sizeof(StationNrStr), "%02d", station_nr);  //Formatowanie informacji o stacji i banku do postaci 00
    u8g2.setCursor(4, 11);                                            // Pozycja numeru stacji na gorze po lewej ekranu
    u8g2.print(StationNrStr);
    u8g2.setDrawColor(1);

    stationStringFormatting(); //Formatujemy stationString wyswietlany przez funkcję Scrollera
    
    u8g2.drawLine(0, 52, 255, 52);
    
    // Przeliczamy Hz na kHz
    int SampleRate = sampleRateString.toInt();
    int SampleRateRest = SampleRate % 1000;
    SampleRateRest = SampleRateRest / 100;
    SampleRate = SampleRate / 1000;     
    
    String displayString = String(SampleRate) + "." + String(SampleRateRest) + "kHz " + bitsPerSampleString + "bit " + bitrateString + "kbps";
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(0, 63, displayString.c_str());  
  }
}

void audio_info(const char *info) 
{
  // Wyświetl informacje w konsoli szeregowej
  Serial.print("info        ");
  Serial.println(info);
  // Znajdź pozycję "BitRate:" w tekście
  int bitrateIndex = String(info).indexOf("BitRate:");
  bitratePresent = false;
  if (bitrateIndex != -1) 
  {
    // Przytnij tekst od pozycji "BitRate:" do końca linii
    bitrateString = String(info).substring(bitrateIndex + 8, String(info).indexOf('\n', bitrateIndex));
    bitrateStringInt = bitrateString.toInt();  // przliczenie bps na Kbps
    bitrateStringInt = bitrateStringInt / 1000;
    bitrateString = String(bitrateStringInt);
    bitratePresent = true;

    audioInfoRefresh2 = true;
    wsAudioRefresh = true;
  }

  // Znajdź pozycję "SampleRate:" w tekście
  int sampleRateIndex = String(info).indexOf("SampleRate:");
  if (sampleRateIndex != -1) {
    // Przytnij tekst od pozycji "SampleRate:" do końca linii
    sampleRateString = String(info).substring(sampleRateIndex + 11, String(info).indexOf('\n', sampleRateIndex));
  }

  // Znajdź pozycję "BitsPerSample:" w tekście
  int bitsPerSampleIndex = String(info).indexOf("BitsPerSample:");
  if (bitsPerSampleIndex != -1) {
    // Przytnij tekst od pozycji "BitsPerSample:" do końca linii
    bitsPerSampleString = String(info).substring(bitsPerSampleIndex + 15, String(info).indexOf('\n', bitsPerSampleIndex));
  }

  if (String(info).indexOf("MP3Decoder") != -1) {
    mp3 = true;
    flac = false;
    aac = false;
    vorbis = false;
    opus = false;
  }

  if (String(info).indexOf("FLACDecoder") != -1) {
    flac = true;
    mp3 = false;
    aac = false;
    vorbis = false;
    opus = false;
  }

  if (String(info).indexOf("AACDecoder") != -1) {
    aac = true;
    flac = false;
    mp3 = false;
    vorbis = false;
    opus = false;
  }
  if (String(info).indexOf("VORBISDecoder") != -1) {
    vorbis = true;
    aac = false;
    flac = false;
    mp3 = false;
    opus = false;
  }
  if (String(info).indexOf("OPUSDecoder") != -1) {
    opus = true;
    vorbis = true;
    aac = false;
    flac = false;
    mp3 = false;
  }
}

void audio_id3data(const char *info) {
  Serial.print("id3data     ");
  Serial.println(info);
}

void audio_bitrate(const char *info) {
  Serial.print("bitrate     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info) {
  Serial.print("eof_mp3     ");
  Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station     ");
  Serial.println(info);
  stationNameStream = info;
  stationNameStreamWeb = info;
  audioInfoRefresh = true;
  wsAudioRefresh = true;
}
void audio_showstreamtitle(const char *info) {

  Serial.print("streamtitle ");
  Serial.println(info);
  stationString = String(info);
  
  ActionNeedUpdateTime = true;
  //if ((volumeSet == false) && (bankMenuEnable == false) && (listedStations == false) && (rcInputDigitsMenuEnable == false) && (equalizerMenuEnable == false))
  //if (displayActive == false)
  //{
    audioInfoRefresh = true;
    wsAudioRefresh = true;
  //}
 
}
void audio_commercial(const char *info) {
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info) {
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info) {
  Serial.print("lasthost    ");
  Serial.println(info);
}
void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  ");
  Serial.println(info);
  if (resumePlay == true)
  {
    ir_code = rcCmdOk; // Przypisujemy kod pilota - OK
    bit_count = 32;
    calcNec();        // Przeliczamy kod pilota na pełny kod NEC
    resumePlay = false;
  }
}

void encoderFunctionOrderChange()
{
  displayActive = true;
  displayStartTime = millis();
  volumeSet = false;
  timeDisplay = false;
  bankMenuEnable = false;
  encoderFunctionOrder = !encoderFunctionOrder;
  u8g2.clearBuffer();
  u8g2.setFont(spleen6x12PL);
  u8g2.drawStr(1,14,"Encoder function order change:");
  if (encoderFunctionOrder == false) {u8g2.drawStr(1,28,"Rotate for volume, press for station list");}
  if (encoderFunctionOrder == true ) {u8g2.drawStr(1,28,"Rotate for station list, press for volume");}
  u8g2.sendBuffer();
}

void bankMenuDisplay()
{
  displayStartTime = millis();
  Serial.println("debug--BankMenuDisplay");
  volumeSet = false;
  //previous_bank_nr = bank_nr;  // jesli weszlimy do menu "wybór banku" to zapisujemy obecny bank zanim zaczniemy krecic enkoderem
  bankMenuEnable = true;
  timeDisplay = false;
  displayActive = true;
  //currentOption = BANK_LIST;  // Ustawienie listy banków do przewijania i wyboru
  String bankNrStr = String(bank_nr);
  Serial.println("Wyświetlenie listy banków");
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(80, 33, "BANK ");
  u8g2.drawStr(145, 33, String(bank_nr).c_str());  // numer banku
  if ((bankNetworkUpdate == true) || (noSPIFFScard == true))
  {
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(185, 24, "NETWORK ");
    u8g2.drawStr(188, 34, "UPDATE  ");
    
    if (noSPIFFScard == true)
    {
      u8g2.drawStr(24, 34, "NO SPIFFS");
    }
  
  }
  //else
  //{
  //  u8g2.setFont(u8g2_font_fub14_tf);
  //  u8g2.drawStr(170, 33, "      ");
  //}
  u8g2.drawRFrame(21, 42, 214, 14, 3);                // Ramka do slidera bankow
  u8g2.drawRBox((bank_nr * 13) + 10, 44, 15, 10, 2);  // wypełnienie slidera
  u8g2.sendBuffer();
  
}

// =========== Funkcja do obsługi przycisków enkoderów, debouncing i długiego naciśnięcia ==============//

void handleButtons() {
  static unsigned long buttonPressTime2 = 0;  // Zmienna do przechowywania czasu naciśnięcia przycisku enkodera 2
  static bool isButton2Pressed = false;       // Flaga do śledzenia, czy przycisk enkodera 2 jest wciśnięty
  static bool action2Taken = false;           // Flaga do śledzenia, czy akcja dla enkodera 2 została wykonana

  static unsigned long lastPressTime = 0;  // Zmienna do kontrolowania debouncingu (ostatni czas naciśnięcia)
  const unsigned long debounceDelay = 50;  // Opóźnienie debouncingu

  // ===== Obsługa przycisku enkodera 2 =====
  int reading2 = digitalRead(SW_PIN2);

  // Debouncing dla przycisku enkodera 2

  if (reading2 == LOW)  // Przycisk jest wciśnięty (stan niski)
  {
    if (millis() - lastPressTime > debounceDelay) {
      //encoderButton2 = true;  // Ustawiamy flagę, że przycisk został wciśnięty
      lastPressTime = millis();  // Aktualizujemy czas ostatniego naciśnięcia

      // Sprawdzamy, czy przycisk był wciśnięty przez 3 sekundy
      if (!isButton2Pressed) {
        buttonPressTime2 = millis();  // Ustawiamy czas naciśnięcia
        isButton2Pressed = true;      // Ustawiamy flagę, że przycisk jest wciśnięty
        action2Taken = false;         // Resetujemy flagę akcji dla enkodera 2
        action3Taken = false;         // Resetujmy flage akcji Super długiego wcisniecia enkodera 2
        volumeSet = false;
      }

      if (millis() - buttonPressTime2 >= buttonLongPressTime2 && millis() - buttonPressTime2 >= buttonSuperLongPressTime2 && action3Taken == false) 
      {
        //encoderFunctionOrderChange();
        
        u8g2.setFont(spleen6x12PL);
        u8g2.clearBuffer();
        displayActive = true;
        displayStartTime = millis();
        debugKeyboard = !debugKeyboard;
        adcKeyboardEnabled = !adcKeyboardEnabled;
        Serial.print("Pomiar wartości ADC ON/OFF:");
        Serial.println(debugKeyboard);
        

        action3Taken = true;
      }


      // Jeśli przycisk jest wciśnięty przez co najmniej 3 sekundy i akcja jeszcze nie była wykonana
      if (millis() - buttonPressTime2 >= buttonLongPressTime2 && !action2Taken && millis() - buttonPressTime2 < buttonSuperLongPressTime2) {
        Serial.println("debug--Bank Menu");
        bankMenuDisplay();
        
        // Ustawiamy flagę akcji, aby wykonała się tylko raz
        action2Taken = true;
      }
    }
  } 
  else 
  {
    isButton2Pressed = false;  // Resetujemy flagę naciśnięcia przycisku enkodera 2
    action2Taken = false;      // Resetujemy flagę akcji dla enkodera 2
    action3Taken = false;
  }
}

int maxSelection() {
  //if (currentOption == INTERNET_RADIO) 
  //{
    return stationsCount - 1;
  //} 
  //return 0;  // Zwraca 0, jeśli żaden warunek nie jest spełniony
}

// Funkcja do przewijania w dół
void scrollDown() 
{
  if (currentSelection < maxSelection()) 
  {
    currentSelection++;
    if (currentSelection >= firstVisibleLine + maxVisibleLines) 
    {
      firstVisibleLine++;
    }
  }
    else
  {
    // Jeśli osiągnięto maksymalną wartość, przejdź do najmniejszej (0)
    currentSelection = 0;
    firstVisibleLine = 0; // Przywróć do pierwszej widocznej linii
  }
    
  Serial.print("Scroll Down: CurrentSelection = ");
  Serial.println(currentSelection);
}

// Funkcja do przewijania w górę
void scrollUp() {
  if (currentSelection > 0) 
  {
    currentSelection--;
    if (currentSelection < firstVisibleLine) 
    {
      firstVisibleLine = currentSelection;
    }
  }
  else
  {
    // Jeśli osiągnięto wartość 0, przejdź do najwyższej wartości
    currentSelection = maxSelection(); 
    firstVisibleLine = currentSelection - maxVisibleLines + 1; // Ustaw pierwszą widoczną linię na najwyższą
  }
  
  Serial.print("Scroll Up: CurrentSelection = ");
  Serial.println(currentSelection);
}

void readVolumeFromSPIFFS() 
{
  // Sprawdź, czy karta SPIFFS jest dostępna
  if (!SPIFFS.begin()) 
  {
    Serial.println("Nie można znaleźć karty SPIFFS, ustawiam wartość Volume z EEPROMu.");
    Serial.print("Wartość Volume: ");
    EEPROM.get(2, volumeValue);
    if (volumeValue > maxVolume) {volumeValue = 10;} // zabezpiczenie przed pusta komorka EEPROM o wartosci FF (255)
    
    audio.setVolume(volumeValue);  // zakres 0...21...42
    volumeBufferValue = volumeValue; 
    
    Serial.println(volumeValue);
    return;
  }
  // Sprawdź, czy plik volume.txt istnieje
  if (SPIFFS.exists("/volume.txt")) 
  {
    myFile = SPIFFS.open("/volume.txt");
    if (myFile) 
    {
      volumeValue = myFile.parseInt();
	    myFile.close();
      
      Serial.println("Wczytano volume.txt z karty SPIFFS");
      Serial.print("Wartość Volume odczytany z SPIFFS: ");
      Serial.println(volumeValue);    
    } 
	  else 
	  {
      Serial.println("Błąd podczas otwierania pliku volume.txt");
    }
  } 
  else 
  {
    Serial.println("Plik volume.txt nie istnieje.");
	  Serial.print("Wartość Volume domyślna:");
    Serial.println(volumeValue);    
  }
  if (volumeValue > maxVolume) {volumeValue = 10;}  // Ustawiamy bezpieczną wartość
  audio.setVolume(volumeValue);  // zakres 0...21...42
  volumeBufferValue = volumeValue;
}

void saveVolumeOnSPIFFS() 
{
  volumeBufferValue = volumeValue; // Wyrownanie wartosci bufora Volume i Volume przy zapisie
  
  // Sprawdź, czy plik volume.txt istnieje
  Serial.print("Zaspis do pliku wartosci Volume: ");
  Serial.println(volumeValue); 
  
  // Sprawdź, czy plik istnieje
  if (SPIFFS.exists("/volume.txt")) 
  {
    Serial.println("Plik volume.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość flitrów equalizera
    myFile = SPIFFS.open("/volume.txt", FILE_WRITE);
    if (myFile) 
	{
      myFile.println(volumeValue);
	  myFile.close();
      Serial.println("Aktualizacja volume.txt na karcie SPIFFS");
    } 
	else 
	{
      Serial.println("Błąd podczas otwierania pliku volume.txt.");
    }
  } 
  else 
  {
    Serial.println("Plik volume.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość głośności
    myFile = SPIFFS.open("/volume.txt", FILE_WRITE);
    if (myFile) 
	{
      myFile.println(volumeValue);
      myFile.close();
      Serial.println("Utworzono i zapisano volume.txt na karcie SPIFFS");
    } 
	else 
	{
      Serial.println("Błąd podczas tworzenia pliku volume.txt.");
     }
  }
  if (noSPIFFScard == true) {EEPROM.write(2,volumeValue); EEPROM.commit(); Serial.println("Zapis volume do EEPROM");}
}

void drawSignalPower(uint8_t xpwr, uint8_t ypwr, bool print)
{
  // Wartosci na podstawie ->  https://www.intuitibits.com/2016/03/23/dbm-to-percent-conversion/
  int signal_dBM[] = { -100, -99, -98, -97, -96, -95, -94, -93, -92, -91, -90, -89, -88, -87, -86, -85, -84, -83, -82, -81, -80, -79, -78, -77, -76, -75, -74, -73, -72, -71, -70, -69, -68, -67, -66, -65, -64, -63, -62, -61, -60, -59, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1};
  int signal_percent[] = {0, 0, 0, 0, 0, 0, 4, 6, 8, 11, 13, 15, 17, 19, 21, 23, 26, 28, 30, 32, 34, 35, 37, 39, 41, 43, 45, 46, 48, 50, 52, 53, 55, 56, 58, 59, 61, 62, 64, 65, 67, 68, 69, 71, 72, 73, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 90, 91, 92, 93, 93, 94, 95, 95, 96, 96, 97, 97, 98, 98, 99, 99, 99, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

  int signalpwr = WiFi.RSSI();
  uint8_t signalLevel = 0;
    
  
  // Pionowe kreseczki, szerokosc 11px
  
  // Czyscimy obszar pod kreseczkami
  u8g2.setDrawColor(0);
  u8g2.drawBox(xpwr,ypwr-10,11,11);
  u8g2.setDrawColor(1);

  // Rysujemy podstawy 1x1px pod kazdą kreseczką
  u8g2.drawBox(xpwr,ypwr - 1, 1, 1);
  u8g2.drawBox(xpwr + 2, ypwr - 1, 1, 1);
  u8g2.drawBox(xpwr + 4, ypwr - 1, 1, 1);
  u8g2.drawBox(xpwr + 6, ypwr - 1, 1, 1);
  u8g2.drawBox(xpwr + 8, ypwr - 1, 1, 1);
  u8g2.drawBox(xpwr + 10, ypwr - 1, 1, 1);

 if (WiFi.status() == WL_CONNECTED)
 {
      // Rysujemy kreseczki
    if (signalpwr > -88) { signalLevel = 1; u8g2.drawBox(xpwr, ypwr - 2, 1, 1); }     // 0-14
    if (signalpwr > -81) { signalLevel = 2; u8g2.drawBox(xpwr + 2, ypwr - 3, 1, 2); } // > 28
    if (signalpwr > -74) { signalLevel = 3; u8g2.drawBox(xpwr + 4, ypwr - 4, 1, 3); } // > 42
    if (signalpwr > -66) { signalLevel = 4; u8g2.drawBox(xpwr + 6, ypwr - 5, 1, 4); } // > 56
    if (signalpwr > -57) { signalLevel = 5; u8g2.drawBox(xpwr + 8, ypwr - 6, 1, 5); } // > 70
    if (signalpwr > -50) { signalLevel = 6; u8g2.drawBox(xpwr + 10, ypwr - 8, 1, 7);} // > 84
  }

  if (print == true) // Jesli flaga print =1 to wypisujemy na serialu sile sygnału w % i w skali 1-6
  {
    for (int j = 0; j < 100; j++) 
    {
      if (signal_dBM[j] == signalpwr) 
      {
        Serial.print("Sygnału WiFi: ");
        Serial.print(signal_percent[j]);
        Serial.print("%  Poziom: ");
        Serial.print(signalLevel);
        Serial.print("   dBm: ");
        Serial.println(signalpwr);
        
        break;
      }
    }
  }
}


void rcInputKey(uint8_t i)
{
  rcInputDigitsMenuEnable = true;
  if (bankMenuEnable == true)
  {
    
    if (i == 0) {i = 10;}
    bank_nr = i;
    bankMenuDisplay();
  }
  else
  {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis(); 
    
    if (rcInputDigit1 == 0xFF)
    {
      rcInputDigit1 = i;
    }
    else 
    {
      rcInputDigit2 = i;
    }

    int y = 35;
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
    u8g2.drawStr(65, y, "Station:"); 
    if (rcInputDigit1 != 0xFF)
    {u8g2.drawStr(153, y, String(rcInputDigit1).c_str());} 
    else {u8g2.drawStr(153, x,"_" ); }
    
    if (rcInputDigit2 != 0xFF)
    {u8g2.drawStr(164, y, String(rcInputDigit2).c_str());} 
    else {u8g2.drawStr(164, y,"_" ); }



    if ((rcInputDigit1 != 0xFF) && (rcInputDigit2 != 0xFF)) // jezeli obie wartosci nie są puste
    {
      station_nr = (rcInputDigit1 *10) + rcInputDigit2;
    }
    else if ((rcInputDigit1 != 0xFF) && (rcInputDigit2 == 0xFF))  // jezeli tylko podalismy jedna cyfrę
    { 
      station_nr = rcInputDigit1;
    }

    if (station_nr > stationsCount)  // sprawdzamy czy wprowadzona wartość nie wykracza poza licze stacji w danym banku
    {
      station_nr = stationsCount;  // jesli wpisana wartość jest wieksza niz ilosc stacji to ustawiamy war
    }
    
    if (station_nr < 1)
    {
      station_nr = stationFromBuffer;
    }

    // Odczyt stacji pod daną komórka pamieci PSRAM:
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych
    int length = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1)];   // Odczytaj długość nazwy stacji z PSRAM dla bieżącego indeksu stacji
    
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) { // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
      station[j] = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }
    u8g2.setFont(spleen6x12PL);
    String stationNameText = String(station);
    stationNameText = stationNameText.substring(0, 25); // Przycinamy do 23 znakow

    u8g2.drawLine(0,48,256,48);
    u8g2.setFont(spleen6x12PL);
    u8g2.setCursor(0, 60);
    u8g2.print("Bank:" + String(bank_nr) + ", 1-" + String(stationsCount) + "     " + stationNameText);
    u8g2.sendBuffer();

    if ((rcInputDigit1 !=0xFF) && (rcInputDigit2 !=0xFF)) // jezeli wpisalismy obie cyfry to czyscimy pola aby mozna bylo je wpisac ponownie
    {
      rcInputDigit1 = 0xFF; // czyscimy cyfre 1, flaga pustej zmiennej F aby naciskajac kolejny raz mozna bylo wypisac cyfre bez czekania 6 sek
      rcInputDigit2 = 0xFF; // czyscimy cyfre 2, flaga pustej zmiennej F
    }
  }  
}

// Funkcja do zapisywania numeru stacji i numeru banku na karcie SPIFFS
void saveStationOnSPIFFS() 
{
  // Sprawdź, czy plik station_nr.txt istnieje

  Serial.print("Zapisujemy bank: ");
  Serial.println(bank_nr);
  Serial.print("Zapisujemy stacje: ");
  Serial.println(station_nr);

  // Sprawdź, czy plik station_nr.txt istnieje
  if (SPIFFS.exists("/station_nr.txt")) {
    Serial.println("Plik station_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość station_nr
    myFile = SPIFFS.open("/station_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(station_nr);
      myFile.close();
      Serial.println("Aktualizacja station_nr.txt na karcie SPIFFS");
    } else {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  } else {
    Serial.println("Plik station_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość station_nr
    myFile = SPIFFS.open("/station_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(station_nr);
      myFile.close();
      Serial.println("Utworzono i zapisano station_nr.txt na karcie SPIFFS");
    } else {
      Serial.println("Błąd podczas tworzenia pliku station_nr.txt.");
    }
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SPIFFS.exists("/bank_nr.txt")) 
  {
    Serial.println("Plik bank_nr.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość bank_nr
    myFile = SPIFFS.open("/bank_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(bank_nr);
      myFile.close();
      Serial.println("Aktualizacja bank_nr.txt na karcie SPIFFS.");
    } else {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  } 
  else 
  {
    Serial.println("Plik bank_nr.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość bank_nr
    myFile = SPIFFS.open("/bank_nr.txt", FILE_WRITE);
    if (myFile) {
      myFile.println(bank_nr);
      myFile.close();
      Serial.println("Utworzono i zapisano bank_nr.txt na karcie SPIFFS");
    } else {
      Serial.println("Błąd podczas tworzenia pliku bank_nr.txt.");
    }
  }
  
  if (noSPIFFScard == true)
  {
    Serial.println("Brak karty SPIFFS zapisujemy do EEPROM");
    EEPROM.write(0, station_nr);
    EEPROM.write(1, bank_nr);
    EEPROM.commit();
  }
}


// Funkcja odpowiedzialna za zmianę aktualnie wybranej stacji radiowej.
void changeStation2() 
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(34, 33, "Loading stream..."); // 8 znakow  x 11 szer
  u8g2.sendBuffer();

  mp3 = flac = aac = vorbis = opus = false;
  stationFromBuffer = station_nr;
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString
  stationNameStream.remove(0);

  // Tworzymy nazwę pliku banku
  String fileName = String("/bank") + (bank_nr < 10 ? "0" : "") + String(bank_nr) + ".txt";

  // Sprawdzamy, czy plik istnieje
  if (!SPIFFS.exists(fileName)) {
    Serial.println("Błąd: Plik banku nie istnieje.");
    return;
  }

  // Otwieramy plik w trybie do odczytu
  File bankFile = SPIFFS.open(fileName, FILE_READ);
  if (!bankFile)  // jesli brak pliku to...
  {
    Serial.println("Błąd: Nie można otworzyć pliku banku.");
    return;
  }

  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String stationUrl = "";
  while (bankFile.available()) {
    String line = bankFile.readStringUntil('\n');
    currentLine++;

    if (currentLine == station_nr) 
    {
      // Wyciągnij pierwsze 42 znaki i przypisz do stationName
      stationName = line.substring(0, 41);  //42 Skopiuj pierwsze 42 znaki z linii
      Serial.print("Nazwa stacji: ");
      Serial.println(stationName);

      // Znajdź część URL w linii, np. po numerze stacji
      int urlStart = line.indexOf("http");  // Szukamy miejsca, gdzie zaczyna się URL
      if (urlStart != -1) {
        stationUrl = line.substring(urlStart);  // Wyciągamy URL od "http"
        stationUrl.trim();                      // Usuwamy białe znaki na początku i końcu
      }
      break;
    }
  }
  bankFile.close();  // Zamykamy plik po odczycie
  // Sprawdzamy, czy znaleziono stację
  if (stationUrl.isEmpty()) {
    Serial.println("Błąd: Nie znaleziono stacji dla podanego numeru.");
    return;
  }

  // Weryfikacja, czy w linku znajduje się "http" lub "https"
  if (stationUrl.startsWith("http://") || stationUrl.startsWith("https://")) {
    // Wydrukuj nazwę stacji i link na serialu
    Serial.print("Aktualnie wybrana stacja: ");
    Serial.println(station_nr);
    Serial.print("Link do stacji: ");
    Serial.println(stationUrl);
    
    u8g2.setFont(spleen6x12PL);  // wypisujemy jaki stream jakie stacji jest ładowany
    u8g2.drawStr(34, 55, String(stationName.substring(0, stationNameLenghtCut)).c_str());
    u8g2.sendBuffer();
    
    // Połącz z daną stacją
    audio.connecttohost(stationUrl.c_str());
    //seconds = 0;
    stationFromBuffer = station_nr;
    bankFromBuffer = bank_nr;
    saveStationOnSPIFFS();
  } else {
    Serial.println("Błąd: link stacji nie zawiera 'http' lub 'https'");
    Serial.println("Odczytany URL: " + stationUrl);
  }
  currentSelection = station_nr - 1; // ustawiamy stacje na liscie na obecnie odtwarzaczną po zmianie stacji
  firstVisibleLine = currentSelection + 1; // pierwsza widoczna lina to grająca stacja przy starcie
  if (currentSelection + 1 >= stationsCount - 1) 
  {
   firstVisibleLine = currentSelection - 3;
  }
  //screenRefresh = true;

  //screenRefreshTime = millis();
}


void changeStation() 
{
  fwupd = false;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(34, 33, "Loading stream..."); // 8 znakow  x 11 szer
  u8g2.sendBuffer();

  mp3 = flac = aac = vorbis = opus = false;
  stationFromBuffer = station_nr;
   
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString
  stationNameStream.remove(0);

 

  Serial.println("debug-- Read station from PSRAM");
  String stationUrl = "";

  // Odczyt stacji pod daną komórka pamieci PSRAM:
  char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
  memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych
  int length = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1)];   // Odczytaj długość nazwy stacji z PSRAM dla bieżącego indeksu stacji
      
  for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
  { // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
    station[j] = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
  }
  
  //Serial.println("-------- GRAMY OBECNIE ---------- ");
  //Serial.print(station_nr - 1);
  //Serial.print(" ");
  //Serial.println(String(station));

  String line = String(station); // Przypisujemy dane odczytane z PSRAM do zmiennej line
  
  // Wyciągnij pierwsze 42 znaki i przypisz do stationName
  stationName = line.substring(0, 41);  //42 Skopiuj pierwsze 42 znaki z linii
  Serial.print("Nazwa stacji: ");
  Serial.println(stationName);

  // Znajdź część URL w linii, np. po numerze stacji
  int urlStart = line.indexOf("http");  // Szukamy miejsca, gdzie zaczyna się URL
  if (urlStart != -1) 
  {
    stationUrl = line.substring(urlStart);  // Wyciągamy URL od "http"
    stationUrl.trim();                      // Usuwamy białe znaki na początku i końcu
  }
  else
  {
	return;
  }
  
  if (stationUrl.isEmpty()) // jezeli link URL jest pusty
  {
    Serial.println("Błąd: Nie znaleziono stacji dla podanego numeru.");
    return;
  }
  
  //Serial.print("URL: ");
  //Serial.println(stationUrl);

  // Weryfikacja, czy w linku znajduje się "http" lub "https"
  if (stationUrl.startsWith("http://") || stationUrl.startsWith("https://")) 
  {
    // Wydrukuj nazwę stacji i link na serialu
    Serial.print("Aktualnie wybrana stacja: ");
    Serial.println(station_nr);
    Serial.print("Link do stacji: ");
    Serial.println(stationUrl);
    
    u8g2.setFont(spleen6x12PL);  // wypisujemy jaki stream jakie stacji jest ładowany
    u8g2.drawStr(34, 55, String(stationName.substring(0, stationNameLenghtCut)).c_str());
    u8g2.sendBuffer();
    
    // Połącz z daną stacją
    audio.connecttohost(stationUrl.c_str());
    stationFromBuffer = station_nr;
    bankFromBuffer = bank_nr;
  
    saveStationOnSPIFFS(); // Zapisujemy jaki numer stacji i który bank gramy
  } 
  else 
  {
    Serial.println("Błąd: link stacji nie zawiera 'http' lub 'https'");
    Serial.println("Odczytany URL: " + stationUrl);
  }
  currentSelection = station_nr - 1; // ustawiamy stacje na liscie na obecnie odtwarzaczną po zmianie stacji
  firstVisibleLine = currentSelection + 1; // pierwsza widoczna lina to grająca stacja przy starcie
  if (currentSelection + 1 >= stationsCount - 1) 
  {
   firstVisibleLine = currentSelection - 3;
  }
  wsStationChange(station_nr);
}

// Funkcja do wyświetlania listy stacji radiowych z opcją wyboru poprzez zaznaczanie w negatywie
void displayStations() 
{
  listedStations = true;
  u8g2.clearBuffer();  // Wyczyść bufor przed rysowaniem, aby przygotować ekran do nowej zawartości
  u8g2.setFont(spleen6x12PL);
  u8g2.setCursor(60, 10);                                          // Ustaw pozycję kursora (x=60, y=10) dla nagłówka
  u8g2.print("RADIO STATIONS:   ");                                // Wyświetl nagłówek "Radio Stations:"
  u8g2.print(String(station_nr) + " / " + String(stationsCount));  // Dodaj numer aktualnej stacji i licznik wszystkich stacji

  int displayRow = 1;  // Zmienna dla numeru wiersza, zaczynając od drugiego (pierwszy to nagłówek)
  
  //erial.print("FirstVisibleLine:");
  //Serial.print(firstVisibleLine);

  // Wyświetlanie stacji, zaczynając od drugiej linii (y=21)
  for (int i = firstVisibleLine; i < min(firstVisibleLine + maxVisibleLines, stationsCount); i++) 
  {
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych

    // Odczytaj długość nazwy stacji z PSRAM dla bieżącego indeksu stacji
    int length = psramData[i * (STATION_NAME_LENGTH + 1)];  //----------------------------------------------

    // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
    {
      station[j] = psramData[i * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }

    // Sprawdź, czy bieżąca stacja to ta, która jest aktualnie zaznaczona
    if (i == currentSelection) 
    {
      u8g2.setDrawColor(1);                           // Ustaw biały kolor rysowania
      u8g2.drawBox(0, displayRow * 13 - 2, 256, 13);  // Narysuj prostokąt jako tło dla zaznaczonej stacji (x=0, szerokość 256, wysokość 10)
      u8g2.setDrawColor(0);                           // Zmień kolor rysowania na czarny dla tekstu zaznaczonej stacji
    } else {
      u8g2.setDrawColor(1);  // Dla niezaznaczonych stacji ustaw zwykły biały kolor tekstu
    }
    // Wyświetl nazwę stacji, ustawiając kursor na odpowiedniej pozycji
    u8g2.drawStr(0, displayRow * 13 + 8, String(station).c_str());
    //u8g2.print(station);  // Wyświetl nazwę stacji

    // Przejdź do następnej linii (następny wiersz na ekranie)
    displayRow++;
  }
  // Przywróć domyślne ustawienia koloru rysowania (biały tekst na czarnym tle)
  u8g2.setDrawColor(1);  // Biały kolor rysowania
  u8g2.sendBuffer();     // Wyślij zawartość bufora do ekranu OLED, aby wyświetlić zmiany
}

void updateTimerFlag() 
{
  ActionNeedUpdateTime = true;
}

void displayPowerSave(bool saveON)
{
  if ((saveON == 1) && (displayActive == false) && (fwupd == false) && (audio.isRunning() == true)) {u8g2.setPowerSave(1);}
  if (saveON == 0) {u8g2.setPowerSave(0);}
}



// Funkcja wywoływana co sekundę przez timer do aktualizacji czasu na wyświetlaczu
void updateTimer() 
{
  u8g2.setDrawColor(1);  // Ustaw kolor na biały

  if (timeDisplay == true) 
  {
    if ((audio.isRunning() == true) && (displayMode == 0) || (displayMode == 2)) {
      if (mp3 == true) {
        u8g2.drawStr(133, 63, "MP3");
        //Serial.println("Gram MP3");
      }
      if (flac == true) {
        u8g2.drawStr(133, 63, "FLC");
        //Serial.println("Gram FLAC");
      }
      if (aac == true) {
        u8g2.drawStr(133, 63, "AAC");
        //Serial.println("Gram AAC");
      }
      if (vorbis == true) {
        u8g2.drawStr(133, 63, "VRB");
        //Serial.println("Gram VORBIS");
      }
      if (opus == true) {
        u8g2.drawStr(129, 63, "OPUS");
        //Serial.println("Gram OPUS");
      }
    }

    if ((timeDisplay == true) && (audio.isRunning() == true))
    {
      // Struktura przechowująca informacje o czasie
      struct tm timeinfo;

      // Sprawdź, czy udało się pobrać czas z lokalnego zegara czasu rzeczywistego
      if (!getLocalTime(&timeinfo, 5)) 
      {
        // Wyświetl komunikat o niepowodzeniu w pobieraniu czasu
        Serial.println("Nie udało się uzyskać czasu");
        return;  // Zakończ funkcję, gdy nie udało się uzyskać czasu
      }

      // Konwertuj godzinę, minutę i sekundę na stringi w formacie "HH:MM:SS"
      char timeString[9];  // Bufor przechowujący czas w formie tekstowej
      //snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

      if ((timeinfo.tm_min == 0) && (timeinfo.tm_sec == 0) && (timeVoiceInfoEveryHour == true)) {voiceTimePlay = true; }

      if ((displayMode == 0) || (displayMode == 2))
      { 
        snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        u8g2.setFont(spleen6x12PL);
        u8g2.drawStr(208, 63, timeString);
      }
      else if (displayMode == 1)
      {
        int xtime = 0;
        u8g2.setFont(u8g2_font_7Segments_26x42_mn);
        snprintf(timeString, sizeof(timeString), "%2d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        u8g2.drawStr(xtime+7, 45, timeString);
        
        
        u8g2.setFont(u8g2_font_fub14_tf); // 14x11
        snprintf(timeString, sizeof(timeString), "%02d", timeinfo.tm_mday);
        u8g2.drawStr(203,17, timeString);

        String month = "";
        switch (timeinfo.tm_mon) {
        case 0: month = "JAN"; break;     
        case 1: month = "FEB"; break;     
        case 2: month = "MAR"; break;
        case 3: month = "APR"; break;
        case 4: month = "MAY"; break;
        case 5: month = "JUN"; break;
        case 6: month = "JUL"; break;
        case 7: month = "AUG"; break;
        case 8: month = "SEP"; break;
        case 9: month = "OCT"; break;
        case 10: month = "NOV"; break;
        case 11: month = "DEC"; break;                                
        }
        u8g2.setFont(spleen6x12PL);
        u8g2.drawStr(232,14, month.c_str());

        String dayOfWeek = "";
        switch (timeinfo.tm_wday) {
        case 0: dayOfWeek = " Sunday  "; break;     
        case 1: dayOfWeek = " Monday  "; break;     
        case 2: dayOfWeek = " Tuesday "; break;
        case 3: dayOfWeek = "Wednesday"; break;
        case 4: dayOfWeek = "Thursday "; break;
        case 5: dayOfWeek = " Friday  "; break;
        case 6: dayOfWeek = "Saturday "; break;
        }
        
        u8g2.drawRBox(198,20,58,15,3);  // Box z zaokraglonymi rogami, biały pod dniem tygodnia
        u8g2.drawLine(198,20,256,20); // Linia separacyjna dzien miesiac / dzien tygodnia
        u8g2.setDrawColor(0);
        u8g2.drawStr(201,31, dayOfWeek.c_str());
        u8g2.setDrawColor(1);
        u8g2.drawRFrame(198,0,58,35,3); // Ramka na całosci kalendarza

        snprintf(timeString, sizeof(timeString), ":%02d", timeinfo.tm_sec);
        u8g2.drawStr(xtime+163, 45, timeString);
      }


      //u8g2.sendBuffer(); // nie piszemy po ekranie w tej funkcji tylko przygotowujemy bufor. Nie mozna pisac podczas pracy scrollera
    }
    else if ((timeDisplay == true) && (audio.isRunning() == false))
    {
      displayPowerSave(0); 
      // Struktura przechowująca informacje o czasie
      struct tm timeinfo;
      if (!getLocalTime(&timeinfo, 5)) 
      {
        Serial.println("Nie udało się uzyskać czasu");
        return;  // Zakończ funkcję, gdy nie udało się uzyskać czasu
      }
      char timeString[9];  // Bufor przechowujący czas w formie tekstowej
      
      snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      u8g2.setFont(spleen6x12PL);
      
      if ((displayMode == 0) || (displayMode == 2)) { u8g2.drawStr(0, 63, "                         ");}
      if (displayMode == 1) { u8g2.drawStr(0, 33, "                         ");}

      if (millis() - lastCheckTime >= 1000)
      {
        if ((displayMode == 0) || (displayMode == 2)) { u8g2.drawStr(0, 63, "... No audio stream ! ...");}
        if (displayMode == 1) { u8g2.drawStr(0, 33, "... No audio stream ! ...");}
        lastCheckTime = millis(); // Zaktualizuj czas ostatniego sprawdzenia
      }       
      u8g2.drawStr(208, 63, timeString);

    }
  }
}

void saveEqualizerOnSPIFFS() 
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(1, 33, "Saving equalizer settings"); // 8 znakow  x 11 szer
  u8g2.sendBuffer();
  
  
  // Sprawdź, czy plik equalizer.txt istnieje

  Serial.print("Filtr High: ");
  Serial.println(toneHiValue);
  
  Serial.print("Filtr Mid: ");
  Serial.println(toneMidValue);
  
  Serial.print("Filtr Low: ");
  Serial.println(toneLowValue);
  
  
  // Sprawdź, czy plik istnieje
  if (SPIFFS.exists("/equalizer.txt")) 
  {
    Serial.println("Plik equalizer.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość flitrów equalizera
    myFile = SPIFFS.open("/equalizer.txt", FILE_WRITE);
    if (myFile) 
	  {
      myFile.println(toneHiValue);
	    myFile.println(toneMidValue);
	    myFile.println(toneLowValue);
      myFile.close();
      Serial.println("Aktualizacja equalizer.txt na karcie SPIFFS");
    } 
	  else 
	  {
      Serial.println("Błąd podczas otwierania pliku equalizer.txt.");
    }
  } 
  else 
  {
    Serial.println("Plik equalizer.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualną wartość filtrów equalizera
    myFile = SPIFFS.open("/equalizer.txt", FILE_WRITE);
    if (myFile) 
	  {
      myFile.println(toneHiValue);
	    myFile.println(toneMidValue);
	    myFile.println(toneLowValue);
      myFile.close();
      Serial.println("Utworzono i zapisano equalizer.txt na karcie SPIFFS");
    } 
	  else 
	  {
      Serial.println("Błąd podczas tworzenia pliku equalizer.txt.");
    }
  }
}

void readEqualizerFromSPIFFS() 
{
  // Sprawdź, czy karta SPIFFS jest dostępna
  if (!SPIFFS.begin()) 
  {
    Serial.println("Nie można znaleźć karty SPIFFS, ustawiam domyślne wartości filtrow Equalziera.");
    toneHiValue = 0;  // Domyślna wartość filtra gdy brak karty SPIFFS
	  toneMidValue = 0; // Domyślna wartość filtra gdy brak karty SPIFFS
	  toneLowValue = 0; // Domyślna wartość filtra gdy brak karty SPIFFS
    return;
  }

  // Sprawdź, czy plik equalizer.txt istnieje
  if (SPIFFS.exists("/equalizer.txt")) 
  {
    myFile = SPIFFS.open("/equalizer.txt");
    if (myFile) 
    {
      toneHiValue = myFile.parseInt();
	    toneMidValue = myFile.parseInt();
	    toneLowValue = myFile.parseInt();
      myFile.close();
      
      Serial.println("Wczytano equalizer.txt z karty SPIFFS: ");
	  
      Serial.print("Filtr High equalizera odczytany z SPIFFS: ");
      Serial.println(toneHiValue);
    
      Serial.print("Filtr Mid equalizera odczytany z SPIFFS: ");
      Serial.println(toneMidValue);
    
      Serial.print("Filtr Low equalizera odczytany z SPIFFS: ");
      Serial.println(toneLowValue);
	  
      
    } 
	  else 
	  {
      Serial.println("Błąd podczas otwierania pliku equalizer.txt.");
    }
  } 
  else 
  {
    Serial.println("Plik equalizer.txt nie istnieje.");
    toneHiValue = 0;  // Domyślna wartość filtra gdy brak karty SPIFFS
	  toneMidValue = 0; // Domyślna wartość filtra gdy brak karty SPIFFS
	  toneLowValue = 0; // Domyślna wartość filtra gdy brak karty SPIFFS
  }
  audio.setTone(toneLowValue, toneMidValue, toneHiValue); // Ustawiamy filtry - zakres regulacji -40 + 6dB jako int8_t ze znakiem
}

// Funkcja do odczytu danych stacji radiowej z karty SPIFFS
void readStationFromSPIFFS() {
  // Sprawdź, czy karta SPIFFS jest dostępna
  if (!SPIFFS.begin()) {
    //Serial.println("Nie można znaleźć karty SPIFFS. Ustawiam domyślne wartości: Station=1, Bank=1.");
    Serial.println("Nie można znaleźć karty SPIFFS, ustawiam wartości z EEPROMu");
    //station_nr = 1;  // Domyślny numer stacji gdy brak karty SPIFFS
    //bank_nr = 1;     // Domyślny numer banku gdy brak karty SPIFFS
    EEPROM.get(0, station_nr);
    EEPROM.get(1, bank_nr);
    
    Serial.print("Odczyt EEPROM Stacja: ");
    Serial.println(station_nr);
    Serial.print("Odczyt EEPROM Bank: ");
    Serial.println(bank_nr);

    if ((station_nr > 99) || (station_nr == 0)) { station_nr = 1;} // zabezpiecznie na wypadek błędnego odczytu EEPROMu lub wartości
    if ((bank_nr > 16) || (bank_nr == 0)) { bank_nr = 1;}
    
    return;
  }

  // Sprawdź, czy plik station_nr.txt istnieje
  if (SPIFFS.exists("/station_nr.txt")) {
    myFile = SPIFFS.open("/station_nr.txt");
    if (myFile) {
      station_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano station_nr z karty SPIFFS: ");
      Serial.println(station_nr);
    } else {
      Serial.println("Błąd podczas otwierania pliku station_nr.txt.");
    }
  } else {
    Serial.println("Plik station_nr.txt nie istnieje.");
    station_nr = 9;  // ustawiamy stacje w przypadku braku pliku na karcie
  }

  // Sprawdź, czy plik bank_nr.txt istnieje
  if (SPIFFS.exists("/bank_nr.txt")) {
    myFile = SPIFFS.open("/bank_nr.txt");
    if (myFile) {
      bank_nr = myFile.parseInt();
      myFile.close();
      Serial.print("Wczytano bank_nr z karty SPIFFS: ");
      Serial.println(bank_nr);
    } else {
      Serial.println("Błąd podczas otwierania pliku bank_nr.txt.");
    }
  } else {
    Serial.println("Plik bank_nr.txt nie istnieje.");
    bank_nr = 1;  // // ustawiamy bank w przypadku braku pliku na karcie
  }
}

void vuMeter() 
{
  vuMeterR = min(audio.getVUlevel() & 0xFF, 250);  // wyciagamy ze zmiennej typu int16 kanał L
  vuMeterL = min(audio.getVUlevel() >> 8, 250);  // z wyzszej polowki wyciagamy kanal P

  // Aktualizacja peak&hold dla Lewego kanału
  if (vuMeterL >= peakL) 
  {
    peakL = vuMeterL;
    peakHoldTimeL = 0;
  } 
  else 
  {
    if (peakHoldTimeL < peakHoldThreshold) 
    {
      peakHoldTimeL++;
    } 
    else 
    {
      if (peakL > 0) peakL--;
    }
  }

  // Aktualizacja peak&hold dla Prawego kanału
  if (vuMeterR >= peakR) 
  {
    peakR = vuMeterR;
    peakHoldTimeR = 0;
  } 
  else 
  {
    if (peakHoldTimeR < peakHoldThreshold) 
    {
      peakHoldTimeR++;
    } 
    else 
    {
      if (peakR > 0) peakR--;
    }
  }

  if (volumeMute == false)  
  {
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, vuLy, 256, 3);  //czyszczenie ekranu pod VU meter
    u8g2.drawBox(0, vuRy, 256, 3);
    u8g2.setDrawColor(1);

    // Biale pola pod literami L i R
    u8g2.drawBox(0, vuLy - 3, 7, 7);  
    u8g2.drawBox(0, vuRy - 3, 7, 7);  

    // Rysujemy litery L i R
    u8g2.setDrawColor(0);
    u8g2.setFont(u8g2_font_04b_03_tr);
    u8g2.drawStr(2, vuLy + 3, "L");
    u8g2.drawStr(2, vuRy + 3, "R");
    u8g2.setDrawColor(1);  // Przywracamy białe rysowanie

    if (vuMeterMode == 1)  // tryb 1 ciagle paski
    {
      u8g2.setDrawColor(1);
      u8g2.drawBox(10, vuLy, vuMeterL, 2);  // rysujemy kreseczki o dlugosci odpowiadajacej wartosci VU
      u8g2.drawBox(10, vuRy, vuMeterR, 2);

      // Rysowanie peaków jako cienka kreska
      u8g2.drawBox(9 + peakL, vuLy, 1, 2);
      u8g2.drawBox(9 + peakR, vuRy, 1, 2);
    } 
    else  // vuMeterMode == 0  tryb podstawowy, kreseczki z przerwami
    { 
      for (uint8_t vusize = 0; vusize < vuMeterL; vusize++) 
      {
        if ((vusize % 9) < 8) u8g2.drawBox(9 + vusize, vuLy, 1, 2); // rysuj tylko 8 pikseli, potem 1px przerwy
      }  
	   
      for (uint8_t vusize = 0; vusize < vuMeterR; vusize++) 
      {
        if ((vusize % 9) < 8) u8g2.drawBox(9 + vusize, vuRy, 1, 2); // rysuj tylko 8 pikseli, potem 1px przerwy
      } 

      // Peak - kreski w trybie przerywanym
      u8g2.drawBox(9 + peakL, vuLy, 1, 2);
      u8g2.drawBox(9 + peakR, vuRy, 1, 2);

    }
  }   
}

void displayClearUnderScroller() // Funkcja odpwoiedzialna za przewijanie informacji strem tittle lub stringstation
{
  if (displayMode == 0) // Tryb normalny Mode 0- radio
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(0,yPositionDisplayScrollerMode0, "                                           "); //43 spacje - czyszczenie ekranu   
    //u8g2.setDrawColor(0);
    //u8g2.drawBox(0,yPositionDisplayScrollerMode0,255,12);
    //u8g2.setDrawColor(1);
  } 
  else if (displayMode == 1)  // Tryb zegara - Mode 1
  {
    u8g2.drawStr(0,yPositionDisplayScrollerMode1, "                                           "); //43 znaki czyszczenie ekranu
  }
  else if (displayMode == 2)  // Tryb mały tekst - Mode 2
  {
    u8g2.setDrawColor(1);
    u8g2.setFont(spleen6x12PL);   
    u8g2.drawStr(0,yPositionDisplayScrollerMode2, "                                           "); //43 znaki czyszczenie ekranu
    u8g2.drawStr(0,yPositionDisplayScrollerMode2 + 12, "                                           "); //43 znaki czyszczenie ekranu
    u8g2.drawStr(0,yPositionDisplayScrollerMode2 + 12 + 12, "                                           "); //43 znaki czyszczenie ekranu
  }
  u8g2.sendBuffer();  // rysujemy całą zawartosc ekranu.  
}

void displayRadioScroller() // Funkcja odpwoiedzialna za przewijanie informacji strem tittle lub stringstation
{
  // Jesli zmieniła sie dlugosc wyswietlanego stationString to wyczysc ekran OLED w miescach Scrollera
  if (stationStringScroll.length() != stationStringScrollLength) 
  {
    //Serial.print("debug -- czyscimy obszar scrollera stationStringScrollLength - stara wartosc: ");
    //Serial.print(stationStringScrollLength);
    //Serial.print(" <-> nowa wartosc: ");
    //Serial.println(stationStringScroll.length());
    
    stationStringScrollLength = stationStringScroll.length();
    displayClearUnderScroller();
  }

  if (displayMode == 0) // Tryb normalny - Mode 0, radio + VUmeter
  {
    if (stationStringScroll.length() > maxStationVisibleStringScrollLength) //42 + 4 znaki spacji separatora. Realnie widzimy 42 znaki
    {    
      xPositionStationString = offset;
      u8g2.setFont(spleen6x12PL);
      u8g2.setDrawColor(1);
      do {
        u8g2.drawStr(xPositionStationString, yPositionDisplayScrollerMode0, stationStringScroll.c_str());
        xPositionStationString = xPositionStationString + stationStringScrollWidth;
      } while (xPositionStationString < 256);
      
      offset = offset - 1;
      if (offset < (65535 - stationStringScrollWidth)) { 
        offset = 0;
      }

    } else {
      xPositionStationString = 0;
      u8g2.setDrawColor(1);
      u8g2.setFont(spleen6x12PL);    
      u8g2.drawStr(xPositionStationString, yPositionDisplayScrollerMode0, stationStringScroll.c_str());
      
    }
  } 
  else if (displayMode == 1)  // Tryb zegara
  {
    
    if (stationStringScroll.length() > maxStationVisibleStringScrollLength) 
    {     
      xPositionStationString = offset;
      u8g2.setFont(spleen6x12PL);
      u8g2.setDrawColor(1);
      do {
        u8g2.drawStr(xPositionStationString, yPositionDisplayScrollerMode1, stationStringScroll.c_str());

        xPositionStationString = xPositionStationString + stationStringScrollWidth;
      } while (xPositionStationString < 256);
      
      offset = offset - 1;
      if (offset < (65535 - stationStringScrollWidth)) {
        offset = 0;
      }

    } 
    else 
    {
      xPositionStationString = 0;
      u8g2.setDrawColor(1);
      u8g2.setFont(spleen6x12PL);       
      u8g2.drawStr(xPositionStationString, yPositionDisplayScrollerMode1, stationStringScroll.c_str());
    }

  }
  else if (displayMode == 2)  // Tryb Mode 2, Radio, 3 linijki station tekst
  {

    // Parametry do obługi wyświetlania w 3 kolejnych wierszach z podzialem do pełnych wyrazów
    const int maxLineLength = 41;  // Maksymalna długość jednej linii w znakach
    String currentLine = "";       // Bieżąca linia
    int yPosition = yPositionDisplayScrollerMode2; // Początkowa pozycja Y


    // Podziel tekst na wyrazy
    String word;
    int wordStart = 0;

    for (int i = 0; i <= stationStringScroll.length(); i++)
    {
      // Sprawdź, czy dotarliśmy do końca słowa lub do końca tekstu
      if (i == stationStringScroll.length() || stationStringScroll.charAt(i) == ' ')
      {
        // Pobierz słowo
        String word = stationStringScroll.substring(wordStart, i);
        wordStart = i + 1;

        // Sprawdź, czy dodanie słowa do bieżącej linii nie przekroczy maxLineLength
        if (currentLine.length() + word.length() <= maxLineLength)
        {
          // Dodaj słowo do bieżącej linii
          if (currentLine.length() > 0)
          {
            currentLine += " ";  // Dodaj spację między słowami
          }
          currentLine += word;
        }
        else
        {
          // Jeśli słowo nie pasuje, wyświetl bieżącą linię i przejdź do nowej linii
          u8g2.setFont(spleen6x12PL);
          u8g2.drawStr(0, yPosition, currentLine.c_str());
          yPosition += 12;  // Przesunięcie w dół dla kolejnej linii
          // Zresetuj bieżącą linię i dodaj nowe słowo
          currentLine = word;
        }
      }
    }
    // Wyświetl ostatnią linię, jeśli coś zostało
    if (currentLine.length() > 0)
    {
      u8g2.setFont(spleen6x12PL);
      u8g2.drawStr(0, yPosition, currentLine.c_str());
    }
  }
}

//void handleKeyboard(void* pvParameters)
void handleKeyboard()
{
  //for (;;) {
  // vTaskDelay(50);
  //int keyboardValue = analogRead(keyboardPin); // Odczyt wartości analogowej przycisku
  uint8_t key = 17;
  
  
  for (int i = 0; i < 32; i++)
  {
    keyboardValue = keyboardValue + analogRead(keyboardPin);
  }
  keyboardValue = keyboardValue / 32;  
  

  if (debugKeyboard == 1) 
  { 
    displayActive = true;
    displayStartTime = millis();
    u8g2.clearBuffer();
    u8g2.setCursor(10,10); u8g2.print("ADC:   " + String(keyboardValue));
    u8g2.setCursor(10,23); u8g2.print("BUTTON:" + String(keyboardButtonPressed));
    u8g2.sendBuffer(); 
    Serial.print("debug - ADC odczyt: ");
    Serial.print(keyboardValue);
    Serial.print(" flaga przycisk wcisniety:");
    Serial.println(keyboardButtonPressed);
  }
  // Kasujemy flage nacisnietego przycisku tylko jesli nic nie jest wcisnięte
  if (keyboardValue > keyboardButtonNeutral-keyboardButtonThresholdTolerance)
  {  
    keyboardButtonPressed = false;
  }
  // Jesli nic nie jest wcisniete i nic nie było nacisnięte analizujemy przycisk
  if (keyboardValue < keyboardButtonNeutral-keyboardButtonThresholdTolerance)
  {  
    if (keyboardButtonPressed == false)
    {
      // Sprawdzamy stan klawiatury
      if ((keyboardValue <= keyboardButtonThreshold_1 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_1 - keyboardButtonThresholdTolerance ))
      { key=1;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_2 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_2 - keyboardButtonThresholdTolerance ))
      { key=2;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_3 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_3 - keyboardButtonThresholdTolerance ))
      { key=3;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_4 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_4 - keyboardButtonThresholdTolerance ))
      { key=4;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_5 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_5 - keyboardButtonThresholdTolerance ))
      { key=5;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_6 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_6 - keyboardButtonThresholdTolerance ))
      { key=6;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_7 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_7 - keyboardButtonThresholdTolerance ))
      { key=7;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_8 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_8 - keyboardButtonThresholdTolerance ))
      { key=8;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_9 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_9 - keyboardButtonThresholdTolerance ))
      { key=9;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_0 + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_0 - keyboardButtonThresholdTolerance ))
      { key=0;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_Memory + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_Memory - keyboardButtonThresholdTolerance ))
      { key=11;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_Shift + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_Shift - keyboardButtonThresholdTolerance ))
      { key=12;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_Auto + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_Auto - keyboardButtonThresholdTolerance ))
      { key=13;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_Band + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_Band - keyboardButtonThresholdTolerance ))
      { key=14;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_Mute + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_Mute - keyboardButtonThresholdTolerance ))
      { key=15;keyboardButtonPressed = true;}

      if ((keyboardValue <= keyboardButtonThreshold_Scan + keyboardButtonThresholdTolerance ) && (keyboardValue >= keyboardButtonThreshold_Scan - keyboardButtonThresholdTolerance ))
      { key=16;keyboardButtonPressed = true;}

      if ((key < 17) && (keyboardButtonPressed == true))
      {
        Serial.print("ADC odczyt poprawny: ");
        Serial.print(keyboardValue);
        Serial.print(" przycisk: ");
        Serial.println(key);
        keyboardValue = 0;
     
        if ((debugKeyboard == false) && (key < 10)) // Dla przyciskoq 0-9 wykonujemy akcje jak na pilocie
        { rcInputKey(key);}
        else if ((debugKeyboard == false) && (key == 11)) // Przycisk Memory wywyłuje menu wyboru Banku
        { bankMenuDisplay();}
        else if ((debugKeyboard == false) && (key == 12)) // Przycisk Shift zatwierdza zmiane stacji, banku, działa jak "OK" na pilocie
        { 
         ir_code = rcCmdOk; // Przycisk Auto TUning udaje OK
          bit_count = 32;
          calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC
        }
        else if ((debugKeyboard == false) && (key == 13)) // Przycisk Auto - przełaczanie tryb Zegar/Radio
        { 
          ir_code = rcCmdSrc; // Przycisk Auto TUning udaje Src
          bit_count = 32;
          calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC
        }
        else if ((debugKeyboard == false) && (key == 14)) // Przycisk Band udaje Back
        {  
          ir_code = rcCmdBack; // Udajemy komendy pilota
          bit_count = 32;
          calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC
        }
        else if ((debugKeyboard == false) && (key == 15)) // Przycisk Mute
        { 
          ir_code = rcCmdMute; // Przypisujemy kod polecenia z pilota
          bit_count = 32; // ustawiamy informacje, ze mamy pelen kod NEC do analizy 
          calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC     
        }
        else if ((debugKeyboard == false) && (key == 16)) // Przycisk Memory Scan - zmiana janości wyswietlacza - funkcja "Dimmer"
        { 
          ir_code = rcCmdDirect; // Udajemy komendy pilota
          bit_count = 32;
          calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC
        }


        key=17; // Reset "KEY" do pozycji poza zakresem klawiatury
      }
    }
  } 
  //  }
  // vTaskDelete(NULL);
}


void volumeDisplay()
{
  //volumeBufferValue = volumeValue;
  displayStartTime = millis();
  timeDisplay = false;
  displayActive = true;
  volumeSet = true;
  //volumeMute = false;
  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  audio.setVolume(volumeValue);  // zakres 0...21
  String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
  u8g2.clearBuffer();
  //u8g2.setFont(DotMatrix13pl);
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(65, 33, "VOLUME");
  u8g2.drawStr(163, 33, volumeValueStr.c_str());

  u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
  if (maxVolume == 42) { u8g2.drawRBox(23, 44, volumeValue * 5, 10, 2);}  // Progress bar głosnosci
  if (maxVolume == 21) { u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);}  // Progress bar głosnosci
  u8g2.sendBuffer();
  wsVolumeChange(volumeValue); // wyślij aktualizację przez WebSocket
  }



void volumeUp()
{
  volumeSet = true;
  timeDisplay = false;
  displayActive = true;
  volumeMute = false;
  displayStartTime = millis();   
  volumeValue++;

  if (volumeValue > maxVolume) 
  {
    volumeValue = maxVolume;
  }

  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  audio.setVolume(volumeValue);  // zakres 0...21
  String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(65, 33, "VOLUME");
  u8g2.drawStr(163, 33, volumeValueStr.c_str());
  u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
  if (maxVolume == 42) { u8g2.drawRBox(23, 44, volumeValue * 5, 10, 2);}  // Progress bar głosnosci
  if (maxVolume == 21) { u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);}  // Progress bar głosnosci
  u8g2.sendBuffer();
  wsVolumeChange(volumeValue); // wyślij aktualizację przez WebSocket
}

void volumeDown()
{
  volumeSet = true;
  timeDisplay = false;
  displayActive = true;
  volumeMute = false;
  displayStartTime = millis();
  volumeValue--;
  if (volumeValue < 1) 
  {
    volumeValue = 1;
  } 
  Serial.print("Wartość głośności: ");
  Serial.println(volumeValue);
  audio.setVolume(volumeValue);  // zakres 0...21
  String volumeValueStr = String(volumeValue);  // Zamiana liczby VOLUME na ciąg znaków
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(65, 33, "VOLUME");
  u8g2.drawStr(163, 33, volumeValueStr.c_str());
  u8g2.drawRFrame(21, 42, 214, 14, 3);             // Rysujmey ramke dla progress bara głosnosci
  if (maxVolume == 42) { u8g2.drawRBox(23, 44, volumeValue * 5, 10, 2);}  // Progress bar głosnosci
  if (maxVolume == 21) { u8g2.drawRBox(23, 44, volumeValue * 10, 10, 2);}  // Progress bar głosnosci
  u8g2.sendBuffer();
  wsVolumeChange(volumeValue); // wyślij aktualizację przez WebSocket
}

void clearFlags()  // Kasuje wszystkie flagi przebywania w menu, funkcjach itd. Pozwala pwrócic do wyswietlania ekranu głownego
{
  //displayDimmer(0);
  debugKeyboard = false;
  //menuEnable = false;
  displayActive = false;
  timeDisplay = true;
  listedStations = false;
  volumeSet = false;
  bankMenuEnable = false;
  bankNetworkUpdate = false;
  equalizerMenuEnable = false;
  rcInputDigitsMenuEnable = false;
  rcInputDigit1 = 0xFF; // czyscimy cyfre 1, flaga pustej zmiennej to FF
  rcInputDigit2 = 0xFF; // czyscimy cyfre 2, flaga pustej zmiennej to FF
  station_nr = stationFromBuffer;
  bank_nr = previous_bank_nr;  
}


void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) 
{
  if (type == WS_EVT_CONNECT) 
  {
    Serial.println("WebSocket klient podlaczony");

    client->text("station:" + String(station_nr));
    client->text("stationname:" + stationName.substring(0, stationNameLenghtCut));
    client->text("volume:" + String(volumeValue)); 
    client->text("bank:" + String(bank_nr));
    if (audio.isRunning() == true)
    {
       client->text("stationtext$" + stationStringWeb);
    }
    else
    {
     client->text("stationtext$... No audio stream ! ...");
    }
    
    client->text("bitrate:" + String(bitrateString)); 
    client->text("samplerate:" + String(sampleRateString)); 
    client->text("bitpersample:" + String(bitsPerSampleString)); 

  if (mp3 == true) {client->text("streamformat:MP3"); }
  if (flac == true) {client->text("streamformat:FLAC"); }
  if (aac == true) {client->text("streamformat:AAC"); }
  if (vorbis == true) {client->text("streamformat:VRB"); }
  if (opus == true) {client->text("streamformat:OPUS"); }  
  } 
  else if (type == WS_EVT_DATA) 
  {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) 
    {
      String msg = "";
      for (size_t i = 0; i < len; i++) 
      {
        msg += (char) data[i];
      }

      Serial.println("Odebrano WS: " + msg);

      if (msg.startsWith("volume:")) 
      {
        int newVolume = msg.substring(7).toInt();
        volumeValue = newVolume;
        volumeDisplay();   // wyswietle wartosci na OLED i aktualizacja obiektu audio
      }
      else if (msg.startsWith("station:")) 
      {
        int newStation = msg.substring(8).toInt();
        station_nr = newStation;
        
        ir_code = rcCmdOk; // Przypisujemy kod polecenia z pilota
        bit_count = 32; // ustawiamy informacje, ze mamy pelen kod NEC do analizy 
        calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC    
      }
      else if (msg.startsWith("bank:")) 
      {
        int newBank = msg.substring(5).toInt();
        bank_nr = newBank;
        
        //bankMenuEnable = true;        
        //displayStartTime = millis();
        //timeDisplay = false;
        
        bankMenuDisplay();
        fetchStationsFromServer();
        clearFlags();
        
        station_nr = 1;
        
        //Zatwirdzenie zmiany stacji
        ir_code = rcCmdOk; // Przypisujemy kod polecenia z pilota
        bit_count = 32; // ustawiamy informacje, ze mamy pelen kod NEC do analizy 
        calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC    
        
      }
      else if (msg.startsWith("displayMode")) 
      {
        ir_code = rcCmdSrc; // Udajemy kod pilota SRC - zmiana trybu wyswietlacza 
        bit_count = 32;
        calcNec();          // Przeliczamy kod pilota na pełny oryginalny kod NEC
      }

    }
  }

}

void bufforAudioInfo()
{
  Serial.print("debug--Bufor Audio pojemność / zapełniony:");
  Serial.print(audio.inBufferSize());
  Serial.print(" / ");
  Serial.println(audio.inBufferFilled());
  //Serial.println(audio.inBufferFree());
  //Serial.println(audio.inBufferSize());
  //Serial.println(audio.inBufferSize() - audio.inBufferFilled());
}

// Funkcja obsługująca przerwanie (reakcja na zmianę stanu pinu)
void IRAM_ATTR pulseISR()
{
  if (digitalRead(recv_pin) == HIGH)
  {
    pulse_start_high = micros();  // Zapis początku impulsu
  }
  else
  {
    pulse_end_high = micros();    // Zapis końca impulsu
    pulse_ready = true;
  }

  if (digitalRead(recv_pin) == LOW)
  {
    pulse_start_low = micros();  // Zapis początku impulsu
  }
  else
  {
    pulse_end_low = micros();    // Zapis końca impulsu
    pulse_ready_low = true;
  }

   // ----------- ANALIZA PULSOW -----------------------------
  if (pulse_ready_low) // spradzamy czy jest stan niski przez 9ms - start ramki
  {
    pulse_duration_low = pulse_end_low - pulse_start_low;
  
    if (pulse_duration_low > (LEAD_HIGH - TOLERANCE) && pulse_duration_low < (LEAD_HIGH + TOLERANCE))
    {
      pulse_duration_9ms = pulse_duration_low; // przypisz czas trwania puslu Low do zmiennej puls 9ms
      pulse_ready9ms = true; // flaga poprawnego wykrycia pulsu 9ms w granicach tolerancji
    }

  }

  // Sprawdzenie, czy impuls jest gotowy do analizy
  if ((pulse_ready== true) && (pulse_ready9ms = true))
  {
    pulse_ready = false;
    pulse_ready9ms = false; // kasujemy flage wykrycia pulsu 9ms

    // Obliczenie czasu trwania impulsu
    pulse_duration = pulse_end_high - pulse_start_high;
    //Serial.println(pulse_duration); odczyt dlugosci pulsow z pilota - debug
    if (!data_start_detected)
    {
    
      // Oczekiwanie na sygnał 4,5 ms wysoki
      if (pulse_duration > (LEAD_LOW - TOLERANCE) && pulse_duration < (LEAD_LOW + TOLERANCE))
      {
        pulse_duration_4_5ms = pulse_duration;
        // Początek sygnału: 4,5 ms wysoki
        
        data_start_detected = true;  // Ustawienie flagi po wykryciu sygnału wstępnego
        bit_count = 0;               // Reset bit_count przed odebraniem danych
        ir_code = 0;                 // Reset kodu IR przed odebraniem danych
      }
    }
    else
    {
      // Sygnały dla bajtów (adresu ADDR, IADDR, komendy CMD, ICMD) zaczynają się po wstępnym sygnale
      if (pulse_duration > (HIGH_THRESHOLD - TOLERANCE) && pulse_duration < (HIGH_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 1;  // Dodanie "1" do kodu IR
        bit_count++;
        pulse_duration_1690us = pulse_duration;
       

      }
      else if (pulse_duration > (LOW_THRESHOLD - TOLERANCE) && pulse_duration < (LOW_THRESHOLD + TOLERANCE))
      {
        ir_code = (ir_code << 1) | 0;  // Dodanie "0" do kodu IR
        bit_count++;
        pulse_duration_560us = pulse_duration;
       
      }

      // Sprawdzenie, czy otrzymano pełny 32-bitowy kod IR
      if (bit_count == 32)
      {
        // Rozbicie kodu na 4 bajty
        uint8_t ADDR = (ir_code >> 24) & 0xFF;  // Pierwszy bajt
        uint8_t IADDR = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
        uint8_t CMD = (ir_code >> 8) & 0xFF;    // Trzeci bajt (komenda)
        uint8_t ICMD = ir_code & 0xFF;          // Czwarty bajt (inwersja komendy)

        // Sprawdzenie poprawności (inwersja) bajtów adresu i komendy
        if ((ADDR ^ IADDR) == 0xFF && (CMD ^ ICMD) == 0xFF)
        {
          data_start_detected = false;
          //bit_count = 0;
        }
        else
        {
          ir_code = 0; 
          data_start_detected = false;
          //bit_count = 0;        
        }

      }
    }
  }
 //runTime2 = esp_timer_get_time();
}

void displayEqualizer() // Funkcja rysująca menu 3-punktowego equalizera
{

  displayStartTime = millis();  // Uaktulniamy czas dla funkcji auto-pwrotu z menu
  equalizerMenuEnable = true;   // Ustawiamy flage menu equalizera
  timeDisplay = false;          // Wyłaczamy zegar
  displayActive = true;         // Wyswietlacz aktywny
  
  Serial.println("--Equalizer--");
  Serial.print("Wartość tonów Niskich/Low:   ");
  Serial.println(toneLowValue);
  Serial.print("Wartość tonów Średnich/Mid:  ");
  Serial.println(toneMidValue);
  Serial.print("Wartość tonów Wysokich/High: ");
  Serial.println(toneHiValue);
    
  audio.setTone(toneLowValue, toneMidValue, toneHiValue); // Zakres regulacji -40 + 6dB jako int8_t ze znakiem

  u8g2.setDrawColor(1);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(60, 14, "EQUALIZER");
  //u8g2.drawStr(1, 14, "EQUALIZER");
  u8g2.setFont(spleen6x12PL);
  //u8g2.setCursor(138,12);
  //u8g2.print("P1    P2    P3    P4");
  uint8_t xTone;
  uint8_t yTone;  
  
  // ---- Tony Wysokie ----
  xTone=0; 
  yTone=28;
  u8g2.setCursor(xTone,yTone);
  if (toneHiValue >= 10) {u8g2.print("High: " + String(toneHiValue) + "dB");}  // piszemy o 1 znak wczesniej
  if ((toneHiValue >= 0) && (toneHiValue <= 9)) {u8g2.print("High:  " + String(toneHiValue) + "dB");}  // piszemy o 2 znak wczesniej
  if ((toneHiValue < 0) && (toneHiValue >= -9)) {u8g2.print("High: " + String(toneHiValue) + "dB");}    // piszemy o 1 znak wczesniej
  if (toneHiValue <= -10) {u8g2.print("High:" + String(toneHiValue) + "dB");}  // piszemy normalnie
  
  xTone=20;
  if (toneSelect == 1) 
  { 
    u8g2.drawRBox(xTone+56,yTone-9,154,9,1); // Rysujemy biały pasek pod regulacją danego tonu  
    u8g2.setDrawColor(0); 
    u8g2.drawLine(xTone+57,yTone-5,xTone+208,yTone-5);
    //u8g2.drawLine(xTone+57,yTone-4,xTone+208,yTone-4);
    if (toneHiValue >= 0){ u8g2.drawRBox((5 * toneHiValue) + xTone + 138,yTone-7,10,5,1);}
    if (toneHiValue < 0) { u8g2.drawRBox((2 * toneHiValue) + xTone + 138,yTone-7,10,5,1);}
    u8g2.setDrawColor(1);
  }
  else 
  {
    u8g2.drawLine(xTone+57,yTone-5,xTone+208,yTone-5);
    //u8g2.drawLine(xTone+57,yTone-4,xTone+208,yTone-4);
    if (toneHiValue >= 0){ u8g2.drawRBox((5 * toneHiValue) + xTone + 138,yTone-7,10,5,1);}
    if (toneHiValue < 0) { u8g2.drawRBox((2 * toneHiValue) + xTone + 138,yTone-7,10,5,1);}
    //u8g2.drawRBox((3 * toneHiValue) + xTone + 178,yTone-7,10,6,1);
  }

  // ---- Tony średnie ----
  xTone=0;
  yTone=46;
  u8g2.setCursor(xTone,yTone);
  if (toneMidValue >= 10) {u8g2.print("Mid:  " + String(toneMidValue) + "dB");}  // Dla wartosci dodatnich piszemy normalnie
  if ((toneMidValue >= 0) && (toneMidValue <= 9)) {u8g2.print("Mid:   " + String(toneMidValue) + "dB");}
  if ((toneMidValue < 0) && (toneMidValue >= -9)) {u8g2.print("Mid:  " + String(toneMidValue) + "dB");}
  if (toneMidValue <= -10) {u8g2.print("Mid: " + String(toneMidValue) + "dB");} 
  
  xTone=20;
  if (toneSelect == 2) 
  { 
    u8g2.drawRBox(xTone+56,yTone-9,154,9,1);
    u8g2.setDrawColor(0);
    u8g2.drawLine(xTone+57,yTone-5,xTone + 208,yTone-5);
    //u8g2.drawLine(xTone+57,yTone-4,xTone + 208,yTone-4);
    if (toneMidValue >= 0) { u8g2.drawRBox((5 * toneMidValue) + xTone + 138,yTone-7,10,5,1);}
    if (toneMidValue < 0) { u8g2.drawRBox((2 * toneMidValue) + xTone + 138,yTone-7,10,5,1);}
    u8g2.setDrawColor(1);
  }
  else
  {
    u8g2.drawLine(xTone+57,yTone-5,xTone + 208,yTone-5);
    //u8g2.drawLine(xTone+57,yTone-4,xTone + 208,yTone-4);
    if (toneMidValue >= 0) { u8g2.drawRBox((5 * toneMidValue) + xTone + 138,yTone-7,10,5,1);}
    if (toneMidValue < 0)  { u8g2.drawRBox((2 * toneMidValue) + xTone + 138,yTone-7,10,5,1);}
    //u8g2.drawRBox((3 * toneMidValue) + xTone + 138,yTone-7,10,6,1);
  }


  // Tony niskie
  xTone=0; 
  yTone=64;
  u8g2.setCursor(xTone,yTone);
  if (toneLowValue >= 10) { u8g2.print("Low:  " + String(toneLowValue) + "dB");}
  if ((toneLowValue >= 0) && (toneLowValue <= 9)) { u8g2.print("Low:   " + String(toneLowValue) + "dB");}
  if ((toneLowValue < 0) && (toneLowValue >= -9)) { u8g2.print("Low:  " + String(toneLowValue) + "dB");}
  if (toneLowValue <= -10) { u8g2.print("Low: " + String(toneLowValue) + "dB");}
  xTone=20;
  if (toneSelect == 3) 
  { 
    u8g2.drawRBox(xTone+56,yTone-9,154,9,1);
    u8g2.setDrawColor(0);
    u8g2.drawLine(xTone + 57,yTone-5,xTone + 208,yTone-5);
   // u8g2.drawLine(xTone + 57,yTone-4,xTone + 208,yTone-4); 
    if ( toneLowValue >= 0 ) { u8g2.drawRBox((5 * toneLowValue) + xTone + 138,yTone-7,10,5,1);}
    if ( toneLowValue < 0 )  { u8g2.drawRBox((2 * toneLowValue) + xTone + 138,yTone-7,10,5,1);}
    u8g2.setDrawColor(1);
  }
  else
  {
    u8g2.drawLine(xTone + 57,yTone-5,xTone + 208,yTone-5);
    //u8g2.drawLine(xTone + 57,yTone-4,xTone + 208,yTone-4);  
    if ( toneLowValue >= 0 ) { u8g2.drawRBox((5 * toneLowValue) + xTone + 138,yTone-7,10,5,1);}
    if ( toneLowValue < 0 )  { u8g2.drawRBox((2 * toneLowValue) + xTone + 138,yTone-7,10,5,1);}
    //u8g2.drawRBox((3 * toneLowValue) + xTone + 138,yTone-7,10,6,1);
  }
  u8g2.sendBuffer(); 
}

void displayBasicInfo()
{
  displayStartTime = millis();  // Uaktulniamy czas dla funkcji auto-powrotu z menu
  timeDisplay = false;          // Wyłaczamy zegar
  displayActive = true;         // Wyswietlacz aktywny
  u8g2.clearBuffer();
  u8g2.setFont(spleen6x12PL);
  u8g2.drawStr(0, 10, "Info:");
  u8g2.setCursor(0,25); u8g2.print("ESP32 SN:" + String(ESP.getEfuseMac()) + ",  FW Ver.:" + String(softwareRev));
  u8g2.setCursor(0,38); u8g2.print("Hostname:" + String(hostname) + ",  WiFi Signal:" + String(WiFi.RSSI()) + "dBm");
  u8g2.setCursor(0,51); u8g2.print("WiFi SSID:" + String(wifiManager.getWiFiSSID()));
  u8g2.setCursor(0,64); u8g2.print("IP:" + currentIP + "  MAC:" + String(WiFi.macAddress()) );
  
  u8g2.sendBuffer();
}

void audioProcessing(void *p)
{
  while (true) {
  audio.loop();
  vTaskDelay(1 / portTICK_PERIOD_MS); // Opóźnienie 1 milisekundy
  }
}

//  OTA update callback dla Wifi Managera i trybu Recovery Mode
void handlePreOtaUpdateCallback()
{
  Update.onProgress([](unsigned int progress, unsigned int total) 
  {
    u8g2.setCursor(1,56); u8g2.printf("Update: %u%%\r", (progress / (total / 100)) );
    u8g2.setCursor(80,56); u8g2.print(String(progress) + "/" + String(total));
    u8g2.sendBuffer();
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
}

void recoveryModeCheck()
{
  if (digitalRead(SW_PIN2) == 0)
  {
    int8_t recoveryMode = 0;
    u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(1, 14, "RECOVERY / RESET MODE - release encoder");
    u8g2.sendBuffer();
    delay(2000);

    while (digitalRead(SW_PIN2) == 0) {;} // Czekamy na zwolnienie przycisku
    u8g2.clearBuffer();
    u8g2.drawStr(1, 14, "Please Wait...");
    u8g2.sendBuffer();
    delay(1000);
    u8g2.clearBuffer();

    int prev_CLK_state2 = digitalRead(CLK_PIN2);
    unsigned long lastTurnTime = 0;

    while (true)
    {
      // Prosty debounce
      if (millis() - lastTurnTime > 100)
      {
        int CLK_state2 = digitalRead(CLK_PIN2);
        if (CLK_state2 != prev_CLK_state2 && CLK_state2 == LOW)
        {
          if (digitalRead(DT_PIN2) != CLK_state2)
          {
            recoveryMode++;
          }
          else
          {
            recoveryMode--;
          }

          if (recoveryMode > 1) recoveryMode = 0;
          if (recoveryMode < 0) recoveryMode = 1;

          lastTurnTime = millis();
        }
        prev_CLK_state2 = CLK_state2;
      }

      // Wyświetlanie menu
      u8g2.clearBuffer();
      u8g2.drawStr(1, 14, "[ -- Rotate Enckoder -- ]");

      if (recoveryMode == 0)
      {
        u8g2.drawStr(1, 28, ">> RESET BANK=1, STATION=1 <<");
        u8g2.drawStr(1, 42, "   RESET WIFI SSID, PASSWD   ");
      }
      else if (recoveryMode == 1)
      {
        u8g2.drawStr(1, 28, "   RESET BANK=1, STATION=1   ");
        u8g2.drawStr(1, 42, ">> RESET WIFI SSID, PASSWD <<");
      }

      u8g2.sendBuffer();

      // Akcja po naciśnięciu przycisku
      if (digitalRead(SW_PIN2) == 0)
      {
        if (recoveryMode == 0)
        {
          bank_nr = 1;
          station_nr = 1;
          saveStationOnSPIFFS();
          u8g2.clearBuffer();
          u8g2.drawStr(1, 14, "SET BANK=1, STATION=1");
          u8g2.drawStr(1, 28, "ESP will RESET in 3sec.");
          u8g2.sendBuffer();
          delay(3000);
          ESP.restart();
        }
        else if (recoveryMode == 1)
        {
          u8g2.clearBuffer();
          u8g2.drawStr(1, 14, "WIFI SSID, PASSWD CLEARED");
          u8g2.drawStr(1, 28, "ESP will RESET in 3sec.");
          u8g2.sendBuffer();
          wifiManager.resetSettings();
          delay(3000);
          ESP.restart();
        }
      }

      delay(10);  // Lekki delay by zmniejszyć obciążenie CPU
    }
  }
}


void displayDimmer(bool dimmerON)
{
  displayDimmerActive = dimmerON;
  Serial.print("displayDimmerActive: ");
  Serial.println(displayDimmerActive);
  //if ((dimmerON == 1) && (displayBrightness == 15)) { u8g2.sendF("ca", 0xC7, dimmerDisplayBrightness);}
  
  if ((dimmerON == 1) && (displayActive == false) && (fwupd == false)) { u8g2.setContrast(dimmerDisplayBrightness);}
  if (dimmerON == 0) 
  { 
    displayPowerSave(0);
    u8g2.setContrast(displayBrightness); 
    displayDimmerTimeCounter = 0;
    displayPowerSaveTimeCounter = 0;
  }

  //if ((dimmerON == 1) && (displayActive == false) && (fwupd == false)) { u8g2.sendF("ca", 0xC7, dimmerDisplayBrightness);}
  //if (dimmerON == 0) { u8g2.sendF("ca", 0xC7, displayBrightness); displayDimmerTimeCounter = 0;}
}



void displayDimmerTimer()
{
  displayDimmerTimeCounter++;
  
  if ((displayActive == true) && (displayDimmerActive == true)) // Jezeli wysweitlacz aktywny i jest przyciemniony
  { 
    displayDimmerTimeCounter = 0;
    displayDimmer(0);
  }
  // Jesli upłynie czas po którym mamy przyciemnic wyswietlacz i funkcja przyciemniania włączona oraz nie jestemswy w funkcji aktulizacji OTA to
  if ((displayDimmerTimeCounter >= displayAutoDimmerTime) && (displayAutoDimmerOn == true) && (fwupd == false)) 
  {
    if (displayDimmerActive == false) // jesli wyswietlacz nie jest jeszcze przyciemniony
    {
      displayDimmer(1); // wywolujemy funkcje przyciemnienia z parametrem 1 (załacz)
      displayDimmerTimeCounter = 0;
    }
  }

  ///*
  displayPowerSaveTimeCounter++;
  
  if ((displayPowerSaveTimeCounter >= displayPowerSaveTime) && (displayPowerSaveEnabled == true) && (fwupd == false)) 
  {
    displayPowerSaveTimeCounter = 0;
    displayPowerSave(1);
  }
  //*/

  /*
  if (audio.isRunning() == false) // Przyciemniamy wyswietlacz oraz właczamy tryb Power Save tylko jest poprawnie odtwarzaczmy stream audio
  {
    displayPowerSave(0);
    displayDimmer(0);
    displayDimmerTimeCounter = 0;
    displayPowerSaveTimeCounter = 0;
  }
  */

}



void handleEncoder2StationsVolumeClick()
{
  CLK_state2 = digitalRead(CLK_PIN2);                       // Odczytanie aktualnego stanu pinu CLK enkodera 2
  if (CLK_state2 != prev_CLK_state2 && CLK_state2 == HIGH)  // Sprawdzenie, czy stan CLK zmienił się na wysoki
  {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis();

    if ((volumeSet == false) && (bankMenuEnable == false))  // Przewijanie listy stacji radiowych
    {
      station_nr = currentSelection + 1;
      if (digitalRead(DT_PIN2) == HIGH) 
      {
        station_nr--;
        //if (listedStations == 1) station_nr--;
        if (station_nr < 1) 
        {
          station_nr = stationsCount;//1;
        }
        Serial.print("Numer stacji do tyłu: ");
        Serial.println(station_nr);
        scrollUp();
      } 
      else 
      {
        station_nr++;
        //if (listedStations == 1) station_nr++;
        if (station_nr > stationsCount) 
        {
          station_nr = 1;//stationsCount;
        }
        Serial.print("Numer stacji do przodu: ");
        Serial.println(station_nr);
        scrollDown();
      }
      displayStations();
    } 
    else 
    {
      if (bankMenuEnable == false)
      {
        if (digitalRead(DT_PIN2) == HIGH) // pokrecenie enkoderem 2
        {volumeDown();} 
        else 
        {volumeUp();}
        //volumeDisplay();
      }
    }

    if (bankMenuEnable == true)  // Przewijanie listy banków stacji radiowych
    {
      if (digitalRead(DT_PIN2) == HIGH) 
      {
        bank_nr--;
        if (bank_nr < 1) 
        {
          bank_nr = 16;
        }
      } 
      else 
      {
        bank_nr++;
        if (bank_nr > 16) 
        {
          bank_nr = 1;
        }
      }
      bankMenuDisplay();
    }
  }
  prev_CLK_state2 = CLK_state2;
    
  
  if ((button2.isReleased()) && (listedStations == true)) 
  {
    listedStations = false;
    volumeSet = false;
    changeStation();
    displayRadio();
    u8g2.sendBuffer();
    clearFlags();
  }

  if ((button2.isPressed()) && (listedStations == false) && (bankMenuEnable == false)) 
  {
    displayStartTime = millis();
    timeDisplay = false;
    volumeSet = true;
    displayActive = true;
    volumeDisplay();  // Po nacisnieciu enkodera2 wyswietlamy menu głośnosci
  }


  if ((button2.isPressed()) && (bankMenuEnable == true)) 
  {
    previous_bank_nr = bank_nr;

    station_nr = 1;

    fetchStationsFromServer();
    changeStation();
    u8g2.clearBuffer();
    displayRadio();

    volumeSet = false;
    bankMenuEnable = false;
  }

}

void handleEncoder2VolumeStationsClick()
{
  CLK_state2 = digitalRead(CLK_PIN2);                       // Odczytanie aktualnego stanu pinu CLK enkodera 2
  if (CLK_state2 != prev_CLK_state2 && CLK_state2 == HIGH)  // Sprawdzenie, czy stan CLK zmienił się na wysoki
  {
    timeDisplay = false;
    displayActive = true;
    displayStartTime = millis();

    if ((listedStations == false) && (bankMenuEnable == false))  // Przewijanie listy stacji radiowych
    {
      if (digitalRead(DT_PIN2) == HIGH) 
      {
        volumeDown();
      } 
      else 
      {
        volumeUp();
      }
    } 
    else 
    {
      if ((bankMenuEnable == false) && (volumeSet == true)) 
      {
        if (digitalRead(DT_PIN2) == HIGH)  // pokrecenie enkoderem 2
        {
          volumeDown();
        } 
        else 
        {
          volumeUp();
        }
      
      }
    }

    if ((listedStations == true) && (bankMenuEnable == false)) {
      station_nr = currentSelection + 1;
      if (digitalRead(DT_PIN2) == HIGH) 
      {
        station_nr--;
        if (station_nr < 1) { station_nr = stationsCount; }
        scrollUp();
      } 
      else 
      {
        station_nr++;
        if (station_nr > stationsCount) { station_nr = 1; }  //stationsCount;
        scrollDown();
      }
      displayStations();
    }

    if (bankMenuEnable == true)  // Przewijanie listy banków stacji radiowych
    {
      if (digitalRead(DT_PIN2) == HIGH) 
      {
        bank_nr--;
        if (bank_nr < 1) 
        {
          bank_nr = 16;
        }
      } 
      else 
      {
        bank_nr++;
        if (bank_nr > 16) 
        {
          bank_nr = 1;
        }
      }
      bankMenuDisplay();
    }
  }
  prev_CLK_state2 = CLK_state2;




  if ((button2.isReleased()) && (encoderButton2 == true))  // jestesmy juz w menu listy stacji to zmieniamy stacje po nacisnieciu przycisku
  {
    encoderButton2 = false;
    //  Serial.println("debug--------------------------------> SET ENCODER button 2 FALSE");
  }


  if ((button2.isPressed())) // zmieniamy stację
  {
    if ((encoderButton2 == false) && (listedStations == true) && (bankMenuEnable == false) && (volumeSet == false))  // jestesmy juz w menu listy stacji to zmieniamy stacje po nacisnieciu przycisku
    {
                                            
      //timeDisplay = true;
      //listedStations = false;
      //equalizerMenuEnable = false;    
      encoderButton2 = true;
            
      u8g2.clearBuffer();
      changeStation();
      displayRadio();
      clearFlags(); 
      //u8g2.sendBuffer();
      
    }
  
    else if ((encoderButton2 == false) && (listedStations == false) && (bankMenuEnable == false) && (volumeSet == false))  // wchodzimy do listy
    {
      displayStartTime = millis();
      timeDisplay = false;
      displayActive = true;
      listedStations = true;
      currentSelection = station_nr - 1; 

      if (currentSelection >= 0)
      {
        if (currentSelection < firstVisibleLine) // jezeli obecne zaznaczenie ma wartosc mniejsza niz pierwsza wyswietlana linia
        {
          firstVisibleLine = currentSelection;
        }
      }
      else 
      {  // Jeśli osiągnięto wartość 0, przejdź do najwyższej wartości
        if (currentSelection = maxSelection())
        {
        firstVisibleLine = currentSelection - maxVisibleLines + 1;  // Ustaw pierwszą widoczną linię na najwyższą
        }
      }   
      displayStations();
      //Serial.println("debug--------------------------------------------------> button 2 PRESSED station list");
    } 
      
    //else if ((listedStations == false) && (bankMenuEnable == true) && (volumeSet == false)) 
    else if ((bankMenuEnable == true) && (volumeSet == false)) 
    {
      displayStartTime = millis();
      timeDisplay = false;
      displayActive = true;
      
      previous_bank_nr = bank_nr;
      station_nr = 1;

      listedStations = false;
      volumeSet = false;
      bankMenuEnable = false;
      bankNetworkUpdate = false;

      fetchStationsFromServer();
      changeStation();
      u8g2.clearBuffer();
      displayRadio();
      //u8g2.sendBuffer();

    }
  }
}


void drawSwitch(uint8_t x, uint8_t y, bool state) // Ikona przełacznika szeroka (x) na 21, wysoka(y) na 10
{
 u8g2.setFont(u8g2_font_spleen5x8_mf);
 y = y - 9;
 u8g2.drawRFrame(x, y, 21, 10, 1);
 
 if (state == 1)          // Rysujemy przełacznik w pozycji ON z napisem
 {
  u8g2.drawRBox(x + 8, y, 13, 10, 3); 
  u8g2.setDrawColor(0);
  u8g2.drawStr(x + 10,y + 8, "ON");
  u8g2.setDrawColor(1);
 }
  else if (state == 0)   // Rysujemy w pozycji OFF
  {
    u8g2.drawRBox(x, y, 11, 10, 3);  
  }
  u8g2.setFont(spleen6x12PL); // Przywracamy podstawową czcionkę
}

void displayConfig()
{
  displayStartTime = millis();  // Uaktulniamy czas dla funkcji auto-pwrotu z menu
  equalizerMenuEnable = true;   // Ustawiamy flage menu equalizera
  timeDisplay = false;          // Wyłaczamy zegar
  displayActive = true;         // Wyswietlacz aktywny
  
  u8g2.setFont(spleen6x12PL);
  
  // Strona 1
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Menu Config:");
  //drawSwitch(0,15,displayAutoDimmerOn); u8g2.setCursor(25,24); u8g2.print("Display auto dimmer   Auto dimmmer time:" + String(displayAutoDimmerTime) + "s");
  
  //u8g2.setCursor(0,25); u8g2.print("Display auto dimmer:" + String(ESP.getEfuseMac()) + ",  FW Ver.:" + String(softwareRev));
    
  u8g2.setCursor(0,24); u8g2.print("Display auto dimmer on/off"); drawSwitch(220,24,displayAutoDimmerOn); 
  u8g2.setCursor(0,36); u8g2.print("Auto dimmer time:"); u8g2.setCursor(225,36); u8g2.print(String(displayAutoDimmerTime) + "s");
  u8g2.setCursor(0,47); u8g2.print("Auto dimmer value 0-14:              14");
  u8g2.setCursor(0,58); u8g2.print("Night dimmer value 0-14:              0"); 
  u8g2.sendBuffer();
}

void saveConfig() 
{
  /*
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(1, 33, "Saving configuration"); // 8 znakow  x 11 szer
  u8g2.sendBuffer();
  */
  
  // Sprawdź, czy plik istnieje
  if (SPIFFS.exists("/config.txt")) 
  {
    Serial.println("Plik config.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość konfiguracji
    myFile = SPIFFS.open("/config.txt", FILE_WRITE);
    if (myFile) 
	  {
      myFile.println("#### ESP32 Radio Config File ####");
      myFile.print("Display Brightness =");    myFile.print(displayBrightness); myFile.println(";");
      myFile.print("Dimmer Display Brightness =");    myFile.print(dimmerDisplayBrightness); myFile.println(";");
      myFile.print("Auto Dimmer Time ="); myFile.print(displayAutoDimmerTime); myFile.println(";");
      myFile.print("Auto Dimmer ="); myFile.print(displayAutoDimmerOn); myFile.println(";");
      myFile.println("Voice Info Every Hour =" + String(timeVoiceInfoEveryHour) + ";");
      myFile.println("VU Meter Mode =" + String(vuMeterMode) + ";");
      myFile.println("Encoder Function Order  =" + String(encoderFunctionOrder) + ";");
      myFile.println("Startup Display Mode  =" + String(displayMode) + ";");
      myFile.println("VU Meter On  =" + String(vuMeterOn) + ";");
	    myFile.println("VU Meter Refresh Time  =" + String(vuMeterRefreshTime) + ";");
      myFile.println("Scroller Refresh Time =" + String(scrollingRefresh) + ";");
      myFile.println("ADC Keyboard Enabled =" + String(adcKeyboardEnabled) + ";");
      myFile.println("Display Power Save Enabled =" + String(displayPowerSaveEnabled) + ";");  
      myFile.println("Display Power Save Time =" + String(displayPowerSaveTime) + ";");
      myFile.println("Max Volume Extended =" + String(maxVolumeExt) + ";");
      myFile.close();
      Serial.println("Aktualizacja config.txt na karcie SPIFFS");
    } 
	  else 
	  {
      Serial.println("Błąd podczas otwierania pliku config.txt.");
    }
  } 
  else 
  {
    Serial.println("Plik config.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualne wartości konfiguracji
    myFile = SPIFFS.open("/config.txt", FILE_WRITE);
    if (myFile) 
	  {
      myFile.println("#### ESP32 Radio Config File ####");
      myFile.print("Display Brightness =");    myFile.print(displayBrightness); myFile.println(";");
      myFile.print("Dimmer Display Brightness =");    myFile.print(dimmerDisplayBrightness); myFile.println(";");
      myFile.print("Auto Dimmer Time ="); myFile.print(displayAutoDimmerTime); myFile.println(";");
      myFile.print("Auto Dimmer On ="); myFile.print(displayAutoDimmerOn); myFile.println(";");
		  myFile.println("Voice Info Every Hour =" + String(timeVoiceInfoEveryHour) + ";");
      myFile.println("VU Meter Mode =" + String(vuMeterMode) + ";");
      myFile.println("Encoder Function Order  =" + String(encoderFunctionOrder) + ";");
      myFile.println("Startup Display Mode  =" + String(displayMode) + ";");
      myFile.println("VU Meter On  =" + String(vuMeterOn) + ";");
      myFile.println("VU Meter Refresh Time  =" + String(vuMeterRefreshTime) + ";");
      myFile.println("Scroller Refresh Time =" + String(scrollingRefresh) + ";");
      myFile.println("ADC Keyboard Enabled =" + String(adcKeyboardEnabled) + ";");
      myFile.println("Display Power Save Enabled =" + String(displayPowerSaveEnabled) + ";");  
      myFile.println("Display Power Save Time =" + String(displayPowerSaveTime) + ";");
      myFile.println("Max Volume settings Extended =" + String(maxVolumeExt) + ";");
      myFile.close();
      Serial.println("Utworzono i zapisano config.txt na karcie SPIFFS");
    } 
	  else 
	  {
      Serial.println("Błąd podczas tworzenia pliku config.txt.");
    }
  }


}

void saveAdcConfig() 
{

  // Sprawdź, czy plik istnieje
  if (SPIFFS.exists("/adckbd.txt")) 
  {
    Serial.println("Plik adckbd.txt już istnieje.");

    // Otwórz plik do zapisu i nadpisz aktualną wartość konfiguracji klawiatury ADC
    myFile = SPIFFS.open("/adckbd.txt", FILE_WRITE);
    if (myFile) 
	  {
      myFile.println("#### ESP32 Radio Config File - ADC Keyboard ####");
      myFile.println("keyboardButtonThreshold_0 =" + String(keyboardButtonThreshold_0) + ";");
      myFile.println("keyboardButtonThreshold_1 =" + String(keyboardButtonThreshold_1) + ";");
      myFile.println("keyboardButtonThreshold_2 =" + String(keyboardButtonThreshold_2) + ";");
      myFile.println("keyboardButtonThreshold_3 =" + String(keyboardButtonThreshold_3) + ";");
      myFile.println("keyboardButtonThreshold_4 =" + String(keyboardButtonThreshold_4) + ";");
      myFile.println("keyboardButtonThreshold_5 =" + String(keyboardButtonThreshold_5) + ";");
      myFile.println("keyboardButtonThreshold_6 =" + String(keyboardButtonThreshold_6) + ";");
      myFile.println("keyboardButtonThreshold_7 =" + String(keyboardButtonThreshold_7) + ";");
      myFile.println("keyboardButtonThreshold_8 =" + String(keyboardButtonThreshold_8) + ";");
      myFile.println("keyboardButtonThreshold_9 =" + String(keyboardButtonThreshold_9) + ";");
      myFile.println("keyboardButtonThreshold_Shift =" + String(keyboardButtonThreshold_Shift) + ";");
      myFile.println("keyboardButtonThreshold_Memory =" + String(keyboardButtonThreshold_Memory) + ";");
      myFile.println("keyboardButtonThreshold_Band =" + String(keyboardButtonThreshold_Band) + ";");
      myFile.println("keyboardButtonThreshold_Auto =" + String(keyboardButtonThreshold_Auto) + ";");
      myFile.println("keyboardButtonThreshold_Scan =" + String(keyboardButtonThreshold_Scan) + ";");
      myFile.println("keyboardButtonThreshold_Mute =" + String(keyboardButtonThreshold_Mute) + ";");
      myFile.println("keyboardButtonThresholdTolerance =" + String(keyboardButtonThresholdTolerance) + ";");
      myFile.println("keyboardButtonNeutral =" + String(keyboardButtonNeutral) + ";");
      myFile.println("keyboardSampleDelay =" + String(keyboardSampleDelay) + ";");
      myFile.close();
      Serial.println("Aktualizacja adckbd.txt na karcie SPIFFS");
    } 
	  else 
	  {
      Serial.println("Błąd podczas otwierania pliku adckbd.txt.");
    }
  } 
  else 
  {
    Serial.println("Plik adckbd.txt nie istnieje. Tworzenie...");

    // Utwórz plik i zapisz w nim aktualne wartości konfiguracji
    myFile = SPIFFS.open("/adckbd.txt", FILE_WRITE);
    if (myFile) 
	  {
      myFile.println("#### ESP32 Radio Config File - ADC Keyboard ####");
      myFile.println("keyboardButtonThreshold_0 =" + String(keyboardButtonThreshold_0) + ";");
      myFile.println("keyboardButtonThreshold_1 =" + String(keyboardButtonThreshold_1) + ";");
      myFile.println("keyboardButtonThreshold_2 =" + String(keyboardButtonThreshold_2) + ";");
      myFile.println("keyboardButtonThreshold_3 =" + String(keyboardButtonThreshold_3) + ";");
      myFile.println("keyboardButtonThreshold_4 =" + String(keyboardButtonThreshold_4) + ";");
      myFile.println("keyboardButtonThreshold_5 =" + String(keyboardButtonThreshold_5) + ";");
      myFile.println("keyboardButtonThreshold_6 =" + String(keyboardButtonThreshold_6) + ";");
      myFile.println("keyboardButtonThreshold_7 =" + String(keyboardButtonThreshold_7) + ";");
      myFile.println("keyboardButtonThreshold_8 =" + String(keyboardButtonThreshold_8) + ";");
      myFile.println("keyboardButtonThreshold_9 =" + String(keyboardButtonThreshold_9) + ";");
      myFile.println("keyboardButtonThreshold_Shift =" + String(keyboardButtonThreshold_Shift) + ";");
      myFile.println("keyboardButtonThreshold_Memory =" + String(keyboardButtonThreshold_Memory) + ";");
      myFile.println("keyboardButtonThreshold_Band =" + String(keyboardButtonThreshold_Band) + ";");
      myFile.println("keyboardButtonThreshold_Auto =" + String(keyboardButtonThreshold_Auto) + ";");
      myFile.println("keyboardButtonThreshold_Scan =" + String(keyboardButtonThreshold_Scan) + ";");
      myFile.println("keyboardButtonThreshold_Mute =" + String(keyboardButtonThreshold_Mute) + ";");
      myFile.println("keyboardButtonThresholdTolerance =" + String(keyboardButtonThresholdTolerance) + ";");
      myFile.println("keyboardButtonNeutral =" + String(keyboardButtonNeutral) + ";");
      myFile.println("keyboardSampleDelay =" + String(keyboardSampleDelay) + ";");
      myFile.close();
      Serial.println("Utworzono i zapisano adckbd.txt na karcie SPIFFS");
    } 
	  else 
	  {
      Serial.println("Błąd podczas tworzenia pliku adckbd.txt.");
    }
  }


}


void readConfig() 
{

  Serial.println("Odczyt pliku config.txt z karty");
  String fileName = String("/config.txt"); // Tworzymy nazwę pliku

  if (!SPIFFS.exists(fileName))               // Sprawdzamy, czy plik istnieje
  {
    Serial.println("Błąd: Plik nie istnieje.");
    configExist = false;
    return;
  }
 
  File configFile = SPIFFS.open(fileName, FILE_READ);// Otwieramy plik w trybie do odczytu
  if (!configFile)  // jesli brak pliku to...
  {
    Serial.println("Błąd: Nie można otworzyć pliku konfiguracji");
    return;
  }
  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String configValue = "";
  while (configFile.available()) 
  {
    String line = configFile.readStringUntil(';'); //('\n');
    
    int lineStart = line.indexOf("=") + 1;  // Szukamy miejsca, gdzie zaczyna wartość zmiennej
    if ((lineStart != -1)) //&& (currentLine != 0)) // Pomijamy pierwszą linijkę gdzie jest opis pliku
	  {
      configValue = line.substring(lineStart);  // Wyciągamy URL od "http"
      configValue.trim();                      // Usuwamy białe znaki na początku i końcu
      Serial.print(" Odczytano zmienna konfiguracji numer:" + String(currentLine) + " wartosc:");
      Serial.println(configValue);
      configArray[currentLine] = configValue.toInt();
    }
    currentLine++;
  }
  Serial.print("Zamykamy plik config na wartosci currentLine:");
  Serial.println(currentLine);
  
  //Odczyt kontrolny
  //for (int i = 0; i < 16; i++) 
  //{
  //  Serial.print("wartosc: " + String(i) + " z tablicy konfiguracji:");
  //  Serial.println(configArray[i]);
  //}

  configFile.close();  // Zamykamy plik po odczycie kodow pilota

  displayBrightness = configArray[0];
  dimmerDisplayBrightness = configArray[1];
  displayAutoDimmerTime = configArray[2];
  displayAutoDimmerOn = configArray[3];
  timeVoiceInfoEveryHour = configArray[4];
  vuMeterMode = configArray[5];
  encoderFunctionOrder = configArray[6];
  displayMode = configArray[7];
  vuMeterOn = configArray[8];
  vuMeterRefreshTime = configArray[9];
  scrollingRefresh = configArray[10];
  adcKeyboardEnabled = configArray[11];
  displayPowerSaveEnabled = configArray[12];
  displayPowerSaveTime = configArray[13];
  maxVolumeExt = configArray[14];
  
  if (maxVolumeExt == 1)
  { 
    maxVolume = 42;
  }
  else
  {
    maxVolume = 21;
  }
  audio.setVolumeSteps(maxVolume);
}


void readAdcConfig() 
{

  Serial.println("Odczyt pliku konfiguracji klawiatury ADC adckbd.txt z karty");
  String fileName = String("/adckbd.txt"); // Tworzymy nazwę pliku

  if (!SPIFFS.exists(fileName))               // Sprawdzamy, czy plik istnieje
  {
    Serial.println("Błąd: Plik nie istnieje.");
    return;
  }
 
  File configFile = SPIFFS.open(fileName, FILE_READ);// Otwieramy plik w trybie do odczytu
  if (!configFile)  // jesli brak pliku to...
  {
    Serial.println("Błąd: Nie można otworzyć pliku konfiguracji klawiatury ADC");
    return;
  }
  // Przechodzimy do odpowiedniego wiersza pliku
  int currentLine = 0;
  String configValue = "";
  while (configFile.available()) 
  {
    String line = configFile.readStringUntil(';'); //('\n');
    
    int lineStart = line.indexOf("=") + 1;  // Szukamy miejsca, gdzie zaczyna wartość zmiennej
    if ((lineStart != -1)) //&& (currentLine != 0)) // Pomijamy pierwszą linijkę gdzie jest opis pliku
	  {
      configValue = line.substring(lineStart);  // Wyciągamy URL od "http"
      configValue.trim();                      // Usuwamy białe znaki na początku i końcu
      Serial.print(" Odczytano ustawienie ADC nr.:" + String(currentLine) + " wartosc:");
      Serial.println(configValue);
      configAdcArray[currentLine] = configValue.toInt();
    }
    currentLine++;
  }
  Serial.print("Zamykamy plik config na wartosci currentLine:");
  Serial.println(currentLine);
  
  //Odczyt kontrolny
  //for (int i = 0; i < 16; i++) 
  //{
  //  Serial.print("wartosc poziomu ADC: " + String(i) + " z tablicy:");
  //  Serial.println(configAdcArray[i]);
 // }

  configFile.close();  // Zamykamy plik po odczycie kodow pilota

 // ---- Progi przełaczania ADC dla klawiatury matrycowej 5x3 w tunrze Sony ST-120 ---- //
        // Pozycja neutralna
  keyboardButtonThreshold_0 = configAdcArray[0];          // Przycisk 0
  keyboardButtonThreshold_1 = configAdcArray[1];          // Przycisk 1
  keyboardButtonThreshold_2 = configAdcArray[2];          // Przycisk 2
  keyboardButtonThreshold_3 = configAdcArray[3];          // Przycisk 3
  keyboardButtonThreshold_4 = configAdcArray[4];          // Przycisk 4
  keyboardButtonThreshold_5 = configAdcArray[5];          // Przycisk 5
  keyboardButtonThreshold_6 = configAdcArray[6];        // Przycisk 6
  keyboardButtonThreshold_7 = configAdcArray[7];        // Przycisk 7
  keyboardButtonThreshold_8 = configAdcArray[8];        // Przycisk 8
  keyboardButtonThreshold_9 = configAdcArray[9];        // Przycisk 9
  keyboardButtonThreshold_Shift = configAdcArray[10];   // Shift - funkcja Enter/OK
  keyboardButtonThreshold_Memory = configAdcArray[11];  // Memory - funkcja Bank menu
  keyboardButtonThreshold_Band = configAdcArray[12];    // Przycisk Band - funkcja Back
  keyboardButtonThreshold_Auto = configAdcArray[13];    // Przycisk Auto - przelacza Radio/Zegar
  keyboardButtonThreshold_Scan = configAdcArray[14];    // Przycisk Scan - funkcja Dimmer ekranu OLED
  keyboardButtonThreshold_Mute = configAdcArray[15];      // Przycisk Mute - funkcja MUTE
  keyboardButtonThresholdTolerance = configAdcArray[16];  // Tolerancja dla pomiaru ADC
  keyboardButtonNeutral = configAdcArray[17];             // Pozycja neutralna 
  keyboardSampleDelay = configAdcArray[18];               // Czas co ile ms odczytywac klawiature

}



void readPSRAMstations()  // Funkcja testowa-debug, do odczytu PSRAMu, nie uzywana przez inne funkcje
{
  Serial.println("-------- POCZATEK LISTY STACJI ---------- ");
  for (int i = 0; i < stationsCount; i++) 
  {
    // Odczyt stacji pod daną komórka pamieci PSRAM:
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych
    int length = psramData[(i) * (STATION_NAME_LENGTH + 1)];   // Odczytaj długość nazwy stacji z PSRAM dla bieżącego indeksu stacji
      
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
    { // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
        station[j] = psramData[(i) * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }
    String stationNameText = String(station);
    
    Serial.print(i+1);
    Serial.print(" ");
    Serial.println(stationNameText);
  }	
  
  Serial.println("-------- KONIEC LISTY STACJI ---------- ");

  String stationUrl = "";

  // Odczyt stacji pod daną komórka pamieci PSRAM:
  char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
  memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych
  int length = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1)];   // Odczytaj długość nazwy stacji z PSRAM dla bieżącego indeksu stacji
      
  for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
  { // Odczytaj nazwę stacji z PSRAM jako ciąg bajtów, maksymalnie do STATION_NAME_LENGTH
    station[j] = psramData[(station_nr - 1) * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
  }
  
  //String stationNameText = String(station);
  
  Serial.println("-------- OBECNIE GRAMY  ---------- ");
  Serial.print(station_nr - 1);
  Serial.print(" ");
  Serial.println(String(station));

}

void webUrlStationPlay() 
{
  stationString.remove(0);  // Usunięcie wszystkich znaków z obiektu stationString
  stationNameStream.remove(0);
  
  audio.stopSong();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_fub14_tf); // cziocnka 14x11
  u8g2.drawStr(34, 33, "Loading stream..."); // 8 znakow  x 11 szer
  u8g2.sendBuffer();

  mp3 = flac = aac = vorbis = opus = false;
    
  Serial.println("debug-- Read station from WEB URL");
    
  if (url2play != "") 
  {
    url2play.trim(); // Usuwamy białe znaki na początku i końcu
  }
  else
  {
	  return;
  }
  
  if (url2play.isEmpty()) // jezeli link URL jest pusty
  {
    Serial.println("Błąd: Nie znaleziono stacji dla podanego numeru.");
    return;
  }
  
  
  // Weryfikacja, czy w linku znajduje się "http" lub "https"
  if (url2play.startsWith("http://") || url2play.startsWith("https://")) 
  {
    // Wydrukuj nazwę stacji i link na serialu
    Serial.print("Link do stacji: ");
    Serial.println(url2play);
    
    u8g2.setFont(spleen6x12PL);  // wypisujemy jaki stream jakie stacji jest ładowany
    u8g2.drawStr(34, 55, String(url2play).c_str());
    u8g2.sendBuffer();
    
    station_nr = 0;
    bank_nr = 0;
    //stationName = "Remote URL";
    
    // Połącz z daną stacją
    audio.connecttohost(url2play.c_str());
    

  } 
  else 
  {
    Serial.println("Błąd: link stacji nie zawiera 'http' lub 'https'");
    Serial.println("Odczytany URL: " + url2play);
  }
  station_nr = 0;
  bank_nr = 0;
  //stationName = stationNameStream;
  stationName = "WEB URL";
  url2play = "";
}

String stationBankListHtmlMobile()
{
  String html1;
  
  html1 += "<p>Volume: <span id='textSliderValue'>--</span></p>" + String("\n");
  html1 += "<p><input type='range' onchange='updateSliderVolume(this)' id='volumeSlider' min='1' max='" + String(maxVolume) + "' value='1' step='1' class='slider'></p>" + String("\n");
  html1 += "<p>Memory Bank Selection:</p>" + String("\n");
    
  html1 += "<p>";
  for (int i = 1; i < 17; i++) // Przyciski Banków
  {
    
    if (i == bank_nr)
    {
      html1 += "<button class='buttonBankSelected' onClick=\"changeBank('" + String(i) + "');\" id=\"Bank\">" + String(i) + "</button>" + String("\n");
    }
    else
    {
      html1 += "<button class=\"buttonBank\" onClick=\"changeBank('" + String(i) + "');\" id=\"Bank\">" + String(i) + "</button>" + String("\n");
    }
    if (i == 8) {html1 += "</p><p>";}
  }
  html1 += "</p>";
  html1 += "<center>"; 
 
  html1 += "<table>";


  for (int i = 0; i < stationsCount; i++) // lista stacji
  {
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych
    int length = psramData[i * (STATION_NAME_LENGTH + 1)];
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
    {
      station[j] = psramData[i * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }
    
    html1 += "<tr>";
    html1 += "<td><p class='stationNumberList'>" + String(i + 1) + "</p></td>";
    html1 += "<td><p class='stationList' onClick=\"changeStation('" + String(i + 1) +  "');\">" + String(station).substring(0, stationNameLenghtCut) + "</p></td>";
    html1 += "</tr>" + String("\n");
          
  }
 
  html1 += "</table>" + String("\n");
  html1 += "</div>" + String("\n");

  html1 += "<p style=\"font-size: 0.8rem;\">Web Radio, mobile, Evo: " + String(softwareRev) + "</p>" + String("\n");
  html1 += "<p style='font-size: 0.8rem;'>IP: "+ currentIP + "</p>" + String("\n");
  
  html1 += "<a href='/menu' class='button' style='padding: 0.2rem; font-size: 0.7rem; height: auto; line-height: 1;color: white; width: 65px; border: 1px solid black; display: inline-block; border-radius: 5px; text-decoration: none;'>Menu</a>";
  html1 += "</center></body></html>";
  
  return html1;
}

String stationBankListHtmlPC()
{
  String html2;
  

  html2 += "<p>Volume: <span id='textSliderValue'>--</span></p>" + String("\n");
  html2 += "<p><input type='range' onchange='updateSliderVolume(this)' id='volumeSlider' min='1' max='" + String(maxVolume) + "' value='1' step='1' class='slider'></p>" + String("\n");
  html2 += "<p>Memory Bank Selection:</p>" + String("\n");
  
  
  html2 += "<p>";
  for (int i = 1; i < 17; i++)
  {
    
    if (i == bank_nr)
    {
      html2 += "<button class=\"buttonBankSelected\" onClick=\"changeBank('" + String(i) + "');\" id=\"Bank\">" + String(i) + "</button>" + String("\n");
    }
    else
    {
      html2 += "<button class=\"buttonBank\" onClick=\"changeBank('" + String(i) + "');\" id=\"Bank\">" + String(i) + "</button>" + String("\n");
    }
  }
  
  html2 += "</p>";
  html2 += "<center>" + String("\n");
  html2 += "<div class=\"column\">" + String("\n");
  for (int i = 0; i < MAX_STATIONS; i++) 
  //for (int i = 0; i < stationsCount; i++) 
  {
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych

    int length = psramData[i * (STATION_NAME_LENGTH + 1)];
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
    {
      station[j] = psramData[i * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }     

    
    if ((i == 0) || (i == 25) || (i == 50) || (i == 75))
    { 
      html2 += "<table>" + String("\n");
    } 
    
    // 0-98   >98
    if (i >= stationsCount) { station[0] ='\0'; } // Jesli mamy mniej niz 99 stacji to wypełniamy pozostałe komórki pustymi wartościami
                 
    html2 += "<tr>";
    html2 += "<td><p class='stationNumberList'>" + String(i + 1) + "</p></td>";
    html2 += "<td><p class='stationList' onClick=\"changeStation('" + String(i + 1) +  "');\">" + String(station).substring(0, stationNameLenghtCut) + "</p></td>";
    html2 += "</tr>" + String("\n");

    if ((i == 24) || (i == 49) || (i == 74)) //||(i == 98))
    { 
      html2 += "</table>" + String("\n");
    }
           
  }

  html2 += "</table>" + String("\n");
  html2 += "</div>" + String("\n");
  html2 += "<p style=\"font-size: 0.8rem;\">Web Radio, desktop, Evo: " + String(softwareRev) + "</p>" + String("\n");
  html2 += "<p style='font-size: 0.8rem;'>IP: " + currentIP + "</p>" + String("\n");
  html2 += "<a href='/menu' class='button' style='padding: 0.2rem; font-size: 0.7rem; height: auto; line-height: 1;color: white; width: 65px; border: 1px solid black; display: inline-block; border-radius: 5px; text-decoration: none;'>Menu</a>";
  html2 += "</center></body></html>"; 

  return html2;
}

/*
String stationBankListHtml(bool mobilePage)
{
  String html3;

  html3 += "<p>Volume: <span id='textSliderValue'>%%SLIDERVALUE%%</span></p>";
  html3 += "<p><input type='range' onchange='updateSliderVolume(this)' id='volumeSlider' min='1' max='" + String(maxVolume) + "' value='%%SLIDERVALUE%%' step='1' class='slider'></p>";
  html3 += "<br>";
  html3 += "<p>Bank selection:</p>";


  html3 += "<p>";
  for (int i = 1; i < 17; i++)
  {
    
    if (i == bank_nr)
    {
      html3 += "<button class=\"buttonBankSelected\" onClick=\"bankLoad('" + String(i) + "');\" id=\"Bank\">" + String(i) + "</button>" + String("\n");
    }
    else
    {
      html3 += "<button class=\"buttonBank\" onClick=\"bankLoad('" + String(i) + "');\" id=\"Bank\">" + String(i) + "</button>" + String("\n");
    }

    if ((mobilePage == 1) && (i == 8)) {html3 += "</p><p>";}
  }
  
  html3 += "</p>";
  html3 += "<center>" + String("\n");


  if (mobilePage == 1)
  {
   html3 += "<table>";
  }
  else if (mobilePage == 0)
  { 
    html3 += "<div class=\"column\">" + String("\n");
  }
  
  int limit = (mobilePage == 1) ? stationsCount : MAX_STATIONS;

  for (int i = 0; i < limit; i++)
  {
    char station[STATION_NAME_LENGTH + 1];  // Tablica na nazwę stacji o maksymalnej długości zdefiniowanej przez STATION_NAME_LENGTH
    memset(station, 0, sizeof(station));    // Wyczyszczenie tablicy zerami przed zapisaniem danych

    int length = psramData[i * (STATION_NAME_LENGTH + 1)];
    for (int j = 0; j < min(length, STATION_NAME_LENGTH); j++) 
    {
      station[j] = psramData[i * (STATION_NAME_LENGTH + 1) + 1 + j];  // Odczytaj znak po znaku nazwę stacji
    }     

    
    if ((mobilePage == 0) && ((i == 0) || (i == 25) || (i == 50) || (i == 75)))
    { 
      html3 += "<table>" + String("\n");
    } 
 
    // 0-98   >98
    if (mobilePage == 0) 
    {
      if (i >= stationsCount) { station[0] ='\0'; } // Jesli mamy mniej niz 99 stacji to wypełniamy pozostałe komórki pustymi wartościami
    }

    html3 += "<tr>";
    html3 += "<td><p class='stationNumberList'>" + String(i + 1) + "</p></td>";
    html3 += "<td><p class='stationList' onClick=\"changeStation('" + String(i + 1) +  "');\">" + String(station).substring(0, stationNameLenghtCut) + "</p></td>";
    html3 += "</tr>" + String("\n");
   
    if ((mobilePage == 0) && ((i == 24) || (i == 49) || (i == 74))) //||(i == 98))
    { 
      html3 += "</table>" + String("\n");
    }
           
  }

  html3 += "</table>" + String("\n");
  html3 += "</div>" + String("\n");
  if (mobilePage == 0) { html3 += "<p style=\"font-size: 0.8rem;\">Web Radio, desktop, Evo: " + String(softwareRev) + "</p>" + String("\n"); }
  if (mobilePage == 1) { html3 += "<p style=\"font-size: 0.8rem;\">Web Radio, mobile, Evo: " + String(softwareRev) + "</p>" + String("\n"); }
  html3 += "<p style='font-size: 0.8rem;'>IP: " + currentIP + "</p>" + String("\n");
  html3 += "<p style='font-size: 0.8rem;'><a href='/menu'>MENU</a></p>" + String("\n");

  html3 += "</center></body></html>"; 

  return html3;
}
*/

void voiceTime()
{
  resumePlay = true;
  char chbuf[30];      //Bufor komunikatu tekstowego zegara odczytywanego głosowo
  String time_s;
  struct tm timeinfo;
  char timeString[9];  // Bufor przechowujący czas w formie tekstowej
  if (!getLocalTime(&timeinfo, 5)) 
  {
    Serial.println("Nie udało się uzyskać czasu funkcji voice time");
    return;  // Zakończ funkcję, gdy nie udało się uzyskać czasu
  }

  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  time_s = String(timeString);
  int h = time_s.substring(0,2).toInt();
  Serial.print("wartosc h:");
  Serial.print(h);
  snprintf(chbuf, sizeof (chbuf), "Jest godzina %i:%02i:00", h, time_s.substring(3,5).toInt());
  Serial.println(chbuf);
  audio.connecttospeech(chbuf, "pl");
}

void voiceTimeEn()
{
  resumePlay = true;
  char chbuf[30];      //Bufor komunikatu tekstowego zegara odczytywanego głosowo
  String time_s;
  char am_pm[5] = "am.";
  struct tm timeinfo;
  char timeString[9];  // Bufor przechowujący czas w formie tekstowej

  if (!getLocalTime(&timeinfo, 5)) 
  {
    Serial.println("Nie udało się uzyskać czasu funkcji voice time");
    return;  // Zakończ funkcję, gdy nie udało się uzyskać czasu
  }

  snprintf(timeString, sizeof(timeString), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  time_s = String(timeString);
  int h = time_s.substring(0,2).toInt();
  if(h > 12){h -= 12; strcpy(am_pm,"pm.");}
  //snprintf(chbuf, sizeof (chbuf), "Jest godzina %i:%02i", h, time_s.substring(3,5).toInt());
  sprintf(chbuf, "It is now %i%s and %i minutes", h, am_pm, time_s.substring(3,5).toInt());
  Serial.println(chbuf);
  Serial.println(chbuf);
  audio.connecttospeech(chbuf, "en");
}


void readRemoteConfig() 
{

  Serial.println("IR - Config, odczyt pliku konfiguracji pilota IR remote.txt z karty");
  
    // Tworzymy nazwę pliku
  String fileName = String("/remote.txt");

  // Sprawdzamy, czy plik istnieje
  if (!SPIFFS.exists(fileName)) 
  {
    Serial.println("IR - Config, błąd, Plik konfiguracji pilota IR remote.txt nie istnieje.");
    configIrExist = false;
    return;
  }

  // Otwieramy plik w trybie do odczytu
  File configRemoteFile = SPIFFS.open(fileName, FILE_READ);
  if (!configRemoteFile)  // jesli brak pliku to...
  {
    Serial.println("IR - Config, błąd, nie można otworzyć pliku konfiguracji pilota IR");
    configIrExist = false;
    return;
  }
  // Przechodzimy do odpowiedniego wiersza pliku
  configIrExist = true;
  int currentLine = 0;
  String configValue = "";
  while (configRemoteFile.available())
  {
    String line = configRemoteFile.readStringUntil(';'); //('\n');
    int lineStart = line.indexOf("0x") + 2;  // Szukamy miejsca, gdzie zaczyna wartość zmiennej
    if ((lineStart != -1)) //&& (currentLine != 0)) // Pomijamy pierwszą linijkę gdzie jest opis pliku
	  {
      configValue = line.substring(lineStart, lineStart + 5);  // Wyciągamy adres i komende
      configValue.trim();                      // Usuwamy białe znaki na początku i końcu
      Serial.print(" Odczytano kod: " + String(currentLine) + " wartosc:");
      Serial.print(configValue + " wartosc ConfigArray: ");
      configRemoteArray[currentLine] = strtol(configValue.c_str(), NULL, 16);
      Serial.println(configRemoteArray[currentLine], HEX);
    }
    currentLine++;
  }
  //Serial.print("Zamykamy plik config na wartosci currentLine:");
  //Serial.println(currentLine);
  configRemoteFile.close();  // Zamykamy plik po odczycie kodow pilota
}

void assignRemoteCodes()
{
  Serial.print("IR Config - Plik konfiguracji pilota istnieje, configIrExist: ");
  Serial.println(configIrExist);

  if ((noSPIFFScard == false) && (configIrExist == true)) 
  {
  Serial.println("IR Config - Plik istnieje, przypisuje wartosci z pliku Remote.txt");
  rcCmdVolumeUp = configRemoteArray[0];    // Głosnosc +
  rcCmdVolumeDown = configRemoteArray[1];  // Głośnosc -
  rcCmdArrowRight = configRemoteArray[2];  // strzałka w prawo - nastepna stacja
  rcCmdArrowLeft = configRemoteArray[3];   // strzałka w lewo - poprzednia stacja  
  rcCmdArrowUp = configRemoteArray[4];     // strzałka w góre - lista stacji krok do gory
  rcCmdArrowDown = configRemoteArray[5];   // strzałka w dół - lista stacj krok na dół
  rcCmdBack = configRemoteArray[6]; 	     // Przycisk powrotu
  rcCmdOk = configRemoteArray[7];          // Przycisk Ent - zatwierdzenie stacji
  rcCmdSrc = configRemoteArray[8];         // Przełączanie źródła radio, odtwarzacz
  rcCmdMute = configRemoteArray[9];        // Wyciszenie dzwieku
  rcCmdAud = configRemoteArray[10];        // Equalizer dzwieku
  rcCmdDirect = configRemoteArray[11];     // Janość ekranu, dwa tryby 1/16 lub pełna janość     
  rcCmdBankMinus = configRemoteArray[12];  // Wysweitla wybór banku
  rcCmdBankPlus = configRemoteArray[13];   // Wysweitla wybór banku
  rcCmdRed = configRemoteArray[14];        // Przełacza ładowanie banku kartaSPIFFS - serwer GitHub w menu bank
  rcCmdGreen = configRemoteArray[15];      // VU wyłaczony, VU tryb 1, VU tryb 2, zegar
  rcCmdKey0 = configRemoteArray[16];       // Przycisk "0"
  rcCmdKey1 = configRemoteArray[17];       // Przycisk "1"
  rcCmdKey2 = configRemoteArray[18];       // Przycisk "2"
  rcCmdKey3 = configRemoteArray[19];       // Przycisk "3"
  rcCmdKey4 = configRemoteArray[20];       // Przycisk "4"
  rcCmdKey5 = configRemoteArray[21];       // Przycisk "5"
  rcCmdKey6 = configRemoteArray[22];       // Przycisk "6"
  rcCmdKey7 = configRemoteArray[23];       // Przycisk "7"
  rcCmdKey8 = configRemoteArray[24];       // Przycisk "8"
  rcCmdKey9 = configRemoteArray[25];       // Przycisk "9"
  }
  else if ((noSPIFFScard == true) || (configIrExist == false)) // Jesli nie ma karty SPIFFS przypisujemy standardowe wartosci dla pilota Kenwood RC-406
  {
    Serial.println("IR Config - BRAK konfiguracji pilota, przypisuje wartosci domyslne");
    rcCmdVolumeUp = 0xB914;   // Głosnosc +
    rcCmdVolumeDown = 0xB915; // Głośnosc -
    rcCmdArrowRight = 0xB90B; // strzałka w prawo - nastepna stacja
    rcCmdArrowLeft = 0xB90A;  // strzałka w lewo - poprzednia stacja  
    rcCmdArrowUp = 0xB987;    // strzałka w góre - lista stacji krok do gory
    rcCmdArrowDown = 0xB986;  // strzałka w dół - lista stacj krok na dół
    rcCmdBack = 0xB985;	   	  // Przycisk powrotu
    rcCmdOk = 0xB90E;         // Przycisk Ent - zatwierdzenie stacji
    rcCmdSrc = 0xB913;        // Przełączanie źródła radio, odtwarzacz
    rcCmdMute = 0xB916;       // Wyciszenie dzwieku
    rcCmdAud = 0xB917;        // Equalizer dzwieku
    rcCmdDirect = 0xB90F;     // Janość ekranu, dwa tryby 1/16 lub pełna janość     
    rcCmdBankMinus = 0xB90C;  // Wysweitla wybór banku
    rcCmdBankPlus = 0xB90D;   // Wysweitla wybór banku
    rcCmdRed = 0xB988;        // Przełacza ładowanie banku kartaSPIFFS - serwer GitHub w menu bank
    rcCmdGreen = 0xB992;      // VU wyłaczony, VU tryb 1, VU tryb 2, zegar
    rcCmdKey0 = 0xB900;       // Przycisk "0"
    rcCmdKey1 = 0xB901;       // Przycisk "1"
    rcCmdKey2 = 0xB902;       // Przycisk "2"
    rcCmdKey3 = 0xB903;       // Przycisk "3"
    rcCmdKey4 = 0xB904;       // Przycisk "4"
    rcCmdKey5 = 0xB905;       // Przycisk "5"
    rcCmdKey6 = 0xB906;       // Przycisk "6"
    rcCmdKey7 = 0xB907;       // Przycisk "7"
    rcCmdKey8 = 0xB908;       // Przycisk "8"
    rcCmdKey9 = 0xB909;       // Przycisk "9"
  }
}

void listFiles(String path, String &html) 
{
    File root = SPIFFS.open(path);
    if (!root) {
        Serial.println("Nie można otworzyć karty SPIFFS!");
        return;
    }

    // Przeglądanie plików w katalogu
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            html += "<tr>";

            html += "<td>" + String(file.name()) + "</td>";
            html += "<td>" + String(file.size()) + "</td>";
            html += "<td>" + String("\n");
            
            // Kasowanie pliku
            html += "<form action=\"/delete\" method=\"POST\" style=\"display:inline; margin-right: 35px;\">";
            html += "<input type=\"hidden\" name=\"filename\" value=\"" + String(file.name()) + "\">";
            html += "<input type=\"submit\" value=\"Delete\">";
            html += "</form>" + String("\n");

            // Podglad pliku
            html += "<form action=\"/view\" method=\"GET\" style=\"display:inline;\">";
            html += "<input type=\"hidden\" name=\"filename\" value=\"" + String(file.name()) + "\">";
            html += "<input type=\"submit\" value=\"View\">";
            html += "</form>" + String("\n");
            
            // Edycja pliku
            html += "<form action=\"/edit\" method=\"GET\" style=\"display:inline;\">";
            html += "<input type=\"hidden\" name=\"filename\" value=\"" + String(file.name()) + "\">";
            html += "<input type=\"submit\" value=\"Edit\">";
            html += "</form>" + String("\n");            
            
            // Pobieranie pliku
            html += "<form action=\"/download\" method=\"GET\" style=\"display:inline;\">";
            html += "<input type=\"hidden\" name=\"filename\" value=\"" + String(file.name()) + "\">";
            html += "<input type=\"submit\" value=\"Download\">";
            html += "</form>" + String("\n");

            html += "</td>";
            html += "</tr>" + String("\n");
        }
        file = root.openNextFile();
    }
    root.close();
}


//####################################################################################### SETUP ####################################################################################### //

void setup() 
{
  // Inicjalizuj komunikację szeregową (Serial)
  Serial.begin(115200);
  Serial.println("---------- START of ESP32 Network Radio -----------");
  
  psramData = (unsigned char *)ps_malloc(PSRAM_lenght * sizeof(unsigned char));

  if (psramInit()) {
    Serial.println("debug--pamiec PSRAM zainicjowana poprawnie");
    Serial.print("Dostepna pamiec PSRAM:");
    Serial.println(ESP.getPsramSize());
    Serial.print("Wolna pamiec PSRAM:");
    Serial.println(ESP.getFreePsram());


  } else {
    Serial.println("debug-- BLAD Pamieci PSRAM");
  }

  //AudioBuffer(16384);
  audioBuffer.changeMaxBlockSize(16384);
  wifiManager.setHostname(hostname);
  
  EEPROM.begin(128);

  // Konfiguruj piny enkodera jako wejścia
  pinMode(CLK_PIN2, INPUT_PULLUP);
  pinMode(DT_PIN2, INPUT_PULLUP);
  // Inicjalizacja przycisków enkoderów jako wejścia
  pinMode(SW_PIN2, INPUT_PULLUP);
  
  //pinMode(recv_pin, INPUT_PULLUP);
  pinMode(recv_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(recv_pin), pulseISR, CHANGE);

  analogReadResolution(12); // Set ADC resolution to 12 bits (0-4095 range)
  analogSetAttenuation(ADC_11db); // Set the ADC input attenuation (0dB for 0-3.3V range)
  
  // Odczytaj początkowy stan pinu CLK enkodera
  prev_CLK_state2 = digitalRead(CLK_PIN2);

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);  // Konfiguruj pinout dla interfejsu I2S audio
    

  // Inicjalizuj interfejs SPI wyświetlacza
  SPI.begin(SPI_SCK_OLED, SPI_MISO_OLED, SPI_MOSI_OLED);
  SPI.setFrequency(1000000);

  // Inicjalizuj wyświetlacz i odczekaj 250 milisekund na włączenie
  u8g2.begin();
  delay(250);

  // Powitanie na wyswietlaczu:
  //u8g2.sendF("ca", 0xC7, displayBrightness); // Ustawiamy jasność ekranu zgodnie ze zmienna displayBrightness
  
  u8g2.drawXBMP(0, 5, notes_width, notes_height, notes);  // obrazek - nutki
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(58, 17, "Internet Radio");
  u8g2.setFont(spleen6x12PL);
  //u8g2.drawStr(226, 62, softwareRev);
  u8g2.drawStr(208, 62, softwareRev);
  u8g2.sendBuffer();

  // Inicjalizacja karty SPIFFS
 
  if (!SPIFFS.begin(true))    // jesli chcemy używać pamięci SPIFFS zamieniamy wszystkie wpisy "SPIFFS." na "SPIFFS."
  {
    // Informacja na wyswietlaczu o problemach lub braku karty SPIFFS
    Serial.println("Błąd inicjalizacji karty SPIFFS!");
    //u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(5, 62, "Error - Please check SPIFFS card");
    u8g2.setDrawColor(0);
    u8g2.drawBox(212, 0, 44, 45);
    u8g2.setDrawColor(1);
    u8g2.drawXBMP(220, 3, 30, 40, sdcard);  // ikona SPIFFS karty
    u8g2.sendBuffer();
  //  while(true) {;;} // Zostajemy tutaj az do resetu i ponownego sprawdzenia karty
  //  return;
    noSPIFFScard = true; // Flaga braku karty SPIFFS, będziemy użwyać EEPROM 
    //delay(1000);
  }
  else
  {
  Serial.println("Karta SPIFFS zainicjalizowana pomyślnie.");
  }
  Serial.print("Numer seryjny ESP:");
  Serial.println(ESP.getEfuseMac());

  
 // audioBuffer.changeMaxBlockSize(16384);

  //u8g2.drawStr(5, 32, "Internet Radio");
  //u8g2.sendBuffer();
  u8g2.setFont(spleen6x12PL);
  u8g2.drawStr(5, 62, "Connecting to network...    ");
  
  
  u8g2.sendBuffer();
  //u8g2.sendF("ca", 0xC7, displayBrightness); // Ustawiamy jasność ekranu zgodnie ze zmienna displayBrightness
  button2.setDebounceTime(50);  // Ustawienie czasu debouncingu dla przycisku enkodera 2
 
  // Inicjalizacja WiFiManagera
  wifiManager.setConfigPortalBlocking(false);

  readConfig();          // Odczyt konfiguracji
  if (configExist == false) { saveConfig(); readConfig();} // Jesli nie ma pliku config.txt to go tworzymy
  readStationFromSPIFFS();   // Odczytujemy zapisaną ostanią stację i bank z karty SPIFFS /EEPROMu
  readEqualizerFromSPIFFS(); // Odczytujemy ustawienia filtrów equalizera z karty SPIFFS 
  readVolumeFromSPIFFS();    // Odczytujemy nastawę ostatniego poziomu głośnosci z karty SPIFFS /EEPROMu
  readAdcConfig();       // Odczyt konfiguracji klawitury ADC
  readRemoteConfig();    // Odczyt konfiguracji pilota IR
  assignRemoteCodes();   // Przypisanie kodów pilota IR
  
  audio.setVolumeSteps(maxVolume);
  audio.setVolume(volumeValue);                  // Ustaw głośność na podstawie wartości zmiennej volumeValue w zakresie 0...21

  u8g2.setContrast(displayBrightness);    // Ustawiamy jasność ekranu zgodnie ze zmienna displayBrightness

  /*-------------------- RECOVERY MODE --------------------*/
  recoveryModeCheck();
  
  previous_bank_nr = bank_nr;  // wyrównanie wartości przy stacie radia aby nie podmienic bank_nr na wartość 0 po pierwszym upływie czasu menu
  Serial.print("debug1...wartość bank_nr:");
  Serial.println(bank_nr);
  Serial.print("debug1...wartość previous_bank_nr:");
  Serial.println(previous_bank_nr);
  Serial.print("debug1...wartość station_nr:");
  Serial.println(station_nr);

  // Rozpoczęcie konfiguracji Wi-Fi i połączenie z siecią, jeśli konieczne
  if (wifiManager.autoConnect("ESP32-Radio")) 
  {
    Serial.println("Połączono z siecią WiFi");
    //u8g2.clearBuffer();
    //u8g2.setFont(DotMatrix13pl);
    //u8g2.setFont(u8g2_font_fub14_tf);
    //u8g2.drawStr(5, 32, "WiFi Connected");
    currentIP = WiFi.localIP().toString();  //konwersja IP na string
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(5, 62, "                               ");  // czyszczenie lini spacjami
    u8g2.sendBuffer();
    u8g2.drawStr(5, 62, "Connected, IP:");  //wyswietlenie IP
    u8g2.drawStr(90, 62, currentIP.c_str());   //wyswietlenie IP
    u8g2.sendBuffer();
    delay(1000);  // odczekaj 1 sek przed wymazaniem numeru IP
    
    if (MDNS.begin(hostname)) { Serial.println("mDNS wystartowal, adres: " + String(hostname) + ".local w przeglądarce"); }
    
    //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2 );
    configTzTime("CET-1CEST,M3.5.0/2,M10.5.0/3", ntpServer1, ntpServer2);
    
    //timer1.attach(0.5, updateTimerFlag);  // Ustaw timer, aby wywoływał funkcję updateTimer
    timer1.attach(1, updateTimerFlag);  // Ustaw timer, aby wywoływał funkcję updateTimer
    timer2.attach(1, displayDimmerTimer);
    //timer3.attach(10, switchWeatherData);   // Ustaw timer, aby wywoływał funkcję switchWeatherData co 10 sekund
  

    uint8_t temp_station_nr = station_nr; // Chowamy na chwile odczytaną stacje z karty SPIFFS aby podczas ładowania banku nie zmienic ostatniej odtwarzanej stacji
    fetchStationsFromServer();            // Ładujemy liste stacji z karty SPIFFS  lub serwera GitHub
    station_nr = temp_station_nr ;        // Przywracamy numer po odczycie stacji
    changeStation();                      // Ustawiamy stację
    
    // ########################################### WEB Server ######################################################
    
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      String userAgent = request->header("User-Agent");
      String html =""; 
      //html.reserve(48000);  // rezerwuje bufor (np. 24KB)

      if (userAgent.indexOf("Mobile") != -1 || userAgent.indexOf("Android") != -1 || userAgent.indexOf("iPhone") != -1) 
      {
        html = stationBankListHtmlMobile();
      } 
      else //Jestemy na komputerze 
      {
        html = stationBankListHtmlPC();
      }
      
      String finalhtml = String(index_html) + html;  // Składamy cześć stałą html z częscią generowaną dynamicznie
      request->send(200, "text/html", finalhtml.c_str());
    });

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/favicon.ico", "image/x-icon");       
    });
    
    server.on("/icon.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/icon.png", "image/x-icon");       
    });

    server.on("/menu", HTTP_GET, [](AsyncWebServerRequest *request){   
      String html = String(menu_html);
      request->send(200, "text/html", html);
    });
    
    server.on("/firmwareota", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Update done");
      },
     [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) 
    {

      static size_t total = 0;
      static size_t contentLength = 0;
      static unsigned long lastPrint = 0;

      if (index == 0) 
      {
        // Tylko przy pierwszym pakiecie
        Serial.printf("OTA Start: %s\n", filename.c_str());
        total = 0;
        contentLength = request->contentLength();  // pełna długość pliku
        lastPrint = millis();

        size_t sketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

        if (contentLength > sketchSpace) 
        {
          Serial.printf("Error: firmware too big (%u > %u bytes)\n", contentLength, sketchSpace);
          return;
        }

        if (!Update.begin(sketchSpace)) 
        {
          Update.printError(Serial);
          return;
        }
      }

      if (Update.write(data, len) != len) 
      {
        Update.printError(Serial);
      } 
      else 
      {
        total += len;
        
        // Wyświetlaj pasek postępu co 300 ms
        unsigned long now = millis();
        if (now - lastPrint >= 200 || final) 
        {     
          int percent = (total * 100) / contentLength;
          Serial.printf("Progress: %d%% (%u/%u bytes)\n", percent, total, contentLength);
          u8g2.setCursor(5, 24); u8g2.print("File: " + String(filename));
          u8g2.setCursor(5, 36); u8g2.print("Flashing... " + String(total / 1024) + " KB");
          u8g2.sendBuffer();
          lastPrint = now;
        }
      }

      if (final) 
      {
        if (Update.end(true)) 
        {
          request->send(200, "text/plain", "Update done - reset in 3sec");
          Serial.println("Update complete");
          u8g2.setCursor(5, 48); u8g2.print("Completed - reset in 3sec");
          u8g2.sendBuffer();
              
          AsyncWebServerRequest *reqCopy = request;
          reqCopy->onDisconnect([]() 
          {
            delay(3000);
            ESP.restart();
          });
        } 
        else 
        {
          Update.printError(Serial);
          request->send(500, "text/plain", "Update failed");
        }
      }
    });

    server.on("/editor", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      request->send(SPIFFS, "/editor.html", "text/html"); //"application/octet-stream");
    });

    server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *request)
    {
      ws.closeAll();
      audio.stopSong();
          
      unsigned long now = millis();
      timeDisplay = false;
      displayActive = true;
      //clearFlags();
      fwupd = true;
      u8g2.setDrawColor(1);
      u8g2.clearBuffer();
      u8g2.setFont(spleen6x12PL);     
      u8g2.setCursor(5, 12); u8g2.print("ESP-Radio, OTA Firwmare Update");
      u8g2.sendBuffer();

      String html = "";
      html += "<!DOCTYPE html>";
      html += "<html lang='en'>";
      html += "<head>";
      html += "<meta charset='UTF-8' />";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'/>";
      html += "<title>ESP OTA Update</title>";
      html += "<style>";
      html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f3f4f6; color: #333; padding: 40px; text-align: center; }";
      html += "h2 { margin-bottom: 30px; color: #111; }";
      html += "#uploadSection { background-color: white; padding: 30px; border-radius: 8px; display: inline-block; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }";
      html += "input[type='file'] { padding: 10px; }";
      html += "button { margin-top: 20px; padding: 10px 20px; background-color: #4CAF50; border: none; color: white; font-size: 16px; border-radius: 5px; cursor: pointer; }";
      html += "button:disabled { background-color: #ccc; cursor: not-allowed; }";
      html += "progress { width: 100%; height: 20px; margin-top: 20px; border-radius: 5px; appearance: none; -webkit-appearance: none; }";
      html += "progress::-webkit-progress-bar { background-color: #eee; border-radius: 5px; }";
      html += "progress::-webkit-progress-value { background-color: #4CAF50; border-radius: 5px; }";
      html += "progress::-moz-progress-bar { background-color: #4CAF50; border-radius: 5px; }";
      html += "#status { margin-top: 15px; font-weight: bold; }";
      html += "#fileInfo { color: #555; margin-top: 10px; }";
      html += "</style>";
      html += "</head>";
      html += "<body>";
      html += "<div id='uploadSection'>";
      html += "<h2>ESP32 Radio - OTA Firmware Update</h2>";
      html += "<input type='file' id='fileInput' name='update' /><br />";
      html += "<div id='fileInfo'>No file selected</div>";
      html += "<button id='uploadBtn'>Upload</button>";
      html += "<p id='status'></p>";
      html += "</div>";
      html += "<script>";
      html += "const fileInput = document.getElementById('fileInput');";
      html += "const uploadBtn = document.getElementById('uploadBtn');";
      html += "const status = document.getElementById('status');";
      html += "const fileInfo = document.getElementById('fileInfo');";
      html += "fileInput.addEventListener('change', function () {";
      html += "  const file = this.files[0];";
      html += "  if (file) {";
      html += "    fileInfo.textContent = `Selected: ${file.name} (${(file.size / 1024).toFixed(2)} KB)`;";
      html += "  } else {";
      html += "    fileInfo.textContent = 'No file selected';";
      html += "  }";
      html += "});";
      html += "uploadBtn.addEventListener('click', function () {";
      html += "  const file = fileInput.files[0];";
      html += "  if (!file) { alert('Please select a file first.'); return; }";
      html += "  uploadBtn.disabled = true;";
      html += "  status.textContent = 'Uploading...';";
      html += "  const xhr = new XMLHttpRequest();";
      html += "  const formData = new FormData();";
      html += "  formData.append('update', file);";
      html += "  xhr.open('POST', '/firmwareota', true);";
      html += "  xhr.onload = function () {";
      html += "    if (xhr.status === 200) {";
      html += "      status.textContent = '✅ Upload completed, reboot in 3 sec.';";
      html += "      setTimeout(() => { window.location.href = '/'; }, 10000);";
      html += "    } else {";
      html += "      status.textContent = '❌ Upload failed.';";
      html += "    }";
      html += "    uploadBtn.disabled = false;";
      html += "  };";
      html += "  xhr.onerror = function () {";
      html += "    status.textContent = '❌ Network error.';";
      html += "    uploadBtn.disabled = false;";
      html += "  };";
      html += "  xhr.send(formData);";
      html += "});";
      html += "</script>";
      html += "</body>";
      html += "</html>";

      request->send(200, "text/html", html);
      //request->send(SPIFFS, "/ota.html", "text/html"); //"application/octet-stream");
    });

    server.on("/page1", HTTP_GET, [](AsyncWebServerRequest *request)
    { // Strona do celow testowych ladowana z karty SPIFFS
      request->send(SPIFFS, "/page1.html", "text/html"); //"application/octet-stream");
    });

    server.on("/edit", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      if (!request->hasParam("filename")) 
      {
          request->send(400, "text/plain", "Brak parametru filename");
          return;
      }

      String filename = request->getParam("filename")->value();
      if (!filename.startsWith("/")) { filename = "/" + filename;}
      File file = SPIFFS.open(filename, FILE_READ);
      if (!file) {
          request->send(404, "text/plain", "Nie można otworzyć pliku");
          return;
      }

      String html = "<!DOCTYPE html><html><head>";
      html += "<meta charset=\"UTF-8\">";
      html = "<html><head><title>Edit File</title></head><body>";
      html += "<h2>Editing: " + filename + "</h2>";
      html += "<form method='POST' action='/save'>";
      html += "<input type='hidden' name='filename' value='" + filename + "'>";
      html += "<textarea name='content' rows='100' cols='130'>";

      while (file.available()) 
      {
          html += (char)file.read();
      }

      file.close();

      html += "</textarea><br>";
      html += "<input type='submit' value='Save'>";
      html += "</form>";
      html += "<p><a href='/list'>Back to list</a></p>";
      html += "</body></html>";

      request->send(200, "text/html; charset=utf-8", html); // <- ważne!
    });

    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request) 
    {
      if (!request->hasParam("filename", true) || !request->hasParam("content", true)) 
      {
        request->send(400, "text/plain", "Brakuje danych");
        return;
      }

      String filename = request->getParam("filename", true)->value();
      String content = request->getParam("content", true)->value();

      File file = SPIFFS.open(filename, FILE_WRITE);
      if (!file) 
      {
        request->send(500, "text/plain", "Nie można zapisać pliku");
        return;
      }

      file.print(content);
      file.close();

      request->send(200, "text/html", "<p>File Saved!</p><p><a href='/list'>Back to list</a></p>");
    });

    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      String html = String(list_html) + String("\n");
      
      html += "<body><h1>ESP32 Radio - SPIFFS card:</h1>" + String("\n");
        
      html += "<form action=\"/upload\" method=\"POST\" enctype=\"multipart/form-data\">";
      html += "<input type=\"file\" name=\"file\">";
      html += "<input type=\"submit\" value=\"Upload\">";
      html += "</form>";
      html += "<div class=\"columnlist\">" + String("\n");
      html += "<table><tr><th>File</th><th>Size (Bytes)</th><th>Action</th></tr>";

      listFiles("/", html);
      html += "</table></div>";
      html += "<p style='font-size: 0.8rem;'><a href='/menu'>Go Back</a></p>" + String("\n"); 
      html += "</body></html>";     
      request->send(200, "text/html", html);
      });

      server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request) {
      String filename = "/"; // Dodajemy sciezke do głownego folderu
        if (request->hasParam("filename", true)) {
          filename += request->getParam("filename", true)->value();
          if (SPIFFS.remove(filename.c_str())) {
              Serial.println("Plik usunięty: " + filename);
          } else {
              Serial.println("Nie można usunąć pliku: " + filename);
          }
        }
        request->redirect("/list"); // Przekierowujemy na stronę listy
      });

      server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
      String html = String(config_html);
      html.replace("%D1", String(displayBrightness).c_str());
      html.replace("%D2", String(dimmerDisplayBrightness).c_str());
      html.replace("%D3", String(displayAutoDimmerTime).c_str());
      html.replace("%D4", String(vuMeterMode).c_str());
      html.replace("%D5", String(encoderFunctionOrder).c_str());
      html.replace("%D6", String(displayMode).c_str());
      html.replace("%D7", String(vuMeterRefreshTime).c_str());
      html.replace("%D8", String(scrollingRefresh).c_str());
      html.replace("%D9", String(displayPowerSaveTime).c_str());
      
      html.replace("%S10", displayPowerSaveEnabled ? "" : " selected");
      html.replace("%S11", maxVolumeExt ? " selected" : "");
      html.replace("%S12", maxVolumeExt ? "" : " selected");
      html.replace("%S1", displayAutoDimmerOn ? " selected" : "");
      html.replace("%S2", displayAutoDimmerOn ? "" : " selected");
      html.replace("%S3", timeVoiceInfoEveryHour ? " selected" : "");
      html.replace("%S4", timeVoiceInfoEveryHour ? "" : " selected");
      html.replace("%S5", vuMeterOn ? " selected" : "");
      html.replace("%S6", vuMeterOn ? "" : " selected");
      html.replace("%S7", adcKeyboardEnabled ? " selected" : "");
      html.replace("%S8", adcKeyboardEnabled ? "" : " selected");
      html.replace("%S9", displayPowerSaveEnabled ? " selected" : "");
    

      request->send(200, "text/html", html);
    });

    server.on("/adc", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      String html = String(adc_html);
      html.replace("%D10", String(keyboardButtonThreshold_Shift).c_str());
      html.replace("%D11", String(keyboardButtonThreshold_Memory).c_str()); 
      html.replace("%D12", String(keyboardButtonThreshold_Band).c_str()); 
      html.replace("%D13", String(keyboardButtonThreshold_Auto).c_str()); 
      html.replace("%D14", String(keyboardButtonThreshold_Scan).c_str()); 
      html.replace("%D15", String(keyboardButtonThreshold_Mute).c_str()); 
      html.replace("%D16", String(keyboardButtonThresholdTolerance).c_str()); 
      html.replace("%D17", String(keyboardButtonNeutral).c_str());
      html.replace("%D18", String(keyboardSampleDelay).c_str()); 
      html.replace("%D0", String(keyboardButtonThreshold_0).c_str()); 
      html.replace("%D1", String(keyboardButtonThreshold_1).c_str()); 
      html.replace("%D2", String(keyboardButtonThreshold_2).c_str()); 
      html.replace("%D3", String(keyboardButtonThreshold_3).c_str()); 
      html.replace("%D4", String(keyboardButtonThreshold_4).c_str()); 
      html.replace("%D5", String(keyboardButtonThreshold_5).c_str()); 
      html.replace("%D6", String(keyboardButtonThreshold_6).c_str()); 
      html.replace("%D7", String(keyboardButtonThreshold_7).c_str()); 
      html.replace("%D8", String(keyboardButtonThreshold_8).c_str()); 
      html.replace("%D9", String(keyboardButtonThreshold_9).c_str()); 
      
      request->send(200, "text/html", html);
    });

    server.on("/configadc", HTTP_POST, [](AsyncWebServerRequest *request) 
    {
      if (request->hasParam("keyboardButtonThreshold_Shift", true)) {
        keyboardButtonThreshold_Shift = request->getParam("keyboardButtonThreshold_Shift", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonThreshold_Memory", true)) {
        keyboardButtonThreshold_Memory = request->getParam("keyboardButtonThreshold_Memory", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonThreshold_Band", true)) {
        keyboardButtonThreshold_Band = request->getParam("keyboardButtonThreshold_Band", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonThreshold_Auto", true)) {
        keyboardButtonThreshold_Auto = request->getParam("keyboardButtonThreshold_Auto", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonThreshold_Scan", true)) {
        keyboardButtonThreshold_Scan = request->getParam("keyboardButtonThreshold_Scan", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonThreshold_Mute", true)) {
        keyboardButtonThreshold_Mute = request->getParam("keyboardButtonThreshold_Mute", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonThresholdTolerance", true)) {
        keyboardButtonThresholdTolerance = request->getParam("keyboardButtonThresholdTolerance", true)->value().toInt();
      }
      if (request->hasParam("keyboardButtonNeutral", true)) {
        keyboardButtonNeutral = request->getParam("keyboardButtonNeutral", true)->value().toInt();
      }
      if (request->hasParam("keyboardSampleDelay", true)) {
        keyboardSampleDelay = request->getParam("keyboardSampleDelay", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_0", true)) {
        keyboardButtonThreshold_0 = request->getParam("keyboardButtonThreshold_0", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_1", true)) {
        keyboardButtonThreshold_1 = request->getParam("keyboardButtonThreshold_1", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_2", true)) {
        keyboardButtonThreshold_2 = request->getParam("keyboardButtonThreshold_2", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_3", true)) {
        keyboardButtonThreshold_3 = request->getParam("keyboardButtonThreshold_3", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_4", true)) {
        keyboardButtonThreshold_4 = request->getParam("keyboardButtonThreshold_4", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_5", true)) {
        keyboardButtonThreshold_5 = request->getParam("keyboardButtonThreshold_5", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_6", true)) {
        keyboardButtonThreshold_6 = request->getParam("keyboardButtonThreshold_6", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_7", true)) {
        keyboardButtonThreshold_7 = request->getParam("keyboardButtonThreshold_7", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_8", true)) {
        keyboardButtonThreshold_8 = request->getParam("keyboardButtonThreshold_8", true)->value().toInt();
      } 
      if (request->hasParam("keyboardButtonThreshold_9", true)) {
        keyboardButtonThreshold_9 = request->getParam("keyboardButtonThreshold_9", true)->value().toInt();
      }

      request->send(200, "text/html", "<h1>ADC Keyboard Thresholds Updated!</h1><a href='/menu'>Go Back</a>");
      
      saveAdcConfig(); 
      
      //ODswiezenie ekranu OLED po zmianach konfiguracji
      ir_code = rcCmdBack; // Udajemy kod pilota Back
      bit_count = 32;
      calcNec();          // Przeliczamy kod pilota na pełny oryginalny kod NEC
    });

    server.on("/configupdate", HTTP_POST, [](AsyncWebServerRequest *request) 
    {
      if (request->hasParam("displayBrightness", true)) {
        displayBrightness = request->getParam("displayBrightness", true)->value().toInt();
      }
      if (request->hasParam("dimmerDisplayBrightness", true)) {
        dimmerDisplayBrightness = request->getParam("dimmerDisplayBrightness", true)->value().toInt();
      }
      if (request->hasParam("displayAutoDimmerTime", true)) {
        displayAutoDimmerTime = request->getParam("displayAutoDimmerTime", true)->value().toInt();
      }
      if (request->hasParam("displayAutoDimmerOn", true)) {
        displayAutoDimmerOn = request->getParam("displayAutoDimmerOn", true)->value() == "1";
      }
      if (request->hasParam("timeVoiceInfoEveryHour", true)) {
        timeVoiceInfoEveryHour = request->getParam("timeVoiceInfoEveryHour", true)->value() == "1";
      }
      if (request->hasParam("vuMeterMode", true)) {
        vuMeterMode = request->getParam("vuMeterMode", true)->value().toInt();
      }
      if (request->hasParam("encoderFunctionOrder", true)) {
        encoderFunctionOrder = request->getParam("encoderFunctionOrder", true)->value().toInt();
      }
      if (request->hasParam("displayMode", true)) {
        displayMode = request->getParam("displayMode", true)->value().toInt();
      }
      if (request->hasParam("vuMeterOn", true)) {
        vuMeterOn = request->getParam("vuMeterOn", true)->value() == "1";
      }
      if (request->hasParam("vuMeterRefreshTime", true)) {
        vuMeterRefreshTime = request->getParam("vuMeterRefreshTime", true)->value().toInt();
      }
      if (request->hasParam("scrollingRefresh", true)) {
        scrollingRefresh = request->getParam("scrollingRefresh", true)->value().toInt();
      }
      if (request->hasParam("adcKeyboardEnabled", true)) {
        adcKeyboardEnabled = request->getParam("adcKeyboardEnabled", true)->value() == "1";
      }
      if (request->hasParam("displayPowerSaveEnabled", true)) {
        displayPowerSaveEnabled = request->getParam("displayPowerSaveEnabled", true)->value() == "1";
      }
      if (request->hasParam("displayPowerSaveTime", true)) {
        displayPowerSaveTime = request->getParam("displayPowerSaveTime", true)->value().toInt();
      }
      if (request->hasParam("maxVolumeExt", true)) {
        maxVolumeExt = request->getParam("maxVolumeExt", true)->value() == "1";
      }  

      request->send(200, "text/html", "<h1>Configuration Updated!</h1><a href='/menu'>Go Back</a>");
      saveConfig(); 
      readConfig();
      //ODswiezenie ekranu OLED po zmianach konfiguracji
      ir_code = rcCmdBack; // Udajemy komendy pilota
      bit_count = 32;
      calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC
    });

    server.on("/toggleAdcDebug", HTTP_POST, [](AsyncWebServerRequest *request) 
    {
      // Przełączanie wartości
      u8g2.clearBuffer();
      displayActive = true;
      displayStartTime = millis();
      debugKeyboard = !debugKeyboard;
      //adcKeyboardEnabled = !adcKeyboardEnabled;
      Serial.print("Pomiar wartości ADC ON/OFF:");
      Serial.println(debugKeyboard);
      request->send(200, "text/plain", debugKeyboard ? "1" : "0"); // Wysyłanie aktualnej wartości
      //debugKeyboard = !debugKeyboard;
      
      if (debugKeyboard == 0)
      {
        ir_code = rcCmdBack; // Przypisujemy kod polecenia z pilota
        bit_count = 32; // ustawiamy informacje, ze mamy pelen kod NEC do analizy 
        calcNec();  // przeliczamy kod pilota na kod oryginalny pełen kod NEC  
      }
        
    });

    server.on("/view", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      String filename = "/";
      if (request->hasParam("filename")) 
      {
        filename += request->getParam("filename")->value();
        //String fullPath = "/" + filename;

        File file = SPIFFS.open(filename);
        if (file) 
        {
            String content = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"></head><body>";
            content += "<h1>File content: " + filename + "</h1><pre>";
            while (file.available()) {
                content += (char)file.read();
            }
            content += "</pre><a href=\"/list\">Back to list</a></body></html>";
            request->send(200, "text/html; charset=utf-8", content); // <- ważne!
            file.close();
          } 
          else 
          {
            request->send(404, "text/plain", "File not found");
          }
        } 
        else 
        {
        request->send(400, "text/plain", "No file name");
      }
    });
    // Format viewweb ver2 na potrzeby zewnetrznego Edytora Bankow HTML

    server.on("/viewweb2", HTTP_GET, [](AsyncWebServerRequest *request)  
    {
      String filename = "/";
      if (request->hasParam("filename")) 
      {
        filename += request->getParam("filename")->value();
        //String fullPath = "/" + filename;

        File file = SPIFFS.open(filename);
        if (file) 
        {
            String content; //= "<html><body>";
            while (file.available()) {
                //line = file.readStringUntil('\n');
                content += file.readStringUntil('\n') + String("\n");
            }
            //content +="</body></html>";
            request->send(200, "text/html", content);
            file.close();
          } 
          else 
          {
            request->send(404, "text/plain", "File not found");
          }
        } 
        else 
        {
        request->send(400, "text/plain", "No file name");
      }
    });
 
    server.on("/download", HTTP_ANY, [](AsyncWebServerRequest *request) {
      if (request->hasParam("filename")) {
        String filename = request->getParam("filename")->value();
        String fullPath = "/" + filename;
        //String fullPath = filename;

        Serial.println("Pobieranie pliku: " + fullPath);

        File file = SPIFFS.open(fullPath);
        //Serial.println("SPIFFS: " + SPIFFS + "/" + filename);

        if (file) 
        {
            if (file.size() > 0) 
            {         
              request->send(SPIFFS, fullPath, "application/octet-stream", true); //"application/octet-stream");"text/plain"
              file.close();
              Serial.println("Plik wysłany: " + filename);
            } 
            else 
            {
              request->send(404, "text/plain", "Plik jest pusty");
            }
        } 
        else 
        {
          Serial.println("Nie znaleziono pliku: " + fullPath);
          request->send(404, "text/plain", "Plik nie znaleziony");
        }
      } 
      else 
      {
        Serial.println("Brak parametru filename");
        request->send(400, "text/plain", "Brak nazwy pliku");
      }
    });
 
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "File Uploaded!");
      },
      //nullptr,
      [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }

      Serial.print("Upload File Name: ");
      Serial.println(filename);

      // Otwórz plik na karcie SPIFFS
      static File file; // Użyj statycznej zmiennej do otwierania pliku tylko raz
      if (index == 0) {
          file = SPIFFS.open(filename, FILE_WRITE);
          if (!file) {
              Serial.println("Failed to open file for writing");
              request->send(500, "text/plain", "Failed to open file for writing");
              return;
          }
      }

      // Zapisz dane do pliku
      size_t written = file.write(data, len);
      if (written != len) {
          Serial.println("Error writing data to file");
          file.close();
          request->send(500, "text/plain", "Error writing data to file");
          return;
      }

      // Jeśli to ostatni fragment, zamknij plik i wyślij odpowiedź do klienta
      if (final) {
          file.close();
          Serial.println("File upload completed successfully.");
          request->send(200, "text/plain", "File upload successful");
      } else {
          Serial.println("Received chunk of size: " + String(len));
      }
    });

    server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
      if (request->hasParam("url")) 
      {
        String inputMessage = request->getParam("url")->value();
        url2play = inputMessage.c_str();
        urlToPlay = true;
        Serial.println("Odebrany URL: " + inputMessage);
        request->send(200, "text/plain", "URL received");
      } 
      else 
      {
        request->send(400, "text/plain", "Missing URL parameter");
      }  
    });

    server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) 
    {
      String html = String(info_html);
  
      
      html.replace("%D1", String(softwareRev).c_str()); 
      html.replace("%D2", String(hostname).c_str()); 
      html.replace("%D3", String(WiFi.RSSI()).c_str()); 
      html.replace("%D4", String(wifiManager.getWiFiSSID()).c_str()); 
      html.replace("%D5", currentIP.c_str()); 
      html.replace("%D6", WiFi.macAddress().c_str()); 
      html.replace("%D0", String(ESP.getEfuseMac()).c_str()); 

      request->send(200, "text/html", html);

    });

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.begin();
    currentSelection = station_nr - 1; // ustawiamy stacje na liscie na obecnie odtwarzaczną przy starcie radia
    firstVisibleLine = currentSelection + 1; // pierwsza widoczna lina to grająca stacja przy starcie
    if (currentSelection + 1 >= stationsCount - 1) 
    {
      firstVisibleLine = currentSelection - 3;
    }  
    displayRadio();
    updateTimer();
  } 
  else 
  {
    Serial.println("Brak połączenia z siecią WiFi");  // W przypadku braku polaczenia wifi - wyslij komunikat na serial
    u8g2.clearBuffer();
    u8g2.setFont(spleen6x12PL);
    u8g2.drawStr(5, 13, "No network connection");  // W przypadku braku polaczenia wifi - wyswietl komunikat na wyswietlaczu OLED
    u8g2.drawStr(5, 26, "Connect to WiFi: ESP Internet Radio");
    u8g2.drawStr(5, 39, "Open web page http://192.168.4.1");
    u8g2.sendBuffer();
    while(true)
    { wifiManager.process(); } // Nieskonczona petla z procesowaniem Wifi aby nie przejsc do ekranu radia gdy nie ma Wifi
  }


}
// #######################################################################################  LOOP  ####################################################################################### //
void loop() 
{
  runTime1 = esp_timer_get_time();
  audio.loop();           // Wykonuje główną pętlę dla obiektu audio (np. odtwarzanie dźwięku, obsługa audio)
  button2.loop();         // Wykonuje pętlę dla obiektu button2 (sprawdza stan przycisku z enkodera 2)
  handleButtons();        // Wywołuje funkcję obsługującą przyciski i wykonuje odpowiednie akcje (np. zmiana opcji, wejście do menu)
  vTaskDelay(1);          // Krótkie opóźnienie, oddaje czas procesora innym zadaniom


  /*---------------------  FUNKCJA KLAWIATURA / Odczyt stanu klawiatura ADC pod GPIO 9 ---------------------*/
  if ((millis() - keyboardLastSampleTime >= keyboardSampleDelay) && (adcKeyboardEnabled)) // Sprawdzenie ADC - klawiatury 
  {
    keyboardLastSampleTime = millis();
    handleKeyboard();
  }
  
  /*---------------------  FUNKCJA DIMMER ---------------------*/
  if ((displayActive == true) && (displayDimmerActive == true) && (fwupd == false)) {displayDimmer(0);}  

  // Obsługa enkodera 2
  if (encoderFunctionOrder == 0) { handleEncoder2VolumeStationsClick(); } 
  else if (encoderFunctionOrder == 1) { handleEncoder2StationsVolumeClick(); }
  
 /*---------------------  FUNKCJA BACK / POWROTU ze wszystkich opcji Menu, Ustawien, itd ---------------------*/
  if ((fwupd == false) && (displayActive) && (millis() - displayStartTime >= displayTimeout))  // Przywracanie poprzedniej zawartości ekranu po 6 sekundach
  {
    //volumeBufferValue = volumeValue;
    if (volumeBufferValue != volumeValue) { saveVolumeOnSPIFFS(); }    
    
    // Jezeli nastapiła zmiana numeru stacji to wczytujemy nową stacje
    if ((rcInputDigitsMenuEnable == true) && (station_nr != stationFromBuffer)) { changeStation(); }
    
    displayDimmer(0); 
    clearFlags();
    displayRadio();
    u8g2.sendBuffer();
  }

 /*---------------------  FUNKCJA PILOT IR  / Obsluga pilota IR w kodzie NEC ---------------------*/ 
  if (bit_count == 32) // sprawdzamy czy odczytalismy w przerwaniu pełne 32 bity kodu IR NEC
  {
    if (ir_code != 0) // sprawdzamy czy zmienna ir_code nie jest równa 0
    {
      
      detachInterrupt(recv_pin);            // Rozpinamy przerwanie
      Serial.print("Kod NEC OK:");
      Serial.print(ir_code, HEX);
      ir_code = reverse_bits(ir_code,32);   // Rotacja bitów - zmiana porządku z LSB-MSB na MSB-LSB
      Serial.print("  MSB-LSB: ");
      Serial.print(ir_code, HEX);
    
      uint8_t CMD = (ir_code >> 16) & 0xFF; // Drugi bajt (inwersja adresu)
      uint8_t ADDR = ir_code & 0xFF;        // Czwarty bajt (inwersja komendy)
      
      Serial.print("  ADR:");
      Serial.print(ADDR, HEX);
      Serial.print(" CMD:");
      Serial.println(CMD, HEX);
      ir_code = ADDR << 8 | CMD;      // Łączymy ADDR i CMD w jedną zmienną 0x ADR CMD

      // Info o przebiegach czasowytch kodu pilota IR
      Serial.print("debug-- puls 9ms:"); 
      Serial.print(pulse_duration_9ms);
      Serial.print("  4.5ms:");
      Serial.print(pulse_duration_4_5ms);
      Serial.print("  1690us:");
      Serial.print(pulse_duration_1690us);
      Serial.print("  690us:");
      Serial.println(pulse_duration_560us);


      fwupd = false;        // Kasujemy flagę aktulizacji OTA gdyby była ustawiona
      //displayActive = true; // jesli odbierzemy kod z pilota to uatywnij wyswietlacz i wyłacz przyciemnienie OLEDa
      //displayDimmer(0); // jesli odbierzemy kod z pilota to wyłaczamy przyciemnienie wyswietlacza OLED
      displayPowerSave(0);
      
      if (ir_code == rcCmdVolumeUp)  { volumeUp(); }         // Przycisk głośniej
      else if (ir_code == rcCmdVolumeDown) { volumeDown(); } // Przycisk ciszej
      else if (ir_code == rcCmdArrowRight) // strzałka w prawo - nastepna stacja, bank lub nastawy equalizera
      {  
        if (bankMenuEnable == true)
        {
          bank_nr++;
          if (bank_nr > 16) 
          {
            bank_nr = 1;
          }
        bankMenuDisplay();
        }
        else if (equalizerMenuEnable == true)
        {
          if (toneSelect == 1) {toneHiValue++;}
          if (toneSelect == 2) {toneMidValue++;}
          if (toneSelect == 3) {toneLowValue++;}
          
          if (toneHiValue > 12) {toneHiValue = 12;}
          if (toneMidValue > 12) {toneMidValue = 12;}
          if (toneLowValue > 12) {toneLowValue = 12;}
          displayEqualizer();
        }     
        else
        {
          station_nr++;
          if (station_nr > stationsCount) { station_nr = stationsCount; }
          changeStation();
          displayRadio();
          u8g2.sendBuffer();
        }
      }
      else if (ir_code == rcCmdArrowLeft) // strzałka w lewo - poprzednia stacja, bank lub nastawy equalizera
      {  
        if (bankMenuEnable == true)
        {
          bank_nr--;
          if (bank_nr < 1) 
          {
            bank_nr = 16;
          }
        bankMenuDisplay();  
        }
        else if (equalizerMenuEnable == true)
        {
          if (toneSelect == 1) {toneHiValue--;}
          if (toneSelect == 2) {toneMidValue--;}
          if (toneSelect == 3) {toneLowValue--;}
          
          if (toneHiValue < -40) {toneHiValue = -40;}
          if (toneMidValue < -40) {toneMidValue = -40;}
          if (toneLowValue < -40) {toneLowValue = -40;}
         
          displayEqualizer();
        }     
        else
        {        
          station_nr--;
          if (station_nr < 1) { station_nr = 1; }
          changeStation();
          displayRadio();
          u8g2.sendBuffer();
        }
      }
      else if ((ir_code == rcCmdArrowUp) && (volumeSet == false) && (equalizerMenuEnable == true))
      {
        toneSelect--;
        if (toneSelect < 1){toneSelect = 1;}
        displayEqualizer();
      }
      else if ((ir_code == rcCmdArrowUp) && (equalizerMenuEnable == false))// Przycisk w góre
      {  
        if ((volumeSet == true) && (volumeBufferValue != volumeValue))
        {
          saveVolumeOnSPIFFS();
          volumeSet = false;
        }
        timeDisplay = false;
        displayActive = true;
        displayStartTime = millis();
        
        station_nr = currentSelection + 1;
        
        if (listedStations == true) {station_nr--;}
        //station_nr--;
        if (station_nr < 1) { station_nr = stationsCount; } // jesli dojdziemy do początku listy stacji to przewijamy na koniec
        
        scrollUp(); 
        displayStations();
      }
      else if ((ir_code == rcCmdArrowDown) && (volumeSet == false) && (equalizerMenuEnable == true))
      {
        toneSelect++;
        if (toneSelect > 3){toneSelect = 3;}
        displayEqualizer();
      }
      else if ((ir_code == rcCmdArrowDown) && (equalizerMenuEnable == false)) // Przycisk w dół
      {  
        if ((volumeSet == true) && (volumeBufferValue != volumeValue))
        {
          saveVolumeOnSPIFFS();
          volumeSet = false;
        }
        timeDisplay = false;
        displayActive = true;
        displayStartTime = millis();
        
        station_nr = currentSelection + 1;
        
        //station_nr++;
        if (listedStations == true) {station_nr++;}
        if (station_nr > stationsCount) 
	      {
          station_nr = 1;//stationsCount;
        }
        
        Serial.println(station_nr);
        scrollDown(); 
        displayStations();
      }    
      else if (ir_code == rcCmdOk)
      {
        if (bankMenuEnable == true)
        {
          station_nr = 1;
          fetchStationsFromServer(); // Ładujemy stacje z karty lub serwera 
          bankMenuEnable = false;
        }  
        if (equalizerMenuEnable == true) { saveEqualizerOnSPIFFS();}    // zapis ustawien equalizera
        //if (volumeSet == true) { saveVolumeOnSPIFFS();}                 // zapis ustawien głośnosci po nacisnięciu OK, wyłaczony aby można było przełączyć stacje na www bez czekania
        //if ((equalizerMenuEnable == false) && (volumeSet == false)) // jesli nie zapisywaliśmy equlizer i glonosci to wywolujemy ponizsze funkcje
        if ((equalizerMenuEnable == false)) // jesli nie zapisywaliśmy equlizer 
        {
          changeStation(); 
          clearFlags();                                             // Czyscimy wszystkie flagi przebywania w różnych menu
          displayRadio();
          u8g2.sendBuffer();
        }
        equalizerMenuEnable = false; // Kasujemy flage ustawiania equalizera
        volumeSet = false; // Kasujemy flage ustawiania głośnosci
      } 
      else if (ir_code == rcCmdKey0) {rcInputKey(0);}
      else if (ir_code == rcCmdKey1) {rcInputKey(1);}     
      else if (ir_code == rcCmdKey2) {rcInputKey(2);}     
      else if (ir_code == rcCmdKey3) {rcInputKey(3);}     
      else if (ir_code == rcCmdKey4) {rcInputKey(4);}     
      else if (ir_code == rcCmdKey5) {rcInputKey(5);}     
      else if (ir_code == rcCmdKey6) {rcInputKey(6);}     
      else if (ir_code == rcCmdKey7) {rcInputKey(7);}     
      else if (ir_code == rcCmdKey8) {rcInputKey(8);}     
      else if (ir_code == rcCmdKey9) {rcInputKey(9);}
      else if (ir_code == rcCmdBack) 
      {  
        displayDimmer(0);
        clearFlags();   // Zerujemy wszystkie flagi
        displayRadio(); // Ładujemy erkran radia
        u8g2.sendBuffer(); // Wysyłamy bufor na wyswietlacz
      }
      else if (ir_code == rcCmdMute) 
      {
        volumeMute = !volumeMute;
        if (volumeMute == true)
        {
          audio.setVolume(0);   
        }
        else if (volumeMute == false)
        {
          audio.setVolume(volumeValue);   
        }
        displayRadio();
      }
      else if (ir_code == rcCmdDirect) // Przycisk Direct -> Menu Bank - udpate GitHub, Menu Equalizer - reset wartosci, Radio Display - fnkcja przyciemniania ekranu
      {
        if ((bankMenuEnable == true) && (equalizerMenuEnable == false))// flage można zmienic tylko bedąc w menu wyboru banku
        { 
          bankNetworkUpdate = !bankNetworkUpdate; // zmiana flagi czy aktualizujemy bank z sieci czy karty SPIFFS
          bankMenuDisplay(); 
        }
        if ((bankMenuEnable == false) && (equalizerMenuEnable == true))
        {
          toneHiValue = 0;
          toneMidValue = 0;
          toneLowValue = 0;    
          displayEqualizer();
        }
        if ((bankMenuEnable == false) && (equalizerMenuEnable == false) && (volumeSet == false))
        { 
          
          displayDimmer(!displayDimmerActive); // Dimmer OLED
          Serial.println("Właczono display Dimmer rcCmdDirect");
        }
      }      
      else if (ir_code == rcCmdSrc) 
      {
        displayMode++;
        if (displayMode > 2) {displayMode = 0;}
        displayRadio();
        //u8g2.sendBuffer();
        clearFlags();
        ActionNeedUpdateTime = true;
      }
      else if (ir_code == rcCmdRed) 
      {     
       //voiceTime();
        vuMeterMode = !vuMeterMode;
        
      }
      else if (ir_code == rcCmdGreen) 
      {
        //voiceTimeEn();
        voiceTime();
              
      }   
      else if (ir_code == rcCmdBankMinus) 
      {
        if (bankMenuEnable == true)
        {
          bank_nr--;
          if (bank_nr < 1) 
          {
            bank_nr = 16;
          }
        }  
        
        bankMenuDisplay();
      }
      else if (ir_code == rcCmdBankPlus) 
      {
        if (bankMenuEnable == true)
        {
          bank_nr++;
          if (bank_nr > 16) 
          {
            bank_nr = 1;
          }
        }       
        bankMenuDisplay();
      }
     
      else if (ir_code == rcCmdAud) {displayEqualizer();}
      else { Serial.println("Inny przycisk"); }
    }
    else
    {
      Serial.println("Błąd - kod pilota NEC jest niepoprawny!");
      Serial.print("debug-- puls 9ms:");
      Serial.print(pulse_duration_9ms);
      Serial.print("  4.5ms:");
      Serial.print(pulse_duration_4_5ms);
      Serial.print("  1690us:");
      Serial.print(pulse_duration_1690us);
      Serial.print("  690us:");
      Serial.println(pulse_duration_560us);    
    }
    ir_code = 0;
    bit_count = 0;
    attachInterrupt(digitalPinToInterrupt(recv_pin), pulseISR, CHANGE);
    
    // Serial.print("debug-- Czas2 - Czas1 = ");
    // runTime2 = runTime2 - runTime1;
    // Serial.println(runTime);
    // Serial.print("debug-- Kontrola stosu:");
    // Serial.print(uxTaskGetStackHighWaterMark(NULL));
    // Serial.println(" DWORD");
  }
  

  /*---------------------  FUNKCJA PETLI MILLIS SCROLLER / Odswiezanie VU Meter, Time, Scroller, OLED, WiFi ver. 1 ---------------------*/ 
  if ((millis() - scrollingStationStringTime > scrollingRefresh) && (displayActive == false)) 
  {
    scrollingStationStringTime = millis();
    if (ActionNeedUpdateTime == true) // Aktualizacja zegara, zegar głosowy, debug Audio, sygnał wifi 
    {
      ActionNeedUpdateTime = false;
      updateTimer();
   
      if (voiceTimePlay == true) // Zegar głosowy
      {
        voiceTimePlay = false;
        voiceTime();
      }

      if (debugAudioBuffor == true) 
      { 
        bufforAudioInfo(); 
        drawSignalPower(194,63,1); // Narysuj wskaznik zasiegu WiFi X,Y z wydrukiem na terminalu     
      }
      else
      {
        if ((displayMode == 0) || (displayMode == 2)) {drawSignalPower(194,63,0);} // x, y, 0-bez wydruku mocy sygnału na terminalu , 1-z wydrukiem
        if ((displayMode == 1) && (volumeMute == false)) {drawSignalPower(244,47,0);}
      }
      
      if ((audioInfoRefresh == true) && (displayActive == false)) // Zmiana streamtitle - wymaga odswiezenia na wyswietlaczu
      { 
        audioInfoRefresh = false;
        stationStringFormatting(); // Formatujemy StationString do wyswietlenia przez Scroller
      } 

        if ((audioInfoRefresh2 == true) && (displayActive == false)) // Zmiana bitrate - wymaga odswiezenia na wyswietlaczu
      { 
        audioInfoRefresh2 = false;
        displayRadio();
      } 

      if (wsAudioRefresh == true)
      {
        wsAudioRefresh = false;
        wsStreamInfoRefresh();
      }
    }
            
    if (volumeMute == true)   // Obsługa wyciszenia dzwięku, wprowadzamy napis MUTE na ekran
    {   
      u8g2.setDrawColor(0);
      if (displayMode == 0) {u8g2.drawStr(0,48, "> MUTED <");}
      if (displayMode == 1) {u8g2.drawStr(200,47, "> MUTED <");}
      u8g2.setDrawColor(1);
    } 
    
    //Rysujmey wskaznik VU jesli flaga rysowania aktywna, mute = 0 i tylko trybie displayMode 0
    if (vuMeterOn == true && displayActive == false && displayMode == 0 && volumeMute == false) {vuMeter();}
    
    if (urlToPlay == true) // Jesli web serwer ustawił flagę "odtwarzaj URL" to uruchamiamy funkcje odtwarzania z adresu URL wysłanego przez strone WWW
    {
      urlToPlay = false;
      webUrlStationPlay();
      displayRadio();
    }
    
    displayRadioScroller();  // wykonujemy przewijanie tekstu station stringi przygotowujemy bufor ekranu
    u8g2.sendBuffer();  // rysujemy całą zawartosc ekranu.
   
  }



 
  //runTime2 = esp_timer_get_time();
  //runTime = runTime2 - runTime1;  
}

