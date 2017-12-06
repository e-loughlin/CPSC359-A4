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
 * 2) CPSC 359 Course Material, special thanks to Andrew Groene and Dr. Boyd.
 */

// Macros

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sched.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

// GPIO Pin Definitions
#define 	SDA1		2
#define		SCL1		3

// Definitions of memory addresses for GPIO and BSC1
#define BCM2708_PERI_BASE		0x3F000000
#define GPIO_BASE				(BCM2708_PERI_BASE + 0x200000) // GPIO Controller
#define BSC1_BASE				(BCM2708_PERI_BASE + 0x804000) // I2C Controller

#define BLOCK_SIZE				(4*1024)


int fd;
volatile unsigned int *bsc1;
volatile unsigned int *gpio;


#define GPIO_SET 			*(gpio[0] + 0b0111)   // Sets bits which are 1, ignores bits which are 0.
#define GPIO_CLR			*(gpio[0] + 0b1010)   // Clears bits which are 1, ignores bits which are 0.

#define GPIO_READ(g)		*(gpio[0] + 0b1101) &= (1<<(g)) 


// I2C Addresses:

#define BSC1_CONTROL		bsc1[0]				         // Address for I2C Control: 
                                                        // Used to enable interrupts, clear the FIFO,
                                                        // define a read or write operation, and start
                                                        // a transfer.
                                                        
#define BSC1_STATUS         bsc1[1]				        // Status register is used to record activity
														// status, errors and interrupt requests.
														
#define BSC1_DLEN			bsc1[2]						// Data length register defines the number of 
														// bytes of data to transmit or receive in the 
														// I2C transfer. 

#define BSC1_SLAVEADDR		bsc1[3]						// Slave Address register specifies the slave
														// address and cycle type. (Slave is defined 
														// as the peripheral device)
														
#define BSC1_FIFO			bsc1[4]						// FIFO (First In First Out) Register.
														// Read cycles acces data received from the bus.
														// Writes to the register adds to stack.
														// Reads from register remove the next item.
														
// I2C Macros for setting bits:
//(Note: Not all functions are defined here. Just what is required for the ADXL345 Accelerometer...
// ... for details of every function, refer to BCM2835 ARM Peripherals manual)

// Control Functions:
#define BSC_C_I2CEN		(1 << 15)							// I2C Enable. 0 = BSC controller disabled. 1 = enabled
#define BSC_C_ST		(1 << 7)							// ST = Start Transfer. 0 = No Action. 1 = Start a new transfer
#define BSC_C_CLEAR		(1 << 4)							// CLEAR FIFO. 00 = No Action. 01, 10, or 11 = Clear FIFO
#define BSC_C_READ		1									// READ Transfer. 0 = Write Packet Transfer. 1 = Read Packet Transfer

#define START_READ		BSC_C_I2CEN|BSC_C_ST|BSC_C_CLEAR|BSC_C_READ		// Sets I2C up for reading data.
#define START_WRITE		BSC_C_I2CEN|BSC_C_ST							// Sets I2C up for writing data.

// Status Register Functions:
#define BSC_S_CLKT		(1 << 9)							// CLKT Clock Stretch Timeout
															// 0 = No errors detected. 1 = Slave has held the SCL signal
															// low (clock stretching) for too long. Cleared by writing 1.
														
#define BSC_S_ERR		(1 << 8)							// ERR ACK Error
															// 0 = No errors detected. 1 = Slave has not acknowledged its
															// address. Cleared by writing 1.
														
#define BSC_S_RXF		(1 << 7)							// RXF - FIFO Full
															// 0 = FIFO not full. 1 = FIFO is Full

#define BSC_S_TXE		(1 << 6)							// TXE - FIFO Empty
															// 0 = FIFO not empty. 1 = FIFO is full

#define BSC_S_RXD		(1 << 5)							// RXD - FIFO contains Data
															// 0 = FIFO is empty. 1 = FIFO contains at least 1 byte

#define BSC_S_TXD		(1 << 4)							// TXD - FIFO can accept Data
															// 0 = FIFO is full. The FIFO cannot accept more data.

#define BSC_S_RXR		(1 << 3)							// RXR - FIFO needs Reading (full)
															// 0 = FIFO is less than full and a read is underway.

#define BSC_S_TXW		(1 << 2)							// TXW - FIFO needs Writing (full)
															// 0 = FIFO is at least full and a write is underway.

#define BSC_S_DONE		(1 << 1)							// DONE - Transfer Done. (1 = transfer complete)
															// Cleared by writing 1 to the field.

#define BSC_S_TA		1									// TA = Transfer Active. (0 = not active, 1 = active)


#define CLEAR_STATUS	BSC_S_CLKT|BSC_S_ERR|BSC_S_DONE		// Clears the status flags


// Wait I2C Done: Function that waits for the I2C transaction to complete.
void wait_i2c_done() {
	int timeout = 50;
	while((!((BSC1_STATUS) & BSC_S_DONE)) && --timeout) {
		usleep(1000);
	}
	
	if(timeout == 0)
		printf("wait_i2c_done() timeout. Something ain't right, fool.\n");
}


// Sets GPIO2 and GPIO3 pins for initialization, opens fd, and maps memory.
int init()
{
	if ((fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("Failed to open /dev/mem. Try again with sudo.\n");
		return -1;
	}
	
	gpio = (unsigned int *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE,
	MAP_SHARED, fd, GPIO_BASE);
	
	if (gpio == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	
	printf( "mmap successful: %08x\n", GPIO_BASE);
	
	bsc1 = (unsigned int *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE,
	MAP_SHARED, fd, BSC1_BASE);
	
	if (gpio == MAP_FAILED) {
		perror("mmap");
		return -1;
	}
	
	printf("mmap successful: %08x\n", GPIO_BASE);

	gpio[0] = gpio[0] & ~(0b111 << (SDA1 * 3));    // Clear GPIO2
	gpio[0] = gpio[0] |  (0b100 << (SDA1 * 3));    // Set GPIO2 to ALT0
	gpio[0] = gpio[0] & ~(0b111 << (SCL1 * 3));    // Clear GPIO3
	gpio[0] = gpio[0] |  (0b100 << (SCL1 * 3));    // Set GPIO3 to ALT0
}

// Unmaps memory and closes the fd.
void uninit()
{
munmap( (void*)bsc1, BLOCK_SIZE);
munmap( (void*)gpio, BLOCK_SIZE);
close(fd);
}


// Function for reading byte data

unsigned char read_byte(unsigned char address, unsigned char regAddress) 
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


void write_byte(unsigned char address, unsigned char regAddress, unsigned char data)
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

