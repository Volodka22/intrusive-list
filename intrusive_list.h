#pragma once

#include <iterator>
#include <type_traits>

namespace intrusive {
struct default_tag;

template <typename Type_, typename TAG_>
struct list;

namespace detail {

struct base_list_element {

  base_list_element();
  base_list_element(const base_list_element&);
  base_list_element(base_list_element&& other);
  base_list_element& operator=(base_list_element&& other);
  void unlink();
  ~base_list_element();

  template <typename T_, typename Tag_>
  friend struct intrusive::list;

private:
  base_list_element* prev{nullptr};
  base_list_element* next{nullptr};

  void make_loop();
  void copy_base(base_list_element&&);
  bool is_looping() const;
};

} // namespace detail

template <typename Tag = default_tag>
struct list_element : detail::base_list_element {};

template <typename T, typename Tag = default_tag>
struct list {

private:
  template <typename Type>
  struct list_iterator;

  using element = list_element<Tag>;

public:
  using iterator = list_iterator<T>;
  using const_iterator = list_iterator<T const>;

public:
  list() noexcept : fake_element() {};

  list(list<T, Tag>&& a) noexcept : fake_element(std::move(a.fake_element)) {}

  list(list const&) = delete;
  list& operator=(list const& a) = delete;

  list& operator=(list&& a) noexcept {
    clear();
    fake_element = std::move(a.fake_element);
    return *this;
  }

  void clear() noexcept {
    while (!empty()) {
      pop_front();
    }
  }

  void push_back(T& p) noexcept {
    insert(end(), p);
  }

  void push_front(T& p) noexcept {
    insert(begin(), p);
  }

  void pop_front() noexcept {
    erase(begin());
  }

  void pop_back() noexcept {
    erase(std::prev(end()));
  }

  T& front() noexcept {
    return *begin();
  }

  T const& front() const noexcept {
    return *begin();
  }

  T& back() noexcept {
    return *std::prev(end());
  }

  T const& back() const noexcept {
    return *std::prev(end());
  }

  bool empty() const noexcept {
    return fake_element.is_looping();
  }

  iterator end() noexcept {
    return iterator(&fake_element);
  }

  const_iterator end() const noexcept {
    return const_iterator(const_cast<element*>(&fake_element));
  }

  iterator begin() noexcept {
    return ++end();
  }

  const_iterator begin() const noexcept {
    return ++end();
  }

  iterator insert(const_iterator to, T& value) noexcept {
    auto e = &static_cast<element&>(value);
    if (e != to.data) {
      e->unlink();
      link(static_cast<element*>(to.data->prev), e);
      link(e, to.data);
    }
    return iterator(e);
  }

  iterator erase(const_iterator pos) {
    iterator ans = iterator(pos.data->next);
    pos.data->unlink();
    return ans;
  }

  void splice(const_iterator p, list&, const_iterator f, const_iterator e) {
    if (f != e) {
      iterator right = iterator(e.data->prev);
      link(static_cast<element*>(f.data->prev), e.data);
      link(static_cast<element*>(p.data->prev), f.data);
      link(right.data, p.data);
    }
  }

private:
  element fake_element;

  void link(element* a, element* b) {
    a->next = b;
    b->prev = a;
  }

  template <typename Type>
  struct list_iterator {

    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Type;
    using pointer = Type*;
    using reference = Type&;

    list_iterator() = default;

    template <typename Other,
              typename = std::enable_if_t<std::is_const_v<value_type> &&
                                          !std::is_const_v<Other>>>
    list_iterator(list_iterator<Other> const& other) : data(other.data) {}

    reference operator*() const {
      return static_cast<reference>(*data);
    }

    list_iterator operator++() {
      return *this = list_iterator(data->next);
    }

    list_iterator operator++(int) {
      list_iterator tmp(*this);
      ++*this;
      return tmp;
    }

    list_iterator operator--() {
      return *this = list_iterator(data->prev);
    }

    list_iterator operator--(int) {
      list_iterator tmp(*this);
      --(*this);
      return tmp;
    }

    pointer operator->() const {
      return static_cast<pointer>(data);
    }

    friend bool operator==(list_iterator const& a, list_iterator const& b) {
      return a.data == b.data;
    }

    friend bool operator!=(list_iterator const& a, list_iterator const& b) {
      return !(a == b);
    }

    template <typename T_, typename Tag_>
    friend struct list;

  private:
    element* data;

    explicit list_iterator(element* a) : data(a) {}
    explicit list_iterator(detail::base_list_element* a)
        : data(static_cast<element*>(a)) {}
  };
};

} // namespace intrusive
