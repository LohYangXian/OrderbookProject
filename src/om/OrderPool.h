#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <new>
#include <utility>
#include <vector>

#include "Order.h"

class OrderPool {
public:
    explicit OrderPool(std::size_t chunkSize = 4096, std::size_t initialChunkCount = 0)
        : chunkSize_(chunkSize) {
        chunks_.reserve(initialChunkCount);
        for (std::size_t i = 0; i < initialChunkCount; ++i) {
            addChunkUnlocked();
        }
    }

    void preallocate(std::size_t objectCount) {
        if (objectCount == 0) {
            return;
        }

        std::scoped_lock lock(mutex_);
        const std::size_t currentCapacity = chunks_.size() * chunkSize_;
        if (currentCapacity >= objectCount) {
            return;
        }

        const std::size_t remaining = objectCount - currentCapacity;
        const std::size_t chunksToAdd = (remaining + chunkSize_ - 1) / chunkSize_;
        chunks_.reserve(chunks_.size() + chunksToAdd);
        for (std::size_t i = 0; i < chunksToAdd; ++i) {
            addChunkUnlocked();
        }
    }

    template <typename... Args>
    Order* allocate(Args&&... args) {
        Slot* slot = nullptr;
        {
            std::scoped_lock lock(mutex_);
            if (freeList_ == nullptr) {
                addChunkUnlocked();
            }
            slot = freeList_;
            freeList_ = freeList_->next;
        }

        try {
            return new (slot->storage) Order(std::forward<Args>(args)...);
        } catch (...) {
            std::scoped_lock lock(mutex_);
            slot->next = freeList_;
            freeList_ = slot;
            throw;
        }
    }

    void deallocate(Order* order) {
        if (order == nullptr) {
            return;
        }

        order->~Order();
        Slot* slot = slotFromOrder(order);

        std::scoped_lock lock(mutex_);
        slot->next = freeList_;
        freeList_ = slot;
    }

private:
    struct Slot {
        alignas(Order) unsigned char storage[sizeof(Order)];
        Slot* next = nullptr;
    };

    static_assert(offsetof(Slot, storage) == 0, "Slot storage must be first field");

    static Slot* slotFromOrder(Order* order) {
        return reinterpret_cast<Slot*>(order);
    }

    void addChunkUnlocked() {
        auto chunk = std::make_unique<Slot[]>(chunkSize_);
        for (std::size_t i = 0; i < chunkSize_; ++i) {
            chunk[i].next = freeList_;
            freeList_ = &chunk[i];
        }
        chunks_.push_back(std::move(chunk));
    }

    std::size_t chunkSize_;
    std::vector<std::unique_ptr<Slot[]>> chunks_;
    Slot* freeList_ = nullptr;
    std::mutex mutex_;
};
