#include <stdio.h>
#include "IntGenerator.h"

ASYNC(IntGenerator, int, fibonacci, int step; int a; int b;)
        co_yield(IntGenerator, 0)
        co_yield(IntGenerator, 1)
        this->a = 0;
        this->b = 1;
        while (CALL(IntGenerator, CO, has_next) && this->step -- > 0) {
            co_yield(IntGenerator, this->a + this->b)
            this->b = this->b + this->a;
            this->a = this->b - this->a;
        }
        co_return(IntGenerator, this->a + this->b)
ASYNC_END

ASYNC(IntGenerator, int, co_main, IntGenerator *gen2;IntGenerator *gen4;)
    CO_NEW(IntGenerator, gen, list, 1, 5)

    // for each test
    LB_CALL(IntGenerator, &gen, foreach, int, {
        printf("cb %d \n", cb->x);
    }, 3)

    if (gen) {
        // map test,
        LB_CO_NEW(IntGenerator, $$(gen2), map, char, {
            cb_return(cb->x + 'a');
        }, gen)

        // consume gen until no output
        while (CALL(IntGenerator, $$(gen2), has_next)) {
            printf("A-%c\n", CALL(IntGenerator, $$(gen2), next));
        }
    }

    // flat map, chain call
    // gen_3
    CO_NEW(IntGenerator, gen_3, fibonacci, 5)
    // -> flat_map( (x) -> return fibonacci(x) )
    LB_CO_NEW(IntGenerator, $$(gen4), flat_map, IntGeneratorRef, {
        CO_NEW(IntGenerator, temp, list, 0, cb->x);
        cb_return(temp);
    }, gen_3)
    // -> map( (x) -> return x != 0 ? 'x' : '\n' )
    LB_CO_NEW(IntGenerator, $$(gen2), map, char, {
        if (cb->x == 0) printf("\nfibonacci : ");
        cb_return(cb->x != 0 ? 'x' : '\n');
    }, $$(gen4))
    // -> foreach( (x) -> printf("%c", x) )
    LB_CALL(IntGenerator, &$$(gen2), foreach, int, {
        printf("%c ", cb->x);
    }, 1000)

    co_return(IntGenerator, 0)
ASYNC_END


int main() {
    CO_INIT(IntGenerator, gen, co_main)
    while(gen) CALL(IntGenerator, gen, next);
    return 0;
}