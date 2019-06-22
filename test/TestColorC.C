#include <iostream>
#include <cpgplot.h>

constexpr unsigned int size = 100;
constexpr unsigned int tsize = size * size;

int main(int ac, char * av[]) {
		float val = 0.0f;
		if(ac < 2) {
				std::cerr << "Usage: " << av[0] << " VALUE " << std::endl;
				return 0;
		}
		val = std::stof(av[1]);
		// data 
		float arr[tsize];
		std::fill(arr, arr + tsize, val);
		// colorscheme
		float contrast = 1.0;
		float brightness = 0.5;
		int csize = 0;
		// there's black in the middle
		//csize = 7;
		//float light[] = {0.0f, 0.17f, 0.33f, 0.5f, 0.68f, 0.84f, 1.0f};
		//float red[]   = {0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f};
		//float green[] = {0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f};
		//float blue[]  = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.3f, 1.0f};
		// to alleviate the black
		csize = 6;
		float light[] = {0.0f, 0.2f, 0.4f, 0.6f, 0.8f, 1.0f};
		float red[]   = {0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f};
		float green[] = {0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 1.0f};
		float blue[]  = {0.0f, 0.5f, 0.0f, 0.0f, 0.3f, 1.0f};
		// plot matrix
		float tr[] = {0.0f, 1.0f/size, 0.0f, 0.0f, 0.0f, 1.0f/size};
		tr[0] -= 0.5 * tr[1];
		tr[3] -= 0.5 * tr[5];
		// pgplot routines
		cpgbeg(0, "/tmp/test.png/png", 1, 1);
		cpgask(0);
		cpgsvp(0.3,0.9,0.1,0.9);
		cpgswin(0,1,0,1);
		cpglab("X", "Y", "Color Calibration");
		cpgbox("BCN",0.0,0,"BCNV",0.0,0);
		cpgctab (light, red, green, blue, csize, contrast, brightness);
		cpgimag(arr, size, size, 1, size, 1, size, -1 , 5 , tr);
		cpgwedg("BI", 2, 3, -1, 5, "Jy");
		cpgend();
		return 0;
}

