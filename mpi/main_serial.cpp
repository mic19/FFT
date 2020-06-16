#include<iostream>
#include<complex>
#include<chrono>
#include<fstream>


using namespace std;
typedef double ComplexCoeff;
typedef complex<ComplexCoeff> Complex;
const Complex I(0, 1);
const unsigned int n = 4096;  //Number of samples

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


void bitReverseArray(Complex * arr, Complex * y, unsigned int size){
    for(unsigned int i = 0; i < size; i++){
        y[i] = arr[bitReverse(i)];
    }
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


void fft(Complex * arr, Complex * y){
    bitReverseArray(arr, y, n);

    for (unsigned int j = 1; j <= log2(n); j++) {
        int d = 1 << j;
        Complex w = Complex (1, 0);

        Complex wm = exp(I * Complex (-2 * M_PI / d, 0));
        for (int k = 0; k < d/2; k++) {
            for (int m = k; m < n; m += d) {
                Complex t = w * y[m + d/2];
                Complex x = y[m];

                y[m] = x + t;
                y[m + d/2] = x - t;
            }

            w *= wm;
        }
    }
}


// MAIN
#ifndef TEST
int main(int argc, char * argv[]){

    Complex * arr = new Complex[n];
    Complex  * y = new Complex[n];

    string input_file = argv[1];
    string output_file = argv[2];

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

        arr[i++] = Complex(real, imag);
    }

    file.close();

    fft(arr, y);

    ofstream file_output;
    file_output.open(output_file);

    for(int i = 0; i < n; i++) {
        file_output << y[i].real() << ", " << y[i].imag() << endl;
    }

    file_output.close();

    delete[] arr;
    delete [] y;

    return 0;
}

#endif
#ifdef TEST
int main(int argc, char * argv[]){

    Complex * arr = new Complex[n];
    Complex  * y = new Complex[n];
    string times_file = "data/serial_times.txt";

    ofstream file;
    file.open(times_file, ofstream::out | ofstream::trunc);
    file.close();

    for(int num = 0; num < 100; num++) {
        string input_file = "data/inputs/input_" + to_string(num) + ".txt";
        string output_file = "data/serial_outputs/serial_output_" + to_string(num) + ".txt";

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

            arr[i++] = Complex(real, imag);

        }

        file.close();

        // START
        auto start = std::chrono::high_resolution_clock::now();

        fft(arr, y);

        // END
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        auto runtime = elapsed.count();

        ofstream file_times;
        file_times.open(times_file, ios_base::app);
        file_times << "Runtime = " << runtime << endl;
        file_times.close();

        ofstream file_output;
        file_output.open(output_file);

        for (int i = 0; i < n; i++) {
            file_output << y[i].real() << ", " << y[i].imag() << endl;
        }

        file_output.close();
    }

    delete[] arr;
    delete [] y;

    return 0;
}
#endif

