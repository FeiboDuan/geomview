/*
 * This software is copyrighted as noted below.  It may be freely copied,
 * modified, and redistributed, provided that the copyright notice is 
 * preserved on all copies.
 * 
 * There is no warranty or other guarantee of fitness for this software,
 * it is provided solely "as is".  Bug reports or fixes may be sent
 * to the author, who may or may not act on them as he desires.
 *
 * You may not include this software in a program or other software product
 * without supplying the source, or without informing the end-user that the 
 * source is available for no extra charge.
 *
 * If you modify this software, you should include a notice giving the
 * name of the person performing the modification, the date of modification,
 * and the reason for such modification.
 *
 *  Modified at BRL 16-May-88 by Mike Muuss to avoid Alliant STDC desire
 *  to have all "void" functions so declared.
 */
/* 
 * dither.c - Functions for RGB color dithering.
 * 
 * Author:	Spencer W. Thomas
 * 		Computer Science Dept.
 * 		University of Utah
 * Date:	Mon Feb  2 1987
 * Copyright (c) 1987, University of Utah
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>

#ifdef USE_PROTOTYPES
void	make_square( double );
#else
void	make_square();
#endif

extern unsigned long mgx11colors[216];
int mgx11divN[256];
int mgx11modN[256];
int mgx11magic[16][16];

static int magic4x4[4][4] =  {
 	 0, 14,  3, 13,
	11,  5,  8,  6,
	12,  2, 15,  1,
	 7,  9,  4, 10
};

/* basic dithering macro */
#define DMAP(v,x,y)	(mgx11modN[v]>mgx11magic[x][y] ? mgx11divN[v] + 1 : mgx11divN[v])


/*****************************************************************
 * TAG( dithermap )
 * 
 * Create a color dithering map with a specified number of intensity levels.
 * Inputs:
 * 	levels:		Intensity levels per primary.
 *	gamma:		Display gamma value.
 * Outputs:
 * 	rgbmap:		Generated color map.
 *	mgx11divN:		"div" function for dithering.
 *	mgx11modN:		"mod" function for dithering.
 * Assumptions:
 * 	rgbmap will hold levels^3 entries.
 * Algorithm:
 *	Compute gamma compensation map.
 *	N = 255.0 / (levels - 1) is number of pixel values per level.
 *	Compute rgbmap with red ramping fastest, green slower, and blue
 *	slowest (treat it as if it were rgbmap[levels][levels][levels][3]).
 *	Call make_square to get mgx11divN, mgx11modN, and magic
 *
 * Note:
 *	Call dithergb( x, y, r, g, b, levels, mgx11divN, mgx11modN, magic ) to get index
 *	into rgbmap for a given color/location pair, or use
 *	    row = y % 16; col = x % 16;
 *	    DMAP(v,col,row) =def (mgx11divN[v] + (mgx11modN[v]>mgx11magic[col][row] ? 1 : 0))
 *	    DMAP(r,col,row) + DMAP(g,col,row)*levels + DMAP(b,col,row)*levels^2
 *	if you don't want function call overhead.
 */
void
dithermap(int levels, double gamma, int rgbmap[][3])
{
    double N;
    int i;
    int levelsq, levelsc;
    int gammamap[256];
    
    for ( i = 0; i < 256; i++ )
	gammamap[i] = (int)(0.5 + 255 * pow( i / 255.0, 1.0/gamma ));

    levelsq = levels*levels;	/* squared */
    levelsc = levels*levelsq;	/* and cubed */

    N = 255.0 / (levels - 1);    /* Get size of each step */

    /* 
     * Set up the color map entries.
     */
    for(i = 0; i < levelsc; i++) {
	rgbmap[i][0] = gammamap[(int)(0.5 + (i%levels) * N)];
	rgbmap[i][1] = gammamap[(int)(0.5 + ((i/levels)%levels) * N)];
	rgbmap[i][2] = gammamap[(int)(0.5 + ((i/levelsq)%levels) * N)];
    }

    make_square( N );
}


/*****************************************************************
 * TAG( bwdithermap )
 * 
 * Create a color dithering map with a specified number of intensity levels.
 * Inputs:
 * 	levels:		Intensity levels.
 *	gamma:		Display gamma value.
 * Outputs:
 * 	bwmap:		Generated black & white map.
 *	mgx11divN:		"div" function for dithering.
 *	mgx11modN:		"mod" function for dithering.
 * Assumptions:
 * 	bwmap will hold levels entries.
 * Algorithm:
 *	Compute gamma compensation map.
 *	N = 255.0 / (levels - 1) is number of pixel values per level.
 *	Compute bwmap for levels entries.
 *	Call make_square to get mgx11divN, mgx11modN, and magic.
 * Note:
 *	Call ditherbw( x, y, val, mgx11divN, mgx11modN, magic ) to get index into 
 *	bwmap for a given color/location pair, or use
 *	    row = y % 16; col = x % 16;
 *	    mgx11divN[val] + (mgx11modN[val]>mgx11magic[col][row] ? 1 : 0)
 *	if you don't want function call overhead.
 *	On a 1-bit display, use
 *	    mgx11divN[val] > mgx11magic[col][row] ? 1 : 0
 */
