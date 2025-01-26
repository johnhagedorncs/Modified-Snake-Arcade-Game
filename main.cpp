// Importing libraries
#include <LinkedList.h>
#include "MCUFRIEND_kbv.h"
#include "SerialMP3Player.h"

#define ARDUINO_RX 12  //should connect to TX of the Serial MP3 Player module
#define ARDUINO_TX 13  //connect to RX of the module

//SoftwareSerial mp3(ARDUINO_RX, ARDUINO_TX);
SerialMP3Player mp3(ARDUINO_RX, ARDUINO_TX);

static int8_t Send_buf[8] = {0}; // Buffer for Send commands.  // BETTER LOCALLY
const uint8_t ROWS = 48, COLS = 32, LEN = 4; // defining variables

MCUFRIEND_kbv tft; // display screen

// Declare variables
uint8_t head_x, head_y; // x and y coordinates of head of the snake
uint8_t apple_x[4], apple_y[4];
uint8_t fakeApple_x[4], fakeApple_y[4];
uint8_t goldenApple_x[4], goldenApple_y[4];
uint8_t star_x, star_y;
LinkedList<uint8_t> snake_x = LinkedList<uint8_t>();
LinkedList<uint8_t> snake_y = LinkedList<uint8_t>();
LinkedList<uint8_t> poop_x = LinkedList<uint8_t>();
LinkedList<uint8_t> poop_y = LinkedList<uint8_t>();
uint8_t direction = 0; // initial direction is left
uint8_t length = LEN; // length initially is 4
bool immunity = false;
unsigned long startStarTime = 0;
bool showStarTimer = false;
unsigned long startGoldenTime = 0;
bool showGoldenTimer = false;
unsigned long startStarShowingTime = 0;
bool showStarShowingTimer = false;
bool confused = false;

// Helper functions
int* getLocation(int row, int column);
int* randomPosition();

int* randomPosition() {
 static int position[2];
 bool insidePoop = false;
 do {
   insidePoop = false;
   position[0] = random(ROWS-1);
   position[1] = random(COLS-1);
   for (int i = position[0] - 2 ; i <= position[0] + 2 ; i++) {
     for (int j = position[1] - 2 ; j <= position[1] + 2 ; j++) {
       if (inPoop(i, j)) insidePoop = true;
     }
   }
 } while (inSnake(position[0], position[1], length)
     || inSnake(position[0] + 1, position[1], length)
     || inSnake(position[0], position[1] + 1, length)
     || inSnake(position[0] + 1, position[1] + 1, length)
     || insidePoop
     || (position[1] < 3));
 return position;
}

bool inSnake(int x, int y, int length) {
 for (int i = 0 ; i < length ; i++) {
   if (snake_x.get(i) == x && snake_y.get(i) == y) return true;
 }
 return false;
}

bool inPoop(int x, int y) {
 for (int i = 0 ; i < poop_x.size() ; i++) {
   if (poop_x.get(i) == x && poop_y.get(i) == y)
     return true;
 }
 return false;
}

int whatPoop(int x, int y) {
 for (int i = 0 ; i < poop_x.size() ; i++) {
   if (poop_x.get(i) == x && poop_y.get(i) == y)
     return i;
 }
 return -1;
}

bool inApple(int x, int y) {
 for (int i = 0 ; i < 4 ; i++) {
   if (apple_x[i] == x && apple_y[i] == y)
     return true;
 }
 return false;
}

bool inFakeApple(int x, int y) {
 for (int i = 0 ; i < 4 ; i++) {
   if (fakeApple_x[i] == x && fakeApple_y[i] == y)
     return true;
 }
 return false;
}

bool inGoldenApple(int x, int y) {
 for (int i = 0 ; i < 4 ; i++) {
   if (goldenApple_x[i] == x && goldenApple_y[i] == y)
     return true;
 }
 return false;
}

bool inStar (int x, int y) {
 return x== star_x && star_y == y;
}

