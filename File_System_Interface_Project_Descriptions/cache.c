#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;

int cache_create(int num_entries) {
  return -10;
}

int cache_destroy(void) {
  return -10;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  return -10;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  return -10;
}

bool cache_enabled(void) {
  return false;
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
