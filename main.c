
#include <stdlib.h>
#include <stdio.h>

#include <ctype.h>
#include <unistd.h>

#include <threads.h>
#include <stdatomic.h>

#include <string.h>

#include <complex.h>

struct rgb {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

typedef void (*coloring_f)(struct rgb *p, long double complex * zn, unsigned int iter);

struct calc_data {
  unsigned long width;
  unsigned long long pxmax;
  unsigned int imax;
  long double complex origin;
  long double delta;
  long double bailout;
  coloring_f coloring;
};

atomic_ullong g_current_pixel = 0;
struct rgb *g_pixels;

#define MODULO_COLOR_COUNT (6)
struct rgb g_modulo_colors[MODULO_COLOR_COUNT] = {
  {   0,   0, 255 },
  {   0, 255,   0 },
  {   0, 255, 255 },
  { 255,   0,   0 },
  { 255,   0, 255 },
  { 255, 255,   0 }
};

void modulo_coloring(struct rgb *p, long double complex *zn, unsigned int i) {
  *p = g_modulo_colors[i % MODULO_COLOR_COUNT];
}

void smooth_coloring(struct rgb *p, long double complex *zn, unsigned int i) {
  //long double f = i - log( log(sqrtl(creall(*zn) * creall(*zn) + cimagl(*zn) * cimagl(*zn))) / log
}


int mandelbrot_thread(void *calc_data) {
  struct calc_data *cd = calc_data;
  unsigned long width = cd->width;
  unsigned long long pxmax = cd->pxmax;
  unsigned int imax = cd->imax;
  long double origin_r = creall(cd->origin);
  long double origin_i = cimagl(cd->origin);
  long double delta = cd->delta;
  long double bailout = cd->bailout * cd->bailout;
  coloring_f coloring = cd->coloring;

  unsigned long long pixel;
  unsigned long x;
  unsigned long y;
  long double complex c, z;
  unsigned long i;
  while ((pixel = ++g_current_pixel) < pxmax) {
    x = pixel % width;
    y = pixel / width;
    c = (origin_r + x * delta) + (origin_i - y * delta) * I;
    i = 0;
    z = c;
    while (((creall(z) * creall(z) + cimagl(z) * cimagl(z)) < bailout) && (i < imax)) {
      z = z * z + c;
      i++;
    }
    //printf("pixel: %lld (%ld, %ld) C: %.3Lf + %.3Lf i   iterations: %ld\n", pixel, x, y, creall(c), cimagl(c), i);
    if (i >= imax) {
      g_pixels[pixel].r = 0;
      g_pixels[pixel].g = 0;
      g_pixels[pixel].b = 0;
    } else {
      coloring(&g_pixels[pixel], &z, i);
    }
  };
  return 0;
}


int main(int argc, char **argv) {

  unsigned long long width = 800;
  unsigned long long height = 600;
  unsigned long max_iter = 127;
  long double re = 0.0, im = 0.0;
  long double bailout = 2.0;
  long double zoom = 1.0;
  int thread_count = 2;
  char *filename = NULL;
  coloring_f coloring = NULL;
  int c;
  opterr = 0; // I want to provide my own help/error text. have to handle '?'.
  while ((c = getopt(argc, argv, "d:i:p:b:z:c:t:o:h")) != -1) {
    switch (c) {
      case 'd': // dimension
        if (2 != sscanf(optarg," %llu x %llu ", &width, &height)) {
          fprintf(stderr, "Argument for -d must be a dimension in the form \"WIDTH x HEIGHT\", not \"%s\".\n", optarg);
          return 1;
        }
        break;
      case 'i': // iterations
        if (1 != sscanf(optarg, " %lu ", &max_iter)) {
          fprintf(stderr, "Argument for -i must be an unsigned integer, not \"%s\".\n", optarg);
          return 1;
        }
        break;
      case 'p': // position
        if (2 != sscanf(optarg, " %La + %La i ", &re, &im)) {
          fprintf(stderr, "Argument for -p must be a complex number of the form \"REAL + IMAGINARY i\", not \"%s\".\n", optarg);
          return 1;
        }
        break;
      case 'b': // bailout
        if (1 != sscanf(optarg, " %La ", &bailout)) {
          fprintf(stderr, "Argument for -b must be a number, not \"%s\".\n", optarg);
          return 1;
        }
        break;
      case 'z': // zoom
        if (1 != sscanf(optarg, " %La ", &zoom)) {
          fprintf(stderr, "Argument for -z must be a number, not \"%s\".\n", optarg);
          return 1;
        }
        break;
      case 'c': // coloring
        if (0 == strcmp(optarg, "modulo")) {
          coloring = modulo_coloring;
        } else if (0 == strcmp(optarg, "smooth")) {

        }
        break;
      case 't': // threads
        if (1 != sscanf(optarg, " %d ", &thread_count)) {
          fprintf(stderr, "Argument for -t must be a integer, not \"%s\".\n", optarg);
          return 1;
        }
        break;
      case 'o': // output
        filename = malloc(sizeof(char) * (strlen(optarg) + 1));
        if (filename == NULL) {
          return 1;
        }
        strcpy(filename, optarg);
        break;
      case '?':
        printf("bad argument."); // which ? optopt ?
      case 'h': // help
        printf("hier könnte ihre werbung oder ein hilfetext stehen.\n\n");
        return 0;
     }
  }
  if (filename == NULL) {
    filename = "fraktal.ppm";
  }
  if (coloring == NULL) {
    coloring = modulo_coloring;
  }

  /* ************************************************ */

  g_pixels = malloc(sizeof(struct rgb) * width * height);
  if (g_pixels == NULL) {
    return 1;
  }

  struct calc_data cd;
  cd.width = width;
  cd.pxmax = width * height;
  cd.imax = max_iter;
  cd.delta = 4.0 / (width * zoom);
  cd.origin = (re - cd.delta * (width / 2.0)) + (im + cd.delta * (height / 2.0)) * I;
  cd.bailout = bailout;
  cd.coloring = coloring;

  thrd_t t[thread_count];
  for (int i = 0; i < thread_count; i++) {
    thrd_create(&t[i], mandelbrot_thread, &cd);
  }
  for (int i = 0; i < thread_count; i++) {
    thrd_join(t[i], NULL);
  }

  FILE *file = fopen(filename, "w");
  if (file == NULL) {
    printf("couldn't create file.");
    return 1;
  }
  fprintf(file, "P6 %lld %lld 255\n", width, height);
  for(unsigned long long i = 0; i < width * height; i++) {
    //fwrite(g_pixels, sizeof(struct rgb), width * height, file);
    fputc(g_pixels[i].r, file);
    fputc(g_pixels[i].g, file);
    fputc(g_pixels[i].b, file);
  }
  fclose(file);
  free(g_pixels);
  //free(filename);


  return 0;
}
