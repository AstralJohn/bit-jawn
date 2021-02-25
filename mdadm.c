#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"


  // Loop though 256 blocks in a disk
  /*for (uint8_t i = 0; i < 256; i++) {*/
    /*printf("\nI am I: %d\n", i);*/
  /*}*/

uint32_t createOp(uint32_t *command, uint32_t *diskId, uint32_t *blockId) {

  uint32_t returnOp = 0x0, tempBlockId, tempDiskId, tempCommand;

  tempBlockId = *blockId&0xff; // 8 bits
  printf("\nTempBlockId: 0x%08x\n", tempBlockId);
  printf("\nOriginalDiskId: 0x%08x\n", *diskId);
  tempDiskId  = (*diskId&0xf) << 22; // 4 bits
  printf("\nTempDiskId: 0x%08x\n", tempDiskId);
  // 14 bits of reserved (free) space
  printf("\nOriginalCommand: 0x%08x\n", *command);
  tempCommand = *command&0x3f << 26; // 6 bits
  
  printf("\nTempCommand: 0x%08x\n", tempCommand);

  returnOp = tempCommand|tempDiskId|tempBlockId;
  printf("\n0x%08x\n", returnOp);

  return returnOp;

}

int mdadm_mount(void) {

  uint32_t diskId = 0, blockId = 0;
  uint32_t *diskIdPointer = &diskId;
  uint32_t *blockIdPointer = &blockId;
  uint32_t command = JBOD_MOUNT;
  uint32_t *commandPointer = &command;

  for (diskId = 0; diskId < JBOD_NUM_DISKS; diskId++) {
    for (blockId = 0; blockId < JBOD_BLOCK_SIZE; blockId++) {
      uint32_t op = createOp(commandPointer, diskIdPointer, blockIdPointer);
      int result = jbod_operation(op, NULL);
      printf("\n Passing? %d\n", result);
      return result;
    }
  }
  return -1;
}

int mdadm_unmount(void) {
  return -1;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  return len;
}
