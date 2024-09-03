#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/input.hpp"
#include "fheco/dsl/ops_overloads.hpp"
#include <iostream>
using namespace std ;
namespace fheco {
    
    
    Input::Input(const std::string &name , const std::vector<Var> iterator_variables, const Type type){
        name_=name;
        iterator_variables_ = iterator_variables ;
        type_ = type ;
        // a check to be added later to verify that there isnt redundant variables  in
        // iterator_variables 
        if (type == Type::ciphertxt ){
            Ciphertext  info(name);
            ciphertexts_.push_back(info) ; 
        }else if (type == Type::vectorciphertxt){
            std::cout<<"Filling inputs for vectorciphertext \n";
            size_t vector_size = iterator_variables[0].upper_bound()-iterator_variables[0].lower_bound() ; 
            std::vector<Ciphertext> ciphertexts = {} ; 
            std::cout<<"size of vector :<<"<<vector_size<<"\n";
            for(int i = 0 ; i<vector_size ; i++){
                Ciphertext line(name + "[" + std::to_string(i) + "]");
                ciphertexts.push_back(line);
            }
            ciphertexts_ = ciphertexts ;
            std::cout<<"size of ciphertexts_ :<<"<<ciphertexts_.size()<<"\n\n";

        }

    }
    // Copy constructor
    Input::Input(const Input &other)
        : name_(other.name_), iterator_variables_(other.iterator_variables_), expression_(other.expression_), type_(other.type_),
        ciphertexts_(ciphertexts_){}
    // Move constructor
    Input::Input(Input &&other) noexcept
        : name_(std::move(other.name_)), iterator_variables_(std::move(other.iterator_variables_)),
          expression_(std::move(other.expression_)), type_(other.type_), ciphertexts_(std::move(other.ciphertexts_)) {
        other.ciphertexts_.clear();  // Ensure the moved-from object does not hold invalid pointers
    }
    // Destructor
    
    /**************************************************************************************/
    
    Expression& Input::apply_operator(const std::vector<Var> &compute_args) const{
        if (compute_args.size() != iterator_variables_.size()) {
            throw std::invalid_argument("number of args different from required_dimensions !!!!!!!");
        }
        bool has_non_zero = false ; 
        for(int i = 0 ; i< compute_args.size();i++){
            if (compute_args[i].rotation_steps()!=0){
                has_non_zero = true;
                break;
            }
        }
        if (has_non_zero) {
            int rotation_step = compute_args[0].rotation_steps();
            int total_dim = iterator_variables_[0].upper_bound()-iterator_variables_[0].lower_bound() ;
            for(int i = 1 ; i< compute_args.size();i++){
                total_dim=total_dim*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
                rotation_step=rotation_step*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound())+compute_args[i].rotation_steps() ;
            }
            if (type_ == Type::ciphertxt ){
                Ciphertext info = ciphertexts_[0];
                info = info << (rotation_step%total_dim) ;
                std::vector<Ciphertext> ciphertexts = {info} ; 
                Expression* new_instance = new Expression(ciphertexts,iterator_variables_,compute_args,type_);
                return *new_instance ;
            }
            else if (type_ == Type::vectorciphertxt){
             // this case is still to be considered 
            }
        }
        //args_ = args;  // Assuming args_ is mutable to allow modification in a const function
        Expression* new_instance = new Expression(ciphertexts_,iterator_variables_,compute_args,type_);
        return *new_instance ;  // Cast away const-ness to return a non-const reference
    }
}