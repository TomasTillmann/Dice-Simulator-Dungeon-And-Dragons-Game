#include "funshield.h"


class Button{
  public:
    int pin;

    /* returns true if button is pressed - false otherwise
     * optional arguments: 
          1st - (int) 0 default - specify for how long must the button be held 
          2nd - (bool) false default - if true - returns true until the button is released
    */
    bool IsPressed(int interval = 0, bool measureAfterStillPressed = false){
      return internalButtonDelay(interval, measureAfterStillPressed);
    }

  Button(int pin_){
    pin = pin_;
  }
  
  private:
    unsigned long lastTime = 0;
    bool lastState = false;                
    bool isMeasured = false;               

    bool buttonPressed(){
      return digitalRead(pin) ? false : true;
    }

    bool internalButtonDelay(int interval, bool measureAfterStillPressed = false){
      bool curState = buttonPressed();
      bool isPressed = curState;                           // to better understand the code
      if (curState != lastState){                          // has the state of the button changed?
        if (isPressed){
          lastTime = millis();                              // if yes and the button is pressed, starts measuring from the current time
          isMeasured = true;
        }
        else{
          isMeasured = false;                               // if the state has changed but the button is not pressed -> from pressed to unpressed -> stops measuring
        }
        lastState = curState;                              // updates the state of the button
      }
      if (isMeasured){                                      // button is measured only if it's pressed
        if (isPressed){
          if (millis() >= (lastTime + interval)){
            if (measureAfterStillPressed){
              lastTime = millis();                          // doesn't change is_measured to false, changes last time to current time -> resets the button -> function returns true every time the interval elapses
            }
            else{
              isMeasured = false;                           // after the interval elapses, is_measured = false -> returns true only once
            }
            return true;
          }
        }
      }
      return false;
  }
};
Button button[]{button1_pin, button2_pin, button3_pin}; 
constexpr int buttonCount = sizeof(button)/sizeof(button[0]);


class Display
{
public:
  /* 
   * writes the specified character on given position (0 - 3) - on the seg display from left to right
  */
  void ShowChar(const char* ch, int pos){
    byte g = getGlyph(*ch);
    glyphsToShow[pos] = g;
  }

  /*
   * writes the specified digit (0-9) on a given position (0 - 3)  
  */
  void ShowNum(int n, int pos){
    byte g = getGlyph(n);
    glyphsToShow[pos] = g;
  }

  /*
   * writes the given text - max size 4 - on the seg display from left to right 
  */
  void ShowText(const char *text){
    byte g;
    int pos = displayCount - strlen(text);
    while (*text)
    {
      g = getGlyph(*text++);
      glyphsToShow[pos++] = g;
    }
  }

