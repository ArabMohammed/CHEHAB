#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/var.hpp"
using namespace std ;
namespace fheco {
    size_t Var::count_ = 0;
    Var::Var(const std::string& n , const int lower_bound, const int upper_bound)
        : name_(n), lower_bound_(lower_bound), upper_bound_(upper_bound),op_(Op_var::o_none) {
            id_=count_ ;
            count_=count_+1;
            /* if(Var::declared_vars_.find(n) != Var::declared_vars_.end()){
                throw invalid_argument("a variable with this name already exists");
            }
            Var::declared_vars_.insert(n); */
        }
    Var::Var(const std::string& n , const int lower_bound, const int upper_bound, const int increment_step)
            : name_(n),lower_bound_(lower_bound), upper_bound_(upper_bound),increment_step_(increment_step),op_(Op_var::o_none){
                id_=count_ ;
                count_=count_+1;
                /*   if(Var::declared_vars_.find(n) != Var::declared_vars_.end()){
                    throw invalid_argument("a variable with this name already exists");
                }
                Var::declared_vars_.insert(n); */
            }
    Var::Var(const Op_var type,const std::vector<Var> operands)
            : op_(type), operands_(operands){
                id_=count_ ;
                count_=count_+1;
            }
    
    Var::Var(const Op_var type,const std::vector<Var> operands,const int rotation_steps)
            : op_(type), operands_(operands), rotation_steps_(rotation_steps){
                id_=count_ ;
                count_=count_+1;
            }
    
    Var::Var(const int n)
        : name_(std::to_string(n)),lower_bound_(n),upper_bound_(n+1),is_const_(true),op_(Op_var::o_none){
            id_=count_ ;
            count_=count_+1;
        }

    // Default constructor
    Var::Var() : name_(""), lower_bound_(0), upper_bound_(1) {}

    // Copy constructor
    Var::Var(const Var &other): name_(other.name_), lower_bound_(other.lower_bound_), upper_bound_(other.upper_bound_),rotation_steps_(other.rotation_steps_),
        increment_step_(other.increment_step_), is_const_ (other.is_const_),op_(other.op_),operands_(other.operands_),id_(other.id_)
    {}
    // Move constructor
    Var::Var(Var &&other) noexcept : lower_bound_(other.lower_bound_), upper_bound_(other.upper_bound_),rotation_steps_(other.rotation_steps_),name_(std::move(other.name_)), 
                                increment_step_(other.increment_step_), is_const_ (other.is_const_),op_(other.op_),operands_(std::move(other.operands_))
    {
        other.id_=id_;
        other.lower_bound_ = 0;
        other.upper_bound_ = 1;
        other.rotation_steps_ = 0;
        other.increment_step_ = 1;
        other.is_const_ = false;
        other.op_ = Op_var::o_none; // Resetting to default state
        other.operands_.clear(); // Clear the vector (safe)
        other.name_.clear();    
    }
    const std::string &Var::name() const {
        return name_;
    }
    /***********************************************************************/
    /***********************************************************************/

    // Copy assignment operator
    Var &Var::operator=(const Var &other) {
        if (this != &other) {
            id_ = other.id_ ;
            name_ = other.name_;
            upper_bound_ = other.upper_bound_ ;
            lower_bound_ = other.lower_bound_ ;
            rotation_steps_ = other.rotation_steps_ ;
            increment_step_ = other.increment_step_ ;
            is_const_  = other.is_const_ ;
            op_ = other.op_;
            operands_ = other.operands_ ;
        }
        return *this;
    }
    // Move assignment operator
    /*The && indicates that the parameter is an rvalue reference,
     meaning it's a reference to a temporary object that can be "moved from.*/
    Var &Var::operator=(Var &&other) noexcept {
        if (this != &other) {
            id_=other.id_;
            name_ = std::move(other.name_);
            upper_bound_ = other.upper_bound_;
            lower_bound_ = other.lower_bound_ ;
            rotation_steps_ = other.rotation_steps_ ; 
            increment_step_ = other.increment_step_ ;
            is_const_  = other.is_const_ ;
            op_ = other.op_;
            operands_ = std::move(other.operands_) ;
            // Leave 'other' in a valid state
            other.lower_bound_ = 0;
            other.upper_bound_ = 1;
            other.rotation_steps_ = 0;
            other.increment_step_ = 1;
            other.is_const_ = false;
            other.op_ = Op_var::o_none;
            other.operands_.clear();  // Optional but safe
            other.name_.clear();      // Optional but safe
        }
        return *this;
    }
    
    Var operator+(const Var &lhs, const int value){
        Var updated = lhs ;
        updated.rotation_steps_+= value; 
        return updated;   
    }
    Var operator+(const int value, const Var &rhs){
        Var updated = rhs ;
        updated.rotation_steps_+= value; 
        return updated;  
    } 
    Var operator+(const Var &lhs, const Var rhs){
        std::vector<Var> operands ={};
        operands.push_back(lhs);
        operands.push_back(rhs);
        return Var(Var::Op_var::add,operands,lhs.rotation_steps() + rhs.rotation_steps());   
    }  
    Var operator-(const Var &lhs, const int value){
        Var updated = lhs ;
        updated.rotation_steps_-= value; 
        return updated; 
    }
    Var operator-(const Var &lhs, const Var rhs){
        std::vector<Var> operands ={};
        operands.push_back(lhs);
        operands.push_back(rhs);
        return Var(Var::Op_var::sub,operands,lhs.rotation_steps() + rhs.rotation_steps());   
    }  
    Var operator*(const Var &lhs, const Var rhs){
        //std::cout<<"welcome  lhs : rhs in multi \n";
        Var lhs_updated ;
        if(lhs.rotation_steps()!=0){
            //std::cout<<"lhs rot steps !=0 \n";
            lhs_updated = lhs ; 
            lhs_updated.rotation_steps_ = 0 ;
            lhs_updated = lhs_updated + Var(lhs.rotation_steps());
        }else{lhs_updated = lhs;}
        Var rhs_updated ;
        if(rhs.rotation_steps()!=0){
            //std::cout<<"rhs rot steps !=0 \n";
            rhs_updated = rhs ; 
            rhs_updated.rotation_steps_ = 0 ;
            rhs_updated = rhs_updated + Var(rhs.rotation_steps());
        }else{rhs_updated = rhs;}
        return Var(Var::Op_var::mul,{lhs_updated,rhs_updated});
    }  
    
    /********************************************************************************/   
    
    size_t Var::evalute(std::vector<std::tuple<std::string , size_t >>  bindings){
        size_t res = 0 ;
        switch (op_) {
            case Op_var::add:
                if (operands_.size() >= 2) {
                    return operands_[0].evalute(bindings) + operands_[1].evalute(bindings);
                } else {
                    throw std::runtime_error("Not enough operands for addition");
                }
                break;
            case Op_var::sub:
                if (operands_.size() >= 2) {
                    return operands_[0].evalute(bindings) - operands_[1].evalute(bindings);
                } else {
                    throw std::runtime_error("Not enough operands for subtraction");
                }
                break;
            case Op_var::mul:
                if (operands_.size() >= 2) {
                    return operands_[0].evalute(bindings) * operands_[1].evalute(bindings);
                } else {
                    throw std::runtime_error("Not enough operands for multiplication");
                }
                break;
            case Op_var::o_none:
                if(is_const()){
                    res=lower_bound();
                }else{
                    for (const auto& binding : bindings) {
                        auto[var_name, value] = binding;
                        if (var_name == name()) {
                            res = value;
                            break;
                        }
                    }
                }
                break;
        }
        return res ;
    }         
    
}