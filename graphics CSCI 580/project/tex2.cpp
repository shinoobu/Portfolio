/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image2 = NULL;
GzColor	*frontImage = (GzColor*)malloc(sizeof(GzColor)*(2048 + 1)*(2048 + 1));
GzColor	*backImage = (GzColor*)malloc(sizeof(GzColor)*(2048 + 1)*(2048 + 1));
GzColor	*rightImage = (GzColor*)malloc(sizeof(GzColor)*(2048 + 1)*(2048 + 1));
GzColor	*leftImage = (GzColor*)malloc(sizeof(GzColor)*(2048 + 1)*(2048 + 1));
GzColor	*upImage = (GzColor*)malloc(sizeof(GzColor)*(2048 + 1)*(2048 + 1));
GzColor	*downImage = (GzColor*)malloc(sizeof(GzColor)*(2048 + 1)*(2048 + 1));
int xs2, ys2;
int reset2 = 1;
int frontReset = 1;
int backReset = 1;
int rightReset = 1;
int leftReset = 1;
int upReset = 1;
int downReset = 1;

int etex_fun(float u, float v, GzColor color, char dirChar) {
	unsigned char		pixel[3];
	unsigned char     dummy;
	char  		foo[8];
	int   		i, j;
	FILE			*fd;

	if (dirChar == 'f') {		//need front image2 and has not loaded
		image2 = frontImage;
		if (frontReset) {
			reset2 = 1;
			fd = fopen("front", "rb");
			frontReset = 0;
		}
	}
	else if (dirChar == 'b') {		//need back image2 and has not loaded
		image2 = backImage;
		if (backReset) {
			reset2 = 1;
			fd = fopen("back", "rb");
			backReset = 0;
		}
	}
	else if (dirChar == 'r') {		//need right image2 and has not loaded
		image2 = rightImage;
		if (rightReset) {
			reset2 = 1;
			fd = fopen("right", "rb");
			rightReset = 0;
		}
	}
	else if (dirChar == 'l') {		//need left image2 and has not loaded
		image2 = leftImage;
		if (leftReset) {
			reset2 = 1;
			fd = fopen("left", "rb");
			leftReset = 0;
		}
	}
	else if (dirChar == 'u') {		//need up image2 and has not loaded
		image2 = upImage;
		if (upReset) {
			reset2 = 1;
			fd = fopen("up", "rb");
			upReset = 0;
		}
	}
	else if (dirChar == 'd') {		//need down image2 and has not loaded
		image2 = downImage;
		if (downReset) {
			reset2 = 1;
			fd = fopen("down", "rb");
			downReset = 0;
		}
	}

	if (reset2) {          /* open and load texture file */
		//fd = fopen("texture", "rb");
		if (fd == NULL) {
			fprintf(stderr, "texture file not found\n");
			exit(-1);
		}
		fscanf(fd, "%s %d %d %c", foo, &xs2, &ys2, &dummy);
		//image2 = (GzColor*)malloc(sizeof(GzColor)*(xs2 + 1)*(ys2 + 1));
		if (image2 == NULL) {
			fprintf(stderr, "malloc for texture image2 failed\n");
			exit(-1);
		}

		for (i = 0; i < xs2*ys2; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image2[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
			image2[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
			image2[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
		}

		reset2 = 0;          /* init is done */
		fclose(fd);
	}

	if (u < 0 || u>1 || v < 0 || v>1) {
		return GZ_FAILURE;
	}
	int aX = floor(u * (xs2 - 1));
	int aY = floor(v * (ys2 - 1));
	int cX = aX + 1;
	int cY = aY + 1;
	float t = v * (ys2 - 1) - aY;
	float s = u * (xs2 - 1) - aX;

	//Color(p) = s t C + (1-s) t D + s (1-t) B + (1-s) (1-t) A 
	color[0] = s * t*image2[cX + xs2 * cY][0] + (1 - s)*t*image2[aX + xs2 * cY][0] + s * (1 - t)*image2[cX + xs2 * aY][0] + (1 - s)*(1 - t)*image2[aX + xs2 * aY][0];
	color[1] = s * t*image2[cX + xs2 * cY][1] + (1 - s)*t*image2[aX + xs2 * cY][1] + s * (1 - t)*image2[cX + xs2 * aY][1] + (1 - s)*(1 - t)*image2[aX + xs2 * aY][1];
	color[2] = s * t*image2[cX + xs2 * cY][2] + (1 - s)*t*image2[aX + xs2 * cY][2] + s * (1 - t)*image2[cX + xs2 * aY][2] + (1 - s)*(1 - t)*image2[aX + xs2 * aY][2];


	return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture2() {
	if (image2 != NULL)
		free(image2);
	return GZ_SUCCESS;
}