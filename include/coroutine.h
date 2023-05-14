//
// Created by NANAnoo on 5/13/2023.
//

#ifndef CCOT_COROUTINE_H
#define CCOT_COROUTINE_H

#include "struct_class.h"

typedef struct promise_handle promise_handle;
typedef struct coroutine_handle coroutine_handle;
typedef struct awaiter_type awaiter_type;
typedef struct lambda_handle lambda_handle;

struct coroutine_handle {
    int state;
    void (*resume)(coroutine_handle *);
    lambda_handle *_cb_handle_;
};

struct lambda_handle {
    int return_state;
    coroutine_handle *_co_handle_;
};

struct awaiter_type {
    int (*await_ready)(awaiter_type *);
    void (*await_suspend)(awaiter_type *, coroutine_handle *);
    void (*await_resume)(awaiter_type *);
};

struct promise_handle {
    void (*return_void)(promise_handle *);
    awaiter_type *(*initial_suspend)(promise_handle *);
    awaiter_type *(*final_suspend)(promise_handle *);
    // T  (* T##_return_value)(promise_handle *);
    // T  (* T##_return_obj)(T *);
};

#define PROMISE(T, ...) \
    struct T {\
        promise_handle _handle_; \
        coroutine_handle *_co_handle_;\
        __VA_ARGS__\
    };\
    typedef struct T T;

#define AWAITER(T, ...) \
    struct T {\
        awaiter_type _handle_;\
        __VA_ARGS__\
    };\
    typedef struct T T;

#define LAMBDA_CAPTURE(T, ...) \
    struct T {\
        lambda_handle _handle_;\
        __VA_ARGS__\
    };\
    typedef struct T T;

#define COROUTINE_CAPTURE(T, ...) \
    struct T {\
        coroutine_handle _handle_;\
        __VA_ARGS__\
    };\
    typedef struct T T;

int IMPL(awaiter_type, suspend_always, CO_ARGS(awaiter_type), {
    return 0;
})
int IMPL(awaiter_type, suspend_never, CO_ARGS(awaiter_type), {
    return 1;
})
void IMPL(awaiter_type, empty_suspend, CO_ARGS(awaiter_type, coroutine_handle *co), {})
void IMPL(awaiter_type, empty_resume, CO_ARGS(awaiter_type), {})

static awaiter_type suspend_always  = {
    .await_ready = &FUNC(awaiter_type, suspend_always),
    .await_suspend = &FUNC(awaiter_type, empty_suspend),
    .await_resume = &FUNC(awaiter_type, empty_resume)
};

static awaiter_type suspend_never  = {
    .await_ready = &FUNC(awaiter_type, suspend_never),
    .await_suspend = &FUNC(awaiter_type, empty_suspend),
    .await_resume = &FUNC(awaiter_type, empty_resume)
};

#define WT_INIT(T, name, ...) \
    T name = {{.await_ready = T##_await_ready, .await_suspend = T##_await_suspend, .await_resume = T##_await_resume} __VA_OPT__(,) __VA_ARGS__};

#define co_begin switch(handle->state) { case 0:
#define co_await(awaiter) \
    do {                  \
        handle->state=__LINE__; \
        if(awaiter->await_ready(awaiter) == 0) {\
           awaiter->await_suspend(awaiter, handle);\
           return &CO;                             \
        }\
        case __LINE__:awaiter->await_resume(awaiter); \
    } while (0);

#define co_yield(T, value) this->yield_awaiter = T##_await_transform(this->promise, value); co_await(this->yield_awaiter);

#define co_return(T, value) T##_return_value(&CO, value) ; return &CO;
#define co_return_void CO.return_void(); return &CO;

#define ASYNC(T, yield_type, func, ...) \
COROUTINE_CAPTURE(T##CAP##func, __VA_ARGS__ T *promise; awaiter_type *init_awaiter; awaiter_type *final_awaiter; awaiter_type *yield_awaiter;) \
T * func(T##CAP##func *this); \
void T##_resume_##func (coroutine_handle *_cr_handle) {                                                                                       \
    func((T##CAP##func *)_cr_handle);             \
} \
T * func(T##CAP##func *this) {\
    coroutine_handle *handle = &this->_handle_;         \
    co_begin;               \
    {T _temp_ = {{.return_void = &T##_return_void, .initial_suspend = &T##_initial_suspend, .final_suspend = &T##_final_suspend}, handle};\
    this->promise = malloc(sizeof(T)); memcpy(this->promise, &_temp_, sizeof(T));                                               \
    this->promise = T##_get_return_obj(this->promise);                                    \
    this->init_awaiter = this->promise->_handle_.initial_suspend(&this->promise->_handle_);                                   \
    this->final_awaiter = this->promise->_handle_.final_suspend(&this->promise->_handle_);}                                                   \
    co_await(this->init_awaiter);                        \

#define ASYNC_END co_await(this->final_awaiter);}; \
      free(this->promise);                          \
return NULL;          \
}

#define capture(T, which, ...)LAMBDA_CAPTURE(T##which##lambda __VA_OPT__(,) __VA_ARGS__)

#define lambda_type(T, which) T##which##lambda

#define callback(cb, expr) \
do expr while(0); \
((lambda_handle *)cb)->_co_handle_->resume(((lambda_handle *)cb)->_co_handle_);

#define callback_end(cb) ((lambda_handle *)cb)->_co_handle_->state = ((lambda_handle *)cb)->return_state;callback(cb, {});

#define LAMBDA(T, func, expr, ...) \
    do {\
        lambda_type(T, func) cb = {{__LINE__, handle}};\
        handle->_cb_handle_ = (lambda_handle *) (&cb);\
        handle->state = -__LINE__;\
        CALL(T, gen, func, &cb __VA_OPT__(,) __VA_ARGS__);\
        case __LINE__:;\
    } while (0);              \
    do {                      \
    case -__LINE__ :\
        if (handle->state == -__LINE__) {\
            lambda_type(T, func) *cb = (lambda_type(T, func) *)handle->_cb_handle_;\
            do expr while(0);\
            return &CO;\
        }\
    } while(0);\

#define LAMBDA_IMPL(T, ret, func, expr, ...) \
    capture(T, func __VA_OPT__(,) __VA_ARGS__) \
    ret IMPL(T, func, CO_ARGS(T, lambda_type(T, func) *cb),{ \
         do {expr;}while(0);                 \
         callback_end(cb);                   \
    }) \

#define CO (*this->promise)

#define CO_INIT(T, name, func, ...) \
    T##CAP##func name##T##_cr_handle_ = {{.state = 0, .resume = T##_resume_##func} __VA_OPT__(,) __VA_ARGS__}; \
    T *name = func(&name##T##_cr_handle_);\

#endif //CCOT_COROUTINE_H
