// CPSC 359 - Computing Machinery II
// Assignment 4
// gpio.c: Mapping of GPIO pins for the Raspberry Pi to interface
// with the I2C

// Mapping the RPi's physical I/O space into your program's virtual memory space.

// Modified by Evan Loughlin, Teale Masrani, and Joseph Gorespe
// Date: 2017.12.02


// Include the header file, which defines all the includes required.
#include "gpio.h"

// Open returns -1 if there is an error. (Most likely caused by not having sudo)
// If mmap fails, it returns a defined constant, MAP_FAILED
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

// This code works by first opening /dev/mem, and Raspbian device driver that provides 
// access to the memory of the RPi. 

// We then use the handle fd with mmap to map the physical memory space starting at addr_p into
// the virtual memory space. After mmap returns successfully, you can manipulate the 
// corresponding I/O devices in the RPi using the *map pointer. *map behaves like an array
// of the mapped I/O registers. After that, I/O is a matter of manipulating (reading and writing)
// the I/O registers. 






