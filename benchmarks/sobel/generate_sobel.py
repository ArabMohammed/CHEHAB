import numpy as np
################
import argparse

# Create the parser
parser = argparse.ArgumentParser(description="Get io_file generation parameters")
is_vectorization_activated = True
parser.add_argument("--slot_count", required=True,type=int, help="Slot_count", default=0)

# Parse arguments
args = parser.parse_args()
#################################################
function_slot_count = args.slot_count
n_rows_image = function_slot_count
n_cols_image = n_rows_image
N = n_rows_image*n_cols_image
is_cipher = 1  
is_signed = 0
input_image = np.random.randint(0,10,(N)) 
gx_image = np.zeros((n_rows_image*n_cols_image))
gy_image = np.zeros((n_rows_image*n_cols_image))
output_image = np.zeros((n_rows_image*n_cols_image))

######################################
# Calculate Gx
kernel_x = [[-1,0,1],[-2,0,2],[-1,0,1]]
for x in range(n_rows_image): 
    for y in range(n_cols_image):
        t=0 
        for i in range(-1,2):
            for j in range(-1,2):
                ni = x + i
                nj = y + j
                if ni >= 0 and ni < n_rows_image and nj >= 0 and nj < n_cols_image :
                    t += kernel_x[i+1][j+1]*input_image[ni*n_cols_image+nj]
        gx_image[(x*n_cols_image+y)%N]=t

##################
# Calculate Gy
kernel_y = [[-1,-2,-1],[0,0,0],[1,2,1]] 
for x in range(n_rows_image): 
    for y in range(n_cols_image):
        t=0 
        for i in range(-1,2):
            for j in range(-1,2):
                ni = x + i
                nj = y + j
                if ni >= 0 and ni < n_rows_image and nj >= 0 and nj < n_cols_image :
                    t += kernel_y[i+1][j+1]*input_image[ni*n_cols_image+nj]
        gy_image[(x*n_cols_image+y)%N]=t

# Calculate Magnitude Squared
for x in range(n_rows_image): 
    for y in range(n_cols_image):
        idx = x*n_cols_image+y
        output_image[idx] = gx_image[idx]*gx_image[idx] + gy_image[idx]*gy_image[idx]

# Since the output is a sum of squares, it is always positive.
# However, for consistency in file parsing, we define the flag.
is_signed_output = 0

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
                input_line= "in_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed)+" "+str(int(input_image[i*n_cols_image+j]))+"\n"
                input_lines.append(input_line)
                # Output line now includes is_signed_output
                output_line= "out_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed_output)+" "+str(int(output_image[i*n_cols_image+j]))+"\n"
                output_lines.append(output_line)
        f.writelines(input_lines)
        f.writelines(output_lines)

    # Generate fhe_input_vectors.txt
    with open("fhe_input_vectors.txt", "w") as file:
        num_vectors = 1 
        vector_size = n_rows_image * n_cols_image
        file.write(f"{num_vectors} {vector_size}\n")

        # Create a flattened list of input labels
        image_labels = "c0i " + " ".join([f"in_{i}_{j}" for i in range(n_rows_image) for j in range(n_cols_image)])
        file.write(image_labels + "\n")

    # Generate fhe_input_vectors_values.txt
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
        f.write(f"img {is_cipher} {is_signed} "+" ".join(f"{num}" for num in input_image )+"\n")
        # Output line now includes is_signed_output
        f.write(f"result {is_cipher} {is_signed_output} "+" ".join(f"{int(num)}" for num in output_image)+"\n")