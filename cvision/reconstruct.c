#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define I 3
#define J 3
#define D 7

#define F 500.0
#define B 0.1

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int rowlen;
  unsigned char *data;
} BITMAP;

void die(char *str) {
  fprintf(stderr, "Error: %s\n", str);
  exit(1);
}

inline unsigned long getLong(unsigned char *buf) {
  return (unsigned long)buf[0] + ((unsigned long)buf[1] << 8) + ((unsigned long)buf[2] << 16) + ((unsigned long)buf[3] << 24);
}
inline unsigned int getInt(unsigned char *buf) {
  return (unsigned int)buf[0] + ((unsigned int)buf[1] << 8);
}

BITMAP *load_bmp(char *file) {
  static byte buf[54] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0,
    40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
  FILE *f;
  unsigned int w, h, r;
  unsigned long filesize;
  f = fopen(file, "rb");
  fread(buf, 1, 54, f);
  if (buf[0] != 'B' && buf[1] != 'M') die("Not a bitmap");
  w = getLong(buf + 18);
  h = getLong(buf + 22);
  r = w;
  while (r & 3) r++; // rowlen divisible by 4
  if (getInt(buf + 28) != 24) die("Not a 24-bit BMPs");
  if (getLong(buf + 30)) die("Compressed");
  BITMAP *b = (BITMAP *)malloc(sizeof(BITMAP));
  b->width = w;
  b->height = h;
  b->rowlen = r;
  b->data = (unsigned char *)malloc(filesize = r * h * 3 + 54);
  printf("read size %dx%d = %lu\n", b->rowlen, b->height, filesize);
  fread(b->data + 54, 1, r * h * 3, f);

  buf[ 2] = (unsigned char)(filesize      );
  buf[ 3] = (unsigned char)(filesize >>  8);
  buf[ 4] = (unsigned char)(filesize >> 16);
  buf[ 5] = (unsigned char)(filesize >> 24);

  buf[ 4 + 14] = (unsigned char)(b->width       );
  buf[ 5 + 14] = (unsigned char)(b->width  >>  8);
  buf[ 6 + 14] = (unsigned char)(b->width  >> 16);
  buf[ 7 + 14] = (unsigned char)(b->width  >> 24);
  buf[ 8 + 14] = (unsigned char)(b->height      );
  buf[ 9 + 14] = (unsigned char)(b->height >>  8);
  buf[10 + 14] = (unsigned char)(b->height >> 16);
  buf[11 + 14] = (unsigned char)(b->height >> 24);

  memcpy(b->data, buf, 54);
  return b;
}

void free_bmp(BITMAP *b) {
  free(b->data);
  free(b);
}

BITMAP *clone_bmp(BITMAP *b) {
  unsigned int size = b->rowlen * b->height * 3 + 54;
  BITMAP *c = (BITMAP *)malloc(sizeof(BITMAP));
  c->data = (unsigned char *)malloc(size);
  memcpy(c->data, b->data, size);
  c->width = b->width;
  c->height = b->height;
  c->rowlen = b->rowlen;
  return c;
}

inline void setPixel(BITMAP *b, unsigned int x, unsigned int y,
    unsigned int rr, unsigned int gg, unsigned int bb) {
  if (x >= b->width || y >= b->height) return;
  int offset = 54 + (x + b->rowlen * (b->height - y - 1)) * 3;
  b->data[offset] = rr;
  b->data[offset + 1] = gg;
  b->data[offset + 2] = bb;
}

inline unsigned char red(BITMAP *b, unsigned int x, unsigned int y) {
  return b->data[56 + ((x + b->rowlen * (b->height - y - 1)) * 3)];
}
inline unsigned char grn(BITMAP *b, unsigned int x, unsigned int y) {
  return b->data[55 + ((x + b->rowlen * (b->height - y - 1)) * 3)];
}
inline unsigned char blu(BITMAP *b, unsigned int x, unsigned int y) {
  return b->data[54 + ((x + b->rowlen * (b->height - y - 1)) * 3)];
}

inline double intensity(BITMAP *b, int x, int y) {
  if (x < 0 || y < 0 || x >= b->width || y >= b->height) return 0;
  return red(b, x, y) * 0.2989 + grn(b, x, y) * 0.5870 + blu(b, x, y) * 0.1140;
}


void save_bmp(BITMAP *b, char *filename) {
  FILE *f = fopen(filename, "wb");
  printf("write size %dx%d, filesize %u\n", b->width, b->height, 54 + b->rowlen * b->height * 3);
  fwrite(b->data, 1, 54 + b->rowlen * b->height * 3, f);
}




