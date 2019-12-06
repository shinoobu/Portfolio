/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"rend.h"
#include	<string>
#include	<math.h>
#include	"Gz.h"

#define PI (float) 3.14159265358979323846
extern int etex_fun(float u, float v, GzColor color, char dirChar); /* environment texture function */

int GzRender::GzRotXMat(float degree, GzMatrix mat) {
	/* HW 3.1
	// Create rotate matrix : rotate along x axis
	// Pass back the matrix using mat value
	*/
	float radians = PI / 180 * degree;
	mat[1][1] = cos(radians);
	mat[1][2] = -sin(radians);
	mat[2][1] = sin(radians);
	mat[2][2] = cos(radians);
	return GZ_SUCCESS;
}

int GzRender::GzRotYMat(float degree, GzMatrix mat) {
	/* HW 3.2
	// Create rotate matrix : rotate along y axis
	// Pass back the matrix using mat value
	*/
	float radians = PI / 180 * degree;
	mat[0][0] = cos(radians);
	mat[0][2] = sin(radians);
	mat[2][0] = -sin(radians);
	mat[2][2] = cos(radians);
	return GZ_SUCCESS;
}

int GzRender::GzRotZMat(float degree, GzMatrix mat) {
	/* HW 3.3
	// Create rotate matrix : rotate along z axis
	// Pass back the matrix using mat value
	*/
	float radians = PI / 180 * degree;
	mat[0][0] = cos(radians);
	mat[0][1] = -sin(radians);
	mat[1][0] = sin(radians);
	mat[1][1] = cos(radians);
	return GZ_SUCCESS;
}

int GzRender::GzTrxMat(GzCoord translate, GzMatrix mat) {
	/* HW 3.4
	// Create translation matrix
	// Pass back the matrix using mat value
	*/
	mat[0][3] = translate[0];
	mat[1][3] = translate[1];
	mat[2][3] = translate[2];
	return GZ_SUCCESS;
}

int GzRender::GzScaleMat(GzCoord scale, GzMatrix mat) {
	/* HW 3.5
	// Create scaling matrix
	// Pass back the matrix using mat value
	*/
	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
	return GZ_SUCCESS;
}

GzRender::GzRender(int xRes, int yRes) {
	/* HW1.1 create a framebuffer for MS Windows display:
	 -- set display resolution
	 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
	 -- allocate memory for pixel buffer
	 */
	xres = xRes;
	yres = yRes;
	framebuffer = new char[3 * xRes*yRes];
	pixelbuffer = new GzPixel[xRes*yRes];

	isEnvironmentMapping = 1;

	/* HW 3.6
	- setup Xsp and anything only done once
	- init default camera
	*/
	matlevel = -1;
	numlights = 0;
	GzMatrix tempXsp = {			//transfer from perspective to screen
		xRes / 2,	0.0,		0.0,	xRes / 2,
		0.0,		-yRes / 2,	0.0,	yRes / 2,
		0.0,		0.0,		MAXINT,	0.0,
		0.0,		0.0,		0.0,		1.0
	};
	memcpy(Xsp, tempXsp, sizeof(tempXsp));

	//GzPushMatrix(Xsp);

	m_camera.FOV = DEFAULT_FOV;
	m_camera.lookat[0] = 0.0;
	m_camera.lookat[1] = 0.0;
	m_camera.lookat[2] = 0.0;
	m_camera.worldup[0] = 0.0;
	m_camera.worldup[1] = 1.0;
	m_camera.worldup[2] = 0.0;
	m_camera.position[0] = DEFAULT_IM_X;
	m_camera.position[1] = DEFAULT_IM_Y;
	m_camera.position[2] = DEFAULT_IM_Z;
}

GzRender::~GzRender() {
	/* HW1.2 clean up, free buffer memory */
	delete[]framebuffer;
	delete[]pixelbuffer;
}

int GzRender::GzDefault() {
	/* HW1.3 set pixel buffer to some default values - start a new frame */
	for (int i = 0; i < xres; i++) {
		for (int j = 0; j < yres; j++) {
			pixelbuffer[ARRAY(i, j)].red = 27;
			pixelbuffer[ARRAY(i, j)].green = 40;
			pixelbuffer[ARRAY(i, j)].blue = 79;
			pixelbuffer[ARRAY(i, j)].alpha = 1;
			pixelbuffer[ARRAY(i, j)].z = MAXINT;	//set up background
		}
	}
	return GZ_SUCCESS;
}

int GzRender::GzBeginRender() {
	/* HW 3.7
	- setup for start of each frame - init frame buffer color,alpha,z
	- compute Xiw and projection xform Xpi from camera definition
	- init Ximage - put Xsp at base of stack, push on Xpi and Xiw
	- now stack contains Xsw and app can push model Xforms when needed
	*/
	float zDirx = m_camera.lookat[0] - m_camera.position[0];
	float zDiry = m_camera.lookat[1] - m_camera.position[1];
	float zDirz = m_camera.lookat[2] - m_camera.position[2];
	float zlength = sqrt(zDirx * zDirx + zDiry * zDiry + zDirz * zDirz);
	GzCoord z = { zDirx / zlength, zDiry / zlength, zDirz / zlength };	//z of the camera

	//up' = up - (up*Z)*Z
	float k = (m_camera.worldup[0] * z[0]) + (m_camera.worldup[1] * z[1]) + (m_camera.worldup[2] * z[2]);	//up*Z
	GzCoord tempy = { (m_camera.worldup[0] - k * z[0]), (m_camera.worldup[1] - k * z[1]),(m_camera.worldup[2] - k * z[2]) };
	float ylength = sqrt(tempy[0] * tempy[0] + tempy[1] * tempy[1] + tempy[2] * tempy[2]);
	GzCoord y = { tempy[0] / ylength, tempy[1] / ylength ,tempy[2] / ylength };	//normalize Z
	GzCoord x = { (y[1] * z[2] - y[2] * z[1]), (y[2] * z[0] - y[0] * z[2]), (y[0] * z[1] - y[1] * z[0]) };	//X = Y x Z
	GzMatrix tempXiw = {	//build Xiw (world to image(camera))
		x[0], x[1], x[2], -(x[0] * m_camera.position[0] + x[1] * m_camera.position[1] + x[2] * m_camera.position[2]),
		y[0], y[1], y[2], -(y[0] * m_camera.position[0] + y[1] * m_camera.position[1] + y[2] * m_camera.position[2]),
		z[0], z[1], z[2], -(z[0] * m_camera.position[0] + z[1] * m_camera.position[1] + z[2] * m_camera.position[2]),
		0.0,  0.0,  0.0,  1
	};
	memcpy(m_camera.Xiw, tempXiw, sizeof(tempXiw));

	float dReciprocal = tan((m_camera.FOV * PI / 180) / 2);
	GzMatrix tempXpi = {	//from image(camera) to perspective
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, dReciprocal, 0,
		0.0, 0.0, dReciprocal, 1.0
	};
	memcpy(m_camera.Xpi, tempXpi, sizeof(tempXpi));

	GzMatrix Xsi;
	GzMatDotProduct(Xsp, m_camera.Xpi, Xsi);	//compute Xsi (from image/camera to screen)
	GzPushMatrix(Xsi);
	GzPushMatrix(m_camera.Xiw);

	//old, and line 98//
	/*GzPushMatrix(m_camera.Xpi);
	GzPushMatrix(m_camera.Xiw);*/
	//old//

	return GZ_SUCCESS;
}

