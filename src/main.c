#include "lab.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef TEST
#define main main_exclude
#endif



int main(void)
{
    int result_add = add(5, 3);
    int result_subtract = subtract(5, 3);
    int result_product = product(5, 3);
    int result_badSum = badSum(1, 1);
    printf("Addition Result: %d\n", result_add);
    printf("Subtraction Result: %d\n", result_subtract);
    printf("Multiplication Result: %d\n", result_product);
    printf("Bad Sum result: %d\n", result_badSum);
    char *greeting = get_greeting("World");
    if (greeting) {
        printf("%s\n", greeting);
        free(greeting); // Free the allocated memory for the greeting
    } else {
        printf("Failed to create greeting.\n");
    }
    return 0;
}