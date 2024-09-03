#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include<iostream>
namespace fheco
{
    class Var 
    {
        public:
            /*Construct a new variable with a name and a specific upper 
            and lower bound value, If a variable with the same name 
            already exists this costructor will fail*/
            explicit Var(const std::string& n , const int lower_bound, const int upper_bound);
            
            Var(const std::string& n , const int lower_bound, const int upper_bound,const int rotation_steps);
            /****** define an integer var ******/
            Var(const int n);
            // construct a new variable without specifying name 
            // It can be used when dealing with integer values 
            // lower_bound will be set to 0 , upper_bound will be set to 1 
            explicit Var();
            // Copy constructor
            Var(const Var &other);
            // desctructor 
            ~Var() = default;
            /** Get the name of a Var */
            const std::string &name() const;
            /** Test if two Vars are the same. This simply compares the names. */
            bool same_as(const Var &other) const {
                return upper_bound() == other.upper_bound() && lower_bound()==other.lower_bound();
            }
            // Move assignment operator 
            Var &operator=(Var &&other) noexcept;
            
            // Copy assignment operator
            Var &operator=(const Var &other);
            
            // Move constructor
            Var(Var &&other) noexcept;

            int lower_bound() const{
                return lower_bound_ ;
            }
            int upper_bound() const{
                return upper_bound_ ;
            }
            int rotation_steps()const{
                return rotation_steps_ ;
            }
            std::string name(){
                return name_; 
            }
            /****************************************************************************/
            /****************************************************************************/
            struct VarHash {
                std::size_t operator()(const Var &var) const {
                    return std::hash<std::string>()(var.name());
                }
            };

            struct VarEqual {
                bool operator()(const Var &var1, const Var &var2) const {
                    return var1.same_as(var2);
                }
            };
            /*******************************************************************/
            friend Var operator+(const Var &lhs, const int value);
            friend Var operator+(const int value, const Var &rhs);
            friend Var operator-(const Var &lhs, const int value);
            /*******************************************************************/
            // Function to get the union of two lists of Var objects
            static std::vector<Var> unionOfLists(const std::vector<Var>& list1, const std::vector<Var>& list2) {
                std::unordered_set<Var, VarHash, VarEqual> unionSet;

                // Insert all elements from the first list
                for (const auto& var : list1) {
                    unionSet.insert(var);
                }
                // Insert all elements from the second list
                for (const auto& var : list2) {
                    unionSet.insert(var);
                }
                // Convert the unordered_set to a vector
                std::vector<Var> result(unionSet.begin(), unionSet.end());
                return result;
            }
            static std::unordered_set<std::string> declared_vars_ ;
        private : 
            int lower_bound_ ;
            int upper_bound_ ;
            int rotation_steps_ = 0 ;
            std::string name_ ; 
        };
}