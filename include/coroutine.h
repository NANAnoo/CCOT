//
// Created by NANAnoo on 5/13/2023.
//

#ifndef CCOT_COROUTINE_H
#define CCOT_COROUTINE_H

#include "struct_class.h"

#include <stdlib.h>
#include <string.h>

//#define MALLOC_DEBUG

#ifdef MALLOC_DEBUG
static int m_count = 0, f_count = 0;
#endif
void *m_malloc(size_t size, const char *type) {
#ifdef MALLOC_DEBUG
    m_count ++;
    printf("[%d]my malloc %s\n", m_count, type);
    fflush(stdout);
#endif
    return malloc(size);
}

void m_free(void *ptr, const char *type) {
#ifdef MALLOC_DEBUG
    f_count ++;
    printf("[%d]my free %s\n", f_count, type);
    fflush(stdout);
#endif
    free(ptr);
}

#define FREE_THIS m_free(*_this, __func__);*_this = NULL

typedef struct promise_handle promise_handle;
typedef struct coroutine_handle coroutine_handle;
typedef struct awaiter_type awaiter_type;
typedef struct lambda_handle lambda_handle;

struct coroutine_handle {
    int state;
    int on_stack;
    void (*resume)(coroutine_handle *);
    void (*onFree)(coroutine_handle **);
    promise_handle *promise;
    awaiter_type *init_awaiter;
    awaiter_type *final_awaiter;
    awaiter_type *yield_awaiter;
    lambda_handle *_cb_handle_;
};

// TODO : add capture list for lambda handle
struct lambda_handle {
    int callback_state;
    coroutine_handle *_co_handle_;
};

struct awaiter_type {
    int (*await_ready)(awaiter_type **);
    void (*await_suspend)(awaiter_type **, coroutine_handle *);
    void (*await_resume)(awaiter_type **);
};

