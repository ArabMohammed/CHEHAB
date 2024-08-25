#pragma once

#include "fheco/dsl/common.hpp"
#include "fheco/dsl/compiler.hpp"
#include "fheco/dsl/plaintext.hpp"
#include "fheco/dsl/var.hpp"
#include "fheco/ir/common.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
namespace fheco
{
  namespace ir
  {
    class Function;
  } // namespace ir

class Ciphertext
{
public:
  explicit Ciphertext(std::vector<std::size_t> shape = {Compiler::active_func()->slot_count()});
  /******************************/
  explicit Ciphertext(std::string label,size_t required_dimensions , vector<size_t> shape = {Compiler::active_func()->slot_count()});
  
  explicit Ciphertext(size_t required_dimensions , vector<size_t> shape = {Compiler::active_func()->slot_count()});
  //explicit Ciphertext(std::string label, std::vector<std::size_t> dimensions_sizes = {Compiler::active_func()->slot_count()});
  /*********************************/
  explicit Ciphertext(std::string label ,std::vector<std::size_t> shape = {Compiler::active_func()->slot_count()});
  
  Ciphertext(
    std::string label, PackedVal example_val, std::vector<std::size_t> shape = {Compiler::active_func()->slot_count()});

  Ciphertext(
    std::string label, integer example_val_slot_min, integer example_val_slot_max,
    std::vector<std::size_t> shape = {Compiler::active_func()->slot_count()});

  explicit Ciphertext(const Plaintext &plain);

  ~Ciphertext() = default;

  Ciphertext(const Ciphertext &other) = default;

  Ciphertext(Ciphertext &&other) = default;

  Ciphertext &operator=(const Ciphertext &other);

  Ciphertext &operator=(Ciphertext &&other);

  const Ciphertext operator[](std::size_t idx) const;

  Ciphertext &operator[](std::size_t idx);
 /*****************************/
 // For non-const usage, returning a reference to a new instance if needed
 template<typename... Args>
 Ciphertext& operator()(Args&&...  args) const {
    std::vector<Var> arg_list = {std::forward<Args>(args)...};
    return this->apply_operator(arg_list);
 }
 // For const usage, returning a new instance if needed
/*  template<typename... Args>
 Ciphertext operator()(Args&&...  args) const{
        std::vector<Var> arg_list = {std::forward<Args>(args)...};
        return this->apply_operator(arg_list);
 } */
 /*****************************/
  const Ciphertext &set_output(std::string label) const;

  inline void set_scalar() { set_shape({}); }

  void set_shape(std::vector<std::size_t> shape);

  inline std::size_t id() const { return id_; }

  inline const std::vector<std::size_t> shape() const { return shape_; }

  inline const std::vector<std::size_t> &idx() const { return idx_; }

  inline const std::optional<PackedVal> &example_val() const { return example_val_; }
  /************************************************************/
  void set_dimensions_sizes(std::vector<std::size_t> dims);
  void set_minimum_coordinates(std::vector<std::size_t> minimum_coordinates);
  /************************************************************/

private:
  /**********************************/
  std::size_t required_dimensions_ ;
  std::vector<std::size_t> dimensions_sizes_ ; 
  std::vector<std::size_t> minimum_coordinates_ ;
  std::vector<Var> args_ ;
  Ciphertext& apply_operator(const std::vector<Var> &args) const;

  /****************************************/ 
  // terms ids start from 1
  std::size_t id_;
  
  std::vector<std::size_t> shape_;

  std::vector<std::size_t> idx_;

  std::optional<PackedVal> example_val_;

  friend class ir::Func;
  friend class Var ; 
  friend Ciphertext emulate_subscripted_read(const Ciphertext &arg);
  friend void emulate_subscripted_write(Ciphertext &lhs, const Ciphertext &rhs);

  };
} // namespace fheco
