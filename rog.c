#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <libtrix.h>
#include "stb_image.h"

typedef struct {
	char *img_west; // path to "west" input image
	char *img_east; // path to "east" input image
	char *stl_base; // base output path (white/black suffixes will be appended)
	float xy_scale; // factor applied to x/y coordinates
	int centered; // boolean; if false, southwest corner output at 0,0
	float h_white; // height of white (lower) base layer
	float h_black; // height of black (middle) base layer
	float h_ridge; // height of ridge (upper) image layer
	int threshold; // pixel brightness > threshold considered white
	int ascii; // boolean; if true, output ascii stl instead of binary
} Settings;

Settings CONFIG = {
	NULL,
	NULL,
	NULL,
	1.0,
	1,
	0.25,
	0.25,
	0.5,
	155,
	0
};

typedef struct {
	int w, h;
	unsigned char *data;
} bitmap;

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

void white_a(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
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

void black_a(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
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
	q(25, 23, 12, 8);
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

void white_b(trix_mesh *mesh, trix_vertex *v, int *caps) {

	// top
	q(17, 14, 15, 16);
	
	// bottom
	q(21, 20, 19, 18);

	// no caps

}

void black_b(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
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

void white_c(trix_mesh *mesh, trix_vertex *v, int *caps) {
	
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

void black_c(trix_mesh *mesh, trix_vertex *v, int *caps) {

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

void white_d(trix_mesh *mesh, trix_vertex *v, int *caps) {

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

void black_d(trix_mesh *mesh, trix_vertex *v, int *caps) {

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

void BaseWalls(trix_mesh *mwhite, trix_mesh *mblack, trix_vertex *v, int x, int y, int w, int h) {
	
	// west
	if (x == 0) {
		quad(mwhite, &v[17], &v[21], &v[18], &v[14]);
		quad(mblack, &v[0], &v[17], &v[14], &v[1]);
	}
	
	// east
	if (x + 1 == w) {
		quad(mwhite, &v[15], &v[19], &v[20], &v[16]);
		quad(mblack, &v[4], &v[15], &v[16], &v[5]);
	}
	
	// north
	if (y == 0) {
		
		tri(mwhite, &v[17], &v[29], &v[21]);
		tri(mwhite, &v[29], &v[20], &v[21]);
		tri(mwhite, &v[16], &v[20], &v[29]);
		
		quad(mblack, &v[0], &v[27], &v[29], &v[17]);
		quad(mblack, &v[27], &v[5], &v[16], &v[29]);
	}
	
	// south
	if (y + 1 == h) {
		tri(mwhite, &v[14], &v[18], &v[28]);
		tri(mwhite, &v[28], &v[18], &v[19]);
		tri(mwhite, &v[15], &v[28], &v[19]);
		
		quad(mblack, &v[1], &v[14], &v[28], &v[26]);
		quad(mblack, &v[28], &v[15], &v[4], &v[26]);
	}
}

void updateVertices(trix_vertex *v, float x, float y) {
	
	int i;
	
	v[0].x = x - 0.5;
	v[0].y = y + 0.5;
	v[0].z = CONFIG.h_black + CONFIG.h_white;
	
	v[1].x = x - 0.5;
	v[1].y = y - 0.5;
	v[1].z = CONFIG.h_black + CONFIG.h_white;
	
	v[2].x = x;
	v[2].y = y - 0.5;
	v[2].z = CONFIG.h_white + CONFIG.h_black + CONFIG.h_ridge;
	
	v[3].x = x;
	v[3].y = y + 0.5;
	v[3].z = CONFIG.h_white + CONFIG.h_black + CONFIG.h_ridge;
	
	v[4].x = x + 0.5;
	v[4].y = y - 0.5;
	v[4].z = CONFIG.h_black + CONFIG.h_white;
	
	v[5].x = x + 0.5;
	v[5].y = y + 0.5;
	v[5].z = CONFIG.h_black + CONFIG.h_white;
	
	v[6].x = x - 0.25;
	v[6].y = y - 0.25;
	v[6].z = CONFIG.h_black + CONFIG.h_white;
	
	v[7].x = x + 0.25;
	v[7].y = y - 0.25;
	v[7].z = CONFIG.h_black + CONFIG.h_white;

	v[8].x = x + 0.25;
	v[8].y = y + 0.25;
	v[8].z = CONFIG.h_black + CONFIG.h_white;
	
	v[9].x = x - 0.25;
	v[9].y = y + 0.25;
	v[9].z = CONFIG.h_black + CONFIG.h_white;
	
	v[10].x = x - 0.25;
	v[10].y = y - 0.25;
	v[10].z = CONFIG.h_white;
	
	v[11].x = x + 0.25;
	v[11].y = y - 0.25;
	v[11].z = CONFIG.h_white;
	
	v[12].x = x + 0.25;
	v[12].y = y + 0.25;
	v[12].z = CONFIG.h_white;
	
	v[13].x = x - 0.25;
	v[13].y = y + 0.25;
	v[13].z = CONFIG.h_white;
	
	v[14].x = x - 0.5;
	v[14].y = y - 0.5;
	v[14].z = CONFIG.h_white;
	
	v[15].x = x + 0.5;
	v[15].y = y - 0.5;
	v[15].z = CONFIG.h_white;
	
	v[16].x = x + 0.5;
	v[16].y = y + 0.5;
	v[16].z = CONFIG.h_white;
	
	v[17].x = x - 0.5;
	v[17].y = y + 0.5;
	v[17].z = CONFIG.h_white;
	
	v[18].x = x - 0.5;
	v[18].y = y - 0.5;
	v[18].z = 0.0;
	
	v[19].x = x + 0.5;
	v[19].y = y - 0.5;
	v[19].z = 0.0;
	
	v[20].x = x + 0.5;
	v[20].y = y + 0.5;
	v[20].z = 0.0;
	
	v[21].x = x - 0.5;
	v[21].y = y + 0.5;
	v[21].z = 0.0;
	
	v[22].x = x;
	v[22].y = y - 0.25;
	v[22].z = CONFIG.h_white;
	
	v[23].x = x;
	v[23].y = y + 0.25;
	v[23].z = CONFIG.h_white;
	
	v[24].x = x;
	v[24].y = y - 0.25;
	v[24].z = CONFIG.h_black + CONFIG.h_white;
	
	v[25].x = x;
	v[25].y = y + 0.25;
	v[25].z = CONFIG.h_black + CONFIG.h_white;
	
	v[26].x = x;
	v[26].y = y - 0.5;
	v[26].z = CONFIG.h_black + CONFIG.h_white;
	
	v[27].x = x;
	v[27].y = y + 0.5;
	v[27].z = CONFIG.h_black + CONFIG.h_white;
	
	v[28].x = x;
	v[28].y = y - 0.5;
	v[28].z = CONFIG.h_white;
	
	v[29].x = x;
	v[29].y = y + 0.5;
	v[29].z = CONFIG.h_white;
	
	for (i = 0; i < 30; i++) {
		v[i].x *= CONFIG.xy_scale;
		v[i].y *= CONFIG.xy_scale;
		//v[i].z *= CONFIG.xy_scale;
	}
}

int MergeImages(void) {
	
	bitmap bWest, bEast;
	trix_mesh *mwhite, *mblack;
	int depth;
	unsigned long offset;
	int w, h, x, y;
	int whiteWest, whiteEast;
	float fx, fy;
	// booleans indicating whether the north/south sides of east/west ridges need to be capped
	int caps[4];
	
	char *stl_white, *stl_black;
	
	// updated for each pixel
	trix_vertex v[30];
	
	// load images (note, we load as grayscale)
	bWest.data = stbi_load(CONFIG.img_west, &bWest.w, &bWest.h, &depth, 1);
	bEast.data = stbi_load(CONFIG.img_east, &bEast.w, &bEast.h, &depth, 1);
	if (bWest.data == NULL || bEast.data == NULL) {
		fprintf(stderr, "%s\n", stbi_failure_reason());
		return 1;
	}
	
	// validate that dimensions match
	if (bWest.w != bEast.w || bWest.h != bEast.h) {
		fprintf(stderr, "Image dimensions do not match.\n");
		return 1;
	}
	w = bWest.w;
	h = bWest.h;
	
	if (trixCreate(&mwhite, "onpixels") != TRIX_OK) {
		return 1;
	}
	if (trixCreate(&mblack, "offpixels") != TRIX_OK) {
		return 1;
	}
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			
			if (CONFIG.centered) {
				fx = (float)(x - (w/2));
				fy = (float)((h - y) - (h/2));
			} else {
				fx = (float)x;
				fy = (float)(h - y);
			}	
			
			updateVertices(v, fx, fy);
			
			BaseWalls(mwhite, mblack, v, x, y, w, h);
			
			
			offset = (y * w) + x;
			
			if (bWest.data[offset] > CONFIG.threshold) {
				whiteWest = 1;
			} else {
				whiteWest = 0;
			}
			
			if (bEast.data[offset] > CONFIG.threshold) {
				whiteEast = 1;
			} else {
				whiteEast = 0;
			}
			
			// nw, ne, sw, se; no caps by default
			caps[0] = 0; caps[1] = 0; caps[2] = 0; caps[3] = 0;
			
			// north caps?
			if (y == 0) {
				// nw and ne
				caps[0] = 1;
				caps[1] = 1;
			} else {
				// check north neighbor
				offset = ((y - 1) * w) + x;
				
				// nw
				if (whiteWest != (bWest.data[offset] > CONFIG.threshold)) {
					caps[0] = 1;
				}
				
				// ne
				if (whiteEast != (bEast.data[offset] > CONFIG.threshold)) {
					caps[1] = 1;
				}
			}
			
			// south caps?
			if (y+1 == h) {
				// sw and se
				caps[2] = 1;
				caps[3] = 1;
			} else {
				// check south neighbor
				offset = ((y + 1) * w) + x;
				
				// sw
				if (whiteWest != (bWest.data[offset] > CONFIG.threshold)) {
					caps[2] = 1;
				}
				
				// se
				if (whiteEast != (bEast.data[offset] > CONFIG.threshold)) {
					caps[3] = 1;
				}
			}
			
			if (whiteWest && whiteEast) {
				white_a(mwhite, v, caps);
				black_a(mblack, v, caps);
			}
			else if (!whiteWest && !whiteEast) {
				white_b(mwhite, v, caps);
				black_b(mblack, v, caps);
			}
			else if (whiteWest && !whiteEast) {
				white_c(mwhite, v, caps);
				black_c(mblack, v, caps);
			}
			else if (!whiteWest && whiteEast) {
				white_d(mwhite, v, caps);
				black_d(mblack, v, caps);
			}
		}
	}
	
	// output
	
	stl_white = malloc(strlen(CONFIG.stl_base) + 12);
	stl_black = malloc(strlen(CONFIG.stl_base) + 12);
	strcpy(stl_white, CONFIG.stl_base);
	strcat(stl_white, "-white.stl");
	strcpy(stl_black, CONFIG.stl_base);
	strcat(stl_black, "-black.stl");
	
	trixWrite(mwhite, stl_white, CONFIG.ascii ? TRIX_STL_ASCII : TRIX_STL_BINARY);
	trixWrite(mblack, stl_black, CONFIG.ascii ? TRIX_STL_ASCII : TRIX_STL_BINARY);
	
	// cleanup
	
	free(stl_white);
	free(stl_black);
	
	trixRelease(&mwhite);
	trixRelease(&mblack);
	
	stbi_image_free(bWest.data);
	stbi_image_free(bEast.data);
	
	return 0;
}

void PrintUsage(const char *prog) {
	printf("Usage: %s [OPTIONS] LEFT RIGHT OUTPUT\n", prog);
	printf("\n");
	printf("The LEFT and RIGHT arguments specify the paths to the input images, which\n");
	printf("must have the same dimensions. Grayscale images will be interpreted as black\n");
	printf("and white according to the -t option. Color images will be read as grayscale\n");
	printf("images according to an intermediate luminance conversion (0.3R 0.59G 0.11B).\n");
	printf("Supported image formats include PNG, GIF, and [non-progressive] JPG.\n");
	printf("\n");
	printf("The OUTPUT argument specifies the base output path; output will consist of\n");
	printf("two STL files, named OUTPUT-white.stl and OUTPUT-black.stl.\n");
	printf("\n");
	printf("OPTIONS (CURRENT):\n");
	printf("\n");
	printf("-a THICKNESS (%.2f) Bottom layer (white base) thickness.\n", CONFIG.h_white);
	printf("-b THICKNESS (%.2f) Middle layer (black base) thickness.\n", CONFIG.h_black);
	printf("-c THICKNESS (%.2f) Top layer (image ridges) thickness.\n", CONFIG.h_ridge);
	printf("-s SCALE     (%.2f) Scaling factor applied to x/y coordinates.\n", CONFIG.xy_scale);
	printf("-t THRESHOLD (%3d)  Brightness values > THRESHOLD considered white.\n", CONFIG.threshold);
	printf("-f           (%s)  Output ASCII format STL instead of binary.\n", CONFIG.ascii ? " on" : "off");
	printf("-o           (%s)  Locate origin at bottom left of image instead of center.\n", CONFIG.centered ? "off" : " on");
	printf("-h           (off)  Display this help message and quit.\n");
}

// returns 0 on success
int parseopts(int argc, char **argv) {
	int c;
	opterr = 0;
	
	while ((c = getopt(argc, argv, "a:b:c:s:t:foh")) != -1) {
		switch (c) {
			case 'a':
				// a for first of a b c layer thickness
				if (sscanf(optarg, "%10f", &CONFIG.h_white) != 1 || CONFIG.h_white <= 0) {
					fprintf(stderr, "-a must be a number greater than 0\n");
					return 1;
				}
				break;
			case 'b':
				// b for second of a b c layer thickness
				if (sscanf(optarg, "%10f", &CONFIG.h_black) != 1 || CONFIG.h_black <= 0) {
					fprintf(stderr, "-b must be a number greater than 0\n");
					return 1;
				}
				break;
			case 'c':
				// c for third of a b c layer thickness
				if (sscanf(optarg, "%10f", &CONFIG.h_ridge) != 1 || CONFIG.h_ridge <= 0) {
					fprintf(stderr, "-c must be a number greater than 0\n");
					return 1;
				}
				break;
			case 's':
				// s for scale
				if (sscanf(optarg, "%10f", &CONFIG.xy_scale) != 1 || CONFIG.xy_scale <= 0) {
					fprintf(stderr, "-s must be a number greater than 0\n");
					return 1;
				}
				break;
			case 't':
				// t for threshold
				if (sscanf(optarg, "%10d", &CONFIG.threshold) != 1 || CONFIG.threshold < 0 || CONFIG.threshold > 255) {
					fprintf(stderr, "-t must be a number in range 0 to 255\n");
					return 1;
				}
				break;
			case 'f':
				// f for format
				CONFIG.ascii = !CONFIG.ascii;
				break;
			case 'o':
				// o for origin
				CONFIG.centered = !CONFIG.centered;
				break;
			case 'h':
				PrintUsage(argv[0]);
				exit(0);
				break;
			case '?':
				fprintf(stderr, "Unrecognized option or missing option argument.\n");
				return 1;
				break;
			default:
				fprintf(stderr, "Unexpected trouble parsing options. (%c)\n", optopt);
				return 1;
				break;
		}
	}
		
	if (argc - optind != 3) {
		fprintf(stderr, "Did not find expected number of arguments.\n");
		return 1;
	}
	
	CONFIG.img_west = argv[optind];
	CONFIG.img_east = argv[optind + 1];
	CONFIG.stl_base = argv[optind + 2];
	
	return 0;
}

int main(int argc, char **argv) {
	
	if (parseopts(argc, argv) != 0) {
		fprintf(stderr, "Try the -h option for usage help.\n");
		return 1;
	}
	
	if (MergeImages() != 0) {
		return 1;
	}
	
	return 0;
}
