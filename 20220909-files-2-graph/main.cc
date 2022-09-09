#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <cmath>

const int WIDTH = 1920, HEIGHT = 1080;
const double SX = 30.0, SY = 30.0, SZ = 400.0;
const double
		XMIN=-20.0, XMAX=20.0,
		YMIN=-20.0, YMAX=20.0,
		STEP_SMALL=0.001, STEP_BIG=1.0;

#pragma pack(push, 1)
struct BMPFILEHEADER
{
	char     bm[2];
	uint32_t file_size;
	uint16_t res1, res2;
	uint32_t image_offset;
};

struct BITMAPINFOHEADER
{
	uint32_t header_size;
	int32_t  width;
	int32_t  height;
	uint16_t planes;
	uint16_t bpp;
	uint32_t compression;
	uint32_t image_size;
	uint32_t hres, vres;
	uint32_t pal_size, pal_used;
};
#pragma pack(pop)

typedef std::vector<uint32_t> pixels_t;

struct Image
{
	int width, height;
	pixels_t pixels;

	Image(int w, int h)
	{
		width = w;
		height = h;
		pixels.resize(w*h);
	}
};


void write_bmp(const char * name, const Image &img)
{
	BMPFILEHEADER    h1;
	BITMAPINFOHEADER h2;
	h1.bm[0] = 'B';
	h1.bm[1] = 'M';
	h1.file_size = sizeof(h1) + sizeof(h2) +
			img.pixels.size() * sizeof(img.pixels[0]);
	h1.res1 = h1.res2 = 0;
	h1.image_offset = sizeof(h1) + sizeof(h2);
	h2.header_size = sizeof(h2);
	h2.width = img.width;
	h2.height = img.height;
	h2.planes = 1;
	h2.bpp = 32;
	h2.compression = 0;
	h2.image_size = img.pixels.size() * sizeof(img.pixels[0]);
	h2.hres = 2048;
	h2.vres = 2048;
	h2.pal_size = 0;
	h2.pal_used = 0;

	std::ofstream bmp_file { name, std::ios::binary };
	bmp_file.write(reinterpret_cast<const char*>(&h1), sizeof(h1));
	bmp_file.write(reinterpret_cast<const char*>(&h2), sizeof(h2));
	bmp_file.write(reinterpret_cast<const char*>(&img.pixels[0]),
			h2.image_size);
	bmp_file.close();
}

void clear_image(Image &img, uint32_t colour)
{
	for (auto &&p: img.pixels) p = colour;
}

double sinc(double x)
{
	if (x == 0.0) return 1.0;
	return sin(x) / x;
}

double fun(double x, double y)
{
	return sinc(hypot(x, y));
}

const double Pi = acos(-1.0);

void isometry_convert(
		int width, int height,
		double x, double y, double z,
		int &sx, int &sy)
{
	sx = int(width/2  + SX*x*cos(Pi/6) - SY*y*cos(Pi/6));
	sy = int(height/2 + SX*x*sin(Pi/6) + SY*y*sin(Pi/6) - SZ*z);
}

void plot(Image &image, int x, int y, uint32_t colour)
{
	if (x < 0 or y < 0 or x >= image.width or y >= image.height)
		return;
	image.pixels[y * image.width + x] = colour;
}

int main()
{
	Image image(WIDTH, HEIGHT);

	clear_image(image, 0xff003f00);

	int sx, sy;
	double x, y, z;
	for (x = XMAX; x >= XMIN; x -= STEP_BIG)
		for (y = YMAX; y >= YMIN; y -= STEP_SMALL) {
			z = fun(x, y);
			isometry_convert(image.width, image.height, x, y, z, sx, sy);
			plot(image, sx, image.height - sy, 0xffffffff);
		}

	write_bmp("output.bmp", image);

	return 0;
}
