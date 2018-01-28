#ifndef _QUEUE_HPP
#define _QUEUE_HPP

#include <stddef.h>
#include "screen.h"

/**
 * @brief This Queue class allows data to be passed between two threads
 * (or between a thread and an interrupt handler). It is thread-safe as
 * long as one thread only calls enqueue() and the other thread only calls
 * dequeue(). It is NOT thread-safe if each thread makes calls to both
 * enqueue() and dequeue().
 */
template<typename T, size_t MAX_SIZE>
class Queue
{
public:
    Queue();

    size_t getSize() const;

    bool isEmpty() const;

    bool isFull() const;

    bool enqueue(T item);

    size_t enqueue(const T* buff, size_t buffSize);

    bool dequeue(T& item);

    size_t dequeue(T* buff, size_t buffSize);

    void print();

private:
    T array[MAX_SIZE];
    size_t head;
    size_t tail;
    size_t size;
};

template<typename T, size_t MAX_SIZE>
Queue<T, MAX_SIZE>::Queue()
{
    head = 0;
    tail = 0;
    size = 0;
}

template<typename T, size_t MAX_SIZE>
size_t Queue<T, MAX_SIZE>::getSize() const
{
    return size;
}

template<typename T, size_t MAX_SIZE>
bool Queue<T, MAX_SIZE>::isEmpty() const
{
    return size == 0;
}

template<typename T, size_t MAX_SIZE>
bool Queue<T, MAX_SIZE>::isFull() const
{
    return size == MAX_SIZE;
}

template<typename T, size_t MAX_SIZE>
bool Queue<T, MAX_SIZE>::enqueue(T item)
{
    if (isFull())
    {
        return false;
    }

    array[tail] = item;
    ++tail;
    if (tail >= MAX_SIZE)
    {
        tail = 0;
    }
    ++size;

    return true;
}

template<typename T, size_t MAX_SIZE>
size_t Queue<T, MAX_SIZE>::enqueue(const T* buff, size_t buffSize)
{
    size_t numToEnqueue = buffSize;
    size_t sizeAvailable = MAX_SIZE - size;
    if (numToEnqueue > sizeAvailable)
    {
        numToEnqueue = sizeAvailable;
    }

    for (size_t i = 0; i < numToEnqueue; ++i)
    {
        array[tail] = buff[i];
        ++tail;
        if (tail >= MAX_SIZE)
        {
            tail = 0;
        }
    }

    size += numToEnqueue;

    return numToEnqueue;
}

template<typename T, size_t MAX_SIZE>
bool Queue<T, MAX_SIZE>::dequeue(T& item)
{
    if (isEmpty())
    {
        return false;
    }

    item = array[head];
    ++head;
    if (head >= MAX_SIZE)
    {
        head = 0;
    }
    --size;

    return true;
}

template<typename T, size_t MAX_SIZE>
size_t Queue<T, MAX_SIZE>::dequeue(T* buff, size_t buffSize)
{
    size_t numToDequeue = buffSize;
    if (numToDequeue > size)
    {
        numToDequeue = size;
    }

    for (size_t i = 0; i < numToDequeue; ++i)
    {
        buff[i] = array[head];
        ++head;
        if (head >= MAX_SIZE)
        {
            head = 0;
        }
    }

    size -= numToDequeue;

    return numToDequeue;
}

template<typename T, size_t MAX_SIZE>
void Queue<T, MAX_SIZE>::print()
{
    if (size == 0)
    {
        screen << "<empty>\n";
    }
    else
    {
        for (size_t i = 0; i < size; ++i)
        {
            screen << array[i] << ' ';
        }
        screen << '\n';
    }
}

#endif // _QUEUE_HPP
