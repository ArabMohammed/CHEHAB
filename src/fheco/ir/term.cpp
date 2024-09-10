#include "fheco/ir/term.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <utility>

using namespace std;

namespace fheco::ir
{
size_t Term::count_ = 0;

Term::Term(OpCode op_code, vector<Term *> operands)
  : id_{++count_}, op_code_{move(op_code)}, operands_{move(operands)}, type_{deduce_result_type(op_code_, operands_)}
{}

size_t Term::HashPtr::operator()(const Term *p) const
{
   // Ensure p is not null before dereferencing
    if (p == nullptr) {
        return 0;
    }
    
    // Combine hashes of relevant members of Term
    // Assuming Term has some members like id_, name_, etc.
    std::size_t hash = std::hash<int>()(p->id());  // Example of hashing an integer member
    // You can combine hashes of other members as well to make it more unique
    // hash ^= std::hash<std::string>()(p->name()) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

bool Term::EqualPtr::operator()(const Term *lhs, const Term *rhs) const
{
   // Handle case where either pointer is null
    if (lhs == rhs) return true;  // Same pointer or both null
    if (lhs == nullptr || rhs == nullptr) return false;

    // Compare the relevant members of Term
    return lhs->id() == rhs->id();
    // You can add more conditions if Term has more relevant fields
    // return lhs->id() == rhs->id() && lhs->name() == rhs->name();
}

bool Term::ComparePtr::operator()(const Term *lhs, const Term *rhs) const
{
  return *lhs < *rhs;
}

string Term::term_type_str_repr(Type term_type)
{
  switch (term_type)
  {
  case Type::cipher:
    return "ctxt";

  case Type::plain:
    return "ptxt";

  default:
    throw invalid_argument("invalid term_type");
  }
}

Term::Type Term::deduce_result_type(const OpCode &op_code, const vector<Term *> &operands)
{
  vector<Type> operands_types;
  operands_types.reserve(operands.size());
  transform(operands.cbegin(), operands.cend(), back_inserter(operands_types), [](const Term *operand) {
    return operand->type();
  });
  return deduce_result_type(op_code, operands_types);
}

Term::Type Term::deduce_result_type(const OpCode &op_code, const vector<Type> &operands_types)
{
  if (op_code.arity() != operands_types.size())
    throw invalid_argument("invalid number of operands for op_code");

  if (op_code.arity() == 0)
    throw invalid_argument("cannot deduce result type of operation with 0 operands");

  // non arithmetic operations
  if (op_code.type() == OpCode::Type::encrypt)
  {
    if (operands_types[0] != Type::plain)
      throw invalid_argument("encrypt arg must be plaintext");

    return Type::cipher;
  }

  if (op_code.type() == OpCode::Type::mod_switch)
  {
    if (operands_types[0] != Type::cipher)
      throw invalid_argument("mod_switch arg must be ciphertext");

    return Type::cipher;
  }

  if (op_code.type() == OpCode::Type::relin)
  {
    if (operands_types[0] != Type::cipher)
      throw invalid_argument("relin arg must be cipher");

    return Type::cipher;
  }

  // arithmetic operations
  switch (op_code.arity())
  {
  case 1:
    return operands_types[0];

  case 2:
    return min(operands_types[0], operands_types[1]);

  default:
    throw logic_error("unhandled term type deduction for operations with arity > 2");
  }
}

ostream &operator<<(ostream &os, Term::Type term_type)
{
  switch (term_type)
  {
  case Term::Type::cipher:
    os << "cipher";
    break;

  case Term::Type::plain:
    os << "plain";
    break;

  default:
    throw invalid_argument("invalid term type");
  }
  return os;
}
} // namespace fheco::ir
