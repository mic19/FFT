main:
	upcc -gupc -network=udp main.c
local_run: main
	UPC_NODES=localhost upcrun -c 4 -N 1 -n 4 ./a.out example/input.txt example/actual_output.txt
run: main
	UPC_NODEFILE=nodes upcrun -c 4 -N 1 -n 4 ./a.out example/input.txt example/actual_output.txt
generate_cases:
	python3 generate_cases.py
clean:
	rm -fR a.out data example/actual_output.txt
