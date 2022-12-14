// Downloaded CAN and OBD2 libraries from Arduino library manager
#include <CAN.h>
#include <MemoryUsage.h>
#include <OBD2.h>
#include <SD.h>
#include <SPI.h>

#include "OBDDefs.hpp"
#include "ScreenHelper.hpp"
#include "TimerHelper.hpp"

static constexpr uint8_t LCD_RegisterSelect = 9;
static constexpr uint8_t LCD_Enable = 8;
static constexpr uint8_t LCD_D7 = 4;
static constexpr uint8_t LCD_D6 = 5;
static constexpr uint8_t LCD_D5 = 6;
static constexpr uint8_t LCD_D4 = 7;
static constexpr uint8_t BUTTON_PIN_ANALOG = 0;

static constexpr uint8_t SD_CS_PIN = 4;
static constexpr char LOGFILE_NAME[] = "Logfile.csv";

// Live Stats page lines
static constexpr char kLineVehicleSpeed[] = "Veh Spd: %lukm/h";
static constexpr char kLineEngineSpeed[] = "Eng Spd: %lurpm";
static constexpr char kLineEngineLoad[] = "Eng Load: %u/100";
static constexpr char kLineTimingAdvance[] = "Timing Adv: %dC";
static constexpr char kLineIntakeAirTemp[] = "Intake Air: %dC";
static constexpr char kLineThrottlePosition[] = "Throttle %u/100";
static constexpr char kLineCurrentRuntimeSeconds[] = "Runtime: %us";
static constexpr char kLineManifoldAbsolutePressure[] = "MAP: %ukPa";
static constexpr char kLineAmbientAirTemp[] = "Amb. Air: %dC";
static constexpr char kLineTimeSinceCodesCleared[] = "Code age %um";

static constexpr char kLineFuelType[] = "Fuel: %s";
static constexpr char kLineFuelTankLevel[] = "Tank: %u/100";

static constexpr char PageDebug1[] = "OBD Std: %s";
static constexpr char PageDebug2[] = "ECU: %s";

static constexpr uint16_t kDebounceMs = 100;
// Rename "ScreenMenu" to something more reflective of the actual class purpose
ScreenMenu* gpScreenMenu;
volatile static uint8_t gButtonPinInput = 0;
static uint32_t gLastButtonPressMs = 0;
volatile static bool gTimerTriggered = false;

File gLogFile;
static bool gLogfileLoaded = false;
static bool gEnableLogging = true;

void InitScreen() {
  gpScreenMenu->RegisterPage({true, FormatSpeedPage});
  // gpScreenMenu->RegisterPage({true, FormatAirTemperaturePage});
  // gpScreenMenu->RegisterPage({true, FormatFuelInfoPage});
  // gpScreenMenu->RegisterPage({true, FormatRuntimeStats});
  // gpScreenMenu->RegisterPage({false, FormatDebugPage});
  gpScreenMenu->RegisterPage({false, FormatLoggingPage});
  gpScreenMenu->SetDefaultScreen();
}

