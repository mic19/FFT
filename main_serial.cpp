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


Complex * bitReverseArray(Complex * arr, unsigned int size){
    Complex * y = new Complex[size];

    for(unsigned int i = 0; i < size; i++){
        y[i] = arr[bitReverse(i)];
    }

    return y;
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


int main(int argc, char * argv[]){

    auto start = std::chrono::high_resolution_clock::now();

    Complex * arr = new Complex[n];
    ifstream file("data/inputs/input_"  + string(argv[1]) + ".txt");
    string line = "";

    int i = 0;
    while (getline(file, line) && i < n){

        string delimiter = ", ";
        int pos = line.find(delimiter);

        string string_real = line.substr(0, pos);
        string string_imag = line.substr(pos + delimiter.length(), line.length());

        ComplexCoeff real = atof(string_real.c_str());
        ComplexCoeff imag = atof(string_imag.c_str());

        arr[i++] = Complex(real, imag);

    }

    file.close();

    Complex * y = bitReverseArray(arr, n);

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

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    cout << "Runtime = " << elapsed.count() << endl;

    ofstream file_output;
    file_output.open ("data/serial_outputs/serial_output_" + string(argv[1]) + ".txt");

    for(int i = 0; i < n; i++) {
        file_output << y[i].real() << ", " << y[i].imag() << endl;
    }
    file_output.close();

    delete[] arr;
    delete[] y;

    return 0;
}


