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

struct Cube {		// 3D "cube", with red, green, and blue as the dimensions
public:
	uint16 lower;	// This cubes lowest index into the histogram
	uint16 upper;	// This cubes highest index into the histogram
	uint32 count;	// Number of PIXELS this cube currently contains
	uint8 level;	// This cube's level, or how many previous splits were done

	uint8 redMin;	// Lowest red value in this cube (0-255)
	uint8 redMax;	// Highest red value (0-255)
	uint8 greenMin;
	uint8 greenMax;
	uint8 blueMin;
	uint8 blueMax;
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
	std::vector<uint16> colorTable;	// Formerly histPtr
	uint16 numCubes;
	Cube cubeList[maxColors];
	Cube myCube, myCubeA, myCubeB;

	void trim(Cube &fatCube);			// trim: formerly Shrink
	void averageCubeColors(std::vector<uint16> &histogram, std::vector<uint32> &colorMap, uint16 maxCubes); 	// formerly InvMap
	void findLongestColorDimension(int splitPos);
	void splitCube(int median, int pixelCount, int splitPos);
	void buildColorTable();
	void getMedianColor(int &median, int &pixelCount);
	int getNextSplitPos();
	static bool compareRgb16Component(uint16 left, uint16 right);

	static uint8 red5(uint16 rgb555)   { return ( rgb555 &  0x1F)       << 3; }
	static uint8 green5(uint16 rgb555) { return ((rgb555 >> 5 ) & 0xFF) << 3; }
	static uint8 blue5(uint16 rgb555)  { return ((rgb555 >> 10) & 0xFF) << 3; }
	static uint16 rgb555(uint8 r, uint8 g, uint8 b) { return ((b & ~7)  << 7) | ((g & ~7) << 2) | (r >> 3); }

	static uint32 red8(uint32 rgb888)  { return  rgb888 >> 16   & 0xFF;       }
	static uint32 green8(uint32 rgb888){ return  rgb888 >> 8    & 0xFF;       }
	static uint32 blue8(uint32 rgb888) { return  rgb888         & 0xFF;       }
	// TODO: Should these be ints or uint8's?
	static uint32 rgb888(int r, int g, int b) { return (0xFFu << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF); }
};

//const QRgb  RGB_MASK    = 0x00ffffff;                // masks RGB values

//Q_GUI_EXPORT_INLINE int qRed(QRgb rgb)                // get red part of RGB
//{ return ((rgb >> 16) & 0xff); }

//Q_GUI_EXPORT_INLINE int qGreen(QRgb rgb)                // get green part of RGB
//{ return ((rgb >> 8) & 0xff); }

//Q_GUI_EXPORT_INLINE int qBlue(QRgb rgb)                // get blue part of RGB
//{ return (rgb & 0xff); }

//Q_GUI_EXPORT_INLINE int qAlpha(QRgb rgb)                // get alpha part of RGBA
//{ return ((rgb >> 24) & 0xff); }

//Q_GUI_EXPORT_INLINE QRgb qRgb(int r, int g, int b)// set RGB value
//{ return (0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

//Q_GUI_EXPORT_INLINE QRgb qRgba(int r, int g, int b, int a)// set RGBA value
//{ return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }

//static int longestDimension; // Make a class-scope variable
//static uint16 nCubes;
//static const uint16 histSize = 32768;
//static const uint16 maxColors = 256; // Should always be 256. Actual colors is passed to the function. TODO: Make this dynamic later.
//static cube_type cubeList[maxColors];   /* list of cubes  */ // TODO: Make into vector
//static std::vector<uint16> myHistPtr(histSize, 0);      /* points to colors in "Hist" */

//void myShrink(cube_type * Cube);
//bool compareColor16Component(uint16 lhs, uint16 rhs);
//void myInvMap(std::vector<uint16> &histogram, uint8 colorMap[][4], uint16 nCubes);
//uint16 myMedianCut(std::vector<uint16> &histogram, uint8 colorMap[][4], uint16 maxCubes);

#endif // COLORREDUCER_H
