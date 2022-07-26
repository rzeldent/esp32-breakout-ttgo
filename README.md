# esp32-breakout-TTGO

[![Platform IO CI](https://github.com/rzeldent/esp32-breakout-ttgo/actions/workflows/main.yml/badge.svg)](https://github.com/rzeldent/esp32-breakout-ttgo/actions/workflows/main.yml)

All the credits go to [Volos Projects](https://www.youtube.com/channel/UCit2rVgOvhyuAD-VH5H_IHg).

![Splash screen](images/image_splash.bmp)

Initially just made this repository to easy upload the game using PlatformIO.
However over time stared to refactor the game to make it more readable/understandable for myself.

Changes:
- The splash screen has been converted back to BMP and is stored as a GZIP image.
- Structures for ball(speed), and tiles, reorganized the tile
- New names for variables
- Checking if all the tiles are hit instead of checking the score
- ...

See the original video on YouTube:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/N6V7ZJkhSbc/0.jpg)](http://www.youtube.com/watch?v=N6V7ZJkhSbc "Breakout")
