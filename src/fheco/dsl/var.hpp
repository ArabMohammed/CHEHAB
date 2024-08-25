#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
namespace fheco
{
    class Var 
    {

    public:
        /** Construct a Var with the given name */
        explicit Var(const std::string &n);
        explicit Var(const std::string &n , const std::size_t initialValue);
        explicit Var();
         // Copy constructor
        Var(const Var &other);
        // desctructor 
        ~Var() = default;
        /** Get the name of a Var */
        const std::string &name() const;
        const size_t value() const ;
        /** Test if two Vars are the same. This simply compares the names. */
        bool same_as(const Var &other) const {
            return name() == other.name();
        }
        // Move assignment operator 
        Var &operator=(Var &&other) noexcept;
        
        // Copy assignment operator
        Var &operator=(const Var &other);
        
        // Move constructor
        Var(Var &&other) noexcept;

        // addition 
        friend Var operator+(const Var &lhs, const Var &rhs);
        friend Var operator+(const Var &lhs, const int value);
        friend Var operator+(const int value, const Var &lhs);
        // addition assignement
        Var &operator+=(const Var &rhs);
        Var &operator+=(const int value);
        // subtraction
        friend Var operator-(const Var &lhs, const Var &rhs);
        friend Var operator-(const Var &lhs, const int value);
        friend Var operator-(const int value, const Var &lhs);
        // subtraction assignement
        Var &operator-=(const Var &rhs);
        Var &operator-=(const int value);
        // multiplication
        friend Var operator*(const Var &lhs, const Var &rhs);
        friend Var operator*(const Var &lhs, const int value);
        friend Var operator*(const int value, const Var &lhs);
        // multiplication assignement
        Var &operator*=(const Var &rhs);
        Var &operator*=(const int value);
    private : 
        std::size_t value_ ;
        std::string name_ ; 
    };
}