import numpy as np
import argparse

# Create the parser
parser = argparse.ArgumentParser(description="Get io_file generation parameters")
is_vectorization_activated = True
parser.add_argument("--slot_count", required=True, type=int, help="Slot_count", default=0)

# Parse arguments
args = parser.parse_args()
function_slot_count = args.slot_count

######################################################
# Generate random integer arrays c1 and c2
c1 = np.random.randint(0, 10, (function_slot_count))
c2 = np.random.randint(0, 10, (function_slot_count))

# Calculate the dot product of c1 and c2
res = np.dot(c1, c2)

# Create the result array where each element is the dot product
result = np.full((function_slot_count), res)

is_cipher = 1
is_signed = 1

if is_vectorization_activated:
    # Create and write to the original I/O example file
    with open("fhe_io_example.txt", "w") as file:
        nb_inputs = 2 * function_slot_count
        nb_outputs = 1
        slot_count = 1  # Assuming slot_count is 1 for this vectorized case
        header = f"{slot_count} {nb_inputs} {nb_outputs}\n"
        file.write(header)
        rows = []
        for i in range(function_slot_count):
            rows.append(f"v1_{i} {is_cipher} {is_signed} {c1[i]}\n")
        for i in range(function_slot_count):
            rows.append(f"v2_{i} {is_cipher} {is_signed} {c2[i]}\n")
        rows.append(f"output {is_cipher} {int(result[0])}\n")
        file.writelines(rows)

    # Create and write to the new vectorized input file
    with open("fhe_input_vectors.txt", "w") as file:
        # First line: number of input vectors
        file.write(f"2 {function_slot_count}\n")

        # Second line: names of the components of the first vector
        c1_names = "c1i " + " ".join([f"v1_{i}" for i in range(function_slot_count)])
        file.write(c1_names + "\n")

        # Fourth line: names of the components of the second vector
        c2_names = "c2i " + " ".join([f"v2_{i}" for i in range(function_slot_count)])
        file.write(c2_names + "\n")

        # Create and write to fhe_input_vector_values.txt
    with open("fhe_input_vectors_values.txt", "w") as file:
        # Write c1i line
        c1_line = "c1i 1 1 " + " ".join(map(str, c1)) + "\n"
        file.write(c1_line)

        # Write c2i line
        c2_line = "c2i 1 1 " + " ".join(map(str, c2)) + "\n"
        file.write(c2_line)


else:
    # This part of the script remains unchanged
    with open("fhe_io_example.txt", "w") as file:
        nb_inputs = 2
        nb_outputs = 1
        header = f"{function_slot_count} {nb_inputs} {nb_outputs}\n"
        file.write(header)
        rows = []
        row_c1 = f"c1 {is_cipher} {is_signed} {' '.join(map(str, c1))}\n"
        rows.append(row_c1)
        row_c2 = f"c2 {is_cipher} {is_signed} {' '.join(map(str, c2))}\n"
        rows.append(row_c2)
        row_result = f"result {is_cipher} {' '.join(map(str, result.astype(int)))}\n"
        rows.append(row_result)
        file.writelines(rows)