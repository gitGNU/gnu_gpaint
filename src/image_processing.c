
/*
    Copyright 2000  Li-Cheng (Andy) Tai

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    based on iprocess.c of xpaint, by David Koblas and Torsten Martinsen, whose original
    copyright notice follows
 +-------------------------------------------------------------------+ *
* | Copyright 1993, David Koblas (koblas@netcom.com)		       | *
* | Copyright 1995, 1996 Torsten Martinsen (bullestock@dk-online.dk)  | *
* |								       | *
* | Permission to use, copy, modify, and to distribute this software  | *
* | and its documentation for any purpose is hereby granted without   | *
* | fee, provided that the above copyright notice appear in all       | *
* | copies and that both that copyright notice and this permission    | *
* | notice appear in supporting documentation.	 There is no	       | *
* | representations about the suitability of this software for	       | *
* | any purpose.  this software is provided "as is" without express   | *
* | or implied warranty.					       | *
* |								       | *
* +-------------------------------------------------------------------+ *


*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"

#ifndef False
#define False 0
#endif
#ifndef True
#define True 1
#endif
#include "image_processing.h"

struct imageprocessinfo {
    int oilArea;
    int noiseDelta;
    int spreadDistance;
    int pixelizeXSize;
    int pixelizeYSize;
    int despeckleMask;
    int smoothMaskSize;
    int tilt;
    int solarizeThreshold;
    int contrastW;
    int contrastB;
    int quantizeColors;
    int tiltX1;
    int tiltY1;
    int tiltX2;
    int tiltY2;
    
};

/* Global data */
struct imageprocessinfo ImgProcessInfo =
{
    7,				/* oilArea		*/
    20,				/* noiseDelta		*/
    2,				/* spreadDistance	*/
    4,				/* pixelizeXSize	*/
    4,				/* pixelizeYSize	*/
    3,				/* despeckleMask	*/
    3,				/* smoothMaskSize	*/
    20,				/* tilt			*/
    50,				/* solarizeThreshold	*/
    99,				/* contrastW		*/
    2,				/* contrastB		*/
    16,				/* quantizeColors      */
    0, 0, 0, 0
};

#define RANGE		0x0fffffff
#define RANDOMI()	random()
#define RANDOMI2(s, f)	(((double)(random() % RANGE) / \
			  (double)RANGE) * ((f) - (s)) + (s))
#define SRANDOM(seed)	srandom((unsigned) (seed))

#define ICLAMP(low, value, high) \
		if (value < low) value = low; else if (value > high) value = high
#define DEG2RAD	(M_PI/180.0)

#define ConvMatrixSize	3
typedef float ConvMatrix[ConvMatrixSize * ConvMatrixSize];

static int *histogram(image_buf * input);

/*
 * Return a Gaussian (aka normal) random variable.
 *
 * Adapted from ppmforge.c, which is part of PBMPLUS.
 * The algorithm comes from:
 * 'The Science Of Fractal Images'. Peitgen, H.-O., and Saupe, D. eds.
 * Springer Verlag, New York, 1988.
 */
static double 
gauss(void)
{
    int i;
    double sum = 0.0;

    for (i = 0; i < 4; i++)
	sum += RANDOMI() & 0x7FFF;

    return sum * 5.28596089837e-5 - 3.46410161514;
}

static void
convolve(image_buf *input, image_buf *output, float *mat, int n,
	 unsigned char *basePixel, gboolean absFlag)
{
/* input and output must have been allocated */
    int x, y, xx, yy, xv, yv;
    float sum;
    unsigned char *p;
    unsigned char *op;
    float r, g, b;
    int ir, ig, ib;
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    
    const int xy = -n/2;
    const int ye = height + xy;
    const int xe = width + xy;
    const int hh = height * 2;
    const int ww = width * 2;

    
    op = image_buf_rgbbuf(output);

    sum = 0;
    for (yy = 0; yy < n; yy++) {
	for (xx = 0; xx < n; xx++) {
	    sum += mat[xx + n * yy];
	}
    }
    if (sum <= 0)
	sum = 1;

    for (y = xy; y < ye; y++) {
	for (x = xy; x < xe; x++) {
	    r = g = b = 0;
	    for (yy = 0; yy < n; yy++) {
		for (xx = 0; xx < n; xx++) {
		    xv = x + xx;
		    yv = y + yy;
		    if (xv < 0)
			xv = -xv;
		    if (yv < 0) 
			yv = -yv;
		    if (xv >= width) {
			xv = xv % ww;
			if (xv >= width)
			    xv = ww - xv - 1;
		    }
		    if (yv >= height) {
			yv = yv % hh;
			if (yv >= height)
			    yv = hh - yv - 1;
		    }
		    p = ImagePixel(input, xv, yv);
		    r += (float) *p++ * mat[xx + n * yy];
		    g += (float) *p++ * mat[xx + n * yy];
		    b += (float) *p * mat[xx + n * yy];
		}
	    }
	    if (absFlag) {
		if (r < 0)
		    r = -r;
		if (g < 0)
		    g = -g;
		if (b < 0)
		    b = -b;
	    }
	    ir = r / sum;
	    ig = g / sum;
	    ib = b / sum;
	    if (basePixel) {
		ir += basePixel[0];
		ig += basePixel[1];
		ib += basePixel[2];
	    }
	    ICLAMP(0, ir, 255);
	    ICLAMP(0, ig, 255);
	    ICLAMP(0, ib, 255);

         
	    *op++ = ir;
	    *op++ = ig;
	    *op++ = ib;
	    op += image_buf_pixelsize(output) - 3;
	}

	if (y % 16 == 0)
	    StateTimeStep();
	    
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }

   
}

/*
**  rescale values from 0..255
 */
static void 
normalize(image_buf * image)
{
    int i, count;
    unsigned char *sp;
    unsigned char *ip;
    int maxval = 0;
    int width = image_buf_width(image);
    int height = image_buf_height(image);

    {
	sp = image_buf_rgbbuf(image);
	count = width * height * image_buf_pixelsize(image);
    }

    for (ip = sp, i = 0; i < count; i++, ip++)
	if (*ip > maxval)
	    maxval = *ip;
    if (maxval == 0)
	return;
    for (ip = sp, i = 0; i < count; i++, ip++)
	*ip = ((int) *ip * 255) / maxval;
}

