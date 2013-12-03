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

// vertices a, b, c and d should appear in counter-clockwise order as seen from outside the quad.
// rendered as two triangles, abd and bcd.
void quad(trix_mesh *mesh, trix_vertex *a, trix_vertex *b, const trix_vertex *c, const trix_vertex *d) {
	trix_triangle abd, bcd;
	abd.a = *a; abd.b = *b; abd.c = *d;
	bcd.a = *b; bcd.b = *c; bcd.c = *d;
	trixAddTriangle(mesh, &abd);
	trixAddTriangle(mesh, &bcd);
}

void tri(trix_mesh *mesh, trix_vertex *a, trix_vertex *b, trix_vertex *c) {
	trix_triangle abc;
	abc.a = *a; abc.b = *b; abc.c = *c;
	trixAddTriangle(mesh, &abc);
}

#define q(a, b, c, d) quad(mesh, &v[(a)], &v[(b)], &v[(c)], &v[(d)])
#define t(a, b, c)    tri(mesh, &v[(a)], &v[(b)], &v[(c)])

void on_a(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
	// ridge top
	q(0, 1, 2, 3);
	q(3, 2, 4, 5);
	
	// rings should always probably include midpoints,
	// to avoid t-junctions with neighbors of type c or d
	
	// ridge bottom ring

	q(0, 9, 6, 1);
	q(1, 6, 24, 26);
	q(26, 24, 7, 4);
	q(4, 7, 8, 5);
	q(5, 8, 25, 27);
	q(27, 25, 9, 0);
	
	// trunk west, south, east, north
	q(9, 13, 10, 6);
	q(6, 10, 11, 7);
	q(7, 11, 12, 8);
	q(8, 12, 13, 9);
	
	// base top ring
	q(17, 14, 10, 13);
	q(14, 15, 11, 10);
	q(15, 16, 12, 11);
	q(16, 17, 13, 12);
	
	// base bottom
	q(18, 21, 20, 19);
	
	// nw
	if (caps[0]) {
		t(3, 27, 0);
	}
	// ne
	if (caps[1]) {
		t(5, 27, 3);
	}
	// sw
	if (caps[2]) {
		t(2, 1, 26);
	}
	// se
	if (caps[3]) {
		t(4, 2, 26);
	}

}

void off_a(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
	// top ring
	q(0, 1, 6, 9);
	q(6, 1, 26, 24);
	q(24, 26, 4, 7);
	q(7, 4, 5, 8);
	q(5, 27, 25, 8);
	q(0, 9, 25, 27);
	
	// stem cavity
	q(6, 10, 13, 9);
	q(25, 9, 13, 23);
	q(25, 13, 12, 8);
	q(8, 12, 11, 7);
	q(24, 7, 11, 22);
	q(24, 22, 10, 6);
	
	// bottom ring
	q(17, 13, 10, 14);
	q(10, 22, 28, 14);
	q(22, 11, 15, 28);
	q(12, 16, 15, 11);
	q(29, 16, 12, 23);
	q(17, 29, 23, 13);
	
	// no caps
}

void on_b(trix_mesh *mesh, trix_vertex *v, int *caps) {

	// top
	q(17, 14, 15, 16);
	
	// bottom
	q(21, 20, 19, 18);

	// no caps

}

void off_b(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
	// ridge top
	q(0, 1, 2, 3);
	q(3, 2, 4, 5);
	
	// bottom
	q(17, 29, 28, 14);
	q(29, 16, 15, 28);
	
	// nw
	if (caps[0]) {
		t(3, 27, 0);
	}
	// ne
	if (caps[1]) {
		t(5, 27, 3);
	}
	// sw
	if (caps[2]) {
		t(2, 1, 26);
	}
	// se
	if (caps[3]) {
		t(4, 2, 26);
	}

}

