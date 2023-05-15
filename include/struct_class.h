//
// Created by NANAnoo on 5/13/2023.
//

#ifndef CCOT_STRUCT_CLASS_H
#define CCOT_STRUCT_CLASS_H

#define CO_ARGS(T, ...) (T ** _this __VA_OPT__(,) __VA_ARGS__)

#define FUNC(T, name) T##_##name

#define DECLARE(T, FUNC, args) T##_##FUNC args;

#define IMPL(T, FUNC, args, expr)   \
        __attribute__((unused)) T##_##FUNC args {        \
        T *this = (T *)(*_this); \
        do {expr;}while(0);         \
        }   \

#define CALL(T, _this, FUNC, ...) T##_##FUNC(&_this __VA_OPT__(,) __VA_ARGS__)
#endif //CCOT_STRUCT_CLASS_H
