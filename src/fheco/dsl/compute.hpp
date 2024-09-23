#pragma once

#include <string>
#include <vector>
#include "fheco/dsl/var.hpp"
#include "fheco/dsl/ciphertext.hpp"
#include "fheco/dsl/plaintext.hpp"
#include "fheco/dsl/expression.hpp"
#include "fheco/dsl/tensor.hpp"
#include <unordered_map>

namespace fheco
{
    class Var;
    class Ciphertext ;
    class Plaintext ;
    class Expression;
    template <typename T>
    class DynamicTensor ;
    class Computation 
    {
        public:
            /* Construct a new computation thaat support only ciphertext type */
            explicit Computation(const std::string &name, const std::vector<Var> &iterator_variables,const Expression &expression);
            
            /* Construct a new coputation that support vectorciphertext type*/
            explicit Computation(const std::string &name, const std::vector<Var> &iterator_variables,const std::vector<Var> &output_dim_variables ,const Expression &expression);

            /* Overloaded constructor with Type parameter */
            //explicit Computation(const std::string &name, const std::vector<Var> iterator_variables, Type type);

            //explicit Computation(const std::string &name, const std::vector<Var> iterator_variables, std::vector<Var> output_dim_variables, Type type);

            /** Cast a computation to an expression **/
            Expression expression() const {
                return expression_;
            }

            // Copy constructor 
            Computation(const Computation &other);

            // Move constructor
            Computation(Computation &&other) noexcept ;

            // Destructor 
            ~Computation() = default;
            
            // Move assignment operator 
            Computation &operator=(Computation &&other) noexcept;
            
            // Copy assignment operator
            Computation &operator=(const Computation &other);
            /*****************************************************/       
            template<typename... Args>

            Expression& operator()(Args&&...  args) const {
                std::vector<Var> arg_list = {std::forward<Args>(args)...};
                return this->apply_operator(arg_list);
            }
            /*****************************************************/
            std::string name() const {
                return name_;
            }

            std::vector<Var> iterator_variables() const {
                return iterator_variables_;
            }

            void set_expression(const Expression &expression) {
                expression_ = expression;
            }
            // Evaluate this computation to get corresponding ciphertext  
            void evaluate(bool is_output);

            bool is_reduction(){
                return is_reduction_ ;
            }
            
            //void resize(std::vector<Var> new_dimensions_vars);
        
        private:
            Expression& apply_operator(const std::vector<Var> &compute_args) const;
            /*****************************************************************/
            DynamicTensor<Ciphertext> evaluate_expression(Expression &expression);
            /** Name of the computation **/
            std::string name_;
            /***Set to true if we have a reduction operation ***/
            bool is_reduction_ = false;
            // Variables used to evaluate the expression  
            std::vector<Var> iterator_variables_;
            std::vector<Var> output_dim_variables_;
            // Expression that will be evaluated by this computation 
            Expression expression_;
    };
}
