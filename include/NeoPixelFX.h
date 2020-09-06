#include <Adafruit_NeoPixel.h>

#define BRIGHTNESS_MIN (uint8_t)0
#define BRIGHTNESS_MAX (uint8_t)255
#define MODE_COUNT 5

#define SPEED_MIN (uint16_t)2
#define SPEED_MAX (uint16_t)65535

#define FRAME           (uint8_t)B10000000
#define SET_FRAME       (m_curSegment.aux_param2 |=  FRAME)
#define CLR_FRAME       (m_curSegment.aux_param2 &= ~FRAME)
#define CYCLE           (uint8_t)B01000000
#define SET_CYCLE       (m_curSegment.aux_param2 |=  CYCLE)
#define CLR_CYCLE       (m_curSegment.aux_param2 &= ~CYCLE)
#define CLR_FRAME_CYCLE (m_curSegment.aux_param2 &= ~(FRAME | CYCLE))

#define MAX_NUM_COLORS            3

class NeoPixelFX : public Adafruit_NeoPixel
{
public:
    typedef uint16_t (NeoPixelFX::*mode_ptr)(void);
    typedef struct segment
    { // 16 bytes
        unsigned long next_time;
        uint8_t mode;
        uint16_t speed;
        uint32_t counter_mode_step;
        uint8_t aux_param2;
        uint32_t colors[MAX_NUM_COLORS];
    } segment;

    NeoPixelFX(uint16_t num_leds, uint8_t pin, neoPixelType type) : Adafruit_NeoPixel(num_leds, pin, type)
    {
        m_curSegment.speed = 0;
        m_curSegment.mode = 0;
        m_curSegment.counter_mode_step = 0;
        m_curSegment.aux_param2 = 0;
        m_curSegment.colors[0] = 0;
        m_curSegment.colors[1] = 0;
        m_curSegment.colors[2] = 0;
    }

    void
      init(void),
      service(void),
      strip_off(void),
      setMode(uint8_t m),
      setColor(uint32_t c),
      setColor(uint8_t r, uint8_t g, uint8_t b),
      setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w),
      setBrightness(uint8_t b),
      increaseBrightness(uint8_t s),
      decreaseBrightness(uint8_t s),
      setPixelColor(uint16_t n, uint32_t c),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w),
      setSpeed(uint16_t s),
      increaseSpeed(uint8_t s),
      decreaseSpeed(uint8_t s),
      copyPixels(uint16_t d, uint16_t s, uint16_t c),
      setPixels(uint16_t, uint8_t*),
      show(void);

    uint8_t
        getMode(void),
        getNumBytesPerPixel(void);

    uint32_t
        color_blend(uint32_t, uint32_t, uint8_t),
        color_wheel(uint8_t pos),
        getColor(void);
    
    uint16_t
        getSpeed(void),
        mode_static(void),
        mode_rainbow(void),
        mode_rainbow_cycle(void),
        mode_breath(void),
        mode_running_lights(void);

    segment m_curSegment;
    uint16_t m_numLeds;
    uint8_t m_pin;
    neoPixelType m_type;

    boolean triggered = false;
};

static NeoPixelFX::mode_ptr m_modes[MODE_COUNT] = {
    &NeoPixelFX::mode_static,
    &NeoPixelFX::mode_rainbow,
    &NeoPixelFX::mode_rainbow_cycle,
    &NeoPixelFX::mode_breath,
    &NeoPixelFX::mode_running_lights,
};