void on_c(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
	// half ridge top
	q(0, 1, 2, 3);
	t(2, 26, 24);
	q(2, 24, 25, 3);
	t(3, 25, 27);
	
	// ridge bottom
	q(0, 9, 6, 1);
	q(6, 24, 26, 1);
	q(0, 27, 25, 9);
	
	// stem
	q(9, 13, 10, 6);
	q(6, 10, 22, 24);
	q(22, 23, 25, 24);
	q(9, 25, 23, 13);
	
	
	// base top
	q(17, 14, 10, 13);
	q(10, 14, 15, 22);
	q(22, 15, 16, 23);
	q(17, 13, 23, 16);
	
	// bottom
	q(18, 21, 20, 19);
	
	// nw
	if (caps[0]) {
		t(3, 27, 0);
	}
	// sw
	if (caps[2]) {
		t(2, 1, 26);
	}
	
}

void off_c(trix_mesh *mesh, trix_vertex *v, int *caps) {

	// half ridge top
	q(2, 4, 5, 3);
	t(2, 24, 26);
	q(2, 3, 25, 24);
	t(3, 27, 25);
	
	// stem shaft
	q(24, 25, 23, 22);
	q(25, 9, 13, 23);
	q(9, 6, 10, 13);
	q(6, 24, 22, 10);
	
	// ring top
	q(26, 24, 6, 1);
	q(0, 1, 6, 9);
	q(0, 9, 25, 27);
	
	// ring bottom
	q(17, 13, 10, 14);
	q(10, 22, 28, 14);
	q(22, 11, 15, 28);
	q(11, 12, 16, 15);
	q(23, 29, 16, 12);
	q(17, 29, 23, 13);
	q(23, 12, 11, 22);
	

	// ne
	if (caps[1]) {
		t(5, 27, 3);
	}
	// se
	if (caps[3]) {
		t(4, 2, 26);
	}

}

void on_d(trix_mesh *mesh, trix_vertex *v, int *caps) {

	// half ridge top
	q(2, 4, 5, 3);
	t(2, 24, 26);
	q(3, 25, 24, 2);
	t(3, 27, 25);
	
	// ridge bottom
	
	q(5, 4, 7, 8);
	q(4, 26, 24, 7);
	q(5, 8, 25, 27);
	
	// stem
	q(24, 25, 23, 22);
	q(7, 24, 22, 11);
	q(7, 11, 12, 8);
	q(25, 8, 12, 23);
	
	// base top
	q(16, 12, 11, 15);
	q(22, 14, 15, 11);
	//q(11, 15, 14, 22);
	q(17, 14, 22, 23);
	//q(16, 12, 23, 17);
	q(16, 17, 23, 12);
	
	// bottom
	q(18, 21, 20, 19);
	
	
	// ne
	if (caps[1]) {
		t(5, 27, 3);
	}
	// se
	if (caps[3]) {
		t(4, 2, 26);
	}


}

void off_d(trix_mesh *mesh, trix_vertex *v, int *caps) {

	// half ridge top
	q(0, 1, 2, 3);
	t(2, 26, 24);
	q(2, 24, 25, 3);
	t(3, 25, 27);
	
	// stem shaft
	q(24, 22, 23, 25);
//	q(24, 25, 23, 22);
	q(25, 23, 12, 8);
	q(8, 12, 11, 7);
	q(7, 11, 22, 24);
	
	// ring top
	q(24, 26, 4, 7);
	q(7, 4, 5, 8);
	q(27, 25, 8, 5);
	
	// ring bottom
	q(17, 13, 10, 14);
	q(10, 22, 28, 14);
	q(22, 11, 15, 28);
	q(11, 12, 16, 15);
	q(23, 29, 16, 12);
	q(17, 29, 23, 13);

	q(23, 22, 10, 13);



	// nw
	if (caps[0]) {
		t(3, 27, 0);
	}
	// sw
	if (caps[2]) {
		t(2, 1, 26);
	}
}

