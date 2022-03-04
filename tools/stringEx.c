#include <assert.h>
#include <string.h>
#include <malloc.h>

int strncpy_ex(char *buffer, char *dest, int max_len){
    // assert(sizeof(buffer)<max_len);
    if (buffer == NULL || dest == NULL){
        printf("buffer or dest is null\n");
        return -1;
    }
    int str_len = strnlen(dest, max_len);
    dest[str_len-1] = 0;
    int buf_len = malloc_usable_size(buffer);
    if (buf_len < str_len) {
        printf("buffer size : %d but dest len : %d\n", buf_len, str_len);
        return -1;
    }
    strncpy(buffer, dest, str_len);
    buffer[str_len-1] = 0;
    return 0;
}