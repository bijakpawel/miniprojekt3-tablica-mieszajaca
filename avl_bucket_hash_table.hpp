#pragma once

#include "avl_tree.hpp"

#include <cstddef>
#include <functional>
#include <vector>

class AvlBucketHashTable {
public:
    explicit AvlBucketHashTable(std::size_t bucket_count = 17)
        : buckets_(bucket_count == 0 ? 1 : bucket_count)
    {
    }

    bool insert(int key, int value)
    {
        if (size_ + 1 > buckets_.size() * 3 / 2) {
            rehash(buckets_.size() * 2 + 1);
        }
        return place_entry(key, value);
    }

    bool remove(int key)
    {
        auto& bucket = buckets_[index_for(key, buckets_.size())];
        const bool removed = bucket.erase(key);
        if (removed) {
            --size_;
        }
        return removed;
    }

    bool contains(int key) const
    {
        const auto& bucket = buckets_[index_for(key, buckets_.size())];
        return bucket.contains(key);
    }

    std::size_t size() const
    {
        return size_;
    }

private:
    std::vector<AvlTree<int, int>> buckets_;
    std::size_t size_ = 0;

    static std::size_t index_for(int key, std::size_t bucket_count)
    {
        return std::hash<int>{}(key) % bucket_count;
    }

    bool place_entry(int key, int value)
    {
        auto& bucket = buckets_[index_for(key, buckets_.size())];
        const bool inserted = bucket.insert_or_assign(key, value);
        if (inserted) {
            ++size_;
        }
        return inserted;
    }

    void rehash(std::size_t new_bucket_count)
    {
        std::vector<AvlTree<int, int>> fresh(new_bucket_count);

        for (const auto& bucket : buckets_) {
            bucket.for_each([&](const int& key, const int& value) {
                fresh[index_for(key, new_bucket_count)].insert_or_assign(key, value);
            });
        }

        buckets_ = std::move(fresh);
    }
};