/*
**  Convert image into a monochrome image
 */
static void 
monochrome(image_buf * image)
{
    int i, count;
    unsigned char *sp;
    unsigned char *ip;
    int v;
    int width = image_buf_width(image);
    int height = image_buf_height(image);

    {
	sp = image_buf_rgbbuf(image);
	count = width * height;
    }

    for (ip = sp, i = 0; i < count; i++) {
	v = (ip[0] * 11 + ip[1] * 16 + ip[2] * 5) >> 5;		/* pp = .33R+.5G+.17B */
	
	*ip++ = v;
	*ip++ = v;
	*ip++ = v;
	ip += image_buf_pixelsize(image) - 3;
    }
}


void 
ImageSmooth(image_buf * input, image_buf *output)
{
    float *mat;
    int n, i;
    

    /*
     * Build n x n convolution matrix (all 1's)
     */
    n = ImgProcessInfo.smoothMaskSize;
    mat = malloc(n * n * sizeof(float));
    for (i = 0; i < n * n; ++i)
	mat[i] = 1.0;

    convolve(input, output, mat, n, NULL, False);
    free(mat);
    
}

void
ImageSharpen(image_buf * input, image_buf *output)
{
    static ConvMatrix mat =
    {
	-1, -2, -1,
	-2, 20, -2,
	-1, -2, -1
    };
    convolve(input, output, mat, ConvMatrixSize, NULL, False);
}

void
ImageEdge(image_buf * input, image_buf *output)
{
    static ConvMatrix mat =
    {
	-1, -2, 0,
	-2, 0, 2,
	0, 2, 1
    };
    convolve(input, output, mat, ConvMatrixSize, NULL, True);

    normalize(output);

  
}

void
ImageEmbose(image_buf * input, image_buf *output)
{
    static ConvMatrix mat =
    {
	-1, -2, 0,
	-2, 0, 2,
	0, 2, 1
    };
    static unsigned char base[3] =
    {128, 128, 128};
    convolve(input, output, mat, ConvMatrixSize, base, False);

    monochrome(output);
    normalize(output);

}

void
ImageInvert(image_buf * input, image_buf *output)
{
    unsigned char *op;
    int x,y;
    unsigned char *p;
    int width = image_buf_width(input);
    int height = image_buf_height(input);

    op = image_buf_rgbbuf(output);
    for (y = 0; y < height; y++)
    {
	   for (x = 0; x < width; x++)
       {
	      p = ImagePixel(input, x, y);
	      *op++ = 255 - *p++;
	      *op++ = 255 - *p++;
	      *op++ = 255 - *p;
	      op += image_buf_pixelsize(output) - 3;
       }
      op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }
}

void
ImageOilPaint(image_buf * input, image_buf *output)
{
    unsigned char *op = image_buf_rgbbuf(output);
    int x, y, xx, yy, i;
    int rVal, gVal, bVal;
    int rCnt, gCnt, bCnt;
    int rHist[256], gHist[256], bHist[256];
    int oilArea_2;
    int width = image_buf_width(input);
    int height = image_buf_height(input);


    oilArea_2 = ImgProcessInfo.oilArea / 2;

    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    /*
	    **	compute histogram of (on-screen hunk of) n*n 
	    **	  region centered plane
	     */

	    rCnt = gCnt = bCnt = 0;
	    rVal = gVal = bVal = 0;
	    for (i = 0; i < sizeof(rHist) / sizeof(int); i++)
		rHist[i] = gHist[i] = bHist[i] = 0;

	    for (yy = y - oilArea_2; yy < y + oilArea_2; yy++) {
		if (yy < 0 || yy >= height)
		    continue;
		for (xx = x - oilArea_2; xx < x + oilArea_2; xx++) {
		    int c, p;
		    unsigned char *rgb;

		    if (xx < 0 || xx >= width)
			continue;

		    rgb = ImagePixel(input, xx, yy);

		    if ((c = ++rHist[(p = rgb[0]) / 4]) > rCnt) {
			rVal = p;
			rCnt = c;
		    }
		    if ((c = ++gHist[(p = rgb[1]) / 4]) > gCnt) {
			gVal = p;
			gCnt = c;
		    }
		    if ((c = ++bHist[(p = rgb[2]) / 4]) > bCnt) {
			bVal = p;
			bCnt = c;
		    }
		}
	    }

	    *op++ = rVal;
	    *op++ = gVal;
	    *op++ = bVal;
	    op += image_buf_pixelsize(output) - 3;
	}

	if (y % 16 == 0)
	    StateTimeStep();
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }

    
}


/*
 * Add random noise to an image.
 */
void
ImageAddNoise(image_buf * input, image_buf *output)
{
    unsigned char *p;
    int x, y, r, g, b, ddelta;
    unsigned char *op = image_buf_rgbbuf(output);
    int width = image_buf_width(input);
    int height = image_buf_height(input);


    ddelta = ImgProcessInfo.noiseDelta * 2;

    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    p = ImagePixel(input, x, y);
	    r = p[0] + ddelta * gauss();
	    ICLAMP(0, r, 255);
	    g = p[1] + ddelta * gauss();
	    ICLAMP(0, g, 255);
	    b = p[2] + ddelta * gauss();
	    ICLAMP(0, b, 255);
	    *op++ = r;
	    *op++ = g;
	    *op++ = b;
	    op += image_buf_pixelsize(output) - 3;
	}
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }
   
}

/*
 * This function works in-place.
 * Because it swaps pixels in the input image, which may be colour mapped
 * or not, is has knowledge about the Image format that should probably
 * have stayed in image.h. Too bad.
 */
