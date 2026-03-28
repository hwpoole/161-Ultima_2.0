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
  Node<T> *tail;

public:
  CircularLinkedList();

  /* void insert_front(T value) {...}
   *
   * Inserts a node at the front of the CLL.
   *
   * 1. Creates a pointer to NewNode with value of T value.
   * 2. Checks if the list is empty.
   *    2a. If so, head & tail = NewNode.
   *    3a. Else...
   *        - Point NewNode to head.
   *        - Point tail to NewNode.
   *        - Update head to NewNode.
   */
  void insert_front(T value) {
    Node<T> *NewNode = Node(value);

    if (is_empty()) {
      head = NewNode;
      tail = NewNode;
      head->next = tail;
      tail->next = head;
    } else {
      NewNode->next = head;
      tail->next = NewNode;
      head = NewNode;
    }
  }

  void insert_at(T value, int position);

  void insert_end(T value);

  void remove_first();

  void remove_end();

  bool is_empty();

  void dump();
};
