#include<iostream>
#include<fstream>
#include<complex>
#include<mpi.h>


using namespace std;
typedef double ComplexCoeff;  // remember to change MY_MPI_COMPLEX define
typedef complex<ComplexCoeff> Complex;
#define MY_MPI_COMPLEX MPI::DOUBLE_COMPLEX

const Complex I(0, 1);
const unsigned int n = 4096;  //Number of samples


unsigned int bitReverse(unsigned int);
void readInput(string);
void writeOutput(string, Complex *);
void printComplex(Complex);
void printComplexArray(Complex *, unsigned int);


Complex input[n];


void fft(Complex * y, Complex * output, Complex * receive){

    int id, processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);
    int chunk = n / processes;

    // PART 1 // Each process get its part of the input (indexes are bitwise reversed)
    for(int i = 0; i < chunk; i++){
        y[i] = input[bitReverse(id * chunk + i)];
    }

    // PART 2 // All needed data in single process
    int data_involved = 2;

    for(int j = 0; j < log2(n) - log2(processes); j++){

        for(int i = 0; i < chunk; i += data_involved){
            int d = 2 << j;
            Complex w = Complex (1, 0);
            Complex wd = exp(I * Complex (-2 * M_PI / d, 0));

            for(int k = i; k < i + data_involved / 2; k++){
                Complex a = y[k];
                Complex b = y[k + data_involved / 2];

                y[k] = a + w * b;
                y[k + data_involved / 2] = a - w * b;

                w = w * wd;
            }
        }

        data_involved *= 2;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    int distance = 1;
    int groups = processes / 2;
    int group_size = 2;

    // PART 3 // Communication between processes needed
    for(int j = log2(n) - log2(processes); j < log2(n); j++){

        int receive_id;
        int send_id;
        int middle = group_size * (id / group_size) + group_size / 2;

        if(id < middle){
            send_id = id;
            receive_id = id + distance;
        } else{
            send_id = id;
            receive_id = id - distance;
        }

        MPI_Sendrecv(y, chunk, MY_MPI_COMPLEX, receive_id, 0, receive, chunk,
                MY_MPI_COMPLEX, receive_id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int d = 2 << j;
        Complex w = Complex (1, 0);
        Complex wd = exp(I * Complex (-2 * M_PI / d, 0));
        int which_one = id % (group_size/2);

	w *= pow(wd, chunk * which_one);

        if(id < middle) {
            for (int k = 0; k < chunk; k++) {
                Complex a = y[k];
                Complex b = receive[k];

                y[k] = a + w * b;
                w = w * wd;
            }
        } else{
            for (int k = 0; k < chunk; k++) {
                Complex a = receive[k];
                Complex b = y[k];

                y[k] = a - w * b;
                w = w * wd;
            }
        }

        distance *= 2;
        groups /= 2;
        group_size *= 2;
    }

    MPI_Gather(y, chunk, MY_MPI_COMPLEX, output + id * chunk, chunk, MY_MPI_COMPLEX, 0, MPI_COMM_WORLD);
}


// MAIN
#ifndef TEST
int main(int argc, char * argv[]){
    int id;
    int processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    int chunk = n / processes;
    Complex * y = new Complex[chunk];
    Complex * output = new Complex[n];
    Complex * receive = new Complex[chunk];

    string input_file = argv[1];
    string output_file = argv[2];

    // INPUT
    if (id == 0) {
        readInput(input_file);
    }
    MPI_Bcast(input, n, MY_MPI_COMPLEX, 0, MPI_COMM_WORLD);

    fft(y, output, receive);

    // OUTPUT
    if (id == 0) {
        writeOutput(output_file, output);
    }

    MPI_Finalize();
    delete[] y;
    delete[] output;
    delete[] receive;

    return 0;
}
#endif

#ifdef TEST
int main(int argc, char * argv[]){
    int id;
    int processes;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &processes);

    int chunk = n / processes;
    Complex * y = new Complex[chunk];
    Complex * output = new Complex[n];
    Complex * receive = new Complex[chunk];

    string times_file = "data/mpi_times_n" + to_string(processes) + ".txt";

    if(id == 0){
        ofstream file;
        file.open(times_file, ofstream::out | ofstream::trunc);
        file.close();
    }

    for(int num = 0; num < 100; num++) {
        string input_file = "data/inputs/input_" + to_string(num) + ".txt";
        string output_file = "data/actual_outputs/actual_output_" + to_string(num) + ".txt";

        // INPUT
        if (id == 0) {
            readInput(input_file);
        }
        MPI_Bcast(input, n, MY_MPI_COMPLEX, 0, MPI_COMM_WORLD);

        // Time test
        double start, end;
        MPI_Barrier(MPI_COMM_WORLD);
        start = MPI_Wtime();

        fft(y, output, receive);

        end = MPI_Wtime();
        double runtime = end - start;

        // OUTPUT
        if (id == 0) {
            writeOutput(output_file, output);
            ofstream file;
            file.open(times_file, ios_base::app);
            file << "Runtime = " << runtime << endl;
            file.close();
        }
    }

    MPI_Finalize();
    delete[] y;
    delete[] output;
    delete[] receive;

    return 0;
}
#endif

// FUNCTIONS RELATED TO ITERATIVE FFT
unsigned int bitReverse(unsigned int numToReverse){
    static unsigned int bits = log2(n);
    unsigned int reverseNum = 0;

    for(unsigned int i = 0; i <= bits; i++){
        unsigned int temp = (numToReverse & (1 << i));
        if(temp)
            reverseNum |= (1 << ((bits - 1) - i));
    }

    return reverseNum;
}


void readInput(string input_file){
    ifstream file(input_file);
    string line = "";
    int i = 0;

    while (getline(file, line) && i < n) {
        string delimiter = ", ";
        int pos = line.find(delimiter);

        string string_real = line.substr(0, pos);
        string string_imag = line.substr(pos + delimiter.length(), line.length());

        ComplexCoeff real = atof(string_real.c_str());
        ComplexCoeff imag = atof(string_imag.c_str());

        input[i++] = Complex(real, imag);
    }

    file.close();
}


void writeOutput(string output_file, Complex * output){
    ofstream file;
    file.open(output_file);

    for (int i = 0; i < n; i++) {
        file << output[i].real() << ", " << output[i].imag() << endl;
    }

    file.close();
}


void printComplex(Complex num){
    cout << num.real() << "+" << num.imag() << "i";
}


void printComplexArray(Complex * arr, unsigned int size){
    cout << "[";
    for(int i = 0; i < size; i++){
        printComplex(arr[i]);
        cout << ", ";
    }
    cout << "]" << endl;
}
