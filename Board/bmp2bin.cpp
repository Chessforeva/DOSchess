//////////////////////////////////////////////////////////////////////////////
// bmp2bin.cpp                                                              //
//////////////////////////////////////////////////////////////////////////////
/*
	Bitmap to binary converter
	by Rafael Vuijk (aka Dark Fader)

	History:
		TODO - Gamma correction
		v1.11 - GP32 rotation fixed
		v1.10 - Pokemon Mini sprite
		v1.09 - Pokemon Mini masking
		v1.08 - Pokemon Mini grayscale
		v1.07 - Added support for 1 bpp bmp's, Pokemon Mini output
		v1.06 - GP32 intensity bit, return width/height, some other parameter changes
		v1.05 - GameCube output
		v1.04+ - bigendian support by mr.spiv
		v1.04 - .act & MS palette loading support, palette quantization method 1
		v1.03 - added 8 bit bmp support
		v1.02 - fixed rotation bug, copied structures from WinGDI.h
		v1.01 - combined
		v1.00 - seperate versions for each platform

	Build:
		g++ -mno-cygwin -o bmp2bin bmp2bin.cpp
*/

//////////////////////////////////////////////////////////////////////////////
// Includes                                                                 //
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////////////
// Defines                                                                  //
//////////////////////////////////////////////////////////////////////////////
#define VER					"1.11"
#define ALIGN4(n)			(((n)+3) &~ 3)

#if defined(__i386) || defined(__i386__) || defined(__i486__) || defined(__i586__)
	#define __LITTLE_ENDIAN__
#endif

#define fwrite2(data, a, b, f)	\
	 memcpy(destImagePtr, data, (a)*(b)); destImagePtr += (a)*(b)
	
#define fputc2(data, f)		\
	*destImagePtr++ = data;

//////////////////////////////////////////////////////////////////////////////
// Typedefs                                                                 //
//////////////////////////////////////////////////////////////////////////////
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed int s32;

/*
 * Bitmap headers
 */

#pragma pack(1)

