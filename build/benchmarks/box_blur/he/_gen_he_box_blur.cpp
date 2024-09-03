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
size_t slot_count = encoder.slot_count();
Plaintext p10;
encoder.encode(vector<std::uint64_t>(slot_count, 1), p10);
Ciphertext c9;
evaluator.rotate_rows(c1, 4031, galois_keys, c9);
evaluator.multiply_plain(c9, p10, c9);
Ciphertext c8;
evaluator.rotate_rows(c1, 4032, galois_keys, c8);
evaluator.multiply_plain(c8, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.rotate_rows(c1, 4033, galois_keys, c8);
evaluator.multiply_plain(c8, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.rotate_rows(c1, 4095, galois_keys, c8);
evaluator.multiply_plain(c8, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.multiply_plain(c1, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.rotate_rows(c1, 1, galois_keys, c8);
evaluator.multiply_plain(c8, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.rotate_rows(c1, 63, galois_keys, c8);
evaluator.multiply_plain(c8, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.rotate_rows(c1, 64, galois_keys, c8);
evaluator.multiply_plain(c8, p10, c8);
evaluator.add(c9, c8, c9);
evaluator.rotate_rows(c1, 65, galois_keys, c1);
evaluator.multiply_plain(c1, p10, c1);
evaluator.add(c9, c1, c9);
encrypted_outputs.emplace("result", move(c9));
}

vector<int> get_rotation_steps_box_blur(){
return vector<int>{4031, 1, 4032, 4033, 4095, 65, 63, 64};
}
