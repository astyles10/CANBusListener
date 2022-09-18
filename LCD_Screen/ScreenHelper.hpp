#include <LiquidCrystal.h>

#include <iterator>
#include <vector>

class ScreenMenu {
 public:
  typedef void (*PageCallback)(String&, String&);
  typedef struct Page {
    bool fUpdates;
    PageCallback fCallback;
    Page(bool inUpdates, PageCallback inCallback)
        : fUpdates(inUpdates), fCallback(inCallback) {}
    Page operator=(Page inOther) {
      return { inOther.fUpdates, inOther.fCallback };
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

  void MoveLeft() { fLcdScreen.scrollDisplayLeft(); }

  void MoveRight() { fLcdScreen.scrollDisplayRight(); }

  void RegisterPage(const Page inPage) { fPages.push_back(inPage); }

  void DisplayPage() {
    String aLine1, aLine2;
    fPages[fCurrentScreenNumber].fCallback(aLine1, aLine2);
    fLcdScreen.clear();
    fLcdScreen.setCursor(0, 0);
    fLcdScreen.write(aLine1.c_str());
    fLcdScreen.setCursor(0, 1);
    fLcdScreen.write(aLine2.c_str());
  }

  void UpdatePage() {
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
    UpdatePage();
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