#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/var.hpp"
#include "fheco/dsl/ciphertext.hpp"

namespace fheco
{
    class Var; 
    class Ciphertext;

    class Expression
    {
    public:
        enum class Op_t {
            add,
            sub,
            negate,
            mul,
            mod_switch,
            o_none
        };

        // Create an undefined expressionession 
        Expression();

        /* Create an expressionession for a unary operator */
        Expression(Op_t op, const Expression& expression0);

        /* Create an expressionession for a binary operator */
        Expression(Op_t op, const Expression& expression0, const Expression& expression1, const std::vector<Var> args, const std::vector<Var> compute_args, Type type);

        /* Expressionession representing a dot product */
        Expression(Op_t op, const Expression& expression0, const Expression& expression1, bool is_reduction, const std::vector<Var> args, const std::vector<Var> compute_args, Type type);

        /* Construct an integer expressionession */
        Expression(integer value);

        /* Construct an expressionession corresponding to a new input */
        Expression(std::vector<Ciphertext> ciphertexts, std::vector<Var> iterator_variables, std::vector<Var> compute_args, Type type);

        // Copy constructor 
        Expression(const Expression &other);

        // Move constructor
        Expression(Expression &&other) noexcept;

        // Copy assignment operator
        Expression &operator=(const Expression &other);

        // Move assignment operator 
        Expression &operator=(Expression &&other) noexcept;

        // Destructor 
        ~Expression() = default;
        /*************************************/
        friend Expression operator+(const Expression &lhs,  const Expression &rhs); 
        friend Expression operator*(const Expression &lhs,  const Expression &rhs); 
        friend Expression operator-(const Expression &lhs,  const Expression &rhs); 
        friend Expression operator-(const Expression &lhs); 
        /********************************* */
        integer value() const {
            return value_;
        }

        Type type() const {
            return type_;
        }

        Op_t op() const {
            return operator_;
        }

        bool is_defined() const {
            return is_defined_;
        }

        bool is_numeric() const {
            return is_numeric_;
        }

        bool is_evaluated() const {
            return is_evaluated_;
        }

        void set_is_evaluated(bool info) {
            is_evaluated_ = info;
        }

        bool is_reduction() const {
            return is_reduction_;
        }

        void set_ciphertexts(const std::vector<Ciphertext>& ciphertexts) {
            ciphertexts_ = ciphertexts;
        }

        std::vector<Ciphertext> get_ciphertexts() const {
            return ciphertexts_;
        }

        std::vector<Expression> get_operands() const {
            return operands_;
        }

        void set_args(const std::vector<Var>& args) {
            args_ = args;
        }

        std::vector<Var> get_args() const {
            return args_;
        }

        std::vector<Var> get_compute_args() const {
            return compute_args_;
        }

    private:
        Type type_;
        Op_t operator_ = Op_t::o_none;
        bool is_defined_ = false;
        bool is_reduction_ = false;
        bool is_numeric_ = false;
        integer value_ = 1;
        std::vector<Expression> operands_ = {};
        std::vector<Var> args_ = {};
        std::vector<Var> compute_args_ = {};
        bool is_evaluated_ = false;
        std::vector<Ciphertext> ciphertexts_ = {};
    };

}
