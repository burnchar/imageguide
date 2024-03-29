// This class is in heavy development. Comments/documentation to come.
// Some of this code is based on an article in DDJ by Anton Kruger.
// Any bugs are likely mine, not his.

#include "colorreducer.h"

using std::vector;

static const uint16 histogramSize = 32768;
int ColorReducer::longestDimension;

ColorReducer::ColorReducer()
{
	numCubes = 0;
	myCube.count = 0;
	myCube.lower = 0;
	myCube.level = 0;

	histogram.resize(histogramSize);
	colorTable.reserve(histogramSize / 2);
}


ColorReducer::ColorReducer(uint32 imageBytes[], uint32 pixelcount)
{
	numCubes = 0;
	myCube.count = 0;
	myCube.lower = 0;
	myCube.level = 0;
	histogram.resize(histogramSize);
	colorTable.reserve(histogramSize / 2);
	openImage(imageBytes, pixelcount);
}


void ColorReducer::openImage(uint32 imageBytes[], uint32 pixelCount)
{
	uint32 color;
	for(uint32 pixel = 0; pixel < pixelCount; ++pixel) {
		color = imageBytes[pixel];
		uint8 r = red8(color);
		uint8 g = grn8(color);
		uint8 b = blu8(color);

		uint16 xrgb = rgb555(r,g,b); // Convert 32-bit RGBA8888 to 15-bit RGB555
		++this->histogram[xrgb];
	}
}


/* MAIN LOOP
   For each cube in the array of cubes, find the cube of the lowest level
   and split it. If a cube has only one color, it can't be split.
   TODO: Figure out a smarter way to choose which cube to split other than
	   just whichever has the lowest level, unless this turns out to be best.
	   Perhaps base it on which has the greatest color range, including human
	   perception of the primary colors?
*/
int ColorReducer::reduceColors(vector<uint32> &colorMap, uint16 numColors)
{
	buildColorTable();
	myCube.upper = colorTable.size() - 1;
	trim(myCube); // Scan through colors to set max/min of each component
	cubeList[numCubes] = myCube;	// Add the initial cube to the maxColors-size array of cubes
	numCubes ++;

	// There's got to be a better way to do this!
	// TODO: Change all these uint8's to ints where appropriate for performance
	while(numCubes < numColors) { // Until we have the desired number of cubes
		int splitPos = getNextSplitPos();
		if(splitPos == -1) break; // No more cubes to split! TODO: Fix this evil 2nd exit condition
		findLongestColorDimension(splitPos);

		int median, pixelCount; // Written to by getMedianColor()
		getMedianColor(median, pixelCount);

		// Now split "Cube" at median. Then add two new cubes to list of cubes.
		splitCube(median, pixelCount, splitPos);
	}

	averageCubeColors(histogram, colorMap, numCubes);
	return numCubes;
}

void ColorReducer::findLongestColorDimension(int splitPos)
{
	// Find the longest axis of this cube
	myCube = cubeList[splitPos];
	int lenRed = myCube.redMax - myCube.redMin;
	int lenGrn = myCube.grnMax - myCube.grnMin;
	int lenBlu = myCube.bluMax- myCube.bluMin;
	if     (lenRed >= lenGrn && lenRed >= lenBlu) longestDimension = 0;
	else if(lenGrn >= lenRed && lenGrn >= lenBlu) longestDimension = 1;
	else if(lenBlu >= lenRed && lenBlu >= lenGrn) longestDimension = 2;
}


void ColorReducer::averageCubeColors(
		vector<uint16> &histogram, std::vector<uint32> &colorMap, uint16 numColors)
{
	// for each cube in list of cubes, computes centroid (average value) of
	// colors enclosed by that cube, and loads centroids in the color map. Next
	// loads histogram with indices into the color map.
	uint8      r, g, b;
	uint16     i, j, k, index, color;
	uint32       redSum, grnSum, bluSum; // TODO: Get rid of floats
	float       dr, dg, db, d, dmin;
	Cube   myCube;

	for(k = 0; k <= numColors -1; k++) {
		myCube = cubeList[k];
		redSum = grnSum = bluSum = 0;
		for(i = myCube.lower; i <= myCube.upper; i++) {
			color = colorTable[i];
			r = red5(color);
			redSum += r * histogram[color];
			g = grn5(color);
			grnSum += g*histogram[color];
			b = blu5(color);
			bluSum += b*histogram[color];
		}
		// Update the color map
		colorMap[k] = rgb888(
				(uint8)(redSum / myCube.count),
				(uint8)(grnSum / myCube.count),
				(uint8)(bluSum / myCube.count));

//		colorMap[k][1] = (uint8)(redSum / (float)myCube.count);
//		colorMap[k][2] = (uint8)(grnSum / (float)myCube.count);
//		colorMap[k][3] = (uint8)(bluSum / (float)myCube.count);
	}
	// Fast remap: foreach color in each cube, load the corresponding slot
	// in "Hist" with the centroid of the cube. */
	for(k = 0; k <= numColors - 1; k++) {
		myCube = cubeList[k];
		for(i = myCube.lower; i <= myCube.upper; i++) {
			color = colorTable[i];
			histogram[color] = k;
		}
	}

	return;
}


