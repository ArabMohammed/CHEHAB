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
##################################################
# Initialize the output array
width = n_cols_image
output = np.zeros((width, width), dtype=int)
image = np.zeros((width+1, width+1), dtype=int)
input_image = np.zeros((width, width), dtype=int)

# Populate the image with random integers
for i in range(width):
    for j in range(width):
        input_image[i][j] = np.random.randint(0, 10) 
        image[i][j]=input_image[i][j]

rows, cols = width, width

# Apply the kernels to the image (Roberts Cross)
gx_kernel= [[1, 0], [0, -1]]
gy_kernel= [[0, 1], [-1, 0]]

for i in range(rows):
    for j in range(cols):
        # Apply Kernel 1 (Gx)
        Gx = (image[i][j] * gx_kernel[0][0] + image[i][j + 1] * gx_kernel[0][1] +
              image[i + 1][j] * gx_kernel[1][0] + image[i + 1][j + 1] * gx_kernel[1][1])
        
        # Apply Kernel 2 (Gy)
        Gy = (image[i][j] * gy_kernel[0][0] + image[i][j + 1] * gy_kernel[0][1] +
              image[i + 1][j] * gy_kernel[1][0] + image[i + 1][j + 1] * gy_kernel[1][1])
        
        # Compute the gradient magnitude and store in the output array
        output[i][j] = Gx**2 + Gy**2

# Output is magnitude squared, so it is always positive. 
# However, we define the flag for consistency with the file format.
is_signed_output = 0 

##############################################################################################
##############################################################################################
# Output the results
output_image = output
#########################################################################
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
                # Added consistent formatting
                input_line= "in_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed)+" "+str(int(input_image[i][j]))+"\n"
                input_lines.append(input_line)
                # Added is_signed_output to format
                output_line= "out_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed_output)+" "+str(int(output_image[i][j]))+"\n"
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

    # Generate fhe_input_vector_values.txt
    with open("fhe_input_vectors_values.txt", "w") as file:
        # Flatten the input image values (handling the 2D array structure)
        image_values = [str(int(val)) for row in input_image for val in row]
        image_line = f"c0i {is_cipher} {is_signed} " + " ".join(image_values) + "\n"
        file.write(image_line)

####################################################################
else :
    # Flatten arrays for non-vectorized output
    input_image_flat = np.reshape(input_image,(n_rows_image*n_cols_image))
    output_image_flat = np.reshape(output_image,(n_rows_image*n_cols_image))
    
    function_slot_count= N 
    nb_inputs = 1
    nb_outputs = 1

    with open("fhe_io_example.txt", "w") as f:
        header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
        f.write(header)
        input_line= "img "+str(is_cipher)+" "+str(is_signed)+" "+" ".join(f"{num}" for num in input_image_flat)+"\n"
        f.write(input_line)
        # Added is_signed_output to format
        output_line= "result "+str(is_cipher)+" "+str(is_signed_output)+" "+" ".join(f"{int(num)}" for num in output_image_flat)+"\n"
        f.write(output_line)