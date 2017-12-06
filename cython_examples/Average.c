#include <stdio.h>

int averageCalc(int n);

int averageCalc(int n) {
    printf("This is my C code\n");
    int input = 0;
    int total = 0;
    printf("Please enter grades: \n");
    int j;
    for (j=0;j<n;j++) {
        scanf("%d",&input);
        total += input;
    }
    int result = total/n;
    printf("C code calculates average to be %d\n", result);
    return result;
}