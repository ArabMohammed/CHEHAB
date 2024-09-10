#pragma once 
#include<vector>
namespace fheco
{
template <typename T>
class DynamicTensor {
public:
    DynamicTensor() = default; // Default constructor
    
    DynamicTensor(const std::vector<size_t>& dimensions) : dimensions_(dimensions) {
        size_ = 1;
        for (size_t dim : dimensions_) {
            size_ *= dim;
        }
        data_.resize(size_);
    }

    T& operator()(const std::vector<size_t>& indices) {
        return data_[get_flat_index(indices)];
    }

    const T& operator()(const std::vector<size_t>& indices) const {
        return data_[get_flat_index(indices)];
    }

    DynamicTensor<T> subtensor(const std::vector<size_t>& indices) {
        std::vector<size_t> sub_dims(dimensions_.begin() + indices.size(), dimensions_.end());
        DynamicTensor<T> sub_tensor(sub_dims);

        std::vector<size_t> sub_indices(sub_dims.size(), 0);
        do {
            std::vector<size_t> full_indices = indices;
            full_indices.insert(full_indices.end(), sub_indices.begin(), sub_indices.end());
            sub_tensor(sub_indices) = this->operator()(full_indices);
        } while (increment_indices(sub_dims, sub_indices));

        return sub_tensor;
    }
    const DynamicTensor<T> subtensor(const std::vector<size_t>& indices) const{
        std::vector<size_t> sub_dims(dimensions_.begin() + indices.size(), dimensions_.end());
        DynamicTensor<T> sub_tensor(sub_dims);

        std::vector<size_t> sub_indices(sub_dims.size(), 0);
        do {
            std::vector<size_t> full_indices = indices;
            full_indices.insert(full_indices.end(), sub_indices.begin(), sub_indices.end());
            sub_tensor(sub_indices) = this->operator()(full_indices);
        } while (increment_indices(sub_dims, sub_indices));

        return sub_tensor;
    }

    // Assignment operator between subtensors
    void assign_subtensor(const std::vector<size_t>& indices, const DynamicTensor<T>& source) {
        std::vector<size_t> sub_dims(dimensions_.begin() + indices.size(), dimensions_.end());
        std::vector<size_t> sub_indices(sub_dims.size(), 0);

        do {
            std::vector<size_t> full_indices = indices;//{0,1}//{1}
            full_indices.insert(full_indices.end(), sub_indices.begin(), sub_indices.end());
            this->operator()(full_indices) = source(sub_indices);
        } while (increment_indices(sub_dims, sub_indices));
    }
private:
    std::vector<size_t> dimensions_;
    std::vector<T> data_;
    size_t size_;

    size_t get_flat_index(const std::vector<size_t>& indices) const {
        size_t flat_index = 0;
        size_t multiplier = 1;
        for (int i = dimensions_.size() - 1; i >= 0; --i) {
            flat_index += indices[i] * multiplier;
            multiplier *= dimensions_[i];
        }
        return flat_index;
    }
    bool increment_indices(const std::vector<size_t>& dims, std::vector<size_t>& indices) const {
        for (int i = indices.size() - 1; i >= 0; --i) {
            if (++indices[i] < dims[i]) {
                return true;
            } else {
                indices[i] = 0;
            }
        }
        return false;
    }
};
}