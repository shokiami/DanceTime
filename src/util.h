#ifndef UTIL_H_
#define UTIL_H_

#include "defs.h"

template <typename K, typename V>
class Map {
  public:
  V &operator[](K key) {
    return map[key];
  }

  vector<K> keys() {
    vector<K> key_vec;
    for (pair<K, V> kv_pair : map) {
      key_vec.push_back(kv_pair.first);
    }
    return key_vec;
  }

  bool contains(K key) {
    return map.find(key) != map.end();
  }

  bool empty() {
    return map.size() == 0;
  }

  int size() {
    return map.size();
  }

  private:
  unordered_map<K, V> map;
};

template <typename T>
class CyclicQueue {
  public:
  CyclicQueue(int max_size) : max_size(max_size), front(0), back(0) {
    arr = new T[max_size];
  }

  ~CyclicQueue() {
    delete[] arr;
  }

  void enqueue(T input[], int n) {
    for (int i = 0; i < n; i++) {
      arr[(back + i) % max_size] = input[i];
    }
    back = (back + n) % max_size;
  }

  void dequeue(T output[], int n) {
    for (int i = 0; i < n; i++) {
      output[i] = arr[(front + i) % max_size];
    }
    front = (front + n) % max_size;
  }

  int size() {
    return (back - front + max_size) % max_size;
  }

  int maxSize() {
    return max_size;
  }

  private:
  T* arr;
  int max_size;
  int front;
  int back;
};

#endif
