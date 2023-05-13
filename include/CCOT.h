//
// Created by NANAnoo on 5/13/2023.
//

#ifndef CCOT_CCOT_H
#define CCOT_CCOT_H

typedef struct CR_Handle CRHandle;
struct CR_Handle {
    int state;
    void (*resume)(CRHandle *);
};

#define Await_struct(name, ...) \
    struct name {\
        CRHandle *_cr_handle;\
        __VA_ARGS__\
    };\
    typedef struct name name;

#define CR_struct(T, ...) \
    struct T {\
        CRHandle _cr_handle;\
        __VA_ARGS__\
    };\
    typedef struct T T;

#define crBegin switch(handle->state) { case 0:
#define co_await(expr) do { CO.expr; handle->state=__LINE__; return &CO; case __LINE__:this = handle; } while (0)
#define co_return handle->state=-1; return NULL;

#define CR(T, func, ...) \
CR_struct(T##CO##func __VA_OPT__(,) __VA_ARGS__) \
T * func(T *_co_obj_); \
void T##_resume_##func (CRHandle *_cr_handle) {\
    func((T *)_cr_handle); \
} \
T * func(T *_co_obj_) {\
    CRHandle *handle = _co_obj_->_cr_handle;     \
    T##CO##func *this = (T##CO##func *)handle; \
    crBegin;\


#define CR_INIT(T, name, func, ...) \
    T##CO##func C##T##_cr_handle_ = {{.state = 0, .resume = T##_resume_##func} __VA_OPT__(,) __VA_ARGS__}; \
    T name = {._cr_handle = (CRHandle *)(&C##T##_cr_handle_)};\
    name

#define CR_NEW(T, name, func, ...) \
    T##CO##func C##T##_cr_handle_ = {{.state = 0, .resume = T##_resume_##func} __VA_OPT__(,) __VA_ARGS__}; \
    T##CO##func *C##T##_cr_handle_ptr = malloc(sizeof(T##CO##func));                                       \
    memcpy(C##T##_cr_handle_ptr, &C##T##_cr_handle_, sizeof(T##CO##func));                               \
    T *name = malloc(sizeof(T));   \
    name->_cr_handle = (CRHandle *)C##T##_cr_handle_ptr;                                             \
    name

#define CREND };return NULL;}

#define CO (*_co_obj_)

#endif //CCOT_CCOT_H
