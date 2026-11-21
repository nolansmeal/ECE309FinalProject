// include/util/min_heap.h
//
// YOU IMPLEMENT (this whole file; it is a template, so the code lives in the
// header). Spec §4.1.
//
// A binary MIN-heap stored in an array (a std::vector), plus top_k(), which
// uses a size-k min-heap to pick the k largest items in O(n log k).
//
// Rules for this file:
//   * Write sift-up and sift-down yourself.
//   * Do NOT use std::priority_queue, std::make_heap / push_heap / pop_heap /
//     sort_heap, std::sort / stable_sort / partial_sort / nth_element, or
//     std::set / multiset / map / multimap (full list: spec §7.3).
//   * std::vector is fine as the storage array; std::swap, std::reverse and
//     std::pair are fine too.
//
// `Less` is a "less-than" function object, exactly like the one std::sort
// takes: less(a, b) is true when a should come before b. The heap keeps the
// SMALLEST element (by less) at the top.

#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T, typename Less = std::less<T>>
class MinHeap {
public:
    // Constructor
    explicit MinHeap(Less less = Less()) : less_(std::move(less)) {}

    // Add a new element to the heap
    // Time complexity: O(log n)
    void push(T value) {
        // Add the element to the end of the vector
        data_.push_back(std::move(value));

        // Move the element upward if necessary
        sift_up(data_.size() - 1);
    }

    // Return the smallest element without removing it
    // Time complexity: O(1)
    const T& top() const {
        if(empty()) {
            throw std::out_of_range("MinHeap::top on empty heap");
        }

        return data_[0];
    }

    // Remove and return the smallest element
    // Time complexity: O(log n)
    T pop() {
        if(empty()) {
            throw std::out_of_range("MinHeap::pop on empty heap");
        }

        // Save the smallest element
        T result = std::move(data_[0]);

        // If there is only one element, remove it
        if(data_.size() == 1) {
            data_.pop_back();
            return result;
        }

        // Move the last element to the root
        data_[0] = std::move(data_.back());

        // Remove the last element
        data_.pop_back();

        // Restore the min-heap property
        sift_down(0);

        return result;
    }

    // Return the number of elements
    std::size_t size() const noexcept {
        return data_.size();
    }

    // Check if the heap is empty
    bool empty() const noexcept {
        return data_.empty();
    }

    // Remove all elements
    void clear() noexcept {
        data_.clear();
    }

private:
    // Move an element upward until the heap property is restored
    void sift_up(std::size_t i) {
        while(i > 0) {
            // Find the parent index
            std::size_t parent = (i - 1) / 2;

            // Stop if the child is not smaller than the parent
            if(!less_(data_[i], data_[parent])) {
                break;
            }

            // Swap the child and parent
            std::swap(data_[i], data_[parent]);

            // Continue from the parent's index
            i = parent;
        }
    }

    // Move an element downward until the heap property is restored
    void sift_down(std::size_t i) {
        std::size_t n = data_.size();

        while(true) {
            // Find the left and right child indices
            std::size_t left = 2 * i + 1;
            std::size_t right = 2 * i + 2;

            // Assume the current element is the smallest
            std::size_t smallest = i;

            // Check the left child
            if(left < n && less_(data_[left], data_[smallest])) {
                smallest = left;
            }

            // Check the right child
            if(right < n && less_(data_[right], data_[smallest])) {
                smallest = right;
            }

            // Stop if the current element is already smallest
            if(smallest == i) {
                break;
            }

            // Swap with the smaller child
            std::swap(data_[i], data_[smallest]);

            // Continue checking downward
            i = smallest;
        }
    }

    // Vector used to store heap elements
    std::vector<T> data_;

    // Comparator used to determine ordering
    Less less_;
};

// Return the k largest elements in descending order
// Time complexity: O(n log k) for 1 <= k <= n
// Extra space: O(k)
template <typename T, typename Less = std::less<T>>
std::vector<T> top_k(const std::vector<T>& items, std::size_t k, Less less = Less()) {
    // Return an empty vector if k is zero or there are no items
    if(k == 0 || items.empty()) {
        return {};
    }

    // Make sure k does not exceed the number of items
    if(k > items.size()) {
        k = items.size();
    }

    // Create a min-heap with the given comparator
    MinHeap<T, Less> heap(less);

    // Go through every element
    for(const T& item : items) {
        // Add elements until the heap contains k elements
        if(heap.size() < k) {
            heap.push(item);
        }
        // Replace the smallest element if the new element is larger
        else if(less(heap.top(), item)) {
            heap.pop();
            heap.push(item);
        }
    }

    // Create a vector to store the result
    std::vector<T> result;

    // Reserve enough space for the selected elements
    result.reserve(heap.size());

    // Remove elements from the heap
    while(!heap.empty()) {
        result.push_back(heap.pop());
    }

    // Reverse the vector so the largest elements come first
    std::reverse(result.begin(), result.end());

    return result;
}
