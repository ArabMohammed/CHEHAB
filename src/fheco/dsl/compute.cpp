 
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <iostream>
#include "fheco/dsl/compute.hpp"
#include "fheco/dsl/ops_overloads.hpp"
#include "fheco/dsl/tensor.hpp"
#include "fheco/dsl/compiler.hpp"
#include <cmath>
using namespace std ;
namespace fheco {

    /*************************************************************************************************************/
    Computation::Computation(const std::string &name , const std::vector<Var> iterator_variables, const Expression& expression): 
        name_(name), iterator_variables_(iterator_variables){
            if(expression.type()!=Type::ciphertxt){
                throw invalid_argument("this constructot support only expressions of type ciphertext");
            }
            expression_= expression ;

        }
    Computation::Computation(const std::string &name, const std::vector<Var> iterator_variables,
                            const std::vector<Var> output_dim_variables ,const Expression &expression):
                            name_(name), iterator_variables_(iterator_variables),
                            output_dim_variables_(output_dim_variables)
        {
            expression_ = expression ;
            if(iterator_variables.size()>output_dim_variables.size()){
                is_reduction_=true;
                expression_.set_is_reduction(true);                        
            }else if(iterator_variables.size() < output_dim_variables.size()){
                throw invalid_argument("there are invalid variables in the output_dim_variables");
            }
        }
    /*********************************************************************************************** 
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
    /***************************************************************************************************/
    // Copy constructor 
    Computation::Computation(const Computation &other) : 
        name_(other.name_), iterator_variables_(other.iterator_variables_),
        expression_(other.expression_),output_dim_variables_(other.output_dim_variables_)
        {}
    // Move constructor
    Computation::Computation(Computation &&other) noexcept:  
        name_(std::move(other.name_)), iterator_variables_(std::move(other.iterator_variables_)),
        output_dim_variables_(std::move(other.output_dim_variables_)),expression_(std::move(other.expression_))
        {}
    // Move assignment operator 
    Computation &Computation::operator=(Computation &&other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            iterator_variables_ = std::move(other.iterator_variables_);
            output_dim_variables_ = std::move(other.output_dim_variables_);
            expression_ = std::move(other.expression_);
        }
        return *this;
    }
            
    // Copy assignment operator
    Computation &Computation::operator=(const Computation &other){
         if (this != &other) {
            name_ = other.name_;
            iterator_variables_ = other.iterator_variables_;
            output_dim_variables_ = other.output_dim_variables_;
            expression_ = other.expression_;
            is_reduction_ = other.is_reduction_ ;
        }
        return *this;
    }
    /*************************************************************************************
    void Computation::resize(std::vector<Var> new_dimensions_vars){
        if(!expression_.is_evaluated()){
            throw invalid_argument("Computation must be evaluated before resizing \n");
        }
        int old_total_length = 1 ;
        int new_total_length = 1 ,vector_size =1;
        for (int i =0;i<iterator_variables_.size();i++){
            old_total_length*=(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
        }
        for (int i =0;i<new_dimensions_vars.size();i++){
            new_total_length*=(new_dimensions_vars[i].upper_bound()-new_dimensions_vars[i].lower_bound());
            vector_size = new_dimensions_vars[i].upper_bound()-new_dimensions_vars[i].lower_bound() ;
        }
        if(new_total_length<old_total_length){
            throw invalid_argument("resizing impossible because : new_total_dim < old_total_dim");
        }
        if (expression_.type()==Type::vectorciphertxt){
            DynamicTensor<Ciphertext> compute_ciphertexts = expression_.get_ciphertexts();
            DynamicTensor<Ciphertext> resized_ciphertexts ;
            int total_iterations = ceil(static_cast<double>(old_total_length) / vector_size);
            
            std::vector<std::vector<int>> ranges = {};
            std::vector<Var> sub_iterator_variables(new_dimensions_vars.begin(), new_dimensions_vars.end() - 1);
            iterator_variables_ = new_dimensions_vars ;
            if(sub_iterator_variables.size()==0){
                    //std::cout<<"add new input here \n";
                    resized_ciphertexts = DynamicTensor<Ciphertext>({1});
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
                        return false ;output_dim_variables_
                    }
                    return true; 
                });
            }
        }else{
            throw invalid_argument("you con only resize computation of type vectorciphert\n");
        }
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
                std::vector<size_t> dimensions ={};
                int slot_count = expresssion_args[expresssion_args.size()-1].upper_bound()-expresssion_args[expresssion_args.size()-1].lower_bound();
                for(int i =0; i<expresssion_args.size()-1;i++){
                    dimensions.push_back(expresssion_args[i].upper_bound()-expresssion_args[i].lower_bound());
                }
                
                DynamicTensor<Ciphertext> ciphertexts = expression_.get_ciphertexts();
                DynamicTensor<Ciphertext> updated_ciphertexts(dimensions);

                for(int i = 0 ; i< compute_args.size() ; i++){
                    std::vector<std::vector<int>> ranges = {};
                    std::vector<int> range = {};
                    int vector_size = 0 ;
                    if (compute_args[i].rotation_steps()!=0){
                        int rotation_step = compute_args[i].rotation_steps();
                        int end=0;
                        if(i!=compute_args.size()-1)
                            end=i+1;
                        else
                            end=i;
                        for (int j=0; j<end; j++) {
                            int dim = output_dim_variables_[j].upper_bound() - output_dim_variables_[j].lower_bound();
                            range = {0,dim,output_dim_variables_[j].increment_step()};
                            ranges.push_back(range);
                            vector_size = dim ;
                        }
                        /************Update ciphertexts on one dimension**********/
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
                            if( i != compute_args.size()-1 ){
                                updated_ciphertexts.assign_subtensor(iterator_tuple,ciphertexts.subtensor(rotated_iterator_tuple));
                            }else{
                                updated_ciphertexts(iterator_tuple)=ciphertexts(iterator_tuple) << (rotation_step%slot_count) ;
                            }
                            return true ;
                        });
                    }

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
    void print_location(std::vector<size_t> list1,std::vector<size_t>list2,std::vector<size_t> list3){
        std::cout<<"reduction_op :";
        for(auto elem : list1){
            std::cout<<elem<<" ";
        }
        std::cout<<" ,operand0 :";
        for(auto elem : list2){
            std::cout<<elem<<" ";
        }
        std::cout<<" ,operand1 :";
        for(auto elem : list3){
            std::cout<<elem<<" ";
        }
        std::cout<<"\n";
    }
    DynamicTensor<Ciphertext> Computation::evaluate_expression(Expression& expression ){
            if (expression.is_defined()){
                if (expression.is_evaluated()) {
                    /*****assignment with reduction *****/
                    if(expression.is_reduction()){
                        std::vector<Var> vars0 = expression.get_compute_args();
                        DynamicTensor<Ciphertext> ciphertexts0 = expression.get_ciphertexts();
                        /**********************************************/
                        int ref_sum = 0;
                        std::vector<std::vector<int>> ranges = {};
                        std::vector<size_t> iterator_tuple ={};
                        for (int i=0; i<iterator_variables_.size(); i++) {
                                int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                ranges.push_back(tup);
                                iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                ref_sum+=iterator_variables_[i].lower_bound();
                        }
                        /********************************************/
                        std::vector<size_t> dimensions ;
                        for(int i=0; i<output_dim_variables_.size()-1 ; i++){
                            dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                        }
                        /*********/DynamicTensor<Ciphertext> result = DynamicTensor<Ciphertext>(dimensions); 
                        /*********************************************/
                        int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                        vector<integer> mask ;
                        bool is_vectorization_possible = false ; 
                        int reduction_size = Compiler::active_func()->slot_count();
                        if(reduction_size>1){
                            is_vectorization_possible = true ;
                        }    
                        /*******************************************/    
                        std::vector<std::size_t> arg0_tuple={}, sub_arg0_tuple ={} , sub_ref_arg0_tuple={};
                        Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                        Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                        Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                        bool is_arg0_vectorizble = true ;
                        /**************************************************************************/
                        std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                        generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                            std::vector<std::size_t> reduction_tuple ;
                            iterator_tuple ={},arg0_tuple={};
                            int comp =0 ;
                            for (int val : iterators) {
                                iterator_tuple.push_back(val);
                                comp+=val;
                            } 
                            /****************************************************************************/
                            arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                            reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                            int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                            if(arg0_tuple.size() > 1){
                                sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                            }else{ sub_arg0_tuple={0};}
                            /******************************************************************************/
                            if(comp==ref_sum){
                                sub_ref_arg0_tuple = sub_arg0_tuple ;
                                ref_reduction_tuple = reduction_tuple ;
                                temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0);
                            }else{                                    
                                bool update = false;
                                bool row_updated = false ;
                                for(int i =0;i<reduction_tuple.size();i++){
                                    if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                        update=true ;
                                        if(i<reduction_tuple.size()-1){
                                            row_updated = true;
                                        }
                                        break;
                                    }
                                }
                                if(update){    
                                    mask = vector<integer>(mask_size,0); 
                                    mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                    vectorized_row += temp_res*mask ;
                                    if(row_updated==true){
                                        std::cout<<"row updated \n";
                                        sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                        result(sub_ref_reduction_tuple)=vectorized_row;
                                        vectorized_row = Ciphertext(PackedVal(1,0));
                                    }
                                    ref_reduction_tuple=reduction_tuple;
                                    temp_res = Ciphertext(PackedVal(1,0));
                                }
                                /****************************************************/
                                if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                    sub_ref_arg0_tuple = sub_arg0_tuple ;
                                    temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0);
                                }else{
                                    is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                    temp_res_red+=(ciphertexts0(sub_arg0_tuple)<<rotation0);    
                                }
                            }
                            /***************************************************/
                            if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                if(is_arg0_vectorizble&&is_vectorization_possible){
                                    temp_res += SumVec(ciphertexts0(sub_arg0_tuple),reduction_size);
                                }else{
                                    temp_res+=temp_res_red;
                                }   
                                is_arg0_vectorizble = true ;
                            }
                            
                            return true ;
                        });
                        mask = vector<integer>(mask_size,0); 
                        mask[mask_size-1]=1;
                        vectorized_row += temp_res*mask ;
                        sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                        result(sub_ref_reduction_tuple) = vectorized_row; 
                        return result ;
                    }
                    /*********************************************************************/
                    else{
                        return expression.get_ciphertexts();
                    }
                }
                if (expression.op()!=Expression::Op_t::o_none) {
                    DynamicTensor<Ciphertext> result ;
                    Expression lhs = expression.get_operands()[0] ;
                    Expression rhs = expression.get_operands()[1];
                    switch(expression.op()) {
                        case Expression::Op_t::mul:
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
                                        if(expression.is_reduction()){    
                                            result({0})=SumVec(plaintexts({0})*ciphertexts({0}),reduction_size); 
                                        }else{
                                            result({0})=plaintexts({0})*ciphertexts({0}); 
                                        }
                                    }
                                    /*************************************************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                        int len = rhs.get_args().size();
                                            int reduction_size=(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());                                            
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            result = DynamicTensor<Ciphertext>({1});
                                        if(expression.is_reduction()){
                                            result({0})=SumVec(ciphertexts0({0})*ciphertexts1({0}),reduction_size); 
                                        }{
                                            result({0})= ciphertexts0({0})*ciphertexts1({0}); 
                                        }
                                    }
                                    /************************************************************************************************/
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                            std::cout<<"welcome in plaintext * vectorciphertext multiplication \n";
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
                                            /**********************************************/
                                            int ref_sum = 0;
                                            std::vector<std::vector<int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    ref_sum+=iterator_variables_[i].lower_bound();
                                            }
                                            /********************************************/
                                            std::vector<size_t> dimensions ;
                                            for(int i=0; i<output_dim_variables_.size() ; i++){
                                                dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions); 
                                            /**********************************************/
                                            int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                                            vector<integer> mask ;
                                            bool is_vectorization_possible = false ; 
                                            int reduction_size = Compiler::active_func()->slot_count();
                                            if(vars0[vars0.size()-1].same_as(vars1[vars1.size()-1])&&(reduction_size>1)){
                                                is_vectorization_possible = true ;
                                            }    
                                            /*******************************************/    
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple = {} , sub_arg0_tuple ={}, sub_arg1_tuple={};
                                            Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                                            Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                                            Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                                            std::vector<size_t> sub_ref_arg0_tuple, sub_ref_arg1_tuple ;
                                            bool is_arg0_vectorizble = true , is_arg1_vectorizble = true ;
                                            /******************************************************************/
                                            if(expression.is_reduction()){
                                                std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    std::vector<std::size_t> reduction_tuple ;
                                                    iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /******************************************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_reduction_tuple = reduction_tuple ;
                                                        temp_res_red = (ciphertexts(sub_arg0_tuple)<<rotation0)*(plaintexts(sub_arg1_tuple)<<rotation1) ;
                                                    }else{                                    
                                                        bool update = false;
                                                        bool row_updated = false ;
                                                        for(int i =0;i<reduction_tuple.size();i++){
                                                            if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                                update=true ;
                                                                if(i<reduction_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(update){    
                                                            mask = vector<integer>(mask_size,0); 
                                                            mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                                            vectorized_row += temp_res*mask ;
                                                            if(row_updated==true){
                                                                std::cout<<"row updated \n";
                                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                                result(sub_ref_reduction_tuple)=vectorized_row;
                                                                vectorized_row = Ciphertext(PackedVal(1,0));
                                                            }
                                                            ref_reduction_tuple=reduction_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res_red = (ciphertexts(sub_arg0_tuple)<<rotation0)*(plaintexts(sub_arg1_tuple)<<rotation1);
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res_red+=(ciphertexts(sub_arg0_tuple)<<rotation0)*(plaintexts(sub_arg1_tuple)<<rotation1) ;    
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res += SumVec(ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple),reduction_size);
                                                        }else{
                                                            temp_res+=temp_res_red;
                                                        }   
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    
                                                    return true ;
                                                });
                                                mask = vector<integer>(mask_size,0); 
                                                mask[mask_size-1]=1;
                                                vectorized_row += temp_res*mask ;
                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                                                result(sub_ref_reduction_tuple) = vectorized_row; 
                                            }
                                            /***************************************************************/
                                            /***************************************************************/
                                            else{
                                                std::vector<std::size_t> result_tuple = {} ,sub_ref_result_tuple={} , ref_result_tuple = {};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    iterator_tuple ={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    result_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /*****************************************************************************/
                                                    mask = vector<integer>(mask_size,0); 
                                                    mask[result_tuple[ref_result_tuple.size()-1]]=1;
                                                    /************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_result_tuple = result_tuple ;
                                                        temp_res = (ciphertexts(sub_arg0_tuple)<<rotation0)*(plaintexts(sub_arg1_tuple)<<rotation1)*mask ;
                                                    }else{                                    
                                                        bool row_updated = false ;
                                                        for(int i =0;i<result_tuple.size();i++){
                                                            if(result_tuple[i]!=ref_result_tuple[i]){
                                                                if(i<result_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(row_updated){    
                                                            sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                            result(sub_ref_result_tuple)=temp_res ;
                                                            ref_result_tuple=result_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res = (ciphertexts(sub_arg0_tuple)<<rotation0)*(plaintexts(sub_arg1_tuple)<<rotation1)*mask ;
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res +=(ciphertexts(sub_arg0_tuple)<<rotation0)*(plaintexts(sub_arg1_tuple)<<rotation1)*mask ;  
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res = ciphertexts(sub_arg0_tuple)*plaintexts(sub_arg1_tuple) ;
                                                        }  
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    return true ;
                                                });
                                                sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                result(sub_ref_result_tuple)=temp_res ;
                                            }
                                    }
                                    /************************************************************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::cout<<"welcome in vectorcipher*vectocipher \n";
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /**********************************************/
                                            int ref_sum = 0;
                                            std::vector<std::vector<int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    ref_sum+=iterator_variables_[i].lower_bound();
                                            }
                                            /********************************************/
                                            std::vector<size_t> dimensions ;
                                            for(int i=0; i<output_dim_variables_.size()-1 ; i++){
                                                dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions); 
                                            /*********************************************/
                                            int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                                            vector<integer> mask ;
                                            bool is_vectorization_possible = false ; 
                                            int reduction_size = Compiler::active_func()->slot_count();
                                            if(vars0[vars0.size()-1].same_as(vars1[vars1.size()-1])&&(reduction_size>1)){
                                                is_vectorization_possible = true ;
                                            }    
                                            /*******************************************/    
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple = {} , sub_arg0_tuple ={}, sub_arg1_tuple={};
                                            Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                                            Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                                            Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                                            std::vector<size_t> sub_ref_arg0_tuple, sub_ref_arg1_tuple ;
                                            bool is_arg0_vectorizble = true , is_arg1_vectorizble = true ;
                                            /******************************************************************/
                                            if(expression.is_reduction()){
                                                std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    std::vector<std::size_t> reduction_tuple ;
                                                    iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /******************************************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_reduction_tuple = reduction_tuple ;
                                                        temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0)*(ciphertexts1(sub_arg1_tuple)<<rotation1) ;
                                                    }else{                                    
                                                        bool update = false;
                                                        bool row_updated = false ;
                                                        for(int i =0;i<reduction_tuple.size();i++){
                                                            if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                                update=true ;
                                                                if(i<reduction_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(update){    
                                                            mask = vector<integer>(mask_size,0); 
                                                            mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                                            vectorized_row += temp_res*mask ;
                                                            if(row_updated==true){
                                                                std::cout<<"row updated \n";
                                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                                result(sub_ref_reduction_tuple)=vectorized_row;
                                                                vectorized_row = Ciphertext(PackedVal(1,0));
                                                            }
                                                            ref_reduction_tuple=reduction_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0)*(ciphertexts1(sub_arg1_tuple)<<rotation1);
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res_red+=(ciphertexts0(sub_arg0_tuple)<<rotation0)*(ciphertexts1(sub_arg1_tuple)<<rotation1) ;    
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res += SumVec(ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple),reduction_size);
                                                        }else{
                                                            temp_res+=temp_res_red;
                                                        }   
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    
                                                    return true ;
                                                });
                                                mask = vector<integer>(mask_size,0); 
                                                mask[mask_size-1]=1;
                                                vectorized_row += temp_res*mask ;
                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                                                result(sub_ref_reduction_tuple) = vectorized_row; 
                                            }
                                            /***************************************************************/
                                            /***************************************************************/
                                            else{
                                                std::cout<<"this evaluation is not a reduction  \n";
                                                std::vector<std::size_t> result_tuple = {} ,sub_ref_result_tuple={} , ref_result_tuple = {};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    iterator_tuple ={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    std::cout<<"evaluate arguments\n";
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    result_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /*****************************************************************************/
                                                    mask = vector<integer>(mask_size,0); 
                                                    mask[result_tuple[ref_result_tuple.size()-1]]=1;
                                                    /************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_result_tuple = result_tuple ;
                                                        //std::cout<<"first evaluation\n";
                                                        temp_res = (ciphertexts0(sub_arg0_tuple)<<rotation0)*(ciphertexts1(sub_arg1_tuple)<<rotation1)*mask ;
                                                    }else{        
                                                        //std::cout<<"following evaluations \n";                            
                                                        bool row_updated = false ;
                                                        for(int i =0;i<result_tuple.size();i++){
                                                            if(result_tuple[i]!=ref_result_tuple[i]){
                                                                if(i<result_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(row_updated){    
                                                            sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                            result(sub_ref_result_tuple)=temp_res ;
                                                            ref_result_tuple=result_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res = (ciphertexts0(sub_arg0_tuple)<<rotation0)*(ciphertexts1(sub_arg1_tuple)<<rotation1)*mask ;
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res +=(ciphertexts0(sub_arg0_tuple)<<rotation0)*(ciphertexts1(sub_arg1_tuple)<<rotation1)*mask ;  
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            std::cout<<"vectoror product possible \n";
                                                            temp_res = ciphertexts0(sub_arg0_tuple)*ciphertexts1(sub_arg1_tuple) ;
                                                        }  
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    return true ;
                                                });
                                                sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                result(sub_ref_result_tuple)=temp_res ;
                                            }
                                    }
                            break;       
                         /**************************************************************************************/
                         /**************************************************************************************/
                         case Expression::Op_t::add:
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
                                        if(expression.is_reduction()){    
                                            result({0})=SumVec(plaintexts({0})+ciphertexts({0}),reduction_size); 
                                        }else{
                                            result({0})=plaintexts({0})+ciphertexts({0}); 
                                        }
                                    }
                                    /*************************************************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                        int len = rhs.get_args().size();
                                            int reduction_size=(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());                                            
                                            DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                            DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                            result = DynamicTensor<Ciphertext>({1});
                                        if(expression.is_reduction()){
                                            result({0})=SumVec(ciphertexts0({0})+ciphertexts1({0}),reduction_size); 
                                        }{
                                            result({0})= ciphertexts0({0})+ciphertexts1({0}); 
                                        }
                                    }
                                    /************************************************************************************************/
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                            std::cout<<"welcome in plaintext + vectorciphertext multiplication \n";
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
                                            /**********************************************/
                                            int ref_sum = 0;
                                            std::vector<std::vector<int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    ref_sum+=iterator_variables_[i].lower_bound();
                                            }
                                            /********************************************/
                                            std::vector<size_t> dimensions ;
                                            for(int i=0; i<output_dim_variables_.size() ; i++){
                                                dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions); 
                                            /**********************************************/
                                            int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                                            vector<integer> mask ;
                                            bool is_vectorization_possible = false ; 
                                            int reduction_size = Compiler::active_func()->slot_count();
                                            if(vars0[vars0.size()-1].same_as(vars1[vars1.size()-1])&&(reduction_size>1)){
                                                is_vectorization_possible = true ;
                                            }    
                                            /*******************************************/    
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple = {} , sub_arg0_tuple ={}, sub_arg1_tuple={};
                                            Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                                            Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                                            Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                                            std::vector<size_t> sub_ref_arg0_tuple, sub_ref_arg1_tuple ;
                                            bool is_arg0_vectorizble = true , is_arg1_vectorizble = true ;
                                            /******************************************************************/
                                            if(expression.is_reduction()){
                                                std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    std::vector<std::size_t> reduction_tuple ;
                                                    iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /******************************************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_reduction_tuple = reduction_tuple ;
                                                        temp_res_red = (ciphertexts(sub_arg0_tuple)<<rotation0)+(plaintexts(sub_arg1_tuple)<<rotation1) ;
                                                    }else{                                    
                                                        bool update = false;
                                                        bool row_updated = false ;
                                                        for(int i =0;i<reduction_tuple.size();i++){
                                                            if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                                update=true ;
                                                                if(i<reduction_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(update){    
                                                            mask = vector<integer>(mask_size,0); 
                                                            mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                                            vectorized_row += temp_res*mask ;
                                                            if(row_updated==true){
                                                                std::cout<<"row updated \n";
                                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                                result(sub_ref_reduction_tuple)=vectorized_row;
                                                                vectorized_row = Ciphertext(PackedVal(1,0));
                                                            }
                                                            ref_reduction_tuple=reduction_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res_red = (ciphertexts(sub_arg0_tuple)<<rotation0)+(plaintexts(sub_arg1_tuple)<<rotation1);
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res_red+=(ciphertexts(sub_arg0_tuple)<<rotation0)+(plaintexts(sub_arg1_tuple)<<rotation1) ;    
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res += SumVec(ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple),reduction_size);
                                                        }else{
                                                            temp_res+=temp_res_red;
                                                        }   
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    
                                                    return true ;
                                                });
                                                mask = vector<integer>(mask_size,0); 
                                                mask[mask_size-1]=1;
                                                vectorized_row += temp_res*mask ;
                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                                                result(sub_ref_reduction_tuple) = vectorized_row; 
                                            }
                                            /***************************************************************/
                                            /***************************************************************/
                                            else{
                                                std::vector<std::size_t> result_tuple = {} ,sub_ref_result_tuple={} , ref_result_tuple = {};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    iterator_tuple ={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    result_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /*****************************************************************************/
                                                    mask = vector<integer>(mask_size,0); 
                                                    mask[result_tuple[ref_result_tuple.size()-1]]=1;
                                                    /************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_result_tuple = result_tuple ;
                                                        temp_res = ((ciphertexts(sub_arg0_tuple)<<rotation0)+(plaintexts(sub_arg1_tuple)<<rotation1))*mask ;
                                                    }else{                                    
                                                        bool row_updated = false ;
                                                        for(int i =0;i<result_tuple.size();i++){
                                                            if(result_tuple[i]!=ref_result_tuple[i]){
                                                                if(i<result_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(row_updated){    
                                                            sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                            result(sub_ref_result_tuple)=temp_res ;
                                                            ref_result_tuple=result_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res = ((ciphertexts(sub_arg0_tuple)<<rotation0)+(plaintexts(sub_arg1_tuple)<<rotation1))*mask ;
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res +=((ciphertexts(sub_arg0_tuple)<<rotation0)+(plaintexts(sub_arg1_tuple)<<rotation1))*mask ;  
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res = ciphertexts(sub_arg0_tuple)+plaintexts(sub_arg1_tuple) ;
                                                        }  
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    return true ;
                                                });
                                                sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                result(sub_ref_result_tuple)=temp_res ;
                                            }
                                    }
                                    /************************************************************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::cout<<"welcome in : vectorcipher + vectocipher \n";
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /**********************************************/
                                            int ref_sum = 0;
                                            std::vector<std::vector<int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    ref_sum+=iterator_variables_[i].lower_bound();
                                            }
                                            /********************************************/
                                            std::vector<size_t> dimensions ;
                                            for(int i=0; i<output_dim_variables_.size()-1 ; i++){
                                                dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions); 
                                            /*********************************************/
                                            int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                                            vector<integer> mask ;
                                            bool is_vectorization_possible = false ; 
                                            int reduction_size = Compiler::active_func()->slot_count();
                                            if(vars0[vars0.size()-1].same_as(vars1[vars1.size()-1])&&(reduction_size>1)){
                                                is_vectorization_possible = true ;
                                            }    
                                            /*******************************************/    
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple = {} , sub_arg0_tuple ={}, sub_arg1_tuple={};
                                            Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                                            Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                                            Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                                            std::vector<size_t> sub_ref_arg0_tuple, sub_ref_arg1_tuple ;
                                            bool is_arg0_vectorizble = true , is_arg1_vectorizble = true ;
                                            /******************************************************************/
                                            if(expression.is_reduction()){
                                                std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    std::vector<std::size_t> reduction_tuple ;
                                                    iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /******************************************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_reduction_tuple = reduction_tuple ;
                                                        temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0)+(ciphertexts1(sub_arg1_tuple)<<rotation1) ;
                                                    }else{                                    
                                                        bool update = false;
                                                        bool row_updated = false ;
                                                        for(int i =0;i<reduction_tuple.size();i++){
                                                            if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                                update=true ;
                                                                if(i<reduction_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(update){    
                                                            mask = vector<integer>(mask_size,0); 
                                                            mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                                            vectorized_row += temp_res*mask ;
                                                            if(row_updated==true){
                                                                std::cout<<"row updated \n";
                                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                                result(sub_ref_reduction_tuple)=vectorized_row;
                                                                vectorized_row = Ciphertext(PackedVal(1,0));
                                                            }
                                                            ref_reduction_tuple=reduction_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0)+(ciphertexts1(sub_arg1_tuple)<<rotation1);
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res_red+=(ciphertexts0(sub_arg0_tuple)<<rotation0)+(ciphertexts1(sub_arg1_tuple)<<rotation1) ;    
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res += SumVec(ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple),reduction_size);
                                                        }else{
                                                            temp_res+=temp_res_red;
                                                        }   
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    
                                                    return true ;
                                                });
                                                mask = vector<integer>(mask_size,0); 
                                                mask[mask_size-1]=1;
                                                vectorized_row += temp_res*mask ;
                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                                                result(sub_ref_reduction_tuple) = vectorized_row; 
                                            }
                                            /***************************************************************/
                                            /***************************************************************/
                                            else{
                                                std::cout<<"this evaluation is not a reduction  \n";
                                                std::vector<std::size_t> result_tuple = {} ,sub_ref_result_tuple={} , ref_result_tuple = {};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    iterator_tuple ={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    result_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /*****************************************************************************/
                                                    mask = vector<integer>(mask_size,0); 
                                                    mask[result_tuple[ref_result_tuple.size()-1]]=1;
                                                    /************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_result_tuple = result_tuple ;
                                                        //std::cout<<"first evaluation\n";
                                                        temp_res = ((ciphertexts0(sub_arg0_tuple)<<rotation0)+(ciphertexts1(sub_arg1_tuple)<<rotation1))*mask ;
                                                    }else{        
                                                        //std::cout<<"following evaluations \n";                            
                                                        bool row_updated = false ;
                                                        for(int i =0;i<result_tuple.size();i++){
                                                            if(result_tuple[i]!=ref_result_tuple[i]){
                                                                if(i<result_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(row_updated){    
                                                            sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                            result(sub_ref_result_tuple)=temp_res ;
                                                            ref_result_tuple=result_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res = ((ciphertexts0(sub_arg0_tuple)<<rotation0)+(ciphertexts1(sub_arg1_tuple)<<rotation1))*mask ;
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res +=((ciphertexts0(sub_arg0_tuple)<<rotation0)+(ciphertexts1(sub_arg1_tuple)<<rotation1))*mask ;  
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res = ciphertexts0(sub_arg0_tuple)+ciphertexts1(sub_arg1_tuple) ;
                                                        }  
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    return true ;
                                                });
                                                sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                result(sub_ref_result_tuple)=temp_res ;
                                            }
                                    }
                            break;   
                        /**************************************************************************************/
                        /**************************************************************************************/
                        case Expression::Op_t::sub:
                                    if(lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt){
                                        int reduction_size =1 ;
                                        DynamicTensor<Ciphertext> ciphertexts ; DynamicTensor<Plaintext>  plaintexts;
                                        result = DynamicTensor<Ciphertext>({1});
                                        if(lhs.type()==Type::plaintxt){
                                            plaintexts = lhs.get_plaintexts() ;
                                            ciphertexts = evaluate_expression(rhs) ;
                                            int len = rhs.get_args().size();
                                            reduction_size=reduction_size*(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());
                                            if(expression.is_reduction()){    
                                                result({0})=SumVec(plaintexts({0})-ciphertexts({0}),reduction_size); 
                                            }else{
                                                result({0})=plaintexts({0})-ciphertexts({0}); 
                                            }
                                        }else{
                                            plaintexts = rhs.get_plaintexts() ;
                                            ciphertexts = evaluate_expression(lhs) ;
                                            int len = lhs.get_args().size();
                                            reduction_size=reduction_size*(lhs.get_args()[len-1].upper_bound()-lhs.get_args()[len-1].lower_bound());
                                            if(expression.is_reduction()){    
                                                result({0})=SumVec(ciphertexts({0})-plaintexts({0}),reduction_size); 
                                            }else{
                                                result({0})=ciphertexts({0})-plaintexts({0}); 
                                            }
                                        }
                                    }
                                    /*************************************************************************************************/
                                    else if(lhs.type()==Type::ciphertxt&&rhs.type()==Type::ciphertxt){
                                        int len = rhs.get_args().size();
                                        int reduction_size=(rhs.get_args()[len-1].upper_bound()-rhs.get_args()[len-1].lower_bound());                                            
                                        DynamicTensor<Ciphertext> ciphertexts0=evaluate_expression(lhs) ; 
                                        DynamicTensor<Ciphertext> ciphertexts1=evaluate_expression(rhs);
                                        result = DynamicTensor<Ciphertext>({1});
                                        if(expression.is_reduction()){
                                            result({0})=SumVec(ciphertexts0({0})+ciphertexts1({0}),reduction_size); 
                                        }{
                                            result({0})= ciphertexts0({0})+ciphertexts1({0}); 
                                        }
                                    }
                                    /************************************************************************************************/
                                    else if(lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt){
                                            std::cout<<"welcome in plaintext + vectorciphertext multiplication \n";
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
                                            /**********************************************/
                                            int ref_sum = 0;
                                            std::vector<std::vector<int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    ref_sum+=iterator_variables_[i].lower_bound();
                                            }
                                            /********************************************/
                                            std::vector<size_t> dimensions ;
                                            for(int i=0; i<output_dim_variables_.size() ; i++){
                                                dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions); 
                                            /**********************************************/
                                            int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                                            vector<integer> mask ;
                                            bool is_vectorization_possible = false ; 
                                            int reduction_size = Compiler::active_func()->slot_count();
                                            if(vars0[vars0.size()-1].same_as(vars1[vars1.size()-1])&&(reduction_size>1)){
                                                is_vectorization_possible = true ;
                                            }    
                                            /*******************************************/    
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple = {} , sub_arg0_tuple ={}, sub_arg1_tuple={};
                                            Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                                            Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                                            Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                                            std::vector<size_t> sub_ref_arg0_tuple, sub_ref_arg1_tuple ;
                                            bool is_arg0_vectorizble = true , is_arg1_vectorizble = true ;
                                            /******************************************************************/
                                            if(expression.is_reduction()){
                                                std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    std::vector<std::size_t> reduction_tuple ;
                                                    iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /******************************************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_reduction_tuple = reduction_tuple ;
                                                        if(lhs.type()==Type::vectorciphertxt){
                                                            temp_res_red = (ciphertexts(sub_arg0_tuple)<<rotation0) - (plaintexts(sub_arg1_tuple)<<rotation1) ;
                                                        }else{
                                                            temp_res_red = (plaintexts(sub_arg1_tuple)<<rotation1) - (ciphertexts(sub_arg0_tuple)<<rotation0) ;
                                                        }
                                                    }else{                                    
                                                        bool update = false;
                                                        bool row_updated = false ;
                                                        for(int i =0;i<reduction_tuple.size();i++){
                                                            if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                                update=true ;
                                                                if(i<reduction_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(update){    
                                                            mask = vector<integer>(mask_size,0); 
                                                            mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                                            vectorized_row += temp_res*mask ;
                                                            if(row_updated==true){
                                                                std::cout<<"row updated \n";
                                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                                result(sub_ref_reduction_tuple)=vectorized_row;
                                                                vectorized_row = Ciphertext(PackedVal(1,0));
                                                            }
                                                            ref_reduction_tuple=reduction_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            if(lhs.type()==Type::vectorciphertxt){
                                                                temp_res_red = (ciphertexts(sub_arg0_tuple)<<rotation0) - (plaintexts(sub_arg1_tuple)<<rotation1) ;
                                                            }else{
                                                                temp_res_red = (plaintexts(sub_arg1_tuple)<<rotation1) - (ciphertexts(sub_arg0_tuple)<<rotation0) ;
                                                            }
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            if(lhs.type()==Type::vectorciphertxt){
                                                                temp_res_red+=(ciphertexts(sub_arg0_tuple)<<rotation0) - (plaintexts(sub_arg1_tuple)<<rotation1) ;
                                                            }else{
                                                                temp_res_red+=(plaintexts(sub_arg1_tuple)<<rotation1) - (ciphertexts(sub_arg0_tuple)<<rotation0) ;
                                                            }
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            if(lhs.type()==Type::vectorciphertxt){
                                                                temp_res+=SumVec((ciphertexts(sub_arg0_tuple)<<rotation0) - (plaintexts(sub_arg1_tuple)<<rotation1),reduction_size) ;
                                                            }else{
                                                                temp_res+=SumVec((plaintexts(sub_arg1_tuple)<<rotation1) - (ciphertexts(sub_arg0_tuple)<<rotation0),reduction_size) ;
                                                            }
                                                        }else{
                                                            temp_res+=temp_res_red;
                                                        }   
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    
                                                    return true ;
                                                });
                                                mask = vector<integer>(mask_size,0); 
                                                mask[mask_size-1]=1;
                                                vectorized_row += temp_res*mask ;
                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                                                result(sub_ref_reduction_tuple) = vectorized_row; 
                                            }
                                            /***************************************************************/
                                            /***************************************************************/
                                            else{
                                                std::vector<std::size_t> result_tuple = {} ,sub_ref_result_tuple={} , ref_result_tuple = {};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    iterator_tuple ={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    result_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /*****************************************************************************/
                                                    mask = vector<integer>(mask_size,0); 
                                                    mask[result_tuple[ref_result_tuple.size()-1]]=1;
                                                    /************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_result_tuple = result_tuple ;
                                                        if(lhs.type()==Type::vectorciphertxt){
                                                            temp_res = ((ciphertexts(sub_arg0_tuple)<<rotation0)-(plaintexts(sub_arg1_tuple)<<rotation1))*mask ;
                                                        }else{
                                                            temp_res = ((plaintexts(sub_arg1_tuple)<<rotation1)-(ciphertexts(sub_arg0_tuple)<<rotation0))*mask ;
                                                        }
                                                    }else{                                    
                                                        bool row_updated = false ;
                                                        for(int i =0;i<result_tuple.size();i++){
                                                            if(result_tuple[i]!=ref_result_tuple[i]){
                                                                if(i<result_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(row_updated){    
                                                            sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                            result(sub_ref_result_tuple)=temp_res ;
                                                            ref_result_tuple=result_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            if(lhs.type()==Type::vectorciphertxt){
                                                                temp_res = ((ciphertexts(sub_arg0_tuple)<<rotation0)-(plaintexts(sub_arg1_tuple)<<rotation1))*mask ;
                                                            }else{
                                                                temp_res = ((plaintexts(sub_arg1_tuple)<<rotation1)-(ciphertexts(sub_arg0_tuple)<<rotation0))*mask ;
                                                            }
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            if(lhs.type()==Type::vectorciphertxt){
                                                                temp_res += ((ciphertexts(sub_arg0_tuple)<<rotation0)-(plaintexts(sub_arg1_tuple)<<rotation1))*mask ;
                                                            }else{
                                                                temp_res += ((plaintexts(sub_arg1_tuple)<<rotation1)-(ciphertexts(sub_arg0_tuple)<<rotation0))*mask ;
                                                            }
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            if(lhs.type()==Type::vectorciphertxt){
                                                                temp_res = ciphertexts(sub_arg0_tuple)-plaintexts(sub_arg1_tuple)  ;
                                                            }else{
                                                                temp_res = plaintexts(sub_arg1_tuple)-ciphertexts(sub_arg0_tuple);
                                                            }
                                                        }  
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    return true ;
                                                });
                                                sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                result(sub_ref_result_tuple)=temp_res ;
                                            }
                                    }
                                    /************************************************************************************************/
                                    else if(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::vectorciphertxt){
                                            std::cout<<"welcome in : vectorcipher + vectocipher \n";
                                            std::vector<Var> vars0 = lhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts0 = evaluate_expression(lhs) ;
                                            std::vector<Var> vars1 = rhs.get_compute_args();
                                            DynamicTensor<Ciphertext> ciphertexts1 = evaluate_expression(rhs) ;
                                            /**********************************************/
                                            int ref_sum = 0;
                                            std::vector<std::vector<int>> ranges = {};
                                            std::vector<size_t> iterator_tuple ={};
                                            for (int i=0; i<iterator_variables_.size(); i++) {
                                                    int dim = iterator_variables_[i].upper_bound() - iterator_variables_[i].lower_bound();
                                                    std::vector<int> tup = {0,dim,iterator_variables_[i].increment_step()};
                                                    ranges.push_back(tup);
                                                    iterator_tuple.push_back(iterator_variables_[i].lower_bound());
                                                    ref_sum+=iterator_variables_[i].lower_bound();
                                            }
                                            /********************************************/
                                            std::vector<size_t> dimensions ;
                                            for(int i=0; i<output_dim_variables_.size()-1 ; i++){
                                                dimensions.push_back(output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound());
                                            }
                                            result = DynamicTensor<Ciphertext>(dimensions); 
                                            /*********************************************/
                                            int mask_size = output_dim_variables_[output_dim_variables_.size()-1].upper_bound()-output_dim_variables_[output_dim_variables_.size()-1].lower_bound();
                                            vector<integer> mask ;
                                            bool is_vectorization_possible = false ; 
                                            int reduction_size = Compiler::active_func()->slot_count();
                                            if(vars0[vars0.size()-1].same_as(vars1[vars1.size()-1])&&(reduction_size>1)){
                                                is_vectorization_possible = true ;
                                            }    
                                            /*******************************************/    
                                            std::vector<std::size_t> arg0_tuple, arg1_tuple = {} , sub_arg0_tuple ={}, sub_arg1_tuple={};
                                            Ciphertext temp_res = Ciphertext(PackedVal(1,0));
                                            Ciphertext temp_res_red = Ciphertext(PackedVal(1,0));
                                            Ciphertext vectorized_row = Ciphertext(PackedVal(1,0));
                                            std::vector<size_t> sub_ref_arg0_tuple, sub_ref_arg1_tuple ;
                                            bool is_arg0_vectorizble = true , is_arg1_vectorizble = true ;
                                            /******************************************************************/
                                            if(expression.is_reduction()){
                                                std::vector<size_t> ref_reduction_tuple ={} ,sub_ref_reduction_tuple ={};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    std::vector<std::size_t> reduction_tuple ;
                                                    iterator_tuple ={},arg1_tuple={},arg0_tuple={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    reduction_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /******************************************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_reduction_tuple = reduction_tuple ;
                                                        temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0)-(ciphertexts1(sub_arg1_tuple)<<rotation1) ;
                                                    }else{                                    
                                                        bool update = false;
                                                        bool row_updated = false ;
                                                        for(int i =0;i<reduction_tuple.size();i++){
                                                            if(reduction_tuple[i]!=ref_reduction_tuple[i]){
                                                                update=true ;
                                                                if(i<reduction_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(update){    
                                                            mask = vector<integer>(mask_size,0); 
                                                            mask[ref_reduction_tuple[ref_reduction_tuple.size()-1]]=1;
                                                            vectorized_row += temp_res*mask ;
                                                            if(row_updated==true){
                                                                std::cout<<"row updated \n";
                                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);
                                                                result(sub_ref_reduction_tuple)=vectorized_row;
                                                                vectorized_row = Ciphertext(PackedVal(1,0));
                                                            }
                                                            ref_reduction_tuple=reduction_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res_red = (ciphertexts0(sub_arg0_tuple)<<rotation0)-(ciphertexts1(sub_arg1_tuple)<<rotation1);
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res_red+=(ciphertexts0(sub_arg0_tuple)<<rotation0)-(ciphertexts1(sub_arg1_tuple)<<rotation1) ;    
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            temp_res += SumVec(ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple),reduction_size);
                                                        }else{
                                                            temp_res+=temp_res_red;
                                                        }   
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    
                                                    return true ;
                                                });
                                                mask = vector<integer>(mask_size,0); 
                                                mask[mask_size-1]=1;
                                                vectorized_row += temp_res*mask ;
                                                sub_ref_reduction_tuple = std::vector<size_t>(ref_reduction_tuple.begin(), ref_reduction_tuple.end() - 1);                
                                                result(sub_ref_reduction_tuple) = vectorized_row; 
                                            }
                                            /***************************************************************/
                                            /***************************************************************/
                                            else{
                                                std::cout<<"this evaluation is not a reduction  \n";
                                                std::vector<std::size_t> result_tuple = {} ,sub_ref_result_tuple={} , ref_result_tuple = {};
                                                generateNestedLoops(ranges,[&](const std::vector<int>& iterators) {                                                                                                                                                 
                                                    iterator_tuple ={};
                                                    int comp =0 ;
                                                    for (int val : iterators) {
                                                        iterator_tuple.push_back(val);
                                                        comp+=val;
                                                    } 
                                                    /****************************************************************************/
                                                    arg0_tuple = calculateCurrentPos(iterator_variables_,vars0, iterator_tuple);
                                                    arg1_tuple = calculateCurrentPos(iterator_variables_,vars1, iterator_tuple);
                                                    result_tuple = calculateCurrentPos(iterator_variables_,output_dim_variables_, iterator_tuple);
                                                    int rotation0 = arg0_tuple[arg0_tuple.size()-1];
                                                    int rotation1 = arg1_tuple[arg1_tuple.size()-1];
                                                    if(arg0_tuple.size() > 1){
                                                        sub_arg0_tuple = std::vector<size_t>(arg0_tuple.begin(), arg0_tuple.end() - 1);
                                                    }else{ sub_arg0_tuple={0};}
                                                    if(arg1_tuple.size()>1){
                                                        sub_arg1_tuple = std::vector<size_t>(arg1_tuple.begin(), arg1_tuple.end() - 1);
                                                    }else{ sub_arg1_tuple={0};} 
                                                    /*****************************************************************************/
                                                    mask = vector<integer>(mask_size,0); 
                                                    mask[result_tuple[ref_result_tuple.size()-1]]=1;
                                                    /************************************************/
                                                    if(comp==ref_sum){
                                                        sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                        sub_ref_arg1_tuple = sub_arg1_tuple;
                                                        ref_result_tuple = result_tuple ;
                                                        //std::cout<<"first evaluation\n";
                                                        temp_res = ((ciphertexts0(sub_arg0_tuple)<<rotation0)-(ciphertexts1(sub_arg1_tuple)<<rotation1))*mask ;
                                                    }else{        
                                                        //std::cout<<"following evaluations \n";                            
                                                        bool row_updated = false ;
                                                        for(int i =0;i<result_tuple.size();i++){
                                                            if(result_tuple[i]!=ref_result_tuple[i]){
                                                                if(i<result_tuple.size()-1){
                                                                    row_updated = true;
                                                                }
                                                                break;
                                                            }
                                                        }
                                                        if(row_updated){    
                                                            sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                            result(sub_ref_result_tuple)=temp_res ;
                                                            ref_result_tuple=result_tuple;
                                                            temp_res = Ciphertext(PackedVal(1,0));
                                                        }
                                                        /****************************************************/
                                                        if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].lower_bound()){
                                                            sub_ref_arg0_tuple = sub_arg0_tuple ;
                                                            sub_ref_arg1_tuple = sub_arg1_tuple;
                                                            temp_res = ((ciphertexts0(sub_arg0_tuple)<<rotation0)-(ciphertexts1(sub_arg1_tuple)<<rotation1))*mask ;
                                                        }else{
                                                            is_arg0_vectorizble=is_arg0_vectorizble&&(sub_ref_arg0_tuple==sub_arg0_tuple);
                                                            is_arg1_vectorizble=is_arg1_vectorizble&&(sub_ref_arg1_tuple==sub_arg1_tuple);
                                                            temp_res +=((ciphertexts0(sub_arg0_tuple)<<rotation0)-(ciphertexts1(sub_arg1_tuple)<<rotation1))*mask ;  
                                                        }
                                                    }
                                                    /***************************************************/
                                                    if(iterator_tuple[iterator_tuple.size()-1]==iterator_variables_[iterator_variables_.size()-1].upper_bound()-1){
                                                        if(is_arg0_vectorizble&&is_arg1_vectorizble&&is_vectorization_possible){
                                                            std::cout<<"vectoror product possible \n";
                                                            temp_res = ciphertexts0(sub_arg0_tuple)-ciphertexts1(sub_arg1_tuple) ;
                                                        }  
                                                        is_arg0_vectorizble = true ;
                                                        is_arg1_vectorizble = true ; 
                                                    }
                                                    return true ;
                                                });
                                                sub_ref_result_tuple = std::vector<size_t>(ref_result_tuple.begin(), ref_result_tuple.end() - 1);
                                                result(sub_ref_result_tuple)=temp_res ;
                                            }
                                    }
                            break; 
                        /*************************************************************************************/
                        /*************************************************************************************/
                        default:
                            throw std::runtime_error("Undefined operation. Or unsupporetd operation");
                    }
                    //std::cout<<"\n Returning the result in evaluate_expression\n";
                    expression.set_is_evaluated(true);
                    expression.set_ciphertexts(result);
                    return evaluate_expression(expression);
                }else{
                    throw invalid_argument("This expression cant be evaluated \n");
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
        /* if (expr_compute_vars.size()!=iterator_variables_.size()){
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
        } */
        if(have_same_variables){
            Expression input_expression = expression_ ;
            std::cout<<"\n start expression evaluation  \n";
            // we verify here if ti possible to have an assignment operation 
            vector<Var> dimension_vars = input_expression.get_args();
            if(input_expression.op()==Expression::Op_t::o_none){
                if(dimension_vars.size()==output_dim_variables_.size()){
                    for(int i =0;i<dimension_vars.size();i++){
                        int len1 = dimension_vars[i].upper_bound()-dimension_vars[i].lower_bound();
                        int len2 = output_dim_variables_[i].upper_bound()-output_dim_variables_[i].lower_bound();
                        if(len1>len2){
                            throw invalid_argument("assignment impossible because dest dim is smaller than src dim");
                            break;
                        }
                    }
                }
            }
            DynamicTensor<Ciphertext> ciphertexts = evaluate_expression(input_expression);
            std::cout<<"\n Return from expression evaluation \n";
            expression_.set_is_evaluated(true);
            if(is_output){
                    std::cout<<"\n\n welcome : \n";
                    if (expression_.type()==Type::ciphertxt) {
                        ciphertexts({0}).set_output(name_);
                        expression_.set_args(iterator_variables_);
                    }else{
                        vector<size_t> dimensions ;
                        int dim = 0 ;
                        std::vector<std::vector<int>> ranges = {};
                        for (int i =0; i< output_dim_variables_.size()-1;i++) {
                            dim = output_dim_variables_[i].upper_bound() - output_dim_variables_[i].lower_bound();
                            dimensions.push_back(dim);
                            ranges.push_back({0,dim,output_dim_variables_[i].increment_step()});
                        } 
                        /**********************************************************************************/
                        generateNestedLoops(ranges,[&ciphertexts,this](const std::vector<int>& iterators) {
                            std::vector<std::size_t> iterator_tuple ;
                            std::string name=name_;
                            int comp = 0 ;
                            for (int val : iterators) {
                                iterator_tuple.push_back(val);
                                name+="["+std::to_string(iterator_tuple[comp])+"]";
                                comp+=1;
                            }
                            std::cout<<"setting name for variable :"<<name<<"\n";
                            ciphertexts(iterator_tuple).set_output(name);
                            return true ;
                        });    
                        expression_.set_args(output_dim_variables_);                
                    }
                }
                expression_.set_ciphertexts(ciphertexts);
                std::cout<<"\n\n End of evaluation \n";
        }else{
            throw invalid_argument("Provided expression is invalide");
        }   
        
    }
}