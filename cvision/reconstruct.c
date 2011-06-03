#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// image parameters
#ifndef F
#define F 500.0
#endif
#ifndef B
#define B 0.1
#endif
#ifndef ZMIN
#define ZMIN 2.0
#endif
#ifndef ZMAX
#define ZMAX 7.5
#endif

// matcher box width and height
#ifndef IJ
#define IJ 5
#endif
#ifndef I
#define I IJ
#endif
#ifndef J
#define J IJ
#endif

#ifndef MATCHER
// matcher; possible values: sad, ssd, ncc
#define MATCHER ncc
#endif



#define BR B
#define BL (-B)
#define MAXIJ ((2 * I + 1) * (2 * J + 1))

#define RED(b, x, y) (b->data[56 + ((x + b->rowlen * (b->height1 - y)) * 3)])
#define GRN(b, x, y) (b->data[55 + ((x + b->rowlen * (b->height1 - y)) * 3)])
#define BLU(b, x, y) (b->data[54 + ((x + b->rowlen * (b->height1 - y)) * 3)])

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;

typedef struct {
  unsigned int width;
  unsigned int height;
  unsigned int height1;
  unsigned int rowlen;
  unsigned char *data;
} BITMAP;

void die(char *str) {
  fprintf(stderr, "Error: %s\n", str);
  exit(1);
}

unsigned long get_long(unsigned char *buf) {
  return (unsigned long)buf[0] + ((unsigned long)buf[1] << 8) + ((unsigned long)buf[2] << 16) + ((unsigned long)buf[3] << 24);
}

unsigned int get_int(unsigned char *buf) {
  return (unsigned int)buf[0] + ((unsigned int)buf[1] << 8);
}

