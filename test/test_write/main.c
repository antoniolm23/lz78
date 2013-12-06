#include "bitio.h"

int main() {
  struct bitio* b=bit_open("prova.txt", "w");
  uint64_t a;
  bitio_write(b, 'a', 64);
  bitio_write(b, 'b', 64);
  bitio_write(b, 'c', 32);
  bitio_write(b, 'd', 8);
  bitio_write(b, 'd', 8);
  bitio_write(b, 'd', 8);
  bitio_write(b, 0x08, 6);
  bitio_write(b, 0xefafbfcf, 32);
  bitio_write(b, 'a', 64);
  bitio_write(b, 'b', 64);
  bitio_write(b, 'a', 64);
  bitio_close(b);
  b=bit_open("prova.txt", "r");
  return 0;
}