void
ImageSpread(image_buf * input)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    int w = width, h = height;
    unsigned char *p, *np;
    int x, y, minx, miny, maxx, maxy, xn, yn, dist;


    dist = ImgProcessInfo.spreadDistance;
    for (y = 0; y < h; y++) {
	for (x = 0; x < w; x++) {
	    p = ImagePixel(input, x, y);
	    /* find random neighbour pixel within +- dist */
	    minx = x - dist;
	    if (minx < 0)
		minx = 0;
	    maxx = x + dist;
	    if (maxx >= w)
		maxx = w - 1;
	    xn = RANDOMI2(minx, maxx);

	    miny = y - dist;
	    if (miny < 0)
		miny = 0;
	    maxy = y + dist;
	    if (maxy >= h)
		maxy = h - 1;
	    yn = RANDOMI2(miny, maxy);
	    /* swap pixel with neighbour */
	    np = ImagePixel(input, xn, yn);
	    {
		unsigned char rn, gn, bn;

		rn = np[0];
		gn = np[1];
		bn = np[2];
		np[0] = p[0];
		np[1] = p[1];
		np[2] = p[2];
		p[0] = rn;
		p[1] = gn;
		p[2] = bn;
	    } 
	}
    }

}

void
ImageBlend(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    int w = width, h = height;
    
    unsigned char *op = image_buf_rgbbuf(output);
    unsigned char *rgb;
    int x, y, n, ox, oy, px, py, dx, dy;
    int rSum, gSum, bSum, r, g, b;
    float diagonal, gradient, ap;


    ox = w / 2;
    oy = h / 2;
    diagonal = ((double) h) / w;

    /*
     * Compute average of pixels on edge of region. While we are
     * at it, we copy the edge to the output as well.
     */
    rSum = gSum = bSum = 0;
    for (x = 0; x < w; ++x) {
	rgb = ImagePixel(input, x, 0);
	r = rgb[0];
	g = rgb[1];
	b = rgb[2];
	rSum += r;
	gSum += g;
	bSum += b;
	*op++ = r;
	*op++ = g;
	*op++ = b;
	op += image_buf_pixelsize(output) - 3;
    }
    op = ImagePixel(output, 0, h - 1);
    for (x = 0; x < w; ++x) {
	rgb = ImagePixel(input, x, h - 1);
	r = rgb[0];
	g = rgb[1];
	b = rgb[2];
	rSum += r;
	gSum += g;
	bSum += b;
	*op++ = r;
	*op++ = g;
	*op++ = b;
	op += image_buf_pixelsize(output) - 3;
    }
    for (y = 1; y < h - 1; ++y) {
	rgb = ImagePixel(input, 0, y);
	r = rgb[0];
	g = rgb[1];
	b = rgb[2];
	rSum += r;
	gSum += g;
	bSum += b;
	op = ImagePixel(output, 0, y);
	*op++ = r;
	*op++ = g;
	*op++ = b;

	rgb = ImagePixel(input, w - 1, y);
	r = rgb[0];
	g = rgb[1];
	b = rgb[2];
	rSum += r;
	gSum += g;
	bSum += b;
	op = ImagePixel(output, w - 1, y);
	*op++ = r;
	*op++ = g;
	*op++ = b;
    }

    n = 2 * (w + h - 2);	/* total # of pixels on edge */
    rSum /= n;
    gSum /= n;
    bSum /= n;
    op = ImagePixel(output, 1, 1);

    /* compute colour of each point in the interior */
    for (y = 1; y < h - 1; y++) {
        op = ImagePixel(output, 1, y);
	for (x = 1; x < w - 1; x++) {
	    dx = x - ox;
	    dy = y - oy;
	    if (dx == 0 && dy == 0) {	/* this is the centre point */
		r = rSum;
		g = gSum;
		b = bSum;
	    } else {
		if (dx == 0) {	/* special case 1 */
		    px = ox;
		    py = (dy > 0) ? h - 1 : 0;
		} else if (dy == 0) {	/* special case 2 */
		    py = oy;
		    px = (dx > 0) ? w - 1 : 0;
		} else {	/* general case */
		    gradient = ((double) dy) / dx;
		    if (fabs(gradient) < fabs(diagonal)) {
			px = (dx > 0) ? w - 1 : 0;
			py = oy + ((px - ox) * dy) / dx;
		    } else {
			py = (dy > 0) ? h - 1 : 0;
			px = ox + ((py - oy) * dx) / dy;
		    }
		}

		/*
		 * given O(ox,oy), P(px,py), and A(x,y), compute
		 *   |AO|/|PO|
		 */
		ap = sqrt((double) (dx * dx) + (double) (dy * dy)) /
		    sqrt((double) ((px - ox) * (px - ox)) +
			 (double) ((py - oy) * (py - oy)));

		rgb = ImagePixel(input, px, py);
		r = (1 - ap) * rSum + ap * rgb[0];
		g = (1 - ap) * gSum + ap * rgb[1];
		b = (1 - ap) * bSum + ap * rgb[2];
	    }
	    *op++ = r;
	    *op++ = g;
	    *op++ = b;
	    op += image_buf_pixelsize(output) - 3;
	}
	/*op += 2 * image_buf_pixelsize(output);*/		/* skip last on this line and first on next */
    }

  
}

void
ImagePixelize(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
  
    unsigned char *op, *rgb;
    int x, y, xx, yy, n;
    int rSum, gSum, bSum;
    int xsize, ysize;


    xsize = ImgProcessInfo.pixelizeXSize;
    ysize = ImgProcessInfo.pixelizeYSize;

    if (xsize > width)
	xsize = width;
    if (ysize > height)
	ysize = height;
    n = xsize * ysize;

    for (y = 0; y < height; y += ysize) {
	for (x = 0; x < width; x += xsize) {
	    /*
	     *	compute average of pixels inside megapixel
	     */
	    rSum = gSum = bSum = 0;
	    for (yy = y; yy < y + ysize; yy++) {
		if (yy >= height)
		    continue;
		for (xx = x; xx < x + xsize; xx++) {
		    if (xx >= width)
			continue;
		    rgb = ImagePixel(input, xx, yy);
		    rSum += rgb[0];
		    gSum += rgb[1];
		    bSum += rgb[2];
		}
	    }
	    rSum /= n;
	    gSum /= n;
	    bSum /= n;
	    /*
	     * replace each pixel in megapixel with average
	     */
	    for (yy = y; yy < y + ysize; yy++) {
		if (yy >= height)
		    continue;
		for (xx = x; xx < x + xsize; xx++) {
		    if (xx >= width)
			continue;
		    op = ImagePixel(output, xx, yy);
		    *op++ = rSum;
		    *op++ = gSum;
		    *op = bSum;
		}
	    }
	}

	if (y % 16 == 0)
	    StateTimeStep();
    }

    /* if any incomplete megapixels are left, copy them to the output */
    for (x = (width / xsize) * xsize; x < width; ++x)
	for (y = 0; y < height; ++y) {
	    rgb = ImagePixel(input, x, y);
	    op = ImagePixel(output, x, y);
	    *op++ = *rgb++;
	    *op++ = *rgb++;
	    *op = *rgb;
	}
    for (y = (height / ysize) * ysize; y < height; ++y)
	for (x = 0; x < width; ++x) {
	    rgb = ImagePixel(input, x, y);
	    op = ImagePixel(output, x, y);
	    *op++ = *rgb++;
	    *op++ = *rgb++;
	    *op = *rgb;
	}

}


