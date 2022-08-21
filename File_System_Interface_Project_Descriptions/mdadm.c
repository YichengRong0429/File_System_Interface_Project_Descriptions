#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"

int mdadm_mount(void) {
  return -1;
}

int mdadm_unmount(void) {
  return -1;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  return len;
}

int mdadm_write(uint32_t addr, uint32_t len, const uint8_t *buf) {
  return len;
}
