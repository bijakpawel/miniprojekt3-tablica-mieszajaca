#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>

template <typename Key, typename Value>
class AvlTree {
public:
    AvlTree() = default;
    AvlTree(const AvlTree&) = delete;
    AvlTree& operator=(const AvlTree&) = delete;
    AvlTree(AvlTree&&) noexcept = default;
    AvlTree& operator=(AvlTree&&) noexcept = default;

    bool insert_or_assign(const Key& key, const Value& value)
    {
        bool inserted = false;
        root_ = insert_node(std::move(root_), key, value, inserted);
        if (inserted) {
            ++size_;
        }
        return inserted;
    }

    bool erase(const Key& key)
    {
        bool removed = false;
        root_ = erase_node(std::move(root_), key, removed);
        if (removed) {
            --size_;
        }
        return removed;
    }

    bool contains(const Key& key) const
    {
        return find_node(root_.get(), key) != nullptr;
    }

    std::size_t size() const
    {
        return size_;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        traverse(root_.get(), fn);
    }

private:
    struct Node {
        Node(const Key& key_value, const Value& value_value)
            : key(key_value), value(value_value)
        {
        }

        Key key;
        Value value;
        int height = 1;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;

    static int height(const std::unique_ptr<Node>& node)
    {
        return node ? node->height : 0;
    }

    static void update_height(Node* node)
    {
        node->height = 1 + std::max(height(node->left), height(node->right));
    }

    static int balance_factor(const std::unique_ptr<Node>& node)
    {
        if (!node) {
            return 0;
        }
        return height(node->left) - height(node->right);
    }

    static std::unique_ptr<Node> rotate_right(std::unique_ptr<Node> node)
    {
        std::unique_ptr<Node> pivot = std::move(node->left);
        std::unique_ptr<Node> transfer = std::move(pivot->right);

        pivot->right = std::move(node);
        pivot->right->left = std::move(transfer);

        update_height(pivot->right.get());
        update_height(pivot.get());
        return pivot;
    }

    static std::unique_ptr<Node> rotate_left(std::unique_ptr<Node> node)
    {
        std::unique_ptr<Node> pivot = std::move(node->right);
        std::unique_ptr<Node> transfer = std::move(pivot->left);

        pivot->left = std::move(node);
        pivot->left->right = std::move(transfer);

        update_height(pivot->left.get());
        update_height(pivot.get());
        return pivot;
    }

    static std::unique_ptr<Node> rebalance(std::unique_ptr<Node> node)
    {
        update_height(node.get());
        const int factor = balance_factor(node);

        if (factor > 1) {
            if (balance_factor(node->left) < 0) {
                node->left = rotate_left(std::move(node->left));
            }
            return rotate_right(std::move(node));
        }

        if (factor < -1) {
            if (balance_factor(node->right) > 0) {
                node->right = rotate_right(std::move(node->right));
            }
            return rotate_left(std::move(node));
        }

        return node;
    }

    static std::unique_ptr<Node> insert_node(std::unique_ptr<Node> node, const Key& key, const Value& value, bool& inserted)
    {
        if (!node) {
            inserted = true;
            return std::make_unique<Node>(key, value);
        }

        if (key < node->key) {
            node->left = insert_node(std::move(node->left), key, value, inserted);
        } else if (key > node->key) {
            node->right = insert_node(std::move(node->right), key, value, inserted);
        } else {
            node->value = value;
            return node;
        }

        return rebalance(std::move(node));
    }

    static const Node* find_node(const Node* node, const Key& key)
    {
        while (node) {
            if (key < node->key) {
                node = node->left.get();
            } else if (key > node->key) {
                node = node->right.get();
            } else {
                return node;
            }
        }
        return nullptr;
    }

    static Node* min_node(Node* node)
    {
        while (node->left) {
            node = node->left.get();
        }
        return node;
    }

    static std::unique_ptr<Node> erase_node(std::unique_ptr<Node> node, const Key& key, bool& removed)
    {
        if (!node) {
            return nullptr;
        }

        if (key < node->key) {
            node->left = erase_node(std::move(node->left), key, removed);
        } else if (key > node->key) {
            node->right = erase_node(std::move(node->right), key, removed);
        } else {
            removed = true;
            if (!node->left) {
                return std::move(node->right);
            }
            if (!node->right) {
                return std::move(node->left);
            }

            Node* successor = min_node(node->right.get());
            node->key = successor->key;
            node->value = successor->value;

            bool ignored = false;
            node->right = erase_node(std::move(node->right), successor->key, ignored);
        }

        return rebalance(std::move(node));
    }

    template <typename Fn>
    static void traverse(const Node* node, Fn& fn)
    {
        if (!node) {
            return;
        }

        traverse(node->left.get(), fn);
        fn(node->key, node->value);
        traverse(node->right.get(), fn);
    }
};

#endif