int GzRender::GzPutCamera(GzCamera camera) {
	/* HW 3.8
	/*- overwrite renderer camera structure with new camera definition
	*/
	m_camera = camera;
	return GZ_SUCCESS;
}

int GzRender::GzCopyMatrix(GzMatrix from, GzMatrix to) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			to[i][j] = from[i][j];
		}
	}
	return GZ_SUCCESS;
}

int GzRender::GzMatDotProduct(GzMatrix mat1, GzMatrix mat2, GzMatrix output) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float newElement = 0;
			for (int k = 0; k < 4; k++) {
				float a = mat1[i][k];
				float b = mat2[k][j];
				newElement = newElement + mat1[i][k] * mat2[k][j];
			}
			output[i][j] = newElement;
		}
	}

	return GZ_SUCCESS;
}

int GzRender::GzPushMatrix(GzMatrix	matrix) {
	/* HW 3.9
	- push a matrix onto the Ximage stack
	- check for stack overflow
	*/
	if (matlevel == -1) {
		GzCopyMatrix(matrix, Ximage[0]);	//base of Ximage is Xsp*Xpi
		GzMatrix identityMat = { 1,0,0,0,
								0,1,0,0,
								0,0,1,0,
								0,0,0,1 };
		GzCopyMatrix(identityMat, Xnorm[0]);	//base of Xnorm is Xidentity
		matlevel++;
	} else if (matlevel >= 99) {
		return  GZ_FAILURE;
	} else {
		GzMatDotProduct(Ximage[matlevel], matrix, Ximage[matlevel + 1]);
		/*matrix[0][3] = 0;
		matrix[1][3] = 0;
		matrix[2][3] = 0;*/
		float k = sqrt(matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1] + matrix[0][2] * matrix[0][2]);
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				matrix[i][j] = matrix[i][j] / k;	//remove scale, keep rotation only
			}
			matrix[i][3] = 0;	//remove translation, keep rotation only
		}
		GzMatDotProduct(Xnorm[matlevel], matrix, Xnorm[matlevel + 1]);
		matlevel++;
	}

	return GZ_SUCCESS;
}

int GzRender::GzPopMatrix() {
	/* HW 3.10
	- pop a matrix off the Ximage stack
	- check for stack underflow
	*/
	if (matlevel >= 0) {
		matlevel--;
	} else {
		return GZ_FAILURE;
	}

	return GZ_SUCCESS;
}

int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z) {
	/* HW1.4 write pixel values into the buffer */
	if (i < 0 || i >= xres) return GZ_FAILURE;
	if (j < 0 || j >= yres) return GZ_FAILURE;
	if (r < 0) { r = 0; } else if (r > 4095) { r = 4095; }
	if (g < 0) { g = 0; } else if (g > 4095) { g = 4095; }
	if (b < 0) { b = 0; } else if (b > 4095) { b = 4095; }

	/*if (z > pixelbuffer[ARRAY(i, j)].z) {
		return GZ_FAILURE;		//skip this pixel
	}*/

	pixelbuffer[ARRAY(i, j)].red = r >> 4;
	pixelbuffer[ARRAY(i, j)].green = g >> 4;
	pixelbuffer[ARRAY(i, j)].blue = b >> 4;
	pixelbuffer[ARRAY(i, j)].z = z;

	return GZ_SUCCESS;
}

int GzRender::GzGet(int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z) {
	/* HW1.5 retrieve a pixel information from the pixel buffer */
	*r = pixelbuffer[ARRAY(i, j)].red;
	*g = pixelbuffer[ARRAY(i, j)].green;
	*b = pixelbuffer[ARRAY(i, j)].blue;
	*a = pixelbuffer[ARRAY(i, j)].alpha;
	*z = pixelbuffer[ARRAY(i, j)].z;

	return GZ_SUCCESS;
}

int GzRender::GzFlushDisplay2File(FILE* outfile) {
	/* HW1.6 write image to ppm file -- "P6 %d %d 255\r" */
	int flag = 0; // 0 for P6-BINARY , 1 for P3 ASCII
	std::string outputString = "";

	if (flag)outputString += "P3 ";
	else outputString += "P6 ";

	outputString = outputString + std::to_string(xres) + " " + std::to_string(yres) + " " + "255\n";
	fputs(outputString.c_str(), outfile);
	for (int j = 0; j < yres; j++) {
		std::string outputLine = "";
		for (int i = 0; i < xres; i++) {
			if (flag) { //ascii
				outputLine = outputLine + std::to_string((int)pixelbuffer[ARRAY(i, j)].red) + " ";
				outputLine = outputLine + std::to_string((int)pixelbuffer[ARRAY(i, j)].green) + " ";
				outputLine = outputLine + std::to_string((int)pixelbuffer[ARRAY(i, j)].blue) + " ";

			} else { //binary
				fputc(pixelbuffer[ARRAY(i, j)].red, outfile);
				fputc(pixelbuffer[ARRAY(i, j)].green, outfile);
				fputc(pixelbuffer[ARRAY(i, j)].blue, outfile);
			}
		}
		outputLine = outputLine + "\n";
		if (flag)fputs(outputLine.c_str(), outfile); //only ascii
	}

	return GZ_SUCCESS;
}

int GzRender::GzFlushDisplay2FrameBuffer() {
	/* HW1.7 write pixels to framebuffer:
		- put the pixels into the frame buffer
		- CAUTION: when storing the pixels into the frame buffer, the order is blue, green, and red
		- NOT red, green, and blue !!!
	*/
	for (int i = 0; i < xres; i++) {
		for (int j = 0; j < yres; j++) {
			framebuffer[ARRAY(i, j) * 3 + 2] = pixelbuffer[ARRAY(i, j)].red;
			framebuffer[ARRAY(i, j) * 3 + 1] = pixelbuffer[ARRAY(i, j)].green;
			framebuffer[ARRAY(i, j) * 3] = pixelbuffer[ARRAY(i, j)].blue;
		}
	}

	return GZ_SUCCESS;
}

