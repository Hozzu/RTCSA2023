#include <stdlib.h>
#include <chrono>
#include <cstring>
#include <thread>

#define TOTALTIME 2 //min

using namespace std;

int main(int argc, char* argv[]) 
{
	// Check the number of arguments
        if (argc != 3) {
                printf("please use with two argument\n");
                printf("The argument is period(ms) and memory size(MB)\n");
                return 1;
        }
	int period = atoi(argv[1]);
	int memSize = atoi(argv[2]) * (1 << 20);

	double * m = (double *)malloc(sizeof(double) * memSize);
	memset(m, 9.2, sizeof(double) * memSize);

	__asm__ __volatile__("fmov d0, 1.0");
	__asm__ __volatile__("fmov d1, 1.0");
	__asm__ __volatile__("fmov d2, 1.0");
	__asm__ __volatile__("fmov d3, 1.0");
	__asm__ __volatile__("fmov d4, 1.0");
	__asm__ __volatile__("fmov d5, 1.0");
	__asm__ __volatile__("fmov d6, 1.0");
	__asm__ __volatile__("fmov d7, 1.0");
	__asm__ __volatile__("fmov d8, 1.0");
	__asm__ __volatile__("fmov d9, 1.0");
	__asm__ __volatile__("fmov d10, 1.0");
	__asm__ __volatile__("fmov d11, 1.0");
	__asm__ __volatile__("fmov d12, 1.0");
	__asm__ __volatile__("fmov d13, 1.0");
	__asm__ __volatile__("fmov d14, 1.0");
	__asm__ __volatile__("fmov d15, 1.0");

	int success = 0, fail = 0;
	for(int k = 0; k < TOTALTIME * 60000 / period; k++){
		auto when_started = chrono::steady_clock::now();
		auto target_time = when_started + chrono::milliseconds(period);

		for(int l = 0; l < period / 10; l++){
			for(int i = 0; i < memSize; i+=128) {
				__asm__ __volatile__("fadd d0, d0, d0");
				__asm__ __volatile__("fadd d1, d1, d1");
				__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i]));

				__asm__ __volatile__("fadd d2, d2, d2");
				__asm__ __volatile__("fadd d3, d3, d3");
				__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+16]));

				__asm__ __volatile__("fadd d4, d4, d4");
				__asm__ __volatile__("fadd d5, d5, d5");
				__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+32]));
			
				__asm__ __volatile__("fadd d6, d6, d6");
                        	__asm__ __volatile__("fadd d7, d7, d7");
                        	__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+48]));

				__asm__ __volatile__("fadd d8, d8, d8");
                        	__asm__ __volatile__("fadd d9, d9, d9");
                        	__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+64]));

                        	__asm__ __volatile__("fadd d10, d10, d10");
                        	__asm__ __volatile__("fadd d11, d11, d11");
                        	__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+80]));

                        	__asm__ __volatile__("fadd d12, d12, d12");
                        	__asm__ __volatile__("fadd d13, d13, d13");
                        	__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+96]));

                        	__asm__ __volatile__("fadd d14, d14, d14");
                        	__asm__ __volatile__("fadd d15, d15, d15");
                        	__asm__ __volatile__("ldr x15, [%0]"::"rm" (&m[i+112]));
			}
		}

		auto when_ended = chrono::steady_clock::now();
		if(when_ended > target_time) fail++;
		else success++;

		this_thread::sleep_until(target_time);
	}

	printf("in-time %d, out-of-time %d\n", success, fail);

	return 0;
}

