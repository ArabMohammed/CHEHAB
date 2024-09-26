// C++ Program to Demonstrate // Operator Overloading
 #include <iostream> 
 #include <vector>
 using namespace std; 

/******************Assignment operator overloading ****************/
class Var {
    public : 
        int val_ ; 
        Var(int val ){
            val_=val;
        }
        const int val(){
            return val_ ;
        }
        void print(){
            std::cout<<val_<<" ";
        }
};
class Expression {
    public:
        vector<Var> vars_ ; 
        Expression(vector<Var> vars){
            vars_ = vars ;
        }
        Expression(){

        }
        Expression& operator=(const Expression& C)
        {
            // Check for self-assignment
            if (this == &C) {
                return *this;  // Handle self-assignment
            }
            vars_ = C.vars_;
            return *this;
        }
        /******************************************/
        void print(){
            for(auto var : vars_){
                var.print();
            }
            std::cout<<"\n";
        }
        /******************************************/
        Expression& operator=(const Expression&& C) noexcept
        {
            if (this == &C) {
                return *this;  // Handle self-assignment
            }
            vars_ = std::move(C.vars_);
            return *this;
        }
        /**********************************/
        Expression(const Expression& C) 
            : vars_(C.vars_) {}

        // Move constructor
        Expression(Expression&& C) noexcept 
            : vars_(std::move(C.vars_)) {}
};
class Complex { 
    private: 
        int real, imag;
        Expression expr_ ; 
    public:
        Complex(){
            Var var1 = Var(1);
            Var var2 = Var(2);
            Var var3 = Var(3);
            std::vector<Var> variables = {var1,var2,var3};
            expr_ = Expression(); 
            expr_.vars_=variables ;
        } 

         // This is automatically called when '+' is used with // between two Complex objects Complex 
        /*Complex operator+(Complex const& obj) 
        { 
          Complex res; 
          res.real = real + obj.real;
          res.imag = imag + obj.imag; 
          return res; 
        } */
        void print() 
        {
            expr_.print();
        } 
        /*******************************************/
        // Assignment operator overload
        Complex& operator=(const Complex& C)
        {
            // Check for self-assignment
            if (this != &C) {
                real  = C.real;
                imag  = C.imag;
                expr_ = C.expr_ ;
            }
            return *this;
        }
        /*Complex& operator=(const Complex&& C) noexcept
        {
            // Check for self-assignment
            if (this != &C) {
                real  = C.real;
                imag  = C.imag;
                expr_ = std::move(C.expr_) ;
            }
            return *this;
        }*/
        // Copy constructor
        Complex(const Complex& C) 
            : real(C.real), imag(C.imag), expr_(C.expr_) {}

        // Move constructor
        //Complex(Complex&& C) noexcept 
        //    : real(C.real), imag(C.imag), expr_(std::move(C.expr_)) {}

}; 
/****************************************************************/
Complex create_complex(){
    Complex comp ;
    return comp ;
}
/*****************************************************************/
#include <iostream> 
using namespace std; 
class Fraction { 
    private: 
        int num, den;
    public: 
        Fraction(int n, int d) 
        { num = n; den = d; } // Conversion operator: return float value of fraction 
        operator float() const 
        { 
            return float(num) / float(den); 
        } 
}; 
/*****************************************************************/
int main() { 
    Complex bobi = create_complex();
    bobi.print();
    /*Complex c1(10, 5), 
    c2(2, 4); 
    Complex c3 = c1 + c2; 
    c3.print();
    c1.print();
    c2.print();
    Fraction f(2, 5); 
    float val = f; 
    cout << val << '\n';
    vector<int> info = { 1 , 2 , 3 ,4}  ; 
    vector<int> info1 = std::move(info) ; 
    info1[1]= 5 ;
    for(auto val : info)
        std::cout<<val<<" ";
    std::cout<<"\n";
     for(auto val : info1)
        std::cout<<val<<" ";
    std::cout<<"\n";
    /******************************/
     return 0;
}