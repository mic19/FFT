

if __name__ == '__main__':
	expected_file = open("example/expected_output.txt", "r")
	actual_file = open("example/actual_output.txt", "r")
	counter = 0
	errors = 0
		
	for line1, line2 in zip(actual_file, expected_file):
		temp1 = line1.split(",")
		temp2 = line2.split(",")

		if abs(float(temp1[0]) - float(temp2[0])) > 0.1 \
			or abs(float(temp1[1]) - float(temp2[1])) > 0.1:
			print("Error in line " + str(counter))
			errors += 1

		counter += 1

	if errors == 0:
		print("OK")

	
