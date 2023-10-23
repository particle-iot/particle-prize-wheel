/*
 * Particle Prize Wheel
 *
 * by Brett Walach ( @Technobly / github.com/technobly )
 * July 30th, 2023
 *
 */

#include "Particle.h"
#include "neopixel.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

// MOSI pin MO
#define PIXEL_PIN A7
#define PIXEL_ENABLE_PIN A6
#define PIXEL_COUNT 432
#define PIXEL_TYPE WS2812B

const int IR_RX_PIN = A5;
bool IR_RX_ASSERTED = false;

Adafruit_NeoPixel pixels(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

unsigned long pixelsInterval = 60; // the time we need to wait
unsigned long sparkleInterval = 5; // the time we need to wait
unsigned long colorWipePreviousMillis = 0;
unsigned long theaterChasePreviousMillis = 0;
unsigned long theaterChaseRainbowPreviousMillis = 0;
unsigned long rainbowPreviousMillis = 0;
unsigned long rainbowCyclesPreviousMillis = 0;
unsigned long rainbowCyclesStart = 0;
unsigned long sparklePreviousMillis = 0;
unsigned long sparkleStart = 0;
unsigned long sparkleDuration = 5000;
unsigned long lastSpinMillis = 0;
const unsigned long DELAY_BEFORE_SPARKLE_MS = 1000;

int theaterChaseQ = 0;
int theaterChaseQMax = 14; // theater chase pixel spacing
int theaterChaseRainbowQ = 0;
int theaterChaseRainbowQMax = 14; // theater chase pixel spacing
uint8_t theaterChaseRainbowQDelay = 0;
uint8_t theaterChaseRainbowCycles = 0;
int rainbowCycles = 0;
int rainbowCycleCycles = 0;

uint16_t currentPixel = 0;// what pixel are we operating on

void colorWipe(uint32_t c);
void rainbow();
void rainbowCycle();
void theaterChase(uint32_t c, bool invert = false);
void theaterChaseRainbow(bool invert = false);
uint32_t Wheel(byte WheelPos);
void sparkle();

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint16_t wait) {
    uint16_t i;

    for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, c);
    }
    pixels.show();
    delay(wait);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint32_t wait) {
    for (uint16_t i = pixels.numPixels(); i > 20; i-=20) {
        for (uint8_t x = 1; x <= 20; x++) {
            pixels.setPixelColor(i - x, c);
        }
        pixels.show();
    }
}

void sparkle() {
    const uint32_t colors[9] = {
        pixels.Color(255,0,0),     // RED
        pixels.Color(255,100,0),   // ORANGE
        pixels.Color(255,255,0),   // YELLOW
        pixels.Color(0,255,0),     // GREEN
        pixels.Color(0,255,255),   // CYAN
        pixels.Color(0,0,255),     // BLUE
        pixels.Color(255,0,255),   // MAGENTA
        pixels.Color(100,0,255),   // PURPLE
        pixels.Color(255,255,255), // WHITE
    };

    for (int x = 0; x < 11 ; x++) {
        uint32_t c = colors[random(9)];
        uint32_t p = random(pixels.numPixels()-22);
        pixels.setPixelColor(p, c);
        pixels.setPixelColor(p+1, c);
        pixels.setPixelColor(p+5, c);
        pixels.setPixelColor(p+9, c);
        pixels.setPixelColor(p+15, c);
        pixels.setPixelColor(p+22, c);
    }

    pixels.show();

    for (int x = 0; x < pixels.numPixels(); x++) {
        pixels.setPixelColor(x, 0);
    }

}

