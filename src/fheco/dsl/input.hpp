#pragma once

#include <algorithm>
#include <stdexcept>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/expression.hpp"
#include "fheco/dsl/var.hpp"
#include "fheco/dsl/ciphertext.hpp"
#include "fheco/dsl/tensor.hpp"
namespace fheco
{
    class Var ; 
    class Ciphertext ;
    class Expression ;
    template <typename T>
    class DynamicTensor ;
    class Input
    {
        public:
            /*Construct a new Input and specifying it name in the input io_file and it type 
            whether it ciphertext or ciphertext[] , and also the iterator_variables to loop 
            over it*/
            explicit Input(const std::string &name , const std::vector<Var> iterator_variables, const Type type);
            // Implicit conversion operator to Expr
           /*  operator Expression() {
                return expression_;
            } */
            // Copy constructor
            Input(const Input &other);
            // Move constructor
            Input(Input &&other) noexcept;
            // desctructor 
            ~Input()= default ;
        
            const std::string &name() const {return name_ ; }     
            /*****************************************************/       
            template<typename... Args>

            Expression& operator()(Args&&...  args) const {
                std::vector<Var> arg_list = {std::forward<Args>(args)...};
                return this->apply_operator(arg_list);
            }
            /*****************************************************/
            
        private : 
            Expression& apply_operator(const std::vector<Var> &compute_args) const;
            std::string name_ ; 
            std::vector<Var> iterator_variables_ ;
            Expression expression_ ;
            Type type_ ; 
            DynamicTensor<Ciphertext> ciphertexts_ ;
            DynamicTensor<Plaintext> plaintexts_ ;
    };
}