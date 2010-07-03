#ifndef COLORQUANTIZER_H
#define COLORQUANTIZER_H

#include <algorithm>
#include <vector>
#include <QRgb>
#include <QDebug>

/*
Notes: Anton Kruger's code has a bug in the compare function which does
	not properly sort colors by green. This doesn't seem to have made
	a lot of difference. More experimentation is necessary.
	(Perhaps the sort doesn't matter much?)

*/

/* Macros for converting between (r,g,b)-colors and 15-bit     */
/* colors follow.                                              */
// COMMENTED OUT to avoid redefinition
#define RGB2(r,g,b) (quint16)(((b)&~7)<<7)|(((g)&~7)<<2)|((r)>>3)
#define RED2(x)     (quint8)(((x)&31)<<3)
#define GREEN2(x)   (quint8)((((x)>>5)&255)<< 3)
#define BLUE2(x)    (quint8)((((x)>>10)&255)<< 3)


class cube_type {			// structure for a cube in color space
public:
	quint16 lower;	// one corner's index in histogram
	quint16 upper;	// another corner's index in histogram
	quint32 count;	// cube's histogram count
	quint8 level;	// cube's level

	quint8 rmin, rmax;
	quint8 gmin, gmax;
	quint8 bmin, bmax;
};


static int longestDimension; // Make a class-scope variable
static quint16 nCubes;
static const quint16 histSize = 32768;
static const quint16 maxColors = 256; // Should always be 256. Actual colors is passed to the function. TODO: Make this dynamic later.
static cube_type cubeList[maxColors];   /* list of cubes              */ // TODO: Make into vector
static std::vector<quint16> myHistPtr(histSize, 0);      /* points to colors in "Hist" */



void myShrink(cube_type * Cube);
bool compareColor16Component(quint16 lhs, quint16 rhs);
void myInvMap(std::vector<quint16> &histogram, quint8 colorMap[][4], quint16 nCubes);
quint16 myMedianCut(std::vector<quint16> &histogram, quint8 colorMap[][4], quint16 maxCubes);



#endif // COLORQUANTIZER_H


