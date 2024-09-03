#pragma once

#include <string>
#include <vector>
#include "fheco/dsl/var.hpp"
#include "fheco/dsl/ciphertext.hpp"
#include "fheco/dsl/expression.hpp"
#include <unordered_map>

namespace fheco
{
    class Var;
    class Ciphertext;
    class Expression;
    
    class Computation 
    {
        public:
            /* Construct a new computation */
            explicit Computation(const std::string &name, const std::vector<Var> iterator_variables, const Expression &expression);

            /* Overloaded constructor with Type parameter */
            explicit Computation(const std::string &name, const std::vector<Var> iterator_variables, Type type);

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

            void evaluate(bool is_output);
            // Evaluate this computation to get corresponding ciphertext  

        private:
            Expression& apply_operator(const std::vector<Var> &compute_args) const;
            /*****************************************************************/
            std::vector<Ciphertext> evaluate_expression(Expression &expression);

            /** Name of the computation **/
            std::string name_;
            
            // Variables used to evaluate the expression  
            std::vector<Var> iterator_variables_;
            
            // Expression that will be evaluated by this computation 
            Expression expression_;
    };
}
