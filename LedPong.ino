#include <Modulino.h>
#include <Scheduler.h>
#include <Arduino_LED_Matrix.h>
#include <BleValueSync.h>
#include "crash.h"
#include "pong_start.h"
#include "winner.h"

//#define DEBUG 1

/* Features */
#define BUZZER 1
#define ANIMATION 1
#define LEDS 1
//#define BACKGROUND_LEDS 1
#define BLE_SYNC 1

/* Defines */
#define SECONDARY_ENCODER_ADDRESS 0x08
#define SECONDARY_PIXELS_ADDRESS 0x22
#define WIDTH 12
#define HEIGHT 8
#define START_Y 3
#define BAR_WIDTH 3
#define BALL_SPEED 120
#define LEFT_WIN 8
#define RIGHT_WIN 0
#define MATCH_TO_WIN 5
#define LED_BRI 8
#define WINNER_ANIM_LEFT 0
#define WINNER_ANIM_RIGHT 2

/* Modes */
#define DIR_TOP -1
#define DIR_BOTTOM 1
#define DIR_BALANCE 0
#define DIR_LEFT -1
#define DIR_RIGHT 1

/* Objects */
ArduinoLEDMatrix matrix;
ModulinoKnob sxEncoder, dxEncoder(SECONDARY_ENCODER_ADDRESS);
ModulinoBuzzer buzzer;
ModulinoPixels leds, ledsBackground(SECONDARY_PIXELS_ADDRESS);

/* Colors */
ModulinoColor leftColor(0xff, 0x00, 0x00), rightColor(0x0e, 0x1d, 0xf0);

/*Players positions*/
int8_t psx_x = 0, psx_y = START_Y, pdx_x = WIDTH-1, pdx_y = START_Y;

/*Ball positions and directions */
int8_t ball_x = WIDTH/2, ball_y = HEIGHT/2, dir_x = DIR_LEFT, dir_y = DIR_BALANCE;

/* Status variables */
int8_t game_ongoing = 0, winner = -1, muted = 1;

/* Match Variables */
int8_t win_left=0, win_right=0;

byte frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

#ifdef BLE_SYNC
/* BT Service and properties */
BleSync ble("LedPong", "76095e25-2eda-43c3-9289-f699b4800d2e", 3);
BleSyncValue bleCounter("25bf4001-6fb0-4e91-8ee2-8b218c1930db", BLERead | BLEWrite);
BleSyncValue bleLeftWinner("e6a264b5-32dc-456e-aeb1-07e63189ec7b", BLERead | BLEWrite);
BleSyncValue bleRightWinner("12bd8f99-2d1f-458f-baf0-47ed72c0a1c8", BLERead | BLEWrite);
#endif

void setup() {
  // put your setup code here, to run once:

  #ifdef DEBUG
  Serial.begin(115200);
  #endif

  matrix.begin();
  Modulino.begin();
  dxEncoder.begin();
  dxEncoder.set(START_Y);
  sxEncoder.begin();
  sxEncoder.set(START_Y);

  #ifdef BUZZER
  buzzer.begin();
  pinMode(12, INPUT_PULLUP);
  digitalWrite(12, HIGH);
  #endif

  #ifdef LEDS
  leds.begin();
  for(uint8_t i = 0; i<8; i++){     
    leds.set(i, 0, 0, 0, LED_BRI);
  }
  leds.show();
  #endif

  #ifdef BACKGROUND_LEDS
  randomSeed(analogRead(0));
  ledsBackground.begin();
  Scheduler.startLoop(flickerBackground);
  ledsBackground.show();
  #endif

  #ifdef ANIMATION
  for(auto f:pong_start){
    matrix.loadFrame(f);
    delay(90);
  }
  #endif

  /*Ball start position */
  frame[ball_y][ball_x] = 1;

  Scheduler.startLoop(playerDXLoop);
  Scheduler.startLoop(playerSXLoop);

  #ifdef BLE_SYNC
  ble.addValue(&bleCounter);
  ble.addValue(&bleLeftWinner);
  ble.addValue(&bleRightWinner);
  ble.initBLE();
  #endif

  game_ongoing = 1;

  #ifdef DEBUG
  Serial.println("Setup Completed");
  #endif

}