#define SWAP(a, b)	{ int t = (a); (a) = (b); (b) = t; }

void
ImageDespeckle(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
   
    unsigned char *op, *rgb;
    int x, y, xx, yy, mask, mask2, i, j, k, l;
    int *ra, *ga, *ba;

    mask = ImgProcessInfo.despeckleMask;
    if (mask > width)
	mask = width;
    if (mask > height)
	mask = height;
    mask2 = mask / 2;

    /* arrays for storing pixels inside mask */
    ra = malloc(mask * mask * sizeof(int));
    ga = malloc(mask * mask * sizeof(int));
    ba = malloc(mask * mask * sizeof(int));

    op = image_buf_rgbbuf(output);
    for (y = 0; y < height; ++y) {
	for (x = 0; x < width; ++x) {
	    i = 0;
	    for (yy = MAX(0, y - mask2); yy < MIN(height, y + mask2); ++yy)
		for (xx = MAX(0, x - mask2); xx < MIN(width, x + mask2); ++xx) {
		    rgb = ImagePixel(input, xx, yy);
		    ra[i] = *rgb++;
		    ga[i] = *rgb++;
		    ba[i] = *rgb;
		    ++i;
		}
	    /*
	     * now find median by (shell-)sorting the arrays and
	     * picking the center value
	     */
	    for (j = i / 2; j > 0; j = j / 2)
		for (k = j; k < i; k++) {
		    for (l = k - j; l >= 0 && ra[l] > ra[l + j]; l -= j)
			SWAP(ra[l], ra[l + j]);
		    for (l = k - j; l >= 0 && ga[l] > ga[l + j]; l -= j)
			SWAP(ga[l], ga[l + j]);
		    for (l = k - j; l >= 0 && ba[l] > ba[l + j]; l -= j)
			SWAP(ba[l], ba[l + j]);
		}
	    if (i & 1) {	/* uneven number of data points */
		*op++ = ra[i / 2];
		*op++ = ga[i / 2];
		*op++ = ba[i / 2];
	    } else {		/* even, take average */
		*op++ = (ra[i / 2 - 1] + ra[i / 2]) / 2;
		*op++ = (ga[i / 2 - 1] + ga[i / 2]) / 2;
		*op++ = (ba[i / 2 - 1] + ba[i / 2]) / 2;
	    }
	    op += image_buf_pixelsize(output) - 3;
	}
	if (y % 16 == 0)
	    StateTimeStep();
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
	    
    }

    free(ra);
    free(ga);
    free(ba);

    
}


/*
 * Normalize the contrast of an image.
 */
void
ImageNormContrast(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    unsigned char *p;
    int *hist;
    int x, y, w, h, i, max = 0, cumsum, limit;
    unsigned char *op = image_buf_rgbbuf(output);
    int blackpcnt, blackval, whitepcnt, whiteval;


    blackpcnt = ImgProcessInfo.contrastB;
    whitepcnt = 100 - ImgProcessInfo.contrastW;

    hist = histogram(input);

    w = width;
    h = height;

    /* Find lower knee point in histogram to determine 'black' threshold. */
    cumsum = 0;
    limit = w * h * blackpcnt / 100;
    for (i = 0; i < 256; ++i)
	if ((cumsum += hist[i]) > limit)
	    break;
    blackval = i;

    /* Likewise for 'white' threshold. */
    cumsum = 0;
    limit = w * h * whitepcnt / 100;
    for (i = 256; i > 0; --i) {
	if ((max == 0) && hist[i])
	    max = i + 1;	/* max is index of highest non-zero entry */
	if ((cumsum += hist[i]) > limit)
	    break;
    }
    whiteval = i;

    /*
     * Now create a histogram with a linear ramp
     * from (blackval, 0) to (whiteval, max)
     */
    for (i = 0; i < blackval; ++i)
	hist[i] = 0;
    for (; i <= whiteval; ++i)
	hist[i] = max * (i - blackval) / (whiteval - blackval);
    for (; i < 256; ++i)
	hist[i] = max;

    for (y = 0; y < h; y++) {
	for (x = 0; x < w; x++) {
	    p = ImagePixel(input, x, y);
	    *op++ = hist[*p++];
	    *op++ = hist[*p++];
	    *op++ = hist[*p];
	    op += image_buf_pixelsize(output) - 3;
	}
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }
    free(hist);

  
}

/*
 * Build histogram data for the image.
 */
static int *
histogram(image_buf * input)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    unsigned char *p;
    int x, y, w, h, i, r, g, b, *hist;

    w = width;
    h = height;

    /* Make a table of 256 zeros plus one sentinel */
    hist = malloc(257 * sizeof(int));
    for (i = 0; i <= 256; ++i)
	hist[i] = 0;

    /*
     * Build histogram of intensity values.
     */
    for (y = 0; y < h; y++) {
	for (x = 0; x < w; x++) {
	    p = ImagePixel(input, x, y);
	    r = *p++;
	    g = *p++;
	    b = *p;
	    i = (r * 169 + g * 256 + b * 87) / 512;	/* i = .33R+.5G+.17B */
	    ++hist[i];
	}
    }
    return hist;
}

#if 0

