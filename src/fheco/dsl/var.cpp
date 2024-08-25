#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <fheco/dsl/var.hpp>
using namespace std ;
namespace fheco {
    Var::Var(const std::string &n) : name_(n), value_(0) {}
    
    Var::Var(const std::string &n, const std::size_t initialValue)
        : name_(n), value_(initialValue) {}

    // Default constructor
    Var::Var() : name_(""), value_(0) {}

    // Copy constructor
    Var::Var(const Var &other): name_(other.name_), value_(other.value_) {}
     // Move constructor
    Var::Var(Var &&other) noexcept : value_(other.value_), name_(std::move(other.name_)) {}
    
    const std::string &Var::name() const {
        return name_;
    }
    const size_t Var::value() const {
        return value_ ;
    }
    /***********************************************************************/
    /***********************************************************************/
    // Copy assignment operator
    Var &Var::operator=(const Var &other) {
        if (this != &other) {
            name_ = other.name_;
            value_ = other.value_;
        }
        return *this;
    }
    // Move assignment operator
    /*The && indicates that the parameter is an rvalue reference,
     meaning it's a reference to a temporary object that can be "moved from.*/
    Var &Var::operator=(Var &&other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            value_ = other.value_;
            other.name_ = nullptr; 
        }
        return *this;
    }

    // addition 
    Var operator+(const Var &lhs, const Var &rhs){
        return Var(lhs.name_ + " + " + rhs.name_, lhs.value_ + rhs.value_);
    }
    Var operator+(const Var &lhs, const int value){
         return Var(lhs.name_, lhs.value_ + value);   
    }
    Var operator+(const int value, const Var &rhs){
        return Var(rhs.name_, rhs.value_ + value);   
    }

    // addition assignement
    Var &Var::operator+=(const Var &rhs){
        value_ += rhs.value_;
        return *this;
    }
    Var &Var::operator+=(const int value){
        value_ += value;
        return *this;
    }
    // subtraction
    Var operator-(const Var &lhs, const Var &rhs){
        return Var(lhs.name_ + " - " + rhs.name_, lhs.value_ - rhs.value_);
    }
    Var operator-(const Var &lhs, const int value){
         return Var(lhs.name_, lhs.value_ - value);   
    }
    Var operator-(const int value, const Var &rhs){
        return Var(rhs.name_, rhs.value_ - value);   
    }

    // subtraction assignement
    Var &Var::operator-=(const Var &rhs){
        value_ -= rhs.value_;
        return *this;
    }
    Var &Var::operator-=(const int value){
        value_ -= value;
        return *this;
    }

    // multiplication
    Var operator*(const Var &lhs, const Var &rhs){
        return Var(lhs.name_ + " * " + rhs.name_, lhs.value_ * rhs.value_);
    }
    Var operator*(const Var &lhs, const int value){
         return Var(lhs.name_, lhs.value_ * value);   
    }
    Var operator*(const int value, const Var &rhs){
        return Var(rhs.name_, rhs.value_ * value);   
    }
    // multiplication assignement
    Var &Var::operator*=(const Var &rhs){
        value_ *= rhs.value_;
        return *this;
    }
    Var &Var::operator*=(const int value){
        value_ *= value;
        return *this;
    }
}