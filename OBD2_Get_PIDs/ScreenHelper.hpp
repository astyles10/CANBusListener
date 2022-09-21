#include <LiquidCrystal.h>

#include <iterator>
#include <vector>

class ScreenMenu {
 public:
  typedef void (*RefreshCallback)(String&, String&);
  typedef void (*ButtonPressCallback)(void);
  typedef struct Page {
    bool fUpdates;
    RefreshCallback fRefreshCallback;
    ButtonPressCallback fButtonOneCallback;
    // ButtonPressCallback fButtonTwoCallback;
    Page() : fUpdates(false) {
      fRefreshCallback = NULL;
      fButtonOneCallback = NULL;
    }
    Page(bool inUpdates, RefreshCallback inCallback)
        : fUpdates(inUpdates), fRefreshCallback(inCallback) {
      fButtonOneCallback = NULL;
    }
    Page operator=(Page inOther) {
      return {inOther.fUpdates, inOther.fRefreshCallback};
    }
  } Page;

  ScreenMenu(LiquidCrystal& inLcdScreen)
      : fLcdScreen(inLcdScreen), fCurrentScreenNumber(0) {
    fLcdScreen.begin(16, 2);
  };
  ~ScreenMenu() = default;

  void MoveDown() {
    if (--fCurrentScreenNumber < 0) {
      fCurrentScreenNumber = fPages.size() - 1;
    }
    DisplayPage();
  }

  void MoveUp() {
    if (++fCurrentScreenNumber >= fPages.size()) {
      fCurrentScreenNumber = 0;
    }
    DisplayPage();
  }

  void DoButtonThree() {
    if (fPages[fCurrentScreenNumber].fButtonOneCallback) {
    }
  }

  void DoButtonFour() { fLcdScreen.scrollDisplayRight(); }

  void RegisterPage(const Page inPage) { fPages.push_back(inPage); }

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
    DoRefresh();
  }

  void WriteTemporaryPage(const String& inLine1, const String& inLine2) {
    ClearLcdScreen();
    fLcdScreen.write(inLine1.c_str());
    fLcdScreen.setCursor(0, 1);
    fLcdScreen.write(inLine2.c_str());
  }

 private:
  LiquidCrystal fLcdScreen;
  std::vector<Page> fPages;
  int8_t fCurrentScreenNumber;
};