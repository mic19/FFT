main:
	mpiCC -O2 main.cpp
run: main
	mpirun -n 4 ./a.out example/input.txt example/actual_output.txt
test:
	mpiCC -O2 main.cpp -D TEST
run_test: test generate_cases
	mpirun -n 4 ./a.out
serial:
	g++ -O2 main_serial.cpp -o serial.out
serial_run: serial
	./serial.out example/input.txt example/serial_output.txt
serial_test:
	g++ -O2 main_serial.cpp -D TEST -o serial.out
serial_run_test: serial_test generate_cases
	./serial.out
generate_cases:
	python3 generate_cases.py
clean:
	rm -fR a.out serial.out data example/actual_output.txt example/serial_output.txt
