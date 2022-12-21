#include <LiquidCrystal.h>
#include "LedControl.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <EEPROM.h>

// LED PINS
// const byte rs = 9;
// const byte en = 8;
// const byte d4 = 7;
// const byte d5 = 13; // 6
// const byte d6 = 5;
// const byte d7 = 4;
// const byte contrast = 3;
// const byte lcdBrightnessPin = 6;

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 13; // 6
const byte d6 = 1;
const byte d7 = 4;
const byte contrast = 5;
const byte lcdBrightnessPin = 6;

const int buzzerPin = 3;
int buzzerTone = 1000;

// MATRIX PINS
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;

// JOYSTICK PINS
const int pinSW = 2; // digital pin connected to switch output
const int pinX = A0; // A0 - analog pin connected to X output
const int pinY = A1; // A1 - analog pin connected t Y output

// BUTTON PARAMS
byte isButtonPressed = LOW;
byte lastButtonState;
const unsigned int debounceDelay = 50;
unsigned long lastDebounceTime;
unsigned long releaseTime;
unsigned long pressedTime;

// JOYSTICK PARAMS
int xValue;
int yValue;
int swValue;
bool joyMovedX = false;
bool joyMovedY = false;
bool enterSubMenu = false;
const int minThreshold = 400;
const int maxThreshold = 600;

// MENU PARAMS
byte currentFrame = 0;
byte currentFrameSubMenu = 0;
bool updateFrameSubMenu = false;
bool updateFrame = false;
bool mainMenu = true;
byte currentHighScore;
char name[4] = "___";
int letter;
int pos;
bool intro = true;
// bool intro = false;

struct highscore{
  int score = 0;
  char name[4] = "___";
};

// DATA TO/FROM EEPROM
struct data{
  int difficulty = 1;
  int lcdContrast = 120;
  int lcdBrightness = 255;
  int matrixBrightness = 3;
  bool sounds = true;
  highscore highScores[5];
} mySettings;

// MENU STRINGS
char gameName[] = "Snake";
char aboutText[] = "Made By: Oprea Ana-Maria    Github: ana-oprea       ";
char howToPlayText[] = "You control a snake, which roams around on a bordered plane, picking up food, trying to avoid hitting its own tail or the edges of the playing area. Each time the snake eats a piece of food, its tail grows longer.       ";
int positionInText = 0;

// GAME PARAMS
int foodRow, foodCol;
int snakeRow = 0;
int snakeCol = 4;
int lives = 1;
int score = 0;
bool gameStarted;
unsigned long blinkDuration = 500;
unsigned long lastBlinkTime;
bool shouldGenerateFood;

bool lightUpMatrix = false;

// INITIALIZING LED
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// INITIALIZING MATRIX
LedControl lc = LedControl(dinPin, clockPin, loadPin,1);//DIN, CLK, LOAD, No.DRIVER;

// SNAKE PARAMS
int snakeLength = 1;
short snakeDirection = 0;
short previousDirection = 0;
unsigned long timestamp;

// DIRECTIONS
const short up = 1;
const short right = 2;
const short down = 3; // 'down - 2' must be 'up'
const short left = 4; // 'left - 2' must be 'right'

