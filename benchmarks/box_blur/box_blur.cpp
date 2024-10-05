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
#include <string>
#include <random>
using namespace std;
using namespace fheco;
void box_blur(size_t width)
{
  vector<vector<integer>> kernel = {{1, 1, 1}, {1, 8, 1}, {1, 1, 1}};
  /*   Ciphertext img("img");
  Ciphertext top_row = img >> width; img(x-1,y)
  Ciphertext bottom_row = img << width;
  Ciphertext top_sum = kernel[0][0] * (top_row >> 1) + kernel[0][1] * top_row + kernel[0][2] * (top_row << 1);
  Ciphertext curr_sum = kernel[1][0] * (img >> 1) + kernel[1][1] * img + kernel[1][2] * (img << 1);
  Ciphertext bottom_sum =
    kernel[2][0] * (bottom_row >> 1) + kernel[2][1] * bottom_row + kernel[2][2] * (bottom_row << 1);
  Ciphertext result = top_sum + curr_sum + bottom_sum;
  result.set_output("result"); 

  Var i("i",0,m_a); 
  Var j("j",0,m_a);
  Var k("j",0,m_a);
  Input A("A",{i,j},Type::ciphertxt);
  Input B("B",{i,j},Type::ciphertxt);
  Computation C("C", {i,j}, A(i,j) + B(i,j));
  C.evaluate(true);
  Computation D("D", {i,j}, C(i,j) - B(i,j));
  D.evaluate(true);
  /********************************************
  Var x("x",0,width); 
  Var y("y",0,width);
  Input img("img",{x,y},Type::ciphertxt);
  Computation C("result",{x,y},img(x-1,y-1)*kernel[0][0]+img(x-1,y)*kernel[0][1]+img(x-1,y+1)*kernel[0][2]
                              +img(x,y-1)*kernel[1][0]+img(x,y)*kernel[1][1]+img(x,y+1)*kernel[1][2]+
                              img(x+1,y-1)*kernel[2][0]+img(x+1,y)*kernel[2][1]+img(x+1,y+1)*kernel[2][2]);
  C.evaluate(true);
  /********************************************/
  int n_rows_image = width ;
  int N = n_rows_image*n_rows_image ;
  //PackedVal kernel = {1,1,1,1,8,1,1,1,1};
  Ciphertext img("img");
  Ciphertext totalRes(PackedVal(1,0));
  for(int x =0 ; x<width ; x++){
    for(int y=0; y<width ; y++){
      vector<integer> used_weight = vector<integer>(N,0);
      for(int i=-1 ; i<2 ; i++){
        for(int j=-1 ; j<2 ; j++){
           int pos = ((x+i)*n_rows_image+(y+j))%N ; 
           if(pos<0){
              pos=pos+N ;
           }
           used_weight[pos]=kernel[i+1][j+1];
           //std::cout<<used_weight[pos]<<" : "<<pos<<"\n";
        }
      }
      //std::cout<<"Next\n";
      Ciphertext res = SumVec(img*used_weight,N);
      vector<integer> mask = vector<integer>(N,0);
      mask[0]=1;
      int step = (x*n_rows_image+y)%N;
      totalRes=totalRes+(res*mask>>step);
      //string info = to_string(x)+":"+to_string(y);
      //res.set_output(info);
    }
  }
  totalRes.set_output("result");
  
}
/***************************************************/

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
  string func_name = "box_blur";
  size_t width = 1;
  size_t height = width;
  /*************************************************************************************************/
  /*************************************************************************************************/
  const auto &func = Compiler::create_func(func_name, width * height, 20, false, true);
  box_blur(width);
  string gen_name = "_gen_he_" + func_name;
  string gen_path = "he/" + gen_name;
  ofstream header_os(gen_path + ".hpp");
  if (!header_os)
    throw logic_error("failed to create header file");

  ofstream source_os(gen_path + ".cpp");
  if (!source_os)
    throw logic_error("failed to create source file");
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
