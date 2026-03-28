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
   * 3. Else...
   *    - Point NewNode to head.
   *    - Point tail to NewNode.
   *    - Update head to NewNode.
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

  /* void insert_at(T value, int position) {...}
   *
   * Inserts a node at a specified position.
   *
   * 1. Checks if position is <= 1 or if CLL is empty.
   *    1a. If so, calls insert_front with the provided value.
   * 2. Else...
   *    - Creates NewNode with value.
   *    - Creates temp node at the head.
   *    - Iterates through the list until it finds the position.
   *    - Updates NewNode to point to temp's next node.
   *    - Updates temp to point to NewNode.
   */
  void insert_at(T value, int position) {
    if (position <= 1 || is_empty()) {
      insert_front(value);
    } else {
      Node<T> *NewNode = Node(value);
      Node<T> *temp = head;

      for (int i = 1; i < position - 1 && temp->next != head; i++) {
        temp = temp->next;
      }

      NewNode->next = temp->next;
      temp->next = NewNode;
    }
  }

  void insert_end(T value);

  void remove_front();

  void remove_end();

  bool is_empty();

  void dump();
};
