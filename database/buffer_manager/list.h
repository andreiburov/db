#pragma once
#include "page_size.h"
#include <cstdlib>
#include <fstream>

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
#ifdef DEBUG
  std::ofstream file;
#endif

  public:

  List() {
#ifdef DEBUG
    file.open("list.txt", std::ios::trunc);
#endif
    head_ = NULL;
    tail_ = NULL;
    end_ = new Node(NULL, NULL);
#ifdef DEBUG
    file << "allocate " << end_ << std::endl;
#endif
    size_ = 0;
  }

  ~List() {
    for (auto it = begin(); it != end(); ++it) {
#ifdef DEBUG
      file << "delete " << it.ptr << std::endl;
#endif
      erase(it);
    }
#ifdef DEBUG
    file.close();
#endif
  }

	iterator begin() { return iterator(head_); }
	iterator end() { return iterator(end_); }

  void push_back(T data);
  void erase(iterator it);
  bool empty() { return size_ == 0; }
};

  





    


