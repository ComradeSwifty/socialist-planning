#ifndef plan_h
#define plan_h

#include <inttypes.h>
#include <stdbool.h>

typedef uint_fast32_t uval_t;
typedef int_fast32_t val_t;
typedef uint_fast64_t uval64_t;
typedef int_fast64_t val64_t;

typedef struct {
    uval_t id;
    uval_t count;
} product_t;

typedef struct {
    product_t product;
    uval64_t length;
    product_t costs[];
} production_costs_t;

typedef struct {
    uval64_t length;
    production_costs_t* data[];
} economy_t;

typedef struct {
    uval64_t length;
    product_t resources[];
} supply_t;

typedef struct {
    uval64_t length;
    product_t targets[];
} plan_t;

typedef struct {
    product_t product;
    uval64_t length;
    val_t constituents[];
} decomposition_t;

typedef struct {
    uval_t count;
    uval_t id;
    uval_t source;
    //uval_t destination_prod;
    uval_t destination;
} request_t;

#define Product(m_id, m_count) { .id=(m_id), .count=(m_count) }
const product_t ProductMake(const uval_t id, const uval_t count);
production_costs_t* ProductionCosts(product_t product, uval64_t length, const product_t costs[]);
economy_t* Economy(uval64_t length);
supply_t* Supply(uval64_t length, const product_t resources[]);
plan_t* Plan(uval64_t length, const product_t targets[]);

void dump_product(const product_t product, const char* const * name_lookup_table);
void dump_economy(const economy_t* economy, const char* const * name_lookup_table);
void dump_plan(const plan_t* plan, const char* const * name_lookup_table);

void insert_production_costs(economy_t* economy, production_costs_t* production_costs);

typedef enum {
    OES_Even
} OutputEvaluatingStrategy;

// Evaluating
void evaluate_output(product_t* output, const production_costs_t* production_costs, supply_t* supply, const uval64_t economy_size, const OutputEvaluatingStrategy oes);
void evaluate_outputs(product_t outputs[], const economy_t* economy, const supply_t* _supply, const OutputEvaluatingStrategy oes);

// Resource reallocation
void raw_net_difference(val_t net_difference[], plan_t* plan, const economy_t* economy, const supply_t* supply, const OutputEvaluatingStrategy oes);
void decompose(product_t decomposition[], const product_t product, const economy_t* economy, bool roundUp);
void calculate_decompositions(decomposition_t* decompositions[], plan_t* plan, const economy_t* economy, const supply_t* supply, const OutputEvaluatingStrategy oes);
void print_balanced_plan(plan_t* plan, const economy_t* economy, const supply_t* supply, const OutputEvaluatingStrategy oes, const char* const * name_lookup_table);

#endif /* plan_h */