  /*
   * writes the given number - max size 4 - on the seg display from left to right
  */
  void ShowNumber (int showNumber){
      constexpr int glyph_number[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

      for (int i = displayCount - 1; 0 <= i; --i){
        int cifra = getDigit(showNumber, i);
        glyphsToShow[(displayCount - 1) - i] = glyph_number[cifra];
      }
      removeLeadingZeroes();
    }

  /*
   * cleans the seg display - writes spaces to all positions
  */
  void Reset(){
    for (int i = 0; i < displayCount; i++){
      glyphsToShow[i] = noGlyph;
    }
  }

  /*
   * writes on seg display - multiplexing
  */
  void Loop(){
    writeGlyph(glyphsToShow[(displayCount - 1) - posDigit], posDigit); 
    posDigit = (posDigit + 1) % displayCount;
  }

  /*
   * initializes the display to function - has to be called in setup()!
  */
  void Initialize(){
    pinMode(latch_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
  }

  Display() {}

private:
  const int displayCount = 4;
  // used in loop() - multiplexing 
  int posDigit = 0;

  // special characters
  const int zero = 0xC0; const int dot = 0x7F; const int noGlyph = 0xFF;

  // what is written to this array is shown on the segment display from left to right - 0th position is the leftist one
  int glyphsToShow[4] = {noGlyph, noGlyph, noGlyph, noGlyph};               

  /*
   * gets the bit representation - glyph - of a given character - can also be a char number (e.g. "2")
  */ 
  byte getGlyph(char ch){
    constexpr byte emptyGlyph = 0b11111111;
    constexpr byte glyphNumber[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
    constexpr byte letterGlyph[] {
          0b10001000,   // A
          0b10000011,   // b
          0b11000110,   // C
          0b10100001,   // d
          0b10000110,   // E
          0b10001110,   // F
          0b10000010,   // G
          0b10001001,   // H
          0b11111001,   // I
          0b11100001,   // J
          0b10000101,   // K
          0b11000111,   // L
          0b11001000,   // M
          0b10101011,   // n
          0b10100011,   // o
          0b10001100,   // P
          0b10011000,   // q
          0b10101111,   // r
          0b10010010,   // S
          0b10000111,   // t
          0b11000001,   // U
          0b11100011,   // v
          0b10000001,   // W
          0b10110110,   // ksi
          0b10010001,   // Y
          0b10100100,   // Z
        };

    byte glyph = emptyGlyph;
    // if is a letter
    if (isAlpha(ch)){
      glyph = letterGlyph[ch - (isUpperCase(ch) ? 'A' : 'a')];
    }
    // if is a number
    else if (isdigit(ch)){
      glyph = glyphNumber[ch - '0'];
    }
    return glyph;
  }

  /*
   * gets the bit representation - glyph - of a given number
  */
  byte getGlyph(int n){
    constexpr byte glyphNumber[10] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
    if (0 <= n && n <=9) return glyphNumber[n];
    return 0x00;          //simulate undefiend behaviour
  }

  /*
   * low level function - writes the given glyph on a given position on the display
  */
  void writeGlyph(byte glyphMask, byte position){
    constexpr int bitmask_position[] = {1 << 3, 1 << 2, 1 << 1, 1 << 0};
    digitalWrite(latch_pin, LOW);
    shiftOut(data_pin, clock_pin, MSBFIRST, glyphMask);
    shiftOut(data_pin, clock_pin, MSBFIRST, bitmask_position[position]);
    digitalWrite(latch_pin, HIGH);
  }
  
  /*
   * helper function to getDigit - returns the order a number for a given position - counting from 0 to 3
  */
  int getOrder(int position){
      int increment;
      switch (position){
        case 0: increment = 1; break; case 1: increment = 10; break; case 2: increment = 100; break; case 3: increment = 1000; break;
      }
      return increment;
    }

    /*
     * returns a number on the given position in the given number
    */
    int getDigit(int n, int digitPos){
      int order = getOrder(digitPos);
      int digit = (n / order) % 10;
      return digit;
    }

  /*
   * removes (writes space) all unnecessary leading zeroes on the seg display
  */
  void removeLeadingZeroes(){
      for (int i = 0; i < displayCount - 1; ++i) if (glyphsToShow[i] == 0xC0) glyphsToShow[i] = noGlyph; else break;
    }

};
Display display;


class DiceGenerator{
  public:
    DiceGenerator(){
      Initialize();
    };

    /*
     * activates the dice generator on arduino
    */
    void Run(){
      inStateControl();
    }

  private:
    Button button1 = button[0]; Button button2 = button[1]; Button button3 = button[2];
    enum state {normalMode, configMode}; enum state currState; 
    const int diceTypeCount = 7; int diceType[7] = {4,6,8,10,12,20,100}; int currDiceType;
    int randomNumber;
    int currNumberOfThrows; 

    /*
     * returns a pseudo-random number uniformly distributed
     * follows binomial distribution
    */
    void generateRandomNumber(){
      randomNumber = 0;
      // summing it in order to simulate binomial disrtibution
      for (int i = 0; i < currNumberOfThrows; i++){
        int diceThrow = random(1, diceType[currDiceType] + 1);      // random() generates uniform numbers
        randomNumber += diceThrow;
      }
    }

    void updateNumberOfThrows(){
      currNumberOfThrows == 9 ? currNumberOfThrows = 1 : currNumberOfThrows += 1;
      display.ShowNum(currNumberOfThrows, 0);
    }

    void updateDiceType(){
      currDiceType = (currDiceType + 1) % diceTypeCount;
    }

    void updateTextConfigMode(){
      // shows the current dice type
      display.ShowNumber(diceType[currDiceType]);

      if (diceType[currDiceType] < 10){
        // adds one leading zero on the second position from left (counting from 0)
        display.ShowChar("0", 2);
        // if dice type has more than one digit doesn't have to write the leading zero
      }

      display.ShowChar("d", 1);
      // shows the user given current number of throws on the display
      display.ShowNum(currNumberOfThrows, 0);
    }

    void updateTextNormalMode(){
      // simple just shows the randomly generated number on the display
      display.ShowNumber(randomNumber);
    }

    /*
     * main function of diceGenerator - manages the states and flow of the program
    */
    void inStateControl(){
      // stabilization is set to 20ms to make sure the buttons wouldn't react spontaniously, if user holds it for a little longer
      int diceGenerationDelay = 50; int stabilization = 20;
      switch(currState){
        case normalMode:
          // if the first button is pressed for 50ms - calls this function every 50ms - stop when the button is released
          if(button1.IsPressed(diceGenerationDelay, true)){
            generateRandomNumber();
          }
          if(button2.IsPressed() || button3.IsPressed()){
            currState = configMode;
            // in order to make a cleaner transition - deletes everything on the screen and after that writes the newly given information
            display.Reset();
            return;
          }
          updateTextNormalMode();
          break;

        case configMode:
          if (button1.IsPressed(diceGenerationDelay, true)){
            currState = normalMode;
            display.Reset();
            return;
          }
          if (button2.IsPressed(stabilization)){
            updateNumberOfThrows();
          }
          if (button3.IsPressed(stabilization)){
            updateDiceType();
          }
          updateTextConfigMode();
          break;
      }
    }

    void Initialize(){
      currDiceType = 0;
      currNumberOfThrows = 1;

      // makes more sense to me to be in a cofing mode first than to be in a normal mode
      currState = configMode;
      randomNumber = 0;
    }
};
DiceGenerator diceGenerator;


void setup() {
  // gets a pretty random seed from an unconnected pin's analog signal 
  randomSeed(analogRead(led1_pin));

  // initializes buttons
  for (int i = 0; i < buttonCount; ++i){
    pinMode(button[i].pin, INPUT);
  }

  display.Initialize();
}

void loop() {
  diceGenerator.Run();
  display.Loop();
}