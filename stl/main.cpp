#include <iostream>
#include <complex>
#include <thread>
#include <memory>
#include <vector>
#include <fstream>
#include <chrono>
#include "Barrier.h"


typedef double ComplexCoeff;
typedef std::complex<ComplexCoeff> Complex;

unsigned int bitReverse(unsigned int numToReverse, unsigned int n);
std::vector<Complex> readInput(std::string input_file, int n);
void writeOutput(std::string output_file, std::vector<Complex> output, int n);


std::shared_ptr<Barrier> barrier1(nullptr);
std::shared_ptr<Barrier> barrier2(nullptr);
std::shared_ptr<Barrier> barrier3(nullptr);


void fftTask(std::vector<Complex>& arr, unsigned int threads, int id){
    const Complex I(0, 1);

    // PART 2 - ALL NEEDED DATA IN SINGLE PROCESS
    unsigned int n = arr.size();
    unsigned int chunk = n / threads;
    int data_involved = 2;
    const int left_index = id * chunk;

    for(int j = 0; j < log2(n) - log2(threads); ++j) {
        for (int i = 0; i < chunk; i += data_involved) {

            int d = 2 << j;
            Complex w(1, 0);
            Complex wd = exp(I * Complex(-2 * M_PI / d, 0));

            for(int k = i; k < i + data_involved / 2; ++k){
                Complex a = arr[left_index + k];
                Complex b = arr[left_index + k + data_involved / 2];

                arr[left_index + k] = a + w * b;
                arr[left_index + k + data_involved / 2] = a - w * b;

                w = w * wd;
            }

        }

        data_involved *= 2;
    }

    int distance = 1;
    int groups = threads / 2;
    int group_size = 2;

    // PART 3 - Exchage data between threads
    for(int j = log2(n) - log2(threads); j < log2(n); j++){

        int master_id;
        int middle = group_size * (id / group_size) + group_size / 2;
        int fellow_left_index;

        if(id < middle){
            master_id = id;
            fellow_left_index = (id + distance) * chunk;
        } else{
            master_id = id - distance;
            fellow_left_index = master_id * chunk;
        }

        barrier1->wait();

        int d = 2 << j;
        Complex w = Complex (1, 0);
        Complex wd = exp(I * Complex (-2 * M_PI / d, 0));
        Complex temp[chunk];
        int which_one = id % (group_size/2);

        w *= pow(wd, chunk * which_one);

        if(id < middle) {
            for (int k = 0; k < chunk; k++) {
                Complex a = arr[left_index + k];//y[k];
                Complex b = arr[fellow_left_index + k];//receive[k];

                temp[k] = a + w * b;//y[k] = a + w * b;
                w = w * wd;
            }
        } else{
            for (int k = 0; k < chunk; k++) {
                Complex b = arr[left_index + k];//receive[k];
                Complex a = arr[fellow_left_index + k];//y[k];

                temp[k] = a - w * b;//y[k] = a - w * b;
                w = w * wd;
            }
        }

        barrier2->wait();

        // Actual data change
        for(int i = 0; i < chunk; ++i){
            arr[left_index + i] = temp[i];
        }

        barrier3->wait();

        distance *= 2;
        groups /= 2;
        group_size *= 2;
    }
}


std::vector<Complex> fft(const std::vector<Complex>& input, unsigned int threads){
    const unsigned int n = input.size();
    const unsigned int chunk = n / threads;
    std::vector<std::thread> threads_array(threads);
    std::vector<Complex> arr = std::vector<Complex>(n);

    // PART 1 - BITWISE REVERSING
    for(int i = 0; i < n; ++i){
        arr[i] = input[bitReverse(i, n)];
    }

    barrier1.reset(new Barrier(threads));
    barrier2.reset(new Barrier(threads));
    barrier3.reset(new Barrier(threads));

    for(int i = 0; i < threads; ++i){
        threads_array.emplace_back(fftTask, std::ref(arr), threads, i);
    }

    for(std::thread & t : threads_array){
        if(t.joinable())
            t.join();
    }

    return arr;
}


int main(int argc, char * argv[]){
    const unsigned int n = 4096;
    int threads = std::stoi(argv[1]);
    std::string input_file = argv[2];
    std::string output_file = argv[3];

    auto arr = readInput(input_file, n);
    auto start = std::chrono::high_resolution_clock::now();
    auto output = fft(arr, threads);
    auto finish = std::chrono::high_resolution_clock::now();
    writeOutput(output_file, output, n);

    std::chrono::duration<double> time = finish - start;
    std::cout << "fft runtime: " << time.count() << std::endl;

    return 0;
}


unsigned int bitReverse(unsigned int numToReverse, unsigned int n){
    static unsigned int bits = log2(n);
    unsigned int reverseNum = 0;

    for(unsigned int i = 0; i <= bits; i++){
        unsigned int temp = (numToReverse & (1 << i));
        if(temp)
            reverseNum |= (1 << ((bits - 1) - i));
    }

    return reverseNum;
}


std::vector<Complex> readInput(std::string input_file, int n){
    std::ifstream file(input_file);
    std::string line = "";
    int i = 0;
    std::vector<Complex> input(n);

    while (getline(file, line) && i < n) {
        std::string delimiter = ", ";
        int pos = line.find(delimiter);

        std::string string_real = line.substr(0, pos);
        std::string string_imag = line.substr(pos + delimiter.length(), line.length());

        ComplexCoeff real = atof(string_real.c_str());
        ComplexCoeff imag = atof(string_imag.c_str());

        input[i++] = Complex(real, imag);
    }

    file.close();
    return input;
}


void writeOutput(std::string output_file, std::vector<Complex> output, int n){
    std::ofstream file;
    file.open(output_file);

    for (int i = 0; i < n; i++) {
        file << output[i].real() << ", " << output[i].imag() << std::endl;
    }

    file.close();
}