// GAME MATRIX
int gameMatrix[8][8] = {
  {0, 0, 0, 0, 1, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

const uint64_t IMAGES[] = {
  0x0306ccf8f0783c1c,
  0x79407e027e407e00,
  0x180018183c3c1800,
  0x1800183860663c00,
  0x060e0c0808281800,
  0x105438ee38541000,
  0x6666006666666600,
  0xfe8282c66c381000,
  0x3c3c7effffff7e3c,
  0x003c7effffff7e3c,
  0x10387cfefeee4400
};
const int IMAGES_LEN = sizeof(IMAGES)/8;

void setup() {
  lcd.begin(16, 2);
  pinMode(pinSW, INPUT_PULLUP);
  lcd.setCursor(0,0);
  lcd.print(">");
  lcd.setCursor(1, 0);
  lcd.print("Start Game");
  lcd.setCursor(1, 1);
  lcd.print("Settings");
  readEeprom();

  analogWrite(contrast, mySettings.lcdContrast);
  analogWrite(lcdBrightnessPin, mySettings.lcdBrightness);

  lc.shutdown(0, false);// turn off power saving,enables display
  lc.setIntensity(0, mySettings.matrixBrightness);// sets brightness(0~15 possible values)
  lc.clearDisplay(0);// clear screen

  srand(time(0));
}

void loop() {
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  swValue = digitalRead(pinSW);

  readEeprom();
  // initializeEeprom();

  if (intro == true){
    displayImage(IMAGES[1]);
    if(swValue == LOW && lastButtonState == HIGH){
      lastDebounceTime = millis();
    }
    else if(swValue == HIGH && lastButtonState == LOW){
      releaseTime = millis();
      pressedTime = releaseTime - lastDebounceTime;

      if (pressedTime > debounceDelay){
        bool startMenu = true;
        for (int i = 0; i < 3; i++){
          if (name[i] == '_')
            startMenu = false;
        }
        if (startMenu == true){
          intro = false;
          introSequence();
        }
        }  
      }
    lastButtonState = swValue;

  lcd.clear();
  lcd.print("ENTER NAME");
  lcd.setCursor(2, 1);
  for (int i = 0; i < 3; i++){
    lcd.print(name[i]);
  }
  delay(25);
  enterName();
  } 
  else{
    buttonListener();

    if (mainMenu == true){
      snakeRow = 0;
      snakeCol = 4;
      lives = 1;
      score = 0;
      moveThroughFrames();
      if(updateFrame == true)
        showFrame();
    }else{
      moveThroughFrames();
      showSubDisplay();
    }
  }
}

void introSequence(){
  lcd.clear();
  lcd.print("Welcome to...");
  lcd.setCursor(0, 1);
  lcd.print("SNAKE");
  delay(2000);
  mainMenu = true;
  updateFrame = true;
}

void buttonListener(){
  if(swValue == LOW && lastButtonState == HIGH){
    lastDebounceTime = millis();
  }
  else if(swValue == HIGH && lastButtonState == LOW){
    releaseTime = millis();
    pressedTime = releaseTime - lastDebounceTime;
    if (pressedTime > debounceDelay){
      if (lives == 0){
        lives = 1;
        updateFrame = true; 
        mainMenu = true; 
        lcd.clear();
        showFrame();
      }
      else{
        enterSubMenu = true;
        updateFrameSubMenu = true;
        mainMenu = !mainMenu;
        lightUpMatrix = false;
        for(int i = 0; i<matrixSize; i++){
          for(int j=0; j<matrixSize; j++){
            lc.setLed(0, i, j, lightUpMatrix);
          }
        }
        lcd.clear();
        if (mainMenu == true){
          updateFrame == true;
          showFrame();
        }
      }  
    }}
  lastButtonState = swValue;
}

void startGame(){
  if (shouldGenerateFood == true){
    generateFood();
    shouldGenerateFood = false;
  }
  blinkFood();
  // drawSnake();
  moveSnake();
  checkCollision();
  snakeMatrixCalc();
}

void checkCollision(){
  if ((snakeRow == -1 || snakeRow == 8 || snakeCol == -1 || snakeCol == 8)){
    lives--;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game Over");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
    delay(1000);
    lc.setLed(0, foodRow, foodCol, false);
    lc.setLed(0, snakeRow, snakeCol, false);

    snakeDirection = 0;
    previousDirection = 0;
    snakeRow = 0;
    snakeCol = 4;

    for (int i = 0; i < 8; i++){
      for (int j = 0; j < 8 ; j++){
        gameMatrix[i][j] = 0;        
      }
    }
    snakeLength = 1;
    checkHighScore();
  }
}

void checkHighScore(){
  bool top = false;
  for (int i = 0; i < 5; i++){
    if (score > mySettings.highScores[i].score){
      for (int j = 5; j > i ; j--){
        mySettings.highScores[j].score = mySettings.highScores[j-1].score;
        strncpy(mySettings.highScores[j].name, mySettings.highScores[j-1].name, 3);
      }
      mySettings.highScores[i].score = score;
      strncpy(mySettings.highScores[i].name, name, 3);
      lcd.setCursor(0, 0);
      lcd.print("YOU'RE IN TOP 5");
      top = true;
      initializeEeprom();
      break;
    }
  }
  if (top == false){
    lcd.setCursor(0, 0);
    lcd.print("TRY AGAIN");
  }
}

void enterName(){
  xValue = analogRead(pinX);
  if(xValue < minThreshold && joyMovedX == false) {
    if(pos - 1 < 0)
      pos = 0;
    else{
      pos -= 1;
    }
    joyMovedX = true;
  }else{
    if(xValue > maxThreshold && joyMovedX == false) {
      if(pos + 1 > 3)
        pos = 3;
      else{
        pos += 1;
      }
      joyMovedX = true;
    }
  }

  if(xValue > minThreshold && xValue < maxThreshold){
    joyMovedX = false;
  }

  yValue = analogRead(pinY);
  if(yValue < minThreshold && joyMovedY == false) {
    if(letter - 1 < 0)
      letter = 0;
    else{
      letter -= 1;
    }
    joyMovedY = true;
  }else{
    if(yValue > maxThreshold && joyMovedY == false) {
      if(letter + 1 > 26)
        letter = 26;
      else{
        letter += 1;
      }
      joyMovedY = true;
    }}
  if(yValue > minThreshold && yValue < maxThreshold){
    joyMovedY = false;
  }

  char character;
  if(joyMovedY == true) {
    if(letter == 26) {
      character = ' ';
    }else{
      character = 'A' + letter;
    }
    name[pos] = character;
  }
}

void moveSnake(){
  xValue = analogRead(pinX);
  yValue = analogRead(pinY);
  swValue = digitalRead(pinSW);

  if (xValue > maxThreshold && joyMovedX == false){
    snakeDirection = right;
    joyMovedX = true;
  }
  else if (xValue < minThreshold && joyMovedX == false){
    snakeDirection = left;
    joyMovedX = true;
  }

  if(xValue > minThreshold && xValue < maxThreshold){
    joyMovedX = false;
  }

  if (yValue > maxThreshold && joyMovedY == false){
      snakeDirection = up;
    joyMovedY = true;
  }
  else if (yValue < minThreshold && joyMovedY == false){
      snakeDirection = down;
    joyMovedY = true;
  }

  if(yValue > minThreshold && yValue < maxThreshold){
    joyMovedY = false;
  }
}

void snakeMatrixCalc(){
  if (millis() - timestamp > 400){
    // draw the head in the direction the snake is moving
    switch (snakeDirection) {
      case up:
        snakeRow++;
        break;
      case right:
        snakeCol++;
        break;
      case down:
        snakeRow--;
        break;
      case left:
        snakeCol--;
        break;
      default: // if the snake is not moving, exit
        return;
    }

    checkPositionFood();
    drawSnake();
    timestamp = millis();
  }
}

void drawSnake(){
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      if (gameMatrix[i][j] > 0){
        gameMatrix[i][j]--;
        if (gameMatrix[i][j] > 0)         
          lc.setLed(0, i, j, true);
        else
          lc.setLed(0, i, j, false);
      }
      else{
        lc.setLed(0, i, j, false);
      }
    }
  }
  gameMatrix[snakeRow][snakeCol] = snakeLength;
  lc.setLed(0, snakeRow, snakeCol, true);
}

