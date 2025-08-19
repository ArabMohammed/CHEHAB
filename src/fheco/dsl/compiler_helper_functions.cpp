#include "fheco/dsl/compiler_helper_functions.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <optional>
#include <string>
#include <regex>
#include <set>


using namespace std;

namespace fheco
{

void replace_all(string& str, const string& from, const string& to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Move past the replacement
    }
}

queue<string>split(const string &s)
{
  queue<std::string> tokens;
  stringstream ss(s);
  std::string token;
  while (getline(ss, token, ' '))
  {
    tokens.push(token);
  }
  return tokens;
}

std::vector<std::string> process_vectorized_code(const string& content) {
    std::string cleaned_content = content;
    replace_all(cleaned_content, "(", "( ");
    replace_all(cleaned_content, ")", " )");
    replace_all(cleaned_content, "\n", " ");
    /********************************/
    istringstream iss(cleaned_content);
    vector<string> tokens;
    string token;

    while (iss >> token) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool is_literal(const std::string& token) {
    return std::none_of(token.begin(), token.end(), [](unsigned char c) { return std::isalpha(c); });
}

bool verify_all_vec_elems_eq0(const vector<string>& elems){
  bool all_vec_elems_eq0 = true ;
  for(auto elem : elems){
    if(elem!="0"){
      all_vec_elems_eq0=false;
    }
  }
  return all_vec_elems_eq0 ;
}

bool isSingleOperandExpression(const std::string& expression) {
    // Extract the content inside the outer parentheses
    std::string content = expression.substr(4, expression.size()); // Removes "( - " and ")"
    int depth = 0; // Tracks the current depth of parentheses
    int comp = 0 ;
    istringstream iss(content);
    string token;
    vector<string> tokens = {};
    while (iss >> token) {
      tokens.push_back(token);
    }
    for (auto c : tokens) {
        if (c == "(") {
            depth++; // Entering a nested expression
        } else if (c == ")") {
            depth--; // Exiting a nested expression
        }
        if(depth == 0){
          break;
        }
        comp++ ;
    }
    bool is_unary = false ;
    if(tokens[comp+1]==")")
      is_unary = true ;
    return is_unary ;
}

void decompose_vector_op(const vector<string>& vector_elements, vector<string>& vec_ops1 , vector<string>& vec_ops2){
  vector<string> elems = {};
  for (auto elem : vector_elements) {
      vector<string> elems;
      // Check for a "0" element
      if (elem == "0") {
          vec_ops1.emplace_back(elem);
          vec_ops2.emplace_back(elem);
          continue;
      }
      // Remove the first "( + " and last " )"
      elem = elem.substr(3, elem.size() - 5);
      // Stream processing
      istringstream iss(elem);
      string token, nested_expr;
      int nested_level = 0;
      while (iss >> token) {
          if (token == "(") {
              // Start a new nested expression
              nested_expr.clear();
              nested_expr += token;
              nested_level = 1;
              // Collect tokens until the nested level returns to zero
              while (nested_level > 0 && iss >> token) {
                  nested_expr += " " + token;
                  if (token == "(") ++nested_level;
                  else if (token == ")") --nested_level;
              }
              elems.emplace_back(nested_expr);
          } else {
              // Simple operand
              elems.emplace_back(token);
          }
      }
      if (elems.size() >= 2) {
          vec_ops1.emplace_back(elems[0]);
          vec_ops2.emplace_back(elems[1]);
      }else if(elems.size() == 1){
          vec_ops1.emplace_back(elems[0]);
      }
  }
  // Debug output to check results
}

string generate_rotated_expression(string& expression_to_rotate, int number_of_rotations, string operation) {
  string expression_builder = "";
  expression_to_rotate.erase(0, 1);   // remove the first char bcz it s a space
  
  string op = operation == "+" ? "+" : 
            operation == "-" ? "-" : 
            operation == "*" ? "*" : " ";
 
  if (number_of_rotations == 1) {
    expression_builder += " ( " + op + " " + expression_to_rotate + " ( << " + expression_to_rotate + " 1))";
  } else {
    expression_builder += " ( " + op + " " + expression_to_rotate;
    for (int idx = 1 ; idx <= number_of_rotations - 1; idx++) {
      expression_builder += " ( "+ op +" ( << " + expression_to_rotate + " " + std::to_string(idx) + " )";
    }
    expression_builder += " ( << " + expression_to_rotate + " " + std::to_string(number_of_rotations) + " )";
    
    for (int idx = 1 ; idx <= number_of_rotations; idx++) {
      expression_builder += " )";
    }
  }
  return expression_builder;
}

std::vector<std::string> split_string(const std::string& str, char delimiter){
    std::vector<std::string> substrings;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        substrings.push_back(token);
    }
    return substrings;
}