#define HIST_H	  120		/* Height of histogram */
#define HIST_W	  256		/* Width of histogram */
#define HIST_B	    2		/* Horizontal border width for histogram */
#define HIST_R	   12		/* Height of ruler below histogram */
#define HIST_T	    9		/* Height of tick marks on ruler */
#define LIGHTGREY 200		/* Greyscale level used for bars */
#define MIDGREY	  255		/* Greyscale level used for ruler */
#define DARKGREY  120		/* Greyscale level used for background */

/*
 * Create an image (width 256, height HIST_H) depicting the histogram data.
 */
void
HistogramImage(char *hist, image_buf *output)
{
    unsigned char *p;
    int x, y, i;
    int max, e:
    unsigned char *ihist;

    max = 0;
    ihist = malloc(128 * sizeof(int));
    for (i = 0; i < 128; ++i) {
	e = ihist[i] = hist[i * 2] + hist[i * 2 + 1];
	if (e > max)
	    max = e;
    }
    /*
     * Now create histogram image.
     * Only display 128 bins to make the histogram easier to read.
     * Leave a border of a few pixels in each side.
     */

    memset(image_buf_rgbbuf(output), DARKGREY, (HIST_W + 2 * HIST_B) * (HIST_H + HIST_R));
    for (x = 0; x < 128; ++x) {
	i = ihist[x] * (HIST_H - 1) / max;
	for (y = HIST_H - 1 - i; y < HIST_H - 1; y++) {
	    p = image_buf_rgbbuf(output) + y * (HIST_W + 2 * HIST_B) + x * 2 + HIST_B;
	    *p++ = LIGHTGREY;
	    *p = LIGHTGREY;
	}
    }
    /* Ruler */
    memset(image_buf_rgbbuf(output) + HIST_H * (HIST_W + 2 * HIST_B) + HIST_B,
	   MIDGREY, HIST_W);
    for (x = 0; x < 5; ++x) {
	int tick[] =
	{0, HIST_W / 4, HIST_W / 2, HIST_W * 3 / 4, HIST_W - 1};

	for (y = 0; y < HIST_T; y++) {
	    p = image_buf_rgbbuf(output) + (y + HIST_H + 1) * (HIST_W + 2 * HIST_B)
		+ tick[x] + HIST_B;
	    *p = MIDGREY;
	}
    }
    free(ihist);

   
}
#endif

#ifdef FEATURE_TILT
/*
 * Tilt an image.
 */

/*
 * Compute the interpolated RGB values of a 1 x 1 pixel centered
 * at (x,y) and store these values in op[0] trough op[2].
 * The interpolation is based on weighting the RGB values proportionally
 * to the overlapping areas.
 */
static void 
interpol(float x, float y, image_buf * input, unsigned char *op)
{
    int xl, xh, yl, yh;
    float a, b, c, d, A, B, C, D;
    unsigned char *plh, *phh, *pll, *phl;

    xl = floor(x);
    xh = ceil(x);
    yl = floor(y);
    yh = ceil(y);

    pll = ImagePixel(input, xl, yl);

    if (xh == xl) {
	if (yh == yl) {
	    /* pixel coincides with one pixel */
	    *op++ = *pll++;
	    *op++ = *pll++;
	    *op++ = *pll++;
	} else {
	    /* pixel overlaps with two pixels on top of each other */
	    plh = ImagePixel(input, xl, yh);
	    A = y - yl;
	    B = yh - y;
	    *op++ = A * *plh++ + B * *pll++;
	    *op++ = A * *plh++ + B * *pll++;
	    *op++ = A * *plh++ + B * *pll++;
	}
    } else if (yh == yl) {
	/* pixel overlaps with two side-by-side pixels */
	phl = ImagePixel(input, xh, yl);
	A = x - xl;
	B = xh - x;
	*op++ = A * *phl++ + B * *pll++;
	*op++ = A * *phl++ + B * *pll++;
	*op++ = A * *phl++ + B * *pll++;
    } else {
	/* general case: pixel overlaps all four neighbour pixels */
	a = xh - x;
	b = y - yl;
	c = x - xl;
	d = yh - y;
	A = a * b;
	B = b * c;
	C = a * d;
	D = c * d;
	plh = ImagePixel(input, xl, yh);
	phl = ImagePixel(input, xh, yl);
	phh = ImagePixel(input, xh, yh);
	*op++ = A * *plh++ + B * *phh++ + C * *pll++ + D * *phl++;
	*op++ = A * *plh++ + B * *phh++ + C * *pll++ + D * *phl++;
	*op++ = A * *plh++ + B * *phh++ + C * *pll++ + D * *phl++;
    }
}


void
ImageTilt(image_buf * input, image_buf *output)
{
    int x, y, br, bg, bb;
    
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    Image *output;
    unsigned char *op;
    float xt, yt;
    float X1, Y1, X2, Y2;


    X1 = width * ImgProcessInfo.tiltX1 / 100;
    Y1 = height * ImgProcessInfo.tiltY1 / 100;
    X2 = width * ImgProcessInfo.tiltX2 / 100;
    Y2 = height * ImgProcessInfo.tiltY2 / 100;

    if (((Y1 >= 0) && (Y1 < height)) || ((X2 >= 0) && (X2 < width)))
	return input;

 

    /* Get RGB values of background colour */
    br = ImgProcessInfo.background->red / 256;
    bg = ImgProcessInfo.background->green / 256;
    bb = ImgProcessInfo.background->blue / 256;

    op = image_buf_rgbbuf(output);
    for (x = 0; x < width * height; ++x) {
	*op++ = br;
	*op++ = bg;
	*op++ = bb;
	op += image_buf_pixelsize(output) - 3;
    }

    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    op = ImagePixel(output, x, y);
	    /* find the input coords corresponding to output coords (x,y) */
	    xt = (X1 * y - Y1 * x) / (y - Y1);
	    yt = (-X2 * y + x * Y2) / (x - X2);
	    if ((xt >= 0) && (xt < width) && (yt >= 0) && (yt < height)) {
#if 0
		unsigned char *p;
		/* rounding */
		p = ImagePixel(input, (int) (xt + 0.5), (int) (yt + 0.5));
		*op++ = *p++;
		*op++ = *p++;
		*op = *p;
#else
		interpol(xt, yt, input, op);
#endif
	    }
	}
	
    }


}
#endif

/*
 * Produce the 'solarization' effect seen when exposing a 
 * photographic film to light during the development process.
 * Done here by inverting all pixels above threshold level.
 */