void westwall(trix_mesh *mon, trix_mesh *moff, trix_vertex *v) {
	quad(mon,  &v[17], &v[21], &v[18], &v[14]);
	quad(moff, &v[0], &v[17], &v[14], &v[1]);
}

void northwall(trix_mesh *mon, trix_mesh *moff, trix_vertex *v) {
	quad(mon,  &v[17], &v[16], &v[20], &v[21]);
	quad(moff, &v[0], &v[5], &v[16], &v[17]);
}

void eastwall(trix_mesh *mon, trix_mesh *moff, trix_vertex *v) {
	quad(mon,  &v[15], &v[19], &v[20], &v[16]);
	quad(moff, &v[4], &v[15], &v[16], &v[5]);
}

void southwall(trix_mesh *mon, trix_mesh *moff, trix_vertex *v) {
	quad(mon,  &v[14], &v[18], &v[19], &v[15]);
	quad(moff, &v[1], &v[14], &v[15], &v[4]);
}

void updateVertices(trix_vertex *va, float x, float y) {
	
	// assumes va is allocated/extant with 30 elements
	
	va[0].x = x - 0.5;
	va[0].y = y + 0.5;
	va[0].z = 0.5;
	
	va[1].x = x - 0.5;
	va[1].y = y - 0.5;
	va[1].z = 0.5;
	
	va[2].x = x;
	va[2].y = y - 0.5;
	va[2].z = 1.0;
	
	va[3].x = x;
	va[3].y = y + 0.5;
	va[3].z = 1.0;
	
	va[4].x = x + 0.5;
	va[4].y = y - 0.5;
	va[4].z = 0.5;
	
	va[5].x = x + 0.5;
	va[5].y = y + 0.5;
	va[5].z = 0.5;
	
	// trunk top
	
	va[6].x = x - 0.25;
	va[6].y = y - 0.25;
	va[6].z = 0.5;
	
	va[7].x = x + 0.25;
	va[7].y = y - 0.25;
	va[7].z = 0.5;

	va[8].x = x + 0.25;
	va[8].y = y + 0.25;
	va[8].z = 0.5;
	
	va[9].x = x - 0.25;
	va[9].y = y + 0.25;
	va[9].z = 0.5;
	
	// trunk bottom
	
	va[10].x = x - 0.25;
	va[10].y = y - 0.25;
	va[10].z = 0.25;
	
	va[11].x = x + 0.25;
	va[11].y = y - 0.25;
	va[11].z = 0.25;
	
	va[12].x = x + 0.25;
	va[12].y = y + 0.25;
	va[12].z = 0.25;
	
	va[13].x = x - 0.25;
	va[13].y = y + 0.25;
	va[13].z = 0.25;

	// base top

	va[14].x = x - 0.5;
	va[14].y = y - 0.5;
	va[14].z = 0.25;
	
	va[15].x = x + 0.5;
	va[15].y = y - 0.5;
	va[15].z = 0.25;
	
	va[16].x = x + 0.5;
	va[16].y = y + 0.5;
	va[16].z = 0.25;
	
	va[17].x = x - 0.5;
	va[17].y = y + 0.5;
	va[17].z = 0.25;
	
	// base bottom
	
	va[18].x = x - 0.5;
	va[18].y = y - 0.5;
	va[18].z = 0.0;
	
	va[19].x = x + 0.5;
	va[19].y = y - 0.5;
	va[19].z = 0.0;
	
	va[20].x = x + 0.5;
	va[20].y = y + 0.5;
	va[20].z = 0.0;
	
	va[21].x = x - 0.5;
	va[21].y = y + 0.5;
	va[21].z = 0.0;
	
	// mid points (for type C & D)
	
	va[22].x = x;
	va[22].y = y - 0.25;
	va[22].z = 0.25;
	
	va[23].x = x;
	va[23].y = y + 0.25;
	va[23].z = 0.25;
	
	va[24].x = x;
	va[24].y = y - 0.25;
	va[24].z = 0.5;
	
	va[25].x = x;
	va[25].y = y + 0.25;
	va[25].z = 0.5;
	
	va[26].x = x;
	va[26].y = y - 0.5;
	va[26].z = 0.5;
	
	va[27].x = x;
	va[27].y = y + 0.5;
	va[27].z = 0.5;
	
	va[28].x = x;
	va[28].y = y - 0.5;
	va[28].z = 0.25;
	
	va[29].x = x;
	va[29].y = y + 0.5;
	va[29].z = 0.25;
}

