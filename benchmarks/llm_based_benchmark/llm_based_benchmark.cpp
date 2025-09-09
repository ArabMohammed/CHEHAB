#include "fheco/fheco.hpp"

using namespace std;
using namespace fheco;
#include <chrono>
#include <fstream>
#include <iostream> 
#include <string>
#include <vector>
#include <utility>
#include <limits>
#include <set>
#include <algorithm>
#include <cctype>
#include <random>
#include "../global_variables.hpp" 

#define MAX_NUM_LINES 15854
#define OUTPUT_FILE "fhe_io_example.txt"

/*******************/
std::string clean_expression_line(const std::string& line) {
    std::string result = line;

    // Remove prefix "(Vec "
    const std::string prefix = "(Vec ";
    if (result.rfind(prefix, 0) == 0) {
        result = result.substr(prefix.size());
    }

    // Remove last ')'
    if (!result.empty() && result.back() == ')') {
        result.pop_back();
    }

    return result;
}
/********************/
int find_vector_size(std::string& expression) {
   int open_parens = 0;
    int count = 0;

    for (char ch : expression) {
        if (ch == '(') {
            open_parens++;
        } else if (ch == ')') {
            open_parens--;
            if (open_parens == 0) {
                count++;
            }
        }
    }

    return count;
}
/*******************/
std::queue<std::string> tokenize(const std::string& expr) {
    std::queue<std::string> tokens;
    std::string token;
    for (char ch : expr) {
        if (ch == '(' || ch == ')') {
            if (!token.empty()) {
                tokens.push(token);
                token.clear();
            }
            tokens.push(std::string(1, ch));
        } else if (isspace(ch)) {
            if (!token.empty()) {
                tokens.push(token);
                token.clear();
            }
        } else {
            token.push_back(ch);
        }
    }
    if (!token.empty()) {
        tokens.push(token);
    }
    return tokens;
}
/********************/
Ciphertext parseExpression(std::queue<std::string>& tokens) {
    if (tokens.empty()) {
      throw std::runtime_error("Unexpected end of tokens");
    };

    std::string token = tokens.front();
    tokens.pop();

    if (token == "(") {
        std::string op = tokens.front(); // "+" or "*"
        tokens.pop();

        Ciphertext left = parseExpression(tokens);
        Ciphertext right = parseExpression(tokens);

        if (!tokens.empty() && tokens.front() == ")") {
            tokens.pop(); // consume ")"
        }

        if (op == "+") {
          return left + right;
        } else if(op == "-") {
          return left - right;
        }
        else if (op == "*") {
          return left * right;
        } else {
            throw std::runtime_error("Unknown operator: " + op);
        }
    } 
    else if (token == ")") {
      throw std::runtime_error("Unexpected )");
    } 
    else {
        return Ciphertext(token);
    }
}
/*******************/
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return ""; // all spaces
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}
/******************/
std::vector<std::string> split_expressions(const std::string& line) {
    std::vector<std::string> expressions;
    int open = 0;
    std::string current;

    for (char ch : line) {
        if (ch == '(') {
            open++;
        }
        if (ch == ')') {
            open--;
        }

        current.push_back(ch);

        // if we finished one full balanced expression
        if (open == 0 && !current.empty()) {
            std::string cleaned = trim(current);
            if (!cleaned.empty()) {
                expressions.push_back(cleaned);
            }
            current.clear();
        }
    }

    return expressions;
}
/**************************/
std::pair<std::string, int> read_expressions_from_file(int target_line) {
    std::ifstream infile("llm_dataset_cleaned.txt");
    if (!infile.is_open()) {
        std::cerr << "Error: could not open the file!" << std::endl;
        return {"", -1};
    }

    std::string line;
    int current_line = 0;
  
    while (std::getline(infile, line)) {
        if (current_line == target_line) {
            std::string cleaned = clean_expression_line(line);
            int slot_count = find_vector_size(cleaned);
            return {cleaned, slot_count};
        }
        current_line++;
    }

    return {"", -1};
}