void loop() {

  
  if(game_ongoing){

    #ifdef DEBUG
    Serial.println("Game Ongoing");
    #endif

    // Clear old ball position
    frame[ball_y][ball_x] = 0;

    // Move the ball
    ball_y += dir_y;
    ball_x += dir_x;

    // Check borders
    if(ball_x < 0){ball_x = 0;}
    else if(ball_x > WIDTH -1){ball_x = WIDTH - 1;}

    if(ball_y < 0){ball_y = 0;}
    else if(ball_y > HEIGHT-1){ball_y = HEIGHT-1;}

    #ifdef DEBUG
    Serial.print("Rendering Bitmap with ball position (x,y): ");
    Serial.print(ball_x);
    Serial.print(" , ");
    Serial.println(ball_y);
    #endif

    // Draw matrix
    frame[ball_y][ball_x] = 1;
    matrix.renderBitmap(frame, 8, 12);

    // Check Crashes
    if(ball_x == 0 || (ball_x == 1 && dir_x == DIR_LEFT)){
      //Check if gol or change dir

      #ifdef DEBUG
      Serial.println("[LEFT] Checking if gol or change dir");
      #endif

      if(ball_y >= psx_y && ball_y <= psx_y + BAR_WIDTH -1){

        #ifdef DEBUG
        Serial.println("[LEFT] Player Touched - Changing direction");
        #endif

        dir_x = -dir_x;

        if(ball_y == psx_y){
          dir_y = DIR_BOTTOM;
        } else if(ball_y == psx_y + BAR_WIDTH -1){
          dir_y = DIR_TOP;
        } else {
          dir_y = DIR_BALANCE;
        }

        playSound();

      } else if(ball_x == 0){

        #ifdef DEBUG
        Serial.println("[LEFT] GOL");
        #endif

        game_ongoing = 0;
        winner = RIGHT_WIN;
        resetVariables();
      }

    } else if (ball_x == WIDTH -1 || (ball_x == WIDTH -2 && dir_x == DIR_RIGHT)){
      //Check if gol or change dir

      #ifdef DEBUG
      Serial.println("[RIGHT] Checking if gol or change dir");
      #endif

      if(ball_y >= pdx_y && ball_y <= pdx_y + BAR_WIDTH -1){
        
        #ifdef DEBUG
        Serial.println("[RIGHT] Player Touched - Changing direction");
        #endif

        dir_x = -dir_x;

        if(ball_y == pdx_y){
          dir_y = DIR_BOTTOM;
        } else if(ball_y == pdx_y + BAR_WIDTH -1){
          dir_y = DIR_TOP;
        } else {
          dir_y = DIR_BALANCE;
        }

        playSound();

      } else if (ball_x == WIDTH -1){

        #ifdef DEBUG
        Serial.println("[RIGHT] GOL");
        #endif

        game_ongoing = 0;
        winner = LEFT_WIN;
        resetVariables();
      }
    } else if(ball_y == 0 || ball_y == HEIGHT-1){

      #ifdef DEBUG
      Serial.println("Tentative Changing Y direction");
      #endif

      //Upper or lower border, change direction (not all the time otherwise it is a stalemate)
      int8_t rnd = random(100);
      if(rnd%2 == 0){

        #ifdef DEBUG
        Serial.println("Changing Y direction");
        #endif
        dir_y = -dir_y;
      }
      
    }
    
    delay(BALL_SPEED);

  }

  delay(1);
}

/* Loop for the player on the DX */
void playerDXLoop(){

  if(game_ongoing){
    updateBar(pdx_x, pdx_y, 0);
    handleEncoder(dxEncoder, &pdx_y);
    updateBar(pdx_x, pdx_y, 1);
    matrix.renderBitmap(frame, 8, 12);
    if(pdx_y != START_Y){ muted = 0; }
  } else {
    dxEncoder.set(pdx_y);
  }
  
  delay(1);
}

/* Loop for the player on the SX */
void playerSXLoop(){

  if(game_ongoing){
    updateBar(psx_x, psx_y, 0);
    handleEncoder(sxEncoder, &psx_y);
    updateBar(psx_x, psx_y, 1);
    matrix.renderBitmap(frame, 8, 12);
    if(psx_y != START_Y){ muted = 0; }
  } else {
    sxEncoder.set(psx_y);
  }
  
  delay(1);
}

/* Handle rotary movement and update pos_y */
void handleEncoder(ModulinoKnob k, int8_t *pos_y){

  int value = k.get();

  if(value < 0) {k.set(0);}
  if(value >= HEIGHT - BAR_WIDTH) {k.set((HEIGHT - BAR_WIDTH));}

  value = k.get();
  *pos_y = value;

}

