#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
	int i, j, k;
	// Dynamic memory allocation
	double * m1 = (double *) malloc(sizeof(double)*1024*1024*16);//16*8MB
	memset(m1, 0, sizeof(double)*1024*1024*16);
	double * m2 = (double *) malloc(sizeof(double)*1024*1024*16);//16*8MB
	memset(m2, 1.2, sizeof(double)*1024*1024*16);

	while(true){
		for(i = 0; i < 1024*1024*16; i+=16) {
			m1[i] = m2[i] + m2[i+8];
		}
	}

	return 0;
}

