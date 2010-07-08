#ifndef COLORREDUCER_H
#define COLORREDUCER_H

/*
TODO: If the original PNG is smaller than the new one, like with the 24-bit PNG
	of all colors, recommend to keep old one.
	Similarly, if the SSIM of the old image is much better than the new and
	not much larger, recommend to keep it.
*/

#include <vector>
#include <algorithm>
#include <QDebug>

typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned int	uint32;

struct Cube {		// 3D "cube", with red, grn, and blu as the dimensions
public:
	uint16 lower;	// This cubes lowest index into the histogram
	uint16 upper;	// This cubes highest index into the histogram
	uint32 count;	// Number of PIXELS this cube currently contains
	uint8 level;	// This cube's level, or how many previous splits were done

	uint8 redMin;	// Lowest red value in this cube (0-255)
	uint8 redMax;	// Highest red value (0-255)
	uint8 grnMin;
	uint8 grnMax;
	uint8 bluMin;
	uint8 bluMax;
};


class ColorReducer
{
public:
	ColorReducer();
	ColorReducer(uint32 imageBytes[], uint32 pixelcount);
	void openImage(uint32 imageBytes[], uint32 pixelCount);
	int reduceColors(std::vector<uint32> &colorMap, uint16 numColors);

private:
	static int longestDimension;
	static const uint16 histogramSize = 32768;
	static const uint16 maxColors = 256;
	std::vector<uint16> histogram;
	std::vector<uint16> colorTable;
	uint16 numCubes;
	Cube cubeList[maxColors];
	Cube myCube, myCubeA, myCubeB;

	void trim(Cube &fatCube);
	void averageCubeColors(std::vector<uint16> &histogram,
						   std::vector<uint32> &colorMap, uint16 maxCubes);
	void findLongestColorDimension(int splitPos);
	void splitCube(int median, int pixelCount, int splitPos);
	void buildColorTable();
	void getMedianColor(int &median, int &pixelCount);
	int getNextSplitPos();
	static bool compareRgb16Component(uint16 left, uint16 right);

	static uint8 red5(uint16 rgb555)   { return ( rgb555 &  0x1F)      << 3; }
	static uint8 grn5(uint16 rgb555) { return ((rgb555 >> 5 ) & 0xFF)  << 3; }
	static uint8 blu5(uint16 rgb555)  { return ((rgb555 >> 10) & 0xFF) << 3; }
	static uint16 rgb555(uint8 r, uint8 g, uint8 b)
		{ return ((b & ~7) << 7) | ((g & ~7) << 2) | (r >> 3); }

	static uint32 red8(uint32 rgb888)  { return  rgb888 >> 16   & 0xFF;      }
	static uint32 grn8(uint32 rgb888){ return  rgb888 >> 8    & 0xFF;        }
	static uint32 blu8(uint32 rgb888) { return  rgb888         & 0xFF;       }
	static uint32 rgb888(int r, int g, int b) { return (0xFFu << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF); }
};

#endif // COLORREDUCER_H
