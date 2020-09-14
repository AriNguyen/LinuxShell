#include <stdio.h>
#include <string.h>

int main() {
    char buf[1024];
    while (1) {
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            break;
        }
        printf("buf: %s", buf);
        if (strstr(buf, "exit") != NULL) {
            break;
        }
    }
    return 0;
}