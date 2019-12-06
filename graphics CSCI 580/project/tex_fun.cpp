/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image = NULL;

int xs, ys;
int reset = 1;


/* Image texture function */
int tex_fun(float u, float v, GzColor color) {
	unsigned char		pixel[3];
	unsigned char     dummy;
	char  		foo[8];
	int   		i, j;
	FILE			*fd;

	if (reset) {          /* open and load texture file */
		fd = fopen("texture", "rb");
		if (fd == NULL) {
			fprintf(stderr, "texture file not found\n");
			exit(-1);
		}
		fscanf(fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
		image = (GzColor*)malloc(sizeof(GzColor)*(xs + 1)*(ys + 1));
		if (image == NULL) {
			fprintf(stderr, "malloc for texture image failed\n");
			exit(-1);
		}

		for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
			image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
			image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
		}

		reset = 0;          /* init is done */
		fclose(fd);
	}

	if (u < 0 || u>1 || v < 0 || v>1) {
		return GZ_FAILURE;
	}
	int aX = floor(u * (xs - 1));
	int aY = floor(v * (ys - 1));
	int cX = aX + 1;
	int cY = aY + 1;
	float t = v * (ys - 1) - aY;
	float s = u * (xs - 1) - aX;

	//Color(p) = s t C + (1-s) t D + s (1-t) B + (1-s) (1-t) A 
	color[0] = s * t*image[cX + xs * cY][0] + (1 - s)*t*image[aX + xs * cY][0] + s * (1 - t)*image[cX + xs * aY][0] + (1 - s)*(1 - t)*image[aX + xs * aY][0];
	color[1] = s * t*image[cX + xs * cY][1] + (1 - s)*t*image[aX + xs * cY][1] + s * (1 - t)*image[cX + xs * aY][1] + (1 - s)*(1 - t)*image[aX + xs * aY][1];
	color[2] = s * t*image[cX + xs * cY][2] + (1 - s)*t*image[aX + xs * cY][2] + s * (1 - t)*image[cX + xs * aY][2] + (1 - s)*(1 - t)*image[aX + xs * aY][2];

	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	/* determine texture cell corner values and perform bilinear interpolation */
	/* set color to interpolated GzColor value and return */


	return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color) {

	int interval = 5;

	int uNumber = (int)(u * interval) + 1;
	int vNumber = (int)(v * interval) + 1;
	bool uOdd = false;
	bool vOdd = false;

	uOdd = (uNumber % 2 != 0);
	vOdd = (vNumber % 2 != 0);

	float color1[3] = { 190.0 / 255.0, 227.0 / 255.0, 227.0 / 255.0 };
	//float color1[3] = { 114.0 / 255, 120.0 / 255, 168.0 / 255 };
	float color2[3] = { 166.0 / 255,18.0 / 255,106.0 / 255 };

	if ((uOdd && vOdd) || (!uOdd && !vOdd)) {
		color[0] = color1[0];
		color[1] = color1[1];
		color[2] = color1[2];
	} else {
		color[0] = color2[0];
		color[1] = color2[1];
		color[2] = color2[2];
	}

	return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture() {
	if (image != NULL)
		free(image);
	return GZ_SUCCESS;
}