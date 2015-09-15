// Set tabs to 4 spaces.
#ifndef WS2812_HEADER
#define WS2812_HEADER

// =================================================================================================
//
//		 __      __  _________________   ______  ____________   ____________________.__ 
//		/  \    /  \/   _____/\_____  \ /  __  \/_   \_____  \  \______   \______   \__|
//		\   \/\/   /\_____  \  /  ____/ >      < |   |/  ____/   |       _/|     ___/  |
//		 \        / /        \/       \/   --   \|   /       \   |    |   \|    |   |  |
//		  \__/\  / /_______  /\_______ \______  /|___\_______ \  |____|_  /|____|   |__|
//		       \/          \/         \/      \/             \/         \/              
//
// WS2812 NeoPixel driver
// Based on code by Richard G. Hirst and others
// Adapted for the WS2812 by 626Pilot, April/May 2014
// Huge ASCII art section labels are from http://patorjk.com/software/taag/
//
// License: GPL
//

/*
// =================================================================================================
//	.___              .__            .___             
//	|   | ____   ____ |  |  __ __  __| _/____   ______
//	|   |/    \_/ ___\|  | |  |  \/ __ |/ __ \ /  ___/
//	|   |   |  \  \___|  |_|  |  / /_/ \  ___/ \___ \ 
//	|___|___|  /\___  >____/____/\____ |\___  >____  >
//	         \/     \/                \/    \/     \/ 
// =================================================================================================
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <signal.h>
#include <sys/file.h>	// Used for single instance check



/*
// =================================================================================================
//	________          _____.__                         ____    ____   ____                    
//	\______ \   _____/ ____\__| ____   ____   ______  /  _ \   \   \ /   /____ _______  ______
//	 |    |  \_/ __ \   __\|  |/    \_/ __ \ /  ___/  >  _ </\  \   Y   /\__  \\_  __ \/  ___/
//	 |    `   \  ___/|  |  |  |   |  \  ___/ \___ \  /  <_\ \/   \     /  / __ \|  | \/\___ \ 
//	/_______  /\___  >__|  |__|___|  /\___  >____  > \_____\ \    \___/  (____  /__|  /____  >
//	        \/     \/              \/     \/     \/         \/                \/           \/ 
// =================================================================================================
*/

// Control Block (CB) - this tells the DMA controller what to do.
typedef struct {
	unsigned int
		info,		// Transfer Information (TI)
		src,		// Source address (physical)
		dst,		// Destination address (bus)
		length,		// Length in bytes (not words!)
		stride,		// We don't care about this
		next,		// Pointer to next control block
		pad[2];		// These are "reserved" (unused)
} dma_cb_t;

// The page map contains pointers to memory that we will allocate below. It uses two pointers
// per address. This is because the software (this program) deals only in virtual addresses,
// whereas the DMA controller can only access RAM via physical address. (If that's not confusing
// enough, it writes to peripherals by their bus addresses.)
typedef struct {
	uint8_t *virtaddr;
	uint32_t physaddr;
} page_map_t;




#define PAGE_SIZE	4096					// Size of a RAM page to be allocated
#define PAGE_SHIFT	12						// This is used for address translation
#define NUM_PAGES	((sizeof(struct control_data_s) + PAGE_SIZE - 1) >> PAGE_SHIFT)

#define SETBIT(word, bit) word |= 1<<bit
#define CLRBIT(word, bit) word &= ~(1<<bit)
#define GETBIT(word, bit) word & (1 << bit) ? 1 : 0
#define true 1
#define false 0

// GPIO
#define INP_GPIO(g) *(gpio_reg+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio_reg+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio_reg+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET *(gpio_reg+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio_reg+10) // clears bits which are 1 ignores bits which are 0


// =================================================================================================
//	  ________                                  .__   
//	 /  _____/  ____   ____   ________________  |  |  
//	/   \  ____/ __ \ /    \_/ __ \_  __ \__  \ |  |  
//	\    \_\  \  ___/|   |  \  ___/|  | \// __ \|  |__
//	 \______  /\___  >___|  /\___  >__|  (____  /____/
//	        \/     \/     \/     \/           \/      
// =================================================================================================

void printBinary(unsigned int i, unsigned int bits);
unsigned int reverseWord(unsigned int word);

void terminate(int dummy);

void fatal(char *fmt, ...);

// Memory management
// --------------------------------------------------------------------------------------------------
unsigned int mem_virt_to_phys(void *virt);
unsigned int mem_phys_to_virt(uint32_t phys);
void * map_peripheral(uint32_t base, uint32_t len);


/*
// =================================================================================================
//	.____     ___________________      _________ __          _____  _____ 
//	|    |    \_   _____/\______ \    /   _____//  |_ __ ___/ ____\/ ____\
//	|    |     |    __)_  |    |  \   \_____  \\   __\  |  \   __\\   __\ 
//	|    |___  |        \ |    `   \  /        \|  | |  |  /|  |   |  |   
//	|_______ \/_______  //_______  / /_______  /|__| |____/ |__|   |__|   
//	        \/        \/         \/          \/                           
// =================================================================================================
*/

// Brightness - I recommend 0.2 for direct viewing at 3.3v.
#define DEFAULT_BRIGHTNESS 1.0
extern float brightness;

// LED buffer (this will be translated into pulses in PWMWaveform[])
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Color_t;

// LED buffer (this will be translated into pulses in PWMWaveform[])
typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned int br;
} Color_tb;

