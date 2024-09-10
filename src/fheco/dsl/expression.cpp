#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include<iostream>
#include <unordered_set>
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
    Expression::Expression(integer value): 
            is_defined_(true),value_(value),operator_(Op_t::o_none),type_(Type::plaintxt) {
                DynamicTensor<Plaintext> plaintexts({1});
                plaintexts({0})=value;
                plaintexts_=plaintexts;
            }

    /**********************************Convert an Input to an expression ***************/
    Expression::Expression(DynamicTensor<Ciphertext> ciphertexts , std::vector<Var> iterator_variables,std::vector<Var> compute_args , Type type):
            args_(iterator_variables),type_(type),ciphertexts_(ciphertexts),compute_args_(compute_args),
            operator_(Op_t::o_none),is_defined_(true),is_evaluated_(true)
            {}
    /************************************Covert a plaintext input to an expression*******/
    Expression::Expression(DynamicTensor<Plaintext> plaintexts, std::vector<Var> iterator_variables, std::vector<Var> compute_args, Type type):
            args_(iterator_variables),type_(type),plaintexts_(plaintexts),compute_args_(compute_args),
            operator_(Op_t::o_none),is_defined_(true),is_evaluated_(true)
            {}

    /**************************************************************************************/
    // Copy constructor
    Expression::Expression(const Expression &other): 
            type_(other.type_), operator_(other.operator_),
            is_defined_(other.is_defined_) ,
            value_(other.value_),operands_(other.operands_), args_(other.args_),
            compute_args_(other.compute_args_), is_evaluated_(other.is_evaluated_) , 
            ciphertexts_(other.ciphertexts_),is_reduction_(other.is_reduction_),
            plaintexts_(other.plaintexts_)
            {}
     // Move constructor
    Expression::Expression(Expression &&other)noexcept : 
          type_(other.type_), operator_(other.operator_),is_defined_(other.is_defined_),
          value_(std::move(other.value_)),
          operands_(std::move(other.operands_)),args_(std::move(other.args_)),
          compute_args_(std::move(other.compute_args_)),is_evaluated_(other.is_evaluated_),
          ciphertexts_(std::move(other.ciphertexts_)),is_reduction_(other.is_reduction_),
          plaintexts_(other.plaintexts_)
            {}  
    // Copy assignment operator
    Expression &Expression::operator=(const Expression &other) {
        if (this != &other) {
            type_ = other.type_;
            operator_ = other.operator_;
            is_defined_ = other.is_defined_;
            is_reduction_= other.is_reduction_;
            value_ = other.value_;
            operands_ = other.operands_;
            args_ = other.args_;
            compute_args_= other.compute_args_;
            is_evaluated_ = other.is_evaluated_;
            ciphertexts_ = other.ciphertexts_;
            plaintexts_ = other.plaintexts_;

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
            is_reduction_= other.is_reduction_;
            value_ = std::move(other.value_);
            operands_ = std::move(other.operands_);
            args_ = std::move(other.args_);
            compute_args_=std::move(other.compute_args_);
            is_evaluated_ = other.is_evaluated_;
            ciphertexts_ = std::move(other.ciphertexts_);
            plaintexts_ = std::move(other.plaintexts_);
        }
        return *this;
    }
    /****************************************************************************/
    // Function to check if at least M-1 Vars in one list are in the other
    bool atLeastMMinusOneCommon(const std::vector<Var>& list1, const std::vector<Var>& list2) {
        if (list1.size() != list2.size()) return false; // Ensure both lists are of the same size
        std::unordered_set<int> pos_set = {};
        int M = list1.size();
        int commonCount = 0;
        // Check how many Vars from list1 are in list2
        for (const auto& var1 : list1) {
            //std::cout<<var1.name()<<"\n";
            for (int j=0;j<list2.size();j++) {
                    //std::cout<<"==> "<<var2.name()<<"\n";
                    Var var2=list2[j];
                    if (var1.same_as(var2)&&pos_set.find(j)==pos_set.end()) {
                        commonCount++;
                        pos_set.insert(j);
                        break;
                    }
            }
        }
        return commonCount >= M - 1;
    }
    /*****************************************************************************/
    // addition arg1_tuple
    Expression operator+(const Expression &lhs, const Expression &rhs){
       std::cout<<"Welcome in addition operation \n";
       if(!(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt||
            lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||
            lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt||
            lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||
            lhs.type()==rhs.type()
        )){
            throw invalid_argument("Types of operands not supported");
        } 
        /*************************************************************************************/
        if ((lhs.type()==Type::plaintxt&& rhs.type()!=Type::plaintxt)||(lhs.type()!=Type::plaintxt&& rhs.type()==Type::plaintxt)){
            std::vector<Var> compute_args = {};
            if (rhs.type()!=Type::plaintxt){
                if(lhs.get_compute_args().size()>0){
                     //std::cout<<"welcome here : "<<lhs.get_compute_args().size()<<"\n";
                     compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());  
                }else{
                    compute_args = rhs.get_compute_args() ;
                }
                return Expression(Expression::Op_t::add ,lhs, rhs,compute_args,compute_args,rhs.type());
            }else{
                 if(rhs.get_compute_args().size()>0){
                     //std::cout<<"welcome here : "<<rhs.get_compute_args().size()<<"\n";
                     compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());  
                }else{
                    compute_args = lhs.get_compute_args() ;
                }
                return Expression(Expression::Op_t::add ,lhs, rhs,compute_args,compute_args,lhs.type());
            }    
        }
        if(lhs.type()!=Type::plaintxt&&rhs.type()!=Type::plaintxt){
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
            std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
            return Expression(Expression::Op_t::add,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            /*  if(have_same_variables){
                std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
                return Expression(Expression::Op_t::add,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            }
            else {
                throw invalid_argument("aperands have different dimensions (4*5) ()");
            } */
        }else {
            throw invalid_argument("cant sum two plaintexts arguments");
        }
    }
    /************************************************************************************/
    // subtraction
    Expression operator-(const Expression &lhs, const Expression &rhs){
        std::cout<<"Welcome in substraction operation \n";
        if(!(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt||
            lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||
            lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt||
            lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||
            lhs.type()==rhs.type()
        )){
            throw invalid_argument("Types of operands not supported");
        } 
        /***************************************************************/
        if ((lhs.type()==Type::plaintxt&& rhs.type()!=Type::plaintxt)||(lhs.type()!=Type::plaintxt&& rhs.type()==Type::plaintxt)){
            std::vector<Var> compute_args = {};
            if (rhs.type()!=Type::plaintxt){
                if(lhs.get_compute_args().size()>0){
                     compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());  
                }else{
                    compute_args = rhs.get_compute_args() ;
                }
                return Expression(Expression::Op_t::sub ,lhs, rhs,compute_args,compute_args,rhs.type());
            }else{
                 if(rhs.get_compute_args().size()>0){
                     compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());  
                }else{
                    compute_args = lhs.get_compute_args() ;
                }
                return Expression(Expression::Op_t::sub ,lhs, rhs,compute_args,compute_args,lhs.type());
            }    

        }
        /***************************************************************/
        if(lhs.type()!=Type::plaintxt&&rhs.type()!=Type::plaintxt){
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
            std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
            return Expression(Expression::Op_t::sub,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            /* if(have_same_variables){
                std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
                return Expression(Expression::Op_t::sub,lhs, rhs,rhs.get_args(),compute_args,lhs.type());
            }
            else {
                throw invalid_argument("aperands have different dimensions");
            } */
        }else {
            throw invalid_argument("cant sum two plaintexts arguments");
        }
    }
    /*********************************************************************************************************/
    // multiplication
    Expression operator*(const Expression &lhs, const Expression &rhs){
        // exx : A(i,j) * 4
        std::cout<<"\nExpr operator ** \n";
        if(!(lhs.type()==Type::vectorciphertxt&&rhs.type()==Type::plaintxt||
            lhs.type()==Type::plaintxt&&rhs.type()==Type::vectorciphertxt||
            lhs.type()==Type::ciphertxt&&rhs.type()==Type::plaintxt||
            lhs.type()==Type::plaintxt&&rhs.type()==Type::ciphertxt||
            lhs.type()==rhs.type()
        )){
            throw invalid_argument("Types of operands not supported");
        } 
        if ((lhs.type()==Type::plaintxt&& rhs.type()!=Type::plaintxt)||(lhs.type()!=Type::plaintxt&& rhs.type()==Type::plaintxt)){
            std::vector<Var> compute_args = {};
            if (rhs.type()!=Type::plaintxt){
                if(lhs.get_compute_args().size()>0){
                     compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());  
                }else{
                    compute_args = rhs.get_compute_args() ;
                }
                return Expression(Expression::Op_t::mul ,lhs, rhs,compute_args,compute_args,rhs.type());
            }else{
                 if(rhs.get_compute_args().size()>0){
                     compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());  
                }else{
                    compute_args = lhs.get_compute_args() ;
                }
                return Expression(Expression::Op_t::mul ,lhs, rhs,compute_args,compute_args,lhs.type());
            }    
        }
        if(lhs.type()!=Type::plaintxt&&rhs.type()!=Type::plaintxt){
            std::vector<Var> compute_args = Var::unionOfLists(lhs.get_compute_args(),rhs.get_compute_args());
            return Expression(Expression::Op_t::mul ,lhs, rhs,compute_args,compute_args,lhs.type());
        }
    }
    // negation operator 
    Expression operator-(const Expression &lhs){
        return Expression(Expression::Op_t::negate ,lhs);    
    }
    
}