/***********************************************/
/* HW2 methods: implement from here */

int GzRender::GzPutAttribute(int numAttributes, GzToken	*nameList, GzPointer *valueList) {
	/* HW 2.1
	-- Set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
	-- In later homeworks set shaders, interpolaters, texture maps, and lights
	*/
	/*
	- GzPutAttribute() must accept the following tokens/values:

	- GZ_RGB_COLOR					GzColor		default flat-shade color
	- GZ_INTERPOLATE				int			shader interpolation mode
	- GZ_DIRECTIONAL_LIGHT			GzLight
	- GZ_AMBIENT_LIGHT            	GzLight		(ignore direction)
	- GZ_AMBIENT_COEFFICIENT		GzColor		Ka reflectance
	- GZ_DIFFUSE_COEFFICIENT		GzColor		Kd reflectance
	- GZ_SPECULAR_COEFFICIENT       GzColor		Ks coef's
	- GZ_DISTRIBUTION_COEFFICIENT   float		spec power
	*/
	for (int i = 0; i < numAttributes; i++) {
		if (nameList[i] == GZ_RGB_COLOR) {
			flatcolor[0] = *(float*)valueList[i];
			flatcolor[1] = *((float*)valueList[i] + 1);
			flatcolor[2] = *((float*)valueList[i] + 2);
			//flatcolor[1] = ((float*)valueList[i])[1];
		} else if (nameList[i] == GZ_DIRECTIONAL_LIGHT) {
			lights[numlights] = *(GzLight*)valueList[i];
			numlights++;
		} else if (nameList[i] == GZ_AMBIENT_LIGHT) {
			ambientlight = *(GzLight*)valueList[i];
		} else if (nameList[i] == GZ_DIFFUSE_COEFFICIENT) {
			Kd[0] = *(float*)valueList[i];
			Kd[1] = *((float*)valueList[i] + 1);
			Kd[2] = *((float*)valueList[i] + 2);
		} else if (nameList[i] == GZ_INTERPOLATE) {
			interp_mode = *(int*)valueList[i];
		} else if (nameList[i] == GZ_AMBIENT_COEFFICIENT) {
			Ka[0] = *(float*)valueList[i];
			Ka[1] = *((float*)valueList[i] + 1);
			Ka[2] = *((float*)valueList[i] + 2);
		} else if (nameList[i] == GZ_SPECULAR_COEFFICIENT) {
			Ks[0] = *(float*)valueList[i];
			Ks[1] = *((float*)valueList[i] + 1);
			Ks[2] = *((float*)valueList[i] + 2);
		} else if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT) {
			spec = *(float*)valueList[i];
		} else if (nameList[i] == GZ_TEXTURE_MAP) {
			tex_fun = (GzTexture)valueList[i];
			if (tex_fun == NULL) {
				hasTextureFunction = false;
			} else {
				hasTextureFunction = true;
			}
		} else if (nameList[i] == GZ_AASHIFTX) {
			aaShiftX = *(float*)valueList[i];
		} else if (nameList[i] == GZ_AASHIFTY) {
			aaShiftY = *((float*)valueList[i]);
		} else if (nameList[i] == GZ_AAWEIGHT) {
			aaWeight = *((float*)valueList[i]);
		}
	}

	return GZ_SUCCESS;
}

int GzRender::GzPutTriangle(int numParts, GzToken *nameList, GzPointer *valueList)
/* numParts - how many names and values */
{
	/* HW 2.2
	-- Pass in a triangle description with tokens and values corresponding to
		  GZ_NULL_TOKEN:		do nothing - no values
		  GZ_POSITION:		3 vert positions in model space
	-- Invoke the rastrizer/scanline framework
	-- Return error code
	*/
	/*
	-- Xform positions of verts using matrix on top of stack
	-- Clip - just discard any triangle with any vert(s) behind view plane
			- optional: test for triangles with all three verts off-screen (trivial frustum cull)
	-- invoke triangle rasterizer
	*/
	for (int i = 0; i < numParts; i++) {
		if (nameList[i] == GZ_POSITION) {	//valuelist[0] is a pointer(void*) to vertexList(GZCoord(float[3])[3])
			sortedVertexList = (GzCoord*)valueList[i];
			for (int i = 0; i < 3; i++) {		//for three points
				float x = 0.0;
				float y = 0.0;
				float z = 0.0;
				float w = 0.0;
				for (int j = 0; j < 3; j++) {
					x = x + Ximage[matlevel][0][j] * sortedVertexList[i][j];
					y = y + Ximage[matlevel][1][j] * sortedVertexList[i][j];
					z = z + Ximage[matlevel][2][j] * sortedVertexList[i][j];
					w = w + Ximage[matlevel][3][j] * sortedVertexList[i][j];
				}
				x = x + Ximage[matlevel][0][3];
				y = y + Ximage[matlevel][1][3];
				z = z + Ximage[matlevel][2][3];
				w = w + Ximage[matlevel][3][3];
				if (z < 0) {
					return GZ_FAILURE;	//if one z<0, skip this tri
				}
				sortedVertexList[i][0] = x / w;
				sortedVertexList[i][1] = y / w;
				sortedVertexList[i][2] = z / w;
			}
		} else if (nameList[i] == GZ_NORMAL) {
			normalList = (GzCoord*)valueList[i];
			for (int i = 0; i < 3; i++) {		//for three points, calculate new normal
				float x = 0.0;
				float y = 0.0;
				float z = 0.0;
				for (int j = 0; j < 3; j++) {	//Xnorm is rotation only, no translation
					x = x + Xnorm[matlevel][0][j] * normalList[i][j];
					y = y + Xnorm[matlevel][1][j] * normalList[i][j];
					z = z + Xnorm[matlevel][2][j] * normalList[i][j];
				}
				normalList[i][0] = x;
				normalList[i][1] = y;
				normalList[i][2] = z;
			}
		} else if (nameList[i] == GZ_TEXTURE_INDEX) {
			uvList = (GzTextureIndex*)valueList[i];
		}
	}

	GzInvokeRastrizer();

	return GZ_SUCCESS;
}