void
ImageSolarize(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    int i;
    unsigned char *ip, *op;
    int count, limit;

    limit = ImgProcessInfo.solarizeThreshold * 255 / 100;

   
    {
	ip = image_buf_rgbbuf(input);
	op = image_buf_rgbbuf(output);
	count = width * height;
    }

    for (i = 0; i < count; i++) {
	*op++ = (*ip > limit) ? 255 - *ip++ : *ip++;
	*op++ = (*ip > limit) ? 255 - *ip++ : *ip++;
	*op++ = (*ip > limit) ? 255 - *ip++ : *ip++;
	op += image_buf_pixelsize(output) - 3;
    }

  
}

/*
 * Colourmap quantization. Hacked from ppmqvga.c, which is part of Netpbm and
 * was originally written by Lyle Rains (lrains@netcom.com) and Bill Davidsen
 * (davidsen@crd.ge.com).
 * You are probably not supposed to understand this.
 */

#define RED_BITS   5
#define GREEN_BITS 6
#define BLUE_BITS  5

#define MAX_RED	   (1 << RED_BITS)
#define MAX_GREEN  (1 << GREEN_BITS)
#define MAX_BLUE   (1 << BLUE_BITS)

#define MAXWEIGHT  128
#define STDWEIGHT_DIV  (2 << 8)
#define STDWEIGHT_MUL  (2 << 10)
#define GAIN	   4

static int r, g, b, clutx, rep_threshold, rep_weight, dr, dg, db;
static int *color_cube, ncolors;
static unsigned char *clut;

#define CUBEINDEX(r,g,b)  (r)*(MAX_GREEN*MAX_BLUE) + (g)*MAX_BLUE + (b)

static void 
diffuse(void)
{
    int _7_32nds, _3_32nds, _1_16th;

    if (clutx < ncolors) {
	if (color_cube[CUBEINDEX(r, g, b)] > rep_threshold) {
	    clut[clutx * 4 + 0] = ((2 * r + 1) * 256) / (2 * MAX_RED);
	    clut[clutx * 4 + 1] = ((2 * g + 1) * 256) / (2 * MAX_GREEN);
	    clut[clutx * 4 + 2] = ((2 * b + 1) * 256) / (2 * MAX_BLUE);
	    ++clutx;
	    color_cube[CUBEINDEX(r, g, b)] -= rep_weight;
	}
	_7_32nds = (7 * color_cube[CUBEINDEX(r, g, b)]) / 32;
	_3_32nds = (3 * color_cube[CUBEINDEX(r, g, b)]) / 32;
	_1_16th = color_cube[CUBEINDEX(r, g, b)] - 3 * (_7_32nds + _3_32nds);
	color_cube[CUBEINDEX(r, g, b)] = 0;
	/* spread error evenly in color space. */
	color_cube[CUBEINDEX(r, g, b + db)] += _7_32nds;
	color_cube[CUBEINDEX(r, g + dg, b)] += _7_32nds;
	color_cube[CUBEINDEX(r + dr, g, b)] += _7_32nds;
	color_cube[CUBEINDEX(r, g + dg, b + db)] += _3_32nds;
	color_cube[CUBEINDEX(r + dr, g, b + db)] += _3_32nds;
	color_cube[CUBEINDEX(r + dr, g + dg, b)] += _3_32nds;
	color_cube[CUBEINDEX(r + dr, g + dg, b + db)] += _1_16th;

	/*
	 * Conserve the error at edges if possible
	 * (which it is, except the last pixel)
	 */
	if (color_cube[CUBEINDEX(r, g, b)] != 0) {
	    if (dg != 0)
		color_cube[CUBEINDEX(r, g + dg, b)] +=
		    color_cube[CUBEINDEX(r, g, b)];
	    else if (dr != 0)
		color_cube[CUBEINDEX(r + dr, g, b)] +=
		    color_cube[CUBEINDEX(r, g, b)];
	    else if (db != 0)
		color_cube[CUBEINDEX(r, g, b) + db] +=
		    color_cube[CUBEINDEX(r, g, b)];
	    else
		fprintf(stderr, "lost error term\n");
	}
    }
    color_cube[CUBEINDEX(r, g, b)] = -1;
}

/*
** Find representative color nearest to requested color.  Check color cube
** for a cached color index.  If not cached, compute nearest and cache result.
 */
static int 
nearest_color(unsigned char *p)
{
    register unsigned char *test;
    register unsigned i;
    unsigned long min_dist_sqd, dist_sqd;
    int nearest = 0;
    int *cache;
    int r, g, b;

    r = *p++;
    g = *p++;
    b = *p;
    cache = &(color_cube[CUBEINDEX((r << RED_BITS) / 256,
				   (g << GREEN_BITS) / 256,
				   (b << BLUE_BITS) / 256)]);
    if (*cache >= 0)
	return *cache;
    min_dist_sqd = ~0;
    for (i = 0; i < ncolors; ++i) {
	test = &clut[i * 4];
	dist_sqd =
	    3 * (r - test[0]) * (r - test[0]) +
	    4 * (g - test[1]) * (g - test[1]) +
	    2 * (b - test[2]) * (b - test[2]);
	if (dist_sqd < min_dist_sqd) {
	    nearest = i;
	    min_dist_sqd = dist_sqd;
	}
    }
    return (*cache = nearest);
}

