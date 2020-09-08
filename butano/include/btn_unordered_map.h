#ifndef BTN_UNORDERED_MAP_H
#define BTN_UNORDERED_MAP_H

#include <new>
#include "btn_memory.h"
#include "btn_iterator.h"
#include "btn_algorithm.h"
#include "btn_power_of_two.h"
#include "btn_unordered_map_fwd.h"

namespace btn
{

template<typename Key, typename Value, typename KeyHash, typename KeyEqual>
class iunordered_map
{

public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = pair<const key_type, mapped_type>;
    using size_type = int;
    using hash_type = unsigned;
    using hasher = KeyHash;
    using key_equal = KeyEqual;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    class iterator
    {

    public:
        using key_type = iunordered_map::key_type;
        using mapped_type = iunordered_map::mapped_type;
        using value_type = iunordered_map::value_type;
        using size_type = iunordered_map::size_type;
        using hash_type = iunordered_map::hash_type;
        using hasher = iunordered_map::hasher;
        using key_equal = iunordered_map::key_equal;
        using reference = iunordered_map::reference;
        using const_reference = iunordered_map::const_reference;
        using pointer = iunordered_map::pointer;
        using const_pointer = iunordered_map::const_pointer;
        using iterator_category = bidirectional_iterator_tag;

        iterator& operator++()
        {
            size_type index = _index;
            size_type last_valid_index = _map->_last_valid_index;
            const bool* allocated = _map->_allocated;
            ++index;

            while(index <= last_valid_index && ! allocated[index])
            {
                ++index;
            }

            if(index > last_valid_index)
            {
                index = _map->max_size();
            }

            _index = index;
            return *this;
        }

        iterator& operator--()
        {
            int index = _index;
            int first_valid_index = _map->_first_valid_index;
            const bool* allocated = _map->_allocated;
            --index;

            while(index >= first_valid_index && ! allocated[index])
            {
                --index;
            }

            _index = index;
            return *this;
        }

        [[nodiscard]] const_reference operator*() const
        {
            BTN_ASSERT(_map->_allocated[_index], "Index is not allocated: ", _index);

            return _map->_storage[_index];
        }

        [[nodiscard]] reference operator*()
        {
            BTN_ASSERT(_map->_allocated[_index], "Index is not allocated: ", _index);

            return _map->_storage[_index];
        }

        const_pointer operator->() const
        {
            BTN_ASSERT(_map->_allocated[_index], "Index is not allocated: ", _index);

            return _map->_storage + _index;
        }

        pointer operator->()
        {
            BTN_ASSERT(_map->_allocated[_index], "Index is not allocated: ", _index);

            return _map->_storage + _index;
        }

        [[nodiscard]] friend bool operator==(const iterator& a, const iterator& b)
        {
            return  a._index == b._index;
        }

        [[nodiscard]] friend bool operator!=(const iterator& a, const iterator& b)
        {
            return ! (a == b);
        }

    private:
        friend class iunordered_map;

        size_type _index;
        iunordered_map* _map;

        iterator(size_type index, iunordered_map& map) :
            _index(index),
            _map(&map)
        {
        }
    };

    class const_iterator
    {

    public:
        using key_type = iunordered_map::key_type;
        using mapped_type = iunordered_map::mapped_type;
        using value_type = iunordered_map::value_type;
        using size_type = iunordered_map::size_type;
        using hash_type = iunordered_map::hash_type;
        using hasher = iunordered_map::hasher;
        using key_equal = iunordered_map::key_equal;
        using reference = iunordered_map::reference;
        using const_reference = iunordered_map::const_reference;
        using pointer = iunordered_map::pointer;
        using const_pointer = iunordered_map::const_pointer;
        using iterator_category = bidirectional_iterator_tag;

        const_iterator(const iterator& other) :
            _index(other._index),
            _map(other._map)
        {
        }

        const_iterator& operator++()
        {
            size_type index = _index;
            size_type last_valid_index = _map->_last_valid_index;
            const bool* allocated = _map->_allocated;
            ++index;

            while(index <= last_valid_index && ! allocated[index])
            {
                ++index;
            }

            if(index > last_valid_index)
            {
                index = _map->max_size();
            }

            _index = index;
            return *this;
        }

        const_iterator& operator--()
        {
            int index = _index;
            int first_valid_index = _map->_first_valid_index;
            const bool* allocated = _map->_allocated;
            --index;

            while(index >= first_valid_index && ! allocated[index])
            {
                --index;
            }

            _index = index;
            return *this;
        }

