// Downloaded CAN and OBD2 libraries from Arduino library manager
#include <CAN.h>
// #include <MemoryUsage.h>
#include <OBD2.h>
#include <SD.h>
#include <SPI.h>

#include "OBDDefs.hpp"
#include "ScreenHelperNoVector.hpp"
#include "TimerHelper.hpp"

static constexpr uint8_t LCD_RegisterSelect = 2;
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
static constexpr double kTimerFrequency = 2;
static uint32_t gLastButtonPressMs = 0;
volatile static uint8_t gButtonPinInput = 0;
volatile static bool gTimerTriggered = false;

// Rename "ScreenMenu" to something more reflective of the actual class purpose
LiquidCrystal gLcdScreen(LCD_RegisterSelect, LCD_Enable, LCD_D4, LCD_D5, LCD_D6,
                         LCD_D7);
ScreenMenu gScreenMenu(gLcdScreen);

bool gLoggingEnabled = false;
File gLogfile;

/* Peripheral Initialisation Functions */

void InitScreen() {
  gScreenMenu.ClearLcdScreen();
  gScreenMenu.RegisterPage({true, FormatSpeedPage});
  gScreenMenu.RegisterPage({true, FormatAirTemperaturePage});
  // gpScreenMenu.RegisterPage({true, FormatFuelInfoPage});
  gScreenMenu.RegisterPage({true, FormatRuntimeStats});
  // gpScreenMenu.RegisterPage({false, FormatDebugPage});
  // gpScreenMenu.RegisterPage({false, FormatLoggingPage, HandleLoadLogFile});
}

void InitOBD() {
  gScreenMenu.WriteTemporaryPage("Connecting", "");
  while (true) {
    if (!OBD2.begin()) {
      delay(1000);
    } else {
      gScreenMenu.ClearLcdScreen();
      gScreenMenu.SetDefaultScreen();
      break;
    }
  }
}

void InitSDCard() {
  pinMode(SD_CS_PIN, OUTPUT);
  if (SD.begin(SD_CS_PIN)) {
    gLoggingEnabled = true;
  } else {
    gScreenMenu.WriteTemporaryPage("SD Card", "Init Fail");
  }
  delay(1000);
}

/* Peripheral Function Handlers */

void HandleLogging() {
  if (gLoggingEnabled) {
    gLogfile = SD.open(LOGFILE_NAME, FILE_WRITE);
    if (gLogfile) {
      char aDataLog[9];
      {
        snprintf(aDataLog, 8, "%u,",
                 lround(OBD2.pidRead(RUN_TIME_SINCE_ENGINE_START)));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%u,",
                 lround(OBD2.pidRead(VEHICLE_SPEED)));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%u,", OBD2.pidRead(ENGINE_RPM));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%u,", OBD2.pidRead(CALCULATED_ENGINE_LOAD));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%u,", OBD2.pidRead(TIMING_ADVANCE));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%d,", OBD2.pidRead(AIR_INTAKE_TEMPERATURE));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%d,", OBD2.pidRead(AMBIENT_AIR_TEMPERATURE));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%u,", OBD2.pidRead(THROTTLE_POSITION));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%u,", OBD2.pidRead(INTAKE_MANIFOLD_ABSOLUTE_PRESSURE));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%d,", OBD2.pidRead(SHORT_TERM_FUEL_TRIM_BANK_1));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%d,", OBD2.pidRead(LONG_TERM_FUEL_TRIM_BANK_1));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%d,", OBD2.pidRead(OXYGEN_SENSOR_1_SHORT_TERM_FUEL_TRIM));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        snprintf(aDataLog, 8, "%d,", OBD2.pidRead(OXYGEN_SENSOR_2_SHORT_TERM_FUEL_TRIM));
        gLogfile.print(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      {
        // Last log must be end the line
        snprintf(aDataLog, 8, "%u", OBD2.pidRead(TIME_SINCE_TROUBLE_CODES_CLEARED));
        gLogfile.println(aDataLog);
        memset(aDataLog, 0, sizeof(aDataLog));
      }
      gLogfile.close();
    }
  }
}

void HandleButtonRead() {
  if ((millis() - gLastButtonPressMs) < kDebounceMs) {
    return;
  }
  switch (gButtonPinInput) {
    case A1:
      gScreenMenu.DoButtonFour();
      gLastButtonPressMs = millis();
      break;
    case A2:
      gScreenMenu.DoButtonThree();
      gLastButtonPressMs = millis();
      break;
    case A3:
      gScreenMenu.MoveDown();
      gLastButtonPressMs = millis();
      break;
    case A4:
      gScreenMenu.MoveUp();
      gLastButtonPressMs = millis();
      break;
  }
  if (gButtonPinInput) {
    gButtonPinInput = 0;
  }
}

void InitInterrupts() {
  // pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  // Enable interrupts on analog pins 3, 4 and 5 (PCINT1)
  PCICR |= 0b00000010;
  PCMSK1 |= 0b00011100;
}

void InitTimer() {
  TimerHelper aTimerSettings = TimerHelper::DetermineTimerSettings(kTimerFrequency);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = aTimerSettings.GetTicks();
  TCCR1B |= aTimerSettings.GetPrescaler().fCSRegisters;
  TCCR1B |= (1 << WGM12);
  TIMSK1 |= (1 << OCIE1A);
}

/* Arduino Functions */

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  InitScreen();
  InitSDCard();
  InitOBD();
  noInterrupts();
  InitInterrupts();
  InitTimer();
  interrupts();
}

void loop() {
  HandleButtonRead();

  if (gTimerTriggered) {
    gScreenMenu.DoRefresh();
    gTimerTriggered = false;
    HandleLogging();
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

/* Page Callback Functions */

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
