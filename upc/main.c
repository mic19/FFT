#include<stdio.h>
#include<complex.h>
#include<upc.h>
#include <sys/time.h>

#define N 4096

typedef double ComplexCoeff;
typedef double complex Complex;

unsigned int bitReverse(unsigned int);
double wctime();

typedef shared [] ComplexCoeff * Sdptr;
shared Sdptr all_data_real[THREADS];
shared Sdptr all_data_imag[THREADS];


int main(int argc, char *argv[]){
	
	unsigned int chunk = N / THREADS;
	
	all_data_real[MYTHREAD] = upc_alloc(chunk * sizeof(ComplexCoeff));
	all_data_imag[MYTHREAD] = upc_alloc(chunk * sizeof(ComplexCoeff));
	
	/*
	if(MYTHREAD == 0){
		for(int i = 0; i < chunk; ++i){
			int temp = upc_threadof(all_data_real[MYTHREAD] + i);
			printf("%d\n", temp);
		}
	}
	*/
	
	upc_barrier;
	
	if(MYTHREAD == 0){
		FILE * fp;
		char line[128];
		size_t length = 0;
		ssize_t read;
		
		Complex input[N];
		int counter = 0;
		
        char * input_filename = argv[1];
		fp = fopen(input_filename, "r");
		if (fp == NULL){
			exit(EXIT_FAILURE);
		}
		
		while ((fgets(line, sizeof(line), fp)) != NULL && counter < N) {
			
			char * pch = strtok(line, ",\n");
			char * real_str = pch;
			char * imag_str = strtok(NULL, ",");
			
			char * eptr;
			double real = strtod(real_str, &eptr);
			double imag = strtod(imag_str, &eptr);
			
			input[bitReverse(counter)] = real + I * imag;
			++counter;
		}
		fclose(fp);
		
		counter = 0;
		for(int thread = 0; thread < THREADS; ++thread){
			for(int i = 0; i < chunk; ++i){
				
				Complex temp = input[counter++];
				all_data_real[thread][i] = creal(temp);
				all_data_imag[thread][i] = cimag(temp);
			}
		}	
	}
	
	upc_barrier;
	double start = wctime();
	
	// PART 2 // PROCESS DATA IN SINGLE THREAD
	int data_involved = 2;
	
	for(int j = 0; j < log2(N) - log2(THREADS); ++j){
		for(int i = 0; i < chunk; i += data_involved){
			
			int d = 2 << j;
			Complex w = 1.0;
			Complex wd = cos(-2 * M_PI / d) + I * sin(-2 * M_PI / d);
	
			for(int k = i; k < i + data_involved / 2; ++k){
				
				int index = k + data_involved / 2;
				Complex a = all_data_real[MYTHREAD][k] + I * all_data_imag[MYTHREAD][k];
				Complex b = all_data_real[MYTHREAD][index] + I * all_data_imag[MYTHREAD][index];
				
				Complex new_a = a + w * b;
				Complex new_b = a - w * b;
				
				all_data_real[MYTHREAD][k] = creal(new_a);
				all_data_imag[MYTHREAD][k] = cimag(new_a);
				
				all_data_real[MYTHREAD][index] = creal(new_b);
				all_data_imag[MYTHREAD][index] = cimag(new_b);

                w = w * wd;
			}
		}
		data_involved *= 2;
	}
	
	upc_barrier;
	// PART 3 //
	int distance = 1;
	int group_size = 2;
	
	Complex * left_new_data = (Complex *) malloc(chunk / 2 * sizeof(Complex));
	Complex * right_new_data = (Complex *) malloc(chunk / 2 * sizeof(Complex));
	
	for(int j = log2(N) - log2(THREADS); j < log2(N); ++j){
		
		int receive_id;
        int send_id;
        int middle = group_size * (MYTHREAD / group_size) + group_size / 2;
        int iteration_start = 0;

        if(MYTHREAD < middle){
            send_id = MYTHREAD;
            receive_id = MYTHREAD + distance;
            
        } else{
            send_id = MYTHREAD;
            receive_id = MYTHREAD - distance;
            iteration_start = chunk / 2;
        }
			
		int d = 2 << j;
		Complex w = 1.0;
		Complex wd = cos(-2 * M_PI / d) + I * sin(-2 * M_PI / d);
		int which_one = MYTHREAD % (group_size / 2);
		
		if(MYTHREAD >= middle){
			//for(int i = 0; i < chunk / 2; ++i)
			//	w *= wd;
			w *= cpow(wd, chunk / 2);
		}
		
		w *= cpow(wd, chunk * which_one);
	
		upc_barrier;
		for(int k = iteration_start; k < iteration_start + chunk / 2; ++k){
			int index = k;
			Complex a = all_data_real[MYTHREAD][k] + I * all_data_imag[MYTHREAD][k];
			Complex b = all_data_real[receive_id][index] + I * all_data_imag[receive_id][index];
			
			if(MYTHREAD >= middle){
				Complex temp = a;
				a = b;
				b = temp;
			}
			
			Complex new_a = a + w * b;
			Complex new_b = a - w * b;
			
			left_new_data[k - iteration_start] = new_a;
			right_new_data[k - iteration_start] = new_b;

            w = w * wd;
		}
		upc_barrier;
		
		// Change shared data
		if(MYTHREAD >= middle){
			Complex * temp = left_new_data;
			left_new_data = right_new_data;
			right_new_data = temp;
		}
		
		for(int i = 0; i < chunk / 2; ++i){
			all_data_real[MYTHREAD][iteration_start + i] = creal(left_new_data[i]);
			all_data_imag[MYTHREAD][iteration_start + i] = cimag(left_new_data[i]);
			
			all_data_real[receive_id][iteration_start + i] = creal(right_new_data[i]);
			all_data_imag[receive_id][iteration_start + i] = cimag(right_new_data[i]);
		}
		
		data_involved *= 2;
		distance *= 2;
		group_size *= 2;
		
	}
	
	// Output	
	upc_barrier;
	double end = wctime();
	
	if(MYTHREAD == 0){
		printf("Time: %lf\n", end - start);
		
		FILE * fp;
		char * output_filename = argv[2];
		fp = fopen(output_filename, "w");
		if (fp == NULL){
			exit(EXIT_FAILURE);
		}
		
		for(int thread = 0; thread < THREADS; ++thread){
			for(int i = 0; i < chunk; ++i){
				Complex temp = all_data_real[thread][i] + I * all_data_imag[thread][i];
				fprintf(fp, "%.2lf + %.2lfi\n", creal(temp), cimag(temp));
			}
		}
		fclose(fp);
	}
	
	upc_barrier;
		
	free(left_new_data);
	free(right_new_data);
	upc_free(all_data_real[MYTHREAD]);
	upc_free(all_data_imag[MYTHREAD]);
	
	return 0;
}


unsigned int bitReverse(unsigned int num_to_reverse){
    unsigned int bits = log2(N);
    unsigned int reverse_num = 0;

    for(unsigned int i = 0; i <= bits; i++){
        unsigned int temp = (num_to_reverse & (1 << i));
        if(temp)
            reverse_num |= (1 << ((bits - 1) - i));
    }

    return reverse_num;
}

double wctime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec + 1E-6 * tv.tv_usec);
}

