#include <stdio.h>
#include <stdlib.h>

__attribute__((used))
void helper() {
    asm("jmp *%rsp");
}

void vuln() {
    printf("Dear diary...\n");
    char input[256] = { 0 };
    fgets(input, 1024, stdin);
}

int main() {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    vuln();
}