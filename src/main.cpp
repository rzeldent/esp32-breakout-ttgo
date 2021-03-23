// 240x135

#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "bmp.h"

#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

auto tft = TFT_eSPI(); // Invoke custom library

auto ys = 1;
auto x = (int)random(30, 100); // coordinates of ball
auto y = 70;

auto ny = y; // coordinates of previous position
auto nx = x;

auto px = 45; // 67 is the midfield position of the players
int pxn = px;

const int vrije[2] = {1, -1};
int enx[16] = {8, 33, 58, 83, 108, 8, 33, 58, 83, 108, 22, 47, 72, 97, 47, 72};
int eny[16] = {37, 37, 37, 37, 37, 45, 45, 45, 45, 45, 53, 53, 53, 53, 61, 61};
const int enc[16] = {TFT_RED, TFT_RED, TFT_RED, TFT_RED, TFT_RED, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_ORANGE, TFT_ORANGE, TFT_ORANGE, TFT_ORANGE, TFT_SKYBLUE, TFT_SKYBLUE};

uint score = 0;
uint level = 1;

float amount[4] = {0.25, 0.50, 0.75, 1};
float xs = amount[random(4)] * vrije[random(2)];

enum game_state
{
  initialize,
  playing,
  gameover
};
game_state state = game_state::initialize;

void setup()
{
  // Turn off WiFi and Bluetooth to save power
  WiFi.mode(WIFI_OFF);
  btStop();

  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);

  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 135, 240, bootlogo);
}

int gameSpeed = 10000;

void newLevel()
{
  score++;
  level++;
  gameSpeed -= 500;
  delay(3000);

  tft.setCursor(99, 0, 2);
  tft.println("LVL" + String(level));
  y = 75;
  ys = 1;
  x = random(30, 100);

  int enx2[16] = {8, 33, 58, 83, 108, 8, 33, 58, 83, 108, 22, 47, 72, 97, 47, 72};
  for (int n = 0; n < 16; n++)
    enx[n] = enx2[n];
}

void loop()
{
  switch (state)
  {
  case initialize:
    if (!digitalRead(BUTTON_LEFT) || !digitalRead(BUTTON_RIGHT))
    {
      tft.fillScreen(TFT_BLACK);
      tft.drawLine(0, 17, 0, 240, TFT_LIGHTGREY);
      tft.drawLine(0, 17, 135, 17, TFT_LIGHTGREY);
      tft.drawLine(134, 17, 134, 240, TFT_LIGHTGREY);

      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(1);

      tft.setCursor(0, 0, 2);
      tft.println("SCORE " + String(score));

      tft.setCursor(99, 0, 2);
      tft.println("LVL" + String(level));
      state = playing;
    }
    break;

  case playing:
    if (y != ny)
    {
      tft.fillEllipse(nx, ny, 2, 2, TFT_BLACK); // wiping the ball
      ny = y;
      nx = x;
    }
    if (px != pxn)
    {
      tft.fillRect(pxn, 234, 24, 4, TFT_BLACK); // deleting players
      pxn = px;
    }

    if (px >= 2 && px <= 109)
    {
      if (!digitalRead(BUTTON_LEFT))
        px -= 2;

      if (!digitalRead(BUTTON_RIGHT))
        px += 2;
    }

    if (px <= 3)
      px = 4;

    if (px >= 108)
      px = 107;

    if (y > 232 && x > px && x < px + 24)
    { // delete later
      ys = -ys;
      xs = amount[random(4)] * vrije[random(2)];
    }

    for (int j = 0; j < 16; j++)
    {
      if (x > enx[j] && x < enx[j] + 20 && y > eny[j] && y < eny[j] + 5)
      {
        tft.fillRect(enx[j], eny[j], 20, 4, TFT_BLACK);
        enx[j] = 400;
        ys = -ys;
        xs = amount[random(4)] * vrije[random(2)];
        score++;
        tft.setCursor(0, 0, 2);
        tft.println("SCORE " + String(score));
      }
    }

    if (y < 21)
      ys = -ys;

    if (y > 240)
      state = gameover;

    if (x >= 130)
      xs = -xs;

    if (x <= 4)
      xs = -xs;

    for (int i = 0; i < 16; i++) // draw enemies
      tft.fillRect(enx[i], eny[i], 20, 4, enc[i]);

    tft.fillEllipse(x, y, 2, 2, TFT_WHITE); // draw ball

    y += ys;
    x += xs;

    tft.fillRect(px, 234, 24, 4, TFT_WHITE);

    if (score == 16 || score == 33 || score == 50 || score == 67 || score == 84 || score == 101 || score == 118 || score == 135 || score == 152 || score == 169)
      newLevel();

    delayMicroseconds(gameSpeed);
    break;

  case gameover:
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);

    tft.setCursor(13, 103, 2);
    tft.println("GAME OVER");

    tft.setCursor(13, 123, 4);
    tft.println("Score: " + String(score));

    tft.setCursor(13, 153, 4);
    tft.println("Level: " + String(level));

    delay(10000);
    ESP.restart();
  }
}
