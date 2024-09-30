import numpy as np
"""
# Parameters
n_elements_input = 9  # Number of elements in 1D input
stride = 2  # Stride for the 1D convolution
n_elements_kernel = 3  # Size of the kernel
n_channels_input = 3  # Input channels (e.g., multiple feature maps)
n_channels_output = 2  # Output channels

# Input and Kernel (Random initialization for demonstration purposes)
# in this case we have an image of 30 pixels with each pixel of 3 elements
input_sequence = np.random.randint(0,2,(n_elements_input, n_channels_input))
kernel = np.random.randint(0,10,(n_elements_kernel, n_channels_input, n_channels_output))

# Output size calculation
n_elements_out = int(n_elements_input / stride + n_elements_input % stride)

# Initialize output array
output_sequence = np.zeros((n_elements_out, n_channels_output))

# Perform 1D convolution with stride
element_offset = 0
for i_out in range(n_elements_out):
    for k_out in range(n_channels_output):
        # Apply kernel for each output channel
        for i_kernel in range(n_elements_kernel):
            for k_input in range(n_channels_input):
                if (i_kernel + element_offset) < n_elements_input:
                    print(f"input : {(i_kernel + element_offset, k_input)}, kernel :{(i_kernel, k_input, k_out)} ")
                    output_sequence[i_out, k_out] += (
                        input_sequence[i_kernel + element_offset, k_input] * kernel[i_kernel, k_input, k_out]
                    )
        print(f"output : {(i_out, k_out)}")
    element_offset += stride  # Update offset for stride
############################################################
element_offset = 0
for i_out in range(n_elements_out):
    for k_out in range(n_channels_output):
        # Apply kernel for each output channel
        vector<vector<size_t>> mask = vector<vector<size_t>(size_image,0),n_channels_input>
        for i_kernel in range(n_elements_kernel):
            for k_input in range(n_channels_input):
                if (i_kernel + element_offset) < n_elements_input:
                    print(f"input : {(i_kernel + element_offset, k_input)}, kernel :{(i_kernel, k_input, k_out)} ")
                    output_sequence[i_out, k_out] += (
                        input_sequence[i_kernel + element_offset, k_input] * kernel[i_kernel, k_input, k_out]
                    )
                ############################################
                mask[k_input][i_kernel + element_offset]=kernel[i_kernel, k_input, k_out]
        output_sequence[i_out, k_out]=Reduc(mask[k_input]*input_image[k_input]) for k_input in range(3)
        
        print(f"output : {(i_out, k_out)}")
    element_offset += stride  # Update offset for stride

# Display result
print("Output sequence:")
print(output_sequence)
##############################################"
input : (0, 0), kernel :(0, 0, 0)  
input : (0, 1), kernel :(0, 1, 0) 
input : (0, 2), kernel :(0, 2, 0) 
input : (1, 0), kernel :(1, 0, 0) 
input : (1, 1), kernel :(1, 1, 0) 
input : (1, 2), kernel :(1, 2, 0) 
input : (2, 0), kernel :(2, 0, 0) 
input : (2, 1), kernel :(2, 1, 0) 
input : (2, 2), kernel :(2, 2, 0)
output (0,0)

input(0,0) kernel :(0, 0, 0) 
input(1,0) kernel :(1, 0, 0)  => input(,0)*kernel(,0,0)* vec{1,1,1,0....}
input(2,0) kernel :(2, 0, 0)  

###########################
input : (0, 1), kernel :(0, 1, 0) 
input : (1, 1), kernel :(1, 1, 0) => input(,1)*kernel(,1,0)*vec{1,1,1,0...}
input : (2, 1), kernel :(2, 1, 0) 
#########################
input : (0, 2), kernel :(0, 2, 0) 
input : (1, 2), kernel :(1, 2, 0) => intpu(,2)*kernel(,2,0)*vec{1,1,1,0.....}
input : (2, 2), kernel :(2, 2, 0)
#####################################################
####################################################
output (0,1)
input : (0, 0), kernel :(0, 0, 1) 
input : (0, 1), kernel :(0, 1, 1) 
input : (0, 2), kernel :(0, 2, 1) 
input : (1, 0), kernel :(1, 0, 1) 
input : (1, 1), kernel :(1, 1, 1) 
input : (1, 2), kernel :(1, 2, 1) 
input : (2, 0), kernel :(2, 0, 1) 
input : (2, 1), kernel :(2, 1, 1) 
input : (2, 2), kernel :(2, 2, 1) 
############################
input : (0, 0), kernel :(0, 0, 1) 
input : (1, 0), kernel :(1, 0, 1) => intput(,0)*kernel(,0,1)*vec{1,1,1,0.....}
input : (2, 0), kernel :(2, 0, 1) 
###########################
input : (0, 1), kernel :(0, 1, 1) 
input : (1, 1), kernel :(1, 1, 1) => intput(,1)*kernel(,1,1)*vec{1,1,1,0.....}
input : (2, 1), kernel :(2, 1, 1) 
########################
input : (0, 2), kernel :(0, 2, 1) 
input : (1, 2), kernel :(1, 2, 1) => intput(,2)*kernel(,2,1)*vec{1,1,1,0.....}
input : (2, 2), kernel :(2, 2, 1) 
#################################################
#################################################
input : (2, 0), kernel :(0, 0, 0) 
input : (2, 1), kernel :(0, 1, 0) 
input : (2, 2), kernel :(0, 2, 0) 
input : (3, 0), kernel :(1, 0, 0) 
input : (3, 1), kernel :(1, 1, 0) 
input : (3, 2), kernel :(1, 2, 0) 
input : (4, 0), kernel :(2, 0, 0) 
input : (4, 1), kernel :(2, 1, 0) 
input : (4, 2), kernel :(2, 2, 0) 
#################################
input : (2, 0), kernel :(0, 0, 0) 
input : (3, 0), kernel :(1, 0, 0) => (intput(,0)<<stride)*kernel(,0,0)*vec{1,1,1,0.....} 
input : (4, 0), kernel :(2, 0, 0) 
#################################
input : (2, 1), kernel :(0, 1, 0) 
input : (3, 1), kernel :(1, 1, 0) => (intput(,1)<<stride)*kernel(,1,0)*vec{1,1,1,0.....} 
input : (4, 1), kernel :(2, 1, 0) 
#################################
input : (2, 2), kernel :(0, 2, 0) 
input : (3, 2), kernel :(1, 2, 0) => (intput(,2)<<stride)*kernel(,2,0)*vec{1,1,1,0.....} 
input : (4, 2), kernel :(2, 2, 0) 
#################################################
#################################################
"""