ir::OpCode operationFromString(string operation){
  if (operation == "+")
    return ir::OpCode::add;
  else if (operation == "-")
    return ir::OpCode::sub;
  else if (operation == "*")
    return ir::OpCode::mul;
  else if (operation == "square")
    return ir::OpCode::square;
  else
    throw logic_error("Invalid expression");
}

std::vector<int> split_string_ints(const std::string& str, char delimiter){
    std::vector<int> composingValues;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
      try{
          composingValues.push_back(stoi(token));
      }catch(exception e){
        throw invalid_argument("value :"+token+" cant be converted to int");
      }
    }
    return composingValues;
}

std::string vectorToString(const std::vector<int>& vec){
    std::ostringstream oss;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) {
            oss << " ";  // Add a space before every element except the first
        }
        oss << vec[i];  // Add the element
    }
    
    return oss.str();
}

string constant_folding(queue<string> &tokens){
  //std::cout<<"welcome in constant folding\n";
  while (!tokens.empty())
  {
    //std::cout<<"hereee :"<<tokens.front()<<"\n";
    if (tokens.front() == "(")
    {
      //std::cout<<"here\n";
      tokens.pop();
      string operationString = tokens.front();
      tokens.pop();
      string potential_step = "";
      string operand1="" ,operand2="";
      if (tokens.front() == "(")
      {
        operand1 = constant_folding(tokens);
      }
      else
      {
        //std::cout<<"get op1 \n";
        operand1 = tokens.front();
        tokens.pop();
      }
      if (tokens.front() == "(")
      {
        operand2 = constant_folding(tokens);
        potential_step += " ";

      }
      else if (tokens.front() != ")")
      {
        //std::cout<<"get op2 \n";
        operand2 = tokens.front();
        potential_step = tokens.front();
        tokens.pop();
      }

      // Check for the closing parenthesis
      if (tokens.front() == ")")
      {
        tokens.pop();
      }
      if (potential_step.size() > 0)
      {
        bool is_op1_litteral = is_literal(operand1);
        bool is_op2_litteral = is_literal(operand2);
        if(is_op1_litteral&&is_op2_litteral){
          int op1 = 0;
          int op2 = 0;
          try{
            op1 = stoi(operand1);
            op2 = stoi(operand2); 
            int res = 0 ;
            if(operationString=="+"){
              res = op1+op2 ;
            }else if(operationString=="-"){
              res = op1-op2 ;
            }else if(operationString=="*"){
              res = op1*op2 ;
            }
            string res_op = std::to_string(res);
            return res_op ;
          }catch(exception e){
            throw invalid_argument("value :"+operand1+" or "+operand2+" cant be converted to int");
          }
        }else if(is_op1_litteral){
            int op1 = stoi(operand1);
            int res = 0;
            if(op1==0){
              if(operationString=="+"){
                return operand2;
              }else if(operationString=="-"){
                return "( "+operationString+" "+operand2+" )";
              }else if(operationString=="*"){
                return std::to_string(res);
              }
            }else if (op1==1){
              if(operationString=="*"){
                return operand2;
              }else{
                return "( "+operationString+" "+operand1+" "+operand2+" )" ;
              }
            }else if (op1==-1){
             if(operationString=="*"){
                return "( - "+operand2+" )";
              }else{
                return "( "+operationString+" "+operand1+" "+operand2+" )" ;
              }
            }else{
               return "( "+operationString+" "+operand1+" "+operand2+" )" ;
            }
        }else if (is_op2_litteral){
            //std::cout<<"welcome \n";
            int op2 = stoi(operand2);
            int res = 0;
            if(op2==0){
              if(operationString=="+"){
                return operand1;
              }else if(operationString=="-"){
                return "( "+operationString+" "+operand1+" )";
              }else if(operationString=="*"){
                return std::to_string(res);
              }
            }else if (op2==1){
              if(operationString=="*"){
                return operand1;
              }else{
                return "( "+operationString+" "+operand1+" "+operand2+" )" ;
              }
            }else if (op2==-1){
             if(operationString=="*"){
                return "( - "+operand1+" )";
              }else{
                return "( "+operationString+" "+operand1+" "+operand2+" )" ;
              }
            }
            else{
               return "( "+operationString+" "+operand1+" "+operand2+" )" ;
            }
        }else{
            return "( "+operationString+" "+operand1+" "+operand2+" )" ;
        }
      }else{
        return "( "+operationString+" "+operand1+" )" ;
      }
    }
    else
    {
      return tokens.front();
    }
  }
  throw logic_error("Invalid expression");
}

