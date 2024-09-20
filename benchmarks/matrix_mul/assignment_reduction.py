import numpy as np


n_rows =  8 
n_cols = 8 
n_k =8 
# Initialize padded_in (31x31x1) and kernels (5x5x1x5) randomly
tensor = np.random.randint(1,10,(n_rows, n_cols, n_k))
# kernel will be loaded from an existing file
output = np.zeros((n_rows, n_cols))
############################################################################
############################################################################
for i_kernels in range(n_rows):
    for j_kernels in range(n_cols):
            output[i_kernels,j_kernels]=np.sum(tensor[i_kernels,j_kernels])


# Write results to a file
function_slot_count= 8
is_cipher = 1 
is_signed = 1
nb_inputs = n_rows*n_cols
nb_outputs = n_rows
with open("matrix_mul_io_example.txt", "w") as f:
    # Write padded_in
    header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
    f.write(header)
    for i in range(n_rows):
        for j in range(n_cols):
            f.write(f"tensor[{i}][{j}] {is_cipher} {is_signed} "+ " ".join(f"{int(tensor[i][j][k])}" for k in range(n_k))+"\n")
    
    # Write output
    for i in range(n_rows):
                f.write(f"output[{i}] {is_cipher} "+" ".join(f"{int(output[i][j])}" for j in range(n_cols))+"\n")

#########################################################################
#########################################################################