"""
def zero_pad_1d(signal, padding):
    #Pads the input signal with zeros at both ends.
    return [0] * padding + signal + [0] * padding

def apply_kernel_1d(signal_slice, kernel):
    #Applies the kernel to the given 1D signal slice.
    return sum(signal_slice[i] * kernel[i] for i in range(len(kernel)))

def manual_strided_convolution_1d(input_signal, kernel, stride, padding=0):
    #Performs strided 1D convolution manually without NumPy.
    # Input signal dimensions
    n_input = len(input_signal)
    
    # Kernel dimensions
    n_kernel = len(kernel)
    
    # Pad the input signal
    padded_signal = zero_pad_1d(input_signal, padding)
    
    # Calculate output dimensions
    n_output = (n_input + 2 * padding - n_kernel) // stride + 1
    
    # Initialize the output array
    output = [0] * n_output
    
    # Perform the convolution with the specified stride
    for i_out in range(n_output):
        # Extract the slice of the input signal corresponding to the current position
        offset = i_out * stride
        
        # Extract the 1D slice and apply the kernel
        signal_slice = padded_signal[offset:offset + n_kernel]
        output[i_out] = apply_kernel_1d(signal_slice, kernel)
    
    return output

# Example usage
input_signal = [1, 2, 3, 4, 5, 6, 7, 8]  # Example input signal (1D array)
kernel = [1, 0, -1]  # Example kernel (1D filter)
stride = 2  # Stride value
padding = 1  # Padding value

# Perform manual strided 1D convolution
output_signal = manual_strided_convolution_1d(input_signal, kernel, stride, padding)

# Print the output
print(output_signal)
"""

n_rows_image = 3 
n_cols_image = 3
N = n_rows_image*n_cols_image
input_image = np.random.randint(0,2,(n_rows_image*n_cols_image))
output_image = np.zeros((n_rows_image*n_cols_image))
###################
kernel = [[1,1,1],[1,-8,1],[1,1,1]]
for x in range(n_rows_image):
    for y in range(n_cols_image):
        t=0 
        for j in range(-1,2):
            for i in range(-1,2):
                print(f"kernel :{((j+1)*3)+i+1}, Image :{((x+i)*n_rows_image+(y+j))%N}")
                t+=kernel[i+1][j+1]*input_image[((x+i)*n_rows_image+(y+j))%N]
                output_image[(x*n_rows_image+y)%N]=2*input_image[(x*n_rows_image+y)%N]-t
        print("Next ===>")
#######################################################
#######################################################
kernel :0, Image :5
kernel :1, Image :8
kernel :2, Image :2
kernel :3, Image :6
kernel :4, Image :0
kernel :5, Image :3
kernel :6, Image :7
kernel :7, Image :1
kernel :8, Image :4
##################
kernel :0, Image :5   
kernel :1, Image :8 => 
kernel :2, Image :2
"""
# Image dimensions
n_rows_image = 3
n_cols_image = 3

# Generate a random binary input image (values 0 or 1)
input_image = np.random.randint(0, 2, (n_rows_image, n_cols_image))

# Initialize the output image
output_image = np.zeros((n_rows_image, n_cols_image))

# Define a convolution kernel (Laplacian filter)
kernel = [[1, 1, 1], [1, -8, 1], [1, 1, 1]]

# Apply convolution
for x in range(n_rows_image):
    for y in range(n_cols_image):
        t = 0  # Temporary variable for accumulation
        # Loop over kernel positions
        for j in range(0, 3):
            for i in range(0, 3):
                # Compute image coordinates with boundary checking (using modulo for wrapping)
                img_x = (x + i -1) % n_rows_image
                img_y = (y + j -1) % n_cols_image
                # Access kernel and image values, perform convolution
                print(f"kernel :({i},{j}), Image :({img_x},{img_y})")
                t += kernel[i][j] * input_image[img_x, img_y]
        print("next =====> ")
        # Update the output image with the convolution result
        output_image[x, y] = 2 * input_image[x, y] - t

# Display input and output images
print("Input Image:\n", input_image)
print("Output Image:\n", output_image)
################################################
################################################
kernel :(0,0), Image :(2,2)
kernel :(1,0), Image :(0,2)
kernel :(2,0), Image :(1,2)
kernel :(0,1), Image :(2,0)
kernel :(1,1), Image :(0,0)
kernel :(2,1), Image :(1,0)
kernel :(0,2), Image :(2,1)
kernel :(1,2), Image :(0,1)
kernel :(2,2), Image :(1,1)
"""