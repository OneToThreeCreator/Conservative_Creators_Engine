#include <stdio.h>
#include <coffeechain/engine_common.h>

uint8_t truthTableTest (void)
{
   char *strings[5] = {"a", "!a", "a & b", "a & b & !c", "a & b | c"};
   uint64_t correctAnswers[5] = {0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x8888888888888888, 0x4040404040404040, 0xEAEAEAEAEAEAEAEA};
   uint_fast16_t *operations;
   uint8_t i = 0;
   while (i < 5)
   {
      operations = cceParseStringToLogicOperations(strings[i], NULL);
      printf("Operations for %s: 0x%lx\n", strings[i], *operations);
      free(operations);
      ++i;
   }
   return i / 4;
}

int main (void)
{
   truthTableTest();
   return 0;
}