void
ImageQuantize(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    int w = width, h = height;
   
    unsigned char *op = image_buf_rgbbuf(output);
    int *weight_convert;
    int k, x, y, i, j, nearest;
    int total_weight, cum_weight[MAX_GREEN];
    int *erropt;
    int *errP;
    unsigned char *p, *clutP;


    ncolors = ImgProcessInfo.quantizeColors;

    /* Make a color cube, initialized to zeros */
    color_cube = calloc(MAX_RED * MAX_GREEN * MAX_BLUE, sizeof(int));
    clut = calloc(ncolors * 4, sizeof(int));
    erropt = calloc(ncolors * 4, sizeof(int));

    clutx = 0;

    /* Count all occurrances of each color */
    for (y = 0; y < h; ++y)
	for (x = 0; x < w; ++x) {
	    p = ImagePixel(input, x, y);
	    r = p[0] / (256 / MAX_RED);
	    g = p[1] / (256 / MAX_GREEN);
	    b = p[2] / (256 / MAX_BLUE);
	    ++color_cube[CUBEINDEX(r, g, b)];
	}

    /* Initialize logarithmic weighting table */
    weight_convert = malloc(MAXWEIGHT * sizeof(int));
    weight_convert[0] = 0;
    for (i = 1; i < MAXWEIGHT; ++i) {
	weight_convert[i] = (int) (100.0 * log((double) i));
    }

    k = w * h;
    if ((k /= STDWEIGHT_DIV) == 0)
	k = 1;
    total_weight = i = 0;
    for (g = 0; g < MAX_GREEN; ++g) {
	for (r = 0; r < MAX_RED; ++r) {
	    for (b = 0; b < MAX_BLUE; ++b) {
		register int weight;
		/* Normalize the weights, independent of picture size. */
		weight = color_cube[CUBEINDEX(r, g, b)] * STDWEIGHT_MUL;
		weight /= k;
		if (weight)
		    ++i;
		if (weight >= MAXWEIGHT)
		    weight = MAXWEIGHT - 1;
		total_weight += (color_cube[CUBEINDEX(r, g, b)]
				 = weight_convert[weight]);
	    }
	}
	cum_weight[g] = total_weight;
    }
    rep_weight = total_weight / ncolors;

    /* Magic foo-foo dust here.	 What IS the correct way to select threshold? */
    rep_threshold = total_weight * (28 + 110000 / i) / 95000;

    /*
     * Do a 3-D error diffusion dither on the data in the color cube
     * to select the representative colors.  Do the dither back and forth in
     * such a manner that all the error is conserved (none lost at the edges).
     */

    dg = 1;
    for (g = 0; g < MAX_GREEN; ++g) {
	dr = 1;
	for (r = 0; r < MAX_RED; ++r) {
	    db = 1;
	    for (b = 0; b < MAX_BLUE - 1; ++b)
		diffuse();
	    db = 0;
	    diffuse();
	    ++b;
	    if (++r == MAX_RED - 1)
		dr = 0;
	    db = -1;
	    while (--b > 0)
		diffuse();
	    db = 0;
	    diffuse();
	}
	/* Modify threshold to keep rep points proportionally distributed */
	if ((j = clutx - (ncolors * cum_weight[g]) / total_weight) != 0)
	    rep_threshold += j * GAIN;

	if (++g == MAX_GREEN - 1)
	    dg = 0;
	dr = -1;
	while (r-- > 0) {
	    db = 1;
	    for (b = 0; b < MAX_BLUE - 1; ++b)
		diffuse();
	    db = 0;
	    diffuse();
	    ++b;
	    if (--r == 0)
		dr = 0;
	    db = -1;
	    while (--b > 0)
		diffuse();
	    db = 0;
	    diffuse();
	}
	/* Modify threshold to keep rep points proportionally distributed */
	if ((j = clutx - (ncolors * cum_weight[g]) / total_weight) != 0)
	    rep_threshold += j * GAIN;
    }

    /*
     * Check the error associated with the use of each color, and
     * change the value of the color to minimize the error.
     */
    for (y = 0; y < h; ++y) {
	for (x = 0; x < w; ++x) {
	    p = ImagePixel(input, x, y);
	    nearest = nearest_color(p);
	    errP = &erropt[nearest * 4];
	    clutP = &clut[nearest * 4];
	    errP[0] += *p++ - clutP[0];
	    errP[1] += *p++ - clutP[1];
	    errP[2] += *p - clutP[2];
	    ++errP[3];
	}
    }

    for (i = 0; i < ncolors; ++i) {
	clutP = &clut[i * 4];
	errP = &erropt[i * 4];
	j = errP[3];
	if (j > 0)
	    j *= 4;
	else if (j == 0)
	    j = 1;
	clutP[0] += (errP[0] / j) * 4;
	clutP[1] += (errP[1] / j) * 4;
	clutP[2] += (errP[2] / j) * 4;
    }

    /* Reset the color cache. */
    for (i = 0; i < MAX_RED * MAX_GREEN * MAX_BLUE; ++i)
	color_cube[i] = -1;

    /*
     * Map the colors in the image to their closest match in the new colormap.
     */
    for (y = 0; y < h; ++y) {
	for (x = 0; x < w; ++x) {
	    p = ImagePixel(input, x, y);
	    nearest = nearest_color(p);
	    clutP = &clut[nearest * 4];
	    *op++ = *clutP++;
	    *op++ = *clutP++;
	    *op++ = *clutP;
	    op += image_buf_pixelsize(output) - 3;
	}
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }

    free(clut);
    free(erropt);
    free(weight_convert);

    
}

/*
 * Convert an image to grey scale.
 */
void
ImageGrey(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
   
    unsigned char *p, *op = image_buf_rgbbuf(output);
    int x, y, v;


    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    p = ImagePixel(input, x, y);
	    v = (p[0] * 11 + p[1] * 16 + p[2] * 5) >> 5;	/* .33R + .5G + .17B */
	    *op++ = v;
	    *op++ = v;
	    *op++ = v;
	    op += image_buf_pixelsize(output) - 3;
	}
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }
   
}

#define GRAY(p)		(((p)[0]*169 + (p)[1]*256 + (p)[2]*87)/512)
#define SQ(x)		((x)*(x))

/*
 * Directional filter, according to the algorithm described on p. 60 of:
 * _Algorithms_for_Graphics_and_Image_Processing_. Theo Pavlidis.
 * Computer Science Press, 1982.
 * For each pixel, detect the most prominent edge and apply a filter that
 * does not degrade that edge.
 */
