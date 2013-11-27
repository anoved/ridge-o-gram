#include <stdio.h>

#include <libtrix.h>
#include "stb_image.h"

typedef enum {
	WEST,
	EAST
} ridge_slope;

typedef struct {
	int w, h;
	unsigned char *data;
} bitmap;

unsigned char threshold = 155;

// add one face of the ridge at x y to the given mesh
void face(trix_mesh *mesh, int x, int y, ridge_slope face) {
	trix_vertex v0, v1, v2, v3;
	trix_triangle t0, t1;
	
	if (face == WEST) {
	
		v0.x = (float)x;       v0.y = (float)y;       v0.z = 0.0;
		v1.x = (float)x + 0.5; v1.y = (float)y;       v1.z = 1.0;
		v2.x = (float)x + 0.5; v2.y = (float)y + 1.0; v2.z = 1.0;
		v3.x = (float)x;       v3.y = (float)y + 1.0; v3.z = 0.0;
	
	} else {
		
		v0.x = (float)x + 0.5; v0.y = (float)y + 1.0; v0.z = 1.0;
		v1.x = (float)x + 0.5; v1.y = (float)y;       v1.z = 1.0;
		v2.x = (float)x + 1.0; v2.y = (float)y;       v2.z = 0.0;
		v3.x = (float)x + 1.0; v3.y = (float)y + 1.0; v3.z = 0.0;
		
	}
	
	t0.a = v0; t0.b = v1; t0.c = v3;
	t1.a = v1; t1.b = v2; t1.c = v3;
	
	(void)trixAddTriangle(mesh, &t0);
	(void)trixAddTriangle(mesh, &t1);
}

int MergeImages(void) {
	
	bitmap bWest, bEast;
	trix_mesh *mOn, *mOff;
	int depth;
	unsigned long offset;
	int w, h, x, y;
	
	int onWest, onEast;
	
	// load images (note, we load as grayscale)
	bWest.data = stbi_load("face1b.jpg", &bWest.w, &bWest.h, &depth, 1);
	bEast.data = stbi_load("face2b.jpg", &bEast.w, &bEast.h, &depth, 1);
	if (bWest.data == NULL || bEast.data == NULL) {
		fprintf(stderr, "%s\n", stbi_failure_reason());
		return 1;
	}
	
	// validate that dimensions match
	if (bWest.w != bEast.w || bWest.h != bEast.h) {
		fprintf(stderr, "image dimensions do not match\n");
		return 1;
	}
	w = bWest.w;
	h = bWest.h;
	
	if (trixCreate(&mOn, "onpixels") != TRIX_OK) {
		return 1;
	}
	if (trixCreate(&mOff, "offpixels") != TRIX_OK) {
		return 1;
	}
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
		
			offset = (y * w) + x;
			
			if (bWest.data[offset] > threshold) {
				onWest = 1;
			} else {
				onWest = 0;
			}
			
			if (bEast.data[offset] > threshold) {
				onEast = 1;
			} else {
				onEast = 0;
			}
			
			if (onWest && onEast) {
				// both on
				face(mOn, x, h - y, WEST);
				face(mOn, x, h - y, EAST);
			}
			else if (!onWest && !onEast) {
				// both off
				face(mOff, x, h - y, WEST);
				face(mOff, x, h - y, EAST);
			}
			else if (onWest && !onEast) {
				// west on, east off
				face(mOn, x, h - y, WEST);
				face(mOff, x, h - y, EAST);
			}
			else if (!onWest && onEast) {
				// west off, east on
				face(mOff, x, h - y, WEST);
				face(mOn, x, h - y, EAST);
			}
		
		}
	}
	
	// output
	
	trixWrite(mOn, "rog-on.stl", TRIX_STL_BINARY);
	trixWrite(mOff, "rog-off.stl", TRIX_STL_BINARY);
	
	// cleanup
	
	trixRelease(&mOn);
	trixRelease(&mOff);
	
	stbi_image_free(bWest.data);
	stbi_image_free(bEast.data);
	
	return 0;
}


int main(int argc, char **argv) {
	
	
	if (MergeImages() != 0) {
		return 1;
	}
	
	return 0;
}
