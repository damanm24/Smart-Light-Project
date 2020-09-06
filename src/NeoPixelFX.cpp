#include <NeoPixelFX.h>
#include <Arduino.h>

void NeoPixelFX::init(void)
{
    Adafruit_NeoPixel::begin();
}

void NeoPixelFX::service(void)
{
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    bool doShow = false;
    CLR_FRAME_CYCLE;
    if (now > m_curSegment.next_time || triggered)
    {
        SET_FRAME;
        doShow = true;
        uint16_t delay = (this->*m_modes[m_curSegment.mode])();
        m_curSegment.next_time = now + delay;
        triggered = false;
    }

    if (doShow)
    {
        delay(1);
        show();
    }
}

uint32_t NeoPixelFX::color_blend(uint32_t color1, uint32_t color2, uint8_t blend)
{
    if (blend == 0)
        return color1;
    if (blend == 255)
        return color2;

    uint8_t w1 = (color1 >> 24) & 0xff;
    uint8_t r1 = (color1 >> 16) & 0xff;
    uint8_t g1 = (color1 >> 8) & 0xff;
    uint8_t b1 = color1 & 0xff;

    uint8_t w2 = (color2 >> 24) & 0xff;
    uint8_t r2 = (color2 >> 16) & 0xff;
    uint8_t g2 = (color2 >> 8) & 0xff;
    uint8_t b2 = color2 & 0xff;

    uint32_t w3 = ((w2 * blend) + (w1 * (255U - blend))) / 256U;
    uint32_t r3 = ((r2 * blend) + (r1 * (255U - blend))) / 256U;
    uint32_t g3 = ((g2 * blend) + (g1 * (255U - blend))) / 256U;
    uint32_t b3 = ((b2 * blend) + (b1 * (255U - blend))) / 256U;

    return ((w3 << 24) | (r3 << 16) | (g3 << 8) | (b3));
}

void NeoPixelFX::setColor(uint32_t c)
{
    m_curSegment.colors[0] = c;
    triggered = true;
}

void NeoPixelFX::setPixelColor(uint16_t n, uint32_t c)
{
    uint8_t w = (c >> 24) & 0xFF;
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    setPixelColor(n, r, g, b, w);
}

void NeoPixelFX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
    setPixelColor(n, r, g, b, 0);
}

void NeoPixelFX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    Adafruit_NeoPixel::setPixelColor(n, r, g, b, w);
    triggered = true;
}

void NeoPixelFX::copyPixels(uint16_t dest, uint16_t src, uint16_t count)
{
    uint8_t *pixels = getPixels();
    uint8_t bytesPerPixel = getNumBytesPerPixel(); // 3=RGB, 4=RGBW

    memmove(pixels + (dest * bytesPerPixel), pixels + (src * bytesPerPixel), count * bytesPerPixel);
    triggered = true;
}

void NeoPixelFX::setPixels(uint16_t num_leds, uint8_t *ptr)
{
    free(Adafruit_NeoPixel::pixels); // free existing data (if any)
    Adafruit_NeoPixel::pixels = ptr;
    Adafruit_NeoPixel::numLEDs = num_leds;
    Adafruit_NeoPixel::numBytes = num_leds * ((wOffset == rOffset) ? 3 : 4);
    triggered = true;
}

void NeoPixelFX::show(void)
{
    Adafruit_NeoPixel::show();
}

void NeoPixelFX::setBrightness(uint8_t b)
{
    b = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    Adafruit_NeoPixel::setBrightness(b);
    show();
}