string convert_new_ops(queue<string> &tokens)
{
  while (!tokens.empty())
  {
    if (tokens.front() == "(")
    {
      tokens.pop();
      string operationString = tokens.front();
      tokens.pop();
      string potential_step = "";
      string operand1="" ,operand2="";
      if (tokens.front() == "(")
      {
        operand1 = convert_new_ops(tokens);
      }
      else
      {
        operand1 = tokens.front();
        tokens.pop();
      }
      if (tokens.front() == "(")
      {
        operand2 = convert_new_ops(tokens);
        potential_step += " ";

      }
      else if (tokens.front() != ")")
      {
        operand2 = tokens.front();
        potential_step = tokens.front();
        tokens.pop();
      }

      // Check for the closing parenthesis
      if (tokens.front() == ")")
      {
        tokens.pop();
      }
      if (potential_step.size() > 0)
      {
        if (operationString=="VecAddRot"){
            return "( + "+operand1+" ( << "+operand1+" "+operand2+" ) )" ;
        }else if (operationString=="VecMinusRot"){
            return "( - "+operand1+" ( << "+operand1+" "+operand2+" ) )" ;
        }else if (operationString=="VecMulRot"){
            return "( * "+operand1+" ( << "+operand1+" "+operand2+" ) )" ;
        }else{
          return "( "+operationString+" "+operand1+" "+operand2+" )" ;
        }
      }else{
        return "( "+operationString+" "+operand1+" )" ;
      }
    }
    else
    {
      return tokens.front();
    }
  }
  throw logic_error("Invalid expression");
}

std::vector<std::string> tokenizeExpression(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string currentToken;

    for (size_t i = 0; i < expression.size(); ++i) {
        char ch = expression[i];

        if (std::isalnum(ch) || ch == '_' || ch == '.') { 
            // Part of a token (alphanumeric or underscore/dot for identifiers like c_234 or 1.23)
            currentToken += ch;
        } else if (std::isspace(ch)) {
            // Space: End of current token (if any)
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            // Special characters like '(', ')', '*', etc.
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
            tokens.emplace_back(1, ch); // Add the special character as a single token
        }
    }

    // Add the last token if any
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}


