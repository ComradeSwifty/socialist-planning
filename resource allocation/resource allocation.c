// VERY BAD AND MESSY...I KNOW!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "plan.h"
#include "mem-utils.h"

const product_t ProductMake(const uval_t id, const uval_t count) {
    product_t product;
    product.id = id;
    product.count = count;
    return product;
}
production_costs_t* ProductionCosts(product_t product, uval64_t length, const product_t costs[]) {
    production_costs_t* production_costs = (production_costs_t*)malloc(sizeof(production_costs_t) + length * sizeof(product_t));
    production_costs->product = product;
    production_costs->length = length;
    memcpy(production_costs->costs, costs, length * sizeof(product_t));
    return production_costs;
}
economy_t* Economy(uval64_t length) {
    economy_t* economy = (economy_t*)malloc(sizeof(economy_t) + length * sizeof(production_costs_t*));
    economy->length = length;
    return economy;
}
supply_t* Supply(uval64_t length, const product_t resources[]) {
    supply_t* supply = (supply_t*)malloc(sizeof(supply_t) + length * sizeof(product_t));
    supply->length = length;
    memcpy(supply->resources, resources, length * sizeof(product_t));
    return supply;
}
plan_t* Plan(uval64_t length, const product_t targets[]) {
    plan_t* plan = (plan_t*)malloc(sizeof(plan_t) + length * sizeof(product_t));
    plan->length = length;
    memcpy(plan->targets, targets, length * sizeof(product_t));
    return plan;
}

void dump_product(const product_t product, const char* const * name_lookup_table) {
    if (name_lookup_table == NULL) {
        printf("ID: %u\tCount: %u\n", product.id, product.count);
    } else {
        printf("%s [Count: %u]\n", name_lookup_table[product.id], product.count);
    }
}
void dump_economy(const economy_t* economy, const char* const * name_lookup_table) {
    for (uval_t i = 0; i < economy->length; i++) {
        const production_costs_t* production_costs = economy->data[i];
        dump_product(production_costs->product, name_lookup_table);
        for (uval_t j = 0; j < production_costs->length; j++) {
            fputs(" - ", stdout);
            dump_product(production_costs->costs[j], name_lookup_table);
        }
    }
}
void dump_plan(const plan_t* plan, const char* const * name_lookup_table) {
    printf("Plan Targets:\n");
    for (uval_t i = 0; i < plan->length; i++) {
        if (name_lookup_table == NULL) {
            printf(" - %u of %u\n", plan->targets[i].count, plan->targets[i].id);
        } else {
            printf(" - %u %s\n", plan->targets[i].count, name_lookup_table[plan->targets[i].id]);
        }
    }
}

void insert_production_costs(economy_t* economy, production_costs_t* production_costs) {
    economy->data[production_costs->product.id] = production_costs;
}

void evaluate_output(product_t* output, const production_costs_t* production_costs, supply_t* supply, const uval64_t economy_size, const OutputEvaluatingStrategy oes) {
    output->id = production_costs->product.id;
    uval_t accumulator = 0;
    for (uval_t i = 0; i < production_costs->length; i++) {
        const product_t cost = production_costs->costs[i];
        product_t avaliable = supply->resources[cost.id];
        switch (oes) {
            case OES_Even: {
                if (cost.count) {
                    accumulator += (avaliable.count / economy_size * production_costs->product.count) / (cost.count);
                } else if (avaliable.id == output->id) {
                    accumulator += avaliable.count;
                }
                break;
            }
        }
    }
    output->count = accumulator;
}

void evaluate_outputs(product_t outputs[], const economy_t* economy, const supply_t* _supply, const OutputEvaluatingStrategy oes) {
    supply_t* supply = memdup(_supply, sizeof(supply_t) + _supply->length * sizeof(product_t));
    for (uval_t i = 0; i < economy->length; i++) {
        evaluate_output(outputs + i, economy->data[i], supply, economy->length, oes);
    }
}

