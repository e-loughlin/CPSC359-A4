#include "gpio.h"
#include <stdio.h>

int map_peripheral(struct bcm2835_peripheral *p)
{
	if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0 {
		printf("Failed to open /dev/mem. Check permissions, or try using sudo. \n");
		return -1;
	}

	p->*map = mmap(
		NULL,
		BLOCK_SIZE,                   // BLOCK_SIZE = 4096
		PROT_READ|PROT_WRITE,         // Pages may be read, Pages may be written
		MAP_SHARED,                   // Share this mapping. Updates to the mapping are visible to other processes that map this file, and are carried through to the underlying file.
		fd,
		p->mem_fd,					  // File descriptor the physical memory virtual file '/dev/mem/'.
		p->addr_p					  // Address in physical map that we want this memory block to expose.
	);
	
	if (p->map == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	
	p->addr = (volatile unsigned int *)p->map;
	
	return 0;
}

// Unmap the peripheral device:
void unmap_peripheral(struct bcm2835_peripheral *p) {
	
	munmap(p->map, BLOCK_SIZE);
	close(p->mem_fd);
}

int main(
{
  if(map_peripheral(&gpio) == -1) 
  {
    printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
    return -1;
  }
 
  // Define pin 7 as output
  INP_GPIO(4);
  OUT_GPIO(4);
 
  while(1)
  {
    // Toggle pin 7 (blink a led!)
    GPIO_SET = 1 << 4;
    sleep(1);
 
    GPIO_CLR = 1 << 4;
    sleep(1);
  }
 
  return 0; 
}