typedef struct tagRGBTRIPLE {
        u8    rgbtBlue;
        u8    rgbtGreen;
        u8    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
        u8    rgbBlue;
        u8    rgbGreen;
        u8    rgbRed;
        u8    rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPFILEHEADER {
        u16    bfType;
        u32   bfSize;
        u16    bfReserved1;
        u16    bfReserved2;
        u32   bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
        u32      biSize;
        s32       biWidth;
        s32       biHeight;
        u16       biPlanes;
        u16       biBitCount;
        u32      biCompression;
        u32      biSizeImage;
        s32       biXPelsPerMeter;
        s32       biYPelsPerMeter;
        u32      biClrUsed;
        u32      biClrImportant;
} BITMAPINFOHEADER;

#pragma pack()

typedef void WritePixel(const RGBTRIPLE *p);

//////////////////////////////////////////////////////////////////////////////
// Prototypes                                                               //
//////////////////////////////////////////////////////////////////////////////
void WritePixel24(const RGBTRIPLE *p);

//////////////////////////////////////////////////////////////////////////////
// Variables                                                                //
//////////////////////////////////////////////////////////////////////////////
char *inputFile;
char *outputFile;
char *paletteFile;
RGBTRIPLE palette[256];
RGBTRIPLE *srcImage;	// input data
BITMAPFILEHEADER bfh;
BITMAPINFOHEADER bih;
char flags[256];
FILE *fi;
FILE *fo;
bool output_big = false;	// big endian output?
WritePixel *writePixel = WritePixel24;		// pixel write function
int bpp = 24;				// bpp required for destination
unsigned char *destImage;	// output data
unsigned char *destImagePtr;	// fwrite2, fputc2
unsigned int biWidth, biHeight, biBitCount;	// very handy actually
/*signed*/ int x, y;
int pm_dither_xor, pm_dither_and, pm_dither_or;


//////////////////////////////////////////////////////////////////////////////
// Endian conversion                                                        //
//////////////////////////////////////////////////////////////////////////////
static u8 input_endian(u8 b)
{
	return b;
}

static u16 input_endian(u16 w)
{
	#if defined(__BIG_ENDIAN__)
		u16 rw;
		rw = ((w >> 8) | (w << 8));
		return rw;
	#elif defined(__LITTLE_ENDIAN__)
		return w;
	#else
		#error unknown endianess..
	#endif
}

static u32 input_endian(u32 w)
{
	#if defined(__BIG_ENDIAN__)
		u32 rw;
		u16 wu = w >> 16;
		u16 wl = w & 0xffff;
		wu = (wu >> 8) | (wu << 8);
		wl = (wl >> 8) | (wl << 8);
		rw = ((wl << 16) | (wu));
		return rw;
	#elif defined(__LITTLE_ENDIAN__)
		return w;
	#else
		#error unknown endianess..
	#endif
}

static s32 input_endian(s32 w)
{
	#if defined(__BIG_ENDIAN__)
		s32 rw;
		u16 wu = (u16)(w >> 16);
		u16 wl = (u16)(w & 0xffff);
		wu = (wu >> 8) | (wu << 8);
		wl = (wl >> 8) | (wl << 8);
		rw = ((wl << 16) | (wu));
		return rw;
	#elif defined(__LITTLE_ENDIAN__)
		return w;
	#else
		#error unknown endianess..
	#endif
}

template <typename T> T output_endian(T x)
{
	#if defined(__BIG_ENDIAN__)
		return output_big ? x : input_endian(x);
	#elif defined(__LITTLE_ENDIAN__)
		return output_big ? input_endian(x) : x;
	#else
		#error unknown endianess..
	#endif	
}

//////////////////////////////////////////////////////////////////////////////
// Sqr                                                                      //
//////////////////////////////////////////////////////////////////////////////
inline unsigned long Sqr(unsigned int n)
{
	return n*n;
}

//////////////////////////////////////////////////////////////////////////////
// Dist1                                                                    //
//////////////////////////////////////////////////////////////////////////////
unsigned long Dist1(const RGBTRIPLE *a, const RGBTRIPLE *b)
{
	return
	(
		Sqr((int)a->rgbtRed - (int)b->rgbtRed)*28 +
		Sqr((int)a->rgbtGreen - (int)b->rgbtGreen)*91 +
		Sqr((int)a->rgbtBlue - (int)b->rgbtBlue)*9
	);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelP1                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelP1(const RGBTRIPLE *p)		// '1': 8 bits palette (method 1)
{
	unsigned char out;
	unsigned long bestDist = (unsigned long)-1;
	
	for (int i=0; i<256; i++)
	{
		unsigned long dist = Dist1(p, &palette[i]);
		if (dist < bestDist) { bestDist = dist; out = i; }
	}
	
	fwrite2(&out, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixel24                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixel24(const RGBTRIPLE *p)	// 't': 24 bits
{
	fwrite2(&p->rgbtRed, 1, 1, fo);
	fwrite2(&p->rgbtGreen, 1, 1, fo);
	fwrite2(&p->rgbtBlue, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixel8                                                              //
//////////////////////////////////////////////////////////////////////////////
void WritePixel8(const RGBTRIPLE *p)		// 'e': 8 bits (b2g3r3)
{
	unsigned char out = (p->rgbtBlue>>6<<6) | (p->rgbtGreen>>5<<3) | (p->rgbtRed>>5<<0);
	fwrite2(&out, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGP8                                                            //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGP8(const RGBTRIPLE *p)		// 'i': 8 bits (r3g3b2, GamePark)
{
	unsigned char out = (p->rgbtBlue>>6<<0) | (p->rgbtGreen>>5<<2) | (p->rgbtRed>>5<<5);
	fwrite2(&out, 1, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelPM                                                             //
//////////////////////////////////////////////////////////////////////////////
// rotation won't matter in this case (I think)
void WritePixelPM(const RGBTRIPLE *p)		// 'm','M': 1 bit
{
	int pm_level = ((pm_dither_xor ^ x ^ y) & pm_dither_and | pm_dither_or) ? 85 : 2*85;
	if (p->rgbtGreen < pm_level)
	{
		destImage[y/8*biWidth + x] |= 1 << (y % 8);
	}
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGP32                                                           //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGP32(const RGBTRIPLE *p)		// 'p': 16 bits (r5g5b5i1, GamePark)
{
	u16 out = (
		(p->rgbtBlue>>3<<1) | (p->rgbtGreen>>3<<6) | (p->rgbtRed>>3<<11)
		| (((p->rgbtBlue & 7) * 7209 + (p->rgbtGreen & 7) * 38665 + (p->rgbtRed & 7) * 19662) >> (16+2) & 1)
	);
	out = output_endian(out);
	fwrite2(&out, 2, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGB                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGB(const RGBTRIPLE *p)		// 'g': 16 bits (x1b5g5r5, GameBoy)
{
	unsigned short out = (p->rgbtBlue>>3<<10) | (p->rgbtGreen>>3<<5) | (p->rgbtRed>>3<<0);
	out = output_endian(out);
	fwrite2(&out, 2, 1, fo);
}

//////////////////////////////////////////////////////////////////////////////
// WritePixelGC                                                             //
//////////////////////////////////////////////////////////////////////////////
void WritePixelGC(const RGBTRIPLE *p)		// 'c': 32 bits (y4Cb4y4Cr4, GameCube)
{
	unsigned char y  = (unsigned char)(  0.29900 * p->rgbtRed + 0.58700 * p->rgbtGreen + 0.11400 * p->rgbtBlue);
	unsigned char cb = (unsigned char)(- 0.16874 * p->rgbtRed - 0.33126 * p->rgbtGreen + 0.50000 * p->rgbtBlue + 0x80);
	unsigned char cr = (unsigned char)(  0.50000 * p->rgbtRed - 0.41869 * p->rgbtGreen - 0.08131 * p->rgbtBlue + 0x80);
	unsigned long out = (y << 24) | (cb << 16) | (y << 8) | cr;
	fputc2(out>>24 & 0xFF, fo);
	fputc2(out>>16 & 0xFF, fo);
	fputc2(out>>8 & 0xFF, fo);
	fputc2(out>>0 & 0xFF, fo);
}

//////////////////////////////////////////////////////////////////////////////
// Help                                                                     //
//////////////////////////////////////////////////////////////////////////////
void Help()
{
	fprintf(stderr,
		"bmp2bin "VER" by Rafael Vuijk (aka Dark Fader)\n"
		"with thanks to Jouni Korhonen (aka Mr.Spiv) for big-endian suppport\n"
		"\n"
		"Syntax: bmp2bin [-flags] <input.bmp> <output.raw>\n"
		"\n"
		"Flags/parameters:\n"
		"  -m                  1  bit  output, dithered, 1st screen, Pokemon Mini\n"
		"  -M                  1  bit  output, dithered, 2nd screen, Pokemon Mini\n"
		"  -k                  1  bit  output, non-dithered, 1st screen, Pokemon Mini\n"
		"  -K                  1  bit  output, non-dithered, 2nd screen, Pokemon Mini\n"
		"  -kK                 1  bit  output, sprite data with masking, Pokemon Mini\n"
		"  -i                  8  bits output, r3g3b2, GP32 generic palette\n"
		"  -e                  8  bits output, b2g3r3\n"
		"  -1 palette.act      8  bits output, palette quantization method 1\n"
		"  -g                  16 bits output, x1b5g5r5, GameBoy\n"
		"  -p                  16 bits output, r5g5b5i1, GP32\n"
		"  -t                  24 bits output, b8g8r8, default format\n"
		"  -c                  32 bits output, y4Cb4y4Cr4, GameCube\n"
		"\n"
		"  -r                  Rotate 90 degrees clockwise (for GP32)\n"
		"  -n                  Add width/height header\n"
		"  -b                  Big endian output\n"
		"  -w                  Return width and height\n"
		"  -h                  Return height and width\n"
		"  -?                  This help\n"
	);
}

//////////////////////////////////////////////////////////////////////////////
// DoGfxThing                                                               //
//////////////////////////////////////////////////////////////////////////////
void DoGfxThing(FILE *fo, unsigned int offset=0, unsigned int size1=0, unsigned int size2=0)
{
	unsigned int size = biWidth * biHeight * bpp / 8;
	destImage = new unsigned char[size];
	memset(destImage, 0, size);
	destImagePtr = destImage;	// for fwrite2, fputc2

	// rotated?
	if (flags['r'])
	{
		for (x=0; x<biWidth; x++)
		{
			//pm_dither = pm_dither_xor ^ x & 1;
			for (y=biHeight-1; y>=0; y--)
			{
				RGBTRIPLE *p = srcImage + (y*biWidth) + x;
				writePixel(p);
			}
		}
	}
	else
	{
		RGBTRIPLE *p = srcImage;
		for (y=0; y<biHeight; y++)
		{
			//pm_dither = pm_dither_xor ^ y & 1;
			for (x=0; x<biWidth; x++)
			{
				RGBTRIPLE *p = srcImage + (y*biWidth) + x;
				writePixel(p);
			}
		}
	}

	// do the actual write
	if (!size1)
	{
		fwrite(destImage, 1, size, fo);
	}
	else
	{
		unsigned int ofs1 = 0;
		unsigned int ofs2 = offset;
		while (ofs1 < size)
		{
			fseek(fo, ofs2, SEEK_SET);
			fwrite(destImage + ofs1, 1, size1, fo);
			ofs1 += size1;
			ofs2 += size2;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
// main                                                                     //
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	// parse parameters
	for (int a=1; a<argc; a++)
	{
		if (argv[a][0] == '-')
		{
			for (int i=1; argv[a][i]; i++)
			{
				flags[argv[a][i]]++;
				switch (argv[a][i])
				{
					case 'b': output_big = true; break;
					case 'm': writePixel = WritePixelPM; pm_dither_xor = 0; pm_dither_and = 1; pm_dither_or = 0; bpp = 1; break;
					case 'M': writePixel = WritePixelPM; pm_dither_xor = 1; pm_dither_and = 1; pm_dither_or = 0; bpp = 1; break;

					case 'k': writePixel = WritePixelPM; pm_dither_and = 0; pm_dither_or = 0; bpp = 1; break;
					case 'K': writePixel = WritePixelPM; pm_dither_and = 0; pm_dither_or = 1; bpp = 1; break;

					case 'p': writePixel = WritePixelGP32; bpp = 16; break;
					case 'g': writePixel = WritePixelGB; bpp = 16; break;
					case 'i': writePixel = WritePixelGP8; bpp = 8; break;
					case 'e': writePixel = WritePixel8; bpp = 8; break;
					case 't': writePixel = WritePixel24; bpp = 24; break;
					case '1': writePixel = WritePixelP1; bpp = 8; break;
					case 'c': writePixel = WritePixelGC; bpp = 32; break;
					case '?': Help(); exit(0); break;
				}
				if ((argv[a][i] >= '0') && (argv[a][i] <= '9'))
				{
					paletteFile = argv[++a];
					break;
				}
			}
		}
		else
		{
			if (!inputFile) inputFile = argv[a];
			else if (!outputFile) outputFile = argv[a];
			else
			{
				fprintf(stderr, "Error: Too many filenames given!\n");
			}
		}
	}

	if (!inputFile)
	{
		Help();
		exit(-1);
	}

	// read palette
	if (paletteFile)
	{
		// open bitmap file
		FILE *f = fopen(paletteFile, "rb");
		if (!f) { fprintf(stderr, "Error opening palette file!\n"); exit(-1); }
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);
	
		// read data
		//if (strstr(paletteFile, ".act"))
		if (size == 3*256)
		{
			// assume r,g,b...
			unsigned char paletteData[256][3];
			fread(paletteData, sizeof(paletteData), 1, f);

			for (int i=0; i<256; i++)
			{
				palette[i].rgbtRed = paletteData[i][0];
				palette[i].rgbtGreen = paletteData[i][1];
				palette[i].rgbtBlue = paletteData[i][2];
			}
		}
		else if (size == 4*256)
		{
			// assume r,g,b,x...
			unsigned char paletteData[256][4];
			fread(paletteData, sizeof(paletteData), 1, f);

			for (int i=0; i<256; i++)
			{
				palette[i].rgbtRed = paletteData[i][0];
				palette[i].rgbtGreen = paletteData[i][1];
				palette[i].rgbtBlue = paletteData[i][2];
			}
		}
		else
		{
			fprintf(stderr, "Unknown palette format!\n");
			exit(-1);
		}
		
		// close file
		fclose(f);
	}

	// read
	{
		// open bitmap file
		fi = fopen(inputFile, "rb");
		if (!fi) { fprintf(stderr, "Error opening bitmap file!\n"); exit(-1); }

		// read headers
		fread(&bfh, sizeof(bfh), 1, fi);
		fread(&bih, sizeof(bih), 1, fi);
		biWidth = input_endian(bih.biWidth);
		biHeight = input_endian(bih.biHeight);
		biBitCount = input_endian(bih.biBitCount);

		// checks
		if (input_endian(bih.biPlanes) != 1) { fprintf(stderr, "Unsupported number of planes!\n"); exit(-1); }
		if (input_endian(bih.biCompression) != 0) { fprintf(stderr, "Unsupported compression type!\n"); exit(-1); }

		// load to RGB format
		srcImage = new RGBTRIPLE[biWidth * biHeight + 4/*dummy*/];

		// load data with correct bit depth
		if (biBitCount <= 8)
		{
			//fprintf(stderr,"  The BMP is a 8bits image..\n");
			// read palette (quads -> triples)
			RGBQUAD paletteQ[256];
			RGBTRIPLE palette[256];
			fread(paletteQ, sizeof(paletteQ), 1, fi);
			for (int i=0; i<(1<<biBitCount); i++)
			{
				palette[i].rgbtRed = paletteQ[i].rgbRed;
				palette[i].rgbtGreen = paletteQ[i].rgbGreen;
				palette[i].rgbtBlue = paletteQ[i].rgbBlue;
			}
			
			// go to pixel data
			fseek(fi, input_endian(bfh.bfOffBits), SEEK_SET);
			
			int lineSize = biWidth * biBitCount / 8;
			int dummysize = ALIGN4(lineSize) - lineSize;
			int dummy;
			
			unsigned char *lineData = new unsigned char [lineSize];
	
			int byteparts = 8 / biBitCount;
			int mask = (1<<biBitCount)-1;

			// read & convert pixel data
			for (int y=biHeight-1; y>=0; y--)
			{
				RGBTRIPLE *pImageData = srcImage + (biWidth*y);		// srcImage is up-side down!
				fread(lineData, 1, lineSize, fi);
				fread(&dummy, 1, dummysize, fi);
				for (int x=0; x<biWidth; x++) pImageData[x] = palette[lineData[x / byteparts] >> ((byteparts - 1 - (x % byteparts)) * biBitCount) & mask];
			}
		}
		else if (biBitCount == 24)
		{
			//fprintf(stderr,"  The BMP is a 24bits image..\n");
			// go to pixel data
			fseek(fi, input_endian(bfh.bfOffBits), SEEK_SET);

			int lineSize = biWidth * 3;
			int dummysize = ALIGN4(lineSize) - lineSize;
			int dummy;
			//printf("%X\n", lineSize + dummysize);
	
			// read pixel data
			for (int y=biHeight-1; y>=0; y--)
			{
				RGBTRIPLE *pImageData = srcImage + biWidth*y;		// srcImage is up-side down!
				fread(pImageData, 1, lineSize, fi);
				fread(&dummy, 1, dummysize, fi);
			}
		}
		else
		{
			fprintf(stderr, "Unsupported bit depth!\n");
			exit(-1);
		}

		// close file
		fclose(fi);
	}

	// return width/height
	if (flags['w'])
	{
		printf("%d %d", biWidth, biHeight);
	}
	else if (flags['h'])
	{
		printf("%d %d", biHeight, biWidth);
	}
	if (!outputFile && (flags['w'] || flags['h'])) exit(0);

	if (!outputFile)
	{
		Help();
		exit(-1);
	}

	// write
	{
		fo = fopen(outputFile, "wb");
		if (!fo) { fprintf(stderr, "Error opening output file!\n"); exit(-1); }
		
		// add header
		if (flags['n'])
		{
			u32 _w = output_endian(biWidth);
			u32 _h = output_endian(biHeight);
			if (flags['w'])
			{
				fwrite(&_w, 4, 1, fo);
				fwrite(&_h, 4, 1, fo);
			}
			else if (flags['h'])
			{
				fwrite(&_h, 4, 1, fo);
				fwrite(&_w, 4, 1, fo);
			}
			else
			{
				printf("Please specify w or h option.\n");
				exit(-1);
			}
		}

		if (flags['k'] && flags['K'])
		{
			writePixel = WritePixelPM; pm_dither_and = 0; pm_dither_or = 0; bpp = 1; DoGfxThing(fo, 0, 8, 16);
			writePixel = WritePixelPM; pm_dither_and = 0; pm_dither_or = 1; bpp = 1; DoGfxThing(fo, 8, 8, 16);
		}
		else
		{
			DoGfxThing(fo);
		}

		// close files
		fclose(fo);
	}

	exit(0);
}