int MergeImages(void) {
	
	bitmap bWest, bEast;
	trix_mesh *mOn, *mOff;
	int depth;
	unsigned long offset;
	int w, h, x, y;
	float fx, fy;
	int onWest, onEast;
	
	// booleans indicating whether the north/south sides of east/west ridges need to be capped
//	int north_east, north_west, south_east, south_west;
	int caps[4];
	
	// updated for each pixel
	trix_vertex v[30];
	
	// load images (note, we load as grayscale)
	bWest.data = stbi_load("test-a.png", &bWest.w, &bWest.h, &depth, 1);
	bEast.data = stbi_load("test-b.png", &bEast.w, &bEast.h, &depth, 1);
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
			
			fx = (float)x;
			fy = (float)(h - y);
			
			updateVertices(v, fx, fy);
			
			
			if (x == 0) {
				westwall(mOn, mOff, v);
			}
			if (x + 1 == w) {
				eastwall(mOn, mOff, v);
			}
			
			if (y == 0) {
				northwall(mOn, mOff, v);
			}
			if(y + 1 == h) {
				southwall(mOn, mOff, v);
			}
			
			
			// the on_/off_ functions should be in charge of capping north/south ridges
			// it should occur in two cases:
			// - perimeter (y == 0 or y + 1 == h)
			// - neighbor of different type
			
			
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
			
			
			caps[0] = 0; caps[1] = 0; caps[2] = 0; caps[3] = 0;
/*
			north_west = 0;
			north_east = 0;
			south_west = 0;
			south_east = 0;*/
			
			// north caps?
			if (y == 0) {
				// this is northenmost; always cap.
				caps[0] = 1;
				caps[1] = 1;
				//north_west = 1;
				//north_east = 1;
			} else {
				// check north neighbor
				offset = ((y - 1) * w) + x;
				
				
				if (onWest != (bWest.data[offset] > threshold)) {
					//north_west = 1;
					caps[0] = 1;
				}
				
				if (onEast != (bEast.data[offset] > threshold)) {
					caps[1] = 1;
					//north_east = 1;
				}
			}
			
			// south caps?
			if (y+1 == h) {
				// this is southernmost; always cap.
				caps[2] = 1;
				caps[3] = 1;
				//south_west = 1;
				//south_east = 1;
			} else {
				// check south neighbor
				offset = ((y + 1) * w) + x;
				
				
				if (onWest != (bWest.data[offset] > threshold)) {
					caps[2] = 1;
					//south_west = 1;
				}
				
				if (onEast != (bEast.data[offset] > threshold)) {
					caps[3] = 1;
					//south_east = 1;
				}
			}
			
			if (onWest && onEast) {
				// both on
				on_a(mOn, v, caps);
				off_a(mOff, v, caps);
			}
			else if (!onWest && !onEast) {
				// both off
				on_b(mOn, v, caps);
				off_b(mOff, v, caps);
			}
			else if (onWest && !onEast) {
				// west on, east off
				on_c(mOn, v, caps);
				off_c(mOff, v, caps);
			}
			else if (!onWest && onEast) {
				// west off, east on
				on_d(mOn, v, caps);
				off_d(mOff, v, caps);
			}
			
			
			
		}
	}
	
	// output
	
	trixWrite(mOn, "test-on.stl", TRIX_STL_ASCII);
	trixWrite(mOff, "test-off.stl", TRIX_STL_ASCII);
	
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