void setup() {
 Serial.begin(9600);
 mp3.begin(9600);
 pinMode(ARDUINO_TX, OUTPUT);
 mp3.reset();
 mp3.play();
 pinMode(10, OUTPUT);
 pinMode(A5, INPUT);
 randomSeed(analogRead(0) * random(10));
 uint16_t ID = tft.readID();
 tft.begin(ID);
 tft.fillScreen(0xFFFF); // white

 // Place the snake in the middle of the screen
 head_x = ROWS / 2;
 head_y = COLS / 2;

 // Place the apple in a random position
 apple_x[0] = randomPosition()[0];
 apple_y[0] = randomPosition()[1];
 apple_x[1] = apple_x[0] + 1;
 apple_y[1] = apple_y[0];
 apple_x[2] = apple_x[0];
 apple_y[2] = apple_y[0] + 1;
 apple_x[3] = apple_x[0] + 1;
 apple_y[3] = apple_y[0] + 1;

 goldenApple_x[0] = -10;
 goldenApple_y[0] = -10;
 goldenApple_x[1] = -10;
 goldenApple_y[1] = -10;
 goldenApple_x[2] = -10;
 goldenApple_y[2] = -10;
 goldenApple_x[3] = -10;
 goldenApple_y[3] = -10;

 for (int i = 0 ; i < 4 ; i++) {
   fakeApple_x[i] = -40;
   fakeApple_y[i] = -40;
 }

 star_x = -20;
 star_y= -20;

 // Setting up the snake looking down
 for (int i = 0; i < length; i++) {
   snake_x.add(head_x);
   snake_y.add(head_y - i - 1);
 }

 tft.setRotation(3);
 tft.setTextSize(2); // set text size to 5 (adjust as needed)s
 tft.setTextColor(TFT_BLACK); // set text color to white
 tft.setCursor(10, 10); // set cursor position
 tft.print(length - 4); // print the text
 tft.setRotation(0);
}

