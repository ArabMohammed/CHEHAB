import random
class Matrix : 
    def __init__(self, nb_rows,nb_cols):
        self.nb_rows = nb_rows
        self.nb_cols = nb_cols 
    
def generate_matrix(mat : Matrix): 
    result = []
    for i in range(mat.nb_rows): 
        temp = []
        for j in range(mat.nb_cols):  
            temp.append(random.randrange(0,10))
        result.append(temp)
    return result 
def multiply_matrixes(A : Matrix,B : Matrix): 
    C = Matrix(nb_rows=A.nb_rows,nb_cols=B.nb_cols)
    result = []
    result = [[sum(A.value[i][k] * B.value[k][j] for k in range(A.nb_cols)) for j in range(B.nb_cols)] for i in range(A.nb_rows)]
    C.value = result
    return C
#############################################################################################################
#############################################################################################################
import argparse
# Create the parser
parser = argparse.ArgumentParser(description="Get io_file generation parameters")
is_vectorization_activated = True
parser.add_argument("--slot_count", required=True,type=int, help="Slot_count", default=0)
# Parse arguments
args = parser.parse_args()
function_slot_count = args.slot_count 
####################
is_cipher = 1 
is_signed = 0
matrix_A = Matrix(nb_rows=function_slot_count,nb_cols=function_slot_count)
matrix_B = Matrix(nb_rows=function_slot_count,nb_cols=function_slot_count)
matrix_A.value = generate_matrix(matrix_A)
matrix_B.value = generate_matrix(matrix_B)
matrix_C = multiply_matrixes(matrix_A,matrix_B)
## writing matrix to file  
if is_vectorization_activated : 
    with open("fhe_io_example.txt","w") as file : 
        nb_inputs= matrix_A.nb_rows*matrix_A.nb_cols + matrix_B.nb_rows*matrix_B.nb_cols 
        nb_outputs = matrix_C.nb_rows*matrix_C.nb_cols 
        slot_count = 1
        header = str(slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
        file.write(header) 
        rows=[]
        for i in range(matrix_A.nb_rows): 
            for j in range(matrix_A.nb_cols):
                row = "a_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed)+" "+str(matrix_A.value[i][j])+"\n"
                rows.append(row)
        ######################################################################################################
        for i in range(matrix_A.nb_rows): 
            for j in range(matrix_A.nb_cols):
                row = "b_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(is_signed)+" "+str(matrix_B.value[i][j])+"\n"
                rows.append(row)
        for i in range(matrix_C.nb_rows): 
            for j in range(matrix_C.nb_cols):
                row = "c_{}_{}".format(i,j)+" "+str(is_cipher)+" "+str(matrix_C.value[i][j])+"\n"
                rows.append(row)
        file.writelines(rows)


    # Generate fhe_input_vectors.txt
    with open("fhe_input_vectors.txt", "w") as file:
        num_vectors = 2  # matrix A and matrix B
        file.write(f"{num_vectors} {function_slot_count ** 2}\n")  # Each matrix is flattened

        a_labels = "c0i " + " ".join([f"a_{i}_{j}" for i in range(matrix_A.nb_rows) for j in range(matrix_A.nb_cols)])
        b_labels = "c1i " + " ".join([f"b_{i}_{j}" for i in range(matrix_B.nb_rows) for j in range(matrix_B.nb_cols)])

        file.write(a_labels + "\n")
        file.write(b_labels + "\n")

    # Generate fhe_input_vector_values.txt
    with open("fhe_input_vectors_values.txt", "w") as file:
        # A values flattened
        a_values = [str(matrix_A.value[i][j]) for i in range(matrix_A.nb_rows) for j in range(matrix_A.nb_cols)]
        a_line = "c0i 1 1 " + " ".join(a_values) + "\n"
        file.write(a_line)

        # B values flattened
        b_values = [str(matrix_B.value[i][j]) for i in range(matrix_B.nb_rows) for j in range(matrix_B.nb_cols)]
        b_line = "c1i 1 1 " + " ".join(b_values) + "\n"
        file.write(b_line)

else :
    with open("fhe_io_example.txt","w") as file : 
        nb_inputs= matrix_A.nb_rows + matrix_B.nb_cols 
        nb_outputs = matrix_A.nb_rows 
        header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
        file.write(header) 
        rows=[]
        for i in range(matrix_A.nb_rows): 
            row = "A[{}]".format(i)+" "+str(is_cipher)+" "+str(is_signed)+" "+" ".join(str(num) for num in matrix_A.value[i])+"\n"
            rows.append(row)
        file.writelines(rows)
        cols = []
        for j in range(matrix_B.nb_cols):
            col = "B[{}]".format(j)+" "+str(is_cipher)+" "+str(is_signed)+" "+" ".join(str(matrix_B.value[i][j]) for i  in range(matrix_B.nb_rows))+"\n"
            cols.append(col)
        file.writelines(cols)
        rows=[]
        for i in range(matrix_C.nb_rows): 
            row = "C[{}]".format(i)+" "+str(is_cipher)+" "+" ".join(str(num) for num in matrix_C.value[i])+"\n"
            rows.append(row)
        file.writelines(rows)