extern unsigned int numLEDs;		// How many LEDs there are on the chain

#define LED_BUFFER_LENGTH 24
extern Color_t LEDBuffer[LED_BUFFER_LENGTH];

// Set brightness
unsigned char setBrightness(float b);

// Zero out the PWM waveform buffer
void clearPWMBuffer();

// Zero out the LED buffer
void clearLEDBuffer();

// Turn r, g, and b into a Color_t struct
Color_t RGB2Color(unsigned char r, unsigned char g, unsigned char b);

// Alias for the above
Color_t Color(unsigned char r, unsigned char g, unsigned char b);

// Set pixel color (24-bit color)
unsigned char setPixelColor(unsigned int pixel, unsigned char r, unsigned char g, unsigned char b);
unsigned char setPixelColor_B(unsigned int pixel, unsigned char r, unsigned char g, unsigned char b, float br);

// Set pixel color, by a direct Color_t
unsigned char setPixelColorT(unsigned int pixel, Color_t c);
unsigned char setPixelColorT_B(unsigned int pixel, Color_t c, float br);

// Get pixel color
Color_t getPixelColor(unsigned int pixel);

// Return # of pixels
unsigned int numPixels();

// Return pointer to pixels (FIXME: dunno if this works!)
Color_t* getPixels();

// Set an individual bit in the PWM output array, accounting for word boundaries
// The (31 - bitIdx) is so that we write the data backwards, correcting its endianness
// This means getPWMBit will return something other than what was written, so it would be nice
// if the logic that calls this function would figure it out instead. (However, that's trickier)
void setPWMBit(unsigned int bitPos, unsigned char bit);

// Get an individual bit from the PWM output array, accounting for word boundaries
unsigned char getPWMBit(unsigned int bitPos);


/*
// =================================================================================================
//	________        ___.
//	\______ \   ____\_ |__  __ __  ____  
//	 |    |  \_/ __ \| __ \|  |  \/ ___\ 
//	 |    `   \  ___/| \_\ \  |  / /_/  >
//	 /_______  /\___  >___  /____/\___  / 
//	         \/     \/    \/     /_____/  
// =================================================================================================
*/

// Dump contents of LED buffer
void dumpLEDBuffer();

// Dump contents of PWM waveform
// The last number dumped may not have a multiple of 3 digits (our basic unit of data is 3 bits,
// whereas the RAM comprising the buffer has to be a multiple of 2 bits in size)
void dumpPWMBuffer();

// Display the status of the PWM's control register
void dumpPWMStatus();

// Display the settings in a PWM control word
// If you want to dump the register directly, use this: dumpPWMControl(*(pwm + PWM_CTL));
void dumpPWMControl(unsigned int word);

// Display the settings in the PWM DMAC word
void dumpPWMDMAC();

// Display all PWM registers
void dumpPWM();

// Display all PWM control registers
void dumpDMARegs();

// Display the contents of a Control Block
void dumpControlBlock(dma_cb_t *c);

// Display the contents of a Transfer Information word
void dumpTransferInformation(unsigned int TI);

// Display the readable DMA registers
void dumpDMA();


/*
// =================================================================================================
//	.___       .__  __      ___ ___                  .___                              
//	|   | ____ |__|/  |_   /   |   \_____ _______  __| _/_  _  _______ _______   ____  
//	|   |/    \|  \   __\ /    ~    \__  \\_  __ \/ __ |\ \/ \/ /\__  \\_  __ \_/ __ \ 
//	|   |   |  \  ||  |   \    Y    // __ \|  | \/ /_/ | \     /  / __ \|  | \/\  ___/ 
//	|___|___|  /__||__|    \___|_  /(____  /__|  \____ |  \/\_/  (____  /__|    \___  >
//	         \/                  \/      \/           \/              \/            \/ 
// =================================================================================================
*/

void initHardware();

// Begin the transfer
void startTransfer();



/*
// =================================================================================================
//	  ____ ___            .___       __           .____     ___________________          
//	 |    |   \______   __| _/____ _/  |_  ____   |    |    \_   _____/\______ \   ______
//	 |    |   /\____ \ / __ |\__  \\   __\/ __ \  |    |     |    __)_  |    |  \ /  ___/
//	 |    |  / |  |_> > /_/ | / __ \|  | \  ___/  |    |___  |        \ |    `   \\___ \ 
//	 |______/  |   __/\____ |(____  /__|  \___  > |_______ \/_______  //_______  /____  >
//	           |__|        \/     \/          \/          \/        \/         \/     \/ 
// =================================================================================================
*/

void show();

/*
// =================================================================================================
//	___________ _____  _____              __          
//	\_   _____// ____\/ ____\____   _____/  |_  ______
//	 |    __)_\   __\\   __\/ __ \_/ ___\   __\/  ___/
//	 |        \|  |   |  | \  ___/\  \___|  |  \___ \ 
//	/_______  /|__|   |__|  \___  >\___  >__| /____  >
//	        \/                  \/     \/          \/ 
// =================================================================================================
// The effects in this section are adapted from the Adafruit NeoPixel library at:
// https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/strandtest/strandtest.ino
*/

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
Color_t Wheel(uint8_t WheelPos); 

// Fill the dots one after the other with a color
void colorWipe(Color_t c, uint8_t wait);

// Rainbow
void rainbow(uint8_t wait);

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait);

//Theatre-style crawling lights.
void theaterChase(Color_t c, uint8_t wait);

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait);

#endif
