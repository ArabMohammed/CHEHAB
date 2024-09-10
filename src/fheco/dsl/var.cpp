#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/var.hpp"
using namespace std ;
namespace fheco {
    std::unordered_set<std::string> Var::declared_vars_;

    Var::Var(const std::string& n , const int lower_bound, const int upper_bound)
        : name_(n), lower_bound_(lower_bound), upper_bound_(upper_bound) {
            if(Var::declared_vars_.find(n) != Var::declared_vars_.end()){
                throw invalid_argument("a variable with this name already exists");
            }
            Var::declared_vars_.insert(n);
        }
    
    Var::Var(const std::string& n , const int lower_bound, const int upper_bound, int rotation_steps)
        : name_(n), lower_bound_(lower_bound), upper_bound_(upper_bound), rotation_steps_(rotation_steps) {}
    
    Var::Var(const int n)
        : name_(std::to_string(n)),lower_bound_(n),upper_bound_(n+1){}

    // Default constructor
    Var::Var() : name_(""), lower_bound_(0), upper_bound_(1) {}

    // Copy constructor
    Var::Var(const Var &other): name_(other.name_), lower_bound_(other.lower_bound_), upper_bound_(other.upper_bound_),rotation_steps_(other.rotation_steps_){}
     // Move constructor
    Var::Var(Var &&other) noexcept : lower_bound_(other.lower_bound_), upper_bound_(other.upper_bound_),rotation_steps_(other.rotation_steps_),name_(std::move(other.name_)) {}
    
    const std::string &Var::name() const {
        return name_;
    }
    /***********************************************************************/
    /***********************************************************************/

    // Copy assignment operator
    Var &Var::operator=(const Var &other) {
        if (this != &other) {
            name_ = other.name_;
            upper_bound_ = other.upper_bound_ ;
            lower_bound_ = other.lower_bound_ ;
            rotation_steps_ = other.rotation_steps_ ;
        }
        return *this;
    }
    // Move assignment operator
    /*The && indicates that the parameter is an rvalue reference,
     meaning it's a reference to a temporary object that can be "moved from.*/
    Var &Var::operator=(Var &&other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            upper_bound_ = other.upper_bound_;
            lower_bound_ = other.lower_bound_ ;
            rotation_steps_ = other.rotation_steps_ ;
            other.name_ = ""; 
        }
        return *this;
    }

       // addition 
    /*   Var operator+(const Var &lhs, const Var &rhs){
        return Var(lhs.name_ + " + " + rhs.name_,lhs.lower_bound_ + rhs.lower_bound_,lhs.upper_bound_ + rhs.upper_bound_);
    } */
    Var operator+(const Var &lhs, const int value){
         return Var(lhs.name(),lhs.lower_bound(),lhs.upper_bound(),lhs.rotation_steps() + value);   
    }
   

    // addition assignement
    /* Var &Var::operator+=(const Var &rhs){
        value_ += rhs.value_;
        return *this;
    } */
    /*  Var &Var::operator+=(const int value){
        value_ += value;
        return *this;
    } */
    // subtraction
   /*  Var operator-(const Var &lhs, const Var &rhs){
        return Var(lhs.name_ + " - " + rhs.name_, lhs.value_ - rhs.value_);
    } */
    Var operator-(const Var &lhs, const int value){
        return Var(lhs.name(),lhs.lower_bound(),lhs.upper_bound(),lhs.rotation_steps() - value);    
    }
    /* Var operator-(const int value, const Var &rhs){
        return Var(rhs.name_, rhs.value_ - value);   
    } */
    /**
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
    } */
   /*  Var operator*(const Var &lhs, const int value){
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
    }  */
}