BITMAP *load_bmp(char *file) {
  static byte buf[54];
  FILE *f;
  size_t dummy;
  unsigned int w, h, r;
  f = fopen(file, "rb");
  dummy = fread(buf, 1, 54, f);
  if (buf[0] != 'B' && buf[1] != 'M') die("Not a bitmap");
  w = get_long(buf + 18);
  h = get_long(buf + 22);
  r = w;
  while (r & 3) r++; // rowlen divisible by 4
  if (get_int(buf + 28) != 24) die("Not a 24-bit BMPs");
  if (get_long(buf + 30)) die("Compressed");
  BITMAP *b = (BITMAP *)malloc(sizeof(BITMAP));
  b->width = w;
  b->height = h;
  b->height1 = h - 1;
  b->rowlen = r;
  b->data = (unsigned char *)malloc(r * h * 3 + 54);
  dummy = fread(b->data + 54, 1, r * h * 3, f);
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

inline void set_pixel(BITMAP *b, unsigned int x, unsigned int y,
    unsigned int rr, unsigned int gg, unsigned int bb) {
  if (x >= b->width || y >= b->height) return;
  int offset = 54 + (x + b->rowlen * (b->height - y - 1)) * 3;
  b->data[offset] = rr;
  b->data[offset + 1] = gg;
  b->data[offset + 2] = bb;
}

void save_bmp(BITMAP *b, char *filename) {
  FILE *f = fopen(filename, "wb");
  fwrite(b->data, 1, 54 + b->rowlen * b->height * 3, f);
}


// matchers

double sad(BITMAP *ref, BITMAP *img, int x0, int x1, int y) {
  double item, result = 0;
  int int0, int1;
  int i, yy, xx0, xx1;

  // make sure we don't access outside the bounds
  int tsi = MIN(I, x0);
  int si = -MIN(tsi, x1);
  int tei = MIN(I, img->width - x0);
  int ei = MIN(tei, img->width - x1);

  int sy = MAX(y - J, 0);
  int ey = MIN(y + J + 1, img->height);
  if (si >= ei || sy >= ey) return 0;
  int size = (ei - si) * (ey - sy);
  if (!size) return result;

  // sum the value at each pixel
  for (i = si; i < ei; i++) {
    xx0 = x0 + i;
    xx1 = x1 + i;
    for (yy = sy; yy < ey; yy++) {
      int0 = RED(ref, xx0, yy) + GRN(ref, xx0, yy) + BLU(ref, xx0, yy);
      int1 = RED(img, xx1, yy) + GRN(img, xx1, yy) + BLU(img, xx1, yy);

      // cheap ABS
      item = int0 - int1;
      if (item < 0) item = -item;

      result += item;
    }
  }
  // normalize by number of data points (for image edges)
  return result / size;
}

double ssd(BITMAP *ref, BITMAP *img, int x0, int x1, int y) {
  double result = 0;
  int r, g, b;
  int i, yy, xx0, xx1;

  // make sure we don't access outside the bounds
  int tsi = MIN(I, x0);
  int si = -MIN(tsi, x1);
  int tei = MIN(I, img->width - x0);
  int ei = MIN(tei, img->width - x1);

  int sy = MAX(y - J, 0);
  int ey = MIN(y + J + 1, img->height);
  if (si >= ei || sy >= ey) return 0;
  int size = (ei - si) * (ey - sy);
  if (!size) return result;

  // sum the value at each pixel
  for (i = si; i < ei; i++) {
    xx0 = x0 + i;
    xx1 = x1 + i;
    for (yy = sy; yy < ey; yy++) {
      r = RED(ref, xx0, yy) - RED(img, xx1, yy);
      g = GRN(ref, xx0, yy) - GRN(img, xx1, yy);
      b = BLU(ref, xx0, yy) - BLU(img, xx1, yy);

      // for better matching, use each channel separately
      result += r * r + g * g + b * b;
    }
  }
  // normalize by number of data points (for image edges)
  return result / size;
}

// allocate helper vectors only once, instead for every pixel
double im0[MAXIJ], im1[MAXIJ];

double ncc(BITMAP *ref, BITMAP *img, int x0, int x1, int y) {
  double item, result = 0;
  int int0, int1;
  double int0avg = 0, int1avg = 0;
  int i, yy, xx0, xx1;

  // make sure we don't access outside the bounds
  int tsi = MIN(I, x0);
  int si = -MIN(tsi, x1);
  int tei = MIN(I, img->width - x0);
  int ei = MIN(tei, img->width - x1);

  int sy = MAX(y - J, 0);
  int ey = MIN(y + J + 1, img->height);
  if (si >= ei || sy >= ey) return 0;
  int size = (ei - si) * (ey - sy);

  // calculate the value at each pixel and store
  // calculate the average values
  int c = 0;
  for (i = si; i < ei; i++) {
    xx0 = x0 + i;
    xx1 = x1 + i;
    for (yy = sy; yy < ey; yy++) {
      int0avg += im0[c] = RED(ref, xx0, yy) + GRN(ref, xx0, yy) + BLU(ref, xx0, yy);
      int1avg += im1[c] = RED(img, xx1, yy) + GRN(img, xx1, yy) + BLU(img, xx1, yy);
      c++;
    }
  }
  int0avg /= size;
  int1avg /= size;

  double sum = 0, sum0 = 0, sum1 = 0;
  for (c = 0; c < size; c++) {
    item = im0[c] - im1[c];
    sum += item * item;

    item = im0[c] - int0avg;
    sum0 += item * item;

    item = im1[c] - int1avg;
    sum1 += item * item;
  }

  // square the whole fraction to avoid sqrt call
  return sum * sum / sum0 / sum1;
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

#define DEBUG_ROWS_START 0
#define DEBUG_ROWS_END (c->height)

int main(int argc, char **argv) {
  BITMAP *c = load_bmp("viewC.bmp");
  BITMAP *l = load_bmp("viewL.bmp");
  BITMAP *r = load_bmp("viewR.bmp");
  BITMAP *db = clone_bmp(c);
  int xc, yc, d;
  int xh = c->width / 2;
  int yh = c->height / 2;
  int sd = floor(B * F / ZMAX);
  int ed = ceil(B * F / ZMIN);
  double z, xl, xr;
  double ml, mr;
  double m[ed - sd + 1];
  double md;
  FILE *w = fopen("points.txt", "w");
  fprintf(w, "%d\n", c->height * c->width);

  for (xc = 0; xc < c->width; xc++) {
    printf("\rCol %d", xc);
    fflush(stdout);
    for (yc = 0; yc < c->height; yc++) {
      for (d = sd; d <= ed; d++) {
        z = B * F / d;
        xl = xc - BL * F / z;
        xr = xc - BR * F / z;
        ml = MATCHER(c, l, xc, xl, yc);
        mr = MATCHER(c, r, xc, xr, yc);
        // take the more similar side
        m[d - sd] = ml < mr ? ml : mr;
      }
      md = minimum(sd, ed, m);
      z = -B * F / md;
      fprintf(w, "%f\t%f\t%f\t%d\t%d\t%d\n",
          (xh - xc) * z / F,
          (yc - yh) * z / F,
          z,
          RED(c, xc, yc),
          GRN(c, xc, yc),
          BLU(c, xc, yc));

      // DEBUG:
      db->data[56 + (xc + db->rowlen * (db->height - yc - 1)) * 3] = (md - sd) * 256 / (ed - sd);
    }
  }
  save_bmp(db, "dist-view.bmp");
  free_bmp(db);
  free_bmp(l);
  free_bmp(c);
  printf("\r            \r");
}