void loop() {
 if (showStarTimer && millis() - startStarTime > 10000) {
   immunity = false;
   Serial.print(poop_x.size() / 4);
   Serial.println(" POOPS");
   for (int i = 0 ; i < poop_x.size() - 1 ; i += 4) {
     drawPoop(poop_y.get(i) * 10, poop_x.get(i) * 10);
   }
   showStarTimer = false;
   tft.fillRect(10, 0, 20, 40, 0xFFFF);
 }
 if (showGoldenTimer && millis() - startGoldenTime > 7000) {
   showGoldenTimer = false;
   tft.fillRect(30, 0, 20, 40, 0xFFFF);
   tft.fillRect(goldenApple_y[0] * 10, goldenApple_x[0] * 10, 20, 20, 0xFFFF);
   goldenApple_x[0] = -10;
   goldenApple_y[0] = -10;
   goldenApple_x[1] = -10;
   goldenApple_y[1] = -10;
   goldenApple_x[2] = -10;
   goldenApple_y[2] = -10;
   goldenApple_x[3] = -10;
   goldenApple_y[3] = -10;
 }
 if (showStarShowingTimer && millis() - startStarShowingTime > 7000) {
   tft.fillRect(10, 0, 20, 40, 0xFFFF);
   showStarShowingTimer = false;
   Serial.print("Remove star");
   tft.fillRect(star_y *10, star_x *10, 10, 10, 0xFFFF);
   star_x = -20;
   star_y = -20;
 }

 // New head position
 int new_head_x = head_x;
 int new_head_y = head_y;

 if (showStarTimer) {
   tft.fillRect(10, 0, 20, 40, 0xFFFF);
   tft.setRotation(3);
   tft.setTextSize(2); // set text size to 5 (adjust as needed)s
   tft.setTextColor(TFT_RED); // set text color to white
   tft.setCursor(450, 10); // set cursor position
   tft.print(10 - (millis() - startStarTime) / 1000); // print the text
   tft.setRotation(0);
 }

 if (showGoldenTimer) {
   tft.fillRect(30, 0, 20, 40, 0xFFFF);
   tft.setRotation(3);
   tft.setTextSize(2); // set text size to 5 (adjust as needed)s
   tft.setTextColor(TFT_ORANGE); // set text color to white
   tft.setCursor(450, 30); // set cursor position
   tft.print(7 - (millis() - startGoldenTime) / 1000); // print the text
   tft.setRotation(0);
 }

 if (showStarShowingTimer) {
   tft.fillRect(10, 0, 20, 40, 0xFFFF);
   tft.setRotation(3);
   tft.setTextSize(2); // set text size to 5 (adjust as needed)s
   tft.setTextColor(TFT_RED); // set text color to white
   tft.setCursor(450, 10); // set cursor position
   tft.print(7 - (millis() - startStarShowingTime) / 1000); // print the text
   tft.setRotation(0);
 }

 if (immunity && head_x == ROWS - 1 && direction == 2) new_head_x = -1;
 if (immunity && head_x == 0 && direction == 0) new_head_x = ROWS;
 if (immunity && head_y == COLS - 1 && direction == 1) new_head_y = -1;
 if (immunity && head_y == 0 && direction == 3) new_head_y = COLS;
 if (direction == 0) new_head_x--; // right
 if (direction == 1) new_head_y++; // down
 if (direction == 2) new_head_x++; // left
 if (direction == 3) new_head_y--; // up

 if (immunity && inPoop(new_head_x, new_head_y)) {
   for (int i = 0 ; i < poop_x.size() ; i += 4) drawPoop(poop_y.get(i) * 10, poop_x.get(i) * 10);
 }

 // If not colliding with walls or itself
 if (immunity || (new_head_x >= 0 && new_head_x < ROWS &&
   new_head_y >= 0 && new_head_y < COLS &&
   !inSnake(new_head_x, new_head_y, length) &&
   !inPoop(new_head_x, new_head_y))) {

   // Add new head to the beginning of the snake
   head_x = new_head_x;
   head_y = new_head_y;
   int previous_x = snake_x.get(length - 1);
   int previous_y = snake_y.get(length - 1);

   // Update snake position and remove tail
   for (int i = length - 1; i > 0; i--) {
     snake_x.set(i, snake_x.get(i-1));
     snake_y.set(i, snake_y.get(i-1));
   }
   snake_x.set(0, head_x);
   snake_y.set(0, head_y);

   if (inApple(head_x, head_y)) {
     length++;

     for (int i = 0 ; i < poop_x.size() ; i += 4) drawPoop(poop_y.get(i) * 10, poop_x.get(i) * 10);

     confused = false;
     tft.fillRect(fakeApple_y[0] * 10, fakeApple_x[0] * 10, 20, 20, 0xFFFF);
     for (int i = 0 ; i < 4 ; i++) {
       fakeApple_x[i] = -40;
       fakeApple_y[i] = -40;
     }

     tft.fillRect(10, 440, 20, 40, 0xFFFF);
     tft.setRotation(3);
     tft.setTextSize(2); // set text size to 5 (adjust as needed)s
     tft.setTextColor(TFT_BLACK); // set text color to white
     tft.setCursor(10, 10); // set cursor position
     tft.print(length - 4); // print the text
     tft.setRotation(0);
      snake_x.add(previous_x);
     snake_y.add(previous_y);
     tft.fillRect(apple_y[0] * 10, apple_x[0] * 10, 20, 20, 0xFFFF);
     randomSeed(millis());
     apple_x[0] = randomPosition()[0];
     apple_y[0] = randomPosition()[1];
     apple_x[1] = apple_x[0] + 1;
     apple_y[1] = apple_y[0];
     apple_x[2] = apple_x[0];
     apple_y[2] = apple_y[0] + 1;
     apple_x[3] = apple_x[0] + 1;
     apple_y[3] = apple_y[0] + 1;

     if (length >= 7 && length % 2 == 0 ) {
       Serial.println("MORE POOP");
       int x = snake_x.get(length - 1);
       int y = snake_y.get(length - 1);
       poop_x.add(x);
       poop_y.add(y);
       poop_x.add(x+1);
       poop_y.add(y);
       poop_x.add(x);
       poop_y.add(y+1);
       poop_x.add(x+1);
       poop_y.add(y+1);
     }

     if (random(5) == 1 && !immunity && length > 9) {
       fakeApple_x[0] = randomPosition()[0];
       fakeApple_y[0] = randomPosition()[1];
       fakeApple_x[1] = fakeApple_x[0] + 1;
       fakeApple_y[1] = fakeApple_y[0];
       fakeApple_x[2] = fakeApple_x[0];
       fakeApple_y[2] = fakeApple_y[0] + 1;
       fakeApple_x[3] = fakeApple_x[0] + 1;
       fakeApple_y[3] = fakeApple_y[0] + 1;
     }
     if (random(7) == 1 && !showGoldenTimer && poop_x.size() > 0) {
       startGoldenTime = millis();
       showGoldenTimer = true;
       goldenApple_x[0] = randomPosition()[0];
       goldenApple_y[0] = randomPosition()[1];
       goldenApple_x[1] = goldenApple_x[0] + 1;
       goldenApple_y[1] = goldenApple_y[0];
       goldenApple_x[2] = goldenApple_x[0];
       goldenApple_y[2] = goldenApple_y[0] + 1;
       goldenApple_x[3] = goldenApple_x[0] + 1;
       goldenApple_y[3] = goldenApple_y[0] + 1;
     }

     if (random(12) == 1 && !immunity && star_x > 0) {
       startStarShowingTime = millis();
      
       showStarShowingTimer = true;
       star_x = randomPosition()[0];
       star_y = randomPosition()[1];
     }
   }
  
   if (inGoldenApple(head_x, head_y)) {
     showGoldenTimer = false;
     tft.fillRect(30, 0, 20, 40, 0xFFFF);
     for (int i = 0 ; i < poop_x.size() ; i += 4) {
       tft.fillRect(poop_y.get(i) * 10, poop_x.get(i) * 10, 20, 20, 0xFFFF);
     }
     poop_x.clear();
     poop_y.clear();
     tft.fillRect(goldenApple_y[0] * 10, goldenApple_x[0] * 10, 20, 20, 0xFFFF);
     goldenApple_x[0] = -10;
     goldenApple_y[0] = -10;
     goldenApple_x[1] = -10;
     goldenApple_y[1] = -10;
     goldenApple_x[2] = -10;
     goldenApple_y[2] = -10;
     goldenApple_x[3] = -10;
     goldenApple_y[3] = -10;
   }

   if (inFakeApple(head_x, head_y)) {
     confused = true;
     tft.fillRect(fakeApple_y[0] * 10, fakeApple_x[0] * 10, 20, 20, 0xFFFF);
     for (int i = 0 ; i < 4 ; i++) {
       fakeApple_x[i] = -40;
       fakeApple_y[i] = -40;
     }
   }
   if (inStar (head_x, head_y)){
     confused = false;
     tft.fillRect(fakeApple_y[0] * 10, fakeApple_x[0] * 10, 20, 20, 0xFFFF);
     for (int i = 0 ; i < 4 ; i++) {
       fakeApple_x[i] = -40;
       fakeApple_y[i] = -40;
     }
     tft.fillRect(50, 0, 20, 40, 0xFFFF);
     showStarShowingTimer = false;
     immunity = true;
     startStarTime = millis();
     showStarTimer = true;
     tft.fillRect(star_y *10, star_x *10, 10, 10, 0xFFFF);
     star_x = -20;
     star_y = -20;
   }

   // Use joystick
   digitalWrite(10, LOW);
   int y = 512 - analogRead(A5);
  
   // Set XY_SEL HIGH to read VRy.
   digitalWrite(10, HIGH);
   int x = 512 - analogRead(A5);

   if (y > 150 && x < 300 && x > -300 && (direction == 0 || direction == 2)) direction = confused ? 1 : 3; // up
   if (y < -150 && x < 300 && x > -300 && (direction == 0 || direction == 2)) direction = confused ? 3 : 1; // down
   if (x > 150 && y < 300 && y > -300 && (direction == 1 || direction == 3)) direction = confused ? 2 : 0; // right
   if (x < -150 && y < 300 && y > -300 && (direction == 1 || direction == 3)) direction = confused ? 0 : 2; // left

 } else {
   //(0X01); // play next song
   tft.fillScreen(0x0000);
   tft.setRotation(3);
   tft.setTextSize(13); // set text size to 5 (adjust as needed)s
   tft.setTextColor(TFT_WHITE); // set text color to white
   tft.setCursor(90, 60); // set cursor position
   tft.print("GAME\n OVER"); // print the text
   mp3.playNext();

   tft.setTextSize(2); // set text size to 5 (adjust as needed)s
   tft.setTextColor(TFT_WHITE); // set text color to white
   tft.setCursor(10, 10); // set cursor position
   tft.print("Score: ");
   tft.print(length - 4); // print the text

   delay(3000);
   tft.setCursor(60, 300);
   tft.print("Press red button to play again");

   tft.setRotation(0);
   while (1) {}
 }

 uint16_t snakeColor = 0x780F;
 if (confused) snakeColor = 0xFD20;
 if (immunity) {
   int color = random(7);
   if (color == 0) snakeColor = 0xFD20;
   if (color == 1) snakeColor = 0x7BE0;
   if (color == 2) snakeColor = 0x001F;
   if (color == 3) snakeColor = 0x03E0;
   if (color == 4) snakeColor = 0xF81F;
   if (color == 5) snakeColor = 0xF800;
   if (color == 6) snakeColor = 0x03EF;
 }

 // Draw the snake on the screen
 if (!inApple(head_x, head_y))
   tft.fillRect(snake_y[length - 1] * 10, snake_x[length - 1] * 10, 10, 10, 0xFFFF);
 tft.fillRect(snake_y[0] * 10, snake_x[0] * 10, 10, 10, snakeColor);
 tft.fillRect(snake_y[0] * 10 + 2, snake_x[0] * 10 + 2, 2, 2, 0xFD20);
 if (direction == 0 || direction == 2)
   tft.fillRect(snake_y[0] * 10 + 6, snake_x[0] * 10 + 2, 2, 2, 0xFD20);
 //tft.fillRect(snake_y[0] * 10 + 4, snake_x[0] * 10 + 5, 1, 4, 0xF800);
 if (direction == 1 || direction == 3)
   tft.fillRect(snake_y[0] * 10 + 2, snake_x[0] * 10 + 6, 2, 2, 0xFD20);
 tft.fillRect(snake_y[1] * 10, snake_x[1] * 10, 10, 10, snakeColor);
 //tft.fillRect(apple_y[0] * 10, apple_x[0] * 10, 20, 20, 0xF800);
 drawApple(apple_y[0] * 10, apple_x[0] * 10, 0);

 if (fakeApple_x[0] > -10) drawApple(fakeApple_y[0] * 10, fakeApple_x[0] * 10, 0);

 if (poop_x.size() != 0) drawPoop(poop_y.get(poop_y.size() - 4) * 10, poop_x.get(poop_x.size() - 4) * 10); 

 if (goldenApple_x[0] != -10) drawApple(goldenApple_y[0] * 10, goldenApple_x[0] * 10, 1);

 //if (star_x !=-20) tft.fillRect(star_y * 10, star_x * 10, 10, 10, 0XFFE0);
 if (star_x != 20) drawStar(star_y * 10, star_x * 10);
  delay((double)(400+2*length) / length + 30);
}

