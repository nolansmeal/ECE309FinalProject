// include/util/lru_cache.h
//
// YOU IMPLEMENT (this whole file; it is a template). Spec §4.2.
//
// A fixed-capacity cache that evicts the Least Recently Used entry.
// Both get() and put() must be O(1) (expected), so it is built from two
// structures that point at the same nodes:
//
//   std::unordered_map<Key, Node*>   finds a key's node in O(1)
//   a doubly linked list of Nodes    keeps recency order:
//        head_ = most recently used ... tail_ = least recently used
//
// Rules for this file:
//   * Write the doubly linked list yourself (Node with prev/next pointers,
//     new/delete inside this class only). Do NOT use std::list, std::deque
//     or std::forward_list (don't even #include their headers), or a
//     std::vector you shift around (spec §7.3).
//   * std::unordered_map is allowed (and expected) for the key -> Node* index.
//   * No leaks, no double frees: ASan runs on every test.

#pragma once
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class LruCache {
public:
    struct Stats {
        std::size_t hits = 0;        // get() found the key
        std::size_t misses = 0;      // get() did not find the key
        std::size_t evictions = 0;   // put() removed the LRU entry to make room
    };

    // A cache holding at most `capacity` entries. Capacity 0 is legal: such a
    // cache never stores anything (every get() is a miss).
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    // Frees every node.
    ~LruCache() { clear(); }

    // Copying a cache of raw-pointer nodes is not needed; forbid it.
    LruCache(const LruCache&) = delete;
    LruCache& operator=(const LruCache&) = delete;

    // Hit: marks the entry most recently used, counts a hit, and returns a
    //      pointer to its value (valid until the next put/erase/clear).
    // Miss: counts a miss and returns nullptr.
    const Value* get(const Key& key) {
        // Search for the key in the hash map
        auto it = map_.find(key);

        // If the key does not exist, count a miss
        if(it == map_.end()) {
            stats_.misses++;
            return nullptr;
        }

        // Count a successful lookup
        stats_.hits++;

        // Move the accessed node to the front
        Node* node = it->second;
        move_to_front(node);

        return &node->value;
    }

    // Key present: replaces its value and marks it most recently used (no
    //   eviction, no hit/miss counted).
    // Key absent: if the cache is full, evicts the least recently used entry
    //   (counting one eviction), then inserts the new entry as most recently
    //   used. With capacity 0, does nothing.
    void put(const Key& key, Value value) {
        // A cache with zero capacity cannot store anything
        if(capacity_ == 0) {
            return;
        }

        // Check whether the key already exists
        auto it = map_.find(key);

        if(it != map_.end()) {
            // Update the existing value
            Node* node = it->second;
            node->value = std::move(value);

            // Mark the updated node as most recently used
            move_to_front(node);
            return;
        }

        // Create a new node
        Node* newNode = new Node{key, std::move(value), nullptr, nullptr};

        // Store the key and node pointer in the hash map
        try {
            map_.emplace(newNode->key, newNode);
        }
        catch(...) {
            delete newNode;
            throw;
        }

        // If the cache is full, remove the least recently used node
        if(map_.size() > capacity_) {
            Node* victim = tail_;

            // Remove the node from the linked list
            unlink(victim);

            // Remove the key from the hash map before deleting the node
            map_.erase(victim->key);

            // Free the memory used by the node
            delete victim;

            // Count the eviction
            stats_.evictions++;
        }

        // Insert the new node at the front of the list
        push_front(newNode);
    }

    // True if the key is cached. Does NOT change recency or stats.
    bool contains(const Key& key) const {
        return map_.find(key) != map_.end();
    }

    // Removes the key if present (not an eviction). Returns whether it was.
    bool erase(const Key& key) {
        // Search for the key
        auto it = map_.find(key);

        if(it == map_.end()) {
            return false;
        }

        // Get the node associated with the key
        Node* node = it->second;

        // Remove the node from the linked list
        unlink(node);

        // Remove the key from the hash map
        map_.erase(it);

        // Free the node's memory
        delete node;

        return true;
    }

    // Removes every entry. Stats are kept.
    void clear() {
        Node* current = head_;

        // Walk through the linked list and delete each node
        while(current != nullptr) {
            // Save the next pointer before deleting the current node
            Node* next = current->next;

            delete current;

            // Continue to the next node
            current = next;
        }

        // Remove all entries from the hash map
        map_.clear();

        // Reset the linked list pointers
        head_ = nullptr;
        tail_ = nullptr;
    }

    std::size_t size() const noexcept { return map_.size(); }
    std::size_t capacity() const noexcept { return capacity_; }
    Stats       stats() const noexcept { return stats_; }

    // Keys from most to least recently used, by walking the list from head_.
    // Used by the tests to check your recency order.
    std::vector<Key> keys_mru_to_lru() const {
        std::vector<Key> out;
        for (Node* n = head_; n != nullptr; n = n->next) out.push_back(n->key);
        return out;
    }

private:
    struct Node {
        Key   key;
        Value value;
        Node* prev;
        Node* next;
    };

    // ===== YOUR HELPERS AND MEMBERS =====

    // Remove a node from the doubly linked list
    // Does not delete the node
    void unlink(Node* node) {
        // Connect the previous node to the next node
        if(node->prev != nullptr) {
            node->prev->next = node->next;
        }
        else {
            // The node was the head
            head_ = node->next;
        }

        // Connect the next node to the previous node
        if(node->next != nullptr) {
            node->next->prev = node->prev;
        }
        else {
            // The node was the tail
            tail_ = node->prev;
        }

        // Disconnect the node from the list
        node->prev = nullptr;
        node->next = nullptr;
    }

    // Insert a node at the front of the linked list
    void push_front(Node* node) {
        // The new node has no previous node
        node->prev = nullptr;

        // Point the new node to the current head
        node->next = head_;

        // Update the previous pointer of the old head
        if(head_ != nullptr) {
            head_->prev = node;
        }

        // Make the new node the head
        head_ = node;

        // If the list was empty, the new node is also the tail
        if(tail_ == nullptr) {
            tail_ = node;
        }
    }

    // Move an existing node to the front of the list
    void move_to_front(Node* node) {
        // The node is already the most recently used
        if(node == head_) {
            return;
        }

        // Remove the node from its current position
        unlink(node);

        // Insert it at the front
        push_front(node);
    }

    std::unordered_map<Key, Node*, Hash> map_;
    Node*       head_ = nullptr;
    Node*       tail_ = nullptr;
    std::size_t capacity_;
    Stats       stats_;
};