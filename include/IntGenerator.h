//
// Created by NANAnoo on 5/14/2023.
//

#ifndef CCOT_INTGENERATOR_H
#define CCOT_INTGENERATOR_H

#include "coroutine.h"
#include "struct_class.h"


#include <stdlib.h>
#include <string.h>

PROMISE(IntGenerator,
        int value;
        int valid;
)

int IMPL(IntGenerator, has_next, CO_ARGS(IntGenerator), {
    return this->valid == 1;
})

int IMPL(IntGenerator, next, CO_ARGS(IntGenerator), {
    this->_co_handle_->resume(this->_co_handle_);
    return this->value;
})

// ---------------
IntGenerator* IMPL(IntGenerator, get_return_obj, CO_ARGS(IntGenerator), {
    this->valid = 1;
    this->value = 0;
    return this;
})

void IMPL(IntGenerator, return_void, CO_ARGS(promise_handle), {
    this->valid = 0;
    this->valid = 0;
})

void IMPL(IntGenerator, return_value, CO_ARGS(IntGenerator, int value), {
    this->value = value;
    this->valid = 0;
})

awaiter_type *IMPL(IntGenerator, initial_suspend, CO_ARGS(promise_handle), {
    return &suspend_always;
})
awaiter_type *IMPL(IntGenerator, final_suspend, CO_ARGS(promise_handle), {
    return &suspend_never;
})
awaiter_type *IMPL(IntGenerator, await_transform, CO_ARGS(IntGenerator, int value), {
    this->value = value;
    this->valid = 1;
    return &suspend_always;
})
// -----------------

capture(IntGenerator, int, foreach, int x;)
void __attribute__((unused)) IMPL(IntGenerator, foreach, CO_ARGS(IntGenerator, lambda_type(IntGenerator, foreach) *cb, int max), {
    int i = 0;
    while(CALL(IntGenerator, this, has_next) && i ++ < max) {
        callback(cb, {
        cb->x = CALL(IntGenerator, this, next);
        });
    }
})

ASYNC(IntGenerator, char, list, int N; int i;)
    for(this->i = 0; this->i < this->N - 1; this->i++) {
        co_yield(IntGenerator, this->i + 1);
    }
    co_return(IntGenerator, this->N);
ASYNC_END

capture(IntGenerator, char, map_cb, int x;)
ASYNC(IntGenerator, char, map, lambda_type(IntGenerator, map_cb) *cb; int cb_state; IntGenerator *other;)
    while(CALL(IntGenerator, this->other, has_next)) {
        co_callback(this->cb, {
            this->cb->x = CALL(IntGenerator, this->other, next);
        });
        co_yield(IntGenerator, this->cb->_ret_);
    }
    co_return_void(IntGenerator)
ASYNC_END

capture(IntGenerator, char, flat_map_cb, int x;)
ASYNC(IntGenerator, char, flat_map, lambda_type(IntGenerator, flat_map_cb) *cb; int cb_state; IntGenerator *other; IntGenerator *temp;)
    while(CALL(IntGenerator, this->other, has_next)) {
        int len = CALL(IntGenerator, this->other, next);
        CO_NEW(IntGenerator, map, list, len);
        this->temp = map;
        while(CALL(IntGenerator, this->temp, has_next)) {
            co_callback(this->cb, {
                this->cb->x = CALL(IntGenerator, this->temp, next);
            });
            co_yield(IntGenerator, this->cb->_ret_);
        }
        free(this->temp);
        this->temp = NULL;
    }
    co_return_void(IntGenerator);
ASYNC_END

#endif //CCOT_INTGENERATOR_H
