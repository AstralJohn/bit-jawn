#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"

struct Op {
  uint32_t blockId : 8;
  uint32_t diskId : 4;
  uint32_t command: 6;
};

uint32_t formatOp (uint32_t blockId, uint32_t diskId, uint32_t command) {
  struct Op op = {
    blockId&0xff,
    diskId&0xf,
    command&0x3f
  };

  uint32_t returnOp = 0x0, tempBlockId, tempDiskId, tempCommand;

  tempBlockId = op.blockId;
  tempDiskId = op.diskId << 22;
  tempCommand = op.command << 26;

  returnOp = tempBlockId|tempDiskId|tempCommand;
  /*printf("\n0x%08x\n", returnOp);*/

  return returnOp;
}

int mdadm_mount(void) {
  uint32_t command = JBOD_MOUNT;

  for (uint32_t diskId = 0; diskId < JBOD_NUM_DISKS; diskId++) {
    for (uint32_t blockId = 0; blockId < JBOD_NUM_BLOCKS_PER_DISK; blockId++) {

      uint32_t formattedOp = formatOp(blockId, diskId, command);

      int result = jbod_operation(formattedOp, NULL);

      if (result == 0) {
        return 1;
      }

    }
  }
  return -1;
}

int mdadm_unmount(void) {
  uint32_t command = JBOD_UNMOUNT;

  for (uint32_t diskId = 0; diskId < JBOD_NUM_DISKS; diskId++) {
    for (uint32_t blockId = 0; blockId < JBOD_NUM_BLOCKS_PER_DISK; blockId++) {

      uint32_t formattedOp = formatOp(blockId, diskId, command);
      int result = jbod_operation(formattedOp, NULL);

      if (result == 0) {
        return 1;
      }

    }
  }
  return -1;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  return len;
}
