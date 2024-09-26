import numpy as np

# initialize input image
n_rows_image = 10 # 28
n_cols_image = 10 # 28
n_channels_in = 1 # 1
function_slot_count= 4
kernel_shape = [4,4,1,4] # 5 5 1 5 
print(kernel_shape)
strides = [2,2]
mean_pool_kernel_shape = [2, 2]
input_image = np.random.randint(0,2,(n_rows_image,n_cols_image, n_channels_in))
# pad_2d of image : 
n_rows_kernel = kernel_shape[0] 
n_cols_kernel = kernel_shape[1] 
n_channels_out = kernel_shape[3]  
row_stride = strides[0]
col_stride = strides[1]
##############################################################
n_rows_out =int((n_rows_image + 1) / row_stride); 
n_cols_out = int((n_cols_image + 1) / col_stride); 

pad_rows = max((n_rows_out - 1) * row_stride + n_rows_kernel - n_rows_image,0); # 3 
pad_cols = max((n_cols_out - 1) * col_stride + n_cols_kernel - n_cols_image,0); # 3
pad_top = int(pad_rows / 2)
pad_left =int(pad_cols / 2)

n_rows_out = n_rows_image + pad_rows; 
n_cols_out = n_cols_image + pad_cols;  
# initialize output 
print(n_rows_out,n_cols_out)
padded_2d = np.zeros((n_rows_out, n_cols_out,n_channels_in))
for i in range(n_rows_image):
    for j in range( n_cols_image):     
        for k in range(n_channels_in):                                                                              
          padded_2d[i + pad_top,j + pad_left] = input_image[i,j]

####################################################
# kernel will be loaded from an existing file
print(f"pad_top :{pad_top}, pad_left:{pad_left} \n")
n_channels_kernel = kernel_shape[2]
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
                    comp=comp+1
###############################################
# Initialize output (14x14x5) to zeros
n_rows_out = int(n_rows_image / row_stride + n_rows_image % row_stride) # 14
n_cols_out = int(n_cols_image / col_stride + n_cols_image % col_stride)  # 14
print(f"conv_2d : {n_rows_out,n_cols_out,n_channels_out}")
output = np.zeros((n_rows_out, n_cols_out, n_channels_out))
row_offset = 0
for i_out in range(1): # n_rows_out
    col_offset = 0
    for j_out in range(1): # n_cols_out
        for k_out in range(n_channels_out):
            ###############################
            for i_kernels in range(n_rows_kernel):
                for j_kernels in range(n_cols_kernel):
                    for k_kernels in range(n_channels_kernel):
                        output[i_out, j_out, k_out] += (
                            padded_2d[(i_kernels + row_offset),(j_kernels + col_offset), k_kernels] *
                            kernels[i_kernels, j_kernels, k_kernels, k_out]
                        )
            ##############################################################
        col_offset += col_stride  # Update column offset for stride
    row_offset += row_stride  # Update row offset for stride
##############Sum with b vector#####################
#vector<integer> b_raw = {64818,1519,391,64179,63483};
b_raw = [648,151,391,641]
for i_out in range(n_rows_out):
    for j_out in range(n_cols_out):
        for k_out in range(n_channels_out):
            output[i_out, j_out, k_out]+=b_raw[k_out]
###########################square output############
for i_out in range(n_rows_out):
    for j_out in range(n_cols_out):
        for k_out in range(n_channels_out):
            output[i_out, j_out, k_out]=output[i_out, j_out, k_out]*output[i_out, j_out, k_out]
#### scaled_mean_pool_2d ###############
input = output
n_rows_in = len(input) # 14 
n_cols_in = len(input[0])  # 14 
n_channels_in = len(input[0][0]); # 5
n_rows_kernel = mean_pool_kernel_shape[0];  # = 2
n_cols_kernel = mean_pool_kernel_shape[1];  # = 2
row_stride = strides[0]
col_stride = strides[1]

n_rows_out = int(n_rows_in / row_stride + (n_rows_in % row_stride))
n_cols_out = int(n_cols_in / col_stride + (n_cols_in % col_stride))
n_channels_output = n_channels_in
print(f"scaled_mean_pool_2d : {n_rows_out, n_cols_out, n_channels_output}")
output = np.zeros((n_rows_out, n_cols_out, n_channels_output))
##############
row_offset = 0
for  i_output in range(1) : # n_rows_out
    col_offset = 0
    for j_output in range(1): # n_cols_out
        for k_output in range(n_channels_output):
            for i_kernel in range(n_rows_kernel):
                for j_kernel in range(n_cols_kernel):
                    output[i_output, j_output, k_output] += input[i_kernel + row_offset, j_kernel + col_offset, k_output]
        col_offset += col_stride
row_offset += row_stride
#########################################################

# Write results to a file
is_cipher = 1 
is_signed = 1
nb_inputs = n_rows_image*n_cols_image
#nb_outputs = n_rows_out*n_cols_out*n_channels_out
#nb_outputs = n_rows_out*n_cols_out
nb_outputs = 1
with open("matrix_mul_io_example.txt", "w") as f:
    # Write padded_in
    header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
    f.write(header)
    for i in range(10):
        for j in range(10):
            f.write(f"x[{i}][{j}] {is_cipher} {is_signed} "+" ".join(f"{num}" for num in input_image[i][j] )+" "+" ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")
    
    # Write output
    """
    for i in range(n_rows_out):
        for j in range(n_cols_out):
             for k in range(n_channels_out):
                f.write(f"res[{i}][{j}][{k}] {is_cipher} {int(output[i][j][k])} "+" ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")
    """
    for i in range(1): # n_rows_out
        for j in range(1): # n_cols_out
            #for k in range(n_channels_out):
                f.write(f"res[{i}][{j}] {is_cipher} "+" ".join(f"{int(num)}" for num in output[i, j])+" "+" ".join(f"{0}" for _ in range(function_slot_count-n_channels_out))+"\n")
                #f.write(f"res[{i}][{j}][{k}] {is_cipher} {int(output[i][j][k])} "+" ".join(f"{0}" for _ in range(function_slot_count-1))+"\n")
    

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

