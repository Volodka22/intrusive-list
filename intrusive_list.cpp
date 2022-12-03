#include "intrusive_list.h"

namespace intrusive::detail {

base_list_element::base_list_element() {
  make_loop();
}

base_list_element::base_list_element(const base_list_element&)
    : base_list_element(){}

base_list_element::base_list_element(base_list_element&& other) {
  make_loop();
  copy_base(std::move(other));
}

void base_list_element::unlink() {
  prev->next = next;
  next->prev = prev;
  make_loop();
}

base_list_element::~base_list_element() {
  unlink();
}

base_list_element& base_list_element::operator=(base_list_element&& other) {
  unlink();
  copy_base(std::move(other));
  return *this;
}

void base_list_element::make_loop() {
  prev = next = this;
}

void base_list_element::copy_base(base_list_element&& other) {
  if (!other.is_looping()) {
    next = other.next;
    prev = other.prev;
    prev->next = this;
    next->prev = this;
    other.make_loop();
  } else {
    make_loop();
  }
}

bool base_list_element::is_looping() const {
  return next == this;
}

} // namespace intrusive::detail