// Downloaded CAN and OBD2 libraries from Arduino library manager
#include <CAN.h>
// #include <MemoryUsage.h>
#include <OBD2.h>
#include <SD.h>
#include <SPI.h>

#include "OBDDefs.hpp"
#include "ScreenHelperNoVector.hpp"
#include "TimerHelper.hpp"

static constexpr uint8_t LCD_Reset = 2;
static constexpr uint8_t LCD_Enable = 8;
static constexpr uint8_t LCD_D7 = 3;
static constexpr uint8_t LCD_D6 = 5;
static constexpr uint8_t LCD_D5 = 6;
static constexpr uint8_t LCD_D4 = 7;

static constexpr uint8_t SD_CS_PIN = 4;
static constexpr char LOGFILE_NAME[] = "CarLog.csv";

// Live Stats page lines
static constexpr char LineVehicleSpeed[] = "Veh Spd: %lukm/h";
static constexpr char LineEngineSpeed[] = "Eng Spd: %lurpm";
static constexpr char LineEngineLoad[] = "Eng Load: %u/100";
static constexpr char LineTimingAdvance[] = "Timing Adv: %dC";
static constexpr char LineIntakeAirTemp[] = "Intake Air: %dC";
static constexpr char LineThrottlePosition[] = "Throttle %u/100";
static constexpr char LineCurrentRuntimeSeconds[] = "Runtime: %us";
static constexpr char LineManifoldAbsolutePressure[] = "MAP: %ukPa";
static constexpr char LineAmbientAirTemp[] = "Amb. Air: %dC";
static constexpr char LineTimeSinceCodesCleared[] = "Code age %um";

static constexpr char PageFuelInfo1[] = "Fuel: %s";
static constexpr char PageFuelInfo2[] = "Tank: %u/100";

static constexpr uint16_t kDebounceMs = 100;
// Rename "ScreenMenu" to something more reflective of the actual class purpose
ScreenMenu* gpScreenMenu;
volatile static uint8_t gButtonPinInput = 0;
static uint32_t gLastButtonPressMs = 0;
volatile static bool gTimerTriggered = false;

bool gLoggingEnabled = false;
File gLogfile;

void ConfigureScreenMenu() {
  gpScreenMenu->RegisterPage({true, FormatSpeedPage});
  gpScreenMenu->RegisterPage({true, FormatAirTemperaturePage});
  // gpScreenMenu->RegisterPage({true, FormatFuelInfoPage});
  gpScreenMenu->RegisterPage({true, FormatRuntimeStats});
  // gpScreenMenu->RegisterPage({false, FormatDebugPage});
  // gpScreenMenu->RegisterPage({false, FormatLoggingPage, HandleLoadLogFile});
  gpScreenMenu->SetDefaultScreen();
}

void OBDConnect() {
  gpScreenMenu->WriteTemporaryPage("Connecting", "");
  while (true) {
    if (!OBD2.begin()) {
      delay(1000);
    } else {
      gpScreenMenu->ClearLcdScreen();
      break;
    }
  }
}

void FormatSpeedPage(String& Line1, String& Line2) {
  char aLineBuffer[17];
  {
    unsigned long aVehicleSpeed = lround(OBD2.pidRead(VEHICLE_SPEED));
    snprintf(aLineBuffer, 16, LineVehicleSpeed, aVehicleSpeed);
    Line1 = aLineBuffer;
  }
  {
    unsigned long aEngineSpeed = lround(OBD2.pidRead(ENGINE_RPM));
    snprintf(aLineBuffer, 16, LineEngineSpeed, aEngineSpeed);
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
    snprintf(aLineBuffer, 16, LineAmbientAirTemp, aAmbientAirTemp);
    Line1 = aLineBuffer;
  }
  {
    long aIntakeAirTemp = lround(OBD2.pidRead(AIR_INTAKE_TEMPERATURE));
    snprintf(aLineBuffer, 16, LineIntakeAirTemp, aIntakeAirTemp);
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

void FormatRuntimeStats(String& Line1, String& Line2) {
  char aLineBuffer[17];
  {
    long aRuntimeSeconds = lround(OBD2.pidRead(RUN_TIME_SINCE_ENGINE_START));
    snprintf(aLineBuffer, 16, LineCurrentRuntimeSeconds, aRuntimeSeconds);
    Line1 = aLineBuffer;
  }
  Line2 = "";
}

void DoLogging() {
  if (gLoggingEnabled) {
    gLogfile = SD.open(LOGFILE_NAME, FILE_WRITE);
    if (gLogfile) {
      gLogfile.println("Testing ABC");
      gLogfile.close();
    }
  }
}

void InitSDCard() {
  if (SD.begin(SD_CS_PIN)) {
    gLoggingEnabled = true;
    Serial.println("SD init success");
  } else {
    Serial.println("SD init failed");
  }
  delay(1000);
}

void HandleButtonRead() {
  if ((millis() - gLastButtonPressMs) < kDebounceMs) {
    return;
  }
  Serial.println("Button press");
  switch (gButtonPinInput) {
    case A1:
      gpScreenMenu->DoButtonFour();
      gLastButtonPressMs = millis();
      break;
    case A2:
      gpScreenMenu->DoButtonThree();
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
  // pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  // Enable interrupts on analog pins (PCINT1)
  PCICR |= 0b00000010;
  PCMSK1 |= 0b00011100;
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
  pinMode(SD_CS_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial)
    ;
  LiquidCrystal aLcdScreen(LCD_Reset, LCD_Enable, LCD_D4, LCD_D5, LCD_D6,
                           LCD_D7);
  gpScreenMenu = new ScreenMenu(aLcdScreen);
  gpScreenMenu->ClearLcdScreen();
  InitSDCard();
  // OBDConnect();
  ConfigureScreenMenu();
  noInterrupts();
  ConfigureInterrupts();
  ConfigureTimer();
  interrupts();
}

void loop() {
  HandleButtonRead();

  if (gTimerTriggered) {
    gpScreenMenu->DoRefresh();
    gTimerTriggered = false;
    DoLogging();
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
