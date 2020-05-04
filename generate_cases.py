import numpy as np
import random
import os
from pathlib import Path


def print_list(array):
	for i in array:
		print(str(round(i.real, 2)) + ' + ' + str(round(i.imag, 2)) + 'i')


def write_file(array, filename, p):
	data_file = open(filename, p)
	for i in array:
		data_file.write(str(round(i.real, 2)) + ', ' + str(round(i.imag, 2)) + '\n')

	data_file.close()


n = 100


if __name__ == '__main__':

	Path("data/inputs").mkdir(parents=True, exist_ok=True)
	Path("data/expected_outputs").mkdir(parents=True, exist_ok=True)
	Path("data/actual_outputs").mkdir(parents=True, exist_ok=True)
	Path("data/serial_outputs").mkdir(parents=True, exist_ok=True)

	for i in range(n):
		random.seed(i)
		test_data_arr = [random.randint(0, 10) for i in range(0, 4096)]

		out_np = np.fft.fft(test_data_arr)
		write_file(test_data_arr, "data/inputs/input_" + str(i) + ".txt", "w")
		write_file(out_np, "data/expected_outputs/expected_output_" + str(i) + ".txt", "w")




