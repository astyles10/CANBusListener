#include <CAN.h>
#include <LiquidCrystal.h>
#include <OBD2.h>

#include <iterator>
#include <vector>

class ScreenMenu {
 public:
  typedef void (*ScreenCallback)(void);
  typedef struct Screen {
    const String fLine1;
    const String fLine2;
    const ScreenCallback fCallback;
    Screen(const String& inLine1, const String& inLine2,
           ScreenCallback& inCallback)
        : fLine1(inLine1), fLine2(inLine2), fCallback(inCallback){};
    Screen(const String& inLine1, const String& inLine2)
        : fLine1(inLine1), fLine2(inLine2), fCallback(NULL){};
  } Screen;

  ScreenMenu(LiquidCrystal& inLcdScreen)
      : fLcdScreen(inLcdScreen), fCurrentScreenNumber(0){};
  ~ScreenMenu() = default;

  void MoveDown() {
    if (--fCurrentScreenNumber < 0) {
      fCurrentScreenNumber = fScreens.size() - 1;
    }
    ChangeDisplay(fScreens.at(fCurrentScreenNumber));
  }
  void MoveUp() {
    if (++fCurrentScreenNumber >= fScreens.size()) {
      fCurrentScreenNumber = 0;
    }
    ChangeDisplay(fScreens.at(fCurrentScreenNumber));
  }
  void MoveLeft() {}
  void MoveRight() {}

  void RegisterScreen(const Screen& newScreen) { fScreens.push_back(newScreen); }

 private:
  void ChangeDisplay(Screen& inScreen) {
    fLcdScreen.clear();
    fLcdScreen.setCursor(0, 0);
    // The string needs to be formatted in the callback
    if (inScreen.fCallback) {
    }
    inScreen.fCallback();
    fLcdScreen.write(inScreen.fLine1.c_str());
    fLcdScreen.write(inScreen.fLine2.c_str());
  }

  LiquidCrystal& fLcdScreen;
  std::vector<Screen> fScreens;
  int8_t fCurrentScreenNumber;
};

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

static constexpr char ScreenEngineStats1[] = "%u rpm %u km/h";
static constexpr char ScreenEngineStats2[] = "Ld: %u\% Thr: %u\%";

static constexpr char ScreenDebug1[] = "OBD Std: %s";
static constexpr char ScreenDebug2[] = "VIN: %s";

static constexpr char ScreenTest1[] = "Line 1";
static constexpr char ScreenTest2[] = "Line 2";

static constexpr char ScreenTest3[] = "Line 3";
static constexpr char ScreenTest4[] = "Line 4";

static uint8_t currentScreenNumber = 1;

LiquidCrystal LcdScreen(LCD_Reset, LCD_Enable, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
ScreenMenu gScreenMenu(LcdScreen);

void SetDefaultLcdScreen() {
  LcdScreen.clear();
  LcdScreen.setCursor(0, 0);
}

void DoLcdScreenWait() {
  static uint8_t column = 10;
  LcdScreen.setCursor(column++, 0);
  LcdScreen.write(".");
  if (column > 16) {
    column = 10;
    SetDefaultLcdScreen();
  }
}

void OBDConnect() {
  LcdScreen.print("Connecting");
  while (true) {
    if (!OBD2.begin()) {
      DoLcdScreenWait();
      delay(1000);
    } else {
      LcdScreen.clear();
      LcdScreen.setCursor(0, 0);
      LcdScreen.write("Connected!");
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  LcdScreen.begin(16, 2);
  SetDefaultLcdScreen();
  const ScreenMenu::Screen aTestScreen(String(ScreenTest1), String(ScreenTest2));
  gScreenMenu.RegisterScreen(aTestScreen);
  // OBDConnect();
  // char buf[17];
  // snprintf(buf, 16, ScreenEngineStats1, 1, 2);
  // LcdScreen.print(buf);

  // String val = OBD2.vinRead();
  // float speed = OBD2.pidRead(0x0C);
}

void loop() {
  int buttonRead = analogRead(BUTTON_PIN_ANALOG);
  if (buttonRead <= BUTTON_RIGHT) {
    // menuChange(++currentScreenNumber);
  } else if (buttonRead <= BUTTON_UP) {
    menuChange(--currentScreenNumber);
  } else if (buttonRead <= BUTTON_DOWN) {
    menuChange(++currentScreenNumber);
  } else if (buttonRead <= BUTTON_LEFT) {
  } else if (buttonRead <= BUTTON_SELECT) {
  }
}

void menuChange(uint8_t menuNumber) {
  switch (menuNumber) {};
}
