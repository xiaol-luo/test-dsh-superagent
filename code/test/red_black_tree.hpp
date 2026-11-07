#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

#include <cstdint>
#include <vector>

// A classic left-leaning-consistent (CLRS style) red-black tree supporting
// insert, remove, search, minimum/maximum and in-order traversal.
//
// Invariants maintained by rebalancing after insert/remove:
//   1. Every node is either red or black.
//   2. The root is black.
//   3. Every leaf (NIL) is black.
//   4. A red node has no red child.
//   5. Every root-to-leaf path contains the same number of black nodes.

template <typename T>
class RedBlackTree {
public:
    enum Color : uint8_t { RED, BLACK };

    struct Node {
        T key;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
        Color color = RED;

        explicit Node(const T& value) : key(value) {}
    };

    RedBlackTree() : nil_(new Node(T{})) {
        nil_->color = BLACK;
        root_ = nil_;
    }

    ~RedBlackTree() { destroy(root_); delete nil_; }

    RedBlackTree(const RedBlackTree&) = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;

    // Returns the node containing `key`, or nullptr when absent.
    Node* find(const T& key) const {
        Node* cur = root_;
        while (cur != nil_) {
            if (key == cur->key) return cur;
            cur = key < cur->key ? cur->left : cur->right;
        }
        return nullptr;
    }

    bool contains(const T& key) const { return find(key) != nullptr; }

    // Inserts `key`, ignoring duplicates. Returns true when inserted.
    bool insert(const T& key) {
        Node* parent = nil_;
        Node* cur = root_;
        while (cur != nil_) {
            parent = cur;
            if (key == cur->key) return false;  // duplicate, do nothing
            cur = key < cur->key ? cur->left : cur->right;
        }

        Node* node = new Node(key);
        node->parent = parent;
        if (parent == nil_) {
            root_ = node;
        } else if (key < parent->key) {
            parent->left = node;
        } else {
            parent->right = node;
        }

        node->left = nil_;
        node->right = nil_;
        node->color = RED;
        ++size_;
        insertFixup(node);
        nil_->parent = nullptr;
        return true;
    }

    // Removes `key` if present. Returns true when removed.
    bool remove(const T& key) {
        Node* node = find(key);
        if (node == nullptr) return false;
        removeNode(node);
        return true;
    }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    Node* minimum() const { return root_ == nil_ ? nullptr : minimumNode(root_); }
    Node* maximum() const { return root_ == nil_ ? nullptr : maximumNode(root_); }

    // In-order traversal: keys in ascending order.
    std::vector<T> inOrder() const {
        std::vector<T> out;
        out.reserve(size_);
        inOrder(root_, out);
        return out;
    }

private:
    Node* root_ = nullptr;
    Node* nil_ = nullptr;  // sentinel used for all leaves
    std::size_t size_ = 0;

    Node* minimumNode(Node* node) const {
        while (node->left != nil_) node = node->left;
        return node;
    }

    Node* maximumNode(Node* node) const {
        while (node->right != nil_) node = node->right;
        return node;
    }

    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != nil_) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    void rightRotate(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != nil_) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nil_) {
            root_ = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
        y->right = x;
        x->parent = y;
    }

    void insertFixup(Node* node) {
        while (node->parent->color == RED) {
            if (node->parent == node->parent->parent->left) {
                Node* uncle = node->parent->parent->right;
                if (uncle->color == RED) {
                    node->parent->color = BLACK;
                    uncle->color = BLACK;
                    node->parent->parent->color = RED;
                    node = node->parent->parent;
                } else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        leftRotate(node);
                    }
                    node->parent->color = BLACK;
                    node->parent->parent->color = RED;
                    rightRotate(node->parent->parent);
                }
            } else {  // mirror case
                Node* uncle = node->parent->parent->left;
                if (uncle->color == RED) {
                    node->parent->color = BLACK;
                    uncle->color = BLACK;
                    node->parent->parent->color = RED;
                    node = node->parent->parent;
                } else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        rightRotate(node);
                    }
                    node->parent->color = BLACK;
                    node->parent->parent->color = RED;
                    leftRotate(node->parent->parent);
                }
            }
        }
        root_->color = BLACK;
    }

    void transplant(Node* u, Node* v) {
        if (u->parent == nil_) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;  // safe even when v is the sentinel
    }

    void removeNode(Node* node) {
        Node* y = node;
        Node* x = nil_;
        Color yOriginal = y->color;

        if (node->left == nil_) {
            x = node->right;
            transplant(node, node->right);
        } else if (node->right == nil_) {
            x = node->left;
            transplant(node, node->left);
        } else {
            y = minimumNode(node->right);
            yOriginal = y->color;
            x = y->right;
            if (y->parent == node) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = node->right;
                y->right->parent = y;
            }
            transplant(node, y);
            y->left = node->left;
            y->left->parent = y;
            y->color = node->color;
        }
        delete node;
        --size_;

        if (yOriginal == BLACK) removeFixup(x);
    }

    void removeFixup(Node* node) {
        while (node != root_ && node->color == BLACK) {
            if (node == node->parent->left) {
                Node* sibling = node->parent->right;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    node->parent->color = RED;
                    leftRotate(node->parent);
                    sibling = node->parent->right;
                }
                if (sibling->left->color == BLACK && sibling->right->color == BLACK) {
                    sibling->color = RED;
                    node = node->parent;
                } else {
                    if (sibling->right->color == BLACK) {
                        sibling->left->color = BLACK;
                        sibling->color = RED;
                        rightRotate(sibling);
                        sibling = node->parent->right;
                    }
                    sibling->color = node->parent->color;
                    node->parent->color = BLACK;
                    sibling->right->color = BLACK;
                    leftRotate(node->parent);
                    node = root_;
                }
            } else {  // mirror case
                Node* sibling = node->parent->left;
                if (sibling->color == RED) {
                    sibling->color = BLACK;
                    node->parent->color = RED;
                    rightRotate(node->parent);
                    sibling = node->parent->left;
                }
                if (sibling->right->color == BLACK && sibling->left->color == BLACK) {
                    sibling->color = RED;
                    node = node->parent;
                } else {
                    if (sibling->left->color == BLACK) {
                        sibling->right->color = BLACK;
                        sibling->color = RED;
                        leftRotate(sibling);
                        sibling = node->parent->left;
                    }
                    sibling->color = node->parent->color;
                    node->parent->color = BLACK;
                    sibling->left->color = BLACK;
                    rightRotate(node->parent);
                    node = root_;
                }
            }
        }
        node->color = BLACK;
    }

    void inOrder(Node* node, std::vector<T>& out) const {
        if (node == nil_) return;
        inOrder(node->left, out);
        out.push_back(node->key);
        inOrder(node->right, out);
    }

    void destroy(Node* node) {
        if (node == nil_) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
};

#endif  // RED_BLACK_TREE_HPP
