#include "fheco/dsl/ciphertext.hpp"
#include "fheco/dsl/ops_overloads.hpp"
#include <stdexcept>
#include <utility>
#include <iostream>
using namespace std;

namespace fheco
{
/*********************************************************************************************/
Ciphertext::Ciphertext(std::string label,size_t required_dimensions,vector<size_t> shape){
    Compiler::active_func()->init_input(*this, move(label));
    required_dimensions_=required_dimensions ;
    minimum_coordinates_.push_back(0);
    dimensions_sizes_.push_back(shape[0]);
    args_.emplace_back("x", 0);
    shape_=shape;
}
Ciphertext::Ciphertext(size_t required_dimensions,vector<size_t> shape){
    required_dimensions_=required_dimensions ;
    minimum_coordinates_.push_back(0);
    dimensions_sizes_.push_back(shape[0]);
    args_.emplace_back("x", 0);
    shape_=shape;
}
/************************************************************************************************/
Ciphertext::Ciphertext(vector<size_t> shape) : id_{0}, shape_{move(shape)}, idx_{}, example_val_{}
{
  validate_shape(shape_);
}

Ciphertext::Ciphertext(string label, vector<size_t> shape) : Ciphertext(move(shape))
{
  Compiler::active_func()->init_input(*this, move(label));
}

Ciphertext::Ciphertext(string label, PackedVal example_val, vector<size_t> shape) : Ciphertext(move(shape))
{
  Compiler::active_func()->clear_data_evaluator().adjust_packed_val(example_val);
  example_val_ = move(example_val);
  Compiler::active_func()->init_input(*this, move(label));
}

Ciphertext::Ciphertext(string label, integer example_val_slot_min, integer example_val_slot_max, vector<size_t> shape)
  : Ciphertext(move(shape))
{
  example_val_ =
    Compiler::active_func()->clear_data_evaluator().make_rand_packed_val(example_val_slot_min, example_val_slot_max);
  Compiler::active_func()->init_input(*this, move(label));
}

Ciphertext::Ciphertext(const Plaintext &plain)
{
  *this = encrypt(plain);
}
Ciphertext &Ciphertext::operator=(const Ciphertext &other)
{
  if (idx_.size())
    emulate_subscripted_write(*this, other);
  else
  {
    id_ = other.id_;
    shape_ = other.shape_;
    example_val_ = other.example_val_;
  }
  return *this;
}

Ciphertext &Ciphertext::operator=(Ciphertext &&other)
{
  if (idx_.size())
    emulate_subscripted_write(*this, other);
  else
  {
    id_ = other.id_;
    shape_ = move(other.shape_);
    example_val_ = move(other.example_val_);
  }
  return *this;
}

const Ciphertext Ciphertext::operator[](size_t idx) const
{
  size_t actual_dim = shape_.size() - idx_.size();
  if (actual_dim == 0)
    throw invalid_argument("subscript on dimension 0");

  if (idx < 0 || idx > shape_[shape_.size() - actual_dim] - 1)
    throw invalid_argument("invalid index");

  Ciphertext prep = *this;
  prep.idx_.push_back(idx);
  return emulate_subscripted_read(prep);
}

Ciphertext &Ciphertext::operator[](size_t idx)
{
  size_t actual_dim = shape_.size() - idx_.size();
  if (actual_dim == 0)
    throw invalid_argument("subscript on dimension 0");

  if (idx < 0 || idx > shape_[shape_.size() - actual_dim] - 1)
    throw invalid_argument("invalid index");

  idx_.push_back(idx);
  return *this;
}
/***********************************************************/
Ciphertext& Ciphertext::apply_operator(const std::vector<Var> &args) const{
    if (args.size() != required_dimensions_) {
        throw std::invalid_argument("number of args different from required_dimensions");
    }
    bool has_non_zero = false ; 
    for(int i = 0 ; i< args.size();i++){
      if (args[i].value()!=0){
          has_non_zero = true;
      }
    }
    if (has_non_zero) {
        size_t rotation_step = 0 ;
        rotation_step = args[0].value();
        size_t total_dim = dimensions_sizes_[0] ;
        for(int i = 1 ; i< args.size();i++){
             total_dim*=dimensions_sizes_[i];
             rotation_step=rotation_step*(dimensions_sizes_[i]-minimum_coordinates_[i])+args[i].value() ;
        }
        Ciphertext* new_instance = new Ciphertext(*this);
        *new_instance = *new_instance << (rotation_step%total_dim) ;
        //static Ciphertext stored_instance = std::move(new_instance);
        return *new_instance; // Assign the new instance to the current object
    }
    //args_ = args;  // Assuming args_ is mutable to allow modification in a const function
    return const_cast<Ciphertext&>(*this);  // Cast away const-ness to return a non-const reference
}
/**********************************************************************/
const Ciphertext &Ciphertext::set_output(string label) const
{
  Compiler::active_func()->set_output(*this, move(label));
  return *this;
}

void Ciphertext::set_shape(vector<size_t> shape)
{
  shape_ = move(shape);
  validate_shape(shape_);
}
/*********************************************************************/
void Ciphertext::set_dimensions_sizes(vector<size_t> dims)
{
    if(dims.size()!=required_dimensions_){
        throw invalid_argument("the number of dimension_sizes != required_dimensions"); 
    }
    dimensions_sizes_ = move(dims);
    minimum_coordinates_.assign(required_dimensions_, 0);

}
void Ciphertext::set_minimum_coordinates(std::vector<size_t> minimum_coordinates)
{
    if(minimum_coordinates.size()!=required_dimensions_){
        throw invalid_argument("the number of dimension_sizes != required_dimensions"); 
    }
    minimum_coordinates_ = std::move(minimum_coordinates);
}
/*******************************************************************/
} // namespace fheco