struct promise_handle {
    void (*return_void)(promise_handle **);
    awaiter_type *(*initial_suspend)(promise_handle **);
    awaiter_type *(*final_suspend)(promise_handle **);
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

#define LAMBDA_CAPTURE(T, ret, ...) \
    struct T {\
        lambda_handle _handle_;\
        ret _ret_;             \
        __VA_ARGS__\
    };                              \
    typedef struct T T;

#define COROUTINE_CAPTURE(T, ...) \
    struct T {\
        coroutine_handle _handle_;\
        __VA_ARGS__\
    };\
    typedef struct T T;

void IMPL(coroutine_handle, onFree, CO_ARGS(coroutine_handle), {
    if (!this->on_stack) {
        m_free(this,"coroutine");
    }
})

int IMPL(awaiter_type, suspend_always, CO_ARGS(awaiter_type), {
    return 0;
})
int IMPL(awaiter_type, suspend_never, CO_ARGS(awaiter_type), {
    return 1;
})
void IMPL(awaiter_type, empty_suspend, CO_ARGS(awaiter_type, coroutine_handle *co), {})
void IMPL(awaiter_type, empty_resume, CO_ARGS(awaiter_type), {})

void IMPL(awaiter_type, clean_suspend, CO_ARGS(awaiter_type, coroutine_handle *co), {
    if (co) {
        coroutine_handle **ptr = ((coroutine_handle **) ((co->promise)));
        // equal promise->coroutine_handle = NULL;
        *(ptr + sizeof(promise_handle) / sizeof(coroutine_handle *)) = ((void *) 0);
        co->promise = NULL;
        co->onFree(&co);
    }
})

static awaiter_type suspend_always  = {
    .await_ready = &FUNC(awaiter_type, suspend_always),
    .await_suspend = &FUNC(awaiter_type, empty_suspend),
    .await_resume = &FUNC(awaiter_type, empty_resume)
};

static awaiter_type suspend_clean  = {
        .await_ready = &FUNC(awaiter_type, suspend_always),
        .await_suspend = &FUNC(awaiter_type, clean_suspend),
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
        if(awaiter->await_ready(&awaiter) == 0) {\
           awaiter->await_suspend(&awaiter, handle);\
           return CO;                             \
        }\
        case __LINE__:awaiter->await_resume(&awaiter); \
    } while (0);

#define co_yield(T, value) handle->yield_awaiter = T##_await_transform(&CO, value); co_await(handle->yield_awaiter);

#define co_return(T, value) T##_return_value(&CO, value);co_await(handle->final_awaiter);return NULL;
#define co_return_void(T) T##_return_void((promise_handle **)&CO);co_await(handle->final_awaiter);return NULL;

#define ASYNC(T, yield_type, func, ...) \
COROUTINE_CAPTURE(T##CAP##func, __VA_ARGS__) \
T * func(T##CAP##func *this); \
\
void T##_resume_##func (coroutine_handle *_cr_handle) {\
    func((T##CAP##func *)_cr_handle);\
} \
T * func(T##CAP##func *this) {\
    coroutine_handle *handle = &this->_handle_;\
    T *CO = (T *)handle->promise;\
    if (handle->state != 0 && CO == NULL) return NULL;\
    co_begin;\
    {\
        T _temp_ = {{.return_void = &T##_return_void, .initial_suspend = &T##_initial_suspend, .final_suspend = &T##_final_suspend}, handle};\
        handle->promise = m_malloc(sizeof(T), #T); memcpy(handle->promise, &_temp_, sizeof(T));\
        CO = T##_get_return_obj((T **)(&handle->promise));\
        handle->promise = (promise_handle *)CO;\
        handle->init_awaiter = handle->promise->initial_suspend(&handle->promise);\
        handle->final_awaiter = handle->promise->final_suspend(&handle->promise);\
    }\
    co_await(handle->init_awaiter); \

#define ASYNC_END  \
      if (handle->promise && handle->final_awaiter) {co_await(handle->final_awaiter);};}\
return NULL;          \
}

#define CO_INIT(T, name, func, ...) \
    T##CAP##func name##T##_cr_handle_ = {{.state = 0, .on_stack = 1, .resume = T##_resume_##func, .onFree = &FUNC(coroutine_handle, onFree)} __VA_OPT__(,) __VA_ARGS__}; \
    T *name = func(&name##T##_cr_handle_);\

#define CO_NEW(T, name, func, ...) \
    T##CAP##func name##T##_cr_handle_ = {{.state = 0, .on_stack = 0,.resume = T##_resume_##func, .onFree = &FUNC(coroutine_handle, onFree)} __VA_OPT__(,) __VA_ARGS__}; \
    T##CAP##func *name##T##_cr_handle_ptr = m_malloc(sizeof(T##CAP##func), #func);                                      \
    memcpy(name##T##_cr_handle_ptr, &name##T##_cr_handle_, sizeof(T##CAP##func));                               \
    T *name = func(name##T##_cr_handle_ptr);\


// ------------------ lambda --------------------------
#define lambda_type(T, ret, func) ret##T##func##lambda
#define $lambda(T, ret, func, ...)LAMBDA_CAPTURE(lambda_type(T, ret, func), ret __VA_OPT__(,) __VA_ARGS__)

#define $eval(cb, expr) \
do expr while(0); \
((lambda_handle *)cb)->_co_handle_->state = -((lambda_handle *)cb)->callback_state;\
((lambda_handle *)cb)->_co_handle_->resume(((lambda_handle *)cb)->_co_handle_);

#define $cb_ret cb->_ret_
#define cb_return(expr) cb->_ret_ = expr

#define CB_ARGS(T, cb_ret, func, ...) (T ** _this, lambda_type(T, cb_ret, func) *cb __VA_OPT__(,) __VA_ARGS__)
#define CB_IMPL(F_ret, T, func, args, expr, cb_ret, ...) \
    LAMBDA_CAPTURE(lambda_type(T, cb_ret, func), cb_ret __VA_OPT__(,) __VA_ARGS__)\
    F_ret IMPL(T, cb_ret##func, args, {\
         do {expr;}while(0);\
         if (this == NULL) *_this = NULL;\
    })

#define LB_CALL(T, name, func, ret, expr, ...) \
    do {\
        lambda_type(T, ret, func) cb = {{__LINE__, handle}};\
        handle->_cb_handle_ = (lambda_handle *) (&cb);\
        CALL(T, *name, ret##func, &cb __VA_OPT__(,) __VA_ARGS__);\
        handle->state = __LINE__;              \
    } while (0);              \
    do {                      \
    case -__LINE__ :\
        if (handle->state == -__LINE__) {\
            lambda_type(T, ret, func) *cb = (lambda_type(T, ret, func) *)handle->_cb_handle_;\
            do expr while(0);\
            return CO;\
        }\
    } while(0);

#define CB_ASYNC(T, yield_type, func, cb_ret, ...)\
    COROUTINE_CAPTURE(T##CAP##func, lambda_type(T, cb_ret, func) *cb; __VA_ARGS__) \
    void cb_ret##func##lambdaFree(coroutine_handle **handle) {\
          T##CAP##func *_handle = (T##CAP##func *)*handle;    \
          if (_handle->cb) {                       \
            m_free(_handle->cb, "lambda_handle"); \
            _handle->cb = NULL;                       \
          }                                        \
          coroutine_handle_onFree(handle);\
    }                                              \
    T * func(T##CAP##func *this); \
    void T##_resume_##func (coroutine_handle *_cr_handle) {\
        func((T##CAP##func *)_cr_handle);\
    } \
    T * func(T##CAP##func *this) {\
        coroutine_handle *handle = &this->_handle_;\
        T *CO = (T *)handle->promise;\
        lambda_type(T, cb_ret, func) *cb = this->cb;\
        if (handle->state != 0 && CO == NULL) return NULL;\
        co_begin;\
        {\
            T _temp_ = {{.return_void = &T##_return_void, .initial_suspend = &T##_initial_suspend, .final_suspend = &T##_final_suspend}, handle};\
            handle->promise = m_malloc(sizeof(T), #T); memcpy(handle->promise, &_temp_, sizeof(T));\
            CO = T##_get_return_obj((T **)(&handle->promise));\
            handle->promise = (promise_handle *)CO;\
            handle->init_awaiter = handle->promise->initial_suspend(&handle->promise);\
            handle->final_awaiter = handle->promise->final_suspend(&handle->promise);\
        }\
        co_await(handle->init_awaiter);

#define LB_CO_NEW(T, name, func, ret, expr, ...)\
    do {                                            \
        lambda_type(T, ret, func) *func##_cb = m_malloc(sizeof(lambda_type(T, ret, func)), "lambda_handle");\
        func##_cb->_handle_.callback_state = __LINE__;\
        func##_cb->_handle_._co_handle_ = handle;\
        T##CAP##func ret##T##_cr_handle_ = {{.state = 0, .on_stack = 0,.resume = T##_resume_##func, .onFree = &ret##func##lambdaFree}, func##_cb __VA_OPT__(,) __VA_ARGS__}; \
        T##CAP##func *ret##T##_cr_handle_ptr = m_malloc(sizeof(T##CAP##func), #func);                                      \
        memcpy(ret##T##_cr_handle_ptr, &ret##T##_cr_handle_, sizeof(T##CAP##func));                               \
        name = func(ret##T##_cr_handle_ptr);\
        handle->state = __LINE__;              \
    } while (0);\
    do {                      \
    case -__LINE__ :\
        if (handle->state == -__LINE__) {       \
            lambda_type(T, ret, func) *cb = (lambda_type(T, ret, func) *)((T##CAP##func *)name->_co_handle_)->cb;\
            do expr while(0);\
            return CO;\
        }\
    } while(0);

// this->value
#define $(x) this->x

// express variables in the lambda expression
#define $$(x) cb->x

#endif //CCOT_COROUTINE_H
