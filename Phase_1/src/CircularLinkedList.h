/* Circular Linked List Header File
 * Ultima 2.0
 *
 * This header file shows the private and public resources of the
 * CircularLinkedList class and its supporting class, Node.
 *
 * As there are two classes in this file, their documentation is placed at their
 * definitions, rather than up here.
 *
 * Hunter Poole
 * 03-28-2026
 */

#pragma once

#include <iostream>

using namespace std;

/* template class Node
 *
 * Public:
 *
 * 1. T data.
 *    - Hold's the node's data.
 * 2. Node<T> *next.
 *    - Pointer to the next node.
 * 3. Node(T data).
 *    - Constructor, requiring T data as an argument.
 *    - Sets data as data and the next node as a nullptr.
 *    - As such, creates a node with no relation to other nodes.
 */
template <typename T> class Node {
public:
  T data;
  Node<T> *next;

  Node(T data) {
    this->data = data;
    this->next = nullptr;
  }
};

/* template class CircularLinkedList
 *
 * Private:
 *
 * 1. Node<T> *head.
 *    - Pointer to the head of the list.
 * 2. Node<T> *tail.
 *    - Pointer to the tail of the list.
 * 3. bool empty.
 *    - A bool for the empty status of the list.
 *
 * Public:
 *
 * 1. CircularLinkedList()
 *    - No-arg constructor.
 * 2. ~CircularLinkedList()
 *    - No-arg destructor.
 * 3. void insert_front(T value)
 *    - Insert a node with value of value in the front.
 * 4. void insert_at(T value, int position)
 *    - Insert a node with value of value in the specified position.
 * 5. void insert_end(T value)
 *    - Insert a node with value of value at the end.
 * 6. void remove_front()
 *    - Removes the front node.
 * 7. void remove_end()
 *    - Removes the end node.
 * 8. bool is_empty()
 *    - Returns the empty bool.
 * 9. void dump()
 *    - print method.
 *    - Dumps the current state of the CircularLinkedList.
 */
template <typename T> class CircularLinkedList {
private:
  Node<T> *head;
  Node<T> *tail;
  bool empty;

public:
  /* CircularLinkedList() {...}
   *
   * No-arg constructor.
   *
   * Sets:
   * Head to null.
   * Tail to null.
   * empty = true.
   */
  CircularLinkedList() {
    head = nullptr;
    tail = nullptr;
    empty = true;
  };

  /* ~CircularLinkedList() {...}
   *
   * No-arg destructor.
   *
   * Removes nodes from the front until the list is empty.
   */
  ~CircularLinkedList() {
    while (!empty) {
      remove_front();
    }
  }

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
    auto NewNode = make_unique<Node<T>>(value);

    if (empty) {
      head = NewNode;
      tail = NewNode;
      head->next = tail;
      tail->next = head;

      empty = false;
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
    if (position <= 1 || empty) {
      insert_front(value);
    } else {
      auto NewNode = make_unique<Node<T>>(value);
      auto temp = make_unique<Node<T>>();
      temp = head;

      for (int i = 1; i < position - 1 && temp->next != head; i++) {
        temp = temp->next;
      }

      NewNode->next = temp->next;
      temp->next = NewNode;
    }
  }

  /* void insert_end(T value) {...}
   *
   * Inserts a node at the end of the list.
   *
   * 1. Checks if the list is empty.
   *    - If so, call insert_front(value).
   * 2. Else...
   *    - Make a NewNode.
   *    - Set NewNode's next to be the head.
   *    - Set tail's next to be NewNode.
   *    - Set tail = NewNode.
   */
  void insert_end(T value) {
    if (empty) {
      insert_front(value);
    } else {
      auto NewNode = make_unique<Node<T>>(value);

      NewNode->next = head;
      tail->next = NewNode;
      tail = NewNode;
    }
  }

  /* void remove_front() {...}
   *
   * Removes a node from the front of the list.
   *
   * 1. Checks if the list is empty.
   *    - If so, do nothing.
   * 2. Else, if the list has only one node...
   *    - Dereference that node.
   *    - Set empty = true.
   * 3. Else,
   *    - Point tail to head's next.
   *    - Point head to head's next.
   */
  void remove_front() {
    if (empty) {
      return;
    } else if (head->next == tail) {
      head = nullptr;
      tail = nullptr;
      empty = true;
    } else {
      tail->next = head->next;
      head = head->next;
    }
  }

  /* void remove_end() {...}
   *
   * Removes a node from the end of the list.
   *
   * 1. Checks if the list is empty.
   *    - If so, do nothing.
   * 2. Else, if the list has only one node...
   *    - Dereference that node.
   *    - Set empty = true.
   * 3. Else,
   *    - Make two Node pointers.
   *    - Advance both. One to tail and one previous the tail.
   *    - Point the NewTail to the head.
   *    - Make the old tail's reference to the head null.
   *    - Set tail = NewTail.
   */
  void remove_end() {
    if (empty) {
      return;
    } else if (head->next == tail) {
      head = nullptr;
      tail = nullptr;
      empty = true;
    } else {
      auto temp = make_unique<Node<T>>(0);
      auto NewTail = make_unique<Node<T>>(0);
      temp = head;
      NewTail = head;

      while (temp->next != head) {
        NewTail = temp;
        temp = temp->next;
      }

      tail = NewTail;
      tail->next = head;
      temp->next = nullptr;
    }
  }

  /* bool is_empty() {...}
   *
   * Returns the empty bool.
   */
  bool is_empty() { return (empty); }

  /* void dump() {...}
   *
   * Prints the current status of the list.
   *
   * 1. Checks if the list is empty.
   *    - If so, print that.
   * 2. Else,
   *    - Iterates through the list.
   *    - Prints data from every node.
   *    - Stops when it hits the head again.
   */
  void dump() {
    if (empty) {
      cout << "List is empty" << endl;
    } else {
      auto temp = make_unique<Node<T>>(0);
      temp = head;

      do {
        cout << temp->data << " --> ";
        temp = temp->next;
      } while (temp != head);

      cout << head->data << endl;
    }
  }
};
