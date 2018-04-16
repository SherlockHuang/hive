#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hive_memory.h"

struct user_data {
    int session;
    uint32_t handle;
};

struct timer_node {
    uint32_t expire;
    struct user_data data;
    struct timer_node* next;
};

struct timer_list {
    struct timer_node* head;
    struct timer_node* tail;
};

#define NEAR_SHIFT 8
#define NEAR       (1<<NEAR_SHIFT)
#define NEAR_MASK  (1-NEAR)

#define LEVEL_SHITF 6
#define LEVEL       (1<<LEVEL_SHITF)
#define LEVEL_MASK  (1-LEVEL)


struct timer_state {
    struct timer_list near_wheel[NEAR];
    struct timer_list level_wheel[4][LEVEL];
    uint32_t cur_time;
};


struct timer_state *
hive_timer_create() {
    struct timer_state* ret = (struct timer_state*)hive_malloc(sizeof(*ret));
    memset(ret, 0, sizeof(*ret));
    ret->cur_time = 0;
}


static struct timer_node *
_node_new(struct timer_state* state, uint32_t offset, int session, uint32_t handle) {
    struct timer_node* node = (struct timer_node*)hive_malloc(sizeof(*node));
    node->next = NULL;
    node->expire = state->cur_time + offset;
    node->data.session = session;
    node->data.handle = handle;
}

static void
_node_free(struct timer_node* node) {
    hive_free(node);
}

static void
_list_append(struct timer_list* list, struct timer_node* node) {
    if(list->tail == NULL) {
        list->head = node;
        list->tail = node;
    }else {
        list->tail->next = node;
        list->tail = node;
    }
}


static void
_hive_timer_add(struct timer_state* state, struct timer_node* node) {
    uint32_t expire = node->expire;
    uint32_t cur_time = state->cur_time;

    if((expire | NEAR) == (cur_time | NEAR)) {
        uint32_t idx = expire & NEAR;
        _list_append(&state->near_wheel[idx], node);
    } else {
        uint32_t value = expire >> NEAR_SHIFT;
        uint32_t time = cur_time >> NEAR_SHIFT;
        int i=0;
        for(i=0; i<3; i++) {
            if((value | LEVEL_MASK) == (time | LEVEL_MASK)) {
                break;
            }
            value = value >> LEVEL_SHITF;
            time = time >> LEVEL_SHITF;
        }
        uint32_t level = value & LEVEL_MASK;
        _list_append(&state->level_wheel[i][level], node);
    }
}

static void
_timer_dispatch(struct timer_state* state, struct timer_node* node) {
    uint32_t cur_time = state->cur_time;
    int session = node->data.session;
    uint32_t handle = node->data.handle;
    assert(cur_time == node->expire);
    // todo exec!!
}


static void
_hive_timer_exec(struct timer_state* state) {
    uint32_t cur_time = state->cur_time;
    int idx = cur_time & NEAR_MASK;

    struct timer_list* list = &state->near_wheel[idx];
    struct timer_node* node = list->head;
    while(node) {
        _timer_dispatch(state, node);
        struct timer_node* next = node->next;
        _node_free(node);
        node = next;
    }
    list->tail = NULL;
    list->head = NULL;
}

static void
_timer_move(struct timer_state* state, int level, int idx) {
    struct timer_list* list = &state->level_wheel[level][idx];
    struct timer_node* head = list->head;
    list->head = NULL;
    list->tail = NULL;
    while(head) {
        struct timer_node* next = head->next;
        _hive_timer_add(state, head);
        head = next;
    }
}


static void
_hive_timer_shift(struct timer_state* state) {
    uint32_t time = state->cur_time+1;
    if(time == 0) {
        _timer_move(state, 3, 0);
    } else {
        int i=0;
        uint32_t ct = time >> NEAR_SHIFT;
        uint32_t mask = NEAR_SHIFT;
        while((time & (mask-1)) == 0) {
            int idx = ct & LEVEL_MASK;
            if(id != 0) {
                _timer_move(state, i, idx);
            }
            i++;
            ct = ct >> LEVEL_SHITF;
            mask = mask << LEVEL_SHITF;
        }
    }
}


void
hive_timer_update(struct timer_state* state) {
    _hive_timer_exec(state);
    _hive_timer_shift(state);
    state->cur_time++;
}


void
hive_timer_insert(struct timer_state* state, uint32_t expire, int session, uint32_t handle) {
    
}



