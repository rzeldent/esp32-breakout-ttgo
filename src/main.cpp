#include <WiFi.h>
#include <SPI.h>
#include <soc/rtc_cntl_reg.h>
#include <TFT_eSPI.h>

#include <ttgo_display.h>

#include <miniz.h>
#include <images.h>

constexpr auto font_16pt = 2; // # Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
constexpr auto font_26pt = 4; // # Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters

constexpr auto background_color = TFT_BLACK;
constexpr auto text_color = TFT_WHITE;

constexpr auto frame_color = TFT_LIGHTGREY;
constexpr auto ball_color = TFT_YELLOW;
constexpr auto paddle_color = TFT_SILVER;

constexpr auto ball_size = 2;
constexpr auto paddle_width = 24;
constexpr auto paddle_height = 4;
constexpr auto paddle_y = 234;

constexpr auto tile_width = (uint16_t)20;
constexpr auto tile_height = (uint16_t)4;

struct point
{
  float x, y;

  bool operator!=(const point &other) const
  {
    return x != other.x || y != other.y;
  }

  point &operator+=(const point &other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }
};

struct tile
{
  int16_t x, y;
  uint16_t cx, cy;

  uint16_t color;
  bool visible;
};

auto tft = TFT_eSPI();

float random_speed()
{
  static const float speed_factor[] = {-1.0, -0.75, -0.5, -0.25, 0.25, 0.50, 0.75, 1.0};
  return speed_factor[random(sizeof(speed_factor) / sizeof(speed_factor[0]))];
}

float random_x()
{
  return (float)random(30, 100);
}

auto ball = point{random_x(), 70};
auto ball_prev = ball;
auto ball_speed = point{random_speed(), 1};

auto paddle_x = 45; // 67 is the midfield position of the players
auto paddle_prev_x = paddle_x;

tile tiles[] = {
    {8, 37, tile_width, tile_height, TFT_RED},
    {33, 37, tile_width, tile_height, TFT_RED},
    {58, 37, tile_width, tile_height, TFT_RED},
    {83, 37, tile_width, tile_height, TFT_RED},
    {108, 37, tile_width, tile_height, TFT_RED},
    {8, 45, tile_width, tile_height, TFT_GREEN},
    {33, 45, tile_width, tile_height, TFT_GREEN},
    {58, 45, tile_width, tile_height, TFT_GREEN},
    {83, 45, tile_width, tile_height, TFT_GREEN},
    {108, 45, tile_width, tile_height, TFT_GREEN},
    {22, 53, tile_width, tile_height, TFT_ORANGE},
    {47, 53, tile_width, tile_height, TFT_ORANGE},
    {72, 53, tile_width, tile_height, TFT_ORANGE},
    {97, 53, tile_width, tile_height, TFT_ORANGE},
    {47, 61, tile_width, tile_height, TFT_SKYBLUE},
    {72, 61, tile_width, tile_height, TFT_SKYBLUE},
    {56, 69, tile_width, tile_height, TFT_GREENYELLOW}};

int gameSpeed = 15000;
uint score = 0;
uint level = 1;

enum game_state
{
  splash,
  play,
  over
} game_state = splash;

void setup()
{
  // Disable brownout
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  //  ADC_EN is the ADC detection enable port
  //  If the USB port is used for power supply, it is turned on by default.
  //  If it is powered by battery, it needs to be set to high level
  pinMode(GPIO_ADC_EN, OUTPUT);
  digitalWrite(GPIO_ADC_EN, HIGH);

  // Turn off WiFi and Bluetooth to save power
  WiFi.mode(WIFI_OFF);
  btStop();

  pinMode(GPIO_BUTTON_BOTTOM, INPUT);
  pinMode(GPIO_BUTTON_TOP, INPUT);

  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.setTextColor(text_color, background_color);
  // Show splash screen
  auto image_data = z_image_decode(&image_splash);
  tft.pushImage(0, 0, image_splash.width, image_splash.height, image_data);
  delete[] image_data;
}

