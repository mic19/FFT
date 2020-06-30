

## Fast Fourier Transform

Fast Fourier Transform parallel algorithm in 3 technologies as a part of Parallel Systems lab classes. 

**MPI - OpenMPI in C++**  
Image below shows runtime for different number of processes on 8-core processor. When number of processes exceeds the number of cores, runtime increases. Serial algorithm runtime (main_serial.cpp) for comparison. 
<img src="/figure_mpi.png" border="0" />

**Unified Parallel C**  
Diagram below shows how shared memory is divided in UPC implementation of the algorithm. 
<img src="/figure_upc.png" border="0" />  
Given series - a<sub>0</sub>, a<sub>1</sub>, a<sub>2</sub>, ... , a<sub>7</sub> is an input after bitwise reversing of its indexes. This usage of shared memory is the outcome of following initialization:  

    typedef shared [] ComplexCoeff * Sdptr;
    shared Sdptr all_data_real[THREADS];

and

    unsigned int chunk = N / THREADS;
    all_data_real[MYTHREAD] = upc_alloc(chunk * sizeof(ComplexCoeff));
    
The same happens for imaginary part of complex numbers. 

**STL**  
FFT algorithm is split into separate tasks represented by *fftTask* function. Each task is executed in a single thread. These threads are stored in *thread_array* in order to be joined with the main thread after algorithm is done. 

    for(int i = 0; i < threads - 1; ++i){  
	    threads_array.emplace_back(fftTask, std::ref(arr), threads, i);  
	}
	fftTask(std::ref(arr), threads, threads - 1);  

Below it is shown how these threads are joined.

    for(std::thread & t : threads_array){  
    if(t.joinable())  
        t.join();  
	}  