void generate_final_expression(
  const unordered_map<string,string>& input_entries,
  vector<string>& final_expressions,
  int& final_slot_count,
  std::set<string>& updated_inputs_entries,
  std::unordered_map<string, string>& ciphertext_mapping
){


  for(const auto&pair : input_entries) {
    string label = pair.first;
    string content = pair.second;
    // std::cout << "input_entries.label = " << label << std::endl;
    // std::cout << "input_entries.second = " << content << std::endl;
  }

  std::ifstream vec_expr_file("../vectorized_code.txt");
  if (!vec_expr_file.is_open()) {
      std::cerr << "Error opening file!" << std::endl;
  }

  std::string lines_file, last_line;
  while (std::getline(vec_expr_file, lines_file)) {
      last_line = lines_file; // keep overwriting until EOF
  }

  if (!last_line.empty()) {
      // std::cout << "Last line is: " << last_line << std::endl;
  } else {
      std::cerr << "File is empty!" << std::endl;
  }

  // use the right variable name (tokens instead of to)
  auto to = split(last_line);
  int final_vec_size = std::stoull(to.front());

   std::ifstream input_vectors("fhe_input_vectors.txt");

    if (!input_vectors.is_open()) {
        std::cerr << "Error: Could not open file 'fhe_input_vectors.txt'" << std::endl;
        throw std::runtime_error("File not found or cannot be opened");
    }
    
   unordered_map<string, vector<string>> input_map; // contains the input vectors


    // char delim = ' ';
    string header;
    int nb_inputs, input_vec_size;
    auto tokens = split(header);
     if (getline(input_vectors, header)) {
      auto tokens = split(header); // returns a queue
      if (tokens.size() >= 2) {
        nb_inputs = stoull(tokens.front());
        tokens.pop();
        input_vec_size = stoull(tokens.front());
        tokens.pop();

        // std::cout << "nb_inputs = " << nb_inputs << std::endl;
        // std::cout << "input_vec_size = " << input_vec_size << std::endl;
      }
    }

    string line;

    for (size_t i = 0; i < nb_inputs && getline(input_vectors, line); ++i) {
        auto tokens = split_to_vector(line);
        if (!tokens.empty()) {
            string label = tokens[0];  // e.g. "c0i"
            tokens.erase(tokens.begin());  // remove the label
            input_map[label] = tokens;     // save the rest as the vector
        }
    }

  std::vector<string> plaintext_lines;
  std::unordered_map<std::string, std::string> label_map;
  // all of these are counters to use later
  int plaintext_count = 0;
  int ciphertext_count = 0;
  int partial_ct_count = 0; 


  for(const auto&pair : input_entries) {

    string vector_ciphertexts = pair.second.substr(4);
    // std::cout << "vec ciphertexts after deleting non necessary chars : " << vector_ciphertexts << std::endl;
    bool first_term = true;
    string expression_to_build;
    auto tokens = split(vector_ciphertexts);
    int tokens_type = check_token_types(tokens);
    size_t plaintext_size = tokens.size();
    // std::cout << "plaintext size is : " << plaintext_size << std::endl;

    int token_slot = -1; // this variable to find the slot of the token in the vectors of the extracted expression
    if (tokens_type == 0) { // the vector contains only encrypted values => ciphertext
      
      while (!tokens.empty()) {
      string current = tokens.front();
      token_slot++;
      // std::cout << "current token is : " << current << std::endl;

      // we look for the values on the inputs vectors 
      auto info = find_label_and_index(input_map, current);

      if (info) {
        const string& found_label = info->first;
        int position = info->second;
        
        // std::cout << "→ Found in label: " << found_label << ", at position: " << position << std::endl;
        string label_to_insert;
        label_to_insert = found_label;
      // if (label_map.find(found_label) != label_map.end()) {
      //   // Label already has a mapping
      //     label_to_insert = label_map[found_label];
      // } else {
      //     // Generate new label
      //     label_to_insert = "c" + std::to_string(++ciphertext_count) + "i";
      //     // std::cout << "the created label is : " << label_to_insert << std::endl;
      //     label_map[found_label] = label_to_insert;
      // }
        updated_inputs_entries.insert(label_to_insert);

        string pi = "p0" + std::to_string(plaintext_count);
        updated_inputs_entries.insert(pi);
        ostringstream expr ;

        int rotation_amount = (position - token_slot);

        if (rotation_amount == 0) {
          expr << "( * " << label_to_insert << " " << pi << " )"; 
        } else {
          expr << "( * ( << " << label_to_insert << " " << rotation_amount << " ) " << pi << " )";
        }

        if (first_term) {
            expression_to_build = expr.str();
            plaintext_count++;
            first_term = false;
        } else {
            expression_to_build = "( + " + expression_to_build + " " + expr.str() + " )";
            plaintext_count++;
        }

        /* now , we construct the pi vector , pi vector starts with 0 1 to signify that this
          is a plaintext (0) and signed (1)
          then, we construct its content like : [0, 1, 0 ,0]
          the size of a plaintext is equal to the size of a the vector from the vectorized expression
          this variable exists in the 
        */
      
      // std::cout << "==> expression to build : " << expression_to_build << std::endl;

      std::vector<int> plaintext_vector(plaintext_size + 2, 0);
      plaintext_vector[0] = 0;  // is plaintext not ciphertext
      plaintext_vector[1] = 1;  // is signed

      if (position < plaintext_vector.size()) plaintext_vector[token_slot + 2] = 1;

      std::ostringstream plain_stream;      
      plain_stream << pi;
      for(int val : plaintext_vector) {
        plain_stream << " " << val;
      };

      plaintext_lines.push_back(plain_stream.str());

      } else {
        std::cout << "→ Token not found in any input_map label!" << std::endl;
      }



      tokens.pop();
    }

      // std::cout << "pair.first : " << pair.first << std::endl;
      // std::cout << "Final Expression: " << expression_to_build << std::endl;

      ciphertext_mapping.insert({pair.first, expression_to_build});

    } else if(tokens_type == 2) { // the vector contains only numbers => plaintext
      
      // std::cout << "vector contains only numbers => plaintext" << std::endl;
      updated_inputs_entries.insert(pair.first);
      
    } 
    else {  // the vector contains encrypted values and numbers
      // std::cout << " input vec size is " << input_vec_size << std::endl;
      
      std::vector<int> partial_ciphertext(final_vec_size + 2, 0);

      std::ofstream input_vec_values("fhe_input_vectors_values.txt", std::ios::app);

      if (!input_vec_values) {
        std::cerr << "file fhe_input_vectors_values.txt opening failed" << std::endl;
      }

      while(!tokens.empty()) {

        string current = tokens.front();
        token_slot++;
        // std::cout << "current token is : " << current << " which is " << std::all_of(current.begin(), current.end(), ::isdigit) << std::endl;
        bool is_digit = std::all_of(current.begin(), current.end(), ::isdigit);

        if (!is_digit) {   // encrypted value
          // std::cout << "we manage an encrypted value here !!" << std::endl;
          auto info = find_label_and_index(input_map, current);
          // std::cout << "==> info->first : " << info->first << std::endl;
          // std::cout << "==> info->second : " << info->second << std::endl;
          if (info) {
            const string& found_label = info->first;
            int position = info->second;

            // std::cout << "→ Found in label: " << found_label << ", at position: " << position << std::endl;

            string label_to_insert;

              // Label already has a mapping
            label_to_insert = found_label;
          
                
            updated_inputs_entries.insert(label_to_insert);

            string pi = "p0" + std::to_string(plaintext_count);
            updated_inputs_entries.insert(pi);
            ostringstream expr;

            int rotation_amount = (position - token_slot);

            if (rotation_amount == 0) {
              expr << "( * " << label_to_insert << " " << pi << " )";
            } else {
              expr << "( * ( << " << label_to_insert << " " << rotation_amount << " ) " << pi << " )";
            }

            if (first_term) {
              expression_to_build = expr.str();
              plaintext_count++;
              first_term = false;
            } else {
              expression_to_build = "( + " + expression_to_build + " " + expr.str() + " )";
              plaintext_count++;
            }

            // std::cout << "==> expression to build : " << expression_to_build << std::endl;

            std::vector<int> plaintext_vector(plaintext_size + 2, 0);
            plaintext_vector[0] = 0;  // is plaintext not ciphertext
            plaintext_vector[1] = 1;  // is signed

            if (position < plaintext_vector.size()) plaintext_vector[token_slot + 2] = 1;

            std::ostringstream plain_stream;      
            plain_stream << pi;
            for(int val : plaintext_vector) {
              plain_stream << " " << val;
            };

            plaintext_lines.push_back(plain_stream.str());

          }

        } else {    // number or digit
              if (token_slot >= partial_ciphertext.size()) {
              std::cerr << "ERROR: token_slot " << token_slot 
                << " exceeds partial_ciphertext size "
                << partial_ciphertext.size() << std::endl;
            }
            partial_ciphertext[token_slot + 2] = std::stoull(current);

            // partial_ciphertext[token_slot + 2] = std::stoull(current);
        }
        tokens.pop();
      }

      // create a new ciphertext for the vector with only digits
      partial_ciphertext[0] = 1;
      partial_ciphertext[1] = 1;

      string ci = "c00" + std::to_string(partial_ct_count++) + "i";
      updated_inputs_entries.insert(ci);
      nb_inputs++;

      std::ostringstream cipher_stream;
      cipher_stream << ci;
      for(int val : partial_ciphertext) {
        cipher_stream << " " << val;
      }
      
      input_vec_values << cipher_stream.str();
      input_vec_values << "\n";

      expression_to_build = "( + " + expression_to_build + " " + ci + " )";

      ciphertext_mapping.insert({pair.first, expression_to_build});

    }

    
  }



    // after collecting all plaintexts required to conrscturct the vecotors , we put them in the file
    // std::cout << "Plaintext File Content:\n";
    // for(const auto&line : plaintext_lines) {
    //   std::cout << line << std::endl;
    // }


    // now we update the file with the ciphertexts inputs and the new plaintexts
    // we read the file fhe_io_example_adapted.txt and we modify it
    int slot_count = 0;
    prepare_fhe_file(
      nb_inputs,
      input_vec_size,
      input_map,
      plaintext_lines,
      slot_count
    );

    final_slot_count = slot_count;

  return;
}

