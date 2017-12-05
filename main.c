#include "smbus.h"
#include <stdio.h>

int main()
{

	
	int test;

	test = read_byte_data(0x53, 0x00);
	printf("0x%02x\n", test);

	return 0;	

}