int GzRender::GzInvokeRastrizer() {

	for (int v = 0; v < 3; v++) {
		sortedVertexList[v][0] = sortedVertexList[v][0] + aaShiftX;
		sortedVertexList[v][1] = sortedVertexList[v][1] + aaShiftY;
	}

	GzColor vertexColor[3];
	if (interp_mode == GZ_FLAT) {
		CalculateColorWithNormal(normalList[0], vertexColor[0]);	//use first normal
	}
	//sort vertexs according to y coord//
	for (int i = 0; i < 2; i++) {		//sortedVertexList is a GzCoord*.sortedVertexList[j] is a float* 
		for (int j = 0; j < 2 - i; j++) {
			if (sortedVertexList[j][1] > sortedVertexList[j + 1][1]) {
				GzCoord tempVertex = { sortedVertexList[j][0], sortedVertexList[j][1], sortedVertexList[j][2] };
				GzCoord tempNormal = { normalList[j][0], normalList[j][1],normalList[j][2] };
				GzTextureIndex tempuv = { uvList[j][0], uvList[j][1] };
				sortedVertexList[j][0] = sortedVertexList[j + 1][0];
				sortedVertexList[j][1] = sortedVertexList[j + 1][1];
				sortedVertexList[j][2] = sortedVertexList[j + 1][2];
				sortedVertexList[j + 1][0] = tempVertex[0];
				sortedVertexList[j + 1][1] = tempVertex[1];
				sortedVertexList[j + 1][2] = tempVertex[2];

				normalList[j][0] = normalList[j + 1][0];
				normalList[j][1] = normalList[j + 1][1];
				normalList[j][2] = normalList[j + 1][2];
				normalList[j + 1][0] = tempNormal[0];
				normalList[j + 1][1] = tempNormal[1];
				normalList[j + 1][2] = tempNormal[2];

				uvList[j][0] = uvList[j + 1][0];
				uvList[j][1] = uvList[j + 1][1];
				uvList[j + 1][0] = tempuv[0];
				uvList[j + 1][1] = tempuv[1];
			}
		}
	}
	//sort vertexs according to y coord//

	float yMin = sortedVertexList[0][1];
	float yMax = sortedVertexList[2][1];
	float xMin = sortedVertexList[0][0];
	float xMax = sortedVertexList[0][0];
	if (sortedVertexList[1][0] > xMax) {
		xMax = sortedVertexList[1][0];
	} else if (sortedVertexList[1][0] < xMin) {
		xMin = sortedVertexList[1][0];
	}
	if (sortedVertexList[2][0] > xMax) {
		xMax = sortedVertexList[2][0];
	} else if (sortedVertexList[2][0] < xMin) {
		xMin = sortedVertexList[2][0];
	}

	Edge edge12, edge13, edge23;
	Edge* edgeList[3] = { &edge12, &edge13, &edge23 };
	edgeList[0]->startVertex = 1;
	edgeList[0]->endVertex = 2;
	edgeList[1]->startVertex = 1;
	edgeList[1]->endVertex = 3;
	edgeList[2]->startVertex = 2;
	edgeList[2]->endVertex = 3;

	for (Edge* edge : edgeList) {		//initial each edge
		if (edge->startVertex == 1 && edge->endVertex == 3) {
			edge->edgeA = sortedVertexList[edge->endVertex - 1][1] - sortedVertexList[edge->startVertex - 1][1];
			edge->edgeB = -(sortedVertexList[edge->endVertex - 1][0] - sortedVertexList[edge->startVertex - 1][0]);
			edge->edgeC = -edge->edgeB*sortedVertexList[edge->startVertex - 1][1] - edge->edgeA*sortedVertexList[edge->startVertex - 1][0];
			edge->isRightEdge = false;
			edge->isTopEdge = false;
		} else {
			edge->edgeA = sortedVertexList[edge->startVertex - 1][1] - sortedVertexList[edge->endVertex - 1][1];
			edge->edgeB = -(sortedVertexList[edge->startVertex - 1][0] - sortedVertexList[edge->endVertex - 1][0]);
			edge->edgeC = -edge->edgeB*sortedVertexList[edge->endVertex - 1][1] - edge->edgeA*sortedVertexList[edge->endVertex - 1][0];
			edge->isRightEdge = false;
			edge->isTopEdge = false;
		}
	}

	bool hasHorizontalEdge = false;
	for (int i = 0; i < 3; i++) {
		if (sortedVertexList[edgeList[i]->startVertex - 1][1] == sortedVertexList[edgeList[i]->endVertex - 1][1]) {
			hasHorizontalEdge = true;
			if (i = 1) {
				return GZ_FAILURE;		//edge13 is horizontal
			} else if (i = 0) {			//edge12 is horizontal
				edge12.isTopEdge = true;
				if (sortedVertexList[0][0] == sortedVertexList[1][0]) {
					return GZ_FAILURE;
				} else if (sortedVertexList[0][0] > sortedVertexList[1][0]) {
					edge13.isRightEdge = true;
				} else {
					edge23.isRightEdge = true;
				}
			} else {		//i=2
				if (sortedVertexList[1][0] == sortedVertexList[2][0]) {
					return GZ_FAILURE;
				} else if (sortedVertexList[1][0] > sortedVertexList[2][0]) {
					edge12.isRightEdge = true;
				} else {
					edge13.isRightEdge = true;
				}
			}
		} else {
			if (hasHorizontalEdge) {
				continue;
			}
			float slopex12 = -(edge12.edgeB / edge12.edgeA);
			float slopex13 = -(edge13.edgeB / edge13.edgeA);
			if (slopex12 > slopex13) {
				edge12.isRightEdge = true;
				edge23.isRightEdge = true;
			} else {
				edge13.isRightEdge = true;
			}
		}
	}

	float vector12[3] = { sortedVertexList[1][0] - sortedVertexList[0][0],
							sortedVertexList[1][1] - sortedVertexList[0][1],
							sortedVertexList[1][2] - sortedVertexList[0][2] };
	float vector13[3] = { sortedVertexList[2][0] - sortedVertexList[0][0],
							sortedVertexList[2][1] - sortedVertexList[0][1],
							sortedVertexList[2][2] - sortedVertexList[0][2] };
	//cross product to get A,B,C
	float planeA = vector12[1] * vector13[2] - vector12[2] * vector13[1];
	float planeB = vector12[2] * vector13[0] - vector12[0] * vector13[2];
	float planeC = vector12[0] * vector13[1] - vector12[1] * vector13[0];
	float planeD = -(planeA*sortedVertexList[0][0] + planeB * sortedVertexList[0][1] + planeC * sortedVertexList[0][2]);

	//prespective and normal correction, to prespective//
	for (int i = 0; i < 3; i++) {
		float vzCoefficient = sortedVertexList[i][2] / (MAXINT - sortedVertexList[i][2]);
		if (1) {
			uvList[i][0] = uvList[i][0] / (vzCoefficient + 1);
			uvList[i][1] = uvList[i][1] / (vzCoefficient + 1);
		}
		normalList[i][0] = normalList[i][0] / (vzCoefficient + 1);
		normalList[i][1] = normalList[i][1] / (vzCoefficient + 1);
		normalList[i][2] = normalList[i][2] / (vzCoefficient + 1);
	}
	//prespective and normal correction//

	GzColor gColor[3];
	if (interp_mode == GZ_COLOR) {
		CalculateColorWithoutKt(normalList[0], gColor[0]);
		CalculateColorWithoutKt(normalList[1], gColor[1]);
		CalculateColorWithoutKt(normalList[2], gColor[2]);

	}

	for (int x = floor(xMin); x <= ceil(xMax); x++) {
		for (int y = floor(yMin); y <= ceil(yMax); y++) {
			bool shouldDraw = false;

			//z-buffer and screen test//
			GzDepth z = -(planeA * x + planeB * y + planeD) / planeC;
			if (x < 0 || y < 0 || x > xres-1 || y > yres-1) { continue; }
			if (z >= pixelbuffer[ARRAY(x, y)].z || z < 0) { continue; }
			//z-buffer and screen test//

			//decide if this point should draw//
			float e[3];
			bool onEdge = false;
			e[0] = edge12.edgeA*x + edge12.edgeB*y + edge12.edgeC;
			e[1] = edge13.edgeA*x + edge13.edgeB*y + edge13.edgeC;
			e[2] = edge23.edgeA*x + edge23.edgeB*y + edge23.edgeC;
			for (int i = 0; i < 3; i++) {
				if (e[i] == 0) {	//the point is on one of the edge
					onEdge = true;
					if (edgeList[i]->isRightEdge || edgeList[i]->isTopEdge) {
						shouldDraw = true;
						break;
					}
				}
			}
			if (!onEdge) {
				if (e[0] > 0 && e[1] > 0 && e[2] > 0) {
					shouldDraw = true;
				} else if (e[0] < 0 && e[1] < 0 && e[2] < 0) {
					shouldDraw = true;
				}
			}
			if (shouldDraw == false) { continue; }
			//decide if this point should draw//

			float u;
			float v;
			float u2;
			float v2;
			GzColor textureColor = { 0, 0, 0 };
			GzColor textureColor2 = { 0, 0, 0 };
			float pixelNormal[3];
			//calculate normal for the pixel//
			for (int coord = 0; coord < 3; coord++) {	//for Nx, Ny and Nz
				float normalVector12[3] = { sortedVertexList[1][0] - sortedVertexList[0][0],
						sortedVertexList[1][1] - sortedVertexList[0][1],
						normalList[1][coord] - normalList[0][coord] };	//vector for 1-2
				float normalVector13[3] = { sortedVertexList[2][0] - sortedVertexList[0][0],
						sortedVertexList[2][1] - sortedVertexList[0][1],
						normalList[2][coord] - normalList[0][coord] };	//vector for 1-3
				float normalPlaneA = normalVector12[1] * normalVector13[2] - normalVector12[2] * normalVector13[1];
				float normalPlaneB = normalVector12[2] * normalVector13[0] - normalVector12[0] * normalVector13[2];
				float normalPlaneC = normalVector12[0] * normalVector13[1] - normalVector12[1] * normalVector13[0];
				float normalPlaneD = -(normalPlaneA*sortedVertexList[0][0] + normalPlaneB * sortedVertexList[0][1] + normalPlaneC * normalList[0][coord]);
				pixelNormal[coord] = -(normalPlaneA * x + normalPlaneB * y + normalPlaneD) / normalPlaneC;
			}

			//normal correction, to image//
			float pixelVZCoefficient = (float)z / (MAXINT - z);
			pixelNormal[0] = pixelNormal[0] * (pixelVZCoefficient + 1);
			pixelNormal[1] = pixelNormal[1] * (pixelVZCoefficient + 1);
			pixelNormal[2] = pixelNormal[2] * (pixelVZCoefficient + 1);
			//normal correction//

			float k = sqrt(pixelNormal[0] * pixelNormal[0] + pixelNormal[1] * pixelNormal[1] + pixelNormal[2] * pixelNormal[2]);
			pixelNormal[0] = pixelNormal[0] / k;
			pixelNormal[1] = pixelNormal[1] / k;
			pixelNormal[2] = pixelNormal[2] / k;
			//calculate normal for the pixel//

			char targetTexture;
			if (!isEnvironmentMapping) {
				//texture mapping//
				for (int uvCoord = 0; uvCoord < 2; uvCoord++) {		//for u and v
					float uvVector12[3] = { sortedVertexList[1][0] - sortedVertexList[0][0],
								sortedVertexList[1][1] - sortedVertexList[0][1],
								uvList[1][uvCoord] - uvList[0][uvCoord] };
					float uvVector13[3] = { sortedVertexList[2][0] - sortedVertexList[0][0],
								sortedVertexList[2][1] - sortedVertexList[0][1],
								uvList[2][uvCoord] - uvList[0][uvCoord] };
					float uvPlaneA = uvVector12[1] * uvVector13[2] - uvVector12[2] * uvVector13[1];
					float uvPlaneB = uvVector12[2] * uvVector13[0] - uvVector12[0] * uvVector13[2];
					float uvPlaneC = uvVector12[0] * uvVector13[1] - uvVector12[1] * uvVector13[0];
					float uvPlaneD = -(uvPlaneA*sortedVertexList[0][0] + uvPlaneB * sortedVertexList[0][1] + uvPlaneC * uvList[0][uvCoord]);
					if (uvCoord == 0) {
						u2 = -(uvPlaneA * x + uvPlaneB * y + uvPlaneD) / uvPlaneC;
					} else {
						v2 = -(uvPlaneA * x + uvPlaneB * y + uvPlaneD) / uvPlaneC;
					}
				}
				//texture mapping//
			} else {
				//texture mapping//
				for (int uvCoord = 0; uvCoord < 2; uvCoord++) {		//for u and v
					float uvVector12[3] = { sortedVertexList[1][0] - sortedVertexList[0][0],
								sortedVertexList[1][1] - sortedVertexList[0][1],
								uvList[1][uvCoord] - uvList[0][uvCoord] };
					float uvVector13[3] = { sortedVertexList[2][0] - sortedVertexList[0][0],
								sortedVertexList[2][1] - sortedVertexList[0][1],
								uvList[2][uvCoord] - uvList[0][uvCoord] };
					float uvPlaneA = uvVector12[1] * uvVector13[2] - uvVector12[2] * uvVector13[1];
					float uvPlaneB = uvVector12[2] * uvVector13[0] - uvVector12[0] * uvVector13[2];
					float uvPlaneC = uvVector12[0] * uvVector13[1] - uvVector12[1] * uvVector13[0];
					float uvPlaneD = -(uvPlaneA*sortedVertexList[0][0] + uvPlaneB * sortedVertexList[0][1] + uvPlaneC * uvList[0][uvCoord]);
					if (uvCoord == 0) {
						u2 = -(uvPlaneA * x + uvPlaneB * y + uvPlaneD) / uvPlaneC;
					}
					else {
						v2 = -(uvPlaneA * x + uvPlaneB * y + uvPlaneD) / uvPlaneC;
					}
				}
				//texture mapping//



				//environment mapping//

				//hit point in perspective space//
				float hitPoint[3] = { ((float)2 / xres)*x - 1,
												((float)-2 / yres)*y + 1,
												((float)z / MAXINT)
				};
				//hit point in perspective space//

				//hit point in image space//
				hitPoint[0] = hitPoint[0] * (pixelVZCoefficient + 1);
				hitPoint[1] = hitPoint[1] * (pixelVZCoefficient + 1);
				hitPoint[2] = hitPoint[2] * (pixelVZCoefficient + 1);
				//hit point in image space//

				//calculate reflection vector//
				/*hitPoint[0] = 0;
				hitPoint[1] = 0;
				hitPoint[2] = 0;*/
				float eyeSight[3] = { -hitPoint[0],-hitPoint[1], -hitPoint[2] };
				float reflectVector[3] = { 0,0,0 };

				float enDotPro = pixelNormal[0] * eyeSight[0] + pixelNormal[1] * eyeSight[1] + pixelNormal[2] * eyeSight[2];
				if (enDotPro < 0) {
					pixelNormal[0] = -pixelNormal[0];
					pixelNormal[1] = -pixelNormal[1];
					pixelNormal[2] = -pixelNormal[2];	//flip the normal
					enDotPro = -enDotPro;
				}
				reflectVector[0] = 2 * enDotPro*pixelNormal[0] - eyeSight[0];
				reflectVector[1] = 2 * enDotPro*pixelNormal[1] - eyeSight[1];
				reflectVector[2] = 2 * enDotPro*pixelNormal[2] - eyeSight[2];
				//calculate reflection vector//

				float absX = abs(reflectVector[0]);
				float absY = abs(reflectVector[1]);
				float absZ = abs(reflectVector[2]);

				//注释的是尝试在反射向量上加位移的时候写的，现在位移加在了入射向量上，所以用不到了
				//float originOfReflectVector[3] = { 0,0,0 };
				float maxAxis;
				float uc;
				float vc;

				if (reflectVector[0] > 0 && absX >= absY && absX >= absZ) {
					targetTexture = 'r';	//u=-z+0.5,v=-y+0.5
					/*float planex = 0.5;
					float hitz = ((planex - originOfReflectVector[0]) / reflectVector[0])*reflectVector[2] + originOfReflectVector[2];
					float hity = ((planex - originOfReflectVector[0]) / reflectVector[0])*reflectVector[1] + originOfReflectVector[1];
					u = -hitz + 0.5;
					v = -hity + 0.5;*/
					maxAxis = absX;
					uc = -reflectVector[2];		//uc = -z
					vc = -reflectVector[1];		//vc = -y
				} else if (reflectVector[0] <= 0 && absX >= absY && absX >= absZ) {
					targetTexture = 'l';	//u=z+0.5,v=-y+0.5
					/*float planex = -0.5;
					float hitz = ((planex - originOfReflectVector[0]) / reflectVector[0])*reflectVector[2] + originOfReflectVector[2];
					float hity = ((planex - originOfReflectVector[0]) / reflectVector[0])*reflectVector[1] + originOfReflectVector[1];
					u = hitz + 0.5;
					v = -hity + 0.5;*/
					maxAxis = absX;
					uc = reflectVector[2];		//uc = z
					vc = -reflectVector[1];		//vc = -y
				} else if (reflectVector[2] > 0 && absZ>=absX && absZ>=absY) {
					targetTexture = 'f';	//u=x+0.5,v=-y+0.5
					/*float planez = 0.5;
					float hitx = ((planez - originOfReflectVector[2]) / reflectVector[2])*reflectVector[0] + originOfReflectVector[0];
					float hity = ((planez - originOfReflectVector[2]) / reflectVector[2])*reflectVector[1] + originOfReflectVector[1];
					u = hitx + 0.5;
					v = -hity + 0.5;*/
					maxAxis = absZ;
					uc = reflectVector[0];		//uc = X
					vc = -reflectVector[1];		//vc = -y
				} else if (reflectVector[2] <= 0 && absZ >= absX && absZ >= absY) {
					targetTexture = 'b';	//u=-x+0.5,v=-y+0.5
					/*float planez = -0.5;
					float hitx = ((planez - originOfReflectVector[2]) / reflectVector[2])*reflectVector[0] + originOfReflectVector[0];
					float hity = ((planez - originOfReflectVector[2]) / reflectVector[2])*reflectVector[1] + originOfReflectVector[1];
					u = -hitx + 0.5;
					v = -hity + 0.5;*/
					maxAxis = absZ;
					uc = -reflectVector[0];		//uc = -x
					vc = -reflectVector[1];		//vc = -y
				} else if (reflectVector[1] > 0 && absY>=absX && absY>=absZ) {
					targetTexture = 'u';	//u=x+0.5,v=z+0.5
					/*float planey = 0.5;
					float hitx = ((planey - originOfReflectVector[1]) / reflectVector[1])*reflectVector[0] + originOfReflectVector[0];
					float hitz = ((planey - originOfReflectVector[1]) / reflectVector[1])*reflectVector[2] + originOfReflectVector[2];
					u = hitx + 0.5;
					v = hitz + 0.5;*/
					maxAxis = absY;
					uc = reflectVector[0];		//uc = x
					vc = reflectVector[2];		//vc = z
				} else if (reflectVector[1] <= 0 && absY >= absX && absY >= absZ) {
					targetTexture = 'd';	//u=x+0.5,v=-z+0.5
					/*float planey = -0.5;
					float hitx = ((planey - originOfReflectVector[1]) / reflectVector[1])*reflectVector[0] + originOfReflectVector[0];
					float hitz = ((planey - originOfReflectVector[1]) / reflectVector[1])*reflectVector[2] + originOfReflectVector[2];
					u = hitx + 0.5;
					v = -hitz + 0.5;*/
					maxAxis = absY;
					uc = reflectVector[0];		//uc = x
					vc = -reflectVector[2];		//vc = -z
				}
				u = 0.5f * (uc / maxAxis + 1.0f);
				v = 0.5f * (vc / maxAxis + 1.0f);

				if (u < 0 || u > 1 || v < 0 || v>1) {
					int a = 0;
				}
				//environment mapping
			}

			//prespective correct//
			if (!isEnvironmentMapping) {
				float pixelVZCoefficient = (float)z / (MAXINT - z);
				u = u * (pixelVZCoefficient + 1);
				v = v * (pixelVZCoefficient + 1);
			}
			else {
				float pixelVZCoefficient = (float)z / (MAXINT - z);
				u2 = u2 * (pixelVZCoefficient + 1);
				v2 = v2 * (pixelVZCoefficient + 1);
			}
			//prespective correct//

			//uv mapping//
			//if (u < 0 || u>1 || v < 0 || v>1) { continue; }
			if (isEnvironmentMapping) {
				etex_fun(u, v, textureColor, targetTexture);
				tex_fun(u2, v2, textureColor2);
			} else {
				if (hasTextureFunction) {
					tex_fun(u, v, textureColor);
				}
			}
			//uv mapping//

			GzColor pixelColor = { 0,0,0 };
			if (interp_mode == GZ_FLAT) {
				if (hasTextureFunction) {
					pixelColor[0] = vertexColor[0][0];
					pixelColor[1] = vertexColor[0][1];
					pixelColor[2] = vertexColor[0][2];
				}
			} else if (interp_mode == GZ_COLOR) {
				for (int i = 0; i < 3; i++) {	//for RGB
					vertexColor[0][i] = textureColor[i] * gColor[0][i];
					vertexColor[1][i] = textureColor[i] * gColor[1][i];
					vertexColor[2][i] = textureColor[i] * gColor[2][i];
					if (vertexColor[0][i] > 1) { vertexColor[0][i] = 1; }
					if (vertexColor[1][i] > 1) { vertexColor[1][i] = 1; }
					if (vertexColor[2][i] > 1) { vertexColor[2][i] = 1; }
				}
				for (int c = 0; c < 3; c++) {	//for RGB
					float colorVector12[3] = { sortedVertexList[1][0] - sortedVertexList[0][0],
							sortedVertexList[1][1] - sortedVertexList[0][1],
							vertexColor[1][c] - vertexColor[0][c] };	//vector for 1-2
					float colorVector13[3] = { sortedVertexList[2][0] - sortedVertexList[0][0],
							sortedVertexList[2][1] - sortedVertexList[0][1],
							vertexColor[2][c] - vertexColor[0][c] };	//vector for 1-3
					float colorPlaneA = colorVector12[1] * colorVector13[2] - colorVector12[2] * colorVector13[1];
					float colorPlaneB = colorVector12[2] * colorVector13[0] - colorVector12[0] * colorVector13[2];
					float colorPlaneC = colorVector12[0] * colorVector13[1] - colorVector12[1] * colorVector13[0];
					float colorPlaneD = -(colorPlaneA*sortedVertexList[0][0] + colorPlaneB * sortedVertexList[0][1] + colorPlaneC * vertexColor[0][c]);
					pixelColor[c] = -(colorPlaneA * x + colorPlaneB * y + colorPlaneD) / colorPlaneC;
				}
			} else if (interp_mode == GZ_NORMALS) {
				if (hasTextureFunction) {
					for (int i = 0; i < 3; i++) {
						Kd[i] = 0.5 * textureColor[i] + 0.5 * textureColor2[i];
						Ka[i] = 0.5 * textureColor[i] + 0.5 * textureColor2[i];
					}
				}
				CalculateColorWithNormal(pixelNormal, pixelColor);
			}

			GzPut(x, y, ctoi(pixelColor[0]), ctoi(pixelColor[1]), ctoi(pixelColor[2]), 1, z);
		}
	}
	return GZ_SUCCESS;
}

