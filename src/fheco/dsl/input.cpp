#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/input.hpp"
#include "fheco/dsl/ops_overloads.hpp"
#include "fheco/dsl/tensor.hpp"
#include <iostream>
using namespace std ;
namespace fheco {
    
    Input::Input(const std::string &name , const std::vector<Var> iterator_variables, const Type type){
        name_=name;
        iterator_variables_ = iterator_variables ;
        type_ = type ;
        std::vector<size_t> dimensions;
        
        if(type_==Type::vectorciphertxt){
                std::vector<Var> sub_iterator_variables(iterator_variables.begin(), iterator_variables.end() - 1);
                for (const auto& var : sub_iterator_variables) {
                    dimensions.push_back(var.upper_bound() - var.lower_bound());
                }
                ciphertexts_ = DynamicTensor<Ciphertext>(dimensions);
                std::vector<std::pair<int, int>> ranges = { };
                for(size_t i = 0 ;i<dimensions.size();i++){
                    pair<int, int> dim = {0,dimensions[i]};
                    ranges.push_back(dim);
                }
                generateNestedLoops(ranges,[this](const std::vector<int>& iterators) {
                    std::vector<std::size_t> iterator_tuple ;
                    std::string name = name_;
                    int comp = 0 ;
                    for (int val : iterators) {
                        iterator_tuple.push_back(val);
                        name+="["+std::to_string(iterator_tuple[comp])+"]";
                        comp+=1;
                    }
                    Ciphertext info(name);
                    ciphertexts_(iterator_tuple)= info;
                });

        }else if(type_==Type::ciphertxt){
                dimensions = {1};
                ciphertexts_ = DynamicTensor<Ciphertext>(dimensions);
                ciphertexts_({0})=Ciphertext(name_);

        }else if(type==Type::plaintxt){

                std::vector<Var> sub_iterator_variables(iterator_variables.begin(), iterator_variables.end() - 1);
                for (const auto& var : sub_iterator_variables) 
                    dimensions.push_back(var.upper_bound() - var.lower_bound());
                
                plaintexts_ = DynamicTensor<Plaintext>(dimensions);
                std::vector<std::pair<int, int>> ranges = { };
                for(size_t i = 0 ;i<dimensions.size();i++){
                    pair<int, int> dim = {0,dimensions[i]};
                    ranges.push_back(dim);
                }
                generateNestedLoops(ranges,[this](const std::vector<int>& iterators) {
                    std::vector<std::size_t> iterator_tuple ;
                    std::string name = name_;
                    int comp = 0 ;
                    for (int val : iterators) {
                        iterator_tuple.push_back(val);
                        name+="["+std::to_string(iterator_tuple[comp])+"]";
                        comp+=1;
                    }
                    plaintexts_(iterator_tuple)= Plaintext(name);
                });
        }
    }
    /*********************************************************************************************/
    // Copy constructor
    Input::Input(const Input &other)
        : name_(other.name_), 
        iterator_variables_(other.iterator_variables_), 
        expression_(other.expression_),
         type_(other.type_),
        ciphertexts_(ciphertexts_),
        plaintexts_{plaintexts_}{

        }
    // Move constructor
    Input::Input(Input &&other) noexcept
        : name_(std::move(other.name_)),
        iterator_variables_(std::move(other.iterator_variables_)),
        expression_(std::move(other.expression_)),
        type_(other.type_), ciphertexts_(std::move(other.ciphertexts_)),
        plaintexts_(std::move(other.plaintexts_)) {

        }    
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
            //std::cout<<"there is rotation in input ciphertext \n";
            if (type_ == Type::ciphertxt ){
                int rotation_step = compute_args[0].rotation_steps();
                int total_dim = iterator_variables_[0].upper_bound()-iterator_variables_[0].lower_bound() ;
                for(int i = 1 ; i< compute_args.size();i++){
                    total_dim=total_dim*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
                    rotation_step=rotation_step*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound())+compute_args[i].rotation_steps() ;
                }
                /* Ciphertext info = ciphertexts_({0});
                int total_dim = iterator_variables_[0].upper_bound()-iterator_variables_[0].lower_bound() ;
                int rotation_step1 = compute_args[0].rotation_steps()*total_dim;
                info = info << rotation_step1 ;
                int rotation_step2 =0 ;
                for(int i = 1 ; i< compute_args.size();i++){
                   //total_dim=total_dim*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
                    rotation_step2=compute_args[i].rotation_steps() ;
                }
                info = info << rotation_step2 ;
                std::cout<<"rot-step1 :"<<rotation_step1<<" rot-step2 :"<<rotation_step2<<"\n\n"; */
                Ciphertext info = ciphertexts_({0});
                info = info << rotation_step ;
                DynamicTensor<Ciphertext> ciphertexts({1});
                ciphertexts({0}) = {info} ; 
                Expression* new_instance = new Expression(ciphertexts,iterator_variables_,compute_args,type_);
                return *new_instance ;
            }
            else if (type_ == Type::vectorciphertxt){
                std::vector<size_t> modified_positions ={};
                std::vector<size_t> dimensions ={};
                int slot_count = iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                for(int i =0; i<iterator_variables_.size()-1;i++){
                    dimensions.push_back(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
                }
                for(int i = 0 ; i< compute_args.size();i++){
                    if (compute_args[i].rotation_steps()!=0){
                        modified_positions.push_back(i);
                    }
                }
                DynamicTensor<Ciphertext> updated_ciphertexts(dimensions);
                for(int i = 0 ; i< modified_positions.size() ; i++){
                    /**generate ranges **/
                    std::vector<std::pair<int, int>> ranges = {};
                    int rotation_step = compute_args[i].rotation_steps();
                    int end=0;
                    if(modified_positions[i]!=compute_args.size()-1)
                        end=modified_positions[i]+1;
                    else
                        end=modified_positions[i];

                    for (int j=0; j<end; j++) {
                        int dim = compute_args[j].upper_bound() - iterator_variables_[j].lower_bound();
                        pair<int, int> tup = {0,dim};
                        ranges.push_back(tup);
                    }
                    /************Update ciphertexts on one dimension**********/
                    generateNestedLoops(ranges,[&,this](const std::vector<int>& iterators) {
                        std::vector<std::size_t> iterator_tuple ;  
                        std::vector<std::size_t> rotated_iterator_tuple ; 
                        size_t comp = 0 ;                
                        for (int val : iterators) {
                            iterator_tuple.push_back(val);
                            if(comp==modified_positions[i]){
                                rotated_iterator_tuple.push_back(val+rotation_step);
                            }else{
                                rotated_iterator_tuple.push_back(val);
                            }
                        }
                        if(modified_positions[i]!=compute_args.size()-1){
                             updated_ciphertexts(iterator_tuple)=ciphertexts_(rotated_iterator_tuple);
                        }else{
                            updated_ciphertexts(iterator_tuple)=ciphertexts_(iterator_tuple) << (rotation_step%slot_count) ;
                        }
                    });
                }
                Expression* new_instance = new Expression(updated_ciphertexts,iterator_variables_,compute_args,type_);
                return *new_instance ;
            }
            /**********Plaintext rotation still to be added ******/
        }
        if(type_==Type::ciphertxt||type_==Type::vectorciphertxt){
                Expression* new_instance = new Expression(ciphertexts_,iterator_variables_,compute_args,type_);
                return *new_instance ;  // Cast away const-ness to return a non-const reference
        }else{
            Expression* new_instance = new Expression(plaintexts_,iterator_variables_,compute_args,type_);
            return *new_instance ;  // Cast away const-ness to return a non-const reference
        }
        
    }
}