void drawStar(int y, int x) {
 tft.drawPixel(y+0, x+3, 0x0000);
 tft.drawPixel(y+0, x+4, 0XFFE0);
 tft.drawPixel(y+0, x+5, 0x0000);

 tft.drawPixel(y+1, x+3, 0x0000);
 tft.drawPixel(y+1, x+4, 0XFFE0);
 tft.drawPixel(y+1, x+5, 0XFFE0);
 tft.drawPixel(y+1, x+6, 0x0000);

 for (int i = 0 ; i < 3; i++) tft.drawPixel(y+2, x+i, 0x0000);
 for (int i = 3 ; i < 8 ; i++) tft.drawPixel(y+2, x+i, 0xFFE0);
 for (int i = 8 ; i < 11; i++) tft.drawPixel(y+2, x+i, 0x0000);

 for (int i = 0 ; i < 11 ; i++) tft.drawPixel(y+3, x+i, 0xFFE0);

 tft.drawPixel(y+4, x+0, 0x0000);
 for (int i = 1 ; i < 10 ; i++) tft.drawPixel(y+4, x+i, 0xFFE0);
 tft.drawPixel(y+4, x+10, 0x0000);

 for (int i = 1 ; i < 3 ; i++) tft.drawPixel(y+5, x+i, 0x0000);
 for (int i = 3 ; i < 8 ; i++) tft.drawPixel(y+5, x+i, 0xFFE0);
 tft.drawPixel(y+5, x+8, 0x0000);

 tft.drawPixel(y+6, x+2, 0x0000);
 for (int i = 3 ; i < 8 ; i++) tft.drawPixel(y+6, x+i, 0xFFE0);
 tft.drawPixel(y+6, x+8, 0x0000); 

 tft.drawPixel(y+7, x + 1, 0X0000);
 for (int i = 2 ; i < 3 ; i++) tft.drawPixel(y+7, x + i,0XFFE0);
 for (int i = 4 ; i < 5 ; i++) tft.drawPixel(y+7, x + i, 0X0000);
 for (int i = 6 ; i < 7 ; i++) tft.drawPixel(y+7, x + i,0XFFE0);
 tft.drawPixel(y+7, x + 8, 0X0000);

 tft.drawPixel(y+8, x + 1, 0X0000);
 tft.drawPixel(y+8, x + 2, 0XFFE0);
 tft.drawPixel(y+8, x + 3, 0X0000);
 tft.drawPixel(y+8, x + 6, 0X0000);
 for (int i = 7 ; i < 8 ; i++) tft.drawPixel(y+8, x + i,0XFFE0);
 tft.drawPixel(y+8, x + 9, 0X0000);

 tft.drawPixel(y+9, x, 0X0000);
 for (int i = 1 ; i < 2 ; i++) tft.drawPixel(y+9, x + i,0XFFE0);
 tft.drawPixel(y+9, x + 3, 0X0000);
 tft.drawPixel(y+9, x +7, 0X0000);
 for (int i = 8 ; i < 10 ; i++) tft.drawPixel(y+9, x + i, 0XFFE0);
}

