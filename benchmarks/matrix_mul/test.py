import numpy as np

# initialize input image
function_slot_count= 8 
input_image = np.random.randint(0,10,(function_slot_count,function_slot_count, 1))
temp = []
for i in range(8):
    temp.append(input_image[i,i])  
nb_inputs = 8 
nb_outputs = 1       
is_cipher = 1  
is_signed =1 
with open("matrix_mul_io_example.txt", "w") as f:
    # Write padded_in
    header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
    f.write(header)
    for i in range(function_slot_count):
                f.write(f"X[{i}] {is_cipher} {is_signed} "+" ".join(f"{int(num)}" for num in input_image[i])+"\n")
    
    f.write(f"res {is_cipher} "+" ".join(f"{int(temp[i])}" for i in range(function_slot_count))+"\n")

    