void setup_level()
{
  ball = point{random_x(), 75};
  ball_speed.y = 1;
  for (auto &tile : tiles)
  {
    tile.visible = true;
    tft.fillRect(tile.x, tile.y, tile.cx, tile.cy, tile.color);
  }
}

void next_level()
{
  level++;
  gameSpeed -= 500;
  delay(3000);
  tft.drawString("LVL " + String(level), 99, 0, font_16pt);
  setup_level();
}

void game_wait()
{
  // Wait till any of the buttons is pressed
  if (!digitalRead(GPIO_BUTTON_BOTTOM) || !digitalRead(GPIO_BUTTON_TOP))
  {
    tft.fillScreen(background_color);
    tft.drawLine(0, 17, 0, 240, frame_color);
    tft.drawLine(0, 17, 135, 17, frame_color);
    tft.drawLine(134, 17, 134, 240, frame_color);

    tft.drawString("Score " + String(score), 0, 0, font_16pt);
    setup_level();
    game_state = play;
  }
}

void game_play()
{
  if (ball.y != ball_prev.y)
  {
    // Erase the previous ball position
    tft.fillEllipse(ball_prev.x, ball_prev.y, ball_size, ball_size, background_color);
    ball_prev = ball;
  }

  if (paddle_x != paddle_prev_x)
  {
    // Erase the previous paddle position
    tft.fillRect(paddle_prev_x, paddle_y, paddle_width, paddle_height, background_color);
    paddle_prev_x = paddle_x;
  }

  if (paddle_x >= 4 && !digitalRead(GPIO_BUTTON_BOTTOM))
    paddle_x -= 2;

  if (paddle_x <= 107 && !digitalRead(GPIO_BUTTON_TOP))
    paddle_x += 2;

  for (auto &tile : tiles)
  {
    // Check if the ball hits the tile
    if (tile.visible && (ball.x >= tile.x && ball.x < tile.x + tile.cx && ball.y >= tile.y && ball.y < tile.y + tile.cy))
    {
      tft.fillRect(tile.x, tile.y, tile.cx, tile.cy, background_color);
      tile.visible = false;
      // Bounce off tile
      ball_speed = point{random_speed(), -ball_speed.y};
      score++;
      tft.drawString("Score: " + String(score), 0, 0, font_16pt);
    }
  }

  // Check if ball went through the bottom
  if (ball.y > 240)
    game_state = game_state::over;

  // Bounce on paddle
  if (ball.y > 232 && ball.x > paddle_x && ball.x < paddle_x + paddle_width)
    ball_speed = point{random_speed(), -1};

  // Bounce at the top
  if (ball.y < 21)
    ball_speed.y = 1;

  // Bounce at the sides
  if (ball.x >= 130 || ball.x <= 4)
    ball_speed.x = -ball_speed.x;

  // Draw the ball
  tft.fillEllipse(ball.x, ball.y, ball_size, ball_size, ball_color);
  // Move the ball
  ball += ball_speed;
  // Draw the paddle
  tft.fillRect(paddle_x, paddle_y, paddle_width, paddle_height, paddle_color);

  delayMicroseconds(gameSpeed);

  // If a tile is still visible, level is not done
  for (const auto &tile : tiles)
    if (tile.visible)
      return;

  next_level();
}

void game_over()
{
  tft.fillScreen(background_color);
  tft.drawCentreString("GAME OVER", tft.width() / 2, 103 / 2, font_16pt);
  tft.drawCentreString("Score: " + String(score), tft.width() / 2, 123, font_26pt);
  tft.drawCentreString("Level: " + String(level), tft.width() / 2, 153, font_26pt);
  delay(10000);
  ESP.restart();
}

void loop()
{
  switch (game_state)
  {
  case splash:
    game_wait();
    break;
  case play:
    game_play();
    break;
  case over:
    game_over();
  }
}