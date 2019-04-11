#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void initDisplay() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();
}

String padLeft(String original) {
  int len = original.length();
  String returnTxt = "                ";
  returnTxt = returnTxt + original;
  return returnTxt.substring(returnTxt.length() - 16);

}
int counterDisplayOption = 0;
void displayIdle() {

  if (counterDisplayOption == 0) {
    displayMessage("Finn", " BankingofThings");
    counterDisplayOption = 1;
    return;
  }
  if (counterDisplayOption == 1) {
    displayMessage("pay per touch", "   touch my wire");
    counterDisplayOption = 2;
    return;
  }
  if (counterDisplayOption == 2) {
    displayMessage("pay per action", " press my button");
    counterDisplayOption = 3;
    return;
  }
    if (counterDisplayOption == 3) {
    displayMessage("I pay per use", "       0.02e/min");
    counterDisplayOption = 0;
    return;
  }

}

void displayMessage(String row1, String row2) {
  lcd.clear();
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  lcd.print(row1);
  lcd.setCursor(0, 1);
  lcd.print(row2);

}