void raw_net_difference(val_t net_difference[], plan_t* plan, const economy_t* economy, const supply_t* supply, const OutputEvaluatingStrategy oes) {
    product_t outputs[economy->length];
    evaluate_outputs(outputs, economy, supply, oes);
    
    for (uval_t i = 0; i < economy->length; i++) {
        net_difference[i] = (val_t)((val64_t)(outputs[i].count) - (val64_t)plan->targets[i].count);
    }
}

void decompose(product_t decomposition[], const product_t product, const economy_t* economy, bool roundUp) {
    production_costs_t* production_costs = economy->data[product.id];
    for (uval_t i = 0; i < production_costs->length; i++) {
        const product_t cost = production_costs->costs[i];
        decomposition[i].id = cost.id;
        if (cost.count) {
            decomposition[i].count = (uval_t)((double)(product.count + roundUp) / ((double)production_costs->product.count / (double)cost.count));
        } else {
            decomposition[i].count = 0;
        }
    }
}

void calculate_decompositions(decomposition_t* decompositions[], plan_t* plan, const economy_t* economy, const supply_t* supply, const OutputEvaluatingStrategy oes) {
    
    // Calculate the distance the outputs are from the targets
    val_t dist_from_targets[economy->length];
    raw_net_difference(dist_from_targets, plan, economy, supply, oes);
    
    // Decompose the distances into their constituents
    for (uval_t i = 0; i < economy->length; i++) {
        decomposition_t* decomposition = (decomposition_t*)malloc(sizeof(decomposition_t) + economy->length * sizeof(val_t));
        decomposition->length = economy->length;
        decomposition->product = ProductMake(i, abs(dist_from_targets[i]));
//        dump_product(decomposition->product, NULL);
        product_t constituents[economy->length];
        decompose(constituents, decomposition->product, economy, dist_from_targets[i] < 0);
//        printf("Composition of %u:\n", i);
//        for (uval_t i = 0; i < economy->length; i++) {
//            printf(" - %u of %u\n", constituents[i].count, constituents[i].id);
//        }
        for (uval_t j = 0; j < economy->length; j++) {
            if (dist_from_targets[i] == 0) {
                decomposition->constituents[j] = 0;
            } else {
                decomposition->constituents[j] = ((dist_from_targets[i] < 0) ? -1 : 1) * (val_t)constituents[j].count;
            }
        }
        decompositions[i] = decomposition;
    }
}


#define cond_maker(x,y,op) ((x) ## op ## (y)?(x):(y)
#define max(x,y) cond_maker(x,y,>)
#define min(x,y) cond_maker(x,y,<)

