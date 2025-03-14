#pragma once

#include "value.h"

#include <utility> // std::move()

namespace Surface::JSON
{

struct ArrayIterator
{
    ArrayIterator(Value* ptr) : ptr(ptr) {};

    ArrayIterator& operator++() noexcept
    {
        ++ptr;
        return *this;
    }

    ArrayIterator operator++(int) noexcept
    {
        ArrayIterator cpy = *this;
        ++(*this);
        return cpy;
    }

    ArrayIterator& operator--() noexcept
    {
        --ptr;
        return *this;
    }

    ArrayIterator operator--(int) noexcept
    {
        ArrayIterator cpy = *this;
        --(*this);
        return cpy;
    }

    inline Value& operator[](const size_t index) const noexcept
    {
        return *(ptr + index);
    }

    inline Value* operator->() const noexcept
    {
        return ptr;
    }

    inline Value& operator*() const noexcept
    {
        return *ptr;
    }

    inline bool operator==(const ArrayIterator& other) const noexcept
    {
        return ptr == other.ptr;
    }

    inline bool operator!=(const ArrayIterator& other) const noexcept
    {
        return !(*this == other);
    }

  private:
    Value* ptr;
};

struct ArrayConstIterator
{
    ArrayConstIterator(const Value* ptr) : ptr(ptr) {};

    ArrayConstIterator& operator++() noexcept
    {
        ++ptr;
        return *this;
    }

    ArrayConstIterator operator++(int) noexcept
    {
        ArrayConstIterator cpy = *this;
        ++(*this);
        return cpy;
    }

    ArrayConstIterator& operator--() noexcept
    {
        --ptr;
        return *this;
    }

    ArrayConstIterator operator--(int) noexcept
    {
        ArrayConstIterator cpy = *this;
        --(*this);
        return cpy;
    }

    inline const Value& operator[](const size_t index) const noexcept
    {
        return *(ptr + index);
    }

    inline const Value* operator->() const noexcept
    {
        return ptr;
    }

    inline const Value& operator*() const noexcept
    {
        return *ptr;
    }

    bool operator==(const ArrayConstIterator& other) const noexcept
    {
        return ptr == other.ptr;
    }

    bool operator!=(const ArrayConstIterator& other) const noexcept
    {
        return !(*this == other);
    }

  private:
    const Value* ptr;
};

// JSON Array type, ordered
struct Array
{
  public: // Constructors
    // Copy
    Array(const Array& other);

    // Move
    Array(Array&& other) noexcept;

    ~Array();

  public: // Operators
    // Set the value at an index
    Value& operator[](size_t index);

    // Get the value at an index
    const Value& operator[](size_t index) const;

    // Move other array data into this array
    Array& operator=(Array&& other) noexcept;

    // Copy
    Array& operator=(const Array& other);

    // Arrays are never equal to each other, unless they point to the same memory, which is
    // normally impossible
    inline bool operator==(const Array& right)
    {
        return this == &right;
    }

    inline bool operator!=(const Array& right)
    {
        return !(*this == right);
    }

  public: // Methods
    // Deletes all elements from the list, returning the previous size of the array.
    size_t clear();

    // Retrieve the value at a given index. Modifying the value will modify it in the array.
    inline Value& element_at(size_t index)
    {
        return (*this)[index];
    };

    // Retrieve the value at a given index.
    inline const Value& element_at(size_t index) const
    {
        return (*this)[index];
    };

    /**
     * @brief Finds the first index which contains an equal value, returns `(uint) -1` if it does
     * not exist.
     *
     * When searching by numbers, please see the == operator for Value, which considers numbers of
     * different types as unequal.
     *
     * @param value value
     * @return index, maximum unsigned long long value if it does not exist.
     */
    size_t find(const Value& value) const;

    template <class Type> inline size_t find(const Type& value) const
    {
        return find(Value(value));
    }

    // Get the size of this array, which is the number of values
    inline size_t get_size() const
    {
        return size;
    }

    // Inserts an element at a specified index, throws if index > size(), returns the new size
    size_t insert(const Value& value, size_t index);
    size_t insert(Value&& value, size_t index);

    template <class Type> inline size_t insert(const Type& value, size_t index)
    {
        return insert(Value(value), index);
    }

    template <class Type> size_t insert(Type&& value, size_t index)
    {
        return insert(Value(std::move(value)), index);
    }

    // Adds an element to the end of the array, returning the new size
    inline size_t push(const Value& value)
    {
        return insert(value, size);
    };

    // Adds an element to the end of the array, returning the new size
    size_t push(Value&& value);

    template <class Type> inline size_t push(const Type& value)
    {
        return push(Value(value));
    }

    template <class Type> size_t push(Type&& value)
    {
        return push(Value(std::move(value)));
    }

    /**
     * @brief Removes the first element in the array equal to value, returns true if a value was
     * removed.
     *
     * This method deletes the matched value and updates the indexes of all following values. Use
     * remove_all() if you wish to remove any matching values.
     *
     * @param value value to remove
     * @return true if a value was removed.
     */
    bool remove(const Value& value);

    /**
     * @brief Removes all elements in the array equal to value, returns the number of removed
     * values.
     *
     * This is an optimized method, prefer it to multiple calls to remove() when you intend to
     * remove all occurrences.
     *
     * @param value value to remove
     * @return number of removed values
     */
    size_t remove_all(const Value& value);


  public: // Methods not intended for regular use, available anyway
    // Get the capacity of this array
    inline size_t get_capacity() const
    {
        return capacity;
    }

    /**
     * @brief Set the capacity of this array, returns true if the new capacity was different from
     * the old capacity. This will delete elements from the end if new_capacity < size.
     *
     * Prefer to use this after clearing a large array to be reused with fewer values.
     *
     * @param new_capacity
     * @return true if the array was reallocated
     */
    bool set_capacity(size_t new_capacity);

  public: // Iterators
    using iterator = ArrayIterator;
    using const_iterator = ArrayConstIterator;

    inline iterator begin()
    {
        return iterator(values);
    }

    inline iterator end()
    {
        return iterator(values + size);
    }

    inline const const_iterator begin() const
    {
        return const_iterator(values);
    }

    inline const const_iterator end() const
    {
        return const_iterator(values + size);
    }

  private: // Constructors
    Array(size_t capacity);

  private: // Members
    // Represents the capacity of values
    size_t capacity;

    // Represents the number of values in the array
    size_t size = 0;

    // values
    Value* values = nullptr;

  private: // Methods
    // Common work prior to inserting a value at an index
    void _pre_insert(size_t index);

    // Resizes so that capacity >= new_size, and updates array values to new capacity
    void resize(size_t new_size);

    // Updates array values to the current capacity
    void update_capacity();

  private: // Common data operations
    // Copy
    void copy_from(const Array& other);

    // Move
    void move_from(Array&& other);

    // Delete
    void delete_data();

#ifdef DEBUG
  public: // debug only
    inline static int _constructs()
    {
        return __constructs;
    }

    inline static int _copies()
    {
        return __copies;
    }

    inline static int _moves()
    {
        return __moves;
    }

  private:
    static int __constructs, __copies, __moves;
#endif

  public: // Friends
    friend Value;
};

} // namespace Surface::JSON