int GzRender::CalculateColorWithNormal(GzCoord normal, GzColor color) {
	//GzCoord normal = { normalList[i][0] ,normalList[i][1] ,normalList[i][2] };
	GzColor specularComponent = { 0,0,0 };
	GzColor diffuseComponent = { 0,0,0 };
	for (int j = 0; j < numlights; j++) {
		float nlDotProduct = normal[0] * lights[j].direction[0] + normal[1] * lights[j].direction[1] + normal[2] * lights[j].direction[2];
		float neDotProduct = -normal[2];
		if (nlDotProduct > 0 && neDotProduct > 0) {
			//do nothing
		} else if (nlDotProduct < 0 && neDotProduct < 0) {	//flip normal
			normal[0] = -normal[0];
			normal[1] = -normal[1];
			normal[2] = -normal[2];
		} else {		//skip this light
			continue;
		}
		nlDotProduct = normal[0] * lights[j].direction[0] + normal[1] * lights[j].direction[1] + normal[2] * lights[j].direction[2];
		float rVector[3] = { 2 * nlDotProduct*normal[0] - lights[j].direction[0],
							2 * nlDotProduct*normal[1] - lights[j].direction[1] ,
							2 * nlDotProduct*normal[2] - lights[j].direction[2] };
		float reDotProduct = -rVector[2];
		if (reDotProduct < 0) {
			reDotProduct = 0;
		} else if (reDotProduct > 1) {
			reDotProduct = 1;
		}
		float rePowSpec = pow(reDotProduct, spec);
		specularComponent[0] = specularComponent[0] + lights[j].color[0] * rePowSpec;
		specularComponent[1] = specularComponent[1] + lights[j].color[1] * rePowSpec;
		specularComponent[2] = specularComponent[2] + lights[j].color[2] * rePowSpec;

		diffuseComponent[0] = diffuseComponent[0] + lights[j].color[0] * nlDotProduct;
		diffuseComponent[1] = diffuseComponent[1] + lights[j].color[1] * nlDotProduct;
		diffuseComponent[2] = diffuseComponent[2] + lights[j].color[2] * nlDotProduct;
	}

	specularComponent[0] = specularComponent[0] * Ks[0];
	specularComponent[1] = specularComponent[1] * Ks[1];
	specularComponent[2] = specularComponent[2] * Ks[2];

	diffuseComponent[0] = diffuseComponent[0] * Kd[0];
	diffuseComponent[1] = diffuseComponent[1] * Kd[1];
	diffuseComponent[2] = diffuseComponent[2] * Kd[2];

	GzColor ambientComponent = { ambientlight.color[0] * Ka[0],
								ambientlight.color[1] * Ka[1],
								ambientlight.color[2] * Ka[2] };

	color[0] = specularComponent[0] + diffuseComponent[0] + ambientComponent[0];
	color[1] = specularComponent[1] + diffuseComponent[1] + ambientComponent[1];
	color[2] = specularComponent[2] + diffuseComponent[2] + ambientComponent[2];

	for (int j = 0; j < 3; j++) {
		if (color[j] > 1) {
			color[j] = 1;
		}
	}


	return GZ_SUCCESS;
}

