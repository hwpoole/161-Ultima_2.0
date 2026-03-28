/* Circular Linked List Header File
 * Ultima 2.0
 *
 * This header file shows the private and public resources of the
 * CircularLinkedList class.
 *
 * Of note to the public, who wish to use this class:
 *   1.
 *
 * Of note on the private resources:
 *   1.
 *
 * Hunter Poole
 * 03-28-2026
 */

#pragma once

using namespace std;

template <typename T> class Node {
public:
  T data;
  Node<T> *next;

  Node(T data) {
    this->data = data;
    this->next = nullptr;
  }
};

template <typename T> class CircularLinkedList {
private:
  Node<T> *head;

public:
  CircularLinkedList();

  void insert_front(T value);

  void insert_at(T value, int position);

  void insert_end(T value);

  void remove_first();

  void remove_end();

  void dump();
};
