#include <stdio.h>
#include <stdlib.h>

void win() {
    system("/bin/sh");
}

void vuln() {
    char input[256] = { 0 };
    fgets(input, sizeof(input), stdin);
    printf("Hello, ");
    printf(input);
    printf("\n");
}

int main() {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    vuln();
    exit(0);
}