#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <nvToolsExt.h>
#include <string.h>
#include <assert.h>

#include <cuda.h>
#include <cuda_runtime.h>

#define MEMSIZE (0x1 << 22) //8*2M*4B=128MB

	__global__
void vertorADDGPU(float *x, float *y, float *z) {
	int idx = blockIdx.x * MEMSIZE + 32 * threadIdx.x;
	while(true){
		z[idx] = x[idx] + y[idx];
		z[idx + blockDim.x * 32] = x[idx + blockDim.x * 32] + y[idx + blockDim.x * 32];
		z[idx + 2 * blockDim.x * 32] = x[idx + 2 * blockDim.x * 32] + y[idx + 2 * blockDim.x * 32];
		z[idx + 3 * blockDim.x * 32] = x[idx + 3 * blockDim.x * 32] + y[idx + 3 * blockDim.x * 32];
		z[idx + 4 * blockDim.x * 32] = x[idx + 4 * blockDim.x * 32] + y[idx + 4 * blockDim.x * 32];
		z[idx + 5 * blockDim.x * 32] = x[idx + 5 * blockDim.x * 32] + y[idx + 5 * blockDim.x * 32];
		z[idx + 6 * blockDim.x * 32] = x[idx + 6 * blockDim.x * 32] + y[idx + 6 * blockDim.x * 32];
		z[idx + 7 * blockDim.x * 32] = x[idx + 7 * blockDim.x * 32] + y[idx + 7 * blockDim.x * 32];
		z[idx + 8 * blockDim.x * 32] = x[idx + 8 * blockDim.x * 32] + y[idx + 8 * blockDim.x * 32];
                z[idx + 9 * blockDim.x * 32] = x[idx + 9 * blockDim.x * 32] + y[idx + 9 * blockDim.x * 32];
                z[idx + 10 * blockDim.x * 32] = x[idx + 10 * blockDim.x * 32] + y[idx + 10 * blockDim.x * 32];
                z[idx + 11 * blockDim.x * 32] = x[idx + 11 * blockDim.x * 32] + y[idx + 11 * blockDim.x * 32];
                z[idx + 12 * blockDim.x * 32] = x[idx + 12 * blockDim.x * 32] + y[idx + 12 * blockDim.x * 32];
                z[idx + 13 * blockDim.x * 32] = x[idx + 13 * blockDim.x * 32] + y[idx + 13 * blockDim.x * 32];
		z[idx + 14 * blockDim.x * 32] = x[idx + 14 * blockDim.x * 32] + y[idx + 14 * blockDim.x * 32];
		z[idx + 15 * blockDim.x * 32] = x[idx + 15 * blockDim.x * 32] + y[idx + 15 * blockDim.x * 32];
		z[idx + 16 * blockDim.x * 32] = x[idx + 16 * blockDim.x * 32] + y[idx + 16 * blockDim.x * 32];
                z[idx + 17 * blockDim.x * 32] = x[idx + 17 * blockDim.x * 32] + y[idx + 17 * blockDim.x * 32];
                z[idx + 18 * blockDim.x * 32] = x[idx + 18 * blockDim.x * 32] + y[idx + 18 * blockDim.x * 32];
                z[idx + 19 * blockDim.x * 32] = x[idx + 19 * blockDim.x * 32] + y[idx + 19 * blockDim.x * 32];
                z[idx + 20 * blockDim.x * 32] = x[idx + 20 * blockDim.x * 32] + y[idx + 20 * blockDim.x * 32];
                z[idx + 21 * blockDim.x * 32] = x[idx + 21 * blockDim.x * 32] + y[idx + 21 * blockDim.x * 32];
                z[idx + 22 * blockDim.x * 32] = x[idx + 22 * blockDim.x * 32] + y[idx + 22 * blockDim.x * 32];
                z[idx + 23 * blockDim.x * 32] = x[idx + 23 * blockDim.x * 32] + y[idx + 23 * blockDim.x * 32];
                z[idx + 24 * blockDim.x * 32] = x[idx + 24 * blockDim.x * 32] + y[idx + 24 * blockDim.x * 32];
                z[idx + 25 * blockDim.x * 32] = x[idx + 25 * blockDim.x * 32] + y[idx + 25 * blockDim.x * 32];
                z[idx + 26 * blockDim.x * 32] = x[idx + 26 * blockDim.x * 32] + y[idx + 26 * blockDim.x * 32];
                z[idx + 27 * blockDim.x * 32] = x[idx + 27 * blockDim.x * 32] + y[idx + 27 * blockDim.x * 32];
                z[idx + 28 * blockDim.x * 32] = x[idx + 28 * blockDim.x * 32] + y[idx + 28 * blockDim.x * 32];
                z[idx + 29 * blockDim.x * 32] = x[idx + 29 * blockDim.x * 32] + y[idx + 29 * blockDim.x * 32];
		z[idx + 30 * blockDim.x * 32] = x[idx + 30 * blockDim.x * 32] + y[idx + 30 * blockDim.x * 32];
                z[idx + 31 * blockDim.x * 32] = x[idx + 31 * blockDim.x * 32] + y[idx + 31 * blockDim.x * 32];
		z[idx + 32 * blockDim.x * 32] = x[idx + 32 * blockDim.x * 32] + y[idx + 32 * blockDim.x * 32];
                z[idx + 33 * blockDim.x * 32] = x[idx + 33 * blockDim.x * 32] + y[idx + 33 * blockDim.x * 32];
                z[idx + 34 * blockDim.x * 32] = x[idx + 34 * blockDim.x * 32] + y[idx + 34 * blockDim.x * 32];
                z[idx + 35 * blockDim.x * 32] = x[idx + 35 * blockDim.x * 32] + y[idx + 35 * blockDim.x * 32];
                z[idx + 36 * blockDim.x * 32] = x[idx + 36 * blockDim.x * 32] + y[idx + 36 * blockDim.x * 32];
                z[idx + 37 * blockDim.x * 32] = x[idx + 37 * blockDim.x * 32] + y[idx + 37 * blockDim.x * 32];
                z[idx + 38 * blockDim.x * 32] = x[idx + 38 * blockDim.x * 32] + y[idx + 38 * blockDim.x * 32];
                z[idx + 39 * blockDim.x * 32] = x[idx + 39 * blockDim.x * 32] + y[idx + 39 * blockDim.x * 32];
                z[idx + 40 * blockDim.x * 32] = x[idx + 40 * blockDim.x * 32] + y[idx + 40 * blockDim.x * 32];
                z[idx + 41 * blockDim.x * 32] = x[idx + 41 * blockDim.x * 32] + y[idx + 41 * blockDim.x * 32];
                z[idx + 42 * blockDim.x * 32] = x[idx + 42 * blockDim.x * 32] + y[idx + 42 * blockDim.x * 32];
                z[idx + 43 * blockDim.x * 32] = x[idx + 43 * blockDim.x * 32] + y[idx + 43 * blockDim.x * 32];
                z[idx + 44 * blockDim.x * 32] = x[idx + 44 * blockDim.x * 32] + y[idx + 44 * blockDim.x * 32];
                z[idx + 45 * blockDim.x * 32] = x[idx + 45 * blockDim.x * 32] + y[idx + 45 * blockDim.x * 32];
                z[idx + 46 * blockDim.x * 32] = x[idx + 46 * blockDim.x * 32] + y[idx + 46 * blockDim.x * 32];
                z[idx + 47 * blockDim.x * 32] = x[idx + 47 * blockDim.x * 32] + y[idx + 47 * blockDim.x * 32];
                z[idx + 48 * blockDim.x * 32] = x[idx + 48 * blockDim.x * 32] + y[idx + 48 * blockDim.x * 32];
                z[idx + 49 * blockDim.x * 32] = x[idx + 49 * blockDim.x * 32] + y[idx + 49 * blockDim.x * 32];
                z[idx + 50 * blockDim.x * 32] = x[idx + 50 * blockDim.x * 32] + y[idx + 50 * blockDim.x * 32];
                z[idx + 51 * blockDim.x * 32] = x[idx + 51 * blockDim.x * 32] + y[idx + 51 * blockDim.x * 32];
                z[idx + 52 * blockDim.x * 32] = x[idx + 52 * blockDim.x * 32] + y[idx + 52 * blockDim.x * 32];
                z[idx + 53 * blockDim.x * 32] = x[idx + 53 * blockDim.x * 32] + y[idx + 53 * blockDim.x * 32];
                z[idx + 54 * blockDim.x * 32] = x[idx + 54 * blockDim.x * 32] + y[idx + 54 * blockDim.x * 32];
                z[idx + 55 * blockDim.x * 32] = x[idx + 55 * blockDim.x * 32] + y[idx + 55 * blockDim.x * 32];
                z[idx + 56 * blockDim.x * 32] = x[idx + 56 * blockDim.x * 32] + y[idx + 56 * blockDim.x * 32];
                z[idx + 57 * blockDim.x * 32] = x[idx + 57 * blockDim.x * 32] + y[idx + 57 * blockDim.x * 32];
                z[idx + 58 * blockDim.x * 32] = x[idx + 58 * blockDim.x * 32] + y[idx + 58 * blockDim.x * 32];
                z[idx + 59 * blockDim.x * 32] = x[idx + 59 * blockDim.x * 32] + y[idx + 59 * blockDim.x * 32];
                z[idx + 60 * blockDim.x * 32] = x[idx + 60 * blockDim.x * 32] + y[idx + 60 * blockDim.x * 32];
                z[idx + 61 * blockDim.x * 32] = x[idx + 61 * blockDim.x * 32] + y[idx + 61 * blockDim.x * 32];
		z[idx + 62 * blockDim.x * 32] = x[idx + 62 * blockDim.x * 32] + y[idx + 62 * blockDim.x * 32];
                z[idx + 63 * blockDim.x * 32] = x[idx + 63 * blockDim.x * 32] + y[idx + 63 * blockDim.x * 32];
		z[idx + 64 * blockDim.x * 32] = x[idx + 64 * blockDim.x * 32] + y[idx + 64 * blockDim.x * 32];
		z[idx + 65 * blockDim.x * 32] = x[idx + 65 * blockDim.x * 32] + y[idx + 65 * blockDim.x * 32];
		z[idx + 66 * blockDim.x * 32] = x[idx + 66 * blockDim.x * 32] + y[idx + 66 * blockDim.x * 32];
		z[idx + 67 * blockDim.x * 32] = x[idx + 67 * blockDim.x * 32] + y[idx + 67 * blockDim.x * 32];
		z[idx + 68 * blockDim.x * 32] = x[idx + 68 * blockDim.x * 32] + y[idx + 68 * blockDim.x * 32];
                z[idx + 69 * blockDim.x * 32] = x[idx + 69 * blockDim.x * 32] + y[idx + 69 * blockDim.x * 32];
                z[idx + 70 * blockDim.x * 32] = x[idx + 70 * blockDim.x * 32] + y[idx + 70 * blockDim.x * 32];
                z[idx + 71 * blockDim.x * 32] = x[idx + 71 * blockDim.x * 32] + y[idx + 71 * blockDim.x * 32];
                z[idx + 72 * blockDim.x * 32] = x[idx + 72 * blockDim.x * 32] + y[idx + 72 * blockDim.x * 32];
                z[idx + 73 * blockDim.x * 32] = x[idx + 73 * blockDim.x * 32] + y[idx + 73 * blockDim.x * 32];
		z[idx + 74 * blockDim.x * 32] = x[idx + 74 * blockDim.x * 32] + y[idx + 74 * blockDim.x * 32];
		z[idx + 75 * blockDim.x * 32] = x[idx + 75 * blockDim.x * 32] + y[idx + 75 * blockDim.x * 32];
		z[idx + 76 * blockDim.x * 32] = x[idx + 76 * blockDim.x * 32] + y[idx + 76 * blockDim.x * 32];
                z[idx + 77 * blockDim.x * 32] = x[idx + 77 * blockDim.x * 32] + y[idx + 77 * blockDim.x * 32];
                z[idx + 78 * blockDim.x * 32] = x[idx + 78 * blockDim.x * 32] + y[idx + 78 * blockDim.x * 32];
                z[idx + 79 * blockDim.x * 32] = x[idx + 79 * blockDim.x * 32] + y[idx + 79 * blockDim.x * 32];
                z[idx + 80 * blockDim.x * 32] = x[idx + 80 * blockDim.x * 32] + y[idx + 80 * blockDim.x * 32];
                z[idx + 81 * blockDim.x * 32] = x[idx + 81 * blockDim.x * 32] + y[idx + 81 * blockDim.x * 32];
                z[idx + 82 * blockDim.x * 32] = x[idx + 82 * blockDim.x * 32] + y[idx + 82 * blockDim.x * 32];
                z[idx + 83 * blockDim.x * 32] = x[idx + 83 * blockDim.x * 32] + y[idx + 83 * blockDim.x * 32];
                z[idx + 84 * blockDim.x * 32] = x[idx + 84 * blockDim.x * 32] + y[idx + 84 * blockDim.x * 32];
                z[idx + 85 * blockDim.x * 32] = x[idx + 85 * blockDim.x * 32] + y[idx + 85 * blockDim.x * 32];
                z[idx + 86 * blockDim.x * 32] = x[idx + 86 * blockDim.x * 32] + y[idx + 86 * blockDim.x * 32];
                z[idx + 87 * blockDim.x * 32] = x[idx + 87 * blockDim.x * 32] + y[idx + 87 * blockDim.x * 32];
                z[idx + 88 * blockDim.x * 32] = x[idx + 88 * blockDim.x * 32] + y[idx + 88 * blockDim.x * 32];
                z[idx + 89 * blockDim.x * 32] = x[idx + 89 * blockDim.x * 32] + y[idx + 89 * blockDim.x * 32];
		z[idx + 90 * blockDim.x * 32] = x[idx + 90 * blockDim.x * 32] + y[idx + 90 * blockDim.x * 32];
                z[idx + 91 * blockDim.x * 32] = x[idx + 91 * blockDim.x * 32] + y[idx + 91 * blockDim.x * 32];
		z[idx + 92 * blockDim.x * 32] = x[idx + 92 * blockDim.x * 32] + y[idx + 92 * blockDim.x * 32];
                z[idx + 93 * blockDim.x * 32] = x[idx + 93 * blockDim.x * 32] + y[idx + 93 * blockDim.x * 32];
                z[idx + 94 * blockDim.x * 32] = x[idx + 94 * blockDim.x * 32] + y[idx + 94 * blockDim.x * 32];
                z[idx + 95 * blockDim.x * 32] = x[idx + 95 * blockDim.x * 32] + y[idx + 95 * blockDim.x * 32];
                z[idx + 96 * blockDim.x * 32] = x[idx + 96 * blockDim.x * 32] + y[idx + 96 * blockDim.x * 32];
                z[idx + 97 * blockDim.x * 32] = x[idx + 97 * blockDim.x * 32] + y[idx + 97 * blockDim.x * 32];
                z[idx + 98 * blockDim.x * 32] = x[idx + 98 * blockDim.x * 32] + y[idx + 98 * blockDim.x * 32];
                z[idx + 99 * blockDim.x * 32] = x[idx + 99 * blockDim.x * 32] + y[idx + 99 * blockDim.x * 32];
                z[idx + 100 * blockDim.x * 32] = x[idx + 100 * blockDim.x * 32] + y[idx + 100 * blockDim.x * 32];
                z[idx + 101 * blockDim.x * 32] = x[idx + 101 * blockDim.x * 32] + y[idx + 101 * blockDim.x * 32];
                z[idx + 102 * blockDim.x * 32] = x[idx + 102 * blockDim.x * 32] + y[idx + 102 * blockDim.x * 32];
                z[idx + 103 * blockDim.x * 32] = x[idx + 103 * blockDim.x * 32] + y[idx + 103 * blockDim.x * 32];
                z[idx + 104 * blockDim.x * 32] = x[idx + 104 * blockDim.x * 32] + y[idx + 104 * blockDim.x * 32];
                z[idx + 105 * blockDim.x * 32] = x[idx + 105 * blockDim.x * 32] + y[idx + 105 * blockDim.x * 32];
                z[idx + 106 * blockDim.x * 32] = x[idx + 106 * blockDim.x * 32] + y[idx + 106 * blockDim.x * 32];
                z[idx + 107 * blockDim.x * 32] = x[idx + 107 * blockDim.x * 32] + y[idx + 107 * blockDim.x * 32];
                z[idx + 108 * blockDim.x * 32] = x[idx + 108 * blockDim.x * 32] + y[idx + 108 * blockDim.x * 32];
                z[idx + 109 * blockDim.x * 32] = x[idx + 109 * blockDim.x * 32] + y[idx + 109 * blockDim.x * 32];
                z[idx + 110 * blockDim.x * 32] = x[idx + 110 * blockDim.x * 32] + y[idx + 110 * blockDim.x * 32];
                z[idx + 111 * blockDim.x * 32] = x[idx + 111 * blockDim.x * 32] + y[idx + 111 * blockDim.x * 32];
                z[idx + 112 * blockDim.x * 32] = x[idx + 112 * blockDim.x * 32] + y[idx + 112 * blockDim.x * 32];
                z[idx + 113 * blockDim.x * 32] = x[idx + 113 * blockDim.x * 32] + y[idx + 113 * blockDim.x * 32];
                z[idx + 114 * blockDim.x * 32] = x[idx + 114 * blockDim.x * 32] + y[idx + 114 * blockDim.x * 32];
                z[idx + 115 * blockDim.x * 32] = x[idx + 115 * blockDim.x * 32] + y[idx + 115 * blockDim.x * 32];
                z[idx + 116 * blockDim.x * 32] = x[idx + 116 * blockDim.x * 32] + y[idx + 116 * blockDim.x * 32];
                z[idx + 117 * blockDim.x * 32] = x[idx + 117 * blockDim.x * 32] + y[idx + 117 * blockDim.x * 32];
                z[idx + 118 * blockDim.x * 32] = x[idx + 118 * blockDim.x * 32] + y[idx + 118 * blockDim.x * 32];
                z[idx + 119 * blockDim.x * 32] = x[idx + 119 * blockDim.x * 32] + y[idx + 119 * blockDim.x * 32];
                z[idx + 120 * blockDim.x * 32] = x[idx + 120 * blockDim.x * 32] + y[idx + 120 * blockDim.x * 32];
                z[idx + 121 * blockDim.x * 32] = x[idx + 121 * blockDim.x * 32] + y[idx + 121 * blockDim.x * 32];
		z[idx + 122 * blockDim.x * 32] = x[idx + 122 * blockDim.x * 32] + y[idx + 122 * blockDim.x * 32];
                z[idx + 123 * blockDim.x * 32] = x[idx + 123 * blockDim.x * 32] + y[idx + 123 * blockDim.x * 32];
                z[idx + 124 * blockDim.x * 32] = x[idx + 124 * blockDim.x * 32] + y[idx + 124 * blockDim.x * 32];
                z[idx + 125 * blockDim.x * 32] = x[idx + 125 * blockDim.x * 32] + y[idx + 125 * blockDim.x * 32];
                z[idx + 126 * blockDim.x * 32] = x[idx + 126 * blockDim.x * 32] + y[idx + 126 * blockDim.x * 32];
                z[idx + 127 * blockDim.x * 32] = x[idx + 127 * blockDim.x * 32] + y[idx + 127 * blockDim.x * 32];
	}
}

