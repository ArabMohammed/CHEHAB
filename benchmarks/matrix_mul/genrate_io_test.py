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
            temp.append(random.randrange(-5,5))
        result.append(temp)
    return result 
def multiply_(A : Matrix,B : Matrix): 
    C = Matrix(nb_rows=A.nb_rows,nb_cols=B.nb_cols)
    result = []
    result = [[sum(A.value[i][k] * B.value[k][j] for k in range(A.nb_cols)) for j in range(B.nb_cols)] for i in range(A.nb_rows)]
    C.value = result
    return C
################################################
function_slot_count= 64
size_i = function_slot_count
size_j = function_slot_count
size_k = function_slot_count
input = []
b=[]
output=[]
################################
##############################
for i in range(size_i*size_j): 
        temp = []
        for j in range(size_k): 
            temp.append(random.randrange(-5,5))
        input.append(temp)
##############################
for k in range(size_k):
    b.append(random.randrange(-5,5))
##############################
for i in range(size_i*size_j): 
    temp=[]
    for k in range(size_k):
        temp.append(b[k]*input[i][k])
    output.append(temp)
##############################
is_cipher = 1 
is_signed = 1
## writing matrix to file  
with open("matrix_mul_io_example.txt","w") as file : 
    nb_inputs= size_i*size_j + 1 
    nb_outputs = size_i*size_j 
    header = str(function_slot_count)+" "+str(nb_inputs)+" "+str(nb_outputs)+"\n"
    file.write(header) 
    rows=[]
    for i in range(size_i*size_j):
        nb_row = i // function_slot_count 
        nb_col = i % function_slot_count 
        row = "input[{}][{}]".format(nb_row,nb_col)+" "+str(is_cipher)+" "+str(is_signed)+" "+" ".join(str(num) for num in input[i])+"\n"
        rows.append(row)
    ###
    row = "b"+" "+str(is_cipher)+" "+str(is_signed)+" "+" ".join(str(num) for num in b)+"\n"
    rows.append(row)
    file.writelines(rows)
    ## outputs
    rows=[] 
    for i in range(size_i*size_j): 
        nb_row = i // function_slot_count 
        nb_col = i % function_slot_count 
        row = "output[{}][{}]".format(nb_row,nb_col)+" "+str(is_cipher)+" "+str(is_signed)+" "+" ".join(str(num) for num in output[i])+"\n"
        rows.append(row)
    file.writelines(rows)
  Var i("i",0,m_a);
  Var j("j",0,m_a);
  Var k("k",0,m_a);
  Input input("input",{i,j,k},Type::vectorciphertxt);
  Input b("b",{k},Type::vectorciphertxt);
  Computation output("output",{i,j,k},input(i,j,k)*b(k)); 
  output.evaluate(true);
  */
/******************************************************/
 /*  Var i("i",0,m_a); 
  Var j("j",0,m_a);
  Var k("k",0,m_a);
  Input A("A",{i,k},Type::vectorciphertxt);
  Input B("B",{j,k},Type::vectorciphertxt);
  Computation C("C", {i,j,k},{i,j},A(i,k)*B(j,k)); 
  C.evaluate(true);  */
/*******************************************************/
  auto row_stride = 2; // =2
  auto col_stride = 2; // =2
  Var i_out("i_out",0,2); // 14
  Var j_out("j_out",0,2); // 14
  Var k_out("k_out",0,5);
  Var i_kernels("i_kernels",0,5);
  Var j_kernels("j_kernels",0,5);
  Var k_kernels("k_kernels",0,1);
  Var i_in("i_in",0,31);
  Var j_in("j_in",0,31);
  Var k_in("k_in",0,1);
  Input padded_in ("padded_in",{i_in,j_in,k_in},Type::vectorciphertxt);
  Input kernels ("kernel",{i_kernels,j_kernels,k_kernels,k_out},Type::vectorciphertxt);
  Computation res("res",{i_out,j_out,k_out,i_kernels,k_kernels,j_kernels},{i_out,j_out,k_out},padded_in(i_kernels + i_out*row_stride , j_kernels + j_out*col_stride , k_kernels)*kernels(i_kernels , j_kernels , k_kernels , k_out));
  res.evaluate(true);
  /*********************************************************/