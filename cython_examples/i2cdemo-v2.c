#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

#define	GPIO_BASE		0x3f200000
#define	BSC1_BASE		0x3f804000
#define	BLOCK_SIZE		4096
#define INP_GPIO(g) 	*(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) 	*(gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3))



#define I2C_CONTROL		0  // Control (Enable interrupts, clear the FIFO, define READ/WRITE, and start a transfer)
#define I2C_STATUS 		1  // Flags (9 = CLKT (Clock Stretch Timeout), 8 = ERR (error flag), 1 = DONE (Transfer Done)
#define I2C_DLEN		2  // Data Length (Defines the data length of bytes to transmit or receive from the I2C transfer)
#define I2C_SLAVEADDR   3  // Slave Address (Address of peripheral device)
#define I2C_FIFO		4  // Data of the FIFO stack


// BSC1 pins
#define         SDA1              2		//SDA1 = GPIO 2
#define         SCL1              3		//SCL1 = GPIO 3

volatile unsigned int *bsc1;		// Base address of BSC 
unsigned int *gpio;				// Base address of GPIO stuff
int fd;								// File Descriptor


void write_byte_data(unsigned char address, unsigned char regAddress, unsigned char data)
{
  bsc1[I2C_SLAVEADDR] = address;  						// Set peripheral address
  bsc1[I2C_DLEN] = 1;        			 				// Set length of data to be transmitted to 1 byte
  bsc1[I2C_FIFO] = regAddress;  						// Add the peripheral's offset register to FIFO stack
  bsc1[I2C_FIFO] = data;     			 				// Add "data" byte to FIFO stack
  bsc1[I2C_STATUS] = (1 << 9) | (1 << 8) | (1 << 1); 	// Clear CLKT, ERR, DONE flags
  bsc1[I2C_CONTROL] = (1 << 15) | (1 << 7);  			// I2CEN, ST

  // Wait until done:
  while( !(bsc1[I2C_STATUS] & 0x02) ) {   				// 0x02 masks DONE bit
    usleep(1000);
  }
  
  
  	
}

unsigned char read_byte_data(unsigned char address, unsigned char regAddress) 
{
  
  bsc1[I2C_SLAVEADDR] = address;  						// Set peripheral address
  bsc1[I2C_DLEN] = 1;        			 				// Set length of data to be transmitted to 1 byte
  bsc1[I2C_FIFO] = regAddress;  						// Add the peripheral's offset register to FIFO stack
  bsc1[I2C_STATUS] = (1 << 9) | (1 << 8) | (1 << 1); 	// Clear CLKT, ERR, DONE flags
  bsc1[I2C_CONTROL] = (1 << 15) | (1 << 7);  			// I2CEN, ST

  // Wait until done:
  while( !(bsc1[I2C_STATUS] & 0x02) ) {   				// 0x02 masks DONE bit
    usleep(1000);
  }
  
  // Get the response from the register
  bsc1[I2C_DLEN] = 1;
  bsc1[I2C_STATUS] = (1 << 9) | (1 << 8) | (1 << 1);  // clear CLKT, ERR, DONE
  bsc1[I2C_CONTROL] = (1 << 15) | (1 << 7) | (1 << 4) | 1; // I2CEN, ST, CLEAR, READ

  // Wait until done
  while( !(bsc1[I2C_STATUS] & 0x02) ) {   // 0x02 masks DONE bit
    usleep(1000);
  }

  unsigned int data = bsc1[I2C_FIFO];
  return data;
	
}

int init()
{
	
// Open /dev/mem
  if ((fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
    printf("Failed to open /dev/mem, try checking permissions.\n");
    return -1;
  }

    gpio = (unsigned int *)mmap(
    NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,
    fd, GPIO_BASE );

  if (gpio == MAP_FAILED) {
        perror("mmap");
        return -1;
  }

  printf( "mmap successful: %08x\n", GPIO_BASE );

    bsc1 = (volatile unsigned int *)mmap(
    NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,
    fd, BSC1_BASE );

  if (gpio == MAP_FAILED) {
        perror("mmap");
        return -1;
  }

  printf( "mmap successful: %08x\n", BSC1_BASE );


  // Define gpio0/1 as sda0/scl0 (alt 0)
  gpio[0] = gpio[0] & ~(0b111 << (SDA1 * 3));  // clear gpio2
  gpio[0] = gpio[0] |  (0b100 << (SDA1 * 3));  // alt0 in gpio2
  gpio[0] = gpio[0] & ~(0b111 << (SCL1 * 3));  // clear gpio3
  gpio[0] = gpio[0] |  (0b100 << (SCL1 * 3));  // alt0 in gpio3
  
  

}


void uninit() {
  munmap( (void*)bsc1, BLOCK_SIZE);
  munmap( (void*)gpio, BLOCK_SIZE);
  close(fd);
}


int main()
{

  init();

  // Test the acceleromater 
  // read the power state value
  char powerSave = read_byte_data(0x53, 0x2D);
  printf("0x%02x\n", powerSave);
  powerSave = powerSave | (0b1000);
  
  printf("0x%02x\n", powerSave);
  write_byte_data(0x53, 0x2D, powerSave);
  
  char powerSave2 = read_byte_data(0x53, 0x2D);
  printf("0x%02x\n", powerSave2);
  
  
  
  
  //write_byte_data(0x53,0x00,0x00);
  
  int devID;
  devID = read_byte_data(0x53,0x00);
  
  printf("0x%02x\n",devID);
  
  int devID2;
  devID = read_byte_data(0x53,0x00);
  
  printf("0x%02x\n",devID2);
  
  
  uninit();
}
