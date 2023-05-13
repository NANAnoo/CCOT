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

capture(IntGenerator, foreach, int x;)
void IMPL(IntGenerator, foreach, CO_ARGS(IntGenerator, lambda_type(IntGenerator, foreach) *cb, int max), {
int i = 0;
while(CALL(IntGenerator, this, has_next) && i ++ < max) {
callback(cb, {
cb->x = CALL(IntGenerator, this, next);
});
}
callback_end(cb);
})

#endif //CCOT_INTGENERATOR_H
