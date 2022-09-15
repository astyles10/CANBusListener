#include <CAN.h>
#include <LiquidCrystal.h>
#include <OBD2.h>

#include <iterator>
#include <vector>

class ScreenMenu {
 public:
  typedef void (*ScreenCallback)(String&, String&);
  typedef struct Screen {
    const String fLine1;
    const String fLine2;
    ScreenCallback fCallback;
    Screen(const String& inLine1, const String& inLine2,
           ScreenCallback inCallback)
        : fLine1(inLine1), fLine2(inLine2) {
      fCallback = inCallback;
    };
    Screen operator=(const Screen& other) {
      return {other.fLine1, other.fLine2, other.fCallback};
    }
  } YScreen;

  ScreenMenu(LiquidCrystal& inLcdScreen)
      : fLcdScreen(inLcdScreen), fCurrentScreenNumber(0){};
  ~ScreenMenu() = default;

  void MoveDown() {
    if (--fCurrentScreenNumber < 0) {
      fCurrentScreenNumber = fScreens.size() - 1;
    }
    ChangeDisplay(fScreens[fCurrentScreenNumber]);
  }
  void MoveUp() {
    if (++fCurrentScreenNumber >= fScreens.size()) {
      fCurrentScreenNumber = 0;
    }
    ChangeDisplay(fScreens[fCurrentScreenNumber]);
  }
  void MoveLeft() {}
  void MoveRight() {}

  void RegisterScreen(const YScreen& newScreen) {
    fScreens.push_back(newScreen);
  }

 private:
  void ChangeDisplay(Screen& inScreen) {
    fLcdScreen.clear();
    fLcdScreen.setCursor(0, 0);
    String aLine1, aLine2;
    inScreen.fCallback(aLine1, aLine2);
    fLcdScreen.write(aLine1.c_str());
    fLcdScreen.setCursor(0, 1);
    fLcdScreen.write(aLine2.c_str());
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

static constexpr char ScreenEngineStats1[] = "%.0f rpm %.0f km/h";
static constexpr char ScreenEngineStats2[] = "Ld: %.0f\% Thr: %.0f\%";

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

void ConfigureScreenMenu() {
  gScreenMenu.RegisterScreen(
      {ScreenEngineStats1, ScreenEngineStats2, FormatEngineStatsScreen});
  gScreenMenu.RegisterScreen({ScreenDebug1, ScreenDebug2, NULL});
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

void FormatEngineStatsScreen(String& Line1, String& Line2) {
  char aLineBuffer[17];
  snprintf(aLineBuffer, 16, ScreenEngineStats1, OBD2.pidRead(ENGINE_RPM),
           OBD2.pidRead(VEHICLE_SPEED));

  Line1 = aLineBuffer;
  snprintf(aLineBuffer, 16, ScreenEngineStats2, 1, 2);
  Line2 = aLineBuffer;
}

void FormatDebugScreen(String& Line1, String& Line2) {
  char aLineBuffer[17];
  snprintf(aLineBuffer, 16, ScreenDebug1, OBD2.pidRead(OBD_STANDARDS_THIS_VEHICLE_CONFORMS_TO));
  Line1 = aLineBuffer;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  LcdScreen.begin(16, 2);
  SetDefaultLcdScreen();
  ConfigureScreenMenu();
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