int main(int argc, char *argv[]) {
	int thread_num;
	float *x, *y, *z, *d_x, *d_y, *d_z;
	unsigned int flags = cudaHostAllocMapped;

	// Check the number of arguments
	if (argc != 2) {
		printf("please use with one argument\n");
		printf("The argument is thread num\n");
		return 1;
	}
	thread_num = atoi(argv[1]);

	// Dynamic memory allocation by using zero-copy memory (cache is off)
	cudaHostAlloc((void **)&x, 8 * MEMSIZE * sizeof(float), flags);
	cudaHostAlloc((void **)&y, 8 * MEMSIZE * sizeof(float), flags);
	cudaHostAlloc((void **)&z, 8 * MEMSIZE * sizeof(float), flags);

	// Initialize the two input arrays
	for (int i = 0; i < 8 * MEMSIZE; i++) {
        	x[i] = rand() / (float) RAND_MAX;
        	y[i] = rand() / (float) RAND_MAX;
	}

	cudaHostGetDevicePointer((void **)&d_x, (void *)x, 0);
	cudaHostGetDevicePointer((void **)&d_y, (void *)y, 0);
	cudaHostGetDevicePointer((void **)&d_z, (void *)z, 0);
	
	// Set the number of threads
	dim3 numBlocks(8);
	dim3 threadsPerBlock(thread_num);

	while(true){
		vertorADDGPU<<<numBlocks, threadsPerBlock>>>(d_x, d_y, d_z);
		cudaDeviceSynchronize();
	}

	cudaFree(d_x);
	cudaFree(d_y);
	cudaFree(d_z);
	return 1;
}