void InitOBD() {
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

void FormatSpeedPage(String& Line1, String& Line2) {
  char aLineBuffer[17];
  {
    unsigned long aVehicleSpeed = lround(OBD2.pidRead(VEHICLE_SPEED));
    snprintf(aLineBuffer, 16, kLineVehicleSpeed, aVehicleSpeed);
    Line1 = aLineBuffer;
  }
  {
    unsigned long aEngineSpeed = lround(OBD2.pidRead(ENGINE_RPM));
    snprintf(aLineBuffer, 16, kLineEngineSpeed, aEngineSpeed);
    Line2 = aLineBuffer;
  }
}

void FormatEngineLoadValuePage(String& Line1, String& Line2) {
  // unsigned long aEngineLoad = lround(OBD2.pidRead(CALCULATED_ENGINE_LOAD));
}

void FormatAirTemperaturePage(String& Line1, String& Line2) {
  char aLineBuffer[17];
  {
    long aAmbientAirTemp = lround(OBD2.pidRead(AMBIENT_AIR_TEMPERATURE));
    snprintf(aLineBuffer, 16, kLineAmbientAirTemp, aAmbientAirTemp);
    Line1 = aLineBuffer;
  }
  {
    long aIntakeAirTemp = lround(OBD2.pidRead(AIR_INTAKE_TEMPERATURE));
    snprintf(aLineBuffer, 16, kLineIntakeAirTemp, aIntakeAirTemp);
    Line2 = aLineBuffer;
  }
}

void FormatThrottlePositionPage(String& Line1, String& Line2) {
  // unsigned long aThrottlePosition = lround(OBD2.pidRead(THROTTLE_POSITION));
}

void FormatFuelInfoPage(String& Line1, String& Line2) {
  char aLineBuffer[33];
  {
    long aFuelType = lround(OBD2.pidRead(FUEL_TYPE));
    snprintf(aLineBuffer, 32, kLineFuelType, DetermineFuelType(aFuelType));
    Line1 = aLineBuffer;
  }
  {
    uint8_t aFuelLevel =
        static_cast<uint8_t>(OBD2.pidRead(FUEL_TANK_LEVEL_INPUT));
    snprintf(aLineBuffer, 32, kLineFuelTankLevel, aFuelLevel);
    Line2 = aLineBuffer;
  }
}

void FormatRuntimeStats(String& Line1, String& Line2) {
  char aLineBuffer[17];
  {
    long aRuntimeSeconds = lround(OBD2.pidRead(RUN_TIME_SINCE_ENGINE_START));
    snprintf(aLineBuffer, 16, kLineCurrentRuntimeSeconds, aRuntimeSeconds);
    Line1 = aLineBuffer;
  }
  Line2 = "";
}

void FormatDebugPage(String& Line1, String& Line2) {
  char aLineBuffer[21];
  uint8_t aObdStandard = static_cast<uint8_t>(
      OBD2.pidRead(OBD_STANDARDS_THIS_VEHICLE_CONFORMS_TO));
  snprintf(aLineBuffer, 19, PageDebug1, DetermineOBDStandard(aObdStandard));
  Line1 = aLineBuffer;
}

void FormatLoggingPage(String& Line1, String& Line2) {
  if (gEnableLogging && gLogfileLoaded) {
    char aBuffer[65];
    int aNumBytes = snprintf(aBuffer, 64, "Millis: %lu", millis());
    // gLogFile.write(aBuffer, aNumBytes);
    Line1 = "Logging Enabled!";
    Line2 = "";
  }
}

void InitSDCard() {
  if (!gLogfileLoaded) {
    gLogFile = SD.open(LOGFILE_NAME, FILE_WRITE);
    if (gLogFile) {
      gLogfileLoaded = true;
    }
  }
}

void HandleButtonRead() {
  if ((millis() - gLastButtonPressMs) < kDebounceMs) {
    return;
  }

  switch (gButtonPinInput) {
    case A1:
      gpScreenMenu->DoButtonThree();
      gLastButtonPressMs = millis();
      break;
    case A2:
      gpScreenMenu->DoButtonFour();
      gLastButtonPressMs = millis();
      break;
    case A3:
      gpScreenMenu->DoButtonTwo();
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

void InitInterrupts() {
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  // Enable interrupts on analog pins (PCINT1)
  PCICR |= 0b00000010;
  PCMSK1 |= 0b00011110;
}

void InitTimer() {
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
  Serial.println("Start program");
  MEMORY_PRINT_FREERAM;
  MEMORY_PRINT_HEAPSTART;

  LiquidCrystal aLcdScreen(LCD_RegisterSelect, LCD_Enable, LCD_D4, LCD_D5, LCD_D6,
                           LCD_D7);
  gpScreenMenu = new ScreenMenu(aLcdScreen);
  gpScreenMenu->ClearLcdScreen();

  MEMORY_PRINT_FREERAM;
  MEMORY_PRINT_HEAPEND;
  Serial.println("End init LCD screen");

  // if (!SD.begin(SD_CS_PIN)) {
  //   gpScreenMenu->WriteTemporaryPage("Failed to", "initialize SD!");
  //   delay(1000);
  // } else {
  //   Serial.println("Load log file:");
  //   MEMORY_PRINT_FREERAM;
  //   MEMORY_PRINT_HEAPSTART;
  //   HandleLoadLogFile();
  //   MEMORY_PRINT_FREERAM;
  //   MEMORY_PRINT_HEAPEND;
  //   Serial.println("----------");
  // }
  // OBDConnect();
  Serial.println("Load Screen Menu:");
  MEMORY_PRINT_FREERAM;
  MEMORY_PRINT_HEAPSTART;
  InitScreen();
  MEMORY_PRINT_FREERAM;
  MEMORY_PRINT_HEAPEND;
  noInterrupts();
  InitInterrupts();
  InitTimer();
  interrupts();
  Serial.println("End program");
}

void loop() {
  HandleButtonRead();
  if (gTimerTriggered) {
    gpScreenMenu->DoRefresh();
    gTimerTriggered = false;
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

ISR(TIMER1_COMPA_vect) { gTimerTriggered = true; }