void NeoPixelFX::increaseBrightness(uint8_t s)
{
    triggered = true;
    s = constrain(getBrightness() + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    setBrightness(s);
}

void NeoPixelFX::decreaseBrightness(uint8_t s)
{
    triggered = true;
    s = constrain(getBrightness() - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    setBrightness(s);
}

void NeoPixelFX::strip_off()
{
    Adafruit_NeoPixel::clear();
    show();
}

void NeoPixelFX::setSpeed(uint16_t s)
{
    m_curSegment.speed = s;
    triggered = true;
}

void NeoPixelFX::increaseSpeed(uint8_t s)
{
    triggered = true;
    uint16_t newSpeed = constrain(m_curSegment.speed + s, SPEED_MIN, SPEED_MAX);
    setSpeed(newSpeed);
}

void NeoPixelFX::decreaseSpeed(uint8_t s)
{
    triggered = true;
    uint16_t newSpeed = constrain(m_curSegment.speed - s, SPEED_MIN, SPEED_MAX);
    setSpeed(newSpeed);
}

void NeoPixelFX::setMode(uint8_t m)
{
    triggered = true;
    m_curSegment.mode = m;
}

uint8_t NeoPixelFX::getMode(void)
{
    return m_curSegment.mode;
}

uint8_t NeoPixelFX::getNumBytesPerPixel(void)
{
    return (wOffset == rOffset) ? 3 : 4; // 3=RGB, 4=RGBW
}

uint16_t NeoPixelFX::getSpeed(void)
{
    return m_curSegment.speed;
}

uint16_t NeoPixelFX::mode_static(void)
{
    Serial.println("Filling Pixels");
    fill(m_curSegment.colors[0]);
    SET_CYCLE;
    return 1000;
}

uint16_t NeoPixelFX::mode_rainbow(void)
{
    uint32_t color = color_wheel(m_curSegment.counter_mode_step);
    fill(color);

    m_curSegment.counter_mode_step = (m_curSegment.counter_mode_step + 1) & 0xFF;

    if (m_curSegment.counter_mode_step == 0)
        SET_CYCLE;

    return (m_curSegment.speed / 256);
}

uint16_t NeoPixelFX::mode_rainbow_cycle(void)
{
    for (uint16_t i = 0; i < m_numLeds; i++)
    {
        uint32_t color = color_wheel(((i * 256 / m_numLeds) + m_curSegment.counter_mode_step) & 0xFF);
        setPixelColor(i, color);
    }
    m_curSegment.counter_mode_step = (m_curSegment.counter_mode_step + 1) & 0xFF;
    if (m_curSegment.counter_mode_step == 0)
    {
        SET_CYCLE;
    }
    return (m_curSegment.speed / 256);
}

uint16_t NeoPixelFX::mode_breath(void)
{
    int lum = m_curSegment.counter_mode_step;
    if (lum > 255)
        lum = 511 - lum; // lum = 15 -> 255 -> 15

    uint16_t delay;
    if (lum == 15)
        delay = 970; // 970 pause before each breath
    else if (lum <= 25)
        delay = 38; // 19
    else if (lum <= 50)
        delay = 36; // 18
    else if (lum <= 75)
        delay = 28; // 14
    else if (lum <= 100)
        delay = 20; // 10
    else if (lum <= 125)
        delay = 14; // 7
    else if (lum <= 150)
        delay = 11; // 5
    else
        delay = 10; // 4

    uint32_t color = color_blend(m_curSegment.colors[1], m_curSegment.colors[0], lum);
    fill(color);

    m_curSegment.counter_mode_step += 2;
    if (m_curSegment.counter_mode_step > (512 - 15))
    {
        m_curSegment.counter_mode_step = 15;
        SET_CYCLE;
    }
    return delay;
}

uint16_t NeoPixelFX::mode_running_lights(void)
{
    uint8_t sineIncr = max(1, (256 / m_numLeds));
    for (uint16_t i = 0; i < m_numLeds; i++)
    {
        int lum = (int)sine8((i * sineIncr));
        uint32_t color = color_blend(m_curSegment.colors[0], m_curSegment.colors[1], lum);
        setPixelColor(i, color);
    }
    m_curSegment.counter_mode_step = (m_curSegment.counter_mode_step + 1) % 256;
    if (m_curSegment.counter_mode_step == 0)
        SET_CYCLE;
    return (m_curSegment.speed / m_numLeds);
}

uint32_t NeoPixelFX::getColor(void)
{
    return m_curSegment.colors[0];
}

uint32_t NeoPixelFX::color_wheel(uint8_t pos)
{
    pos = 255 - pos;
    if (pos < 85)
    {
        return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
    }
    else if (pos < 170)
    {
        pos -= 85;
        return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
    }
    else
    {
        pos -= 170;
        return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
    }
}