void rainbow() {
    for (uint16_t i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, Wheel((i + rainbowCycles) & 255));
    }
    pixels.show();
    rainbowCycles++;
    if (rainbowCycles >= 256) rainbowCycles = 0;
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
    uint16_t i;

    for (i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + rainbowCycleCycles) & 255));
    }
    pixels.show();

    rainbowCycleCycles++;
    if (rainbowCycleCycles >= 256 * 5) rainbowCycleCycles = 0;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, bool invert) {
    for (int i = 0; i < pixels.numPixels(); i = i + theaterChaseQMax + 1) {
        if (invert) {
            pixels.setPixelColor(i + theaterChaseQ + 0, 0);  //turn every N pixel off
            pixels.setPixelColor(i + theaterChaseQ + 1, 0);  //turn every N pixel off
        } else {
            pixels.setPixelColor(i + theaterChaseQ, c);  //turn every N pixel on
        }
    }
    pixels.show();
    for (int i = 0; i < pixels.numPixels(); i = i + theaterChaseQMax + 1) {
        if (invert) {
            pixels.setPixelColor(i + theaterChaseQ + 0, c);      //turn every N pixel on
            pixels.setPixelColor(i + theaterChaseQ + 1, c);      //turn every N pixel on
        } else {
            pixels.setPixelColor(i + theaterChaseQ, 0);      //turn every N pixel off
        }
    }
    // theaterChaseQ++;
    // if (theaterChaseQ >= 3) theaterChaseQ = 0;
}

void rgbLEDRainbow() {
    const uint32_t colors[9][3] = {
        {255,0,0},   // RED
        {255,100,0}, // ORANGE
        {255,255,0}, // YELLOW
        {0,255,0},   // GREEN
        {0,255,255}, // CYAN
        {0,0,255},   // BLUE
        {100,0,255}, // PURPLE
        {255,0,255}, // MAGENTA
    };

    RGB.color(colors[theaterChaseRainbowCycles][0], colors[theaterChaseRainbowCycles][1], colors[theaterChaseRainbowCycles][2]);
}

