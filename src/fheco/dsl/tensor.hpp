#pragma once
#include <vector>
#include <utility> // for std::move
#include <iostream>
namespace fheco {

template <typename T>
class DynamicTensor {
public:
    // Default constructor
    DynamicTensor() : size_(0) {}

    // Constructor with dimensions
    DynamicTensor(const std::vector<size_t>& dimensions) : dimensions_(dimensions) {
        size_ = 1;
        for (size_t dim : dimensions_) {
            size_ *= dim;
        }
        data_.resize(size_);
    }

    // Copy constructor
    DynamicTensor(const DynamicTensor& other)
        : dimensions_(other.dimensions_), data_(other.data_), size_(other.size_) {}

    // Move constructor
    DynamicTensor(DynamicTensor&& other) noexcept
        : dimensions_(std::move(other.dimensions_)), data_(std::move(other.data_)), size_(other.size_) {
        other.size_ = 0;
    }

    // Copy assignment operator
    DynamicTensor& operator=(const DynamicTensor& other) {
        if (this == &other) {
            return *this;  // Handle self-assignment
        }
       std::cout<<"Copy assignment operator ==> \n";
        dimensions_ = other.dimensions_;
        data_ = other.data_;
        size_ = other.size_;

        return *this;
    }

    // Move assignment operator
    DynamicTensor& operator=(DynamicTensor&& other) noexcept {
        if (this == &other) {
            return *this;  // Handle self-assignment
        }
        std::cout << "Move assignment operator ==> \n";

        dimensions_ = std::move(other.dimensions_);
        data_ = std::move(other.data_);
        size_ = other.size_;

        // Leave the moved-from object in a valid but empty state
        other.size_ = 0;
        other.data_.clear(); // or other.data_ = {}; if it's a vector or similar
        other.dimensions_.clear(); // or other.dimensions_ = {};

        return *this;
    }
    // Const access to an element
    const T& operator()(const std::vector<size_t>& indices) const {
        std::cout<<"non mutable access in tensor \n";
        return data_[get_flat_index(indices)];
    }
    const T& get_value(const std::vector<size_t>& indices) const {
        std::cout << "Non-mutable access using get_value with size :"<<size_<<"\n";
        return data_[get_flat_index(indices)];
    }
    // Access an element by multi-dimensional indices
    T& operator()(const std::vector<size_t>& indices) {
        std::cout<<"mutable access in tensor : "<<size_<<"\n";
        for(auto val : dimensions_){
            std::cout<<val<<" ";
        }
        std::cout<<"\n";
        return data_[get_flat_index(indices)];
    }
    /***set the value of a position  */
    void set_value(const T& value,const std::vector<size_t>& indices ){
        std::cout << "mutable access using set_value with size :"<<size_<<"\n";
        data_[get_flat_index(indices)]=value ;
    }
    
    // Subtensor extraction
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

    const DynamicTensor<T> subtensor(const std::vector<size_t>& indices) const {
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

    // Resize the data of the tensor
    void resize(const std::vector<size_t>& new_dimensions) {
        dimensions_ = new_dimensions; // Update dimensions
        size_ = 1;
        for (size_t dim : dimensions_) {
            size_ *= dim; // Recalculate the total size
        }
        data_.resize(size_); // Resize the underlying data vector
    }

    // Assignment operator between subtensors
    void assign_subtensor(const std::vector<size_t>& indices, const DynamicTensor<T>& source) {
        std::vector<size_t> sub_dims(dimensions_.begin() + indices.size(), dimensions_.end());
        std::vector<size_t> sub_indices(sub_dims.size(), 0);

        do {
            std::vector<size_t> full_indices = indices;
            full_indices.insert(full_indices.end(), sub_indices.begin(), sub_indices.end());
            this->operator()(full_indices) = source(sub_indices);
        } while (increment_indices(sub_dims, sub_indices));
    }
    const std::vector<size_t> dimensions(){
        return dimensions_ ;
    }
private:
    std::vector<size_t> dimensions_;  // Stores the tensor's dimensions
    std::vector<T> data_;  // Stores the tensor's data as a flattened array
    size_t size_;  // Total number of elements in the tensor

    // Helper function to get the flattened index from multi-dimensional indices
    size_t get_flat_index(const std::vector<size_t>& indices) const {
        size_t flat_index = 0;
        size_t multiplier = 1;
        for (int i = dimensions_.size() - 1; i >= 0; --i) {
            flat_index += indices[i] * multiplier;
            multiplier *= dimensions_[i];
        }
        return flat_index;
    }

    // Helper function to increment multi-dimensional indices
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

} // namespace fheco
