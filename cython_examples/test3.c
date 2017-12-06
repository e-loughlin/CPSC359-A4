#include <stdio.h>

int a = 1, b = 2, c = 3;
int e[] = {4, 5, 6};
int *f[] = {e, &a, &b, &c};
int **g = &f[1];


main()
{
	printf("%d\n", *e);
	printf("%d\n", **g);
	printf("%d\n", **(f+2));
	printf("%d\n", *(e+2));
	printf("%d\n", *g[2]);
	printf("%d\n", *(f[0] + 1));
}
