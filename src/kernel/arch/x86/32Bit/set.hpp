#ifndef _SET_HPP
#define _SET_HPP

#include <stddef.h>

template<typename T, size_t MAX_SIZE>
class Set
{
public:
    Set();

    size_t getSize() const;

    void add(T item);

    void remove(T item);

    void clear();

    T operator [](size_t idx) const;

private:
    T array[MAX_SIZE];
    size_t size;
};

template<typename T, size_t MAX_SIZE>
Set<T, MAX_SIZE>::Set()
{
    size = 0;
}

template<typename T, size_t MAX_SIZE>
size_t Set<T, MAX_SIZE>::getSize() const
{
    return size;
}

template<typename T, size_t MAX_SIZE>
void Set<T, MAX_SIZE>::add(T item)
{
    array[size++] = item;
}

template<typename T, size_t MAX_SIZE>
void Set<T, MAX_SIZE>::remove(T item)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (array[i] == item)
        {
            array[i] = array[size - 1];
            --size;
            break;
        }
    }
}

template<typename T, size_t MAX_SIZE>
void Set<T, MAX_SIZE>::clear()
{
    size = 0;
}

template<typename T, size_t MAX_SIZE>
T Set<T, MAX_SIZE>::operator [](size_t idx) const
{
    return array[idx];
}

#endif // _SET_HPP
