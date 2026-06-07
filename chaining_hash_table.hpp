#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <vector>

struct Entry {
    int key;
    int value;
};

class ChainingHashTable {
public:
    explicit ChainingHashTable(std::size_t bucket_count = 17)
        : buckets_(bucket_count == 0 ? 1 : bucket_count)
    {
    }

    bool insert(int key, int value)
    {
        if (size_ + 1 > buckets_.size() * 2) {
            rehash(buckets_.size() * 2 + 1);
        }
        return place_entry(key, value);
    }

    bool remove(int key)
    {
        auto& bucket = buckets_[index_for(key, buckets_.size())];
        auto it = std::find_if(bucket.begin(), bucket.end(), [key](const Entry& entry) { return entry.key == key; });
        if (it == bucket.end()) {
            return false;
        }

        bucket.erase(it);
        --size_;
        return true;
    }

    bool contains(int key) const
    {
        const auto& bucket = buckets_[index_for(key, buckets_.size())];
        return std::any_of(bucket.begin(), bucket.end(), [key](const Entry& entry) { return entry.key == key; });
    }

    std::size_t size() const
    {
        return size_;
    }

private:
    std::vector<std::vector<Entry>> buckets_;
    std::size_t size_ = 0;

    static std::size_t index_for(int key, std::size_t bucket_count)
    {
        return std::hash<int>{}(key) % bucket_count;
    }

    bool place_entry(int key, int value)
    {
        auto& bucket = buckets_[index_for(key, buckets_.size())];
        for (auto& entry : bucket) {
            if (entry.key == key) {
                entry.value = value;
                return false;
            }
        }

        bucket.push_back(Entry{key, value});
        ++size_;
        return true;
    }

    void rehash(std::size_t new_bucket_count)
    {
        std::vector<std::vector<Entry>> fresh(new_bucket_count);
        for (const auto& bucket : buckets_) {
            for (const auto& entry : bucket) {
                fresh[index_for(entry.key, new_bucket_count)].push_back(entry);
            }
        }
        buckets_ = std::move(fresh);
    }
};
