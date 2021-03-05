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
  uint32_t formattedOp = formatOp(0, 0, command);

  int result = jbod_operation(formattedOp, NULL);

  if (result == 0) {
    return 1;
  }
  return -1;
}

int mdadm_unmount(void) {
  uint32_t command = JBOD_UNMOUNT;
  uint32_t formattedOp = formatOp(0, 0, command);
  int result = jbod_operation(formattedOp, NULL);

  if (result == 0) {
    return 1;
  }
  return -1;
}

void translateAddress
(
  uint32_t addr,
  int *diskNum,
  int *blockNum,
  int *offset
) {
  int diskAddr = addr % JBOD_DISK_SIZE;

  *diskNum = addr / JBOD_DISK_SIZE;
  *blockNum = diskAddr / JBOD_BLOCK_SIZE;
  *offset = diskAddr % JBOD_BLOCK_SIZE;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  int maxAddr = JBOD_NUM_DISKS * JBOD_DISK_SIZE;
  int diskId, blockId, offset;
  translateAddress(addr, &diskId, &blockId, &offset);

  if (len > 1024) {
    return -1;
  }

  if (buf == NULL && len != 0) {
    return -1;
  }

  if ((addr + len) > maxAddr) {
    return -1;
  }

  uint32_t seekDiskOp = formatOp(0, diskId, JBOD_SEEK_TO_DISK);
  uint32_t seekBlockOp = formatOp(blockId, 0, JBOD_SEEK_TO_BLOCK);
  uint32_t readBlockOp = formatOp(0, 0, JBOD_READ_BLOCK);

  uint8_t tempBuf[255];

  jbod_operation(seekDiskOp, NULL);
  jbod_operation(seekBlockOp, NULL);

  int result = jbod_operation(readBlockOp, tempBuf);
  int blockOffset = 0;
  for (int i = 0; i < len; i++) {
    int tempBufIndex = offset + i - blockOffset;
    buf[i] = tempBuf[tempBufIndex];

    if (tempBufIndex == 255) {
      offset = 0;
      blockOffset = i + 1;
      jbod_operation(readBlockOp, tempBuf);
    }
  }

  if (result != 0) {
    return -1;
  }

  return len;
}