/************************************/
void fhe(int target_line_number)
{
  
  auto [expression , vec_size] = read_expressions_from_file(target_line_number);

  if (vec_size <= 0) {
        std::cerr << "Invalid or empty expression at line " << target_line_number << "\n";
        return;
  }

  std::vector<std::string> splitted_expr = split_expressions(expression);

  std::vector<Ciphertext> output(vec_size);

  for (int i = 0 ; i < vec_size ; i++) {
    std::string expr = splitted_expr[i];

    #ifdef DEBUG_MODE
    std::cout << "string expr for iteration : " << i << " is : " << expr << std::endl;
    #endif

    auto tokens = tokenize(expr);
    output[i] = parseExpression(tokens);
    output[i].set_output("output_" + std::to_string(i));
  }

  
}
/*********************************************************/
std::set<std::string> extract_input_variables(const std::string& expression_line) {
    std::set<std::string> inputs;
    std::string current_token;

    for (char ch : expression_line) {
        // Corrected: Include '_' in characters that form a token
        if (isalnum(ch) || ch == '_') { // <--- MODIFIED LINE
            current_token += ch;
        } else {
            // Logic for valid variable names (not operators, "Vec", or numbers)
            if (!current_token.empty() &&
                current_token != "+" && current_token != "-" && current_token != "*" &&
                current_token != "Vec" &&
                !(isdigit(current_token[0]) || (current_token.length() > 1 && current_token[0] == '-' && isdigit(current_token[1]))) // Handles negative numbers
               ) {
                inputs.insert(current_token);
            }
            current_token.clear();
        }
    }
    // Check for the last token (after the loop finishes)
    if (!current_token.empty() &&
        current_token != "+" && current_token != "-" && current_token != "*" &&
        current_token != "Vec" &&
        !(isdigit(current_token[0]) || (current_token.length() > 1 && current_token[0] == '-' && isdigit(current_token[1])))
       ) {
        inputs.insert(current_token);
    }

    return inputs;
}
/******************************************/
int generate_fhe_io_file(int target_line_number) {
    auto [expression_full, vec_size] = read_expressions_from_file(target_line_number);

    if (vec_size <= 0) {
        std::cerr << "Invalid or empty expression at line " << target_line_number << ". Cannot generate " << OUTPUT_FILE << "\n";
        return 1;
    }

    ofstream io_file(OUTPUT_FILE);
    if (!io_file.is_open()) {
        cerr << "Error: could not open " << OUTPUT_FILE << " for writing!" << endl;
        return 1;
    }

    std::set<std::string> input_variables = extract_input_variables(expression_full);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 10000);

    // First line: 1 number_of_inputs number_of_outputs
    io_file << "1 " << input_variables.size() << " " << vec_size << endl;

    // Input lines: input_name 1 random_value
    for (const std::string& input_name : input_variables) {
        io_file << input_name << " 1 1 " << distrib(gen) << endl;
    }

    for (int i = 0; i < vec_size; ++i) {
        io_file << "output_" << i << " 1 1" << endl; // Assuming output_i as name for outputs
    }

    io_file.close();
    cout << "Generated " << OUTPUT_FILE << " successfully." << endl;
    return 0;
}

/***************************************************************/
void print_bool_arg(bool arg, const string &name, ostream &os)
{
  os << (arg ? name : "no_" + name);
}

int main(int argc, char **argv)
{

  bool call_quantifier = true;
  if (argc > 1)
    call_quantifier = stoi(argv[1]);
  
  bool cse = true;
  if (argc > 2)
    cse = stoi(argv[2]);

  bool const_folding = true;
  if (argc > 3)
    const_folding = stoi(argv[3]); 
  
  int tgt_line = 0 ;
  if (argc > 4)
    tgt_line = stoi(argv[4]);

  if (tgt_line > MAX_NUM_LINES) {
    tgt_line = MAX_NUM_LINES;
  }


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


  //Compiler::enable_auto_enc_params_selection();
  chrono::high_resolution_clock::time_point t;
  chrono::duration<double, milli> elapsed;
  string func_name = "fhe";
  if (generate_fhe_io_file(tgt_line) != 0) {
      return 1; // Exit if file generation failed
  }
  /**************/t = chrono::high_resolution_clock::now();
  const auto &func = Compiler::create_func(func_name, 1, 20, false, true);

  fhe(tgt_line);

  string gen_name = "_gen_he_" + func_name;
  string gen_path = "he/" + gen_name;
  ofstream header_os(gen_path + ".hpp");
  if (!header_os)
    throw logic_error("failed to create header file");
  ofstream source_os(gen_path + ".cpp");
  if (!source_os)
    throw logic_error("failed to create source file");
  Compiler::gen_vectorized_code(func);
  auto ruleset = Compiler::Ruleset::depth;
  auto rewrite_heuristic = trs::RewriteHeuristic::bottom_up;
  Compiler::compile(func, ruleset, rewrite_heuristic, header_os, gen_name + ".hpp", source_os);
  Compiler::gen_he_code(func, header_os, gen_name + ".hpp", source_os);
  /************/elapsed = chrono::high_resolution_clock::now() - t;
  cout<<"Compile time : \n";
  cout << elapsed.count() << " ms\n";
  if (call_quantifier)
  {
    util::Quantifier quantifier{func};
    quantifier.run_all_analysis();
    quantifier.print_info(cout);
  }
  
  return 0;
}
/****************************************************************/