void rgbLEDSparkle() {
    const uint32_t colors[9][3] = {
        {255,0,0},   // RED
        {255,100,0}, // ORANGE
        {255,255,0}, // YELLOW
        {0,255,0},   // GREEN
        {0,255,255}, // CYAN
        {0,0,255},   // BLUE
        {100,0,255}, // PURPLE
        {255,0,255}, // MAGENTA
        {255,255,255}, // WHITE
    };

    uint8_t c = random(9); // 0-8
    uint8_t off = random(5); // 66% of the time we are turning it off
    if (off < 2) {
        RGB.color(colors[c][0], colors[c][1], colors[c][2]);
    } else {
        (void) c;
        RGB.color(0,0,0);
    }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(bool invert) {
    const uint32_t colors[9] = {
        pixels.Color(255,0,0),   // RED
        pixels.Color(255,100,0), // ORANGE
        pixels.Color(255,255,0), // YELLOW
        pixels.Color(0,255,0),   // GREEN
        pixels.Color(0,255,255), // CYAN
        pixels.Color(0,0,255),   // BLUE
        pixels.Color(100,0,255), // PURPLE
        pixels.Color(255,0,255), // MAGENTA
    };

    for (int i = 0; i < pixels.numPixels(); i = i + theaterChaseRainbowQMax + 3) {
        if (invert) {
            for (int x = i; x < i + theaterChaseRainbowQMax; x++) {
                pixels.setPixelColor(x, colors[theaterChaseRainbowCycles]);  //turn every pixel on this color
            }
            pixels.setPixelColor(i + theaterChaseRainbowQ + 0, 0);  //turn every N pixel off
            pixels.setPixelColor(i + theaterChaseRainbowQ + 1, 0);  //turn every N pixel off
            pixels.setPixelColor(i + theaterChaseRainbowQ + 2, 0);  //turn every N pixel off
            pixels.setPixelColor(i + theaterChaseRainbowQ + 3, 0);  //turn every N pixel off
        } else {
            pixels.setPixelColor(i + theaterChaseRainbowQ + 0, colors[theaterChaseRainbowCycles]); //turn every N pixel on this color
            pixels.setPixelColor(i + theaterChaseRainbowQ + 1, colors[theaterChaseRainbowCycles]); //turn every N pixel on this color
            pixels.setPixelColor(i + theaterChaseRainbowQ + 2, colors[theaterChaseRainbowCycles]); //turn every N pixel on this color
            pixels.setPixelColor(i + theaterChaseRainbowQ + 3, colors[theaterChaseRainbowCycles]); //turn every N pixel on this color
        }
    }

    pixels.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170) {
        WheelPos -= 85;
        return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setup() {
    currentPixel = 0;

    pinMode(IR_RX_PIN, INPUT);

    pixels.begin(); // This initializes the NeoPixel library.
    pixels.setBrightness(128);
    pinMode(PIXEL_ENABLE_PIN, OUTPUT); // low enabled
    pixels.show(); // This sends the updated pixel color to the hardware.

    Particle.connect();

    RGB.control(true);
    RGB.color(0,255,255); // Solid CYAN

}

const int STATE_IDLE = 0;
const int STATE_OFF = 1;
const int STATE_SPIN_START = 2;
const int STATE_SPIN = 3;
const int STATE_SPARKLE_START = 4;
const int STATE_SPARKLE = 5;
int state = STATE_IDLE;
int spinCount = 0;

void loop() {

    switch (state) {
        case STATE_SPIN_START: {
            colorAll(0, 0);
            // colorWipe(0, 0);
            lastSpinMillis = millis();
            state = STATE_SPIN;
            break;
        }
        case STATE_SPIN: {
            if (millis() - lastSpinMillis < DELAY_BEFORE_SPARKLE_MS) {
                if (millis() - theaterChaseRainbowPreviousMillis >= pixelsInterval) {
                    theaterChaseRainbowPreviousMillis = millis();
                    theaterChaseRainbow(false);
                    rgbLEDRainbow();
                }
            } else {
                if (spinCount > 16) {
                    state = STATE_SPARKLE_START;
                    sparkleStart = millis();
                } else {
                    state = STATE_OFF;
                }
            }
            break;
            }
        case STATE_SPARKLE_START: {
            colorAll(0, 0);
            sparkleInterval = 5;
            state = STATE_SPARKLE;
            break;
        }
        case STATE_SPARKLE: {
            if (millis() - sparklePreviousMillis >= sparkleInterval) {
                sparklePreviousMillis = millis();
                if (sparkleInterval < 80) {
                    sparkleInterval++;
                }
                sparkle();
                rgbLEDSparkle();
            }
            if (millis() - sparkleStart > sparkleDuration) {
                state = STATE_OFF;
            }
            break;
        }
        default:
        case STATE_OFF: {
            colorAll(0, 0);
            spinCount = 0;
            state = STATE_IDLE;
            break;
        }
        case STATE_IDLE: {
            if (millis() - theaterChasePreviousMillis >= pixelsInterval) {
                theaterChasePreviousMillis = millis();
                // if (++theaterChaseQDelay > 4) {
                //     theaterChaseQDelay = 0;
                    theaterChaseQ--;
                    theaterChaseQMax = 14;
                // }
                if (theaterChaseQ <= 0) {
                    theaterChaseQ = theaterChaseQMax;
                }
                theaterChase(pixels.Color(0,255,255), true /* invert */);
                RGB.color(0,255,255); // SOLID CYAN
            }
            break;
        }
    }

    if (!IR_RX_ASSERTED) {
        if (digitalRead(IR_RX_PIN) == HIGH) {
            IR_RX_ASSERTED = true;
            spinCount++;

            if (state != STATE_SPARKLE && spinCount > 2) {
                theaterChaseRainbowCycles++;
                if (theaterChaseRainbowCycles > 7) {
                    theaterChaseRainbowCycles = 0;
                }
                theaterChaseRainbowQMax = 12;
                theaterChaseRainbowQ++;
                if (theaterChaseRainbowQ >= theaterChaseRainbowQMax + 3) {
                    theaterChaseRainbowQ = 0;
                }
                state = STATE_SPIN_START;
                delay(15);
            }
        }
    } else {
        if (digitalRead(IR_RX_PIN) == LOW) {
            IR_RX_ASSERTED = false;
            delay(15);
        }
    }

}
