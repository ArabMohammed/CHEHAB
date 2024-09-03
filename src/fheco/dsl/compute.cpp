 
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <iostream>
#include "fheco/dsl/compute.hpp"
#include "fheco/dsl/ops_overloads.hpp"
using namespace std ;
namespace fheco {
    // erify that all variables of the list are unique 
    Computation::Computation(const std::string &name , const std::vector<Var> iterator_variables, const Expression& expression): 
        name_(name), iterator_variables_(iterator_variables){
            expression_= expression ;
            std::cout<<"\n inside compute constructor, compute_args :"<<expression_.get_compute_args().size()<<", args :"<<expression_.get_args().size()<<"\n";
        }
    Computation::Computation(const std::string &name , const std::vector<Var> iterator_variables, Type type): 
        name_(name), iterator_variables_(iterator_variables){
        std::vector<Ciphertext> ciphertexts = {} ; 
        expression_ =  Expression(ciphertexts,iterator_variables,iterator_variables,type);
    }
    // Copy constructor 
    Computation::Computation(const Computation &other) : 
        name_(other.name_), iterator_variables_(other.iterator_variables_),expression_(other.expression_){


    }
    // Move constructor
    Computation::Computation(Computation &&other) noexcept:  
        name_(std::move(other.name_)), iterator_variables_(std::move(other.iterator_variables_)),expression_(std::move(other.expression_)){

        
    }
    // Move assignment operator 
    Computation &Computation::operator=(Computation &&other) noexcept {
        if (this != &other) {
            name_ = std::move(other.name_);
            iterator_variables_ = std::move(other.iterator_variables_);
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
    /**************************************************************************/
    Expression& Computation::apply_operator(const std::vector<Var> &compute_args) const{
        std::cout<<compute_args.size()<<" "<<iterator_variables_.size()<<" \n";
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
            int rotation_step = 0 ;
            rotation_step = compute_args[0].rotation_steps();
            std::vector<Var> expresssion_args = expression_.get_args();
            int total_dim = expresssion_args[0].upper_bound()-expresssion_args[0].lower_bound() ;
            for(int i = 1 ; i< compute_args.size();i++){
                total_dim=total_dim*(expresssion_args[i].upper_bound()-expresssion_args[i].lower_bound());
                rotation_step=rotation_step*(expresssion_args[i].upper_bound()-expresssion_args[i].lower_bound())+compute_args[i].rotation_steps() ;
            }
            if (expression_.type() == Type::ciphertxt ){
                std::cout<<"\n Type conversion taken into account \n";
                Ciphertext info = expression_.get_ciphertexts()[0];
                info = info << (rotation_step%total_dim) ;
                std::vector<Ciphertext> ciphertexts = {info} ; 
                Expression* new_instance = new Expression(ciphertexts,iterator_variables_,compute_args,expression_.type());

                return *new_instance ;
            }
            else if (expression_.type() == Type::vectorciphertxt){
                throw invalid_argument("Not supported conversion compute--> Expr");
            }
        }
        Expression* new_instance = new Expression(expression_.get_ciphertexts(),iterator_variables_,compute_args,expression_.type());
        return *new_instance ;  // Cast away const-ness to return a non-const reference
    }    
    /***************************************************************************/
    void generateNestedLoops(const std::vector<std::pair<int, int>>& ranges, std::function<void(const std::vector<int>&)> body) {
    std::vector<int> iterators(ranges.size(), 0);
        // Initialize iterators with the start values
        for (std::size_t i = 0; i < ranges.size(); ++i) {
            iterators[i] = ranges[i].first;
        }
        while (true) {
            // Call the body function with the current iterators
            body(iterators);

            // Increment the iterators
            int level = ranges.size() - 1;
            while (level >= 0) {
                iterators[level] += 1;
                if (iterators[level] < ranges[level].second) {
                    break;  // No need to carry over to the next level
                } else {
                    if (level == 0) {
                        return;  // Finished all iterations
                    }
                    iterators[level] = ranges[level].first;  // Reset current level and move to the previous level
                    level -= 1;
                }
            }
        }
    }
    /**********************************************************************/
    /***********************************************************************************/
    std::vector<Ciphertext> Computation::evaluate_expression(Expression& expression ){
            if (expression.is_defined()){
                if (expression.is_evaluated()) {
                    return expression.get_ciphertexts();
                }
                // Recursively evaluate operands if needed
                if (expression.op()!=Expression::Op_t::o_none) {
                    std::vector<Ciphertext> result ;
                    switch(expression.op()) {
                        /*****************************************************/
                        case Expression::Op_t::negate:
                            for(int i =0;i<evaluate_expression(expression.get_operands()[0]).size();i++){
                                //result.push_back(- evaluate(expression.get_operands()[0])[i]);
                            }
                            break;
                        /*****************************************************/
                        case Expression::Op_t::mul:
                            std::cout<<"\n Realising a multipication \n" ;
                            // we treat the case of having a multiplication by an integer 
                            if(!expression.is_reduction()){
                                if(expression.get_operands()[0].is_numeric()){
                                    integer factor = expression.get_operands()[0].value() ;
                                    std::vector<Ciphertext> ciphertexts = evaluate_expression(expression.get_operands()[1]) ;
                                    for(int i =0;i<ciphertexts.size();i++){
                                    result.push_back(ciphertexts[i]*factor);
                                    }   
                                }else if(expression.get_operands()[1].is_numeric()){
                                    integer factor = expression.get_operands()[1].value() ;
                                    std::vector<Ciphertext> ciphertexts = evaluate_expression(expression.get_operands()[0]) ;
                                    for(int i =0;i<ciphertexts.size();i++){
                                    result.push_back(ciphertexts[i]*factor);
                                    }  
                                }else {
                                    std::vector<Ciphertext> ciphertexts0 = evaluate_expression(expression.get_operands()[0]) ;
                                    std::vector<Ciphertext> ciphertexts1 = evaluate_expression(expression.get_operands()[1]) ;
                                    std::vector<Var> vars0 = expression.get_operands()[0].get_compute_args();
                                    std::vector<Var> vars1 = expression.get_operands()[1].get_compute_args();
                                    bool have_same_variables = true ; 
                                    bool found = false ; 
                                    for(int i =0;i<vars0.size();i++){
                                        found = false ;
                                        for(int j=0; j < vars1.size(); j++){
                                            if(vars0[i].same_as(vars1[j])){
                                                found=true;
                                                break;
                                            }
                                        }
                                        if(!found){
                                            have_same_variables = false ;
                                            break;
                                        }
                                    }
                                    // both of operands have the same variable_iterators
                                    if(have_same_variables){
                                        for(int i =0;i<ciphertexts0.size();i++){
                                            result.push_back(ciphertexts0[i]*ciphertexts1[i]);
                                        }
                                    }else{
                                        throw invalid_argument("Multiplication not supported");
                                    }
                                }
                            }
                            //case A(i,j)*B(i,j) still to be considedred 
                            //result = operands_[0].evaluate() * operands_[1].evaluate();
                            break;
                        /********************************************************************/
                        /********************************************************************/
                        case Expression::Op_t::add:
                            std::cout<<"\n Realising an addition \n" ;
                            if (!expression.is_reduction()){
                                // Treating the case of having a sum with a plaintext value
                                if(expression.get_operands()[0].is_numeric()){
                                    integer factor = expression.get_operands()[0].value() ;
                                    std::vector<Ciphertext> ciphertexts = evaluate_expression(expression.get_operands()[1]) ;
                                    for(int i =0;i<ciphertexts.size();i++){
                                        result.push_back(ciphertexts[i]+factor);
                                    }   
                                }else if(expression.get_operands()[1].is_numeric()){
                                    integer factor = expression.get_operands()[1].value() ;
                                    std::vector<Ciphertext> ciphertexts = evaluate_expression(expression.get_operands()[0]) ;
                                    for(int i =0;i<ciphertexts.size();i++){
                                        result.push_back(ciphertexts[i]+factor);
                                    }  
                                }else{
                                    // we treat the general case ex: A(i,j)+B(i,k)
                                    std::vector<Ciphertext> ciphertexts0 = evaluate_expression(expression.get_operands()[0]) ;
                                    std::vector<Ciphertext> ciphertexts1 = evaluate_expression(expression.get_operands()[1]) ;
                                    std::vector<Var> vars0 = expression.get_operands()[0].get_compute_args();
                                    std::vector<Var> vars1 = expression.get_operands()[1].get_compute_args();
                                    std::vector<std::pair<int, int>> ranges = {};
                                    
                                    for(auto &var : iterator_variables()){
                                        std::pair<int, int> range = {var.lower_bound(),var.upper_bound()};
                                        ranges.push_back(range);
                                    }
                                    // Ex: case of A(i,j) + B(i,j)
                                    bool have_same_variables = true ; 
                                    bool found = false ; 
                                    for(int i =0;i<vars0.size();i++){
                                        found = false ;
                                        for(int j=0; j < vars1.size(); j++){
                                            if(vars0[i].same_as(vars1[j])){
                                                found=true;
                                                break;
                                            }
                                        }
                                        if(!found){
                                            have_same_variables = false ;
                                            break;
                                        }
                                    }
                                    // both of operands have the same variable_iterators
                                    if(have_same_variables){
                                        for(int i =0;i<ciphertexts0.size();i++){
                                            result.push_back(ciphertexts0[i]+ciphertexts1[i]);
                                        }
                                    }
                                    else if(vars0[1].same_as(vars1[1])){
                                        // have the same  column iterator  Ex : // A(1,j)+B(i,j)
                                        if ( (vars0[0].upper_bound() - vars0[0].lower_bound())==1){
                                            if (expression.type()==Type::ciphertxt){
                                                int row_size_op0 = expression.get_operands()[0].get_args()[0].upper_bound()-expression.get_operands()[0].get_args()[0].lower_bound();
                                                int col_size_op0 = expression.get_operands()[0].get_args()[1].upper_bound()-expression.get_operands()[0].get_args()[1].lower_bound();
                                                int pos_row_op0 = vars0[0].lower_bound() ; 
                                                int total_size = col_size_op0 * row_size_op0 ;
                                                vector<int64_t> mask(total_size, 0);
                                                for (int i = 0 ; i < row_size_op0;i++)
                                                        mask[i]=1;
                                                Ciphertext row = (ciphertexts0[0] << (pos_row_op0*row_size_op0)%total_size)*mask;
                                                /**********************************************************************/
                                                Ciphertext res ; 
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    res+=row>>(i*row_size_op0);
                                                }
                                                result.push_back(res+ciphertexts1[0]);
                                            }else {
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    result.push_back(ciphertexts0[vars0[0].lower_bound()]+ciphertexts1[i]);

                                                }
                                            }
                                            // have the same  column iterator  Ex : // A(i,j)+B(1,j)
                                        }else if ( (vars1[0].upper_bound() - vars1[0].lower_bound())==1){
                                            if (expression.type()==Type::ciphertxt){
                                                int row_size_op1 = expression.get_operands()[1].get_args()[0].upper_bound()-expression.get_operands()[1].get_args()[0].lower_bound();
                                                int col_size_op1 = expression.get_operands()[1].get_args()[1].upper_bound()-expression.get_operands()[1].get_args()[1].lower_bound();
                                                int pos_row_op1 = vars1[0].lower_bound() ; 
                                                int total_size = col_size_op1 * row_size_op1 ;
                                                vector<int64_t> mask(total_size, 0);
                                                for (int i = 0 ; i < row_size_op1;i++)
                                                        mask[i]=1;
                                                Ciphertext row = (ciphertexts1[0] << (pos_row_op1*row_size_op1)%total_size)*mask;
                                                /**********************************************************************/
                                                Ciphertext res ; 
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    res+=row>>(i*row_size_op1);
                                                }
                                                result.push_back(res+ciphertexts0[0]);
                                            }else {
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    result.push_back(ciphertexts1[vars1[0].lower_bound()]+ciphertexts0[i]);

                                                }
                                            }
                                        }
                                        // have the same column iterator 
                                        
                                    }else{
                                        // else if(vars0[0].same_as(vars1[0]))

                                    }
                                }
                            }else{
                                if(expression.get_operands()[1].is_reduction()){
                                    std::cout<<"\n welcome in treating the reduction In addition \n";
                                    // C(i,j,k-1) + A(i,k)*B(k,j)
                                    Expression expression1 = expression.get_operands()[1];
                                    Expression A = expression1.get_operands()[0] ; 
                                    Expression B = expression1.get_operands()[1] ; 
                                
                                    std::vector<Ciphertext> A_row_encrypted = A.get_ciphertexts();
                                    std::vector<Ciphertext> B_col_encrypted = B.get_ciphertexts() ; 
                                    std::cout<<"\n Nb of rows : "<<A_row_encrypted.size()<<"Number of cols "<<B_col_encrypted.size()<<"\n";
                                    for (size_t i = 0; i < A_row_encrypted.size(); ++i)
                                    {
                                        Ciphertext cline;
                                        for (size_t j = 0; j < B_col_encrypted.size(); ++j)
                                        {
                                            vector<int64_t> mask(A_row_encrypted.size(), 0);
                                            mask[j] = 1;
                                            Ciphertext slot;
                                            ///************************************************
                                            slot = SumVec(A_row_encrypted[i] * B_col_encrypted[j],A.get_args()[1].upper_bound());
                                            ///***********************************************************
                                            if (j == 0)                                                               
                                                cline = slot * mask;
                                            else
                                                cline += slot * mask;
                                        }
                                        cline.set_output(name_+"["+to_string(i)+"]");
                                        result.push_back(cline);
                                    } 
                                    std::cout<<"matrix multiplication done \n" ;
                                }
                            }
                            std::cout<<"before break==>>\n";
                            break;
                        /*******************************************************************************/
                        /******************************************************************************/
                        case Expression::Op_t::sub:
                            std::cout<<"\n Realising a Susbstraction \n" ;
                            // Treating the case of having substraction with a plaintext value
                                if(expression.get_operands()[0].is_numeric()){
                                    integer factor = expression.get_operands()[0].value() ;
                                    std::vector<Ciphertext> ciphertexts = evaluate_expression(expression.get_operands()[1]) ;
                                    for(int i =0;i<ciphertexts.size();i++){
                                        result.push_back(ciphertexts[i]-factor);
                                    }   
                                }else if(expression.get_operands()[1].is_numeric()){
                                    integer factor = expression.get_operands()[1].value() ;
                                    std::vector<Ciphertext> ciphertexts = evaluate_expression(expression.get_operands()[0]) ;
                                    for(int i =0;i<ciphertexts.size();i++){
                                        result.push_back(ciphertexts[i]-factor);
                                    }  
                                }else{
                                    // we treat the general case ex: A(i,j)+B(i,k)
                                    std::vector<Ciphertext> ciphertexts0 = evaluate_expression(expression.get_operands()[0]) ;
                                    std::vector<Ciphertext> ciphertexts1 = evaluate_expression(expression.get_operands()[1]) ;
                                    std::vector<Var> vars0 = expression.get_operands()[0].get_compute_args();
                                    std::vector<Var> vars1 = expression.get_operands()[1].get_compute_args();
                                    std::vector<std::pair<int, int>> ranges = {};
                                    
                                    for(auto &var : iterator_variables()){
                                        std::pair<int, int> range = {var.lower_bound(),var.upper_bound()};
                                        ranges.push_back(range);
                                    }
                                    // Ex: case of A(i,j) + B(i,j)
                                    bool have_same_variables = true ; 
                                    bool found = false ; 
                                    for(int i =0;i<vars0.size();i++){
                                        found = false ;
                                        for(int j=0; j < vars1.size(); j++){
                                            if(vars0[i].same_as(vars1[j])){
                                                found=true;
                                                break;
                                            }
                                        }
                                        if(!found){
                                            have_same_variables = false ;
                                            break;
                                        }
                                    }
                                    // both of operands have the same variable_iterators
                                    if(have_same_variables){
                                        for(int i =0;i<ciphertexts0.size();i++){
                                            result.push_back(ciphertexts0[i]-ciphertexts1[i]);
                                        }
                                    }
                                    else if(vars0[1].same_as(vars1[1])){
                                        // have the same  column iterator  Ex : // A(1,j)+B(i,j)
                                        if ( (vars0[0].upper_bound() - vars0[0].lower_bound())==1){
                                            if (expression.type()==Type::ciphertxt){
                                                int row_size_op0 = expression.get_operands()[0].get_args()[0].upper_bound()-expression.get_operands()[0].get_args()[0].lower_bound();
                                                int col_size_op0 = expression.get_operands()[0].get_args()[1].upper_bound()-expression.get_operands()[0].get_args()[1].lower_bound();
                                                int pos_row_op0 = vars0[0].lower_bound() ; 
                                                int total_size = col_size_op0 * row_size_op0 ;
                                                vector<int64_t> mask(total_size, 0);
                                                for (int i = 0 ; i < row_size_op0;i++)
                                                        mask[i]=1;
                                                Ciphertext row = (ciphertexts0[0] << (pos_row_op0*row_size_op0)%total_size)*mask;
                                                /**********************************************************************/
                                                Ciphertext res ; 
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    res+=row>>(i*row_size_op0);
                                                }
                                                result.push_back(res-ciphertexts1[0]);
                                            }else {
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    result.push_back(ciphertexts0[vars0[0].lower_bound()]-ciphertexts1[i]);

                                                }
                                            }
                                            // have the same  column iterator  Ex : // A(i,j)+B(1,j)
                                        }else if ( (vars1[0].upper_bound() - vars1[0].lower_bound())==1){
                                            if (expression.type()==Type::ciphertxt){
                                                int row_size_op1 = expression.get_operands()[1].get_args()[0].upper_bound()-expression.get_operands()[1].get_args()[0].lower_bound();
                                                int col_size_op1 = expression.get_operands()[1].get_args()[1].upper_bound()-expression.get_operands()[1].get_args()[1].lower_bound();
                                                int pos_row_op1 = vars1[0].lower_bound() ; 
                                                int total_size = col_size_op1 * row_size_op1 ;
                                                vector<int64_t> mask(total_size, 0);
                                                for (int i = 0 ; i < row_size_op1;i++)
                                                        mask[i]=1;
                                                Ciphertext row = (ciphertexts1[0] << (pos_row_op1*row_size_op1)%total_size)*mask;
                                                /**********************************************************************/
                                                Ciphertext res ; 
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    res+=row>>(i*row_size_op1);
                                                }
                                                result.push_back(res-ciphertexts0[0]);
                                            }else {
                                                for(int i=iterator_variables()[0].lower_bound();i<iterator_variables()[0].upper_bound();i++){
                                                    result.push_back(ciphertexts1[vars1[0].lower_bound()]-ciphertexts0[i]);

                                                }
                                            }
                                        }
                                        // have the same column iterator 
                                        
                                    }else{
                                        // else if(vars0[0].same_as(vars1[0]))

                                    }
                                }
                            break;
                        /**************************************************************************************/
                        /**************************************************************************************/
                        case Expression::Op_t::mod_switch:
                            // Implement your specific logic for this operation
                            break;
                        /**************************************************************************************/
                        case Expression::Op_t::o_none:
                        default:
                            throw std::runtime_error("Undefined operation.");
                    }
                    std::cout<<"Returning the result in evaluate_expression\n";
                    expression.set_is_evaluated(true);
                    expression.set_ciphertexts(result);
                    return evaluate_expression(expression);
                } else if (expression.is_numeric()) {
                    return {};
                } else {
                    throw std::runtime_error("Expression cannot be evaluated.");
                }
            }else{
                throw invalid_argument("This expression is undefined \n");
            }
    }
    /*******************************************************************/
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
            std::vector<Ciphertext> ciphertexts = evaluate_expression(input_expression);
            expression_.set_is_evaluated(true);
            expression_.set_ciphertexts(ciphertexts);
            if(is_output){
                    if (ciphertexts.size() == 1) {
                        ciphertexts[0].set_output(name_);
                    }else{
                        for(int i=0; i<ciphertexts.size() ; i++){
                            //ciphertexts[i].set_output(name_+"["+to_string(i)+"]");
                        }
                    }
            }
        }else{
            throw invalid_argument("Provided expression is invalide");
        }   
        
    }
}