int GzRender::CalculateColorWithoutKt(GzCoord normal, GzColor color) {
	//GzCoord normal = { normalList[i][0] ,normalList[i][1] ,normalList[i][2] };
	GzColor specularComponent = { 0,0,0 };
	GzColor diffuseComponent = { 0,0,0 };
	for (int j = 0; j < numlights; j++) {
		float nlDotProduct = normal[0] * lights[j].direction[0] + normal[1] * lights[j].direction[1] + normal[2] * lights[j].direction[2];
		float neDotProduct = -normal[2];
		if (nlDotProduct > 0 && neDotProduct > 0) {
			//do nothing
		} else if (nlDotProduct < 0 && neDotProduct < 0) {	//flip normal
			normal[0] = -normal[0];
			normal[1] = -normal[1];
			normal[2] = -normal[2];
		} else {		//skip this light
			continue;
		}
		nlDotProduct = normal[0] * lights[j].direction[0] + normal[1] * lights[j].direction[1] + normal[2] * lights[j].direction[2];
		float rVector[3] = { 2 * nlDotProduct*normal[0] - lights[j].direction[0],
							2 * nlDotProduct*normal[1] - lights[j].direction[1] ,
							2 * nlDotProduct*normal[2] - lights[j].direction[2] };
		float reDotProduct = -rVector[2];
		if (reDotProduct < 0) {
			reDotProduct = 0;
		} else if (reDotProduct > 1) {
			reDotProduct = 1;
		}
		float rePowSpec = pow(reDotProduct, spec);
		specularComponent[0] = specularComponent[0] + lights[j].color[0] * rePowSpec;
		specularComponent[1] = specularComponent[1] + lights[j].color[1] * rePowSpec;
		specularComponent[2] = specularComponent[2] + lights[j].color[2] * rePowSpec;

		diffuseComponent[0] = diffuseComponent[0] + lights[j].color[0] * nlDotProduct;
		diffuseComponent[1] = diffuseComponent[1] + lights[j].color[1] * nlDotProduct;
		diffuseComponent[2] = diffuseComponent[2] + lights[j].color[2] * nlDotProduct;
	}
	//specularComponent[0] = specularComponent[0] *Ks[0];
	//specularComponent[1] = specularComponent[1] *Ks[1];
	//specularComponent[2] = specularComponent[2] *Ks[2];

	//diffuseComponent[0] = diffuseComponent[0] * Kd[0];
	//diffuseComponent[1] = diffuseComponent[1] * Kd[1];
	//diffuseComponent[2] = diffuseComponent[2] * Kd[2];

	/*GzColor ambientComponent = { ambientlight.color[0] * Ka[0],
								ambientlight.color[1] * Ka[1],
								ambientlight.color[2] * Ka[2] };*/
	GzColor ambientComponent = { ambientlight.color[0],
								ambientlight.color[1],
								ambientlight.color[2] };

	color[0] = specularComponent[0] + diffuseComponent[0] + ambientComponent[0];
	color[1] = specularComponent[1] + diffuseComponent[1] + ambientComponent[1];
	color[2] = specularComponent[2] + diffuseComponent[2] + ambientComponent[2];

	/*for (int j = 0; j < 3; j++) {
		if (color[j] > 1) {
			color[j] = 1;
		}
	}*/

	return GZ_SUCCESS;
}


