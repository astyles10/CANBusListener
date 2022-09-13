#include <LiquidCrystal.h>

static constexpr uint8_t LCD_Reset = 8;
static constexpr uint8_t LCD_Enable = 9;
static constexpr uint8_t LCD_D7 = 7;
static constexpr uint8_t LCD_D6 = 6;
static constexpr uint8_t LCD_D5 = 5;
static constexpr uint8_t LCD_D4 = 4;
static constexpr uint8_t LCD_Backlight = 10;
static constexpr uint8_t BUTTON_PIN_ANALOG = 0;

enum ButtonAnalogValues {
  BUTTON_RIGHT = 60,
  BUTTON_UP = 200,
  BUTTON_DOWN = 400,
  BUTTON_LEFT = 600,
  BUTTON_SELECT = 800
};

LiquidCrystal LcdScreen( LCD_Reset, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7 );

void setup () {
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println("Resetted");
  LcdScreen.begin(16, 2);
  LcdScreen.setCursor(0, 0);
  LcdScreen.print("abcd1234");
  LcdScreen.setCursor(0, 1);
  LcdScreen.print("Press Key:");
}

void loop() {
  int buttonRead = analogRead(BUTTON_PIN_ANALOG);
  LcdScreen.setCursor(10, 1);
  if (buttonRead <= BUTTON_RIGHT) {
    clearScreen();
    LcdScreen.print("Right ");
  } else if (buttonRead <= BUTTON_UP) {
    clearScreen();
    LcdScreen.print("Up    ");
  } else if (buttonRead <= BUTTON_DOWN) {
    clearScreen();
    LcdScreen.print("Down  ");
  } else if (buttonRead <= BUTTON_LEFT) {
    clearScreen();
    LcdScreen.print("Left  ");
  } else if (buttonRead <= BUTTON_SELECT) {
    clearScreen();
    LcdScreen.print("Select");
  }
}

void clearScreen() {
  // for (uint8_t i = 10; i < 16; ++i) {
  //   LcdScreen.print(" ");
  // }
}