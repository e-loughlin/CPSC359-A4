/*
 * CPSC 359 - Assignment 4
 * Using the I2C Protocol to map the SDA and SCL, and connect peripheral devices.
 * Works on the Raspberry Pi 2 and 3
 * 
 * smbus.h: Header file for smbus code.
 * 
 * Evan Loughlin
 * Teale Masrani
 * Joseph Gorospe
 * 
 * Date: Dec. 4 2017
 * 
 * Reference materials:
 * 1) https://github.com/Pieter-Jan/PJ_RPI
 * 2) CPSC 359 Course Material, special help from Andrew Groene and Dr. Boyd.
 */

#include "smbus.h" // Header file that contains all macros and definitions.

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral bsc1 = {BSC1_BASE};



// Map Peripheral: Exposes the physical address of the RPI from the virtual address
// using mmap on /dev/mem

int map_peripheral(struct bcm2835_peripheral *p)
{
	// Open /dev/mem
	if((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("Failed to open /dev/mem. Try running the program again with sudo permissions.\n");
		return -1;
	}
	
	p->map = mmap(
		NULL,
		BLOCK_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		p->mem_fd,   			// File descriptor to physical memory virtual file '/dev/mem'
		p->addr_p				// Address in physical map, which we want this memory block to expose.
	);
		
	if (p->map == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	
	p->addr = (volatile unsigned int *)p->map;
	
	return 0;
	
}





// Unmap Peripheral: Unmaps the previously mapped memory addresses from a peripheral device.

void unmap_peripheral(struct bcm2835_peripheral *p) {
	
	munmap(p->map, BLOCK_SIZE);
	close(p->mem_fd);
}



// Dump BSC Status: Prints the status flags out for BSC1.
void dump_bsc_status() {
	
	unsigned int s = BSC1_STATUS;
	
	printf("BSC1_STATUS: ERR=%d RXF=%d TXE=%d TXD=%d RXR=%d TXW=%d DONE=%d TA=%d\n",
		(s & BSC_S_ERR) != 0,
		(s & BSC_S_RXF) != 0,
		(s & BSC_S_TXE) != 0,
		(s & BSC_S_RXD) != 0,
		(s & BSC_S_TXD) != 0,
		(s & BSC_S_RXR) != 0,
		(s & BSC_S_TXW) != 0,
		(s & BSC_S_DONE) != 0,
		(s & BSC_S_TA) != 0);
}


// Wait I2C Done: Function that waits for the I2C transaction to complete.
void wait_i2c_done() {
	int timeout = 50;
	while((!((BSC1_STATUS) & BSC_S_DONE)) && --timeout) {
		usleep(1000);
	}
	
	if(timeout == 0)
		printf("wait_i2c_done() timeout. Something ain't right, fool.\n");
}


// Sets GPIO2 and GPIO3 pins for initialization.
void i2c_init()
{
	INP_GPIO(2);
	SET_GPIO_ALT(2,0);
	INP_GPIO(3);
	SET_GPIO_ALT(3,0);
}


// Priority
int SetProgramPriority(int priorityLevel)
{
	struct sched_param sched;
	
	memset (&sched, 0, sizeof(sched));
	
	if (priorityLevel > sched_get_priority_max (SCHED_RR))
		priorityLevel = sched_get_priority_max (SCHED_RR);
		
	sched.sched_priority = priorityLevel;
		
	return sched_setscheduler (0, SCHED_RR, &sched);
}



// Initialize the bus by mapping the peripherals and setting GPIO 2 and GPIO 3.
int init()
{
	if(map_peripheral(&gpio) == -1) 
	{
       	 	printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        	return -1;
    }

  printf( "mmap successful: %08x\n", GPIO_BASE );

	if(map_peripheral(&bsc1) == -1) 
	{
       	 	printf("Failed to map the physical BSC1 registers into the virtual memory space.\n");
        	return -1;
    }

  printf( "mmap successful: %08x\n", BSC1_BASE );

  // Initialize the GPIO pins.
  i2c_init();
  
}


// Uninitialize the bus 

void uninit()
{
	unmap_peripheral(&gpio);
	unmap_peripheral(&bsc1);
}

// Function for reading byte data

unsigned char read_byte_data(unsigned char address, unsigned char regAddress) 
{
  
  BSC1_STATUS = CLEAR_STATUS; 							// Clear CLKT, ERR, DONE flags
  BSC1_SLAVEADDR = address;  							// Set peripheral address
  BSC1_DLEN = 1;        				 				// Set length of data to be transmitted to 1 byte
  BSC1_FIFO = regAddress;  								// Add the peripheral's offset register to FIFO stack
  BSC1_CONTROL = START_WRITE;  							// I2CEN, ST set to 1 (Enable I2C and Start Transfer)

  // Wait until done
  wait_i2c_done();
  
  // Get the response from the register
  BSC1_STATUS = CLEAR_STATUS; 							// Clear CLKT, ERR, DONE flags
  BSC1_DLEN = 1;        				 				// Set length of data to be transmitted to 1 byte
  BSC1_CONTROL = START_READ;  							// I2CEN, ST set to 1 (Enable I2C and Start Transfer)
  

  // Wait until done
  wait_i2c_done();

  unsigned int data = BSC1_FIFO;
  BSC1_STATUS = CLEAR_STATUS;
  BSC1_CONTROL |= BSC_C_CLEAR;
  
  return data;
}


void write_byte_data(unsigned char address, unsigned char regAddress, unsigned char data)
{
  BSC1_STATUS = CLEAR_STATUS; 							// Clear CLKT, ERR, DONE flags
  BSC1_SLAVEADDR = address;  							// Set peripheral address
  BSC1_DLEN = 2;        				 				// Set length of data to be transmitted to 2 byte
  BSC1_FIFO = regAddress;  								// Add the peripheral's offset register to FIFO stack  
  BSC1_FIFO = data;		  								// Add the data to FIFO stack
  BSC1_CONTROL = START_WRITE;  							// I2CEN, ST set to 1 (Enable I2C and Start Transfer)
  
  // Wait until done
  wait_i2c_done();
  
  BSC1_CONTROL |= BSC_C_CLEAR;							// Clear the FIFO slack.
 	
}

