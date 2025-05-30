#pragma once

#include "forward.hpp"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <components/base/collection_full_name.hpp>
#include <components/expressions/expression.hpp>
#include <memory_resource>
#include <optional>
#include <unordered_set>
#include <statistics/abstract_statistics.hpp>
#include <optimizer/cost/estimator.hpp> 

namespace components::serializer {
    class base_serializer_t;
    class base_deserializer_t;
} // namespace components::serializer

namespace components::logical_plan {

    class node_t;
    using node_ptr = boost::intrusive_ptr<node_t>;
    using expression_ptr = expressions::expression_ptr;
    using hash_t = expressions::hash_t;

    class node_t : public boost::intrusive_ref_counter<node_t> {
    public:
        node_t(std::pmr::memory_resource* resource, node_type type, const collection_full_name_t& collection);
        virtual ~node_t() = default;

        node_type type() const;
        const collection_full_name_t& collection_full_name() const;
        const database_name_t& database_name() const;
        const collection_name_t& collection_name() const;
        const std::pmr::vector<node_ptr>& children() const;
        std::pmr::vector<node_ptr>& children();
        const std::pmr::vector<expression_ptr>& expressions() const;

        void reserve_child(std::size_t count);
        void append_child(const node_ptr& child);
        void append_expression(const expression_ptr& expression);
        void append_expressions(const std::vector<expression_ptr>& expressions);
        void append_expressions(const std::pmr::vector<expression_ptr>& expressions);

        std::unordered_set<collection_full_name_t, collection_name_hash> collection_dependencies();

        bool operator==(const node_t& rhs) const;
        bool operator!=(const node_t& rhs) const;

        hash_t hash() const;
        virtual void serialize(serializer::base_serializer_t*) const;

        static node_ptr deserialize(serializer::base_deserializer_t* deserializer);

        std::string to_string() const;
        std::pmr::memory_resource* resource() const noexcept;

        // --- Поддержка статистики для CBO ---
        void set_statistics(std::shared_ptr<statistics::AbstractStatistics> stats);
        const std::shared_ptr<statistics::AbstractStatistics>& statistics() const;

        void set_estimated_rows(size_t rows);
        size_t estimated_rows() const;

        void set_cost(const components::optimizer::cost::cost_t& c) { cost_ = c; }
        const std::optional<components::optimizer::cost::cost_t>& cost() const { return cost_; }

        void   set_cpu_cost(double v) { cpu_cost_ = v; }
        void   set_ram_cost(double v) { ram_cost_ = v; }
        double cpu_cost()       const { return cpu_cost_; }
        double ram_cost()       const { return ram_cost_; }


    protected:
        const node_type type_;
        const collection_full_name_t collection_;
        std::pmr::vector<node_ptr> children_;
        std::pmr::vector<expression_ptr> expressions_;
        std::optional<components::optimizer::cost::cost_t> cost_;

        void
        collection_dependencies_(std::unordered_set<collection_full_name_t, collection_name_hash>& upper_dependencies);

    private:
        virtual hash_t hash_impl() const = 0;
        virtual std::string to_string_impl() const = 0;
        virtual void serialize_impl(serializer::base_serializer_t*) const = 0;
        std::shared_ptr<statistics::AbstractStatistics> statistics_;  
        size_t estimated_rows_ = 0;
        double cpu_cost_ = 0.0;
        double ram_cost_ = 0.0;                                   

    };

    struct node_hash final {
        size_t operator()(const node_ptr& node) const { return node->hash(); }
    };

    struct node_equal final {
        size_t operator()(const node_ptr& lhs, const node_ptr& rhs) const { return lhs == rhs || *lhs == *rhs; }
    };

    template<class OStream>
    OStream& operator<<(OStream& stream, const node_ptr& node) {
        stream << node->to_string();
        return stream;
    }

} // namespace components::logical_plan