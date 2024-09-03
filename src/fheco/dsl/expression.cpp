#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include<iostream>
#include "fheco/dsl/expression.hpp"
using namespace std ;
namespace fheco {

    Expression::Expression() : 
            operator_(Op_t::o_none) {}

    Expression::Expression(Op_t op ,const  Expression& expression0): 
            operator_(op),is_defined_(true) {
                operands_.push_back(expression0);
                args_ = expression0.get_args();
                compute_args_ = expression0.get_compute_args();
                is_defined_=true ;
                type_=expression0.type();
            }
    Expression::Expression(Op_t op ,const Expression& expression0 ,const Expression& expression1, std::vector<Var> args ,std::vector<Var> compute_args,Type type): 
            operator_(op),is_defined_(true) {
                /*  if (expression0.type()!=expression1.type()){
                    throw std::invalid_argument("cannot apply operation on operands of different types !!!!!!!!!!!!!!!!!");
                } */
                operands_.push_back(expression0);
                operands_.push_back(expression1);
                args_= args;
                compute_args_ = compute_args ;
                is_defined_=true ;
                type_=type;
            }
    Expression::Expression(Op_t op ,const Expression& expression0 ,const Expression& expression1,bool is_reduction,std::vector<Var> args ,std::vector<Var> compute_args,Type type) : 
            is_reduction_(is_reduction),operator_(op),is_defined_(true) 
            {
                if (expression0.type()!=expression1.type()){
                    throw std::invalid_argument("cannot apply operation on operands of different types !");
                }
                operands_.push_back(expression0);
                operands_.push_back(expression1);
                args_= args ; 
                compute_args_ = compute_args ;
                type_=expression0.type();
            }
    Expression::Expression(integer value): 
            is_defined_(true),is_numeric_(true),value_(value),operator_(Op_t::o_none),type_(Type::plaintxt) {}

