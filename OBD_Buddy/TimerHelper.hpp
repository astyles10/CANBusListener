#include <Arduino.h>
#include <avr/io.h>

class TimerHelper {
 public:
  typedef struct Prescaler {
    uint16_t fValue;
    uint16_t fCSRegisters;
    Prescaler(const uint16_t inValue, const uint16_t inCSRegisters)
        : fValue(inValue), fCSRegisters(inCSRegisters){};
  } Prescaler;

  TimerHelper(const Prescaler& inPrescaler, uint16_t inTicks)
      : fPrescaler(inPrescaler), fTicks(inTicks){};
  ~TimerHelper() = default;

  static Prescaler DeterminePrescaler(const double inDesiredFrequencyHz) {
    if (inDesiredFrequencyHz <= 0.95) {
      return {1024, (1 << CS10 | 1 << CS12)};
    } else if (inDesiredFrequencyHz > 0.95 && inDesiredFrequencyHz <= 3.814) {
      return {256, (1 << CS12)};
    } else if (inDesiredFrequencyHz > 3.814 &&
               inDesiredFrequencyHz <= 30.5176) {
      return {64, (1 << CS10 | 1 << CS11)};
    } else if (inDesiredFrequencyHz > 30.5176 &&
               inDesiredFrequencyHz <= 244.14) {
      return {8, (1 << CS11)};
    }
    return {1, (1 << CS10)};
  }

  static TimerHelper DetermineTimerSettings(const double inFrequencyHz) {
    const Prescaler aPrescaler = DeterminePrescaler(inFrequencyHz);
    const unsigned short aTicks =
        (fClockFrequencyHz / (aPrescaler.fValue * inFrequencyHz));
    return TimerHelper(aPrescaler, aTicks);
  }

  const Prescaler& GetPrescaler(void) const { return fPrescaler; }

  const uint16_t GetTicks(void) const { return fTicks; }

 private:
  static constexpr uint32_t fClockFrequencyHz = 16000000;
  Prescaler fPrescaler;
  uint16_t fTicks;
};