        [[nodiscard]] const_reference operator*() const
        {
            BTN_ASSERT(_map->_allocated[_index], "Index is not allocated: ", _index);

            return _map->_storage[_index];
        }

        const_pointer operator->() const
        {
            BTN_ASSERT(_map->_allocated[_index], "Index is not allocated: ", _index);

            return _map->_storage + _index;
        }

        [[nodiscard]] friend bool operator==(const const_iterator& a, const const_iterator& b)
        {
            return  a._index == b._index;
        }

        [[nodiscard]] friend bool operator!=(const const_iterator& a, const const_iterator& b)
        {
            return ! (a == b);
        }

    private:
        friend class iunordered_map;
        friend class iterator;

        size_type _index;
        const iunordered_map* _map;

        const_iterator(size_type index, const iunordered_map& map) :
            _index(index),
            _map(&map)
        {
        }
    };

    using reverse_iterator = btn::reverse_iterator<iterator>;
    using const_reverse_iterator = btn::reverse_iterator<const_iterator>;

    iunordered_map(const iunordered_map& other) = delete;

    iunordered_map& operator=(const iunordered_map& other)
    {
        if(this != &other)
        {
            BTN_ASSERT(other._size <= max_size(), "Not enough space in map: ", max_size(), " - ", other._size);

            clear();
            _assign(other);
        }

        return *this;
    }

    iunordered_map& operator=(iunordered_map&& other) noexcept
    {
        if(this != &other)
        {
            BTN_ASSERT(other._size <= max_size(), "Not enough space in map: ", max_size(), " - ", other._size);

            clear();
            _assign(move(other));
        }

        return *this;
    }

    [[nodiscard]] size_type size() const
    {
        return _size;
    }

    [[nodiscard]] size_type max_size() const
    {
        return _max_size_minus_one + 1;
    }

    [[nodiscard]] size_type available() const
    {
        return max_size() - _size;
    }

    [[nodiscard]] bool empty() const
    {
        return _size == 0;
    }

    [[nodiscard]] bool full() const
    {
        return _size == max_size();
    }

    [[nodiscard]] const_iterator begin() const
    {
        return const_iterator(_first_valid_index, *this);
    }

    [[nodiscard]] iterator begin()
    {
        return iterator(_first_valid_index, *this);
    }

    [[nodiscard]] const_iterator end() const
    {
        return const_iterator(max_size(), *this);
    }

    [[nodiscard]] iterator end()
    {
        return iterator(max_size(), *this);
    }

    [[nodiscard]] const_iterator cbegin() const
    {
        return const_iterator(_first_valid_index, *this);
    }

    [[nodiscard]] const_iterator cend() const
    {
        return const_iterator(max_size(), *this);
    }

    [[nodiscard]] const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    [[nodiscard]] reverse_iterator rbegin()
    {
        return reverse_iterator(end());
    }

    [[nodiscard]] const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    [[nodiscard]] reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    [[nodiscard]] const_reverse_iterator crbegin() const
    {
        return const_reverse_iterator(cend());
    }

    [[nodiscard]] const_reverse_iterator crend() const
    {
        return const_reverse_iterator(cbegin());
    }

    [[nodiscard]] bool contains(const key_type& key) const
    {
        if(empty())
        {
            return false;
        }

        return contains_hash(hasher()(key), key);
    }

    [[nodiscard]] bool contains_hash(hash_type key_hash, const key_type& key) const
    {
        return find_hash(key_hash, key) != end();
    }

    [[nodiscard]] size_type count(const key_type& key) const
    {
        return count_hash(hasher()(key), key);
    }

    [[nodiscard]] size_type count_hash(hash_type key_hash, const key_type& key) const
    {
        return contains_hash(key_hash, key) ? 1 : 0;
    }

    [[nodiscard]] const_iterator find(const key_type& key) const
    {
        return const_cast<iunordered_map&>(*this).find(key);
    }

    [[nodiscard]] iterator find(const key_type& key)
    {
        if(empty())
        {
            return end();
        }

        return find_hash(hasher()(key), key);
    }

    [[nodiscard]] const_iterator find_hash(hash_type key_hash, const key_type& key) const
    {
        return const_cast<iunordered_map&>(*this).find_hash(key_hash, key);
    }

