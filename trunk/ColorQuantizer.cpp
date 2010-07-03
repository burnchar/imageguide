#include "ColorQuantizer.h"

quint16 myMedianCut(std::vector<quint16> &histogram, quint8 colorMap[][4], quint16 maxCubes)
{
	qDebug() << "I made it to line" << __LINE__;
	// Create initial cube
	cube_type Cube;
	quint16 myColor = 0;
	nCubes = 0;
	Cube.count = 0;
	// TODO: Convert to a for_each()
	for(quint16 histEntry = 0; histEntry < histSize; ++histEntry) { // For each entry in the histogram
		if(0 != histogram[histEntry]) {			// If that entry is actually used in the image
			myHistPtr[myColor] = histEntry;		// Since this color is used, append it to myHistPtr
			++myColor;							// This variable is a count for total colors used
			Cube.count += histogram[histEntry]; // Count of # pixels this cube has
		}
	}

	Cube.lower = 0; // TODO: Put all this in the constructor
	Cube.upper = myColor -1;
	Cube.level = 0;

	myShrink(&Cube); // Scan through colors to set max/min of each component

	cubeList[nCubes++] = Cube;	// Add the initial cube to the maxColors-size array of cubes

	/* MAIN LOOP
	   For each cube in the array of cubes, find the cube of the lowest level
	   and split it. If a cube has only one color, it can't be split.
	   TODO: Figure out a smarter way to choose which cube to split other than
		   just whichever has the lowest level, unless this turns out to be best.
*/
	// There's got to be a better way to do this!
	// TODO: Change all these quint8's to ints where appropriate for performance
	int lowestLevel, splitPos;

	while(nCubes < maxCubes) { // Until we have the desired number of cubes
		lowestLevel = 255; splitPos = -1; // rename this to "SplitMe" or "CubeToSplit"
		// For each cube in the list of cubes
		for(int currentCube = 0; currentCube <= nCubes -1; ++currentCube) {
			// If this cube has more than one color
			if(cubeList[currentCube].upper != cubeList[currentCube].lower) {
				// If this cube's level is lower than the lowest level found so far, note it
				if(cubeList[currentCube].level < lowestLevel) {
					lowestLevel = cubeList[currentCube].level;
					splitPos = currentCube; // Until we know otherwise, THIS is the cube to split
				}
			}
		}
		if(splitPos == -1) {
			qDebug() << "splitPos = -1, no more cubes to split!";
			break; // No more cubes to split! TODO: Fix this evil 2nd exit condition
		}

		// Find the longest axis of this cube
		Cube = cubeList[splitPos];
		int lenRed = Cube.rmax - Cube.rmin;
		int lenGrn = Cube.gmax - Cube.gmin;
		int lenBlu = Cube.bmax - Cube.bmin;
		if     (lenRed >= lenGrn && lenRed >= lenBlu) longestDimension = 0;
		else if(lenGrn >= lenRed && lenGrn >= lenBlu) longestDimension = 1;
		else if(lenBlu >= lenRed && lenBlu >= lenGrn) longestDimension = 2;

		//!		qDebug() << "longestDimension" << longestDimension;

		// DEBUG: Put this in the existing code to compare results. Print myHistPtr
		// Find median of the cube sorting by the color with the longest dimension

		int midarray = (Cube.lower + Cube.upper) / 2;

		// Since this only positions values up to midarray, if the actual median
		// goes past that, it may be the case that the colors are wrong.
		//qDebug() << "Cube.lower:" << Cube.lower << "Cube.upper:" << Cube.upper << "midarray:" << midarray << "myHistPtr.size:" << myHistPtr.size();
		std::nth_element(
				myHistPtr.begin() + Cube.lower,
				myHistPtr.begin() + midarray,
				myHistPtr.begin() + Cube.upper, // Do I need a +1 here?
				compareColor16Component);

		int c, mycount = 0; // Count of the number of pixels in the cube processed so far
		for(c = Cube.lower; c <= Cube.upper -1; c++) { // For each COLOR in the cube
			if(mycount >= Cube.count / 2) break; // Stop when we've considered half the pixels in cube
			myColor = myHistPtr[c];     // Get the color of the current entry
			mycount += histogram[myColor]; // Add the number of pixels of that color
		}                           // Once we get half the total pixels in the cube, done

		quint16 median = c; // Median!
		//qDebug() << "Median is:" << median;

		// Now split "Cube" at median. Then add two new cubes to list of cubes.*/
		cube_type CubeA, CubeB;

		CubeA = Cube; CubeA.upper = median-1; //! Median is 0 sometimes, so this makes it -1!
		CubeA.count = mycount; // We've counted up half the pixels in the old cube
		CubeA.level = Cube.level + 1;
		myShrink(&CubeA);
		cubeList[splitPos] = CubeA;               // add in old slot */

		CubeB = Cube; CubeB.lower = median;
		CubeB.count = Cube.count - mycount;
		CubeB.level = Cube.level + 1;
		myShrink(&CubeB);
		cubeList[nCubes++] = CubeB;               // add in new slot */
	}

	myInvMap(histogram, colorMap, nCubes);
qDebug() << "And then to line" << __LINE__;
	return nCubes;
}