int GzRender::CalculateF(GzCoord normal, GzColor color, float material) {

	float n1 = normal[0];
	float n2 = normal[1];
	float n3 = normal[2];
	GzColor ret = { 0,0,0 };
	float k;
	float eta = 1/material;

	GzCoord E = { 0, 0, -1 };
	float NE = n1 * E[0] + n2 * E[1] + n3 * E[2];
	float NL, RE, Rmod;
	GzCoord R;
	float sumRE[3] = { 0,0,0 };
	float sumNL[3] = { 0,0,0 };

	for (int ii = 0; ii < this->numlights; ii++) {
		NL = n1 * this->lights[ii].direction[0] + n2 * this->lights[ii].direction[1] + n3 * this->lights[ii].direction[2];


		// k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
		k = 1.f - (eta * eta * (1.f - NL * NL));
		if (k < 0) {
			R[0] = R[1] = R[2] = 0;
		}
		else {
			//R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;
			if (NE > 0 && NL > 0) {
				R[0] = eta * this->lights[ii].direction[0] - (eta * NL + sqrt(k)) * n1;
				R[1] = eta * this->lights[ii].direction[1] - (eta * NL + sqrt(k)) * n2;
				R[2] = eta * this->lights[ii].direction[2] - (eta * NL + sqrt(k)) * n3;
			}
			else if (NE < 0 && NL < 0) {
				NL = (-n1) * this->lights[ii].direction[0] + (-n2) * this->lights[ii].direction[1] + (-n3) * this->lights[ii].direction[2];
				R[0] = eta * this->lights[ii].direction[0] - (eta * NL + sqrt(k)) * (-n1);
				R[1] = eta * this->lights[ii].direction[1] - (eta * NL + sqrt(k)) * (-n2);
				R[2] = eta * this->lights[ii].direction[2] - (eta * NL + sqrt(k)) * (-n3);
			}
			else {
				//skip
				continue;
			}

		}

		Rmod = sqrt(R[X] * R[X] + R[Y] * R[Y] + R[Z] * R[Z]);
		R[X] = R[X] / Rmod;
		R[Y] = R[Y] / Rmod;
		R[Z] = R[Z] / Rmod;

		RE = -R[Z];

		//RE = R[X] * E[0] + R[Y] * E[1] + R[Z] * E[2];
		// 0<RE<1

		if (RE > 1) {
			RE = 1;
		}
		if (RE < 0) {
			RE = 0;
		}
		//(Ks sum[Ie (RE)^s]) + (Kd sum[Ie (NL)]) + (Ka Ia) 
		//Ie = RGB

		for (int kk = 0; kk < 3; kk++) {
			sumRE[kk] += pow(RE, this->spec) * this->lights[ii].color[kk];
			sumNL[kk] += NL * this->lights[ii].color[kk];
		}
	}

	for (int jj = 0; jj < 3; jj++) {
		color[jj] = this->Ks[jj] * sumRE[jj] + this->Kd[jj] * sumNL[jj] + this->Ka[jj] * this->ambientlight.color[jj];
	}


	return GZ_SUCCESS;
}