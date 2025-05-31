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

    bool has_cost(const components::logical_plan::node_ptr& node) const {
        return cost_cache_.contains(node->hash());
    }

    components::optimizer::cost::cost_t
    get_cost(const components::logical_plan::node_ptr& node) const {
        return cost_cache_.at(node->hash());
    }

    void put_cost(const components::logical_plan::node_ptr& node,
                  const components::optimizer::cost::cost_t& c) {
        cost_cache_[node->hash()] = c;
    }

private:
    std::unordered_map<size_t, components::logical_plan::node_ptr> cache_;
    std::unordered_map<size_t, components::optimizer::cost::cost_t> cost_cache_; 
};

} // namespace optimizer::utils
