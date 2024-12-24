#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>

template <class T, class Allocator = std::allocator<T>>
    requires(std::is_nothrow_move_constructible_v<T> &&
             std::is_nothrow_destructible_v<T>)
class ConcurrentVector : private Allocator {
  public:
    using AllocatorTrits = std::allocator_traits<Allocator>;
    using size_type      = AllocatorTrits::size_type;
    using pointer        = AllocatorTrits::pointer;
    using const_pointer  = AllocatorTrits::const_pointer;

  private:
    pointer data                    = nullptr;
    std::atomic<size_type> capacity = 0;
    std::atomic<size_type> sz       = 0;
    std::atomic<size_type> position = 0;

    Allocator& get_allocator() {
        return *static_cast<Allocator*>(this);
    }

    void realloc() {
        size_type cur_size = sz.load();
        while (cur_size < capacity) {
            sz.wait(cur_size);
            cur_size = sz.load();
        }

        pointer old_data       = data;
        size_type new_capacity = capacity + std::max<size_type>(capacity >> 1, 1);
        data     = AllocatorTrits::allocate(get_allocator(), new_capacity);
        capacity = new_capacity;
        capacity.notify_all();

        for (size_type i = 0; i < cur_size; ++i) {
            AllocatorTrits::construct(get_allocator(), data + i,
                                      std::move(old_data[i]));
            AllocatorTrits::destroy(get_allocator(), old_data + i);
        }
        AllocatorTrits::deallocate(get_allocator(), old_data, cur_size);
    }

  public:
    ConcurrentVector()                                   = default;
    ConcurrentVector(const ConcurrentVector&)            = delete;
    ConcurrentVector& operator=(const ConcurrentVector&) = delete;

    ~ConcurrentVector() noexcept {
        for (size_type i = 0; i < sz; ++i) {
            AllocatorTrits::destroy(get_allocator(), data + i);
        }
        AllocatorTrits::deallocate(get_allocator(), data, capacity);
    }

    template <class... Args>
        requires(std::is_nothrow_constructible_v<T, Args && ...>)
    void emplace_back(Args&&... args) {
        size_type cur_position = position.fetch_add(1);
        size_type cur_capacity = capacity.load();

        while (cur_position >= cur_capacity) {
            if (cur_position == cur_capacity) {
                realloc();
            } else {
                capacity.wait(cur_capacity);
            }
            cur_capacity = capacity.load();
        }

        AllocatorTrits::construct(get_allocator(), data + cur_position,
                                  std::forward<Args>(args)...);

        sz.fetch_add(1);
        sz.notify_one();
    }

    size_type size() const {
        return sz.load();
    }

    const_pointer begin() const {
        return data;
    }

    const_pointer end() const {
        return data + size();
    }

    void clear() {
        for (size_type i = 0; i < size(); ++i) {
            AllocatorTrits::destroy(get_allocator(), data + i);
        }
        sz.store(0);
        position.store(0);
    }
};