double sad(BITMAP *ref, BITMAP *img, int x0, int x1, int y) {
  // TODO
}

double ssd(BITMAP *ref, BITMAP *img, int x0, int x1, int y) {
  double item, result = 0;
  int i, j;
  for (j = -J; j <= J; j++) {
    for (i = -I; i <= I; i++) {
      //item = intensity(img, x0 + i, y + j) - intensity(ref, x1 + i, y + j);
      //result += item * item;
      int r = red(img, x0 + i, y + j) - red(ref, x1 + i, y + 1);
      int g = grn(img, x0 + i, y + j) - grn(ref, x1 + i, y + 1);
      int b = blu(img, x0 + i, y + j) - blu(ref, x1 + i, y + 1);
      result += r * r + g * g + b * b;
    }
  }
  return result;
}

double ncc(BITMAP *ref, BITMAP *img, int x0, int x1, int y) {
  // TODO
}


double minimum(int from, int to, double *values) {
  int x = from;
  double y = values[0];
  int i = to - from;

  // find least value
  while (--i) {
    if (values[i] < y) {
      x = from + i;
      y = values[i];
    }
  }
  if (x == from || x == to) return x;

  int xp = x + 1;
  int xm = x - 1;
  int x2 = x * x;
  int xp2 = xp * xp;
  int xm2 = xm * xm;
  double ym = values[x - from - 1];
  double yp = values[x - from + 1];
  // system of 3 linear equations at [xm, x, xp]:
  // m(d) = ad^2 + bd + c
  double k = (x2 - xp2) / (x - xp);
  double a = ((y - yp) / (x - xp) - (y - ym) / (x - xm)) / (k - (x2 - xm2) / (x - xm));
  double b = (y - yp) / (x - xp) - a * k;
  if (a == 0 && b == 0) return x;
  // not needed:
  // double c = y - b * x - a * x2;
  
  // minimum is at the null-point of the derivation of the quadratic curve
  return -b / (2 * a);
}

double disparity(BITMAP *ref, BITMAP *img, unsigned int x, unsigned int y, int r) {
  double values[ref->width];
  int i;
  for (i = 0; i <= 2 * D; i++) {
    values[i] = ssd(ref, img, x + r, x + r + i, y);
  }
  return x - minimum(x - D, x + D, values) - r;
}

#define DEBUG_ROWS_START 0
#define DEBUG_ROWS_END (c->height)

int main(int argc, char **argv) {
  BITMAP *c = load_bmp("viewC.bmp");
  BITMAP *l = load_bmp("viewL.bmp");
  BITMAP *r = load_bmp("viewR.bmp");
  BITMAP *db = clone_bmp(c);
  int x, y;
  double xx, yy, zz;
  double dm[c->width * c->height];
  double dl, dr;
  int item;
  double avg, sd, mind, maxd;
  FILE *w;
  //for (y = 0; y < c->height; y++) {
  for (y = DEBUG_ROWS_START; y < DEBUG_ROWS_END; y+=2) {
    printf("row %d\n", y);
    //for (x = 0; x < c->width; x++) {
    for (x = D; x < c->width - D; x++) {
      dl = disparity(c, l, x, y, 0);
//      dr = disparity(c, r, x, y, I);
//      if (-dr < dl) dl = -dr;
      dm[x + c->width * y] = dl;
      sd += dl * dl;
      avg += dl;
    }
  }
  sd = sqrt(sd / (c->width * c->height));
  avg = avg / (c->width * c->height);
  mind = avg - 3 * sd;
  maxd = avg + 3 * sd;
  printf("%f to %f\n", mind, maxd);
  w = fopen("points.dat", "w");
  //for (y = 0; y < c->height; y++) {
  for (y = DEBUG_ROWS_START; y < DEBUG_ROWS_END; y+=2) {
    //for (x = 0; x < c->width; x++) {
    for (x = D; x < c->width - D; x++) {
      dl = dm[x + c->width * y];
      item = (int)((dl - mind) * 256 / (maxd - mind));
      if (item < 0) item = 0;
      else if (item > 255) item = 255;
      zz = - B * F / dl;
      xx = - x * zz / F;
      yy = - y * zz / F;
      setPixel(db, x, y, item, item, item);
      fprintf(w, "%f\t%f\t%f\t%d\t%d\t%d\t%d\t%d\n", xx, yy, zz, red(c, x, y), grn(c, x, y), blu(c, x, y), x, y);
    }
  }
  fclose(w);
  save_bmp(db, "dist-viewL3.bmp");
  free_bmp(db);
  free_bmp(l);
  free_bmp(c);
}

