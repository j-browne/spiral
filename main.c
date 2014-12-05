#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <png.h>

#define PI 3.14159265359

#define IMG_WIDTH 256
#define IMG_HEIGHT 128
#define COLOR_MAX 32

void gen_img(png_bytepp rows, bool*** used);
void write_img(const char* fname, png_bytepp rows);
void rand_color(png_byte* pix);

int main()
{
	srand(0);

	if (IMG_WIDTH * IMG_HEIGHT != COLOR_MAX * COLOR_MAX * COLOR_MAX)
	{
		fprintf (stderr, "You're an idiot. The number of colors does not equal the number of pixels.\n");
		return 1;
	}

	png_bytepp rows;
	bool*** used;

	int i,j,k;
	rows=malloc(IMG_HEIGHT*sizeof(png_bytep));
	if (rows == NULL)
	{
		fprintf(stderr,"Error allocating memory.\n");
		exit(1);
	}
	for(i=0; i<IMG_HEIGHT; ++i)
	{
		rows[i]=malloc(4*IMG_WIDTH*sizeof(png_byte));
		if (rows[i] == NULL)
		{
			fprintf(stderr,"Error allocating memory.\n");
			exit(1);
		}
		for(j=0; j<IMG_WIDTH; ++j)
		{
			png_bytep pix=&(rows[i][j*4]);
			pix[0]=0x00; // R
			pix[1]=0x00; // G
			pix[2]=0x00; // B
			pix[3]=0x00; // A
		}
	}

	used=malloc(COLOR_MAX*sizeof(bool**));
	if (used == NULL)
	{
		fprintf(stderr,"Error allocating memory.\n");
		exit(1);
	}
	for(i=0; i<COLOR_MAX; ++i)
	{
		used[i]=malloc(COLOR_MAX*sizeof(bool*));
		if (used[i] == NULL)
		{
			fprintf(stderr,"Error allocating memory.\n");
			exit(1);
		}
		for(j=0; j<COLOR_MAX; ++j)
		{
			used[i][j]=calloc(COLOR_MAX, sizeof(bool));
			if (used[i][j] == NULL)
			{
				fprintf(stderr,"Error allocating memory.\n");
				exit(1);
			}
		}
	}

	gen_img(rows, used);

	//TODO: REMOVE
	for(i=0; i<IMG_HEIGHT; ++i)
	{
		for(j=0; j<IMG_WIDTH; ++j)
		{
			png_bytep pix=&(rows[i][j*4]);
			pix[3]=0xff; // A
		}
	}
	//END TODO

	write_img("1.png", rows);

	for(i=0; i<IMG_HEIGHT; ++i)
	{
		free(rows[i]);
	}
	free(rows);

	return 0;
}

void gen_img(png_bytepp rows, bool*** used)
{
	png_byte color[3];
	double spiral_size=20;
	int scale=0x100/COLOR_MAX;
	int i,j;
	// Create anchor points for spirals
	for(i=0; i<4; ++i)
	{
		for(j=0; true; ++j)
		{
			int x=spiral_size*j*cos((j-i)*PI/2) + IMG_WIDTH/2;
			int y=spiral_size*j*sin((j-i)*PI/2) + IMG_HEIGHT/2;
			if (x<0 || x>IMG_WIDTH)
			{
				break;
			}

			do
			{
				rand_color(color);
			} while(used[color[0]][color[1]][color[2]]);

			if (y > 0 && y < IMG_HEIGHT)
			{
				png_bytep pix=&(rows[y][x*4]);

				if (pix[3] != 0xff) // If that pixel has not been changed already
				{
					pix[0]=color[0]*scale; // R
					pix[1]=color[1]*scale; // G
					pix[2]=color[2]*scale; // B
					pix[3]=0xff; // A
					used[color[0]][color[1]][color[2]]=true;
				}
			}
		}
	}

//			char filename[128]="";
//			printf("%d %d\n", x, y);
//			sprintf(filename, "img/%05d.png", (r*COLOR_MAX*COLOR_MAX + g*COLOR_MAX + b));
//			write_img(filename, rows);
	return;
}

void write_img(const char* fname, png_bytepp rows)
{
	/* SETUP */
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;

	fp = fopen(fname, "wb");
	if (fp == NULL)
	{
		fprintf(stderr, "Error opening file.\n");
		exit(1);
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fprintf(stderr, "Error creating png struct.\n");
		fclose(fp);
		exit(1);
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fprintf(stderr, "Error creating info struct.\n");
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  NULL);
		exit(1);
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		fprintf(stderr, "Error writing file.\n");
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		exit(1);
	}

	png_set_IHDR(png_ptr, info_ptr, IMG_WIDTH, IMG_HEIGHT,
			8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* WRITE IMAGE */
	png_init_io(png_ptr, fp);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, rows);
	png_write_end(png_ptr, info_ptr);

	/* CLEANUP */
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);

	return;
}

void rand_color(png_byte* pix)
{
	pix[0]=rand()%COLOR_MAX;
	pix[1]=rand()%COLOR_MAX;
	pix[2]=rand()%COLOR_MAX;
	return;
}