    [[nodiscard]] iterator find_hash(hash_type key_hash, const key_type& key)
    {
        if(empty())
        {
            return end();
        }

        const_pointer storage = _storage;
        const bool* allocated = _allocated;
        key_equal key_equal_functor;
        size_type index = _index(key_hash);
        size_type max_size = _max_size_minus_one + 1;
        size_type its = 0;

        while(its < max_size && allocated[index])
        {
            if(key_equal_functor(key, storage[index].first))
            {
                return iterator(index, *this);
            }

            index = _index(index + 1);
            ++its;
        }

        return end();
    }

    [[nodiscard]] const mapped_type& at(const key_type& key) const
    {
        return const_cast<iunordered_map&>(*this).at(key);
    }

    [[nodiscard]] mapped_type& at(const key_type& key)
    {
        return at_hash(hasher()(key), key);
    }

    [[nodiscard]] const mapped_type& at_hash(hash_type key_hash, const key_type& key) const
    {
        return const_cast<iunordered_map&>(*this).at_hash(key_hash, key);
    }

    [[nodiscard]] mapped_type& at_hash(hash_type key_hash, const key_type& key)
    {
        iterator it = find_hash(key_hash, key);
        BTN_ASSERT(it != end(), "Key not found");

        return it->second;
    }

    iterator insert(const value_type& value)
    {
        return insert_hash(hasher()(value.first), value);
    }

    iterator insert(value_type&& value)
    {
        return insert_hash(hasher()(value.first), move(value));
    }

    iterator insert(const key_type& key, const mapped_type& mapped_value)
    {
        return insert_hash(hasher()(key), value_type(key, mapped_value));
    }

    iterator insert(const key_type& key, mapped_type&& mapped_value)
    {
        return insert_hash(hasher()(key), value_type(key, move(mapped_value)));
    }

    iterator insert_hash(hash_type key_hash, const value_type& value)
    {
        return insert_hash(key_hash, value_type(value));
    }

    iterator insert_hash(hash_type key_hash, value_type&& value)
    {
        size_type index = _index(key_hash);
        pointer storage = _storage;
        bool* allocated = _allocated;
        key_equal key_equal_functor;
        size_type current_index = index;

        while(allocated[current_index])
        {
            if(key_equal_functor(value.first, storage[current_index].first))
            {
                return end();
            }

            current_index = _index(current_index + 1);
            BTN_ASSERT(current_index != index, "All indices are allocated");
        }

        ::new(storage + current_index) value_type(move(value));
        allocated[current_index] = true;
        _first_valid_index = min(_first_valid_index, current_index);
        _last_valid_index = max(_last_valid_index, current_index);
        ++_size;
        return iterator(current_index, *this);
    }

    iterator insert_hash(hash_type key_hash, const key_type& key, const mapped_type& mapped_value)
    {
        return insert_hash(key_hash, value_type(key, mapped_value));
    }

    iterator insert_hash(hash_type key_hash, const key_type& key, mapped_type&& mapped_value)
    {
        return insert_hash(key_hash, value_type(key, move(mapped_value)));
    }

    iterator insert_or_assign(const value_type& value)
    {
        return insert_or_assign_hash(hasher()(value.first), value);
    }

    iterator insert_or_assign(value_type&& value)
    {
        return insert_or_assign_hash(hasher()(value.first), move(value));
    }

    iterator insert_or_assign(const key_type& key, const mapped_type& mapped_value)
    {
        return insert_or_assign_hash(hasher()(key), value_type(key, mapped_value));
    }

    iterator insert_or_assign(const key_type& key, mapped_type&& mapped_value)
    {
        return insert_or_assign_hash(hasher()(key), value_type(key, move(mapped_value)));
    }

    iterator insert_or_assign_hash(hash_type key_hash, const value_type& value)
    {
        return insert_or_assign_hash(key_hash, value_type(value));
    }

    iterator insert_or_assign_hash(hash_type key_hash, value_type&& value)
    {
        iterator it = find_hash(key_hash, value.first);

        if(it == end())
        {
            it = insert_hash(key_hash, move(value));
            BTN_ASSERT(it != end(), "Insertion failed");
        }
        else
        {
            pointer storage = _storage;
            size_type index = it._index;
            storage[index].~value_type();
            ::new(storage + index) value_type(move(value));
        }

        return it;
    }

    iterator insert_or_assign_hash(hash_type key_hash, const key_type& key, const mapped_type& mapped_value)
    {
        return insert_or_assign_hash(key_hash, value_type(key, mapped_value));
    }

