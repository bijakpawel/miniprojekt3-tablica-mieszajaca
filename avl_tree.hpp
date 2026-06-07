#pragma once

#include <algorithm>
#include <cstddef>

// Prosta implementacja drzewa AVL jako kubelek do tablicy mieszajacej.
template <typename Key, typename Value>
class AvlTree {
public:
    AvlTree() = default;

    // ochrona przed kopiowaniem.
    AvlTree(const AvlTree&) = delete;
    AvlTree& operator=(const AvlTree&) = delete;

    // Przenoszenie jest potrzebne, bo vector moze przenosic kubelki przy rehashu.
    AvlTree(AvlTree&& other) noexcept
        : root_(other.root_), size_(other.size_)
    {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    AvlTree& operator=(AvlTree&& other) noexcept
    {
        if (this != &other) {
            clear(root_);
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~AvlTree()
    {
        clear(root_);
    }

    bool insert_or_assign(const Key& key, const Value& value)
    {
        bool inserted = false;
        root_ = insert(root_, key, value, inserted);
        if (inserted) {
            ++size_;
        }
        return inserted;
    }

    bool erase(const Key& key)
    {
        bool removed = false;
        root_ = remove(root_, key, removed);
        if (removed) {
            --size_;
        }
        return removed;
    }

    bool contains(const Key& key) const
    {
        return find(root_, key) != nullptr;
    }

    std::size_t size() const
    {
        return size_;
    }

    template <typename Fn>
    void for_each(Fn&& fn) const
    {
        in_order(root_, fn);
    }

private:
    struct Node {
        Key key;
        Value value;
        int height;
        Node* left;
        Node* right;

        Node(const Key& k, const Value& v)
            : key(k), value(v), height(1), left(nullptr), right(nullptr)
        {
        }
    };

    Node* root_ = nullptr;
    std::size_t size_ = 0;

    static int height(Node* node)
    {
        return node == nullptr ? 0 : node->height;
    }

    static int balance_factor(Node* node)
    {
        return node == nullptr ? 0 : height(node->left) - height(node->right);
    }

    static void update_height(Node* node)
    {
        if (node != nullptr) {
            node->height = 1 + std::max(height(node->left), height(node->right));
        }
    }

    static Node* rotate_right(Node* node)
    {
        Node* pivot = node->left;
        Node* transferred_subtree = pivot->right;

        pivot->right = node;
        node->left = transferred_subtree;

        update_height(node);
        update_height(pivot);
        return pivot;
    }

    static Node* rotate_left(Node* node)
    {
        Node* pivot = node->right;
        Node* transferred_subtree = pivot->left;

        pivot->left = node;
        node->right = transferred_subtree;

        update_height(node);
        update_height(pivot);
        return pivot;
    }

    static Node* rebalance(Node* node)
    {
        update_height(node);
        int factor = balance_factor(node);

        // Lewe poddrzewo jest za wysokie.
        if (factor > 1) {
            if (balance_factor(node->left) < 0) {
                node->left = rotate_left(node->left);
            }
            return rotate_right(node);
        }

        // Prawe poddrzewo jest za wysokie.
        if (factor < -1) {
            if (balance_factor(node->right) > 0) {
                node->right = rotate_right(node->right);
            }
            return rotate_left(node);
        }

        return node;
    }

    static Node* insert(Node* node, const Key& key, const Value& value, bool& inserted)
    {
        if (node == nullptr) {
            inserted = true;
            return new Node(key, value);
        }

        if (key < node->key) {
            node->left = insert(node->left, key, value, inserted);
        } else if (key > node->key) {
            node->right = insert(node->right, key, value, inserted);
        } else {
            // Klucz juz istnieje, wiec tylko aktualizujemy wartosc.
            node->value = value;
            return node;
        }

        return rebalance(node);
    }

    static Node* remove(Node* node, const Key& key, bool& removed)
    {
        if (node == nullptr) {
            return nullptr;
        }

        if (key < node->key) {
            node->left = remove(node->left, key, removed);
        } else if (key > node->key) {
            node->right = remove(node->right, key, removed);
        } else {
            removed = true;

            // Przypadek 1: brak lewego dziecka.
            if (node->left == nullptr) {
                Node* right_child = node->right;
                delete node;
                return right_child;
            }

            // Przypadek 2: brak prawego dziecka.
            if (node->right == nullptr) {
                Node* left_child = node->left;
                delete node;
                return left_child;
            }

            // Przypadek 3: dwa dzieci.
            // Bierzemy nastepnik, czyli najmniejszy element z prawego poddrzewa.
            Node* successor = min_node(node->right);
            node->key = successor->key;
            node->value = successor->value;

            bool ignored = false;
            node->right = remove(node->right, successor->key, ignored);
        }

        return rebalance(node);
    }

    static Node* min_node(Node* node)
    {
        while (node->left != nullptr) {
            node = node->left;
        }
        return node;
    }

    static Node* find(Node* node, const Key& key)
    {
        while (node != nullptr) {
            if (key < node->key) {
                node = node->left;
            } else if (key > node->key) {
                node = node->right;
            } else {
                return node;
            }
        }
        return nullptr;
    }

    template <typename Fn>
    static void in_order(Node* node, Fn& fn)
    {
        if (node == nullptr) {
            return;
        }

        in_order(node->left, fn);
        fn(node->key, node->value);
        in_order(node->right, fn);
    }

    static void clear(Node* node)
    {
        if (node == nullptr) {
            return;
        }

        clear(node->left);
        clear(node->right);
        delete node;
    }
};
