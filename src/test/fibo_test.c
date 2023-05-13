#include <stdio.h>
#include "IntGenerator.h"

ASYNC(IntGenerator, int, fibonacci, int a; int b;)
        co_yield(IntGenerator, 0)
        co_yield(IntGenerator, 1)

        this->a = 0;
        this->b = 1;
        while (CALL(IntGenerator, this->promise, has_next)) {
            co_yield(IntGenerator, this->a + this->b)
            this->b = this->b + this->a;
            this->a = this->b - this->a;
        }

        co_return(IntGenerator, this->a + this->b)
ASYNC_END

ASYNC(IntGenerator, int, co_main)
        CO_INIT(IntGenerator, gen, fibonacci)

        // for each test
        LAMBDA(IntGenerator, foreach, {
            printf("%d \n", cb->x);
        }, 12);

        co_return(IntGenerator, 0)
ASYNC_END


int main() {
    CO_INIT(IntGenerator, gen, co_main)
    return CALL(IntGenerator, gen, next);
}