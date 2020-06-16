
## Fast Fourier Transform

Fast Fourier Transform parallel algorithm in 2 technologies as a part of Parallel Systems lab classes. 

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

