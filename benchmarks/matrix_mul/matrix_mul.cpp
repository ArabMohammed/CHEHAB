#include "fheco/fheco.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;
using namespace fheco;

/* Ciphertext reduce_add_lin(const Ciphertext &x, int vector_size)
{
  Ciphertext result = x;
 /*  for (int i = vector_size -1 ; i > 0; --i)
  {
    result += x <<i;
  } 
  for (int i = 1 ; i < vector_size; ++i)
  {
    result += x <<i;
  }
  return result;
} */

void print_bool_arg(bool arg, const string &name, ostream &os)
{
  os << (arg ? name : "no_" + name);
}
/*****************************************************/
vector<string> split(const string &str, char delim)
{
  vector<string> tokens;
  string token = "";
  for (const auto &c : str)
  {
    if (c == delim)
    {
      tokens.push_back(token);
      token = "";
    }
    else
      token += c;
  }
  tokens.push_back(token);
  return tokens;
}
/******************************/
vector<integer> load(istream &is, char delim)
{
  vector<integer> data;
  string line;
  while (getline(is, line))
  {
    auto tokens = split(line, delim);
    vector<integer> line_data;
    line_data.reserve(tokens.size());
    for (const auto &token : tokens)
       data.push_back(static_cast<integer>(stoull(token)));
  }
  return data;
}
/*****************************************************/
void matrix_mul(int m_a, int n_b, int n_a_m_b)
{
  
  // encrypt by li*ne for matrix A
 /****************************** 
 vector<Ciphertext> A_row_encrypted ;
  vector<Ciphertext> B_column_encrypted ;
  for (int i = 0; i < m_a; ++i)
  {
    //
    Ciphertext line("A[" + to_string(i) + "]");
    A_row_encrypted.push_back(line);
  }
  // encrypt by column for matrix B
  for (int i = 0; i < n_b; ++i)
  {
    Ciphertext column("B[" + to_string(i) + "]");
    B_column_encrypted.push_back(column);
  }
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
  vector<Ciphertext> C_row_encrypted;
  for (size_t i = 0; i < A_row_encrypted.size(); ++i)
  {
    //Ciphertext cline;
    for (size_t j = 0; j < B_column_encrypted.size(); ++j)
    {
      //vector<int64_t> mask(A_row_encrypted.size(), 0);
      //mask[j] = 1;
      Ciphertext slot; 
      ///************************************************ 
      slot = SumVec(A_row_encrypted[i] * B_column_encrypted[j],n_b);
      ///***********************************************************
      //if (j == 0)                                                               
      //  cline = slot * mask;
      //else
      //  cline += slot * mask;
      slot.set_output("C[" + to_string(i) + "]["+ to_string(j) +"]");
    } 
    //cline.set_output("C[" + to_string(i) + "][]");
    //C_row_encrypted.push_back(cline);
  }  

  /******************
  ifstream w1_is("w1.txt");
  if (!w1_is)
    throw invalid_argument("failed to open w1 file");
  char delim = ' ';
  auto w1_raw = load(w1_is, delim); 

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
  /****************************************
  Var k_val("k_val",0,64);
  Var k_val1("k_val1",0,8);
  vector<integer> info = vector<integer>(64,1);
  Input test({k_val1,k_val1},Type::plaintxt,info);
  Input cipher("cipher",{k_val},Type::vectorciphertxt);
  std::cout<<"resize ciphertext \n";
  cipher.resize({k_val1,k_val1});
  std::cout<<"resize following ciphertext \n";
  cipher.resize({k_val1,j_kernels,i_out});
  std::cout<<"resize plaintext \n";
  test.resize({k_val1});
  /****************************************
  vector<integer> initial_inputs = w1_raw ;
  Input padded_in ("padded_in",{i_in,j_in,k_in},Type::vectorciphertxt);
  Input kernels ({i_kernels,j_kernels,k_kernels,k_out},Type::plaintxt,initial_inputs);
  Computation res("res",{i_out,j_out,k_out,i_kernels,k_kernels,j_kernels},{i_out,j_out,k_out},padded_in(i_kernels + i_out*row_stride , j_kernels + j_out*col_stride , k_kernels)*kernels(i_kernels , j_kernels , k_kernels , k_out));
  res.evaluate(true);*/
  
  /*****************************************************/
  /*  Var i("i",0,m_a); 
  Var j("j",0,m_a);
  Var k("k",0,m_a);
  Input A("A",{i,j,k},Type::vectorciphertxt);
  Input B("B",{i,j,k},Type::vectorciphertxt);
  Computation C("C", {i,j,k},{i,j,k},A(i,j,k)*B(i,j-2,k)); 
  C.evaluate(true); */
  /*****************************************************/
  /*   Var i("i",0,m_a); 
  Var j("j",0,m_a);
  Var k("k",0,m_a);
  Input A("A",{i,k},Type::vectorciphertxt);
  Input B("B",{j,k},Type::vectorciphertxt);
  Computation C("C", {i,j,k},{i,j},A(i,k)*B(j,k)); 
  C.evaluate(true); */
  /*****************************************************/
  Var i("i",0,m_a); 
  Var j("j",0,m_a);
  Var k("k",0,m_a);
  Input B("tensor",{i,j,k},Type::vectorciphertxt);
  Computation C("output", {i,j,k},{i,j},B(i,j,k)); 
  C.evaluate(true); 
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
  bool cse = false;
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
  string func_name = "matrix_mul" ;
  /******************************/
  size_t slot_count = 8;
  int m_a = slot_count ;
  int n_a_m_b = slot_count ;
  int n_b = slot_count ;
  /*****************************/
  Compiler::enable_scalar_vector_shape();
  const auto &func = Compiler::create_func(func_name, slot_count, 20, true, true);
  matrix_mul(m_a, n_b, n_a_m_b);
  string gen_name = "_gen_he_" + func_name;
  string gen_path = "he/" + gen_name;
  ofstream header_os(gen_path + ".hpp");
  if (!header_os)
    throw logic_error("failed to create header file");

  ofstream source_os(gen_path + ".cpp");
  if (!source_os)                         
    throw logic_error("failed to create source file");
  std::cout<<"\n ==>Print ir representation \n";
  Compiler::print_func_ir_file(func ,"../ir_matrix_mul.txt");
  /////////////////////
  /*auto ruleset = Compiler::Ruleset::joined;
  if (argc > 2)
    ruleset = static_cast<Compiler::Ruleset>(stoi(argv[2]));
  auto rewrite_heuristic = trs::RewriteHeuristic::bottom_up;
  if (argc > 3)
    rewrite_heuristic = static_cast<trs::RewriteHeuristic>(stoi(argv[3]));
  Compiler::compile(func, ruleset, rewrite_heuristic, header_os, gen_name + ".hpp", source_os, true);*/
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
