#include "list.h"
#include <iostream>
#include <stdint.h>

template class List<uint64_t>;
template class List<int>;

template<typename T>
void List<T>::push_back(T data) {
  if (head_ == NULL) {
    Node* n = new Node(data, NULL, end_);
#ifdef DEBUG
    file << "allocate " << n << std::endl;
#endif
    head_ = n;
    tail_ = n;
    size_++;
  } else {
    Node* n = new Node(data, tail_, end_);
#ifdef DEBUG
    file << "allocate " << n << std::endl;
#endif
    tail_->next = n;
    tail_ = n;
    size_++;
  }
}

template<typename T>
void List<T>::erase(iterator it) {
  if (it.ptr == end_) {
    std::cerr << "error: trying to delete the end" << std::endl;
    return;
  }

  if (size_ == 0) {
    std::cerr << "error: trying to delete from empty list" << std::endl;
    return;
  }

  if (head_ == tail_) {
#ifdef DEBUG
    file << "delete " << head_ << std::endl;
#endif
    delete head_;
    head_ = NULL;
    tail_ = NULL;
    size_ = 0;
    return;
  }
  
  if (it.ptr == head_) {
    Node* n = head_->next;
#ifdef DEBUG
    file << "delete " << head_ << std::endl;
#endif
    delete head_;
    n->prev = NULL;
    head_ = n;
    size_--;
    return;
  }

  Node* p = it.ptr->prev;
  Node* n = it.ptr->next;
  p->next = n;
  n->prev = p;
  size_--;
#ifdef DEBUG
  file << "delete " << it.ptr << std::endl;
#endif
  delete it.ptr;
}
