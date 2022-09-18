// Downloaded CAN and OBD2 libraries from Arduino library manager
#include <CAN.h>
#include <OBD2.h>

#include "OBDDefs.hpp"
#include "ScreenHelper.hpp"
#include "TimerHelper.hpp"

static constexpr uint8_t LCD_Reset = 9;
static constexpr uint8_t LCD_Enable = 8;
static constexpr uint8_t LCD_D7 = 4;
static constexpr uint8_t LCD_D6 = 5;
static constexpr uint8_t LCD_D5 = 6;
static constexpr uint8_t LCD_D4 = 7;
static constexpr uint8_t BUTTON_PIN_ANALOG = 0;

static constexpr char PageEngineStats1[] = "%lurpm %lukm/h";
static constexpr char PageEngineStats2[] = "Ld: %u Thr: %u";



static constexpr char PageFuelInfo1[] = "Fuel: %s";
static constexpr char PageFuelInfo2[] = "Tank: %u/100";

static constexpr char PageDebug1[] = "OBD Std: %s";
static constexpr char PageDebug2[] = "ECU: %s";

static constexpr uint16_t kDebounceMs = 100;
// Rename "ScreenMenu" to something more reflective of the actual class purpose
ScreenMenu* gpScreenMenu;
volatile static uint8_t gButtonPinInput = 0;
static uint32_t gLastButtonPressMs = 0;
volatile static bool gRefreshPage = false;

void ConfigureScreenMenu() {
  gpScreenMenu->RegisterPage({true, FormatEngineStatsPage});
  gpScreenMenu->RegisterPage({true, FormatFuelInfoPage});
  gpScreenMenu->RegisterPage({false, FormatDebugPage});
  gpScreenMenu->SetDefaultScreen();
}

void OBDConnect() {
  gpScreenMenu->WriteTemporaryPage("Connecting", "");
  while (true) {
    if (!OBD2.begin()) {
      delay(1000);
    } else {
      gpScreenMenu->ClearLcdScreen();
      gpScreenMenu->WriteTemporaryPage("Connected!", "");
      break;
    }
  }
}

void FormatEngineStatsPage(String& Line1, String& Line2) {
  char aLineBuffer[17];
  {
    unsigned long aEngineSpeed = lround(OBD2.pidRead(ENGINE_RPM));
    unsigned long aVehicleSpeed = lround(OBD2.pidRead(VEHICLE_SPEED));

    snprintf(aLineBuffer, 16, PageEngineStats1, aEngineSpeed, aVehicleSpeed);
    Line1 = aLineBuffer;
  }
  {
    unsigned long aEngineLoad = lround(OBD2.pidRead(CALCULATED_ENGINE_LOAD));
    unsigned long aThrottlePosition = lround(OBD2.pidRead(THROTTLE_POSITION));
    snprintf(aLineBuffer, 16, PageEngineStats2, aEngineLoad,
             aThrottlePosition);
    Line2 = aLineBuffer;
  }
}

void FormatFuelInfoPage(String& Line1, String& Line2) {
  char aLineBuffer[33];
  {
    long aFuelType = lround(OBD2.pidRead(FUEL_TYPE));
    snprintf(aLineBuffer, 32, PageFuelInfo1, DetermineFuelType(aFuelType));
    Line1 = aLineBuffer;
  }
  {
    uint8_t aFuelLevel =
        static_cast<uint8_t>(OBD2.pidRead(FUEL_TANK_LEVEL_INPUT));
    snprintf(aLineBuffer, 32, PageFuelInfo2, aFuelLevel);
    Line2 = aLineBuffer;
  }
}

void FormatDebugPage(String& Line1, String& Line2) {
  char aLineBuffer[33];
  uint8_t aObdStandard = static_cast<uint8_t>(
      OBD2.pidRead(OBD_STANDARDS_THIS_VEHICLE_CONFORMS_TO));
  snprintf(aLineBuffer, 32, PageDebug1, DetermineOBDStandard(aObdStandard));
  Line1 = aLineBuffer;
  // snprintf(aLineBuffer, 32, PageDebug2, OBD2.ecuNameRead().c_str());
  // Line2 = aLineBuffer;
  // Line2 = OBD2.vinRead();
}

void HandleButtonRead() {
  if ((millis() - gLastButtonPressMs) < kDebounceMs) {
    return;
  }

  switch (gButtonPinInput) {
    case A1:
      gpScreenMenu->MoveLeft();
      gLastButtonPressMs = millis();
      break;
    case A2:
      gpScreenMenu->MoveRight();
      gLastButtonPressMs = millis();
      break;
    case A3:
      gpScreenMenu->MoveDown();
      gLastButtonPressMs = millis();
      break;
    case A4:
      gpScreenMenu->MoveUp();
      gLastButtonPressMs = millis();
      break;
  }
  if (gButtonPinInput) {
    gButtonPinInput = 0;
  }
}

void ConfigureInterrupts() {
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  // Enable interrupts on analog pins (PCINT1)
  PCICR |= 0b00000010;
  PCMSK1 |= 0b00011110;
}

void ConfigureTimer() {
  TimerHelper aTimerSettings = TimerHelper::DetermineTimerSettings(8);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = aTimerSettings.GetTicks();
  TCCR1B |= aTimerSettings.GetPrescaler().fCSRegisters;
  TCCR1B |= (1 << WGM12);
  TIMSK1 |= (1 << OCIE1A);
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  LiquidCrystal aLcdScreen(LCD_Reset, LCD_Enable, LCD_D4, LCD_D5, LCD_D6,
                           LCD_D7);
  gpScreenMenu = new ScreenMenu(aLcdScreen);
  gpScreenMenu->ClearLcdScreen();
  OBDConnect();
  ConfigureScreenMenu();
  noInterrupts();
  ConfigureInterrupts();
  ConfigureTimer();
  interrupts();
}

void loop() {
  HandleButtonRead();
  if (gRefreshPage) {
    gpScreenMenu->UpdatePage();
    gRefreshPage = false;
  }
}

// https://www.electrosoftcloud.com/en/pcint-interrupts-on-arduino/
ISR(PCINT1_vect) {
  if (digitalRead(A1)) {
    gButtonPinInput = A1;
  } else if (digitalRead(A2)) {
    gButtonPinInput = A2;
  } else if (digitalRead(A3)) {
    gButtonPinInput = A3;
  } else if (digitalRead(A4)) {
    gButtonPinInput = A4;
  }
}

ISR(TIMER1_COMPA_vect) { gRefreshPage = true; }
