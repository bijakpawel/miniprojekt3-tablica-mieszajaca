#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>

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