void print_balanced_plan(plan_t* plan, const economy_t* economy, const supply_t* supply, const OutputEvaluatingStrategy oes, const char* const * name_lookup_table) {
    // Calculate the decompositions of the product's output-target differences
    decomposition_t* decompositions[economy->length];
    calculate_decompositions(decompositions, plan, economy, supply, OES_Even);
    
    uval_t request_table_length = 0;
    request_t request_table[economy->length * economy->length];
    
    // Form the requests for resource movement
    for (uval_t i = 0; i < economy->length; i++) {
        val_t* constituents = decompositions[i]->constituents;
        for (uval_t j = 0; j < economy->length; j++) {
            if (constituents[j] < 0) {
                if (name_lookup_table == NULL) {
                    printf("Must draw %u of %u from surplus for %u\n", abs(constituents[j]), j, i);
                } else {
                    printf("Must draw %u of %s from surplus for %s\n", abs(constituents[j]), name_lookup_table[j], name_lookup_table[i]);
                }
                request_t request;
                request.count = abs(constituents[j]);
                request.id = j;
                request.source = j;
                request.destination = i;
                request_table[request_table_length++] = request;
            }
        }
    }
    
    // Copy over the old supply
    uval_t new_allocations[economy->length][economy->length];
    for (uval_t i = 0; i < economy->length; i++) {
        const uval_t current = supply->resources[i].count / economy->length;
        for (uval_t j = 0; j < economy->length; j++) {
            new_allocations[i][j] = current;
        }
        //new_allocations[i] = supply->resources[i].count;
    }
    
    // Update resource allocation as per requests
    for (uval_t i = 0; i < request_table_length; i++) {
        const uval_t count = request_table[i].count;
        const uval_t id = request_table[i].id;
        const uval_t source = request_table[i].source;
        const uval_t dest = request_table[i].destination;
        if (new_allocations[id][source] < count) {
            if (name_lookup_table == NULL) {
                printf("NOT ENOUGH %u\n", source);
            } else {
                printf("NOT ENOUGH %s\n", name_lookup_table[source]);
            }
            if (new_allocations[id][source] > 1) {
                printf("ATTEMPTING TO SOLVE...\n");
                uval_t movable = (plan->targets[dest].count * new_allocations[id][source]) / (plan->targets[source].count + plan->targets[dest].count);
                new_allocations[id][source] -= movable;
                new_allocations[id][dest] += movable;
            } else {
                printf("COMPLETE FAIL. TERMINATING...\n");
                return;
            }
        } else {
            new_allocations[id][source] -= count;
            new_allocations[id][dest] += count;
        }
    }
    
    for (uval_t i = 0; i < economy->length; i++) {
        if (name_lookup_table == NULL) {
            printf("New allocations for %u:\n", i);
        } else {
            printf("New allocations for %s:\n", name_lookup_table[i]);
        }
        for (uval_t j = 0; j < economy->length; j++) {
            if (name_lookup_table == NULL) {
                printf(" - %u of %u:\n", j, new_allocations[j][i]);
            } else {
                printf(" - %u %s:\n", new_allocations[j][i], name_lookup_table[j]);
            }
        }
    }
    
    // Clean up allocated decompositions
    for (uval_t i = 0; i < economy->length; i++) {
        free(decompositions[i]);
    }
    
    uval_t outputs[economy->length];
    for (uval_t i = 0; i < economy->length; i++) {
        production_costs_t* production_costs = economy->data[i];
        uval_t accumulator = 0;
        for (uval_t j = 0; j < economy->length; j++) {
            const uval_t given_count = new_allocations[j][i];
            if (production_costs->costs[j].count) {
                accumulator += (production_costs->product.count * given_count) / production_costs->costs[j].count;
            }
        }
        outputs[i] = accumulator;
    }
    val_t closeness_to_targets[economy->length];
    for (uval_t i = 0; i < economy->length; i++) {
        closeness_to_targets[i] = (val_t)((val64_t)(outputs[i]) - (val64_t)plan->targets[i].count);
    }

//    uval_t outputs[economy->length];
//
//
//    raw_net_difference(closeness_to_targets, plan, economy, supply, OES_Even);
    printf("Modified output given redistrubutions\n");
    for (uval_t i = 0; i < economy->length; i++) {
        const val_t closeness = closeness_to_targets[i];
        const uval_t output = outputs[i];
        if (name_lookup_table == NULL) {
            if (closeness > 0) {
                printf(" - %u of %u (%d above targets)\n", output, i, closeness);
            } else if (closeness < 0) {
                printf(" - %u of %u (%d below targets)\n", output, i, -closeness);
            } else {
                printf(" - %u of %u (at targets)\n", output, i);
            }
        } else {
            if (closeness > 0) {
                printf(" - %u %s (%d above targets)\n", output, name_lookup_table[i], closeness);
            } else if (closeness < 0) {
                printf(" - %u %s (%d below targets)\n", output, name_lookup_table[i], -closeness);
            } else {
                printf(" - %u %s (at targets)\n", output, name_lookup_table[i]);
            }
        }
    }
}