void prepare_fhe_file(
  int nb_inputs, 
  int input_vec_size, 
  const unordered_map<string, vector<string>> input_map,
  vector<string> plaintext_lines,
  int& final_slot_count
) {
  std::ifstream infile("fhe_io_example_adapted.txt");
  vector<string> lines;
  string line;

  while(std::getline(infile, line)) {
    lines.push_back(line);
    // std::cout << "lines of fhe_io_example_adapted.txt are : " << line << std::endl;
  }
  infile.close();

  if (lines.empty()){
    std::cerr << "fhe_io_example_adapted.txt is empty !!";
    return;
  }

  string header = lines.front();
  auto tokens = split(header);

  if (tokens.size() < 3) {
    std::cerr << "Header line is malformed!" << std::endl;
    return;
  }

  /*these calues represent stats of the new vectorized expression extracted from e-graph*/
  int new_input_size = stoull(tokens.front());
  tokens.pop();
  int new_input_number = stoull(tokens.front());
  tokens.pop();
  int new_output_number = stoull(tokens.front());
  tokens.pop();


  //output line
  string output_lines;
  for(int i = 0 ; i < new_output_number ; i++) {
      output_lines = lines.back() + "\n";
  }

  // contructing the plaintext string , these plaintexts are generated by e-graph and not in the step of constructing vectors from the initial ones
  string plaintext_string;
  int prev_plaintxt_count = 0;
  for (const auto& line : lines) {
      if (!line.empty() && line[0] == 'p') {
          plaintext_string += line + "\n";
          prev_plaintxt_count++;
      }
    }

  // update the header
  int new_header_vector_size = std::max(input_vec_size, new_input_size);
  int new_header_input_count = nb_inputs + plaintext_lines.size() + prev_plaintxt_count;
  int new_header_output_count = new_output_number;

  std::ostringstream oss;
  oss << new_header_vector_size << " "
      << new_header_input_count << " "
      << new_header_output_count;
  string new_header = oss.str();

  

  // std::cout << "plaintext string : " << plaintext_string << std::endl;

  /* now , we read the file that contains the values of the input vectors */
  std::ifstream another_infile("fhe_input_vectors_values.txt");
   if (!another_infile) {
        std::cerr << "file fhe_input_vectors_values.txt opening failed" << std::endl;
      }
  vector<string> vectors_values;
  string vec_values;
  int counter = 0;

  while(std::getline(another_infile, vec_values)) {
    auto tokens = split(vec_values);
    
    // skip the first 3 tokens
    vector<string> first_three_tokens;
    for(int i = 0; i < 3 && !tokens.empty(); i++) {
      // if (i == 0) {
      //   // Replace first token with c{counter}i
      //   first_three_tokens.push_back("c" + to_string(counter) + "i");
      // } else {
      //   first_three_tokens.push_back(tokens.front());
      // }
      first_three_tokens.push_back(tokens.front());
      tokens.pop();
    }
    ++counter;
    // Now, we collect the remaining tokens
    vector<string> vector_values;
    while(!tokens.empty()) {
      vector_values.push_back(tokens.front());
      tokens.pop();
    }

    // Pad with zeros if needed
    while(vector_values.size() < new_header_vector_size) {
      vector_values.push_back("0");
    }

    // Reconstruct the line
    string updated_line;
    for(const auto& token : first_three_tokens) {
      updated_line += token + " ";
    }

    for (int i = 0 ; i < vector_values.size() ; i++) {
      updated_line += vector_values[i];
      if (i != vector_values.size() - 1) updated_line += " ";
    }
    
    vectors_values.push_back(updated_line);
  }


  // after collecting the required data , we write them in the file

  std::ofstream outfile("fhe_io_example_adapted.txt");
  if (!outfile.is_open())  {
    std::cerr << "Errr opening file fhe_io_example_adapted.txt" << std::endl;
    return;
  }

  outfile << new_header << "\n";

  for(const auto&line : vectors_values) {
    outfile << line << "\n";
  }

  for (const auto& line : plaintext_lines) {
      outfile << line << "\n";
  }

  outfile << plaintext_string;
  outfile << output_lines << "\n";
    

  outfile.close();
  final_slot_count = new_header_vector_size;
}

