// src/memory/memory_index.cpp — YOU IMPLEMENT. Spec §4.3.


#include "memory/memory_index.h"
#include <stack>
#include <utility>

// Destructor
MemoryIndex::~MemoryIndex() {
    clear();
}

// Insert a record into the BST
bool MemoryIndex::insert(MemoryRecord rec) {
    // If the tree is empty, create the root
    if(root_ == nullptr) {
        root_ = new Node{rec, nullptr, nullptr};
        count_++;
        return true;
    }

    Node* current = root_;

    while(current != nullptr) {
        // Duplicate ID
        if(rec.id == current->rec.id) {
            return false;
        }

        // Smaller IDs go left
        if(rec.id < current->rec.id) {
            if(current->left == nullptr) {
                current->left = new Node{rec, nullptr, nullptr};
                count_++;
                return true;
            }

            current = current->left;
        }
        // Larger IDs go right
        else {
            if(current->right == nullptr) {
                current->right = new Node{rec, nullptr, nullptr};
                count_++;
                return true;
            }

            current = current->right;
        }
    }

    return false;
}

// Find a record by ID
const MemoryRecord* MemoryIndex::find(std::uint64_t id) const {
    Node* current = root_;

    while(current != nullptr) {
        if(id == current->rec.id) {
            return &current->rec;
        }

        if(id < current->rec.id) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    return nullptr;
}

// Remove a record from the BST
bool MemoryIndex::remove(std::uint64_t id) {
    Node* current = root_;
    Node* parent = nullptr;

    // Find the node and its parent
    while(current != nullptr && current->rec.id != id) {
        parent = current;

        if(id < current->rec.id) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    // ID not found
    if(current == nullptr) {
        return false;
    }

    // Case 1: Node has two children
    if(current->left != nullptr && current->right != nullptr) {
        Node* successorParent = current;
        Node* successor = current->right;

        // Find the smallest node in the right subtree
        while(successor->left != nullptr) {
            successorParent = successor;
            successor = successor->left;
        }

        // Copy the successor's record
        current->rec = successor->rec;

        // Remove the successor instead
        parent = successorParent;
        current = successor;
    }

    // Cases 2 and 3: Node has one child or no children
    Node* child = nullptr;

    if(current->left != nullptr) {
        child = current->left;
    } else {
        child = current->right;
    }

    // Removing the root
    if(parent == nullptr) {
        root_ = child;
    }
    // Removing a left child
    else if(parent->left == current) {
        parent->left = child;
    }
    // Removing a right child
    else {
        parent->right = child;
    }

    delete current;
    count_--;

    return true;
}

// Return the number of records
std::size_t MemoryIndex::size() const noexcept {
    return count_;
}

// Calculate the height of the BST
std::size_t MemoryIndex::height() const {
    if(root_ == nullptr) {
        return 0;
    }

    std::stack<std::pair<Node*, std::size_t>> nodes;
    nodes.push({root_, 1});

    std::size_t maxHeight = 0;

    while(!nodes.empty()) {
        Node* current = nodes.top().first;
        std::size_t currentHeight = nodes.top().second;
        nodes.pop();

        // Update the maximum height
        if(currentHeight > maxHeight) {
            maxHeight = currentHeight;
        }

        // Check the left subtree
        if(current->left != nullptr) {
            nodes.push({current->left, currentHeight + 1});
        }

        // Check the right subtree
        if(current->right != nullptr) {
            nodes.push({current->right, currentHeight + 1});
        }
    }

    return maxHeight;
}

// Visit records in ascending ID order
void MemoryIndex::for_each_in_order(
    const std::function<void(const MemoryRecord&)>& fn
) const {
    std::stack<Node*> nodes;
    Node* current = root_;

    while(current != nullptr || !nodes.empty()) {
        // Move to the leftmost node
        while(current != nullptr) {
            nodes.push(current);
            current = current->left;
        }

        // Visit the current node
        current = nodes.top();
        nodes.pop();

        fn(current->rec);

        // Move to the right subtree
        current = current->right;
    }
}

// Remove every record from the BST
void MemoryIndex::clear() {
    if(root_ == nullptr) {
        count_ = 0;
        return;
    }

    std::stack<Node*> nodes;
    nodes.push(root_);

    while(!nodes.empty()) {
        Node* current = nodes.top();
        nodes.pop();

        // Save the children before deleting the node
        if(current->left != nullptr) {
            nodes.push(current->left);
        }

        if(current->right != nullptr) {
            nodes.push(current->right);
        }

        delete current;
    }

    // Reset the tree
    root_ = nullptr;
    count_ = 0;
}