void drawApple(int y, int x, int color) {
uint16_t leaf = 0x4BC0;
uint16_t brown = 0x8201;
uint16_t dark = 0xA800;
uint16_t light = 0xD000;
if (color == 1) {
 dark = 0x0000;
 light = 0xF74F;
}
for (int i = x + 5 ; i < x + 8 ; i++) tft.drawPixel(y, i, leaf);
for (int i = x + 6 ; i < x + 9 ; i++) tft.drawPixel(y+1, i, leaf);
tft.drawPixel(y+1, x+12, brown);
for (int i = x + 7 ; i < x + 11 ; i++) tft.drawPixel(y+2, i, leaf);
for (int i = x + 11 ; i < x + 13 ; i++) tft.drawPixel(y+2, i, brown);
tft.drawPixel(y+3, x+11, brown);
tft.drawPixel(y+4, x+11, brown);
tft.drawPixel(y+5, x+6, dark);
for (int i = x + 7 ; i < x + 15 ; i++) tft.drawPixel(y+5, i, light);
tft.drawPixel(y+5, x+15, dark);
tft.drawPixel(y+6, x+5, dark);
for (int i = x + 6 ; i < x + 16 ; i++) tft.drawPixel(y+6, i, light);
tft.drawPixel(y+6, x+16, dark);
tft.drawPixel(y+7, x+4, dark);
tft.fillRect(y+7, x+5, 11, 12, light);
tft.drawPixel(y+7, x+17, dark);
tft.drawPixel(y+8, x+3, dark);
tft.drawPixel(y+8, x+4, dark);
tft.drawPixel(y+8, x+17, dark);
tft.drawPixel(y+8, x+18, dark);
tft.drawPixel(y+9, x+3, dark);
tft.drawPixel(y+9, x+4, light);
tft.drawPixel(y+9, x+17, light);
tft.drawPixel(y+9, x+18, dark);
tft.drawPixel(y+10, x+2, dark);
tft.drawPixel(y+10, x+3, dark);
tft.drawPixel(y+10, x+4, light);
tft.drawPixel(y+10, x+17, light);
tft.drawPixel(y+10, x+18, dark);
tft.drawPixel(y+10, x+19, dark);
tft.drawPixel(y+11, x+2, dark);
tft.drawPixel(y+11, x+3, light);
tft.drawPixel(y+11, x+4, light);
tft.drawPixel(y+11, x+17, light);
tft.drawPixel(y+11, x+18, light);
tft.drawPixel(y+11, x+19, dark);
tft.drawPixel(y+12, x+1, dark);
tft.drawPixel(y+12, x+2, dark);
tft.drawPixel(y+12, x+3, light);
tft.drawPixel(y+12, x+4, light);
tft.drawPixel(y+12, x+17, light);
tft.drawPixel(y+12, x+18, light);
tft.drawPixel(y+12, x+19, dark);
tft.drawPixel(y+13, x+1, dark);
tft.drawPixel(y+13, x+2, light);
tft.drawPixel(y+13, x+3, light);
tft.drawPixel(y+13, x+4, light);
tft.drawPixel(y+13, x+17, light);
tft.drawPixel(y+13, x+18, light);
tft.drawPixel(y+13, x+19, light);
tft.drawPixel(y+14, x+1, dark);
tft.drawPixel(y+14, x+2, light);
tft.drawPixel(y+14, x+3, light);
tft.drawPixel(y+14, x+4, light);
tft.drawPixel(y+14, x+17, light);
tft.drawPixel(y+14, x+18, light);
tft.drawPixel(y+14, x+19, light);
tft.drawPixel(y+15, x+3, dark);
tft.drawPixel(y+15, x+4, light);
tft.drawPixel(y+15, x+17, light);
tft.drawPixel(y+15, x+18, dark);
tft.drawPixel(y+16, x+3, dark);
tft.drawPixel(y+16, x+4, light);
tft.drawPixel(y+16, x+17, light);
tft.drawPixel(y+16, x+18, dark);
tft.drawPixel(y+17, x+4, light);
tft.drawPixel(y+17, x+17, light);
tft.drawPixel(y+18, x+4, dark);
for (int i = x + 5 ; i < x + 17 ; i++) tft.drawPixel(y+18, i, light);
tft.drawPixel(y+18, x+17, dark);
for (int i = x + 7 ; i < x + 16 ; i++) tft.drawPixel(y+19, i, dark);
}

