#include <stdio.h>
#include "IntGenerator.h"

ASYNC(IntGenerator, int, fibonacci, int max; int a; int b;)
        co_yield(IntGenerator, 0)
        co_yield(IntGenerator, 1)

        this->a = 0;
        this->b = 1;
        while (CALL(IntGenerator, this->promise, has_next) && this->max -- > 2) {
            co_yield(IntGenerator, this->a + this->b)
            this->b = this->b + this->a;
            this->a = this->b - this->a;
        }

        co_return(IntGenerator, this->a + this->b)
ASYNC_END

char map_c(int x) {
    return x + 'a';
}

char map_c2(int x) {
    return x <= 1 ? '\n' : 'x';
}

ASYNC(IntGenerator, int, co_main)
    CO_INIT(IntGenerator, gen, list, 10)

    // for each test
    LAMBDA(IntGenerator, foreach, {
        printf("cb %d \n", cb->x);
    }, 3);

    CO_LAMBDA(IntGenerator, gen_2, map, {
        cb_return(cb->x + 'a' - 1);
    }, gen);

    while (CALL(IntGenerator, gen_2, has_next)) {
        int x = CALL(IntGenerator, gen_2, next);
        if (CALL(IntGenerator, gen_2, has_next))
            printf("A-%c\n", x);
    }

    CO_INIT(IntGenerator, gen_4, list, 9)
    // flat map test
    CO_LAMBDA(IntGenerator, gen_3, flat_map, {
        cb_return(cb->x + '0');
    }, gen_4);

    while (CALL(IntGenerator, gen_3, has_next)) {
        char res = CALL(IntGenerator, gen_3, next);
        if (CALL(IntGenerator, gen_3, has_next)) {
            printf("-%c", res);
        }
    }
    co_return(IntGenerator, 0)
ASYNC_END


int main() {
    CO_INIT(IntGenerator, gen, co_main)
    CALL(IntGenerator, gen, next);
    return 0;
}