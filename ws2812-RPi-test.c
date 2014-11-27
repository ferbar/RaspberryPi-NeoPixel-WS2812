#include "ws2812-RPi.c"
#include "ws2812-RPi-effects.c"

void effectsDemo();

int main(int argc, char **argv) { 
	// Check "Single Instance"
	int pid_file = open("/var/run/ws2812RPi.pid", O_CREAT | O_RDWR, 0666);
	int rc = flock(pid_file, LOCK_EX | LOCK_NB);
	if(rc) {
	    if(EWOULDBLOCK == errno)
	    {
	        // another instance is running
	        printf("Instance already running\n");
	        exit(EXIT_FAILURE);
	    }
	}

	// Catch all signals possible - it's vital we kill the DMA engine on process exit!
	int i;
	for (i = 0; i < 64; i++) {
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = terminate;
		sigaction(i, &sa, NULL);
	}

	// Don't buffer console output
	setvbuf(stdout, NULL, _IONBF, 0);

	// How many LEDs?
	numLEDs = NUM_PIXELS;

	// How bright? (Recommend 0.2 for direct viewing @ 3.3V)
	setBrightness(DEFAULT_BRIGHTNESS);

	// Init PWM generator and clear LED buffer
	initHardware();
	clearLEDBuffer();

	// Show some effects
	while(true) {
		effectsDemo();
	}

	// Exit cleanly, freeing memory and stopping the DMA & PWM engines
	// We trap all signals (including Ctrl+C), so even if you don't get here, it terminates correctly
	terminate(0);

	return 0;
}

void effectsDemo() {

	int i, j, ptr;
	float k;

	// Default effects from the Arduino lib
	colorWipe(Color(255, 0, 0), 50); // Red
	colorWipe(Color(0, 255, 0), 50); // Green
	colorWipe(Color(0, 0, 255), 50); // Blue
	theaterChase(Color(127, 127, 127), 50); // White
	theaterChase(Color(127,   0,   0), 50); // Red
	theaterChase(Color(  0,   0, 127), 50); // Blue
	rainbow(5);
	rainbowCycle(5);
	theaterChaseRainbow(50);

	// Watermelon fade :)
	for(k=0; k<0.5; k+=.01) {
		ptr=0;
		setBrightness(k);
		for(i=0; i<numLEDs; i++) {
			setPixelColor(i, i*5, 64, i*2);
		}
		show();
	}
	for(k=0.5; k>=0; k-=.01) {
		ptr=0;
		setBrightness(k);
		for(i=0; i<numLEDs; i++) {
			setPixelColor(i, i*5, 64, i*2);
		}
		show();
	}
	usleep(1000);

	// Random color fade
	srand(time(NULL));
	uint8_t lastRed = 0;
	uint8_t lastGreen = 0;
	uint8_t lastBlue = 0;
	uint8_t red, green, blue;
	Color_t curPixel;
	setBrightness(DEFAULT_BRIGHTNESS);
	for(j=1; j<16; j++) {
		ptr = 0;
		if(j % 3) {
			red = 120;
			green = 64;
			blue = 48;
		} else if(j % 7) {
			red = 255;
			green = 255;
			blue = 255;
		} else {
			red = rand();
			green = rand();
			blue = rand();
		}
		for(k=0; k<1; k+=.01) {
			for(i=0; i<numLEDs; i++) {
				setPixelColor(
					i,
					(red * k) + (lastRed * (1-k)),
					i * (255 / numLEDs), //(green * k) + (lastGreen * (1-k)),
					(blue * k) + (lastBlue * (1-k))
					);
				curPixel = getPixelColor(i);
			}
			show();
		}
		lastRed = red;
		lastGreen = green;
		lastBlue = blue;
	}
}