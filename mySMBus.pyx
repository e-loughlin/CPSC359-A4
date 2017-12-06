cdef extern int fd
cdef extern unsigned int *bsc1
cdef extern unsigned int *gpio
cdef extern int init()
cdef extern void uninit()
cdef extern unsigned char read_byte(unsigned char address, unsigned char regAddress)
cdef extern void write_byte(unsigned char address, unsigned char regAddress, unsigned char data)

def open(int x):
	init()
	
def close(int x):
	uninit()
	
def read_byte_data(address, regAddress):
	x= read_byte(address, regAddress)
	return x
	
def write_byte_data(address, regAddress, data):
	write_byte(address, regAddress, data)
	return