bool is_token_literal(const string& token) {
    // A literal like v1_0, x2_3: must start with a letter, then digits/underscores
    static const regex literal_regex("^[a-zA-Z][a-zA-Z0-9_]*$");
    return regex_match(token, literal_regex);
}
int check_token_types(queue<string> tokens) {
    bool has_literal = false;
    bool has_number = false;

    while (!tokens.empty()) {
        string token = tokens.front();
        tokens.pop();

        // Check if token is a number
        bool is_number = !token.empty() &&
                         all_of(token.begin(), token.end(), [](char c) {
                             return isdigit(c) || c == '.' || c == '-';
                         }) &&
                         (isdigit(token.back()) || token.back() == '.');

        if (is_number) {
            has_number = true;
        } else if (is_token_literal(token)) {
            has_literal = true;
        } else {
            // If it's neither a number nor a valid literal, treat as literal
            has_literal = true;
        }
    }

    if (has_literal && has_number) return 1; // mixed
    if (has_number) return 2;                // only numbers
    return 0;                                // only literals
}

vector<string> split_to_vector(const string& line) {
    vector<string> result;
    stringstream ss(line);
    string token;
    while (ss >> token) {
        result.push_back(token);
    }
    return result;
}

std::optional<std::pair<std::string, size_t>> find_label_and_index(
    const std::unordered_map<std::string, std::vector<std::string>>& input_map,
    const std::string& token)
{
    for (const auto& [label, vec] : input_map) {
        auto it = std::find(vec.begin(), vec.end(), token);
        if (it != vec.end()) {
            size_t index = std::distance(vec.begin(), it);
            return std::make_pair(label, index);
        }
    }
    return std::nullopt;
}

std::string replace_variable(
    const std::string& expr,
    const std::unordered_map<std::string, std::string>& mapping)
{
    std::string result = expr;

    std::regex var_regex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)");
    std::smatch match;

    auto it = result.cbegin();
    std::string output;

    while (std::regex_search(it, result.cend(), match, var_regex)) {
        output.append(it, match[0].first);  // text before match

        std::string var = match[0];

        // Check if it starts with 'c' and exists in mapping
        if (!var.empty() && var[0] == 'c' && mapping.find(var) != mapping.end()) {
            output += mapping.at(var);
        } else {
            output += var;  // leave it as is
        }

        it = match[0].second;
    }

    output.append(it, result.cend());
    return output;
}


} // namespace fheco
