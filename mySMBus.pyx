cdef extern int fd
cdef extern unsigned int *bsc1
cdef extern unsigned int *gpio
cdef extern int c_init()
cdef extern void c_uninit()
cdef extern unsigned char readByte(unsigned char address, unsigned char regAddress)
cdef extern void writeByte(unsigned char address, unsigned char regAddress, unsigned char data)

def open():
	c_init()
	
def close(int x):
	c_uninit()
	
def read_byte_data(address, regAddress):
	return readByte(address, regAddress)
	
def write_byte_data(address, regAddress, data):
	writeByte(address, regAddress, data)
	return