void checkPositionFood(){
  if (snakeRow == foodRow && snakeCol == foodCol){
    lc.setLed(0, foodRow, foodCol, false);
    score++;
    shouldGenerateFood = true;
    snakeLength++;
    tone(buzzerPin, 230, 50);

    for (int i = 0; i < 8; i++){
      for (int j = 0; j < 8; j++){
        if (gameMatrix[i][j] > 0)
          gameMatrix[i][j]++;
      }
    }
  }
}

void generateFood(){
  foodRow = rand() % 8;
  foodCol = rand() % 8;

  if (snakeLength == 64){
    lives--;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("You Win");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
    return;
  }
  while (gameMatrix[foodRow][foodCol] > 0){
    foodRow = rand() % 8;
    foodCol = rand() % 8;
  }
}

void blinkFood(){
  if (millis() - lastBlinkTime > blinkDuration) {
    lastBlinkTime = millis();
    lc.setLed(0, foodRow, foodCol, true); // turns on LED at foodCol, foodRow
  }
  else{
    lc.setLed(0, foodRow, foodCol, false);
  }  
}

void showSubDisplay(){
  switch(currentFrame){
    case 0:
      if (lives > 0){
        lcd.setCursor(0, 0);
        lcd.print("Lives: ");
        lcd.print(lives);
        lcd.setCursor(0, 1);
        lcd.print("Score: ");
        lcd.print(score);
        startGame();
      }
      break;
    case 1:
      showSettingsSubMenu();    
      break;
    case 2:
      displayImage(IMAGES[9]);
      showHighScore();
      break;
    case 3:
      displayImage(IMAGES[3]);
      showHowToPlay();
      break;
    case 4:
      // lcd.clear();
      displayImage(IMAGES[2]);
      showAboutInfo();
      break;
    
  }
}

