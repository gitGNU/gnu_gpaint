
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
*/

#ifndef __IMAGE_PROCESSING_H__
#define __IMAGE_PROCESSING_H__

#include "image_buf.h"

#define ImagePixel(ibuf, x, y) (image_buf_rgbbuf(ibuf) + (y) * image_buf_rowstride((ibuf)) + (x) * image_buf_pixelsize((ibuf)))

void ImageSmooth(image_buf * input, image_buf *output);
void ImageSharpen(image_buf * input, image_buf *output);
void ImageEdge(image_buf * input, image_buf *output);
void ImageEmbose(image_buf * input, image_buf *output);
void ImageInvert(image_buf * input, image_buf *output);
void ImageOilPaint(image_buf * input, image_buf *output);
void ImageAddNoise(image_buf * input, image_buf *output);
void ImageSpread(image_buf * input);
void ImageBlend(image_buf * input, image_buf *output);
void ImagePixelize(image_buf * input, image_buf *output);
void ImageDespeckle(image_buf * input, image_buf *output);
void ImageNormContrast(image_buf * input, image_buf *output);
void ImageHistogram(image_buf * input, image_buf *output);
void ImageSolarize(image_buf * input, image_buf *output);
void ImageQuantize(image_buf * input, image_buf *output);
void ImageGrey(image_buf * input, image_buf *output);
void ImageTilt(image_buf * input, image_buf *output);
void ImageDirectionalFilter(image_buf * input, image_buf *output);

#endif
