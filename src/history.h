#ifndef HISTORY_H
#define HISTORY_H

#include "math/tensors.h"
#include "math/rotations.h"

#include <string>
#include <map>

namespace neml {

/// Enum type of each allowable object
enum StorageType {
  TYPE_VECTOR    = 0,
  TYPE_SCALAR    = 1,
  TYPE_ARRAY     = 2,
  TYPE_RANKTWO   = 3,
  TYPE_SYMMETRIC = 4,
  TYPE_SKEW      = 5,
  TYPE_ROT       = 6
};

/// Black magic to map a type to the enum
template <class T> constexpr StorageType GetStorageType();
template <> constexpr StorageType GetStorageType<Vector>() {return TYPE_VECTOR;}
template <> constexpr StorageType GetStorageType<RankTwo>() {return TYPE_RANKTWO;}
template <> constexpr StorageType GetStorageType<Symmetric>() {return TYPE_SYMMETRIC;}
template <> constexpr StorageType GetStorageType<Skew>() {return TYPE_SKEW;}
template <> constexpr StorageType GetStorageType<Orientation>() {return TYPE_ROT;}
template <> constexpr StorageType GetStorageType<double>() {return TYPE_SCALAR;}

/// Black magic to map a type to the right size
template <class T> constexpr size_t GetStorageSize();
template <> constexpr size_t GetStorageSize<Vector>() {return 3;}
template <> constexpr size_t GetStorageSize<RankTwo>() {return 9;}
template <> constexpr size_t GetStorageSize<Symmetric>() {return 6;}
template <> constexpr size_t GetStorageSize<Skew>() {return 3;}
template <> constexpr size_t GetStorageSize<Orientation>() {return 4;}
template <> constexpr size_t GetStorageSize<double>() {return 1;}

class History {
 public:
  History();
  History(bool store);
  History(const History & other);
  History(const History && other);
  virtual ~History();

  /// Copy constructor
  History & operator=(const History & other);
  History & operator=(const History && other);

  /// Explicit deepcopy
  History deepcopy() const;

  /// Do I own my own data?
  bool store() const {return store_;};
  /// Raw data pointer (const)
  const double * rawptr() const {return storage_;};
  /// Raw data pointer (nonconst)
  double * rawptr() {return storage_;};
  
  /// Set storage to some external pointer
  void set_data(double * input);
  /// Copy data from some external pointer
  void copy_data(const double * const input);
  /// Size of storage required
  size_t size() const;
  
  /// Add a generic object
  template<typename T>
  void add(std::string name)
  {
    error_if_exists_(name);
    loc_.insert(std::pair<std::string,size_t>(name, size_));
    type_.insert(std::pair<std::string,StorageType>(name, GetStorageType<T>()));
    resize(GetStorageSize<T>());
  }
 
  template<class T>
  struct item_return{ typedef T type; };

  template<class T>
  typename item_return<T>::type get(std::string name) const
  {
    error_if_not_exists_(name);
    error_if_wrong_type_(name, GetStorageType<T>());
    return T(&(storage_[loc_.at(name)]));
  }

  /// Getters
  const std::map<std::string,size_t> & get_loc() const {return loc_;};
  const std::map<std::string,StorageType> & get_type() const {return type_;};

  std::map<std::string,size_t> & get_loc() {return loc_;};
  std::map<std::string,StorageType> & get_type() {return type_;};

  /// Resize method
  void resize(size_t inc);

  /// Multiply everything by a scalar
  void scalar_multiply(double scalar);

  /// Add another history to this one
  History & operator+=(const History & other);

 protected:
  void copy_maps_(const History & other);

 private:
  void error_if_exists_(std::string name) const;
  void error_if_not_exists_(std::string name) const;
  void error_if_wrong_type_(std::string name, StorageType type) const;

 private:
  size_t size_;
  bool store_;
  double * storage_;

  std::map<std::string,size_t> loc_;
  std::map<std::string,StorageType> type_;
};

template<>
struct History::item_return<double>{ typedef double & type;};

template<>
inline History::item_return<double>::type History::get<double>(std::string name) const
{
  error_if_not_exists_(name);
  error_if_wrong_type_(name, GetStorageType<double>());
  return storage_[loc_.at(name)];
}

} // namespace neml

#endif // HISTORY_H