void
ImageDirectionalFilter(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    unsigned char *p00, *p10, *p_0, *p11, *p__, *p01, *p0_, *p_1, *p1_;
    int g00, g10, g_0, g11, g__, g01, g0_, g_1, g1_;
    int v0, v45, v90, v135, vmin, theta;
    int x, y;
   
    unsigned char *op = image_buf_rgbbuf(output);

    /*
     * We don't process the border of the image tto avoid the hassle
     * of having do deal with boundary conditions. Hopefully no one
     * will notice.
     */
    /* copy first row unchanged */
    for (x = 0; x < width; x++) {
	p00 = ImagePixel(input, x, 0);
	*op++ = *p00++;
	*op++ = *p00++;
	*op++ = *p00++;
	op += image_buf_pixelsize(output) - 3;
    }
	
    op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);

    for (y = 1; y < height - 1; y++) {
	/* copy first column unchanged */
	p00 = ImagePixel(input, 0, y);
	*op++ = *p00++;
	*op++ = *p00++;
	*op++ = *p00++;
	op += image_buf_pixelsize(output) - 3;
	for (x = 1; x < width - 1; x++) {
	    /* find values of pixel and all neighbours */
	    p00 = ImagePixel(input, x, y);
	    p10 = ImagePixel(input, x + 1, y);
	    p_0 = ImagePixel(input, x - 1, y);
	    p11 = ImagePixel(input, x + 1, y + 1);
	    p__ = ImagePixel(input, x - 1, y - 1);
	    p01 = ImagePixel(input, x, y + 1);
	    p0_ = ImagePixel(input, x, y - 1);
	    p_1 = ImagePixel(input, x - 1, y + 1);
	    p1_ = ImagePixel(input, x + 1, y - 1);

	    /* get grayscale values */
	    g00 = GRAY(p00);
	    g01 = GRAY(p01);
	    g10 = GRAY(p10);
	    g11 = GRAY(p11);
	    g0_ = GRAY(p0_);
	    g_0 = GRAY(p_0);
	    g__ = GRAY(p__);
	    g_1 = GRAY(p_1);
	    g1_ = GRAY(p1_);

	    /* estimate direction of edge, if any */
	    v0 = SQ(g00 - g10) + SQ(g00 - g_0);
	    v45 = SQ(g00 - g11) + SQ(g00 - g__);
	    v90 = SQ(g00 - g01) + SQ(g00 - g0_);
	    v135 = SQ(g00 - g_1) + SQ(g00 - g1_);

	    vmin = MIN(MIN(v0, v45), MIN(v90, v135));
	    theta = 0;
	    if (vmin == v45)
		theta = 1;
	    else if (vmin == v90)
		theta = 2;
	    else if (vmin == v135)
		theta = 3;

	    /* apply filtering according to direction of edge */
	    switch (theta) {
	    case 0:		/* 0 degrees */
		*op++ = (*p_0++ + *p00++ + *p10++) / 3;
		*op++ = (*p_0++ + *p00++ + *p10++) / 3;
		*op++ = (*p_0++ + *p00++ + *p10++) / 3;
	        op += image_buf_pixelsize(output) - 3;
		break;
	    case 1:		/* 45 degrees */
		*op++ = (*p__++ + *p00++ + *p11++) / 3;
		*op++ = (*p__++ + *p00++ + *p11++) / 3;
		*op++ = (*p__++ + *p00++ + *p11++) / 3;
	        op += image_buf_pixelsize(output) - 3;
		break;
	    case 2:		/* 90 degrees */
		*op++ = (*p0_++ + *p00++ + *p01++) / 3;
		*op++ = (*p0_++ + *p00++ + *p01++) / 3;
		*op++ = (*p0_++ + *p00++ + *p01++) / 3;
	        op += image_buf_pixelsize(output) - 3;
		break;
	    case 3:		/* 135 degrees */
		*op++ = (*p1_++ + *p00++ + *p_1++) / 3;
		*op++ = (*p1_++ + *p00++ + *p_1++) / 3;
		*op++ = (*p1_++ + *p00++ + *p_1++) / 3;
	        op += image_buf_pixelsize(output) - 3;
		break;
	    }
	}
	/* copy last column unchanged */
	p00 = ImagePixel(input, x, y);
	*op++ = *p00++;
	*op++ = *p00++;
	*op++ = *p00++;
	op += image_buf_pixelsize(output) - 3;
	
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }

    /* copy last row unchanged */
    for (x = 0; x < width; x++) {
	p00 = ImagePixel(input, x, y);
	*op++ = *p00++;
	*op++ = *p00++;
	*op++ = *p00++;
	op += image_buf_pixelsize(output) - 3;
    }
    
}

#if 0
#define ISFOREGND(p) ((p) ? \
		      (deltaR-deltaRV <= p[0]) && (p[0] <= deltaR+deltaRV) && \
		      (deltaG-deltaGV <= p[1]) && (p[1] <= deltaG+deltaGV) && \
		      (deltaB-deltaBV <= p[2]) && (p[2] <= deltaB+deltaBV) : 1)

/*
 * Thicken an image.
 */
void
ImageThicken(image_buf * input, image_buf *output)
{
    int width = image_buf_width(input);
    int height = image_buf_height(input);
    unsigned char *p00, *p10, *p_0, *p01, *p0_;
    int x, y, br, bg, bb;
    int deltaR, deltaRV, deltaG, deltaGV, deltaB, deltaBV;
    Image *output;
    unsigned char *op;

   
    op = image_buf_rgbbuf(output);

    /* Get RGB values of background colour */
    br = ImgProcessInfo.background->red / 256;
    bg = ImgProcessInfo.background->green / 256;
    bb = ImgProcessInfo.background->blue / 256;

    for (y = 0; y < height; y++)
    {
	for (x = 0; x < width; x++) {
	    /* find values of pixel and all d-neighbours */
	    p00 = ImagePixel(input, x, y);
	    p10 = x < width - 1 ? ImagePixel(input, x + 1, y) : NULL;
	    p_0 = x > 0 ? ImagePixel(input, x - 1, y) : NULL;
	    p01 = y < height - 1 ? ImagePixel(input, x, y + 1) : NULL;
	    p0_ = y > 0 ? ImagePixel(input, x, y - 1) : NULL;

	    if (ISFOREGND(p00)) {
		*op++ = *p00++;
		*op++ = *p00++;
		*op++ = *p00++;
	    } else {
		*op++ = br;
		*op++ = bg;
		*op++ = bb;
	    }
	    op += image_buf_pixelsize(output) - 3;
	}
	op += image_buf_rowstride(output) - width * image_buf_pixelsize(output);
    }

}
#endif
