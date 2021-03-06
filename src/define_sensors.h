// Temperature meter
#define DHTPIN 7      // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22  (AM2302)

// Software serial
#define SOFT_S_RX 2
#define SOFT_S_TX 3

// Buttons
#define SCREEN_BUTTON_PIN 8
#define RESET_BUTTON_PIN 9
#define INSTANT_BUTTON_PIN 5

#define TEN_MINUTES 600000
#define TWO_SECONDS 2000

// 0X3C+SA0 - 0x3C or 0x3D
#define DISPLAY_ADDRESS 0x3C
#define RST_PIN -1