void
bwdithermap(int levels, double gamma, int bwmap[])
{
    double N;
    int i;
    int gammamap[256];
    
    for ( i = 0; i < 256; i++ )
	gammamap[i] = (int)(0.5 + 255 * pow( i / 255.0, 1.0/gamma ));

    N = 255.0 / (levels - 1);    /* Get size of each step */

    /* 
     * Set up the color map entries.
     */
    for(i = 0; i < levels; i++)
	bwmap[i] = gammamap[(int)(0.5 + i * N)];


    make_square( N );
}


/*****************************************************************
 * TAG( make_square )
 * 
 * Build the magic square for a given number of levels.
 * Inputs:
 * 	N:		Pixel values per level (255.0 / levels).
 * Outputs:
 * 	mgx11divN:		Integer value of pixval / N
 *	mgx11modN:		Integer remainder between pixval and mgx11divN[pixval]*N
 *	magic:		Magic square for dithering to N sublevels.
 * Assumptions:
 * 	
 * Algorithm:
 *	mgx11divN[pixval] = (int)(pixval / N) maps pixval to its appropriate level.
 *	mgx11modN[pixval] = pixval - (int)(N * mgx11divN[pixval]) maps pixval to
 *	its sublevel, and is used in the dithering computation.
 *	The magic square is computed as the (modified) outer product of
 *	a 4x4 magic square with itself.
 *	mgx11magic[4*k + i][4*l + j] = (magic4x4[i][j] + magic4x4[k][l]/16.0)
 *	multiplied by an appropriate factor to get the correct dithering
 *	range.
 */
void
make_square(double N)
{
    int i, j, k, l;
    double magicfact;

    for ( i = 0; i < 256; i++ )
    {
	mgx11divN[i] = (int)(i / N);
	mgx11modN[i] = i - (int)(N * mgx11divN[i]);
    }
    mgx11modN[255] = 0;		/* always */

    /*
     * Expand 4x4 dither pattern to 16x16.  4x4 leaves obvious patterning,
     * and doesn't give us full intensity range (only 17 sublevels).
     * 
     * magicfact is (N - 1)/16 so that we get numbers in the matrix from 0 to
     * N - 1: mod N gives numbers in 0 to N - 1, don't ever want all
     * pixels incremented to the next level (this is reserved for the
     * pixel value with mod N == 0 at the next level).
     */
    magicfact = (N - 1) / 16.;
    for ( i = 0; i < 4; i++ )
	for ( j = 0; j < 4; j++ )
	    for ( k = 0; k < 4; k++ )
		for ( l = 0; l < 4; l++ )
		    mgx11magic[4*k+i][4*l+j] =
			(int)(0.5 + magic4x4[i][j] * magicfact +
			      (magic4x4[k][l] / 16.) * magicfact);
}


/*****************************************************************
 * TAG( dithergb )
 * 
 * Return dithered RGB value.
 * Inputs:
 * 	x:		X location on screen of this pixel.
 *	y:		Y location on screen of this pixel.
 *	r, g, b:	Color at this pixel (0 - 255 range).
 *	levels:		Number of levels in this map.
 *	mgx11divN, mgx11modN:	From dithermap.
 *	magic:		Magic square from dithermap.
 * Outputs:
 * 	Returns color map index for dithered pixelv value.
 * Assumptions:
 * 	mgx11divN, mgx11modN, magic were set up properly.
 * Algorithm:
 * 	see "Note:" in dithermap comment.
 */
unsigned long dithergb( int x, int y, int *rgb, int levels)
{
    int col = x % 16, row = y % 16;

    return mgx11colors[DMAP(rgb[0], col, row) +
	DMAP(rgb[1], col, row) * levels +
	    DMAP(rgb[2], col, row) * levels*levels];
}


/*****************************************************************
 * TAG( ditherbw )
 * 
 * Return dithered black & white value.
 * Inputs:
 * 	x:		X location on screen of this pixel.
 *	y:		Y location on screen of this pixel.
 *	val:		Intensity at this pixel (0 - 255 range).
 *	mgx11divN, mgx11modN:	From dithermap.
 *	magic:		Magic square from dithermap.
 * Outputs:
 * 	Returns color map index for dithered pixel value.
 * Assumptions:
 * 	mgx11divN, mgx11modN, magic were set up properly.
 * Algorithm:
 * 	see "Note:" in bwdithermap comment.
 */
ditherbw( int x, int y, int val)
{
    int col = x % 16, row = y % 16;

    return DMAP(val, col, row);
}
