#ifndef BINDER_H
#define BINDER_H

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

namespace cxx {
template <typename K, typename V>
class binder {
   private:
    struct data_t {
        using notes_t = std::list<std::pair<const K*, V>>;
        using index_t = std::map<K, typename notes_t::iterator>;
        notes_t notes{};
        index_t index{};

        data_t() = default;

        data_t(const data_t &other) : notes(other.notes) {
            try {
                for (auto it = notes.begin(); it != notes.end(); ++it) {
                    it->first = &index.emplace(*it->first, it).first->first;
                }
            } catch (...) {
                notes.clear();
                index.clear();
                throw;
            }
        }
    }; // struct data_t

    std::shared_ptr<data_t> data = nullptr;
    bool unsharable = false;

    // Creates a copy of the data if it is shared.
    // Returns a pointer to this copy or to the original data if it was not shared.
    std::shared_ptr<data_t> going_to_edit() {
        if (data.use_count() > 1)
            return std::make_shared<data_t>(*data);
        else
            return data;
    }

    // Overwrites the original data with new data.
    void successful_edit(std::shared_ptr<data_t>&& new_data) noexcept {
        data = std::move(new_data);
        unsharable = false;
    }

   public:
    binder() = default;

    binder(const binder &other) {
        if (other.unsharable && other.data) {
            data = std::make_shared<data_t>(*other.data);
        } else {
            data = other.data;
        }
    }

    binder(binder &&other) noexcept : data(std::move(other.data)), unsharable(other.unsharable) {}

    binder &operator=(binder other) noexcept {
        data.swap(other.data); // copy & swap idiom
        unsharable = false;
        return *this;
    }

    // Inserts a note v with key k at the beginning of the binder.
    // If a note with key k already exists, it throws std::invalid_argument.
    void insert_front(const K &k, const V &v) {
        if (data && data->index.contains(k))
            throw std::invalid_argument("Key already exists");

        if (!data)
            data = std::make_shared<data_t>();

        std::shared_ptr<data_t> new_data = going_to_edit();

        new_data->notes.emplace_front(nullptr, v);
        try {
            new_data->notes.begin()->first = &new_data->index.emplace(k, new_data->notes.begin()).first->first;
        } catch (...) {
            new_data->notes.pop_front();
            throw;
        }

        successful_edit(std::move(new_data));
    }

    // Inserts a note v with key k after the note with key prev_k.
    // If a note with key k already exists or a note with key prev_k does not exist, it throws std::invalid_argument.
    void insert_after(const K &prev_k, const K &k, const V &v) {
        if (!data || !data->index.contains(prev_k))
            throw std::invalid_argument("Prev_key not found");
        if (data->index.contains(k))
            throw std::invalid_argument("Key already exists");

        std::shared_ptr<data_t> new_data = going_to_edit();

        auto prev_it = new_data->index.find(prev_k);
        auto it = new_data->notes.insert(std::next(prev_it->second), std::make_pair(nullptr, v));

        try {
            it->first = &new_data->index.emplace(k, it).first->first;
        } catch (...) {
            new_data->notes.erase(it);
            throw;
        }

        successful_edit(std::move(new_data));
    }

    // Removes the first note from the binder.
    // If the binder is empty, it throws std::invalid_argument.
    void remove() {
        if (size() == 0)
            throw std::invalid_argument("Empty binder");
        remove(*data->notes.front().first);
    }

    // Removes the note with key k from the binder.
    // If a note with key k does not exist, it throws std::invalid_argument.
    void remove(const K &k) {
        if (!data || !data->index.contains(k))
            throw std::invalid_argument("Key not found");

        std::shared_ptr<data_t> new_data = going_to_edit();
        auto it_index = new_data->index.find(k);
        auto it_notes = it_index->second;
        new_data->index.erase(it_index);
        new_data->notes.erase(it_notes);

        successful_edit(std::move(new_data));
    }

    // Returns a reference to the note with key k.
    // If a note with key k does not exist, it throws std::invalid_argument.
    V &read(const K &k) {
        if (!data || !data->index.contains(k))
            throw std::invalid_argument("Key not found");

        std::shared_ptr<data_t> new_data = going_to_edit();
        auto it_index = new_data->index.find(k);

        successful_edit(std::move(new_data));
        unsharable = true; // we are returning a reference, so we cannot share the data

        return it_index->second->second;
    }

    // Returns a const reference to the note with key k.
    // If a note with key k does not exist, it throws std::invalid_argument.
    const V &read(const K &k) const {
        if (!data || !data->index.contains(k))
            throw std::invalid_argument("Key not found");

        auto it_index = data->index.find(k);

        return it_index->second->second;
    }

    std::size_t size() const noexcept {
        if (!data)
            return 0;
        return data->notes.size();
    }

    void clear() noexcept {
        data = nullptr;
        unsharable = false;
    }

    class const_iterator {
       private:
        typename data_t::notes_t::const_iterator it;
        bool null_data = false;

       public:
        using value_type = V;
        using reference = const V &;
        using pointer = const V *;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        // Iterator over an empty binder.
        explicit const_iterator() noexcept : null_data(true) {}

        // Binder iterator from a note list iterator.
        explicit const_iterator(typename data_t::notes_t::const_iterator it) noexcept : it(it) {}

        bool operator==(const const_iterator &other) const noexcept {
            if (null_data || other.null_data)
                return null_data == other.null_data;
            return it == other.it;
        }

        bool operator!=(const const_iterator &other) const noexcept {
            return !(*this == other);
        }

        // pre-increment
        const_iterator &operator++() noexcept {
            ++it;
            return *this;
        }

        // post-increment
        const_iterator operator++(int) noexcept {
            const_iterator tmp = *this;
            ++it;
            return tmp;
        }

        reference operator*() const noexcept {
            return it->second;
        }

        pointer operator->() const noexcept {
            return &it->second;
        }
    }; // class const_iterator

    // Returns an iterator to the beginning of the binder.
    const_iterator cbegin() const noexcept {
        if (!data)
            return const_iterator();
        return const_iterator(data->notes.cbegin());
    }

    // Returns an iterator to the end of the binder.
    const_iterator cend() const noexcept {
        if (!data)
            return const_iterator();
        return const_iterator(data->notes.cend());
    }
}; // class binder
} // namespace cxx

#endif //BINDER_H