void drawPoop(int y, int x) {
// Dark brown = 99, 43, 1 = 0x6140
// Medium brown = 127, 74, 31 = 0x7A43
// Light brown = 151, 121, 102 = 0x93CC
tft.drawPixel(y+3, x+9, 0x93CC);
tft.drawPixel(y+3, x+10, 0x6140);
tft.drawPixel(y+3, x+11, 0x6140);
tft.drawPixel(y+4, x+8, 0x6140);
for (int i = x+9 ; i < x+11 ; i++) tft.drawPixel(y+4, i, 0x7A43);
tft.drawPixel(y+4, x+11, 0x6140);
tft.drawPixel(y+4, x+12, 0x93CC);

for (int i = x+7 ; i < x+12 ; i++) tft.drawPixel(y+5, i, 0x7A43);
tft.drawPixel(y+5, x+12, 0x6140);

tft.drawPixel(y+6, x+7, 0x6140);
for (int i = x+7 ; i < x+12 ; i++) tft.drawPixel(y+6, i, 0x7A43);
tft.drawPixel(y+6, x+12, 0x6140);
tft.drawPixel(y+6, x+13, 0x93CC);

tft.drawPixel(y+7, x+6, 0x93CC);
for (int i = x+7 ; i < x+9 ; i++) tft.drawPixel(y+7, i, 0x6140);
for (int i = x+9 ; i < x+13 ; i++) tft.drawPixel(y+7, i, 0x7A43);
tft.drawPixel(y+7, x+13, 0x6140);
tft.drawPixel(y+7, x+14, 0x93CC);

for (int i = x+5 ; i < x+11 ; i++) tft.drawPixel(y+8, i, 0x6140);
for (int i = x+11 ; i < x+14 ; i++) tft.drawPixel(y+8, i, 0x7A43);
tft.drawPixel(y+8, x+14, 0x6140);

tft.drawPixel(y+9, x+4, 0x93CC);
tft.drawPixel(y+9, x+5, 0x6140);
for (int i = x+6 ; i < x+9 ; i++) tft.drawPixel(y+9, i, 0x93CC);
for (int i = x+9 ; i < x+15 ; i++) tft.drawPixel(y+9, i, 0x6140);

tft.drawPixel(y+10, x+4, 0x93CC);
tft.drawPixel(y+10, x+5, 0x6140);
tft.drawPixel(y+10, x+6, 0x6140);
for (int i = x+7 ; i < x+12 ; i++) tft.drawPixel(y+10, i, 0x7A43);
for (int i = x+12 ; i < x+15 ; i++) tft.drawPixel(y+10, i, 0x6140);
tft.drawPixel(y+10, x+15, 0x93CC);

for (int i = x+3 ; i < x+9 ; i++) tft.drawPixel(y+11, i, 0x6140);
for (int i = x+9 ; i < x+16 ; i++) tft.drawPixel(y+11, i, 0x7A43);
tft.drawPixel(y+11, x+16, 0x93CC);

tft.drawPixel(y+12, x+2, 0x93CC);
tft.drawPixel(y+12, x+3, 0x6140);
for (int i = x+4 ; i < x+7 ; i++) tft.drawPixel(y+12, i, 0x7A43);
for (int i = x+7 ; i < x+11 ; i++) tft.drawPixel(y+12, i, 0x6140);
for (int i = x+11 ; i < x+16 ; i++) tft.drawPixel(y+12, i, 0x7A43);
tft.drawPixel(y+12, x+16, 0x6140); 

tft.drawPixel(y+13, x+2, 0x93CC);
tft.drawPixel(y+13, x+3, 0x6140);
for (int i = x+4 ; i < x+9 ; i++) tft.drawPixel(y+13, i, 0x7A43);
for (int i = x+9 ; i < x+17 ; i++) tft.drawPixel(y+13, i, 0x6140);

tft.drawPixel(y+14, x+2, 0x93CC);
tft.drawPixel(y+14, x+3, 0x6140);
tft.drawPixel(y+14, x+4, 0x6140);
for (int i = x+5 ; i < x+12 ; i++) tft.drawPixel(y+14, i, 0x7A43);
for (int i = x+12 ; i < x+17 ; i++) tft.drawPixel(y+14, i, 0x6140);
tft.drawPixel(y+14, x+17, 0x93CC);

for (int i = x+1 ; i < x+4 ; i++) tft.drawPixel(y+15, i, 0x7A43);
for (int i = x+4 ; i < x+7 ; i++) tft.drawPixel(y+15, i, 0x6140);
for (int i = x+7 ; i < x+18 ; i++) tft.drawPixel(y+15, i, 0x7A43);
tft.drawPixel(y+15, x+18, 0x93CC);

tft.drawPixel(y+16, x+1, 0x6140);
for (int i = x+2 ; i < x+5 ; i++) tft.drawPixel(y+16, i, 0x7A43);
for (int i = x+5 ; i < x+9 ; i++) tft.drawPixel(y+16, i, 0x6140);
for (int i = x+9 ; i < x+18 ; i++) tft.drawPixel(y+16, i, 0x7A43);
tft.drawPixel(y+16, x+18, 0x6140);

tft.drawPixel(y+17, x+1, 0x6140);
for (int i = x+2 ; i < x+7 ; i++) tft.drawPixel(y+17, i, 0x7A43);
for (int i = x+7 ; i < x+11 ; i++) tft.drawPixel(y+17, i, 0x6140);
for (int i = x+11 ; i < x+19 ; i++) tft.drawPixel(y+17, i, 0x7A43);

for (int i = x+1 ; i < x+8 ; i++) tft.drawPixel(y+18, i, 0x7A43);
for (int i = x+8 ; i < x+14 ; i++) tft.drawPixel(y+18, i, 0x6140);
for (int i = x+14 ; i < x+18 ; i++) tft.drawPixel(y+18, i, 0x7A43);
tft.drawPixel(y+18, x+18, 0x6140);
}
