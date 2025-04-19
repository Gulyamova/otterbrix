#pragma once

#include <unordered_map>
#include <components/logical_plan/node.hpp>
#include <optional>

namespace components::optimizer::utils {

class Memo {
public:
    std::optional<components::logical_plan::node_ptr>
    get(const components::logical_plan::node_ptr& node) const {
        size_t h = node->hash();
        auto it = cache_.find(h);
        if (it != cache_.end() && *it->second == *node) {
            return it->second;
        }
    return std::nullopt;
}
    void put(const components::logical_plan::node_ptr& node,
             const components::logical_plan::node_ptr& optimized) {
        size_t h = node->hash();
        cache_[h] = optimized;
    }

private:
    std::unordered_map<size_t, components::logical_plan::node_ptr> cache_;
};

} // namespace optimizer::utils