void showHighScore(){
  lcd.setCursor(0, 0);
  lcd.print("HIGHSCORES");
  lcd.setCursor(0, 1);
  lcd.print(currentHighScore + 1);
  lcd.print(".");
  lcd.print(mySettings.highScores[currentHighScore].name);
  lcd.print(" ");
  lcd.print(mySettings.highScores[currentHighScore].score);

  yValue = analogRead(pinY);
  if (yValue > maxThreshold && joyMovedY == false){
      if(currentHighScore == 4)
        currentHighScore = 0;
      else
        currentHighScore++;
      joyMovedY = true;
      // updateFrame = true;
      lcd.clear();
    }
    else if (yValue < minThreshold && joyMovedY == false){
      if(currentHighScore == 0)
        currentHighScore = 4;
      else
        currentHighScore--;
      joyMovedY = true;
      // updateFrame = true;
      lcd.clear();
    }

    if(yValue > minThreshold && yValue < maxThreshold){
      joyMovedY = false;
    }
}

void showHowToPlay(){
  lcd.setCursor(0,0);
  lcd.print("How to Play");
  scrollingText(howToPlayText);
  delay(650); 
}

void scrollingText(char text[]){
  if (positionInText == strlen(text)){
    positionInText = 0;
  }
  lcd.setCursor(0, 1);
  // the last substring of the text that is 16 characters
  if (positionInText < strlen(text) - 16){
    for (int i = positionInText; i < positionInText + 16; i++){
      lcd.print(text[i]);
    }
  }
  // if we haven't reached the last 16 characters of the text
  else{
    for (int i = positionInText; i < strlen(text); i++){
      lcd.print(text[i]);
    }
  }
  positionInText++;
}

void moveThroughFrames(){
  if (mainMenu == true){
    if (yValue > maxThreshold && joyMovedY == false){
      if(currentFrame == 4)
        currentFrame = 0;
      else
        currentFrame++;
      joyMovedY = true;
      updateFrame = true;
      if (mySettings.sounds == true)
        tone(buzzerPin, buzzerTone, 50);
      // lcd.clear();
    }
    else if (yValue < minThreshold && joyMovedY == false){
      if(currentFrame == 0)
        currentFrame = 4;
      else
        currentFrame--;
      joyMovedY = true;
      updateFrame = true;
      if (mySettings.sounds == true)
        tone(buzzerPin, buzzerTone, 50);
      // lcd.clear();
    }

    if(yValue > minThreshold && yValue < maxThreshold){
      joyMovedY = false;
    }
  }
  else {
    if (yValue > maxThreshold && joyMovedY == false){
      if(currentFrameSubMenu == 4)
        currentFrameSubMenu = 0;
      else
        currentFrameSubMenu++;
      joyMovedY = true;
      updateFrameSubMenu = true;
      if (mySettings.sounds == true)
        tone(buzzerPin, buzzerTone, 50);
      // lcd.clear();
    }
    else if (yValue < minThreshold && joyMovedY == false){
      if(currentFrameSubMenu == 0)
        currentFrameSubMenu = 4;
      else
        currentFrameSubMenu--;
      joyMovedY = true;
      updateFrameSubMenu = true;
      if (mySettings.sounds == true)
        tone(buzzerPin, buzzerTone, 50);
      // lcd.clear();
    }

    if(yValue > minThreshold && yValue < maxThreshold){
      joyMovedY = false;
    }
  }
}

