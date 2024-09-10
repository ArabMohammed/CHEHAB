#include "fheco/fheco.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace fheco;

void roberts_cross(size_t width)
{
  vector<vector<integer>> gx_kernel = {{1, 0}, {0, -1}};
  vector<vector<integer>> gy_kernel = {{0, 1}, {-1, 0}};                                                          
  Var i("i",0,width);
  Var j("j",0,width);
  Input img("img",{i,j},Type::ciphertxt);
  Computation gx_result("R1",{i,j},gx_kernel[0][0]*img(i,j)+gx_kernel[0][1]*img(i,j+1)
                              +gx_kernel[1][0]*img(i+1,j)+gx_kernel[1][1]*img(i+1,j+1));
  gx_result.evaluate(false);
  
  Computation gy_result("R2",{i,j},gy_kernel[0][0]*img(i,j)+gy_kernel[0][1]*img(i,j+1)
                               +gy_kernel[1][0]*img(i+1,j)+gy_kernel[1][1]*img(i+1,j+1));
  gy_result.evaluate(false);
  Computation result("result",{i,j},gx_result(i,j)*gx_result(i,j) + gy_result(i,j)*gy_result(i,j));
  result.evaluate(true);
  /******************************************** */
 /*  Ciphertext img("img");
  Ciphertext bottom_row = img << width; //img >> width; img(x-1,y)
  // gx
  Ciphertext gx_curr_sum = gx_kernel[0][0] * img + gx_kernel[0][1] * (img << 1);
  Ciphertext gx_bottom_sum = gx_kernel[1][0] * bottom_row + gx_kernel[1][1] * (bottom_row << 1);
  Ciphertext gx_result = gx_curr_sum + gx_bottom_sum;
  // gy
  Ciphertext gy_curr_sum = gy_kernel[0][0] * img + gy_kernel[0][1] * (img << 1);
  Ciphertext gy_bottom_sum = gy_kernel[1][0] * bottom_row + gy_kernel[1][1] * (bottom_row << 1);
  Ciphertext gy_result = gy_curr_sum + gy_bottom_sum;
  // combine
  Ciphertext result = gx_result * gx_result + gy_result * gy_result;
  result.set_output("result"); */
}

void print_bool_arg(bool arg, const string &name, ostream &os)
{
  os << (arg ? name : "no_" + name);
}

int main(int argc, char **argv)
{
  auto axiomatic = false;
  if (argc > 1)
    axiomatic = stoi(argv[1]) ? true : false;

  auto window = 0;
  if (argc > 2)
    window = stoi(argv[2]);

  bool call_quantifier = false;
  if (argc > 3)
    call_quantifier = stoi(argv[3]);

  bool cse = true;
  if (argc > 4)
    cse = stoi(argv[4]);

  bool const_folding = true;
  if (argc > 5)
    const_folding = stoi(argv[5]);

  print_bool_arg(call_quantifier, "quantifier", clog);
  clog << " ";
  print_bool_arg(cse, "cse", clog);
  clog << " ";
  print_bool_arg(const_folding, "constant_folding", clog);
  clog << '\n';

  if (cse)
  {
    Compiler::enable_cse();
    Compiler::enable_order_operands();
  }
  else
  {
    Compiler::disable_cse();
    Compiler::disable_order_operands();
  }

  if (const_folding)
    Compiler::enable_const_folding();
  else
    Compiler::disable_const_folding();

  chrono::high_resolution_clock::time_point t;
  chrono::duration<double, milli> elapsed;
  t = chrono::high_resolution_clock::now();
  string func_name = "roberts_cross";
  size_t width = 64;
  size_t height = 64;
  const auto &func = Compiler::create_func(func_name, width * height, 20, true, true);
  roberts_cross(width);
  /*********/   Compiler::print_func_ir_file(func ,"../ir_robert_cross.txt");
  string gen_name = "_gen_he_" + func_name;
  string gen_path = "he/" + gen_name;
  ofstream header_os(gen_path + ".hpp");
  if (!header_os)
    throw logic_error("failed to create header file");

  ofstream source_os(gen_path + ".cpp");
  if (!source_os)
    throw logic_error("failed to create source file");
  /* auto ruleset = Compiler::Ruleset::joined;
  if (argc > 2)
    ruleset = static_cast<Compiler::Ruleset>(stoi(argv[2]));
  auto rewrite_heuristic = trs::RewriteHeuristic::bottom_up;
  if (argc > 3)
    rewrite_heuristic = static_cast<trs::RewriteHeuristic>(stoi(argv[3]));
  Compiler::compile(func, ruleset, rewrite_heuristic, header_os, gen_name + ".hpp", source_os, true);  */
  ///////////////////////////// 
  Compiler::compile(func, header_os, gen_name + ".hpp", source_os, axiomatic, window);
  //Compiler::gen_he_code(func, header_os, gen_name + ".hpp", source_os);
  elapsed = chrono::high_resolution_clock::now() - t;
  cout << elapsed.count() << " ms\n";

  if (call_quantifier)
  {
    util::Quantifier quantifier{func};
    quantifier.run_all_analysis();
    quantifier.print_info(cout);
  }
  return 0;
}