    iterator insert_or_assign_hash(hash_type key_hash, const key_type& key, mapped_type&& mapped_value)
    {
        return insert_or_assign_hash(key_hash, value_type(key, move(mapped_value)));
    }

    iterator erase(const const_iterator& position)
    {
        bool* allocated = _allocated;
        size_type index = position._index;
        BTN_ASSERT(allocated[index], "Index is not allocated: ", index);

        pointer storage = _storage;
        storage[index].~value_type();
        allocated[index] = false;
        --_size;

        if(_size == 0)
        {
            _first_valid_index = max_size();
            _last_valid_index = 0;
            return end();
        }

        hasher hasher_functor;
        size_type current_index = index;
        size_type next_index = _index(index + 1);

        while(allocated[next_index] && _index(hasher_functor(storage[next_index].first)) != next_index)
        {
            ::new(storage + current_index) value_type(move(storage[next_index]));
            storage[next_index].~value_type();
            allocated[current_index] = true;
            allocated[next_index] = false;
            current_index = next_index;
            next_index = _index(next_index + 1);
        }

        if(_size == 1)
        {
            if(index == _first_valid_index)
            {
                index = _last_valid_index;
                _first_valid_index = _last_valid_index;
            }
            else
            {
                index = _first_valid_index;
                _last_valid_index = _first_valid_index;
            }

            return iterator(index, *this);
        }

        size_type first_valid_index = _first_valid_index;

        while(! allocated[first_valid_index])
        {
            ++first_valid_index;
        }

        _first_valid_index = first_valid_index;

        size_type last_valid_index = _last_valid_index;

        while(! allocated[last_valid_index])
        {
            --last_valid_index;
        }

        _last_valid_index = last_valid_index;

        while(index <= last_valid_index && ! allocated[index])
        {
            ++index;
        }

        return iterator(index, *this);
    }

    bool erase(const key_type& key)
    {
        return erase_hash(hasher()(key), key);
    }

    bool erase_hash(hash_type key_hash, const key_type& key)
    {
        iterator it = find_hash(key_hash, key);

        if(it != end())
        {
            erase(it);
            return true;
        }

        return false;
    }

    template<class Pred>
    friend void erase_if(iunordered_map& map, const Pred& pred)
    {
        pointer storage = map._storage;
        bool* allocated = map._allocated;
        size_type size = map._size;
        size_type first_valid_index = map.max_size();
        size_type last_valid_index = 0;

        for(size_type index = map._first_valid_index, last = map._last_valid_index; index <= last; ++index)
        {
            if(allocated[index])
            {
                if(allocated[index] && pred(storage[index]))
                {
                    storage[index].~value_type();
                    allocated[index] = false;
                    --size;
                }
                else
                {
                    first_valid_index = min(index, first_valid_index);
                    last_valid_index = max(index, last_valid_index);
                }
            }
        }

        map._size = size;
        map._first_valid_index = first_valid_index;
        map._last_valid_index = last_valid_index;
    }

    void merge(iunordered_map&& other) noexcept
    {
        if(this != &other)
        {
            BTN_ASSERT(_max_size_minus_one == other._max_size_minus_one,
                       "Invalid max size: ", max_size(), " - ", other.max_size());

            pointer storage = _storage;
            pointer other_storage = other._storage;
            bool* allocated = _allocated;
            bool* other_allocated = other._allocated;
            size_type size = _size;
            size_type first_valid_index = min(_first_valid_index, other._first_valid_index);
            size_type last_valid_index = max(_last_valid_index, other._last_valid_index);

            for(size_type index = first_valid_index; index <= last_valid_index; ++index)
            {
                if(other_allocated[index])
                {
                    value_type&& other_value = other_storage[index];

                    if(allocated[index])
                    {
                        storage[index] = move(other_value);
                    }
                    else
                    {
                        ::new(storage + index) value_type(move(other_value));
                        ++size;
                    }

                    other_value.~value_type();
                }
            }

            _size = size;
            _first_valid_index = first_valid_index;
            _last_valid_index = last_valid_index;

            int other_max_size = other.max_size();
            memory::clear(other_max_size, *other.allocated);
            other._first_valid_index = other_max_size;
            other._last_valid_index = 0;
            other._size = 0;
        }
    }

