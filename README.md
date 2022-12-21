# Matrix-Game
## Matrix Game
### Components:
* LCD
* 8x8 LED matrix
* joystick
* resistors
* wires
* buzzer
* 100k ohm resistor
* 100 ohm resistor

### Technical Task
Create a menu for your game, emphasis on the game. You should scroll on the LCD with the joystick. Remember you have quite a lot of flexibility here, but do not confuse that with a free ticket to slack off. The menu should include the following functionality:

1. When powering up a game, a greeting message should be shown for a few moments.
2. Should contain roughly the following categories:
  a) Start game, starts the initial level of your game
  b) Highscore:
    * Initially, we have 0.
    * Update it when the game is done. Highest possible score should be achieved by starting at a higher level.
    * Save the top 5+ values in EEPROM with name and score.
  c) Settings:
    * Enter name. The name should be shown in highscore. Maybe somewhere else, as well?
    * Difficulty
    * LCD contrast control
    * LCD brightness control (mandatory, must change LED wire that’s directly connected to 5v)
    * Matrix brightness control (see function setIntesnity from the ledControl library)
    * Sounds on or off
    * Extra stuff can include items specific to the game mechanics, or other settings such as chosen theme song etc. Again, save it to eeprom.
  d) About: should include details about the creator(s) of the game. At least game name, author and github link or user (use scrolling text?)
  e) How to play: short and informative description
3. While playing the game: display all relevant info
  * Lives
  * Score
4. Upon game ending:
  * Screen 1: a message such as ”Congratulations on reaching level/score X”. ”You did better than y people.”. etc. Switches to screen 2 upon interaction (button press) or after a few moments.
  * Screen 2: display relevant game info: score, time, lives left etc. Must inform player if he/she beat the highscore. This menu should only be closed by the player, pressing a button.

### Implementation Details
When you power the project, you're prompted to enter your name (3 letters), move the joystick up and down to change letters, once you have all 3 letters, press the joystick to confirm. After that, a short message with "Welcome to snake" is gonna show up, then the main menu opens.

The main menu has 5 options:
* Start game: play the game
* Settings: make changes to LCD contrast/brightness, matrix brightness, difficulty, sounds (ON/OFF)
* High scores: see the top 5 players and their score
* How to play: a brief description of the game and how to play it
* About: information about the developer

## Navigation
* Moving through the menu is done by moving the joystick up and down to cycle the menu, to view an option, press the joystick. 
* If you are in a sub-menu (Start game, Settings, High scores, How to play, About) and want to exit, press the joystick. 
* If you are in Settings and want to change a value, move to that sub-section (LCD contrast/brightness, matrix brightness, difficulty, sounds) and an arrow should be pointing to the option you want to change, from there just move the joystick left and right to change the value, any change you make is going to be saved. If you want to exit, press the joystick and you are in the main manu again.
* If you are in game and want to exit, press the button and that will take you to the main menu.
* If you are at the end of the game (you lost) a brief "Game over" and your score will be shown on the lcd, after a short period you are met with the message "Try again" if you didn't make top 5, or with "You made top 5" if you managed your way on to the high score board. Press the button to return to the main menu.

### Picture of the setup
![46282895-7cbe-4830-bdf6-6f28d0b9ae2c](https://user-images.githubusercontent.com/80201759/208938267-9c9d5e91-2539-4902-8b23-efe7afa9806d.jpeg)

### Video
https://www.youtube.com/watch?v=FRJVdvrxxxs
