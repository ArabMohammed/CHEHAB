import re
file_content = """
#include <cstddef>
#include <cstdint>
#include <utility>
#include "_gen_he_box_blur.hpp"

using namespace std;
using namespace seal;

void box_blur(const unordered_map<string, Ciphertext> &encrypted_inputs,
const unordered_map<string, Plaintext> &encoded_inputs,
unordered_map<string, Ciphertext> &encrypted_outputs,
unordered_map<string, Plaintext> &encoded_outputs,
const BatchEncoder &encoder,
const Encryptor &encryptor,
const Evaluator &evaluator,
const RelinKeys &relin_keys,
const GaloisKeys &galois_keys)
{
Ciphertext c1 = encrypted_inputs.at("img");
Ciphertext c9;
evaluator.rotate_rows(c1, 4032, galois_keys, c9);
Ciphertext c10;
evaluator.rotate_rows(c1, 4031, galois_keys, c10);
evaluator.add(c9, c10, c9);
evaluator.rotate_rows(c1, 4033, galois_keys, c10);
Ciphertext c7;
evaluator.rotate_rows(c1, 4095, galois_keys, c7);
evaluator.add(c1, c7, c7);
evaluator.add(c10, c7, c10);
evaluator.rotate_rows(c1, 63, galois_keys, c7);
Ciphertext c6;
evaluator.rotate_rows(c1, 1, galois_keys, c6);
evaluator.add(c7, c6, c7);
evaluator.rotate_rows(c1, 65, galois_keys, c6);
evaluator.rotate_rows(c1, 64, galois_keys, c1);
evaluator.add(c6, c1, c6);
evaluator.add(c7, c6, c7);
evaluator.add(c10, c7, c10);
evaluator.add(c9, c10, c9);
encrypted_outputs.emplace("result", move(c9));
}

vector<int> get_rotation_steps_box_blur(){
return vector<int>{4032, 4031, 1, 4033, 4095, 65, 63, 64};
}
"""

# Use regular expressions to find occurrences of 'add' and 'rotate_rows'
operations=["multiply_plain","multiply","sub","add","rotate_rows","square"]
result=[]
for op in operations :
    nb_occurences= len(re.findall(rf'\b{op}', file_content))
    print(f"{op} count: {nb_occurences}")
