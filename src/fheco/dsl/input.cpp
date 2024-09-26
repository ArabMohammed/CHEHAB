#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include "fheco/dsl/input.hpp"
#include "fheco/dsl/ops_overloads.hpp"
#include "fheco/dsl/plaintext.hpp"
#include "fheco/dsl/expression.hpp"
#include "fheco/dsl/tensor.hpp"
#include <iostream>
#include <cmath>  // For std::ceil()
using namespace std ;
namespace fheco {
    class Plaintext ;
    class Ciphertext;
    template <typename T>
    class DynamicTensor ;
    /****************************** */
    Input::Input(const std::string &name , const std::vector<Var> iterator_variables, const Type type){
        name_=name;
        iterator_variables_ = iterator_variables ;
        type_ = type ;
        std::vector<size_t> dimensions;
        
        if(type_==Type::vectorciphertxt){
                std::cout<<"Create a new vectorciphertext Input\n";
                std::vector<Var> sub_iterator_variables(iterator_variables.begin(), iterator_variables.end() - 1);
                // case when input vectorciphertext is a packedciphertext .
                if(sub_iterator_variables.size()==0){
                    std::cout<<"This input is a vector \n";
                    ciphertexts_ = DynamicTensor<Ciphertext>({0});
                    Ciphertext info(name_);
                    ciphertexts_({0})= info;
                }else{
                    std::cout<<"input vectorciphertxt has multiple dimensions\n";
                    std::vector<std::vector<int>> ranges = {};
                    std::vector<int> range ={} ;
                    for (const auto& var : sub_iterator_variables){
                        dimensions.push_back(var.upper_bound() - var.lower_bound());
                        range={0,var.upper_bound() - var.lower_bound(),var.increment_step()};
                        ranges.push_back(range);
                    }
                    ciphertexts_ = DynamicTensor<Ciphertext>(dimensions);
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
                        //std::cout<<"start iteration : "<<name<<" \n";
                        ciphertexts_(iterator_tuple)= info;
                        return true;
                    });
                }
        }else if(type_==Type::ciphertxt){
                dimensions = {1};
                ciphertexts_ = DynamicTensor<Ciphertext>(dimensions);
                ciphertexts_({0})=Ciphertext(name_);

        }else if(type==Type::plaintxt){
                std::cout<<"Create a new plaintext \n";
                std::vector<Var> sub_iterator_variables(iterator_variables.begin(), iterator_variables.end() - 1);
                // case when input plaintext is a packedplaintext .
                if(sub_iterator_variables.size()==0){
                    std::cout<<"input palintext is a vector \n";
                    plaintexts_ = DynamicTensor<Plaintext>({1});
                    Plaintext info(name_);
                    plaintexts_({0}) = info;
                }else{
                    std::cout<<"input plainphertxt has multiple dimensions\n";
                    std::vector<std::vector<int>> ranges = {};
                    std::vector<int> range ={} ;
                    for (const auto& var : sub_iterator_variables){
                        dimensions.push_back(var.upper_bound() - var.lower_bound());
                        range={0,var.upper_bound() - var.lower_bound(),var.increment_step()};
                        ranges.push_back(range);
                    }
                    plaintexts_ = DynamicTensor<Plaintext>(dimensions);
                    generateNestedLoops(ranges,[this](const std::vector<int>& iterators) {
                        std::vector<std::size_t> iterator_tuple ;
                        std::string name = name_;
                        int comp = 0 ;
                        for (int val : iterators) {
                            iterator_tuple.push_back(val);
                            name+="["+std::to_string(iterator_tuple[comp])+"]";
                            comp+=1;
                        }
                        //std::cout<<name<<"\n";
                        plaintexts_(iterator_tuple)= Plaintext(name);
                        return true;
                    });
                    
                }
        }
    }
    /*********************************************************************************************/
    Input::Input(const std::vector<Var> iterator_variables, const Type type, std::vector<integer> initializing_inputs){
        iterator_variables_ = iterator_variables ;
        type_ = type ;
        std::vector<size_t> dimensions;
        std::cout<<"Initializing a new plaintext using a vector \n";
        if(type!=Type::plaintxt){
            throw invalid_argument("you can only initialize a plaintext");
        }
        std::vector<Var> sub_iterator_variables(iterator_variables.begin(), iterator_variables.end() - 1);
        // case when input plaintext is a packedplaintext .
        if(sub_iterator_variables.size()==0){
            std::cout<<"Create a plaintext with vector form\n";
            plaintexts_ = DynamicTensor<Plaintext>({1});
            plaintexts_({0}) = Plaintext(initializing_inputs);
        }else{
            std::cout<<"Create a plaintext with tensor form\n";
            std::vector<std::vector<int>> ranges = {};
            std::vector<int> range ={ } ;
            for (const auto& var : sub_iterator_variables){
                dimensions.push_back(var.upper_bound() - var.lower_bound());
                std::cout<<var.upper_bound() - var.lower_bound()<<" ";
                range={0,var.upper_bound() - var.lower_bound(),var.increment_step()};
                ranges.push_back(range);
            }
            plaintexts_ = DynamicTensor<Plaintext>(dimensions);
            int total_length = initializing_inputs.size();
            int vector_size = iterator_variables[iterator_variables.size()-1].upper_bound()-iterator_variables[iterator_variables.size()-1].lower_bound();
            int current_pos = 0 ;
            vector<integer> values  = {} ;
            generateNestedLoops(ranges,[ranges,initializing_inputs,iterator_variables,vector_size,total_length,&current_pos,&values,this](const std::vector<int>& iterators) { 
                std::vector<std::size_t> iterator_tuple ;
                for (int val : iterators) {
                    iterator_tuple.push_back(val);
                }
                values = {};
                for(int j=0 ; j< vector_size;j++){
                    std::cout<<initializing_inputs[current_pos]<<" " ;
                    values.push_back(initializing_inputs[current_pos]);
                    current_pos+=1;
                    if(current_pos==total_length){
                        for(int k=j+1;k<vector_size;k++){
                            values.push_back(0);
                        }
                        plaintexts_(iterator_tuple)= Plaintext(values);
                        return false ;
                    }
                }
                std::cout<<"\n";
                plaintexts_(iterator_tuple)= Plaintext(values);
                std::vector<integer> old_content = {};
                PackedVal example_val = {0};
              
                if (auto opt_val = plaintexts_(iterator_tuple).example_val()) {
                    old_content = opt_val.value();
                } else {
                    old_content = example_val;  // Default
                }
                for(auto val : old_content)
                    std::cout<<val<<" ";
                std::cout<<"\n";
                return true;
            });
        }
    }
    /********************************************************************************************/
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
    // Copy assignment operator
    Input &Input::operator=(const Input &other) {
        if (this != &other) {
            name_ = other.name_;
            iterator_variables_=other.iterator_variables_;
            expression_ = other.expression_;
            type_ =other.type_;
            ciphertexts_=other.ciphertexts_;
            plaintexts_ = other.plaintexts_ ;
        }
        return *this;
    }
    // Move assignment operator
    Input &Input::operator=(Input &&other) noexcept {
        if (this != &other) {
            name_ =std::move(other.name_);
            iterator_variables_= std::move(other.iterator_variables_);
            expression_ = std::move(other.expression_);
            type_ = other.type_;
            ciphertexts_ = std::move(other.ciphertexts_);
            plaintexts_ = std::move(other.plaintexts_);
        }
        return *this;
    }
    /**************************************************************************************/
    void Input::resize(std::vector<Var> new_dimensions_vars){
        int old_total_length = 1 ;
        int new_total_length = 1 ,vector_size =1;
        //std::cout<<"start resizing \n";
        for (int i =0;i<iterator_variables_.size();i++){
            old_total_length*=(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
        }
        //std::cout<<"end1\n";
        for (int i =0;i<new_dimensions_vars.size();i++){
            new_total_length*=(new_dimensions_vars[i].upper_bound()-new_dimensions_vars[i].lower_bound());
            vector_size = new_dimensions_vars[i].upper_bound()-new_dimensions_vars[i].lower_bound() ;
        }
        //std::cout<<"end2 : "<<vector_size<<"\n";
        if(new_total_length<old_total_length){
            throw invalid_argument("resize impossible because the new_total_dim < old_total_dim");
        }
        if (type_==Type::vectorciphertxt){
            int total_iterations = ceil(static_cast<double>(old_total_length) / vector_size);
            /********************************/
            std::vector<std::vector<int>> ranges = {};
            std::vector<Var> sub_iterator_variables(new_dimensions_vars.begin(), new_dimensions_vars.end() - 1);
            iterator_variables_ = new_dimensions_vars ;
            if(sub_iterator_variables.size()==0){
                    //std::cout<<"add new input here \n";
                    ciphertexts_ = DynamicTensor<Ciphertext>({1});
                    Ciphertext info(name_);
                    ciphertexts_({0})= info;
            }else{
                std::vector<size_t> new_dimensions ={};
                for (const auto& var : sub_iterator_variables){
                    new_dimensions.push_back(var.upper_bound() - var.lower_bound());
                    ranges.push_back({0,var.upper_bound() - var.lower_bound(),var.increment_step()});
                }
                ciphertexts_ = DynamicTensor<Ciphertext>(new_dimensions);
                int counter = 0 ;
                std::vector<std::size_t> iterator_tuple = {};
                generateNestedLoops(ranges,[this,&iterator_tuple,&counter,total_iterations](const std::vector<int>& iterators) {
                    iterator_tuple = {} ;
                    std::string name = name_;
                    int comp = 0 ;
                    for (int val : iterators) {
                        iterator_tuple.push_back(val);
                        name+="["+std::to_string(iterator_tuple[comp])+"]";
                        comp+=1;
                    }
                    Ciphertext info(name);
                    counter+=1;
                    //std::cout<<"==> store a new ciphertext \n";
                    ciphertexts_(iterator_tuple)= info;
                    if(counter==total_iterations){
                        return false ;
                    }
                    return true; 
                });
            }
        }
        /*************************************************************************/
        else if(type_==Type::plaintxt){
            //std::cout<<"resizing a pliantext \n";
            std::vector<integer> old_content = {};
            PackedVal example_val = {0};
            if(iterator_variables_.size()==1){
                if (auto opt_val = plaintexts_({0}).example_val()) {
                    old_content = opt_val.value();
                } else {
                    old_content = example_val;  // Default
                }
            }else{
                // std::cout<<"provied plaintext is a tensor \n";
                std::vector<std::vector<int>> ranges = {};
                std::vector<int> range ={} ;
                std::vector<Var> sub_iterator_variables(iterator_variables_.begin(), iterator_variables_.end() - 1);
                for (const auto& var : sub_iterator_variables){
                    range={0,var.upper_bound() - var.lower_bound(),var.increment_step()};
                    ranges.push_back(range);
                }
                vector<integer> temp_plaintxt ={};
                generateNestedLoops(ranges,[this,&old_content,example_val,&temp_plaintxt](const std::vector<int>& iterators) { 
                    std::vector<std::size_t> iterator_tuple ;
                    for (int val : iterators) {
                        iterator_tuple.push_back(val);
                    }
                    temp_plaintxt = {0};
                    if (auto opt_val = plaintexts_(iterator_tuple).example_val()) {
                        temp_plaintxt = opt_val.value();
                    } 
                    old_content.insert(old_content.end(), temp_plaintxt.begin(), temp_plaintxt.end());
                    return true;
                });
            }
            /********************************************************************************************************/
            iterator_variables_ = new_dimensions_vars ;
            std::vector<Var> sub_iterator_variables(new_dimensions_vars.begin(), new_dimensions_vars.end() - 1);
            if(sub_iterator_variables.size()==0){
               // std::cout<<"resized resulting plaintext is of vector form\n";
                plaintexts_ = DynamicTensor<Plaintext>({1});
                if(name_!="")
                    plaintexts_({0}) = Plaintext(name_);
                else 
                    plaintexts_({0}) = Plaintext(old_content);
            }
            /*******************************************/
            else{
                //std::cout<<"resized resulting plaintext is of tensor form\n";
                std::vector<std::vector<int>> ranges = {};
                std::vector<size_t> new_dimensions ={};
                for (const auto& var : sub_iterator_variables){
                    new_dimensions.push_back(var.upper_bound() - var.lower_bound());
                    ranges.push_back({0,var.upper_bound() - var.lower_bound(),var.increment_step()});
                }
                plaintexts_ = DynamicTensor<Plaintext>(new_dimensions);
                int counter = 0 ;
                std::vector<std::size_t> iterator_tuple = {};
                generateNestedLoops(ranges,[this,&iterator_tuple,&counter,old_total_length,vector_size,old_content](const std::vector<int>& iterators) {
                    iterator_tuple = {} ;
                    std::string name = name_;
                    int comp = 0 ;
                    for (int val : iterators) {
                        iterator_tuple.push_back(val);
                        name+="["+std::to_string(iterator_tuple[comp])+"]";
                        comp+=1;
                    }
                    vector<integer> values ;
                    for(int j=0 ; j < vector_size ;j++){
                        values.push_back(old_content[counter]);
                        counter+=1;
                        if(counter==old_total_length){
                            for(int k=j+1;k<vector_size;k++){
                                values.push_back(0);
                            }
                            if(name_!="")
                                plaintexts_(iterator_tuple)=Plaintext(name);
                            else 
                                plaintexts_(iterator_tuple)=Plaintext(values);
                            return false ;
                        }
                    }
                    //std::cout<<"insert a new resized plaintext\n";
                    if(name_!="")
                        plaintexts_(iterator_tuple)=Plaintext(name);
                    else 
                        plaintexts_(iterator_tuple)=Plaintext(values);
                    return true; 
                });
            }
        }
        /*************************************************************************/
        else if(type_==Type::ciphertxt){
            throw invalid_argument("cannot resize a cipphertext");
        }
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
            std::cout<<"there is rotation in input ciphertext \n";
            if (type_ == Type::ciphertxt ){
                int rotation_step = compute_args[0].rotation_steps();
                int total_dim = iterator_variables_[0].upper_bound()-iterator_variables_[0].lower_bound() ;
                for(int i = 1 ; i< compute_args.size();i++){
                    total_dim=total_dim*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
                    rotation_step=rotation_step*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound())+compute_args[i].rotation_steps() ;
                }
                Ciphertext info = ciphertexts_({0});
                info = info << rotation_step ;
                DynamicTensor<Ciphertext> ciphertexts({1});
                ciphertexts({0}) = {info} ; 
                Expression* new_instance = new Expression(ciphertexts,iterator_variables_,compute_args,type_);
                return *new_instance ;
            }
            else if (type_ == Type::vectorciphertxt){
                std::vector<size_t> dimensions ={};
                int slot_count = iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                for(int i =0; i<iterator_variables_.size()-1;i++){
                    dimensions.push_back(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
                }
                /*********************************************************/
                DynamicTensor<Ciphertext> updated_ciphertexts(dimensions);
                DynamicTensor<Ciphertext> ciphertexts = ciphertexts_ ;
                for(int i = 0 ; i< compute_args.size() ; i++){
                    if (compute_args[i].rotation_steps()!=0){
                        std::vector<std::vector<int>> ranges = {};
                        int rotation_step = compute_args[i].rotation_steps();
                        int vector_size =1 ;
                        int end=0;
                        if(i!=compute_args.size()-1)
                            end=i+1;
                        else
                            end=i;
                        for (int j=0; j<end; j++) {
                            int dim = iterator_variables_[j].upper_bound() - iterator_variables_[j].lower_bound();
                            ranges.push_back({0,dim,iterator_variables_[j].increment_step()});
                            vector_size = dim ;
                        }
                        /************Update ciphertexts on one dimension**********/
                        std::cout<<vector_size<<" "<<rotation_step<<"\n";
                        generateNestedLoops(ranges,[&,this](const std::vector<int>& iterators) {
                            std::vector<std::size_t> iterator_tuple ;  
                            std::vector<std::size_t> rotated_iterator_tuple ; 
                            size_t comp = 0 ;                
                            for (int val : iterators) {
                                iterator_tuple.push_back(val);
                                if(comp==i){
                                    int info = ((val+rotation_step)+vector_size)%vector_size ;
                                    rotated_iterator_tuple.push_back(info);
                                }else{
                                    rotated_iterator_tuple.push_back(val);
                                }
                                comp+=1;
                            }
                            if(i!=compute_args.size()-1){
                                //std::cout<<"rotation to be done :"<<iterator_tuple[0]<<":"<<iterator_tuple[1]<<" , "<<rotated_iterator_tuple[0]<<":"<<rotated_iterator_tuple[1]<<"\n";
                                updated_ciphertexts.assign_subtensor(iterator_tuple,ciphertexts.subtensor(rotated_iterator_tuple));
                            }else{
                                updated_ciphertexts(iterator_tuple)=ciphertexts(iterator_tuple) << (rotation_step%slot_count) ;
                            }
                            return true ;
                        });
                    }
                    ciphertexts=updated_ciphertexts;
                }
                std::cout<<"return rotated input expression\n";
                Expression* new_instance = new Expression(updated_ciphertexts,iterator_variables_,compute_args,type_);
                return *new_instance ;
            }
            /**********Plaintext rotation still to be added ******/
        }
        std::cout<<"No rotation have been applied "<<compute_args.size()<<"\n";
        if(type_==Type::ciphertxt||type_==Type::vectorciphertxt){
                Expression* new_instance = new Expression(ciphertexts_,iterator_variables_,compute_args,type_);
                return *new_instance ;  // Cast away const-ness to return a non-const reference
        }else{
            Expression* new_instance = new Expression(plaintexts_,iterator_variables_,compute_args,type_);
            return *new_instance ;  // Cast away const-ness to return a non-const reference
        }
        
    }
}