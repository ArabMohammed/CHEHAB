# Let's generate the padded_in, kernels, and output values, and then write them to a file following the requested naming convention.

# Imports

import numpy as np

# Initialize variables
n_rows_out = 2
n_cols_out = 2
n_channels_out = 5
n_rows_kernel = 5
n_cols_kernel = 5
n_channels_kernel = 1
row_stride = 2  # assuming stride as 1
col_stride = 2  # assuming stride as 1

# Initialize padded_in (31x31x1) and kernels (5x5x1x5) randomly
padded_in = np.random.randint(0,10,(31, 31, 1))
# kernel will be loaded from an existing file
kernels = np.zeros((n_rows_kernel, n_cols_kernel, n_channels_kernel, n_channels_out))
with open("w1.txt", "r") as f:
    lines = f.readlines()
    flatten_kernel = []
    for line in lines : 
        values = line.split(" ")
        for value in values : 
            flatten_kernel.append(int(value))
    ############################
    comp = 0 
    for i_kernels in range(n_rows_kernel):
        for j_kernels in range(n_cols_kernel):
            for k_kernels in range(n_channels_kernel):
                for k_out in range(n_channels_out):
                    kernels[i_kernels, j_kernels, k_kernels, k_out] = flatten_kernel[comp]
                    print(f"{kernels[i_kernels, j_kernels, k_kernels, k_out]} ")
                    comp=comp+1
            print("\n")
# Initialize output (14x14x5) to zeros
output = np.zeros((n_rows_out, n_cols_out, n_channels_out))

############################################################################
############################################################################
# Perform the convolution and store the result in output
row_offset = 0
for i_out in range(n_rows_out):
    col_offset = 0
    for j_out in range(n_cols_out):
        for k_out in range(n_channels_out):
            # Perform convolution
            for i_kernels in range(n_rows_kernel):
                for j_kernels in range(n_cols_kernel):
                    for k_kernels in range(n_channels_kernel):
                        output[i_out, j_out, k_out] += (
                            padded_in[i_kernels + row_offset, j_kernels + col_offset, k_kernels] *
                            kernels[i_kernels, j_kernels, k_kernels, k_out]
                        )
        col_offset += col_stride  # Update column offset for stride
    row_offset += row_stride  # Update row offset for stride

# Write results to a file
function_slot_count= 8
is_cipher = 1 
is_signed = 1
nb_inputs = 31*31 
nb_outputs = n_rows_out*n_cols_out*n_channels_out
with open("matrix_mul_io_example.txt", "w") as f:
    # Write padded_in
    header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
    f.write(header)
    for i in range(31):
        for j in range(31):
            f.write(f"padded_in[{i}][{j}] {is_cipher} {is_signed} {padded_in[i][j][0]} "+ " ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")
    
    # Write output
    for i in range(n_rows_out):
        for j in range(n_cols_out):
            for k in range(n_channels_out):
                f.write(f"res[{i}][{j}][{k}] {is_cipher} {int(output[i][j][k])} "+" ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")

#########################################################################
#########################################################################

"""
import numpy as np

# Initialize variables
n_rows_out = 2
n_cols_out = 2
n_channels_out = 5
n_rows_kernel = 5
n_cols_kernel = 5
n_channels_kernel = 1
row_stride = 2  # assuming stride as 1
col_stride = 2  # assuming stride as 1

# Initialize padded_in (31x31x1) and kernels (5x5x1x5) randomly
padded_in = np.random.randint(0,10,(31, 31, 1))
kernels = np.random.randint(0,10,(n_rows_kernel, n_cols_kernel, n_channels_kernel, n_channels_out))

# Initialize output (14x14x5) to zeros
output = np.zeros((n_rows_out, n_cols_out, n_channels_out))

# Perform the convolution and store the result in output
row_offset = 0
for i_out in range(n_rows_out):
    col_offset = 0
    for j_out in range(n_cols_out):
        for k_out in range(n_channels_out):
            # Perform convolution
            for i_kernels in range(n_rows_kernel):
                for j_kernels in range(n_cols_kernel):
                    for k_kernels in range(n_channels_kernel):
                        output[i_out, j_out, k_out] += (
                            padded_in[i_kernels + row_offset, j_kernels + col_offset, k_kernels] *
                            kernels[i_kernels, j_kernels, k_kernels, k_out]
                        )
        col_offset += col_stride  # Update column offset for stride
    row_offset += row_stride  # Update row offset for stride

# Write results to a file
function_slot_count= 8
is_cipher = 1 
is_signed = 1
nb_inputs = 31*31 + n_rows_kernel*n_cols_kernel*n_channels_kernel
nb_outputs = n_rows_out*n_cols_out*n_channels_out
with open("matrix_mul_io_example.txt", "w") as f:
    # Write padded_in
    header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
    f.write(header)
    for i in range(31):
        for j in range(31):
            f.write(f"padded_in[{i}][{j}] {is_cipher} {is_signed} {padded_in[i][j][0]} "+ " ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")
    
    # Write kernels
    for i in range(n_rows_kernel):
        for j in range(n_cols_kernel):
            for k in range(n_channels_kernel):
                f.write(f"kernel[{i}][{j}][{k}] {is_cipher} {is_signed} "+" ".join(f"{kernels[i][j][k][c]}" for c in range(n_channels_out))+" "+ " ".join(f"{0}" for _ in range(function_slot_count-n_channels_out))+"\n")
    
    # Write output
    for i in range(n_rows_out):
        for j in range(n_cols_out):
            for k in range(n_channels_out):
                f.write(f"res[{i}][{j}][{k}] {is_cipher} {int(output[i][j][k])} "+" ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")
"""