    /********************************************************************************/
    Expression::Expression(std::vector<Ciphertext> ciphertexts , std::vector<Var> iterator_variables,std::vector<Var> compute_args , Type type):
            args_(iterator_variables),type_(type),ciphertexts_(ciphertexts),compute_args_(compute_args),
            operator_(Op_t::o_none),is_defined_(true),is_evaluated_(true)
            {}
    /*********************************************************************************/
    // Copy constructor
    Expression::Expression(const Expression &other): 
            type_(other.type_), operator_(other.operator_),
            is_defined_(other.is_defined_) , is_numeric_(other.is_numeric_),
            value_(other.value_),operands_(other.operands_), args_(other.args_),
            compute_args_(other.compute_args_), is_evaluated_(other.is_evaluated_) , 
            ciphertexts_(other.ciphertexts_),is_reduction_(other.is_reduction_){}
     // Move constructor
    Expression::Expression(Expression &&other)noexcept : 
          type_(other.type_), operator_(other.operator_),is_defined_(other.is_defined_),
          is_numeric_(other.is_numeric_),value_(std::move(other.value_)),
          operands_(std::move(other.operands_)),args_(std::move(other.args_)),
          compute_args_(std::move(other.compute_args_)),is_evaluated_(other.is_evaluated_),
          ciphertexts_(std::move(other.ciphertexts_)),is_reduction_(other.is_reduction_) {}  
    // Copy assignment operator
    Expression &Expression::operator=(const Expression &other) {
        if (this != &other) {
            type_ = other.type_;
            operator_ = other.operator_;
            is_defined_ = other.is_defined_;
            is_numeric_ = other.is_numeric_;
            is_reduction_= other.is_reduction_;
            value_ = other.value_;
            operands_ = other.operands_;
            args_ = other.args_;
            compute_args_= other.compute_args_;
            is_evaluated_ = other.is_evaluated_;
            ciphertexts_ = other.ciphertexts_;

        }
        return *this;
    }
    // Move assignment operator
    /*The && indicates that the parameter is an rvalue reference,
     meaning it's a reference to a temporary object that can be "moved from.*/
    Expression &Expression::operator=(Expression &&other) noexcept {
        if (this != &other) {
            type_ = other.type_;
            operator_ = other.operator_;
            is_defined_ = other.is_defined_;
            is_numeric_ = other.is_numeric_;
            is_reduction_= other.is_reduction_;
            value_ = std::move(other.value_);
            operands_ = std::move(other.operands_);
            args_ = std::move(other.args_);
            compute_args_=std::move(other.compute_args_);
            is_evaluated_ = other.is_evaluated_;
            ciphertexts_ = std::move(other.ciphertexts_);
        }
        return *this;
    }
    /****************************************************************************/
    // Function to check if at least M-1 Vars in one list are in the other
    bool atLeastMMinusOneCommon(const std::vector<Var>& list1, const std::vector<Var>& list2) {
        if (list1.size() != list2.size()) return false; // Ensure both lists are of the same size

        int M = list1.size();
        int commonCount = 0;
        // Check how many Vars from list1 are in list2
        for (const auto& var1 : list1) {
            //std::cout<<var1.name()<<"\n";
            for (const auto& var2 : list2) {
                    //std::cout<<"==> "<<var2.name()<<"\n";
                    if (var1.same_as(var2)) {
                        commonCount++;
                        break;
                    }
            }
        }
        return commonCount >= M - 1;
    }
    /*****************************************************************************/
    // addition 
    Expression operator+(const Expression &lhs, const Expression &rhs){
        // ex : A(i,j) + 4
        //std::cout<<"\nExpr operator ++ \n";
        if ((lhs.is_numeric()&& !rhs.is_numeric())||(!lhs.is_numeric()&&rhs.is_numeric())){
            if (!rhs.is_numeric()){
                return Expression(Expression::Op_t::add ,lhs, rhs,rhs.get_args(),rhs.get_compute_args(),rhs.type());
            }else{
                return Expression(Expression::Op_t::add ,lhs, rhs,lhs.get_args(),lhs.get_compute_args(),lhs.type());
            }    
        }
        if(lhs.type()!=rhs.type()){
            throw invalid_argument("Operands are not of the same type  ");
        }
        // Ex : A(i,k) + B(k,j)
        if(!lhs.is_numeric()&& !rhs.is_numeric()){
            //std::cout<<" Apply verification :\n";
            bool res = atLeastMMinusOneCommon(lhs.get_compute_args(),rhs.get_compute_args());
            if (!res){
                throw invalid_argument("there are more than one variable Not common between the two exxpressions");
            }
            bool have_same_variables = true ;
            if (lhs.get_args().size()!=rhs.get_args().size()){
                have_same_variables = false ; 
            }else{
                for(int i =0;i<lhs.get_args().size();i++){
                    if(!lhs.get_args()[i].same_as(rhs.get_args()[i])){
                        have_same_variables=false;
                        break;
                    }
                }
            }
            std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
            if(have_same_variables){
                return Expression(Expression::Op_t::add ,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            }
            else {
                if(rhs.is_reduction()||lhs.is_reduction()){
                    if(rhs.is_reduction()){
                         return Expression(Expression::Op_t::add ,lhs, rhs,true,rhs.get_args(),compute_args,rhs.type());
                    }else{
                         return Expression(Expression::Op_t::add ,lhs, rhs,true,lhs.get_args(),compute_args,lhs.type());
                    }
                }else{
                    throw invalid_argument("Unsupported addition operation !!");
                }
            }
        }else {
            throw invalid_argument("cant sum two numeric argument");
        }
    }
    /************************************************************************************/
    // subtraction
    Expression operator-(const Expression &lhs, const Expression &rhs){
        // ex : A(i,j) - 4
        //std::cout<<"\nExpr operator -- \n";
        if ((lhs.is_numeric()&& !rhs.is_numeric())||(!lhs.is_numeric()&&rhs.is_numeric())){
            if (!rhs.is_numeric()){
                return Expression(Expression::Op_t::sub ,lhs, rhs,rhs.get_args(),rhs.get_compute_args(),rhs.type());
            }else{
                return Expression(Expression::Op_t::sub ,lhs, rhs,lhs.get_args(),lhs.get_compute_args(),lhs.type());
            }    
        }
        if(lhs.type()!=rhs.type()){
            throw invalid_argument("Operands are not of the same type ");
        }
        // Ex : A(i,j) - B(i,j)
        else if(!lhs.is_numeric()&& !rhs.is_numeric()){
            if (!atLeastMMinusOneCommon(lhs.get_compute_args(),rhs.get_compute_args())){
                throw invalid_argument("there are more than one variable Not common between the two exxpressions");
            }
            bool have_same_variables = true ;
            int compt = 0 ;
            if (lhs.get_args().size()!=rhs.get_args().size()){
                have_same_variables = false ; 
            }else{
                for(int i =0;i<lhs.get_args().size();i++){
                    if(!lhs.get_args()[i].same_as(rhs.get_args()[i])){
                        have_same_variables=false;
                        break;
                    }
                }
            }
            if(have_same_variables){
                std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_args());
                return Expression(Expression::Op_t::sub,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            }
            else {
                throw invalid_argument("aperands have different dimensions (i,j) (k,l)");
            }
        }else {
            throw invalid_argument("cant sum two numeric argument");
        }
    }
    /*********************************************************************************************************/
    // multiplication
    Expression operator*(const Expression &lhs, const Expression &rhs){
        // exx : A(i,j) * 4
        std::cout<<"\nExpr operator ** \n";
        if ((lhs.is_numeric()&& !rhs.is_numeric())||(!lhs.is_numeric()&&rhs.is_numeric())){
            if (!rhs.is_numeric()){
                return Expression(Expression::Op_t::mul ,lhs, rhs,rhs.get_args(),rhs.get_compute_args(),rhs.type());
            }else{
                return Expression(Expression::Op_t::mul ,lhs, rhs,lhs.get_args(),lhs.get_compute_args(),rhs.type());
            }    
        }
        if(lhs.type()!=rhs.type()){
            throw invalid_argument("Operands are not of the same type ");
        }
        if (!atLeastMMinusOneCommon(lhs.get_args(),rhs.get_args())){
            throw invalid_argument("there are more than one variable Not common between the two exxpressions");
        }
        // reduction operation 
        if(!lhs.is_numeric()&& !rhs.is_numeric()){
            // verify if operands have the same copute_args 
            bool have_same_variables = true ;
            int compt = 0 ;
            if (lhs.get_compute_args().size()!=rhs.get_compute_args().size()){
                have_same_variables = false ; 
            }else{
                for(int i =0;i<lhs.get_compute_args().size();i++){
                    //std::cout<<lhs.get_compute_args()[i].name()<<" "<<rhs.get_compute_args()[i].name()<<"\n";
                    if(!lhs.get_compute_args()[i].same_as(rhs.get_compute_args()[i])){
                        have_same_variables=false;
                        break;
                    }
                }
            }
            if(have_same_variables){
                std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
                return Expression(Expression::Op_t::mul ,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            }
            else{
                // check if reduction  
                bool res1 = lhs.get_args()[lhs.get_args().size()-1].same_as(rhs.get_args()[0]) ;
                bool res2 = lhs.get_compute_args()[lhs.get_compute_args().size()-1].same_as(rhs.get_compute_args()[0]) ;
                bool is_reduction = res1&&res2;
                if(is_reduction){
                    std::cout<<"We have a reduction multiplication "<<rhs.get_ciphertexts().size()<<" "<<lhs.get_ciphertexts().size()<<"\n";
                    std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_args());
                    std::vector<Var> args = {lhs.get_args()[0],rhs.get_args()[rhs.get_args().size()-1]};
                    return Expression(Expression::Op_t::mul ,lhs, rhs,true,args,compute_args,lhs.type());
                }
                else {
                    throw invalid_argument("Multiplication is not defined correctly");
                }   
            }
        }
    }
    // negation operator 
    Expression operator-(const Expression &lhs){
        return Expression(Expression::Op_t::negate ,lhs);    
    }
    
    
    // negation operator 
   /*  Var operator*(const Var &lhs, const int value){
         return Var(lhs.name_, lhs.value_ * value);   
    }
    Var operator*(const int value, const Var &rhs){
        return Var(rhs.name_, rhs.value_ * value);   
    } */
    // multiplication assignement
   /*  Var &Var::operator*=(const Var &rhs){
        value_ *= rhs.value_;
        return *this;
    }
    Var &Var::operator*=(const int value){
        value_ *= value;
        return *this;
    } */
}