void showSettingsSubMenu(){
  // moveThroughFrames();
  switch(currentFrameSubMenu){
    case 0:
      if (updateFrameSubMenu == true){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(1, 0);
        lcd.print("Difficulty");

        lcd.setCursor(12, 0);
        if (mySettings.difficulty == 1){
          lcd.print("EASY");
        }
        else if (mySettings.difficulty == 2){
          lcd.print("MED");
        }
        else{
          lcd.print("HARD");
        }
        lcd.setCursor(1, 1);
        lcd.print("LCD Co");
        updateFrameSubMenu = false;
        lightUpMatrix = false;
        for(int i = 0; i<matrixSize; i++){
          for(int j=0; j<matrixSize; j++){
            lc.setLed(0, i, j, lightUpMatrix);
          }
        }
        displayImage(IMAGES[6]);

      }

      xValue = analogRead(pinX);
      
      if (xValue > maxThreshold && joyMovedX == false){
        if(mySettings.difficulty < 3){
          mySettings.difficulty++;
          initializeEeprom();
          if (mySettings.sounds == true)
            tone(buzzerPin, 466, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Difficulty");
        lcd.print(mySettings.difficulty);
      }
      else if (xValue < minThreshold && joyMovedX == false){
        if(mySettings.difficulty > 1){
          mySettings.difficulty--;
          initializeEeprom();
          if (mySettings.sounds == true)
            tone(buzzerPin, 450, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Difficulty");
        lcd.print(mySettings.difficulty);
      }

      if(xValue > minThreshold && xValue < maxThreshold){
        joyMovedX = false;
      }
      break;
    case 1:
      if (updateFrameSubMenu == true){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(1, 0);
        lcd.print("LCD Co");
        int val = map(mySettings.lcdContrast, 0, 140, 0, 100);
        lcd.setCursor(12, 0);
        lcd.print(val);
        lcd.print('%');
        lcd.setCursor(1, 1);
        lcd.print("LCD Bri");

        updateFrameSubMenu = false;
        lightUpMatrix = false;
        for(int i = 0; i<matrixSize; i++){
          for(int j=0; j<matrixSize; j++){
            lc.setLed(0, i, j, lightUpMatrix);
          }
        }
        displayImage(IMAGES[5]);
      }
      xValue = analogRead(pinX);
      
      if (xValue > maxThreshold && joyMovedX == false){
        if(mySettings.lcdContrast < 140){
          mySettings.lcdContrast += 20;
          initializeEeprom();
          if (mySettings.sounds == true)
            tone(buzzerPin, 466, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("LCD Co");
        lcd.print(mySettings.lcdContrast);
      }
      else if (xValue < minThreshold && joyMovedX == false){
        if(mySettings.lcdContrast > 0){
          mySettings.lcdContrast -= 20;
          initializeEeprom();
          if (mySettings.sounds == true)
            tone(buzzerPin, 450, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("LCD Co");
        lcd.print(mySettings.lcdContrast);
      }

      if(xValue > minThreshold && xValue < maxThreshold){
        joyMovedX = false;
      }
      analogWrite(contrast, mySettings.lcdContrast);
      
      break;
    case 2:
      if (updateFrameSubMenu == true){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(1, 0);
        lcd.print("LCD Bri");
        int val = map(mySettings.lcdBrightness, 0, 255, 0, 100);
        lcd.setCursor(12, 0);
        lcd.print(val);
        lcd.print('%');
        lcd.setCursor(1, 1);
        lcd.print("Matrix Bri");
        
        updateFrameSubMenu = false;
        lightUpMatrix = false;
        for(int i = 0; i<matrixSize; i++){
          for(int j=0; j<matrixSize; j++){
            lc.setLed(0, i, j, lightUpMatrix);
          }
        }
        displayImage(IMAGES[8]);
      }
        xValue = analogRead(pinX);
      
        if (xValue > maxThreshold && joyMovedX == false){
          if(mySettings.lcdBrightness < 255){
            mySettings.lcdBrightness += 25;
            initializeEeprom();
            if (mySettings.sounds == true)
              tone(buzzerPin, 466, 50);
          }
          joyMovedX = true;
          updateFrameSubMenu = true;
          // lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("LCD Bri");
          lcd.print(mySettings.lcdBrightness);
        }
        else if (xValue < minThreshold && joyMovedX == false){
          if(mySettings.lcdBrightness - 25 > 0){
            mySettings.lcdBrightness -= 25;
            initializeEeprom(); 
            if (mySettings.sounds == true)
              tone(buzzerPin, 450, 50);
          }
          joyMovedX = true;
          updateFrameSubMenu = true;
          // lcd.clear();
          lcd.setCursor(1, 0);
          lcd.print("LCD Bri");
          lcd.print(mySettings.lcdBrightness);
        }

        if(xValue > minThreshold && xValue < maxThreshold){
          joyMovedX = false;
        }
        analogWrite(lcdBrightnessPin, mySettings.lcdBrightness);
      
      break;
    case 3:
      if (updateFrameSubMenu == true){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(1, 0);
        lcd.print("Matrix Bri");
        int val = map(mySettings.matrixBrightness, 0, 15, 0, 100);
        lcd.setCursor(12, 0);
        lcd.print(val);
        lcd.print('%');
        lcd.setCursor(1, 1);
        lcd.print("Sounds");

        updateFrameSubMenu = false;
        lightUpMatrix = true;
        for(int i = 0; i<matrixSize; i++){
          for(int j=0; j<matrixSize; j++){
            lc.setLed(0, i, j, lightUpMatrix);
          }
        }
      }

      xValue = analogRead(pinX);
      
      if (xValue > maxThreshold && joyMovedX == false){
        if(mySettings.matrixBrightness + 3 <= 15){
          mySettings.matrixBrightness += 3;
          initializeEeprom();
          if (mySettings.sounds == true)
            tone(buzzerPin, 466, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Matrix Bri");
        lcd.print(mySettings.matrixBrightness);
      }
      else if (xValue < minThreshold && joyMovedX == false){
        if(mySettings.matrixBrightness - 3 > 0){
          mySettings.matrixBrightness -= 3;
          initializeEeprom();
          if (mySettings.sounds == true)
            tone(buzzerPin, 450, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Matrix Bri");
        lcd.print(mySettings.matrixBrightness);
      }

      if(xValue > minThreshold && xValue < maxThreshold){
        joyMovedX = false;
      }
      
      lc.setIntensity(0, mySettings.matrixBrightness);
      break;
    case 4:
      if (updateFrameSubMenu == true){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(">");
        lcd.setCursor(1, 0);
        lcd.print("Sounds");
        lcd.setCursor(13, 0);
        if(mySettings.sounds == true)
          lcd.print("ON");
        else
          lcd.print("OFF");
        lcd.setCursor(1, 1);
        lcd.print("Difficulty");

        updateFrameSubMenu = false;
        lightUpMatrix = false;
        for(int i = 0; i<matrixSize; i++){
          for(int j=0; j<matrixSize; j++){
            lc.setLed(0, i, j, lightUpMatrix);
          }
        }
        displayImage(IMAGES[4]);
      }

      xValue = analogRead(pinX);
      
      if (xValue > maxThreshold && joyMovedX == false){
        if(mySettings.sounds == false){
          mySettings.sounds = true;
          initializeEeprom();
          tone(buzzerPin, 466, 50);
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Sounds");
        if(mySettings.sounds == true)
          lcd.print("ON");
        else
          lcd.print("OFF");
      }
      else if (xValue < minThreshold && joyMovedX == false){
        if(mySettings.sounds == true){
          mySettings.sounds = false;
          initializeEeprom();
        }
        joyMovedX = true;
        updateFrameSubMenu = true;
        // lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Sounds");
        if(mySettings.sounds == true)
          lcd.print("ON");
        else
          lcd.print("OFF");
      }

      if(xValue > minThreshold && xValue < maxThreshold){
        joyMovedX = false;
      }
      
      break;
  }
}

void showAboutInfo(){
  lcd.setCursor(0,0);
  lcd.print("About");
  scrollingText(aboutText);
  delay(650); 
}

void showFrame(){
  switch(currentFrame){
    case 0:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("Start Game");
      lcd.setCursor(1, 1);
      lcd.print("Settings");
      displayImage(IMAGES[10]);
      updateFrame = false;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("Settings");
      lcd.setCursor(1, 1);
      lcd.print("High Score");
      displayImage(IMAGES[0]);
      updateFrame = false;
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("High Score");
      lcd.setCursor(1, 1);
      lcd.print("How to Play");
      displayImage(IMAGES[9]);
      updateFrame = false;
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("How To Play");
      lcd.setCursor(1, 1);
      lcd.print("About");
      displayImage(IMAGES[3]);
      updateFrame = false;
      break;
    case 4:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("About");
      lcd.setCursor(1, 1);
      lcd.print("Start Game");
      updateFrame = false;
      displayImage(IMAGES[2]);
      break;
  }
}

void initializeEeprom() {
  EEPROM.put(0, mySettings);
}

void readEeprom() {
  EEPROM.get(0, mySettings);
}

void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}