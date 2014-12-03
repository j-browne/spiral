#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <png.h>

#define IMG_WIDTH 256
#define IMG_HEIGHT 128
#define COLOR_MAX 32

void gen_img(png_bytepp rows, bool*** used)
{
	int r,g,b;
	int scale=0x100/COLOR_MAX;
	for (r=0; r<COLOR_MAX; ++r)
	{
		for (g=0; g<COLOR_MAX; ++g)
		{
			for (b=0; b<COLOR_MAX; ++b)
			{
				int y=(r*COLOR_MAX*COLOR_MAX + g*COLOR_MAX + b)/IMG_WIDTH;
				int x=(r*COLOR_MAX*COLOR_MAX + g*COLOR_MAX + b)%IMG_WIDTH;
				png_bytep pix=&(rows[y][x*4]);

				pix[0]=r*scale; // R
				pix[1]=g*scale; // G
				pix[2]=b*scale; // B
				pix[3]=0xff; // A
				used[r][g][b]=true;
			}
		}
	}
	return;
}

int main()
{
	if (IMG_WIDTH * IMG_HEIGHT != COLOR_MAX * COLOR_MAX * COLOR_MAX)
	{
		fprintf (stderr, "You're an idiot. The number of colors does not equal the number of pixels.\n");
		return 1;
	}

FILE *fp;
png_structp png_ptr;
png_infop info_ptr;
png_bytepp row_pointers;
bool*** used;

/* SETUP
 * Allocate and initialize all needed space
 */
	fp = fopen("test.png", "wb");
	if (fp == NULL)
	{
		fprintf (stderr, "Error opening file.\n");
		return 1;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
	{
		fprintf (stderr, "Error creating png struct.\n");
		fclose(fp);
		return 1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fprintf (stderr, "Error creating info struct.\n");
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return 1;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		fprintf (stderr, "Error writing file.\n");
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return 1;
	}

	png_set_IHDR(png_ptr, info_ptr, IMG_WIDTH, IMG_HEIGHT,
		8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	int i,j,k;
	row_pointers=malloc(sizeof(png_bytep)*IMG_HEIGHT);
	if (row_pointers == NULL)
	{
		longjmp(png_jmpbuf(png_ptr), 0);
	}
	for(i=0; i<IMG_HEIGHT; ++i)
	{
		row_pointers[i]=malloc(png_get_rowbytes(png_ptr,info_ptr));
		if (row_pointers[i] == NULL)
		{
			longjmp(png_jmpbuf(png_ptr), 0);
		}
		for(j=0; j<IMG_WIDTH; ++j)
		{
			png_bytep pix=&(row_pointers[i][j*4]);
			pix[0]=0x00; // R
			pix[1]=0x00; // G
			pix[2]=0x00; // B
			pix[3]=0x00; // A
		}
	}

	used=malloc(COLOR_MAX*sizeof(bool**));
	if (used == NULL)
	{
		longjmp(png_jmpbuf(png_ptr), 0);
	}
	for(i=0; i<COLOR_MAX; ++i)
	{
		used[i]=malloc(COLOR_MAX*sizeof(bool*));
		if (used[i] == NULL)
		{
			longjmp(png_jmpbuf(png_ptr), 0);
		}
		for(j=0; j<COLOR_MAX; ++j)
		{
			used[i][j]=calloc(COLOR_MAX, sizeof(bool));
			if (used[i][j] == NULL)
			{
				longjmp(png_jmpbuf(png_ptr), 0);
			}
		}
	}

	/* GENERATE IMAGE
	 */
	gen_img(row_pointers, used);

	/* WRITE IMAGE
	 */
	png_init_io(png_ptr, fp);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, info_ptr);

	/* CLEANUP
	 */
	png_destroy_write_struct(&png_ptr, &info_ptr);
	for(i=0; i<IMG_HEIGHT; ++i)
	{
		free(row_pointers[i]);
	}
	free (row_pointers);
	fclose (fp);

	return 0;
}
