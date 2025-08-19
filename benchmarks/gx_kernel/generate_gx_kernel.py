import numpy as np
from math import sqrt
################
import argparse

# Create the parser
parser = argparse.ArgumentParser(description="Get io_file generation parameters")
is_vectorization_activated = True
parser.add_argument("--slot_count", required=True,type=int, help="Slot_count", default=0)

# Parse arguments
args = parser.parse_args()
function_slot_count = args.slot_count
###############################################
n_rows_image = function_slot_count
n_cols_image = n_rows_image
N = n_rows_image*n_cols_image
is_cipher = 1
is_signed = 0 # Output might be negative, so this should probably be 1
input_image = np.random.randint(2,10,(N))
output_image = np.zeros((n_rows_image*n_cols_image))
######################################
kernel = [[-1,0,1],[-2,0,2],[-1,0,1]]
for x in range(n_rows_image):
    for y in range(n_cols_image):
        t=0
        for i in range(-1,2):
            for j in range(-1,2):
                ni = x + i
                nj = y + j
                if ni >= 0 and ni < n_rows_image and nj >= 0 and nj < n_cols_image :
                    t += kernel[i+1][j+1]*input_image[ni*n_cols_image+nj]
        output_image[(x*n_cols_image+y)%N]=t

# Since the kernel has negative values, the output can be negative.
# It is recommended to set is_signed to 1 for the output.
is_signed_output = 1
###############################################################################
###############################################################################
if is_vectorization_activated :
    function_slot_count = 1
    nb_inputs = N
    nb_outputs = N
    with open("fhe_io_example.txt", "w") as f:
        header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
        f.write(header)
        input_lines = []
        output_lines = []
        for i in range(n_rows_image):
            for j in range(n_cols_image):
                # Input lines (assuming input pixels are not signed)
                input_line= "in_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed)+" "+str(int(input_image[i*n_cols_image+j]))+"\n"
                input_lines.append(input_line)
                # Output lines (using is_signed_output for potentially negative results)
                output_line= "out_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed_output)+" "+str(int(output_image[i*n_cols_image+j]))+"\n"
                output_lines.append(output_line)
        f.writelines(input_lines)
        f.writelines(output_lines)

    # Generate fhe_input_vectors.txt
    with open("fhe_input_vectors.txt", "w") as file:
        num_vectors = 1  # Just the input image
        vector_size = n_rows_image * n_cols_image
        file.write(f"{num_vectors} {vector_size}\n")

        # Create a flattened list of input labels
        image_labels = "c0i " + " ".join([f"in_{i}_{j}" for i in range(n_rows_image) for j in range(n_cols_image)])
        file.write(image_labels + "\n")

    # Generate fhe_input_vector_values.txt
    with open("fhe_input_vectors_values.txt", "w") as file:
        # Flatten the input image values
        image_values = [str(int(val)) for val in input_image]
        image_line = f"c0i {is_cipher} {is_signed} " + " ".join(image_values) + "\n"
        file.write(image_line)

####################################################################
else :
    function_slot_count= N
    nb_inputs = 1
    nb_outputs = 1
    with open("fhe_io_example.txt", "w") as f:
        header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
        f.write(header)
        # Input line
        f.write(f"img {is_cipher} {is_signed} "+" ".join(f"{num}" for num in input_image )+"\n")
        # Output line (using is_signed_output)
        f.write(f"result {is_cipher} {is_signed_output} "+" ".join(f"{int(num)}" for num in output_image)+"\n")


# import numpy as np
# from math import sqrt 
# ################
# import argparse
# # Create the parser 
# parser = argparse.ArgumentParser(description="Get io_file generation parameters")
# is_vectorization_activated = True
# parser.add_argument("--slot_count", required=True,type=int, help="Slot_count", default=0)
# # Parse arguments
# args = parser.parse_args()
# function_slot_count = args.slot_count
# ###############################################
# n_rows_image = function_slot_count
# n_cols_image = n_rows_image
# N = n_rows_image*n_cols_image 
# is_cipher = 1 
# is_signed = 0
# input_image = np.random.randint(2,10,(N)) 
# output_image = np.zeros((n_rows_image*n_cols_image))
# ######################################
# kernel = [[-1,0,1],[-2,0,2],[-1,0,1]]
# for x in range(n_rows_image): 
#     for y in range(n_cols_image):
#         t=0 
#         for i in range(-1,2):
#             for j in range(-1,2):
#                 ni = x + i
#                 nj = y + j
#                 if ni >= 0 and ni < n_rows_image and nj >= 0 and nj < n_cols_image :
#                     t += kernel[i+1][j+1]*input_image[ni*n_cols_image+nj]
#         output_image[(x*n_cols_image+y)%N]=t
# ###############################################################################
# ###############################################################################
# if is_vectorization_activated :
#     function_slot_count = 1 
#     nb_inputs = N
#     nb_outputs = N
#     with open("fhe_io_example.txt", "w") as f:
#         header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
#         f.write(header)
#         input_lines = []
#         output_lines = []
#         for i in range(n_rows_image):
#             for j in range(n_cols_image):
#                 input_line= "in_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed)+" "+str(int(input_image[i*n_cols_image+j]))+"\n"
#                 input_lines.append(input_line)
#                 output_line= "out_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(int(output_image[i*n_cols_image+j]))+"\n"
#                 output_lines.append(output_line)
#         f.writelines(input_lines)
#         f.writelines(output_lines)
# ####################################################################
# else :
#     function_slot_count= N 
#     nb_inputs = 1
#     nb_outputs = 1
#     #nb_outputs = n_rows_out*n_cols_out
#     #nb_outputs = 1 # 3*3*4 
#     with open("fhe_io_example.txt", "w") as f:
#         header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
#         f.write(header)
#         f.write(f"img {is_cipher} {is_signed} "+" ".join(f"{num}" for num in input_image )+"\n")
#         f.write(f"result {is_cipher} "+" ".join(f"{int(num)}" for num in output_image)+"\n")



