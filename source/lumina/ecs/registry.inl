#pragma once

#include <lumina/meta/console.hpp>
#include <lumina/meta/type_index.hpp>

#include "registry.hpp"

namespace lumina::ecs {
    Registry::Registry(const core::ApplicationInfo& applicationInfo)
        : validation_(applicationInfo.features.validation) {
    }

    Registry::~Registry() {
        meta::logDebug(validation_, "{} entity(s) in registry destroyed", versions_.size() - freeList_.size());
    }

    Entity Registry::create() {
        std::size_t id;
        std::size_t version;

        if (!freeList_.empty()) {
            id = freeList_.back();
            version = versions_[id];

            freeList_.pop_back();
            statuses_[id] = false;

            meta::logDebug(validation_, "entity created [freelist] (id: {} version: {})", id, version);
        } else {
            id = static_cast<std::size_t>(versions_.size());
            version = versions_.emplace_back(0);

            statuses_.emplace_back(true);

            meta::logDebug(validation_, "entity created [new] (id: {} version: {})", id, version);
        }

        return Entity{id, version};
    }

    bool Registry::contains(const Entity& target) const {
        if (!target) {
            return false;
        }

        auto id = target.id();
        auto version = target.version();

        return versions_.size() > id && versions_[id] == version && statuses_[id];
    }

    void Registry::destroy(const Entity& target) {
        if (!contains(target)) {
            return;
        }

        auto id = target.id();

        removeAll(target);

        versions_[id]++;
        freeList_.emplace_back(id);
        statuses_[id] = false;

        meta::logDebug(validation_, "entity destroyed (id: {} version: {})", target.id(), target.version());
    }

    void Registry::clear() {
        memoryPools_.clear();
        indexTables_.clear();
        typeSizes_.clear();

        versions_.clear();
        freeList_.clear();
        statuses_.clear();
    }

    template <Component T>
    T& Registry::emplace(const Entity& target) {
        auto index = acquire<T>();
        auto& memoryPool = memoryPools_[index];
        auto& indexTable = indexTables_[index];

        return SparseSet<T>::insert(memoryPool, indexTable, target.id());
    }

    template <Component T>
    T& Registry::emplace(const Entity& target, const T& value) {
        auto index = acquire<T>();
        auto& memoryPool = memoryPools_[index];
        auto& indexTable = indexTables_[index];

        return SparseSet<T>::insert(memoryPool, indexTable, target.id(), value);
    }

    template <Component T>
    T& Registry::emplace(const Entity& target, T&& value) {
        auto index = acquire<T>();
        auto& memoryPool = memoryPools_[index];
        auto& indexTable = indexTables_[index];

        return SparseSet<T>::insert(memoryPool, indexTable, target.id(), value);
    }

    template <Component T>
    bool Registry::has(const Entity& target) const {
        if (!acquirable<T>()) {
            return false;
        }

        auto index = typeIndex<T>();
        auto& indexTable = indexTables_[index];

        return indexTable.contains(target.id());
    }

    template <Component T>
    T& Registry::get(const Entity& target) {
        auto index = typeIndex<T>();
        auto& memoryPool = memoryPools_[index];
        auto& indexTable = indexTables_[index];

        return SparseSet<T>::at(memoryPool, indexTable, target.id());
    }

    template <Component T>
    const T& Registry::get(const Entity& target) const {
        auto index = typeIndex<T>();
        auto& memoryPool = memoryPools_[index];
        auto& indexTable = indexTables_[index];

        return SparseSet<T>::at(memoryPool, indexTable, target.id());
    }

    template <Component T>
    void Registry::remove(const Entity& target) {
        auto index = typeIndex<T>();
        auto& memoryPool = memoryPools_[index];
        auto& indexTable = indexTables_[index];

        SparseSet<T>::remove(memoryPool, indexTable, target.id());
    }

    void Registry::removeAll(const Entity& target) {
        for (std::size_t i = 0; i < memoryPools_.size(); ++i) {
            auto& indexTable = indexTables_[i];

            if (!indexTable.contains(target.id())) {
                return;
            }

            auto& memoryPool = memoryPools_[i];
            auto location = indexTable[target.id()];
            auto stride = typeSizes_[i];
            auto size = memoryPool.size();

            auto destinationOffset = location * stride;
            auto sourceOffset = size - stride;

            if (destinationOffset < sourceOffset) {
                auto* data = memoryPool.data();
                auto* destination = data + destinationOffset;
                auto* source = data + sourceOffset;

                std::memmove(destination, source, stride);

                memoryPool.free(stride);
                indexTable.remove(target.id());
            }
        }
    }

    template <Component... Ts>
    View<Ts...> Registry::view() {
        return View<Ts...>{&indexTables_, &memoryPools_, &versions_};
    }

    template <Component T>
    std::size_t Registry::acquire() {
        auto index = typeIndex<T>();

        if (index >= memoryPools_.size()) {
            memoryPools_.resize(std::max(index + 1, index * 2));
            indexTables_.resize(std::max(index + 1, index * 2));
            typeSizes_.resize(std::max(index + 1, index * 2), 0xFFFFFFFFFFFFFFFFull);
        }

        if (typeSizes_[index] == 0xFFFFFFFFFFFFFFFFull) {
            typeSizes_[index] = sizeof(T);
        }

        return index;
    }

    template <Component T>
    bool Registry::acquirable() const {
        return typeIndex<T>() < memoryPools_.size();
    }
}
