

n = 2000

def avg(filename):
	times_file = open(filename, "r+")
	times_file
	times_sum = 0
	counter = 0
		
	for line in times_file:
		if counter >= n:
			break
		times_sum += float(line.split("=")[1])
		counter += 1		

	average = times_sum / n
	print(filename)
	print("Average runtime = " + str(average))
	times_file.truncate(0)


if __name__ == '__main__':
	avg("data/serial_times.txt")
	avg("data/mpi_times_n1.txt")
	avg("data/mpi_times_n2.txt")
	avg("data/mpi_times_n4.txt")
	avg("data/mpi_times_n8.txt")
	avg("data/mpi_times_n16.txt")
