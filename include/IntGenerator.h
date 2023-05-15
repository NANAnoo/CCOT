//
// Created by NANAnoo on 5/14/2023.
//

#ifndef CCOT_INTGENERATOR_H
#define CCOT_INTGENERATOR_H

#include "coroutine.h"
#include "struct_class.h"

PROMISE(IntGenerator,
        int value;
        int valid;
)

int IMPL(IntGenerator, has_next, CO_ARGS(IntGenerator), {
    return this != NULL && this->valid == 1;
})

//int IMPL(IntGenerator, next, CO_ARGS(IntGenerator), {
//    this->_co_handle_->resume(this->_co_handle_);
//    int ret = this->value;
//    if (this->_co_handle_ == NULL) {
//        m_free(this, "IntGenerator");
//        this = NULL;
//    }
//    return ret;
//})

__attribute__((unused))int IntGenerator_next(IntGenerator**_this) {
    IntGenerator *this = (IntGenerator *) (*_this);
    do {
        {
            this->_co_handle_->resume(this->_co_handle_);
            int ret = this->value;
            if (this->_co_handle_ == ((void *) 0)) {
                FREE_THIS;
            }
            return ret;
        }
    } while (0);
}

// ---------------
IntGenerator* IMPL(IntGenerator, get_return_obj, CO_ARGS(IntGenerator), {
    this->valid = 1;
    this->value = 0;
    return this;
})

void IMPL(IntGenerator, return_void, CO_ARGS(promise_handle), {
    this->valid = 0;
    this->value = 0;
})

void IMPL(IntGenerator, return_value, CO_ARGS(IntGenerator, int value), {
    this->value = value;
    this->valid = 0;
})

awaiter_type *IMPL(IntGenerator, initial_suspend, CO_ARGS(promise_handle), {
    return &suspend_always;
})
awaiter_type *IMPL(IntGenerator, final_suspend, CO_ARGS(promise_handle), {
    return &suspend_clean;
})
awaiter_type *IMPL(IntGenerator, await_transform, CO_ARGS(IntGenerator, int value), {
    this->value = value;
    this->valid = 1;
    return &suspend_always;
})
// -----------------
// COROUTINE: IntGenerator list();
ASYNC(IntGenerator, char, list, int begin; int end; int i;)
        for($$(i) = $$(begin); $$(i) < $$(end); $$(i)++) {
            co_yield(IntGenerator, this->i)
        }
        co_return(IntGenerator, $$(end))
ASYNC_END

// IntGenerator::foreach([(int x)->int] callback, int max)
CB_IMPL(void, IntGenerator, foreach, CB_ARGS(IntGenerator, int, foreach, int max),{
    int i = 0;
    while(CALL(IntGenerator, this, has_next) && i ++ < max) {
        $eval(cb, {cb->x = CALL(IntGenerator, this, next);});
    }
}, int, int x;)

// COROUTINE: IntGenerator::map([(int x)->char] callback, IntGenerator *other)
$lambda(IntGenerator, char, map, int x;)
CB_ASYNC(IntGenerator, char, map, char, IntGenerator *other; IntGenerator *temp;)
    while(CALL(IntGenerator, this->other, has_next)) {
        $eval(cb, {cb->x = CALL(IntGenerator, this->other, next);})
        if (this->other) {
            co_yield(IntGenerator, $cb_ret)
        }
    }
    co_return(IntGenerator, $cb_ret)
ASYNC_END

// COROUTINE: IntGenerator::flat_map([(int x)->char] callback, IntGenerator *gen)
typedef IntGenerator * IntGeneratorRef;
$lambda(IntGenerator, IntGeneratorRef, flat_map, int x;)
CB_ASYNC(IntGenerator, char, flat_map, IntGeneratorRef, IntGenerator *other; int ret_val;)
    while(CALL(IntGenerator, this->other, has_next)) {
        $eval(cb, {cb->x = CALL(IntGenerator, this->other, next);})
        while (CALL(IntGenerator, $cb_ret, has_next)) {
            $$(ret_val) = CALL(IntGenerator, $cb_ret, next);
            co_yield(IntGenerator, $$(ret_val));
            if (! $cb_ret && ! $$(other)) {
                co_return(IntGenerator, $$(ret_val));
            }
        }
    }
    co_return(IntGenerator, 0)
ASYNC_END

#define KILL_INTGENERATOR(gen) while(gen) CALL(IntGenerator, gen, next);

#endif //CCOT_INTGENERATOR_H
