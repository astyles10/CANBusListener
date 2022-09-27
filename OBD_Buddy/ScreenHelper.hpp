#include <LiquidCrystal.h>

class ScreenMenu {
 public:
  typedef void (*RefreshCallback)(String&, String&);
  typedef void (*ButtonPressCallback)(void);
  typedef struct Page {
    bool fUpdates;
    RefreshCallback fRefreshCallback;
    ButtonPressCallback fButtonCallback;
    Page() : fUpdates(false) {
      fRefreshCallback = NULL;
      fButtonCallback = NULL;
    }
    Page(bool inUpdates, RefreshCallback inCallback)
        : fUpdates(inUpdates), fRefreshCallback(inCallback) {
      fButtonCallback = NULL;
    }
    Page(bool inUpdates, RefreshCallback inCallback, ButtonPressCallback inButtonPressCallback) :
      fUpdates(inUpdates), fRefreshCallback(inCallback), fButtonCallback(inButtonPressCallback) {}
    Page& operator=(Page& inOther) = default;
  } Page;

  ScreenMenu(LiquidCrystal& inLcdScreen)
      : fLcdScreen(inLcdScreen), fNumberOfPages(0), fCurrentScreenNumber(0) {
    fLcdScreen.begin(16, 2);
  };
  ScreenMenu() = default;
  ~ScreenMenu() = default;

  ScreenMenu& operator=(ScreenMenu& inMenu) = default;

  void DoButtonTwo() {
    if (--fCurrentScreenNumber < 0) {
      fCurrentScreenNumber = fNumberOfPages - 1;
    }
    DisplayPage();
  }

  void DoButtonOne() {
    if (++fCurrentScreenNumber >= fNumberOfPages) {
      fCurrentScreenNumber = 0;
    }
    DisplayPage();
  }

  void DoButtonThree() {
    if (fPages[fCurrentScreenNumber].fButtonCallback) {
      fPages[fCurrentScreenNumber].fButtonCallback();
    }
  }

  void DoButtonFour() { fLcdScreen.scrollDisplayRight(); }

  void RegisterPage(Page&& inPage) {
    fPages[fNumberOfPages++] = inPage;
  }

  void DisplayPage() {
    String aLine1, aLine2;
    fPages[fCurrentScreenNumber].fRefreshCallback(aLine1, aLine2);
    fLcdScreen.clear();
    fLcdScreen.setCursor(0, 0);
    fLcdScreen.write(aLine1.c_str());
    fLcdScreen.setCursor(0, 1);
    fLcdScreen.write(aLine2.c_str());
  }

  void DoRefresh() {
    if (fPages[fCurrentScreenNumber].fUpdates) {
      DisplayPage();
    }
  }

  void ClearLcdScreen() {
    fLcdScreen.clear();
    fLcdScreen.setCursor(0, 0);
  }

  void SetDefaultScreen() {
    fCurrentScreenNumber = 0;
    DisplayPage();
  }

  void WriteTemporaryPage(const String& inLine1, const String& inLine2) {
    ClearLcdScreen();
    fLcdScreen.write(inLine1.c_str());
    fLcdScreen.setCursor(0, 1);
    fLcdScreen.write(inLine2.c_str());
  }

 private:
  static constexpr uint8_t fMaxPages = 5;
  LiquidCrystal& fLcdScreen;
  Page fPages[fMaxPages];
  uint8_t fNumberOfPages;
  int8_t fCurrentScreenNumber;
};