void myShrink(cube_type * Cube) // TODO: Add to cube class
{
	// Encloses "Cube" with a tight-fitting cube by updating (rmin, gmin, bmin)
	// and (rmax, gmax, bmax) members of "Cube". */
	quint8        r, g, b;
	quint16        i, color;

	Cube->rmin = 255; Cube->rmax = 0;
	Cube->gmin = 255; Cube->gmax = 0;
	Cube->bmin = 255; Cube->bmax = 0;

	// For each color (not pixel: color) in this cube, get the red, green, and
	// blue components, find the lowest and highest of each, and assign them
	// to rmin, rmax, etc.
	for(i = Cube->lower; i <= Cube->upper; ++i) { // For each color in this cube
		color = myHistPtr[i];
		r = RED2(color);
		if(r > Cube->rmax) Cube->rmax = r;
		if(r < Cube->rmin) Cube->rmin = r;
		g = GREEN2(color);
		if(g > Cube->gmax) Cube->gmax = g;
		if(g < Cube->gmin) Cube->gmin = g;
		b = BLUE2(color);
		if(b > Cube->bmax) Cube->bmax = b;
		if(b < Cube->bmin) Cube->bmin = b;
	}
}



//void myInvMap(word * Hist, byte ColMap[][3], word ncubes)
void myInvMap(std::vector<quint16> &histogram, quint8 colorMap[][4], quint16 nCubes)
{



	// foreach cube in list of cubes, computes centroid (average value) of
	// colors enclosed by that cube, and loads centroids in the color map. Next
	// loads histogram with indices into the color map. A preprocessor directive
	// #define FAST_REMAP controls whether cube centroids become output color
	// forall the colors in a cube, or whether a "best remap" is followed. */
	quint8      r, g, b;
	quint16     i, j, k, index, color;
	float       rsum, gsum, bsum;
	float       dr, dg, db, d, dmin;
	cube_type   Cube;

	for(k = 0; k <= nCubes -1; k++) {
		Cube = cubeList[k];
		rsum = gsum = bsum = (float)0.0;
		for(i = Cube.lower; i <= Cube.upper; i++) {
			color = myHistPtr[i];
			r = RED2(color);
			rsum += (float)r*(float)histogram[color];
			g = GREEN2(color);
			gsum += (float)g*(float)histogram[color];
			b = BLUE2(color);
			bsum += (float)b*(float)histogram[color];
		}
		// Update the color map */
		colorMap[k][1] = (quint8)(rsum / (float)Cube.count);
		colorMap[k][2] = (quint8)(gsum / (float)Cube.count);
		colorMap[k][3] = (quint8)(bsum / (float)Cube.count);
	}
#ifdef FAST_REMAP
	// Fast remap: foreach color in each cube, load the corresponding slot
	// in "Hist" with the centroid of the cube. */
	for(k=0;k<=ncubes-1;k++) {
		Cube = list[k];
		for(i=Cube.lower;i<=Cube.upper;i++) {
			color = myHistPtr[i];
			Hist[color] = k;
		}
	}
#else
	// Best remap: foreach color in each cube, find entry in ColMap that has
	// smallest Euclidian distance from color. Record this in "Hist". */
	for(k = 0; k <= nCubes -1; k++) {
		Cube = cubeList[k];
		for( i = Cube.lower; i <= Cube.upper; i++) {
			color = myHistPtr[i];
			r = RED2(color);  g = GREEN2(color); b = BLUE2(color);

			// Search for closest entry in "ColMap" */
			dmin = (float)4000000000.0;
			for(j = 0; j <= nCubes -1; j++) {
				dr = (float)colorMap[j][1] - (float)r;
				dg = (float)colorMap[j][2] - (float)g;
				db = (float)colorMap[j][3] - (float)b;
				d = dr*dr + dg*dg + db*db;
				if(d == (float)0.0) {
					index = j; break;
				}
				else if(d < dmin) {
					dmin = d; index = j;
				}
			}
			histogram[color] = index;
		}
	}
#endif
	return;
}


bool compareColor16Component(quint16 lhs, quint16 rhs)
{
	bool retval;

	switch(longestDimension) {
	case 0:
		retval = RED2(lhs) < RED2(rhs);
		break;
	case 1:
		retval = GREEN2(lhs) < GREEN2(rhs);
		break;
	case 2:
		retval = BLUE2(lhs) < BLUE2(rhs);
		break;
	}
	return retval;
}






