#ifndef HASH_TABLES_HPP
#define HASH_TABLES_HPP

#include "avl_tree.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <utility>
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

class LinearProbingHashTable {
public:
    explicit LinearProbingHashTable(std::size_t capacity = 32)
        : slots_(capacity == 0 ? 1 : capacity)
    {
    }

    bool insert(int key, int value)
    {
        if ((size_ + 1) * 10 >= slots_.size() * 7) {
            rehash(slots_.size() * 2 + 1);
        }
        return place_entry(key, value);
    }

    bool remove(int key)
    {
        std::size_t index = index_for(key, slots_.size());
        for (std::size_t probe = 0; probe < slots_.size(); ++probe) {
            Slot& slot = slots_[index];
            if (slot.state == SlotState::Empty) {
                return false;
            }

            if (slot.state == SlotState::Occupied && slot.key == key) {
                slot.state = SlotState::Deleted;
                --size_;
                return true;
            }

            index = (index + 1) % slots_.size();
        }

        return false;
    }

    bool contains(int key) const
    {
        std::size_t index = index_for(key, slots_.size());
        for (std::size_t probe = 0; probe < slots_.size(); ++probe) {
            const Slot& slot = slots_[index];
            if (slot.state == SlotState::Empty) {
                return false;
            }

            if (slot.state == SlotState::Occupied && slot.key == key) {
                return true;
            }

            index = (index + 1) % slots_.size();
        }

        return false;
    }

    std::size_t size() const
    {
        return size_;
    }

private:
    enum class SlotState {
        Empty,
        Occupied,
        Deleted,
    };

    struct Slot {
        SlotState state = SlotState::Empty;
        int key = 0;
        int value = 0;
    };

    std::vector<Slot> slots_;
    std::size_t size_ = 0;

    static std::size_t index_for(int key, std::size_t capacity)
    {
        return std::hash<int>{}(key) % capacity;
    }

    bool place_entry(int key, int value)
    {
        std::size_t index = index_for(key, slots_.size());
        std::size_t first_deleted = slots_.size();

        for (std::size_t probe = 0; probe < slots_.size(); ++probe) {
            Slot& slot = slots_[index];
            if (slot.state == SlotState::Empty) {
                if (first_deleted != slots_.size()) {
                    Slot& target = slots_[first_deleted];
                    target.state = SlotState::Occupied;
                    target.key = key;
                    target.value = value;
                } else {
                    slot.state = SlotState::Occupied;
                    slot.key = key;
                    slot.value = value;
                }

                ++size_;
                return true;
            }

            if (slot.state == SlotState::Deleted && first_deleted == slots_.size()) {
                first_deleted = index;
            } else if (slot.state == SlotState::Occupied && slot.key == key) {
                slot.value = value;
                return false;
            }

            index = (index + 1) % slots_.size();
        }

        if (first_deleted != slots_.size()) {
            Slot& target = slots_[first_deleted];
            target.state = SlotState::Occupied;
            target.key = key;
            target.value = value;
            ++size_;
            return true;
        }

        throw std::runtime_error("Linear probing table is full");
    }

    void rehash(std::size_t new_capacity)
    {
        std::vector<Slot> old_slots = std::move(slots_);
        slots_.assign(new_capacity, Slot{});
        size_ = 0;

        for (const Slot& slot : old_slots) {
            if (slot.state == SlotState::Occupied) {
                place_entry(slot.key, slot.value);
            }
        }
    }
};

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

#endif
