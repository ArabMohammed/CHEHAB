#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>
#include <stdexcept>
#include "fheco/dsl/var.hpp"
namespace fheco
{
using integer = std::int64_t;

using PackedVal = std::vector<integer>;

 enum class CiphertextType
    {
      ciphertxt,
      vectorciphertxt,
      plaintxt
    };

// Define an alias for the enum
using Type = CiphertextType;

enum class SecurityLevel
{
  none,
  tc128,
  tq128,
  tc192,
  tq192,
  tc256,
  tq256
};
void generateNestedLoops(const std::vector<std::pair<int, int>>& ranges, std::function<void(const std::vector<int>&)> body) ;

void validate_shape(const std::vector<std::size_t> &shape);

std::tuple<std::vector<size_t>, std::vector<size_t>, std::vector<size_t>> match_positions(
    const std::vector<Var>& iterator_variables_,
    const std::vector<Var>& reduction_variables_,
    const std::vector<Var>& vars0,
    const std::vector<Var>& vars1);

} // namespace fheco
