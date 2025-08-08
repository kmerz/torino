#include <stdio.h>

/* Declare a buffer for user input of size 2048 */
static char input[2028];

int main(int argc, char** argv) {
  
  /* Print Version and Exit Information */
  puts("Torino Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

  while(1) {
    fputs("torino> ", stdout);
    fgets(input, 2048, stdin);

    printf("No, you are a %s", input);
  }

  return 0;
}
