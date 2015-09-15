
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
void colorWipe(Color_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
void theaterChase(Color_t c, uint8_t wait);
void theaterChaseRainbow(uint8_t wait);
void rainbowCycle_r(uint8_t wait);
void rainbowCycle_wipe(uint8_t wait);
void RainFall(Color_t c,uint8_t wait,int sleepafter);
void Twinkle();

float twinklearray[NUM_PIXELS];

Color_t Wheel(uint8_t WheelPos) {
	if(WheelPos < 85) {
		return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	} else if(WheelPos < 170) {
		WheelPos -= 85;
		return Color(255 - WheelPos * 3, 0, WheelPos * 3);
	} else {
		WheelPos -= 170;
		return Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}


// Fill the dots one after the other with a color
void colorWipe(Color_t c, uint8_t wait) {
	uint16_t i;
	for(i=0; i<numPixels(); i++) {
		setPixelColorT(i, c);
		show();
		usleep(wait * 1000);
	}
}

// Fill the dots one after the other with a color
// Same as above - just the other way around the chain
void colorWipe_r(Color_t c, uint8_t wait) {
	int16_t i;
	for(i=numPixels()-1; i >= 0; i--) {
		setPixelColorT(i, c);
		show();
		usleep(wait * 1000);
	}
}

// Rainbow
void rainbow(uint8_t wait) {
	uint16_t i, j;

	for(j=0; j<256; j++) {
		for(i=0; i<numPixels(); i++) {
			setPixelColorT(i, Wheel((i+j) & 255));
		}
		show();
		usleep(wait * 1000);
	}
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
	uint16_t i, j;
	j = 0;
	for(i=0; i<numPixels(); i++) {
		setPixelColorT(i, Wheel(((i * 256 / numPixels()) + j) & 255));
		show();
		usleep(50000);
	}
	for(j = 0; j<256*5; j++) { // 5 cycles of all colors on wheel
		for(i=0; i<numPixels(); i++) {
			setPixelColorT(i, Wheel(((i * 256 / numPixels()) + j) & 255));
		}
		show();
		usleep(wait * 1000);
	}
}

// Slightly different, this makes the rainbow equally distributed throughout
// Same as above - just the other way around the chain
void rainbowCycle_r(uint8_t wait) {
	uint16_t i, j;

	for(j=256*5; j>0; j--) { // 5 cycles of all colors on wheel
		for(i=0; i<numPixels(); i++) {
			setPixelColorT(i, Wheel(((i * 256 / numPixels()) + j) & 255));
		}
		show();
		usleep(wait * 1000);
	}
}

// Slightly different, this makes the rainbow equally distributed throughout
// Same as rainbowCycle() but does a colour wipe with a rainbow effect before starting the Cycle
void rainbowCycle_wipe(uint8_t wait) {
	uint16_t i, j;
	j = 0;
	for(i=numPixels(); i>0; i--) {
		setPixelColorT(i, Wheel(((i * 256 / numPixels()) + j) & 255));
		show();
		usleep(50000);
	}
	// for(j; j<256*5; j++) { // 5 cycles of all colors on wheel
	// 	for(i=0; i<numPixels(); i++) {
	// 		setPixelColorT(i, Wheel(((i * 256 / numPixels()) + j) & 255));
	// 	}
	// 	show();
	// 	usleep(wait * 1000);
	// }
}


//Theatre-style crawling lights.
void theaterChase(Color_t c, uint8_t wait) {
	unsigned int j, q, i;
	for (j=0; j<15; j++) {  //do this many cycles of chasing
		for (q=0; q < 3; q++) {
			for (i=0; i < numPixels(); i=i+3) {
				setPixelColorT(i+q, c);			// Turn every third pixel on
			}
			show();
     
			usleep(wait * 1000);

			for (i=0; i < numPixels(); i=i+3) {
				setPixelColor(i+q, 0, 0, 0);	// Turn every third pixel off
			}
		}
	}
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
	int j, q, i;
	for (j=0; j < 256; j+=4) {     // cycle through every 4th color on the wheel
		for (q=0; q < 3; q++) {
			for (i=0; i < numPixels(); i=i+3) {
				setPixelColorT(i+q, Wheel((i+j) % 255));    //turn every third pixel on
			}
			show();

			usleep(wait * 1000);
       
			for (i=0; i < numPixels(); i=i+3) {
				setPixelColor(i+q, 0, 0, 0);        //turn every third pixel off
			}
		}
	}
}

// "Rain Fall" effect - Test
void RainFall(Color_t c,uint8_t wait,int sleepafter) {
	int i, j, k;
	j = (numPixels() / 2) - 1;
	k = j + 1;
	for (j,k; j >= 0; j--,k++) {
		//printf("Pixel - %i %i\n", j, k);
		setPixelColorT(j, c);
		setPixelColorT(k, c);
		show();
		usleep(wait * 10000);
	}
	sleep(sleepafter);
	//printf("Done:\n");
}

void Twinkle() {
	int j;
	for (j=0;j<numPixels();j++) {

		if (twinklearray[j] == 0) { // LED is off - Give it a chance to turn on.
			if ((rand() % 20) == 1) { // LED Got the change to be turned on now lets give it a random brightness level
				twinklearray[j] = (float) rand()/RAND_MAX;
				setPixelColor_B(j, 255, 255, 255,twinklearray[j]);
			}
		}
		else if (twinklearray[j] < 0.000) {
			twinklearray[j] = 0;
			setPixelColor(j, 0, 0, 0);
		}
		else if (twinklearray[j] > 0.000) {
			twinklearray[j] = twinklearray[j] - 0.1;
			if (twinklearray[j]<0) twinklearray[j] = 0;
			setPixelColor_B(j, 255, 255, 255,twinklearray[j]);
		}
	}
	show();
	usleep(100000);
}

void Twinkle_Fade() {
	int j,i;
	for (i=0;i<15;i++) {
		for (j=0;j<numPixels();j++) {
			if (twinklearray[j] < 0.000) {
				twinklearray[j] = 0;
				setPixelColor(j, 0, 0, 0);
			}
			else if (twinklearray[j] > 0.000) {
				twinklearray[j] = twinklearray[j] - 0.1;
				if (twinklearray[j]<0) twinklearray[j] = 0;
				setPixelColor_B(j, 255, 255, 255,twinklearray[j]);
			}
		}
		show();
		usleep(100000);
	}
}

void Twinkle_T(Color_t c) {
	int j;
	for (j=0;j<numPixels();j++) {

		if (twinklearray[j] == 0) { // LED is off - Give it a chance to turn on.
			if ((rand() % 15) == 1) { // LED Got the change to be turned on now lets give it a random brightness level
				twinklearray[j] = (float) rand()/RAND_MAX;
				setPixelColorT_B(j, c, twinklearray[j]);
			}
		}
		else if (twinklearray[j] < 0.000) {
			twinklearray[j] = 0;
			setPixelColor(j, 0, 0, 0);
		}
		else if (twinklearray[j] > 0.000) {
			twinklearray[j] = twinklearray[j] - 0.1;
			if (twinklearray[j]<0) twinklearray[j] = 0;
			setPixelColorT_B(j, c, twinklearray[j]);
		}
	}
	show();
	// usleep(100000);
	usleep(1000000);
}
