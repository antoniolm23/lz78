#include "bitio.h"

int main() {
  struct bitio* b=bit_open("prova.txt", "w");
  uint64_t a=0;
  bitio_write(b, 'a', 64);
  bitio_write(b, 'b', 64);
  bitio_write(b, 'c', 32);
  bitio_write(b, 'd', 8);
  bitio_write(b, 'd', 8);
  bitio_write(b, 'd', 8);
  bitio_write(b, 0x08, 6);
  bitio_write(b, 0x33, 9);
  bitio_write(b, 'a', 64);
  bitio_write(b, 'b', 64);
  bitio_write(b, 'a', 64);
  bitio_close(b);
  struct bitio* f=bit_open("prova.txt", "r");
  bitio_read(f, &a, 64);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 64);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 32);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 8);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 8);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 8);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 6);
  fprintf(stderr, "%x\n",(int)a);
  bitio_read(f, &a, 9);
  fprintf(stderr, "%x\n",(int)a);
  bitio_close(f);
  return 0;
}