/* Update pos_y in matrix */
void updateBar(int8_t x, int8_t start_y, byte state){
  for(uint8_t k = 0; k<BAR_WIDTH; k++){
    frame[start_y+k][x] = state;
  }
}

/* Reset variables */
void resetVariables(){

  /* Play Animation */
  #ifdef ANIMATION
  playEndAnimation(winner);
  #endif

  handleMatch(winner);

  /* Reset Matrix */
  memset(frame, 0, WIDTH * HEIGHT * sizeof(byte));
  
  /* Reset Status */
  ball_x = WIDTH/2;
  ball_y = HEIGHT/2;
  dir_x = DIR_LEFT;
  dir_y = DIR_BALANCE;
  psx_x = 0;
  psx_y = START_Y;
  pdx_x = WIDTH-1;
  pdx_y = START_Y;
  winner = -1;

  frame[ball_y][ball_x] = 1;
  dxEncoder.set(START_Y);
  sxEncoder.set(START_Y);

  #ifndef BLE_SYNC
  delay(1500);
  #endif

  #ifdef BLE_SYNC
  ble.sync(3000);
  #endif

  muted = 1;
  game_ongoing = 1;
}

/* Add point to winner*/
void handleMatch(int8_t winner){
  switch(winner){
    case LEFT_WIN:
      win_left++;
      #ifdef LEDS
      leds.set(win_left-1, leftColor, LED_BRI);
      #endif
      break;
    case RIGHT_WIN:
      win_right++;
      #ifdef LEDS
      leds.set(8-win_right, rightColor, LED_BRI);
      #endif
      break;
  }

  #ifdef LEDS
  leds.show();
  #endif

  if(win_right == MATCH_TO_WIN || win_left == MATCH_TO_WIN){
    showVictoryAnimation(winner);

    #ifdef BLE_SYNC
    bleCounter.setValue(bleCounter.getValue()+1);
    if(winner == LEFT_WIN){
      bleLeftWinner.setValue(bleLeftWinner.getValue()+1);
    } else {
      bleRightWinner.setValue(bleRightWinner.getValue()+1);
    }
    #endif

    win_right = 0;
    win_left = 0;
  }
}

/* Animate pixels for the winner player */
void showVictoryAnimation(int8_t winner) {

  ModulinoColor *winColor;
  uint8_t winnerAnim;

  switch(winner){
    case LEFT_WIN:
      winColor = &leftColor;
      winnerAnim = WINNER_ANIM_LEFT;
      break;
    case RIGHT_WIN:
      winColor = &rightColor;
      winnerAnim = WINNER_ANIM_RIGHT;
      break;
  }

  for(uint8_t j = 0; j<6; j++){

    #ifdef LEDS
    for(uint8_t i = 0; i<8; i++){ 
      leds.set(i, *winColor, LED_BRI);
    }
    leds.show();
    #endif

    #ifdef ANIMATION
    matrix.loadFrame(winner_animation[winnerAnim]);
    #endif

    delay(250);
    
    #ifdef LEDS
    for(uint8_t i = 0; i<8; i++){     
      leds.set(i, 0, 0, 0, LED_BRI);
    }
    leds.show();
    #endif

    #ifdef ANIMATION
    matrix.loadFrame(winner_animation[winnerAnim+1]);
    #endif

    delay(250);

    
  }

}

/* Play sound */
void playSound(){
  #ifdef BUZZER
  if(muted){return;}
  if(digitalRead(12) == LOW){
      return;
  }
  buzzer.tone(300+25*ball_x, 200);
  #endif
}

#ifdef ANIMATION
/* Show end game animation */
void playEndAnimation(int8_t start){
  for(int8_t i = start; i<start+7; i++){
    #ifdef BUZZER
    buzzer.tone(250+25*(i-start), 150);
    #endif
    matrix.loadFrame(crash[i]);
    delay(150);
  }
}
#endif

#ifdef BACKGROUND_LEDS
#define FLICKER_TIME 150
#define FLICKER_MAX 180
#define FLICKER_DIFF 30
void flickerBackground(){

  for(uint8_t i = 0; i<4; i++){
    uint8_t rand = random(FLICKER_DIFF);
    ledsBackground.set(i, rightColor, FLICKER_MAX-rand);
    delay(1);
  }

  for(uint8_t i = 4; i<8; i++){
    uint8_t rand = random(FLICKER_DIFF);
    ledsBackground.set(i, leftColor, FLICKER_MAX-rand);
    delay(1);
  }

  ledsBackground.show();
  delay(FLICKER_TIME);

}
#endif
