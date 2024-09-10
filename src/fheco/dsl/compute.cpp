 
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <iostream>
#include "fheco/dsl/compute.hpp"
#include "fheco/dsl/ops_overloads.hpp"
#include "fheco/dsl/tensor.hpp"
using namespace std ;
namespace fheco {

    /*************************************************************************************************************/
    Computation::Computation(const std::string &name , const std::vector<Var> iterator_variables, const Expression& expression): 
        name_(name), iterator_variables_(iterator_variables){
            expression_= expression ;

        }
    Computation::Computation(const std::string &name, const std::vector<Var> iterator_variables,
                            const std::vector<Var> reduction_variables ,const Expression &expression):
                            name_(name), iterator_variables_(iterator_variables),
                            reduction_variables_(reduction_variables),expression_(expression) 
        {
            is_reduction_=true;
            expression_.set_is_reduction(true);                        
        }

    Computation::Computation(const std::string &name , const std::vector<Var> iterator_variables,
                             const std::vector<Var> reduction_variables, Type type): 
                            name_(name), iterator_variables_(iterator_variables)
        {
            std::vector<size_t> dimensions ;
            for (const auto& var : reduction_variables) {
                    dimensions.push_back(var.upper_bound() - var.lower_bound());
            }
            DynamicTensor<Ciphertext> ciphertexts(dimensions) ; 
            expression_ =  Expression(ciphertexts,iterator_variables,iterator_variables,type);
        }
    /**************************************************************************************************** */
    Computation::Computation(const std::string &name, const std::vector<Var> iterator_variables,Type type):
                            name_(name), iterator_variables_(iterator_variables)
        {
            std::vector<size_t> dimensions ;
            for (const auto& var : iterator_variables) {
                    dimensions.push_back(var.upper_bound() - var.lower_bound());
            }
            DynamicTensor<Ciphertext> ciphertexts(dimensions) ; 
            expression_ =  Expression(ciphertexts,iterator_variables,iterator_variables,type);
        }
    // Copy constructor 
    Computation::Computation(const Computation &other) : 
        name_(other.name_), iterator_variables_(other.iterator_variables_),
        expression_(other.expression_),reduction_variables_(other.reduction_variables_)
        {}
    // Move constructor
    Computation::Computation(Computation &&other) noexcept:  
        name_(std::move(other.name_)), iterator_variables_(std::move(other.iterator_variables_)),
        reduction_variables_(std::move(other.reduction_variables_)),expression_(std::move(other.expression_))
        {}
    // Move assignment operator 
    Computation &Computation::operator=(Computation &&other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            iterator_variables_ = std::move(other.iterator_variables_);
            reduction_variables_ = std::move(other.reduction_variables_);
            expression_ = std::move(other.expression_);
        }
        return *this;
    }
            
    // Copy assignment operator
    Computation &Computation::operator=(const Computation &other){
         if (this != &other) {
            name_ = other.name_;
            iterator_variables_ = other.iterator_variables_;
            expression_ = other.expression_;
        }
        return *this;
    }
    /*************************************************************************************/
   
    Expression& Computation::apply_operator(const std::vector<Var> &compute_args) const{
        //std::cout<<compute_args.size()<<" "<<iterator_variables_.size()<<" \n";
        if (compute_args.size() != iterator_variables_.size()) {
            throw std::invalid_argument("number of args different from required_dimensions");
        }
        bool has_non_zero = false ; 
        for(int i = 0 ; i< compute_args.size();i++){
            if (compute_args[i].rotation_steps()!=0){
                has_non_zero = true;
                break;
            }
        }
        if (has_non_zero) {
            std::vector<Var> expresssion_args = expression_.get_args();
            if (expression_.type() == Type::ciphertxt ){
                int rotation_step = compute_args[0].rotation_steps();
                int total_dim = expresssion_args[0].upper_bound()-expresssion_args[0].lower_bound() ;
                for(int i = 1 ; i< compute_args.size();i++){
                    total_dim=total_dim*(expresssion_args[i].upper_bound()-expresssion_args[i].lower_bound());
                    rotation_step=rotation_step*(expresssion_args[i].upper_bound()-expresssion_args[i].lower_bound())+compute_args[i].rotation_steps() ;
                }
                Ciphertext info = expression_.get_ciphertexts()({0});
                info = info << (rotation_step%total_dim) ;
                DynamicTensor<Ciphertext> ciphertexts({1});
                ciphertexts({0}) = {info} ; 
                Expression* new_instance = new Expression(ciphertexts,iterator_variables_,compute_args,expression_.type());
                return *new_instance ;
            }
            else if (expression_.type() == Type::vectorciphertxt){
                std::vector<size_t> modified_positions ={};
                std::vector<size_t> dimensions ={};
                int slot_count = expresssion_args[expresssion_args.size()-1].upper_bound()-expresssion_args[expresssion_args.size()-1].lower_bound();
                for(int i =0; i<expresssion_args.size();i++){
                    dimensions.push_back(expresssion_args[i].upper_bound()-expresssion_args[i].lower_bound());
                }
                for(int i = 0 ; i< compute_args.size();i++){
                    if (compute_args[i].rotation_steps()!=0){
                        modified_positions.push_back(i);
                    }
                }
                DynamicTensor<Ciphertext> ciphertexts = expression_.get_ciphertexts();
                DynamicTensor<Ciphertext> updated_ciphertexts(dimensions);
                for(int i = 0 ; i< modified_positions.size() ; i++){
                    /**generate ranges **/
                    std::vector<std::pair<int, int>> ranges = {};
                    int rotation_step = compute_args[i].rotation_steps();
                    for (int j=0; j<modified_positions[i]+1; j++) {
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
                             updated_ciphertexts.assign_subtensor(iterator_tuple,ciphertexts.subtensor(rotated_iterator_tuple));
                        }else{
                            updated_ciphertexts(iterator_tuple)=ciphertexts(iterator_tuple) << (rotation_step%slot_count) ;
                        }
                    });

                }
                Expression* new_instance = new Expression(updated_ciphertexts,iterator_variables_,compute_args,Type::vectorciphertxt);
                return *new_instance ;

            }
        }
        if(expression_.type()==Type::ciphertxt||expression_.type()==Type::vectorciphertxt){
            Expression* new_instance = new Expression(expression_.get_ciphertexts(),iterator_variables_,compute_args,expression_.type());
            return *new_instance ;  // Cast away const-ness to return a non-const reference
        }else{
            Expression* new_instance = new Expression(expression_.get_plaintexts(),iterator_variables_,compute_args,expression_.type());
            return *new_instance ;  // Cast away const-ness to return a non-const reference
        }
    }    

    /**************************************************************************************/
    DynamicTensor<Ciphertext> Computation::evaluate_expression(Expression& expression ){
            if (expression.is_defined()){
                if (expression.is_evaluated()) {
                    return expression.get_ciphertexts();
                }
                if (expression.op()!=Expression::Op_t::o_none) {
                    DynamicTensor<Ciphertext> result ;
                    Expression lhs = expression.get_operands()[0] ;
                    Expression rhs = expression.get_operands()[1];
                    switch(expression.op()) {
                        case Expression::Op_t::mul:
                               //std::cout<<"\n Realising a multipication \n" ;
                                if(expression.is_reduction()){
                                    std::cout<<"\nRealised operation is a reduction multiplication\n";
                                    /***************** 
                                    if(!expression.get_operands()[0].is_evaluated()){
                                       expression.get_operands()[0].set_is_reduction(true); 
                                    }
                                    if(!expression.get_operands()[1].is_evaluated()){
                                       expression.get_operands()[0].set_is_reduction(true); 
                                    }
                                    /**********************************************************************/

                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                            int reduction_size =1 ;
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            result = DynamicTensor<Ciphertext>({1});
                                            if(lhs.type()==Type::plaintxt){
                                                plaintexts = lhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(rhs) ;
                                                int len = rhs.get_args().size();
                                                reduction_size=reduction_size*(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());
                                            }else{
                                                plaintexts = rhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(lhs) ;
                                                int len = lhs.get_args().size();
                                                reduction_size=reduction_size*(lhs.get_args()[len-1].upper_bound()-lhs.get_args()[len-1].lower_bound());
                                            }
                                            //std::cout<<"reduction size is :"<<reduction_size<<"\n";
                                            result({0})=SumVec(plaintexts({0})*ciphertexts({0}),reduction_size); 
                                    }
                                    /**********************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                            int len = rhs.get_args().size();
                                            int reduction_size=(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());                                            
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            result = DynamicTensor<Ciphertext>({1});
                                            std::cout<<"size of reduction in cipher*cipher add :"<<reduction_size<<"\n";
                                            result({0})=SumVec(ciphertexts0({0})*ciphertexts1({0}),reduction_size); 
                                    }
                                    /**********************************************************************/
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            std::vector<Var> vars0, vars1 ;
                                            if(lhs.type()==Type::plaintxt){
                                                vars1 = lhs.get_compute_args();
                                                plaintexts = lhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                vars1 = rhs.get_compute_args();
                                                plaintexts = rhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            int reduction_size =iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                                            auto [reduction_pos, vars0_pos, vars1_pos] = match_positions(iterator_variables_, reduction_variables_, vars0, vars1);
                                            std::vector<std::size_t>   ref_reduction_tuple ={};
                                            for(int i =0;i<reduction_variables_.size();i++){
                                                ref_reduction_tuple.push_back(reduction_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions ;
                                            for(int i=0;i<reduction_variables_.size()-1;i++){
                                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions);  
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            /***************************************************/
                                            int mask_size = iterator_variables_[iterator_variables_.size()-2].upper_bound()-iterator_variables_[iterator_variables_.size()-2].lower_bound();
                                            std::vector<int64_t> mask1(mask_size,0);
                                            mask1[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                            temp_res=SumVec(ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple),reduction_size)*mask1;
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                std::vector<std::size_t> reduction_tuple ;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<reduction_pos.size();i++){
                                                        if(reduction_pos[i]==comp){
                                                            reduction_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                bool row_updated = false;
                                                for(int i =0;i<reduction_tuple.size();i++){
                                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                        update=true ;
                                                        if(i<reduction_tuple.size()-1){
                                                             row_updated = true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                std::vector<int64_t> mask(mask_size,0);
                                                mask[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                                std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                if(update){    
                                                    if(row_updated){    
                                                        std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                        result(sub_ref_reduction_tuple)=temp_res;
                                                        temp_res  = SumVec(ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple),reduction_size)*mask;
                                                    }else{
                                                        temp_res += SumVec(ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple),reduction_size)*mask;
                                                    }
                                                    ref_reduction_tuple=reduction_tuple;
                                                }
                                            });
                                            std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                            result(sub_ref_reduction_tuple) = temp_res; 
                                    }
                                    /**********************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /*************************/
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            int reduction_size =iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                                            auto [reduction_pos, vars0_pos, vars1_pos] = match_positions(iterator_variables_, reduction_variables_, vars0, vars1);
                                            std::vector<std::size_t>   ref_reduction_tuple ={};
                                            for(int i =0;i<reduction_variables_.size();i++){
                                                ref_reduction_tuple.push_back(reduction_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions ;
                                            for(int i=0;i<reduction_variables_.size()-1;i++){
                                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions);  
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            /****************************************************************************/
                                            int mask_size = iterator_variables_[iterator_variables_.size()-2].upper_bound()-iterator_variables_[iterator_variables_.size()-2].lower_bound();
                                            std::vector<int64_t> mask1(mask_size,0);
                                            mask1[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                            temp_res=SumVec(ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple),reduction_size)*mask1;
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                std::vector<std::size_t> reduction_tuple={};
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<reduction_pos.size();i++){
                                                        if(reduction_pos[i]==comp){
                                                            reduction_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                bool row_updated = false;
                                                for(int i =0;i<reduction_tuple.size();i++){
                                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                        update=true ;
                                                        if(i<reduction_tuple.size()-1){
                                                             row_updated = true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                std::vector<int64_t> mask(mask_size,0);
                                                mask[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                                std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                if(update){    
                                                    if(row_updated){
                                                        std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                        result(sub_ref_reduction_tuple)=temp_res;
                                                        temp_res  = SumVec(ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple),reduction_size)*mask;
                                                    }else{
                                                        temp_res += SumVec(ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple),reduction_size)*mask;

                                                    }           
                                                    ref_reduction_tuple=reduction_tuple;
                                                }/* else{
                                                    temp_res+=ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple)*mask;
                                                } */
                                            });
                                            std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                            result(sub_ref_reduction_tuple) = temp_res; 
                                    }
                                }else{
                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                           std::cout<<"welcome in plain cipher mul \n"; 
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            if(lhs.type()==Type::plaintxt){
                                                plaintexts = lhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                plaintexts = rhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            result = DynamicTensor<Ciphertext>({1});
                                            result({0})=plaintexts({0})*ciphertexts({0}); 
                                    }
                                    /***************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                            result = DynamicTensor<Ciphertext>({1});
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            result({0})= ciphertexts0({0})*ciphertexts1({0}); 
                                    }
                                    /**************************************************************/                                      
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                           // std::cout<<"welcome in plaintxt*vectortxt mul \n";
                                            int reduction_size =1 ;
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            std::vector<Var> vars0, vars1 ;
                                            if(lhs.type()==Type::plaintxt){
                                                vars1 = lhs.get_compute_args();
                                                plaintexts = lhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                vars1 = rhs.get_compute_args();
                                                plaintexts = rhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            std::vector<size_t> dimensions ;
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    dimensions.push_back(dim);
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions1(dimensions.begin(),dimensions.end()-1) ;
                                            result = DynamicTensor<Ciphertext>(dimensions1);  
                                            vector<int> vars0_pos ={},vars1_pos ={};
                                            /*********************************/
                                            for (int i = 0; i < iterator_variables_.size(); ++i) {
                                                for (int j = 0; j < vars0.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars0[j])) {
                                                        vars0_pos.push_back(i);
                                                        break;
                                                    }
                                                }

                                                for (int j = 0; j < vars1.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars1[j])) {
                                                        vars1_pos.push_back(i);
                                                        break;
                                                    }
                                                }
                                            }
                                            /*********************************/
                                            /*Initialize result ciphertexts*/
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            std::vector<size_t> ref_iterator_tuple(iterator_tuple.begin(),iterator_tuple.end()-1);
                                            temp_res=ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple);
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                int comp = 0;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                std::vector<size_t> sub_iterator_tuple(iterator_tuple.begin(), iterator_tuple.end() - 1);
                                                for(int i =0;i<sub_iterator_tuple.size();i++){
                                                    if(sub_iterator_tuple[i]!=ref_iterator_tuple[i]){
                                                        update=true ;
                                                        break;
                                                    }
                                                }
                                                if(update){   
                                                    std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1); 
                                                    result(ref_iterator_tuple)=temp_res;                            
                                                    ref_iterator_tuple=iterator_tuple;
                                                    temp_res=ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple);
                                                }
                                            });
                                            result(ref_iterator_tuple) = temp_res; 

                                    /**************************************************************/
                                    }else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            //std::cout<<"welcome in vector*vector \n";
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            //std::cout<<"succeful evaluation\n";
                                            /*************************/
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            std::vector<size_t> dimensions ;
                                            //std::cout<<"size of iterator varaibles :"<<iterator_variables_.size()<<"\n";
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    dimensions.push_back(dim);
                                            }
                                            std::vector<size_t> dimensions1(dimensions.begin(),dimensions.end()-1) ;
                                            result = DynamicTensor<Ciphertext>(dimensions1);  
                                            vector<int> vars0_pos ={},vars1_pos ={};
                                            /*********************************/
                                            for (int i = 0; i < iterator_variables_.size(); ++i) {
                                                for (int j = 0; j < vars0.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars0[j])) {
                                                        vars0_pos.push_back(i);
                                                        break;
                                                    }
                                                }

                                                for (int j = 0; j < vars1.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars1[j])) {
                                                        vars1_pos.push_back(i);
                                                        break;
                                                    }
                                                }
                                            }
                                            /*********************************/
                                            /*Initialize result ciphertexts*/
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            std::vector<size_t> ref_iterator_tuple(iterator_tuple.begin(),iterator_tuple.end()-1);
                                            //std::cout<<"==>before entering loop\n";
                                            temp_res=ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple);
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) { 
                                                //std::cout<<"iteration :\n";                                                                                                                                                
                                                int comp = 0;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                std::vector<size_t> sub_iterator_tuple(iterator_tuple.begin(), iterator_tuple.end() - 1);
                                                for(int i =0;i<sub_iterator_tuple.size();i++){
                                                    if(sub_iterator_tuple[i]!=ref_iterator_tuple[i]){
                                                        update=true ;
                                                        break;
                                                    }
                                                }
                                                if(update){   
                                                    std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1); 
                                                    result(ref_iterator_tuple)=temp_res;                            
                                                    ref_iterator_tuple=sub_iterator_tuple;
                                                    temp_res=ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple);
                                                }
                                            });
                                            result(ref_iterator_tuple) = temp_res; 
                                            //std::cout<<"End of iterations\n";
                                    }
                                }
                            break;
                        case Expression::Op_t::add:
                                std::cout<<"\n Realising an Addition \n" ;
                                if(expression.is_reduction()){
                                    std::cout<<"\nRealised operation is a reduction Addition\n";
                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                            
                                            int reduction_size =1 ;
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            if(lhs.type()==Type::plaintxt){
                                                int len = rhs.get_args().size();
                                                reduction_size=reduction_size*(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());
                                                plaintexts = lhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                plaintexts = rhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(lhs) ;
                                                int len = lhs.get_args().size();
                                                reduction_size=reduction_size*(lhs.get_args()[len-1].upper_bound()-lhs.get_args()[len-1].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>({1});
                                            result({0})=SumVec(plaintexts({0})+ciphertexts({0}),reduction_size); 
                                    }
                                    /**********************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                            result = DynamicTensor<Ciphertext>({1});
                                            int len = rhs.get_args().size();
                                            int reduction_size=(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());              
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            std::cout<<"size of reduction in cipher+cipher add :"<<reduction_size<<"\n";
                                            result({0})=SumVec(ciphertexts0({0})+ciphertexts1({0}),reduction_size); 
                                    }
                                    /**********************************************************************/
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                             DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            std::vector<Var> vars0, vars1 ;
                                            if(lhs.type()==Type::plaintxt){
                                                vars1 = lhs.get_compute_args();
                                                plaintexts = lhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                vars1 = rhs.get_compute_args();
                                                plaintexts = rhs.get_plaintexts() ;
                                                vars0 = lhs.get_compute_args();
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            int reduction_size =iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                                            auto [reduction_pos, vars0_pos, vars1_pos] = match_positions(iterator_variables_, reduction_variables_, vars0, vars1);
                                            //std::cout<<"vars0_pos :"<<vars0_pos[0]<<" , "<<vars0_pos[1]<<" \n";
                                            //std::cout<<"vars1_pos :"<<vars1_pos[0]<<" , "<<vars1_pos[1]<<" \n";
                                            //std::cout<<"reduction_pos :"<<reduction_pos[0]<<" , "<<reduction_pos[1]<<" \n\n";

                                            std::vector<std::size_t>   ref_reduction_tuple ={};
                                            for(int i =0;i<reduction_variables_.size();i++){
                                                ref_reduction_tuple.push_back(reduction_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions ;
                                            for(int i=0;i<reduction_variables_.size()-1;i++){
                                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions);  
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            /***************************************************/
                                            int mask_size = iterator_variables_[iterator_variables_.size()-2].upper_bound()-iterator_variables_[iterator_variables_.size()-2].lower_bound();
                                            std::vector<int64_t> mask1(mask_size,0);
                                            mask1[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                            temp_res=SumVec(ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple),reduction_size)*mask1;
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                std::vector<std::size_t> reduction_tuple ;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<reduction_pos.size();i++){
                                                        if(reduction_pos[i]==comp){
                                                            reduction_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                bool row_updated = false;
                                                for(int i =0;i<reduction_tuple.size();i++){
                                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                        update=true ;
                                                        if(i<reduction_tuple.size()-1){
                                                             row_updated = true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                std::vector<int64_t> mask(mask_size,0);
                                                mask[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                                std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                if(update){    
                                                    if(row_updated){    
                                                        std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                        result(sub_ref_reduction_tuple)=temp_res;
                                                        temp_res  = SumVec(ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple),reduction_size)*mask;
                                                    }else{
                                                        temp_res += SumVec(ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple),reduction_size)*mask;
                                                    }
                                                    ref_reduction_tuple=reduction_tuple;
                                                }
                                            });
                                            std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                            result(sub_ref_reduction_tuple) = temp_res;       
                                    }
                                    /***************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /*************************/
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            int reduction_size =iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                                            auto [reduction_pos, vars0_pos, vars1_pos] = match_positions(iterator_variables_, reduction_variables_, vars0, vars1);
                                            std::vector<std::size_t>   ref_reduction_tuple ={};
                                            for(int i =0;i<reduction_variables_.size();i++){
                                                ref_reduction_tuple.push_back(reduction_variables_[i].lower_bound());
                                            }                                             
                                            std::vector<size_t> dimensions ;
                                            for(int i=0;i<reduction_variables_.size()-1;i++){
                                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions);  
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            /************************************************************************/
                                            int mask_size = iterator_variables_[iterator_variables_.size()-2].upper_bound()-iterator_variables_[iterator_variables_.size()-2].lower_bound();
                                            std::vector<int64_t> mask1(mask_size,0);
                                            mask1[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                            temp_res=SumVec(ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple),reduction_size)*mask1;
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {
                                                arg0_tuple ={},arg1_tuple={}, iterator_tuple={};                                                                                                                                   
                                                std::vector<std::size_t> reduction_tuple={};
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<reduction_pos.size();i++){
                                                        if(reduction_pos[i]==comp){
                                                            reduction_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                bool row_updated = false;
                                                for(int i =0;i<reduction_tuple.size();i++){
                                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                        update=true ;
                                                        if(i<reduction_tuple.size()-1){
                                                             row_updated = true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                std::vector<int64_t> mask(reduction_size,0);
                                                mask[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                                std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                if(update){    
                                                    if(row_updated){
                                                        std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                        result(sub_ref_reduction_tuple)=temp_res;
                                                        temp_res  = SumVec(ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple),reduction_size)*mask;
                                                    }else{
                                                        temp_res += SumVec(ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple),reduction_size)*mask;

                                                    }           
                                                    ref_reduction_tuple=reduction_tuple;
                                                }
                                            });
                                            std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                            result(sub_ref_reduction_tuple) = temp_res; 
                                    }
                                }else{
                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                            std::cout<<"welcome in plain cipher add \n";
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            if(lhs.type()==Type::plaintxt){
                                                plaintexts = lhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                plaintexts = rhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            result = DynamicTensor<Ciphertext>({1});
                                            result({0})=plaintexts({0})+ciphertexts({0}); 
                                    }
                                    /***************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                            result = DynamicTensor<Ciphertext>({1});
                                            std::cout<<"calculate first operand  \n";
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            std::cout<<"calculate second operand  \n";
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            result({0})= ciphertexts0({0})+ciphertexts1({0}); 
                                    }
                                    /**************************************************************/                                      
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                            int reduction_size =1 ;
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            std::vector<Var> vars0, vars1 ;
                                            if(lhs.type()==Type::plaintxt){
                                                vars1 = lhs.get_compute_args();
                                                plaintexts = lhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                vars1 = rhs.get_compute_args();
                                                plaintexts = rhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            std::vector<size_t> dimensions ;
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    dimensions.push_back(dim);
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions1(dimensions.begin(),dimensions.end()-1) ;
                                            result = DynamicTensor<Ciphertext>(dimensions1);  
                                            vector<int> vars0_pos ={},vars1_pos ={};
                                            /*********************************/
                                            for (int i = 0; i < iterator_variables_.size(); ++i) {
                                                for (int j = 0; j < vars0.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars0[j])) {
                                                        vars0_pos.push_back(i);
                                                        break;
                                                    }
                                                }

                                                for (int j = 0; j < vars1.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars1[j])) {
                                                        vars1_pos.push_back(i);
                                                        break;
                                                    }
                                                }
                                            }
                                            /*********************************/
                                            /*Initialize result ciphertexts*/
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            std::vector<size_t> ref_iterator_tuple(iterator_tuple.begin(),iterator_tuple.end()-1);
                                            temp_res=ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple);
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                int comp = 0;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                std::vector<size_t> sub_iterator_tuple(iterator_tuple.begin(), iterator_tuple.end() - 1);
                                                for(int i =0;i<sub_iterator_tuple.size();i++){
                                                    if(sub_iterator_tuple[i]!=ref_iterator_tuple[i]){
                                                        update=true ;
                                                        break;
                                                    }
                                                }
                                                if(update){   
                                                    std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1); 
                                                    result(ref_iterator_tuple)=temp_res;                            
                                                    ref_iterator_tuple=iterator_tuple;
                                                    temp_res=ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple);
                                                }
                                            });
                                            result(ref_iterator_tuple) = temp_res; 
                                    }
                                    /**************************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /*************************/
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            std::vector<size_t> dimensions ;
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    dimensions.push_back(dim);
                                            }
                                            std::vector<size_t> dimensions1(dimensions.begin(),dimensions.end()-1) ;
                                            result = DynamicTensor<Ciphertext>(dimensions1);  
                                            vector<int> vars0_pos ={},vars1_pos ={};
                                            /*********************************/
                                            for (int i = 0; i < iterator_variables_.size(); ++i) {
                                                for (int j = 0; j < vars0.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars0[j])) {
                                                        vars0_pos.push_back(i);
                                                        break;
                                                    }
                                                }

                                                for (int j = 0; j < vars1.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars1[j])) {
                                                        vars1_pos.push_back(i);
                                                        break;
                                                    }
                                                }
                                            }
                                            /*********************************/
                                            /*Initialize result ciphertexts*/
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            std::vector<size_t> ref_iterator_tuple(iterator_tuple.begin(),iterator_tuple.end()-1);
                                            temp_res=ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple);
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {  
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};                                                                                                                                               
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                std::vector<size_t> sub_iterator_tuple(iterator_tuple.begin(), iterator_tuple.end() - 1);
                                                for(int i =0;i<sub_iterator_tuple.size();i++){
                                                    if(sub_iterator_tuple[i]!=ref_iterator_tuple[i]){
                                                        update=true ;
                                                        break;
                                                    }
                                                }
                                                if(update){   
                                                    std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1); 
                                                    result(ref_iterator_tuple)=temp_res;                            
                                                    ref_iterator_tuple=iterator_tuple;
                                                    temp_res=ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple);
                                                }
                                            });
                                            result(ref_iterator_tuple) = temp_res; 
                                    }
                                }
                            break;
                        /*********************************************************************/
                        case Expression::Op_t::sub:
                            std::cout<<"\n Realising a Susbstraction \n" ;
                               if(expression.is_reduction()){
                                    //std::cout<<"\nRealised operation is a reduction Addition\n";
                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                            int reduction_size =1 ;
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            result = DynamicTensor<Ciphertext>({1});
                                            if(lhs.type()==Type::plaintxt){
                                                int len = rhs.get_args().size();
                                                reduction_size=reduction_size*(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());
                                                plaintexts = lhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(rhs) ;
                                                result({0})=SumVec(plaintexts({0})-ciphertexts({0}),reduction_size); 

                                            }else{
                                                plaintexts = rhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(lhs) ;
                                                int len = lhs.get_args().size();
                                                reduction_size=reduction_size*(lhs.get_args()[len-1].upper_bound()-lhs.get_args()[len-1].lower_bound());
                                                result({0})=SumVec(ciphertexts({0})-plaintexts({0}),reduction_size); 
                                            }
                                    }
                                    /**********************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                            result = DynamicTensor<Ciphertext>({1});
                                            int len = rhs.get_args().size();
                                            int reduction_size=(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());              
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            std::cout<<"size of reduction in cipher-cipher sub :"<<reduction_size<<"\n";
                                            int reduction_size1 = rhs.get_args()[0].upper_bound()-rhs.get_args()[0].lower_bound();
                                            std::cout<<"mutated reduction is :"<<reduction_size1<<"\n";
                                            result({0})=SumVec(ciphertexts0({0})-ciphertexts1({0}),reduction_size); 
                                    }
                                    /**********************************************************************/
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                             DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            std::vector<Var> vars0, vars1 ;
                                            if(lhs.type()==Type::plaintxt){
                                                vars1 = lhs.get_compute_args();
                                                plaintexts = lhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                vars1 = rhs.get_compute_args();
                                                plaintexts = rhs.get_plaintexts() ;
                                                vars0 = lhs.get_compute_args();
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            int reduction_size =iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                                            auto [reduction_pos, vars0_pos, vars1_pos] = match_positions(iterator_variables_, reduction_variables_, vars0, vars1);
                                            //std::cout<<"vars0_pos :"<<vars0_pos[0]<<" , "<<vars0_pos[1]<<" \n";
                                            //std::cout<<"vars1_pos :"<<vars1_pos[0]<<" , "<<vars1_pos[1]<<" \n";
                                            //std::cout<<"reduction_pos :"<<reduction_pos[0]<<" , "<<reduction_pos[1]<<" \n\n";

                                            std::vector<std::size_t>   ref_reduction_tuple ={};
                                            for(int i =0;i<reduction_variables_.size();i++){
                                                ref_reduction_tuple.push_back(reduction_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions ;
                                            for(int i=0;i<reduction_variables_.size()-1;i++){
                                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions);  
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            /***************************************************/
                                            int mask_size = iterator_variables_[iterator_variables_.size()-2].upper_bound()-iterator_variables_[iterator_variables_.size()-2].lower_bound();
                                            std::vector<int64_t> mask1(mask_size,0);
                                            mask1[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                            if(lhs.type()==Type::plaintxt){
                                                temp_res=SumVec(plaintexts(sub_arg1_tuple)-ciphertexts(sub_arg0_tuple),reduction_size)*mask1;
                                            }else{
                                                temp_res=SumVec(ciphertexts(sub_arg0_tuple)-plaintexts(sub_arg1_tuple),reduction_size)*mask1;
                                            }
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                std::vector<std::size_t> reduction_tuple ;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<reduction_pos.size();i++){
                                                        if(reduction_pos[i]==comp){
                                                            reduction_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                bool row_updated = false;
                                                for(int i =0;i<reduction_tuple.size();i++){
                                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                        update=true ;
                                                        if(i<reduction_tuple.size()-1){
                                                             row_updated = true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                std::vector<int64_t> mask(mask_size,0);
                                                mask[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                                std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                if(update){    
                                                    if(row_updated){    
                                                        std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                        result(sub_ref_reduction_tuple)=temp_res;
                                                        if(lhs.type()==Type::plaintxt){
                                                             temp_res = SumVec(plaintexts(sub_arg1_tuple) - ciphertexts(sub_arg0_tuple),reduction_size)*mask;
                                                        }else{
                                                             temp_res = SumVec(ciphertexts(sub_arg0_tuple) - plaintexts(sub_arg1_tuple),reduction_size)*mask;
                                                        }
                                                    }else{
                                                        if(lhs.type()==Type::plaintxt){
                                                             temp_res += SumVec(plaintexts(sub_arg1_tuple) - ciphertexts(sub_arg0_tuple),reduction_size)*mask;
                                                        }else{
                                                             temp_res += SumVec(ciphertexts(sub_arg0_tuple) - plaintexts(sub_arg1_tuple),reduction_size)*mask;
                                                        }
                                                    }
                                                    ref_reduction_tuple=reduction_tuple;
                                                }
                                            });
                                            std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                            result(sub_ref_reduction_tuple) = temp_res;       
                                    }
                                    /***************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /*************************/
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            int reduction_size =iterator_variables_[iterator_variables_.size()-1].upper_bound()-iterator_variables_[iterator_variables_.size()-1].lower_bound();
                                            auto [reduction_pos, vars0_pos, vars1_pos] = match_positions(iterator_variables_, reduction_variables_, vars0, vars1);
                                            std::vector<std::size_t>   ref_reduction_tuple ={};
                                            for(int i =0;i<reduction_variables_.size();i++){
                                                ref_reduction_tuple.push_back(reduction_variables_[i].lower_bound());
                                            }                                             
                                            std::vector<size_t> dimensions ;
                                            for(int i=0;i<reduction_variables_.size()-1;i++){
                                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions);  
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            /************************************************************************/
                                            int mask_size = iterator_variables_[iterator_variables_.size()-2].upper_bound()-iterator_variables_[iterator_variables_.size()-2].lower_bound();
                                            std::vector<int64_t> mask1(mask_size,0);
                                            mask1[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                            temp_res=SumVec(ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple),reduction_size)*mask1;
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {
                                                arg0_tuple ={},arg1_tuple={}, iterator_tuple={};                                                                                                                                   
                                                std::vector<std::size_t> reduction_tuple={};
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<reduction_pos.size();i++){
                                                        if(reduction_pos[i]==comp){
                                                            reduction_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                bool row_updated = false;
                                                for(int i =0;i<reduction_tuple.size();i++){
                                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                        update=true ;
                                                        if(i<reduction_tuple.size()-1){
                                                             row_updated = true;
                                                        }
                                                        break;
                                                    }
                                                }
                                                std::vector<int64_t> mask(reduction_size,0);
                                                mask[iterator_tuple[iterator_tuple.size()-2]] = 1;
                                                std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                if(update){    
                                                    if(row_updated){
                                                        std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                        result(sub_ref_reduction_tuple)=temp_res;
                                                        temp_res  = SumVec(ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple),reduction_size)*mask;
                                                    }else{
                                                        temp_res += SumVec(ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple),reduction_size)*mask;

                                                    }           
                                                    ref_reduction_tuple=reduction_tuple;
                                                }
                                            });
                                            std::vector<size_t> sub_ref_reduction_tuple(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                            result(sub_ref_reduction_tuple) = temp_res; 
                                    }
                                }else{
                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                            std::cout<<"welcome in plain cipher add \n";
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            result = DynamicTensor<Ciphertext>({1});
                                            if(lhs.type()==Type::plaintxt){
                                                plaintexts = lhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(rhs) ;
                                                result({0})=plaintexts({0})-ciphertexts({0}); 
                                            }else{
                                                plaintexts = rhs.get_plaintexts() ;
                                                ciphertexts = evaluate_expression(lhs) ;
                                                result({0})=ciphertexts({0})-plaintexts({0}); 
                                            }
                                    }
                                    /***************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                            
                                            result = DynamicTensor<Ciphertext>({1});
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            result({0})= ciphertexts0({0})-ciphertexts1({0}); 
                                    }
                                    /**************************************************************/                                      
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                            int reduction_size =1 ;
                                            DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                            std::vector<Var> vars0, vars1 ;
                                            if(lhs.type()==Type::plaintxt){
                                                vars1 = lhs.get_compute_args();
                                                plaintexts = lhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(rhs) ;
                                            }else{
                                                vars1 = rhs.get_compute_args();
                                                plaintexts = rhs.get_plaintexts() ;
                                                vars0 = rhs.get_compute_args();
                                                ciphertexts = evaluate_expression(lhs) ;
                                            }
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            std::vector<size_t> dimensions ;
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    dimensions.push_back(dim);
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                            }
                                            std::vector<size_t> dimensions1(dimensions.begin(),dimensions.end()-1) ;
                                            result = DynamicTensor<Ciphertext>(dimensions1);  
                                            vector<int> vars0_pos ={},vars1_pos ={};
                                            /*********************************/
                                            for (int i = 0; i < iterator_variables_.size(); ++i) {
                                                for (int j = 0; j < vars0.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars0[j])) {
                                                        vars0_pos.push_back(i);
                                                        break;
                                                    }
                                                }

                                                for (int j = 0; j < vars1.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars1[j])) {
                                                        vars1_pos.push_back(i);
                                                        break;
                                                    }
                                                }
                                            }
                                            /*********************************/
                                            /*Initialize result ciphertexts*/
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            std::vector<size_t> ref_iterator_tuple(iterator_tuple.begin(),iterator_tuple.end()-1);
                                            if(lhs.type()==Type::plaintxt){
                                                temp_res=plaintexts(sub_arg1_tuple)-ciphertexts(sub_arg0_tuple);
                                            }else{
                                                temp_res=ciphertexts(sub_arg0_tuple)-plaintexts(sub_arg1_tuple);
                                            }
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                int comp = 0;
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                std::vector<size_t> sub_iterator_tuple(iterator_tuple.begin(), iterator_tuple.end() - 1);
                                                for(int i =0;i<sub_iterator_tuple.size();i++){
                                                    if(sub_iterator_tuple[i]!=ref_iterator_tuple[i]){
                                                        update=true ;
                                                        break;
                                                    }
                                                }
                                                if(update){   
                                                    std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1); 
                                                    result(ref_iterator_tuple)=temp_res;                            
                                                    ref_iterator_tuple=iterator_tuple;
                                                    if(lhs.type()==Type::plaintxt){
                                                         temp_res=plaintexts(sub_arg1_tuple)-ciphertexts(sub_arg0_tuple);
                                                    }else{
                                                         temp_res=ciphertexts(sub_arg0_tuple)-plaintexts(sub_arg1_tuple);
                                                    }
                                                }
                                            });
                                            result(ref_iterator_tuple) = temp_res; 
                                    }
                                    /**************************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /*************************/
                                            std::vector<std::pair<int, int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            std::vector<size_t> dimensions ;
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    pair<int, int> tup = {0,dim};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    dimensions.push_back(dim);
                                            }
                                            std::vector<size_t> dimensions1(dimensions.begin(),dimensions.end()-1) ;
                                            result = DynamicTensor<Ciphertext>(dimensions1);  
                                            vector<int> vars0_pos ={},vars1_pos ={};
                                            /*********************************/
                                            for (int i = 0; i < iterator_variables_.size(); ++i) {
                                                for (int j = 0; j < vars0.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars0[j])) {
                                                        vars0_pos.push_back(i);
                                                        break;
                                                    }
                                                }

                                                for (int j = 0; j < vars1.size(); ++j) {
                                                    if (iterator_variables_[i].same_as(vars1[j])) {
                                                        vars1_pos.push_back(i);
                                                        break;
                                                    }
                                                }
                                            }
                                            /*********************************/
                                            /*Initialize result ciphertexts*/
                                            int comp = 0 ;
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple ;
                                            for (auto val : iterator_tuple){
                                                for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                }
                                                for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                }
                                                comp+=1;
                                            }
                                            Ciphertext temp_res ; 
                                            std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                            std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                            std::vector<size_t> ref_iterator_tuple(iterator_tuple.begin(),iterator_tuple.end()-1);
                                            temp_res=ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple);
                                            generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {  
                                                iterator_tuple ={},arg1_tuple={},arg0_tuple={};                                                                                                                                               
                                                int comp = 0;
                                                for (int val : iterators) {
                                                    iterator_tuple.push_back(val);
                                                    for(int i=0;i<vars0_pos.size();i++){
                                                        if(vars0_pos[i]==comp){
                                                            arg0_tuple.push_back(val);
                                                        }
                                                    }
                                                    for(int i=0;i<vars1_pos.size();i++){
                                                        if(vars1_pos[i]==comp){
                                                            arg1_tuple.push_back(val);
                                                        }
                                                    }
                                                    comp+=1;
                                                }  
                                                bool update = false;
                                                std::vector<size_t> sub_iterator_tuple(iterator_tuple.begin(), iterator_tuple.end() - 1);
                                                for(int i =0;i<sub_iterator_tuple.size();i++){
                                                    if(sub_iterator_tuple[i]!=ref_iterator_tuple[i]){
                                                        update=true ;
                                                        break;
                                                    }
                                                }
                                                if(update){   
                                                    std::vector<size_t> sub_arg0_tuple(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    std::vector<size_t> sub_arg1_tuple(arg1_tuple.begin(), arg1_tuple.end() - 1); 
                                                    result(ref_iterator_tuple)=temp_res;                            
                                                    ref_iterator_tuple=iterator_tuple;
                                                    temp_res=ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple);
                                                }
                                            });
                                            result(ref_iterator_tuple) = temp_res; 
                                    }
                                }
                            break;
                        /**************************************************************************************/
                        /**************************************************************************************
                        case Expression::Op_t::mod_switch:
                            // Implement your specific logic for this operation
                            break;
                        /**************************************************************************************/
                        case Expression::Op_t::o_none:
                        default:
                            throw std::runtime_error("Undefined operation. Or unsupporetd operation");
                    }
                    //std::cout<<"\n Returning the result in evaluate_expression\n";
                    expression.set_is_evaluated(true);
                    expression.set_ciphertexts(result);
                    return evaluate_expression(expression);
                } else {
                    throw std::runtime_error("Expression cannot be evaluated.");
                }
            }else{
                throw invalid_argument("This expression is undefined \n");
            }
    }
    /***************************************************************************************************************************************/
    /***************************************************************************************************************************************/
    void Computation::evaluate(bool is_output){
        std::vector<Var> expr_compute_vars = expression_.get_compute_args();
        bool have_same_variables = true ; 
        bool found = false ; 
        // verify that the expression to evaluate and the computation have the same computation variables
        if (expr_compute_vars.size()!=iterator_variables_.size()){
            have_same_variables = false ; 
        }else{
            for(int i =0;i<expr_compute_vars.size();i++){
                found = false ;
                for(int j=0; j < iterator_variables_.size(); j++){
                    if(expr_compute_vars[i].same_as(iterator_variables_[j])){
                        found=true;
                        break;
                    }
                }
                if(!found){
                    have_same_variables = false ;
                    break;
                }
            }
        }
        if(have_same_variables){
            Expression input_expression = expression_ ;
            std::cout<<"\n start expression evaluation  \n";
            DynamicTensor<Ciphertext> ciphertexts = evaluate_expression(input_expression);
            std::cout<<"\n Return from expression evaluation \n";
            expression_.set_is_evaluated(true);
            if(is_output){
                    if (expression_.type()==Type::ciphertxt) {
                        ciphertexts({0}).set_output(name_);
                    }else{
                        vector<size_t> dimensions ;
                        if(is_reduction_){
                            for (int i =0; i< reduction_variables_.size()-1;i++) {
                                dimensions.push_back(reduction_variables_[i].upper_bound() - reduction_variables_[i].lower_bound());
                            } 
                        }else{
                            for (int i =0; i< iterator_variables_.size()-1;i++) {
                                dimensions.push_back(iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound());
                            } 
                        }
                        std::vector<std::pair<int, int>> ranges = { };
                        for(size_t i = 0 ;i<dimensions.size();i++){
                            pair<int, int> dim = {0,dimensions[i]};
                            ranges.push_back(dim);
                        }
                        /********************************************************/
                        generateNestedLoops(ranges,[&ciphertexts,this](const std::vector<int>& iterators) {
                            std::vector<std::size_t> iterator_tuple ;
                            std::string name=name_;
                            int comp = 0 ;
                            for (int val : iterators) {
                                iterator_tuple.push_back(val);
                                name+="["+std::to_string(iterator_tuple[comp])+"]";
                                comp+=1;
                            }
                            ciphertexts(iterator_tuple).set_output(name);
                        });                    
                    }
                }
                expression_.set_ciphertexts(ciphertexts);

        }else{
            throw invalid_argument("Provided expression is invalide");
        }   
        
    }
}