    void clear()
    {
        if(_size)
        {
            pointer storage = _storage;
            bool* allocated = _allocated;
            size_type first_valid_index = _first_valid_index;
            size_type last_valid_index = _last_valid_index;

            for(size_type index = first_valid_index; index <= last_valid_index; ++index)
            {
                if(allocated[index])
                {
                    storage[index].~value_type();
                }
            }

            size_type max_size = _max_size_minus_one + 1;
            memory::clear(max_size, *allocated);
            _first_valid_index = max_size;
            _last_valid_index = 0;
            _size = 0;
        }
    }

    [[nodiscard]] mapped_type& operator[](const key_type& key)
    {
        return operator()(hasher()(key), key);
    }

    [[nodiscard]] mapped_type& operator()(const key_type& key)
    {
        return operator()(hasher()(key), key);
    }

    [[nodiscard]] mapped_type& operator()(hash_type key_hash, const key_type& key)
    {
        iterator it = find_hash(key_hash, key);

        if(it == end())
        {
            it = insert_hash(key_hash, key, mapped_type());
            BTN_ASSERT(it != end(), "Insertion failed");
        }

        return it->second;
    }

    void swap(iunordered_map& other)
    {
        if(this != &other)
        {
            BTN_ASSERT(_max_size_minus_one == other._max_size_minus_one,
                       "Invalid max size: ", max_size(), " - ", other.max_size());

            pointer storage = _storage;
            pointer other_storage = other._storage;
            bool* allocated = _allocated;
            bool* other_allocated = other._allocated;
            size_type first_valid_index = min(_first_valid_index, other._first_valid_index);
            size_type last_valid_index = max(_last_valid_index, other._last_valid_index);

            for(size_type index = first_valid_index; index <= last_valid_index; ++index)
            {
                if(other_allocated[index])
                {
                    if(allocated[index])
                    {
                        btn::swap(storage[index], other_storage[index]);
                    }
                    else
                    {
                        ::new(storage + index) value_type(move(other_storage[index]));
                        other_storage[index].~value_type();
                        other_allocated[index] = false;
                        allocated[index] = true;
                    }
                }
                else
                {
                    if(allocated[index])
                    {
                        ::new(other_storage + index) value_type(move(storage[index]));
                        storage[index].~value_type();
                        allocated[index] = false;
                        other_allocated[index] = true;
                    }
                }
            }

            btn::swap(_size, other._size);
            btn::swap(_first_valid_index, other._first_valid_index);
            btn::swap(_last_valid_index, other._last_valid_index);
        }
    }

    friend void swap(iunordered_map& a, iunordered_map& b)
    {
        a.swap(b);
    }

