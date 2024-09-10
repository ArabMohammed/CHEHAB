#include "fheco/dsl/common.hpp"
#include "fheco/dsl/compiler.hpp"
#include "fheco/util/common.hpp"


using namespace std;

namespace fheco
{
  void validate_shape(const vector<size_t> &shape)
  {
    size_t slot_count = 1;
    for (auto dim_size : shape)
      slot_count = util::mul_safe(slot_count, dim_size);

    if (slot_count > Compiler::active_func()->slot_count())
      throw invalid_argument("shape too large, total number of elements must be <= function slot count");
  }
  /***************************************************************************************************************************/
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
    /***********************************************************************************************************/
    std::tuple<std::vector<size_t>, std::vector<size_t>, std::vector<size_t>> match_positions(
    const std::vector<Var>& iterator_variables_,
    const std::vector<Var>& reduction_variables_,
    const std::vector<Var>& vars0,
    const std::vector<Var>& vars1)
    {
        std::vector<size_t> reduction_pos;
        std::vector<size_t> vars0_pos;
        std::vector<size_t> vars1_pos;
        bool added = false ;
        int reduction_size=1;
        for (int i = 0; i < iterator_variables_.size(); ++i) {
            added=false ;
            for (int j = 0; j < reduction_variables_.size(); ++j) {
                if (iterator_variables_[i].same_as(reduction_variables_[j])) {
                    reduction_pos.push_back(j);
                    added=true;
                    break;
                }
            }

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
            /***************/
            if(!added){
                reduction_size=reduction_size*(iterator_variables_[i].upper_bound()-iterator_variables_[i].lower_bound());
            }
        }
        return std::make_tuple(reduction_pos, vars0_pos, vars1_pos);
    }
} // namespace fheco
