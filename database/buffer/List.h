#ifndef DB_LIST_H
#define DB_LIST_H

#include <cstdlib>
#include <iostream>

template<typename T>
class List {

  public:

  struct Node {
    T data;
    Node* prev;
    Node* next;

    Node() : prev(NULL), next(NULL) {}
    Node(Node* prev, Node* next) : prev(prev), next(next) {}
    Node(T data, Node* prev, Node* next) : data(data), prev(prev), next(next) {}
  };

  struct iterator {
    Node* ptr;
    
    iterator(Node* ptr) : ptr(ptr) {}
    iterator operator++(int) { iterator i = *this; ptr = ptr->next; return i; } 
    iterator& operator++() { ptr = ptr->next; return *this; } 
    T& operator*() { return ptr->data; }
    bool operator==(const iterator& rhs) { return ptr == rhs.ptr; }
    bool operator!=(const iterator& rhs) { return ptr != rhs.ptr; }
  };

  private:

  Node* head_;
  Node* tail_;
  Node* end_;
  int size_;

  public:

  List() {
    head_ = NULL;
    tail_ = NULL;
    end_ = new Node(NULL, NULL);
    size_ = 0;
  }

  ~List() {
    for (auto it = begin(); it != end(); ++it) {
      erase(it);
    }
  }

	iterator begin() { return iterator(head_); }
	iterator end() { return iterator(end_); }

  void push_back(T data) {
      if (head_ == NULL) {
          Node* n = new Node(data, NULL, end_);
          head_ = n;
          tail_ = n;
          size_++;
      } else {
          Node* n = new Node(data, tail_, end_);
          tail_->next = n;
          tail_ = n;
          size_++;
      }
  }

  void erase(iterator it) {
      if (it.ptr == end_) {
          std::cerr << "error: trying to delete the end" << std::endl;
          return;
      }

      if (size_ == 0) {
          std::cerr << "error: trying to delete from empty list" << std::endl;
          return;
      }

      if (head_ == tail_) {
          delete head_;
          head_ = NULL;
          tail_ = NULL;
          size_ = 0;
          return;
      }

      if (it.ptr == head_) {
          Node* n = head_->next;
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
      delete it.ptr;
  }

  bool empty() { return size_ == 0; }
};

#endif //DB_LIST_H