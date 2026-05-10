#pragma once

#include <lumina/meta/console.hpp>
#include <lumina/meta/type_index.hpp>

#include "registry.hpp"
#include "set_functions.hpp"

namespace lumina::ecs {
    Registry::Registry(const core::ApplicationInfo& applicationInfo)
        : validation_(applicationInfo.features.validation) {
    }

    Registry::~Registry() {
        meta::logDebug(validation_, "{} entity(s) in registry destroyed", versions_.size() - freeList_.size());
    }

    Entity Registry::create() {
        IndexType id;
        IndexType version;

        if (!freeList_.empty()) {
            id = freeList_.back();
            version = versions_[id];

            freeList_.pop_back();
            statuses_[id] = false;

            meta::logDebug(validation_, "entity created [freelist] (id: {} version: {})", id, version);
        } else {
            id = static_cast<IndexType>(versions_.size());
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
        allocations_.clear();
        indexTables_.clear();

        versions_.clear();
        freeList_.clear();
        statuses_.clear();
    }

    template <Component T, typename... Args>
    T& Registry::emplace(const Entity& target, Args&&... args) {
        auto index = acquire<T>();
        auto& allocation = allocations_[index];
        auto& table = indexTables_[index];

        return SetFunctions::insert<T>(allocation, table, target.id(), std::forward<Args>(args)...);
    }

    template <Component T>
    bool Registry::has(const Entity& target) const {
        if (!acquirable<T>()) {
            return false;
        }

        auto index = typeIndex<T>();
        auto& table = indexTables_[index];

        return table.contains(target.id());
    }

    template <Component T>
    T& Registry::get(const Entity& target) {
        auto index = typeIndex<T>();
        auto& allocation = allocations_[index];
        auto& table = indexTables_[index];

        return SetFunctions::get<T>(allocation, table, target.id());
    }

    template <Component T>
    const T& Registry::get(const Entity& target) const {
        auto index = typeIndex<T>();
        auto& allocation = allocations_[index];
        auto& table = indexTables_[index];

        return SetFunctions::get<T>(allocation, table, target.id());
    }

    template <Component T>
    void Registry::remove(const Entity& target) {
        auto index = typeIndex<T>();
        auto& allocation = allocations_[index];
        auto& table = indexTables_[index];

        SetFunctions::remove(allocation, table, target.id());
    }

    void Registry::removeAll(const Entity& target) {
        for (std::size_t i = 0; i < allocations_.size(); ++i) {
            auto& table = indexTables_[i];

            if (!table.contains(target.id())) {
                return;
            }

            auto& allocation = allocations_[i];
            auto location = table[target.id()];
            auto stride = allocation.stride();
            auto size = allocation.size();

            auto destinationOffset = location * stride;
            auto sourceOffset = size - stride;

            if (destinationOffset < sourceOffset) {
                auto* data = allocation.data();
                auto* destination = data + destinationOffset;
                auto* source = data + sourceOffset;

                std::memmove(destination, source, stride);

                allocation.shrink(1);
                table.remove(target.id());
            }
        }
    }

    template <Component... Ts>
    View<Ts...> Registry::view() {
        return View<Ts...>{&indexTables_, &allocations_, &versions_};
    }

    template <Component... Ts>
    ConstView<Ts...> Registry::view() const {
        return ConstView<Ts...>{&indexTables_, &allocations_, &versions_};
    }

    template <Component T>
    std::size_t Registry::acquire() {
        auto index = typeIndex<T>();

        if (index >= allocations_.size()) {
            allocations_.resize(std::max(4ul, index * 2));
            indexTables_.resize(std::max(4ul, index * 2));
        }

        if (!allocations_[index]) {
            allocations_[index].initialise(sizeof(T));
        }

        return index;
    }

    template <Component T>
    bool Registry::acquirable() const {
        return typeIndex<T>() < allocations_.size();
    }
}
