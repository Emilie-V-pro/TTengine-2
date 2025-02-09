#include <array>
#include <iostream>

namespace TTe {



template <typename T, int size>
class CircularQueue {
   public:
    CircularQueue() : front(0), rear(0), actual_size(0) {}

    bool push(const T& value) {
        if (actual_size == size) {
            front = (front + 1) % size;  // Écrase l'élément le plus ancien
        } else {
            actual_size++;
        }
        data[rear] = value;
        rear = (rear + 1) % size;
        return true;
    }

    void pop() {
        if (!empty()) {
            front = (front + 1) % size;
            actual_size--;
        }
    }

    T& peek() const {
        if (empty()) {
            throw std::out_of_range("Queue is empty");
        }
        return data[front];
    }

    T& back()  {
        if (empty()) {
            throw std::out_of_range("Queue is empty");
        }
        // L'élément le plus récent est à (rear - 1), en prenant en compte le cas circulaire
        size_t lastIndex = (rear == 0) ? size - 1 : rear - 1;
        return data[lastIndex];
    }

    bool empty() const { return actual_size == 0; }

    bool full() const { return actual_size == size; }

    size_t getSize() const { return actual_size; }

    size_t getCapacity() const { return size; }

   private:
    std::array<T, size> data;
    size_t front, rear, actual_size;
};
}