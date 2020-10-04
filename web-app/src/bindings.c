#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <mtsdata.h>

void mtsd_error(mtsd_error_source src, int error, char* msg) {
  // TODO
}
void* mtsd_malloc(size_t size) {
  return malloc(size);
}
void* mtsd_realloc(void* ptr, size_t size) {
  return realloc(ptr, size);
}
void mtsd_free(void* ptr) {
  free(ptr);
}
