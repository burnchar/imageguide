#ifndef MEDIANCUT_H
#define MEDIANCUT_H

//#define  FAST_REMAP
#include <algorithm>
//#include <stdio.h>
//#include <stddef.h>              /* for NULL                   */
#include <stdlib.h>              /* for "qsort"                */
#include <float.h>               /* for FLT_MAX, FLT_MIN       */
#define MAXCOLORS 256            /* maximum # of output colors */
#define HSIZE     32768          /* size of image histogram    */
typedef unsigned  char byte;     /* range 0-255                */
typedef unsigned  short word;    /* range 0-65,535             */
typedef unsigned  long dword;    /* range 0-4,294,967,295      */

/* Macros for converting between (r,g,b)-colors and 15-bit     */
/* colors follow.                                              */
#define RGB(r,g,b) (word)(((b)&~7)<<7)|(((g)&~7)<<2)|((r)>>3)
#define RED(x)     (byte)(((x)&31)<<3)
#define GREEN(x)   (byte)((((x)>>5)&255)<< 3)
#define BLUE(x)    (byte)((((x)>>10)&255)<< 3)

typedef  struct {       /* structure for a cube in color space */
	word  lower;         /* one corner's index in histogram     */
	word  upper;         /* another corner's index in histogram */
	dword count;         /* cube's histogram count              */
	int   level;         /* cube's level                        */

	byte  rmin,rmax;
	byte  gmin,gmax;
	byte  bmin,bmax;
} cube_t;

static cube_t list[MAXCOLORS];   /* list of cubes              */
static int longdim;              /* longest dimension of cube  */
static word HistPtr[HSIZE];      /* points to colors in "Hist" */

void Shrink(cube_t * Cube);
void InvMap(word * Hist, byte ColMap[][3],word ncubes);
int  compare(const void * a1, const void * a2);

word MedianCut(word Hist[], byte ColMap[][3], int maxcubes);
void Shrink(cube_t * Cube);
void InvMap(word * Hist, byte ColMap[][3], word ncubes);
int compare(const void *a1, const void *a2);


#endif // MEDIANCUT_H