void ColorReducer::trim(Cube &fatCube)
{
	// Encloses "Cube" with a tight-fitting cube by updating (redMin, grnMin, bluMin)
	// and (redMax, grnMax, bluMax) members of "Cube". */
	uint8        r, g, b;
	uint16        i, color;

	fatCube.redMin = 255; fatCube.redMax = 0;
	fatCube.grnMin = 255; fatCube.grnMax = 0;
	fatCube.bluMin = 255; fatCube.bluMax = 0;

	// For each color (not pixel: color) in this cube, get the red, grn, and
	// blu components, find the lowest and highest of each, and assign them
	// to redMin, redMax, etc.
	for(i = fatCube.lower; i <= fatCube.upper; ++i) { // For each color in this cube
//		qDebug() << "Made it to line: " << __LINE__;
//		qDebug() << "myCube.upper:" << myCube.upper << "colorTable.size:" << colorTable.size() << "while" << i << "<=" << fatCube.upper;
		color = colorTable[i];
//		qDebug() << "Then line: " << __LINE__;
		r = red5(color);
		if(r > fatCube.redMax) fatCube.redMax = r;
		if(r < fatCube.redMin) fatCube.redMin = r;
		g = grn5(color);
		if(g > fatCube.grnMax) fatCube.grnMax = g;
		if(g < fatCube.grnMin) fatCube.grnMin = g;
		b = blu5(color);
		if(b > fatCube.bluMax) fatCube.bluMax = b;
		if(b < fatCube.bluMin) fatCube.bluMin = b;
	}
}


bool ColorReducer::compareRgb16Component(uint16 left, uint16 right)
{
	bool retval = false;

	switch(longestDimension) {
	case 0:
		retval = red5(left) < red5(right);
		break;
	case 1:
		retval = grn5(left) < grn5(right);
		break;
	case 2:
		retval = blu5(left) < blu5(right);
		break;
	}
	return retval;
}


void ColorReducer::splitCube(int median, int pixelCount, int splitPos)
{
	myCubeA = myCube; myCubeA.upper = median - 1;
	myCubeA.count = pixelCount; // We've counted up half the pixels in the old cube
	myCubeA.level = myCube.level + 1;
	trim(myCubeA);
	cubeList[splitPos] = myCubeA;               // add in old slot */

	myCubeB = myCube; myCubeB.lower = median;
	myCubeB.count = myCube.count - pixelCount;
	myCubeB.level = myCube.level + 1;
	trim(myCubeB);
	cubeList[numCubes++] = myCubeB;               // add in new slot */
}


void ColorReducer::buildColorTable()
{
	for(uint16 histEntry = 0; histEntry < histogramSize; ++histEntry) { // For each entry in the histogram
		if(0 != histogram[histEntry]) {				// If that entry is actually used in the image
			colorTable.push_back(histEntry);
			myCube.count += histogram[histEntry];	// Count of # pixels this cube has
		}
	}
}

int ColorReducer::getNextSplitPos()
{
	int splitPos = -1;
	uint8 lowestLevel = 255;
	// For each cube in the list of cubes
	for(int currentCube = 0; currentCube <= numCubes -1; ++currentCube) {
		// If this cube has more than one color
		if(cubeList[currentCube].upper != cubeList[currentCube].lower) {
			// If this cube's level is lower than the lowest level found so far, note it
			if(cubeList[currentCube].level < lowestLevel) {
				lowestLevel = cubeList[currentCube].level;
				splitPos = currentCube; // Until we know otherwise, THIS is the cube to split
			}
		}
	}
	return splitPos;
}


void ColorReducer::getMedianColor(int &median, int &pixelCount)
{
	int midarray = (myCube.lower + myCube.upper) / 2;

	/* PROBABLE BUG */
	// Since this only positions values up to midarray, if the actual median
	// doesn't land at midarray, it may be the case that a few colors are
	// basically random (though close, since they must be > midarray.
	// Strangely, images saved using nth_element consistently look better
	// than images which use a full sort (theoretically ideal),
	// so I am keeping nth_element despite this theoretical bug.
	// TODO: Do another nth_element on the elements from midarray to median
	std::nth_element(colorTable.begin() + myCube.lower, // Actually better, actually faster
					 colorTable.begin() + midarray,
					 colorTable.begin() + myCube.upper + 1,
					 compareRgb16Component);

//		std::sort(colorTable.begin() + myCube.lower, // Theoretically better, actually slower
//				  colorTable.begin() + myCube.upper + 1,
//				  compareRgb16Component);

	int c, myColor;
	pixelCount = 0; // Count of the number of pixels in the cube processed so far
	for(c = myCube.lower; c <= myCube.upper -1; c++) { // For each COLOR in the cube
		if(pixelCount >= myCube.count / 2) break; // Stop when we've considered half the pixels in cube
		myColor = colorTable[c];			// Get the color of the current entry
		pixelCount += histogram[myColor];	// Add the number of pixels of that color
	}										// Once we get half the total pixels in the cube, done
	median = c;
}