    [[nodiscard]] friend bool operator==(const iunordered_map& a, const iunordered_map& b)
    {
        size_type first_valid_index = a._first_valid_index;
        size_type last_valid_index = a._last_valid_index;

        if(a._size != b._size || first_valid_index != b._first_valid_index || last_valid_index != b._last_valid_index)
        {
            return false;
        }

        const_pointer a_storage = a._storage;
        const_pointer b_storage = b._storage;
        const bool* a_allocated = a._allocated;
        const bool* b_allocated = b._allocated;

        for(size_type index = first_valid_index; index <= last_valid_index; ++index)
        {
            if(a_allocated[index] != b_allocated[index])
            {
                return false;
            }

            if(a_allocated[index] && a_storage[index] != b_storage[index])
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] bool friend operator!=(const iunordered_map& a, const iunordered_map& b)
    {
        return ! (a == b);
    }

    [[nodiscard]] friend bool operator<(const iunordered_map& a, const iunordered_map& b)
    {
        return lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
    }

    [[nodiscard]] friend bool operator>(const iunordered_map& a, const iunordered_map& b)
    {
        return b < a;
    }

    [[nodiscard]] friend bool operator<=(const iunordered_map& a, const iunordered_map& b)
    {
        return ! (a > b);
    }

    [[nodiscard]] friend bool operator>=(const iunordered_map& a, const iunordered_map& b)
    {
        return ! (a < b);
    }

protected:
    iunordered_map(reference storage, bool& allocated, size_type max_size) :
        _storage(&storage),
        _allocated(&allocated),
        _max_size_minus_one(max_size - 1),
        _first_valid_index(max_size)
    {
        BTN_ASSERT(power_of_two(max_size), "Max size is not power of two: ", max_size);
    }

private:
    pointer _storage;
    bool* _allocated;
    size_type _max_size_minus_one;
    size_type _first_valid_index;
    size_type _last_valid_index = 0;
    size_type _size = 0;

    void _assign(const iunordered_map& other)
    {
        const_pointer other_storage = other._storage;
        pointer storage = _storage;
        bool* allocated = _allocated;
        size_type first_valid_index = other._first_valid_index;
        size_type last_valid_index = other._last_valid_index;
        memory::copy(*other._allocated, other.max_size(), *allocated);

        for(size_type index = first_valid_index; index <= last_valid_index; ++index)
        {
            if(allocated[index])
            {
                ::new(storage + index) value_type(other_storage[index]);
            }
        }

        _first_valid_index = other._first_valid_index;
        _last_valid_index = other._last_valid_index;
        _size = other._size;
    }

    void _assign(iunordered_map&& other)
    {
        pointer other_storage = other._storage;
        pointer storage = _storage;
        bool* allocated = _allocated;
        size_type first_valid_index = other._first_valid_index;
        size_type last_valid_index = other._last_valid_index;
        int other_max_size = other.max_size();
        memory::copy(*other._allocated, other_max_size, *allocated);

        for(size_type index = first_valid_index; index <= last_valid_index; ++index)
        {
            if(allocated[index])
            {
                value_type& other_value = other_storage[index];
                ::new(storage + index) value_type(move(other_value));
                other_value.~value_type();
            }
        }

        _first_valid_index = other._first_valid_index;
        _last_valid_index = other._last_valid_index;
        _size = other._size;

        memory::clear(other_max_size, *other.allocated);
        other._first_valid_index = other_max_size;
        other._last_valid_index = 0;
        other._size = 0;
    }

    [[nodiscard]] size_type _index(hash_type key_hash) const
    {
        return key_hash & _max_size_minus_one;
    }
};


template<typename Key, typename Value, int MaxSize, typename KeyHash, typename KeyEqual>
class unordered_map : public iunordered_map<Key, Value, KeyHash, KeyEqual>
{
    static_assert(power_of_two(MaxSize));

public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = pair<const key_type, mapped_type>;
    using size_type = int;
    using hash_type = unsigned;
    using hasher = KeyHash;
    using key_equal = KeyEqual;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    unordered_map() :
        iunordered_map<Key, Value, KeyHash, KeyEqual>(
            *reinterpret_cast<pointer>(_storage_buffer), *_allocated_buffer, MaxSize)
    {
    }

    unordered_map(const unordered_map& other) :
        unordered_map()
    {
        this->_assign(other);
    }

    unordered_map(unordered_map&& other) noexcept :
        unordered_map()
    {
        this->_assign(move(other));
    }

    unordered_map(const iunordered_map<Key, Value, KeyHash, KeyEqual>& other) :
        unordered_map()
    {
        BTN_ASSERT(other.size() <= MaxSize, "Not enough space in map: ", MaxSize, " - ", other.size());

        this->_assign(other);
    }

    unordered_map(iunordered_map<Key, Value, KeyHash, KeyEqual>&& other) noexcept :
        unordered_map()
    {
        BTN_ASSERT(other.size() <= MaxSize, "Not enough space in map: ", MaxSize, " - ", other.size());

        this->_assign(move(other));
    }

    unordered_map& operator=(const unordered_map& other)
    {
        if(this != &other)
        {
            this->clear();
            this->_assign(other);
        }

        return *this;
    }

    unordered_map& operator=(unordered_map&& other) noexcept
    {
        if(this != &other)
        {
            this->clear();
            this->_assign(move(other));
        }

        return *this;
    }

    unordered_map& operator=(const iunordered_map<Key, Value, KeyHash, KeyEqual>& other)
    {
        if(this != &other)
        {
            BTN_ASSERT(other.size() <= MaxSize, "Not enough space in map: ", MaxSize, " - ", other.size());

            this->clear();
            this->_assign(other);
        }

        return *this;
    }

    unordered_map& operator=(iunordered_map<Key, Value, KeyHash, KeyEqual>&& other) noexcept
    {
        if(this != &other)
        {
            BTN_ASSERT(other.size() <= MaxSize, "Not enough space in map: ", MaxSize, " - ", other.size());

            this->clear();
            this->_assign(move(other));
        }

        return *this;
    }

    ~unordered_map()
    {
        this->clear();
    }

private:
    alignas(alignof(value_type)) char _storage_buffer[sizeof(value_type) * MaxSize];
    bool _allocated_buffer[MaxSize] = {};
};

}

#endif