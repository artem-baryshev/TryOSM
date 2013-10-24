#ifndef POINTERS_H
#define POINTERS_H

#include <stdlib.h>
#include <memory>

template <class T>
class TSharedPtr;

template <class T>
class TWeakPtr;

template <class T>
class TSharedPtr
{
public:
    TSharedPtr(T *pointer = NULL)
    {
        count = new size_t((pointer == NULL) ? 0 : 1);
        ptr = pointer;
    }

    TSharedPtr(const TSharedPtr &src)
    {
        ++*src.count;
        ptr = src.ptr;
        count = src.count;
    }

    bool operator ==(const TSharedPtr &src)
    {
        return src.ptr == ptr;
    }

    TSharedPtr & operator = (const TSharedPtr &src)
    {
        if (src.ptr == ptr)
        {
            return *this;
        }
        if (ptr != NULL)
        {
            if (*count <= 1)
            {
                delete ptr;
                delete count;
            }
            else
            {
                --*count;
            }
        }
        ++*src.count;
        ptr = src.ptr;
        count = src.count;
        return *this;
    }

    void reset(T *pointer)
    {
        if (*count <= 1)
        {
            delete count;
            delete ptr;
        }
        else
        {
            --*count;
        }
        count = new size_t(1);
        ptr = pointer;
    }

    ~TSharedPtr()
    {
        if (*count <= 1)
        {
            delete count;
            delete ptr;
        }
        else
        {
            --*count;
        }
    }

    T * get()
    {
        return ptr;
    }

    T & operator * ()
    {
        return *ptr;
    }

    T * operator -> ()
    {
        return ptr;
    }

protected:
private:
    T *ptr;
    size_t *count;
};

template <class T>
class TWeakPtr
{
public:
    TWeakPtr(T *pointer)
    {
        ptr = pointer;
    }

    TWeakPtr(const TWeakPtr &src)
    {
        ptr = src.ptr;
    }

    TWeakPtr & operator = (const TWeakPtr &src)
    {
        ptr = src.ptr;
    }

    T * get()
    {
        return ptr;
    }

    T & operator * ()
    {
        return *ptr;
    }

    T * operator -> ()
    {
        return ptr;
    }

    ~TWeakPtr()
    {
    }

protected:
private:
    T *ptr;
};

#endif // POINTERS_H
