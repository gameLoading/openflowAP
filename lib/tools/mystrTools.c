#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <syslog.h>
#include "tools/mystrTools.h"
#include "tools/myleak_detector_c.h"
// #include "tools/mymemory.h"

int strlen_safe(char *str, int maxlen){
    int i=0;
    for (;str[i]!=0;i++){
        if (i >= maxlen) return i;
    }
    return i;
}
int strcat_safe(char *buffer, char *dest, int max_len, int buf_len){
    if (buffer == NULL || dest == NULL){
        printf("buffer or dest is null\n");
        return -1;
    }
    int str_len = strlen_safe(dest, max_len);
    if (str_len == -1){
        printf("this str is longer than %d\n", max_len);
        return -1;
    }
    if (str_len < buf_len) dest[str_len] = 0;
    printf("%s %d\n", dest, str_len);

    if (buf_len < str_len) {
        openlog("ofproto.c", LOG_CONS, LOG_USER);
	    syslog(LOG_INFO, "strcat buffer size : %d but dest len : %d\n", buf_len, str_len);
	    closelog();
        printf("strcat buffer size : %d but dest len : %d\n", buf_len, str_len);
        return -1;
    }

    strncat(buffer, dest, str_len);
    return 0;
}

int copyString_m(char *buffer, char *dest, int max_len){
    if (buffer == NULL || dest == NULL){
        printf("buffer or dest is null\n");
        return -1;
    }
    int str_len = strlen_safe(dest, max_len);
    if (str_len == -1){
        printf("this str is longer than %d\n", max_len);
        return -1;
    }
    dest[str_len-1] = 0;
    int buf_len = malloc_usable_size(buffer);

    if (buf_len < str_len) {
        openlog("ofproto.c", LOG_CONS, LOG_USER);
	    syslog(LOG_INFO, "buffer size : %d but dest len : %d\n", buf_len, str_len);
	    closelog();
        printf("buffer size : %d but dest len : %d\n", buf_len, str_len);
        return -1;
    }

    strncpy(buffer, dest, str_len);
    buffer[str_len-1] = 0;
    return 0;
}

int copyString_a(char buffer[], char *dest, int max_len, int buf_len){
    if (buffer == NULL || dest == NULL){
        printf("buffer or dest is null\n");
        return -1;
    }
    int str_len = strlen_safe(dest, max_len);
    openlog("ofproto.c", LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "str len : %d\n", str_len);
	closelog();
    if (buf_len < str_len) {
        openlog("ofproto.c", LOG_CONS, LOG_USER);
	    syslog(LOG_INFO, "buffer size : %d but dest len : %d\n", buf_len, str_len);
	    closelog();
        printf("buffer size : %d but dest len : %d\n", buf_len, str_len);
        return -1;
    }

    strncpy(buffer, dest, str_len);
    if (str_len < buf_len) buffer[str_len] = 0;
    return 0;
}

// int strReplace(char *dest, char *find, char *to){
//     char *ptr = strstr(dest, find);
//     if (ptr == NULL) return -1;

//     strncpy(ptr, )
// }

char** freeStrings(char **str, int len){
    if (str == NULL) return NULL;
    for (int i=0;i < len;i++){
        memset(str[i], 0, malloc_usable_size(str[i]));
        if(str[i]!=NULL) free(str[i]);
        str[i] = NULL;
    }
    memset(str, 0, malloc_usable_size(str));
    free(str);
    return NULL;
}

const char *number2comma(long n)
{
    static char comma_str[128];
    char   str[128];
    int    idx, len, cidx = 0, mod;
    
    sprintf(str, "%ld", n);
    len = strlen(str);
    mod = len % 3;
    
    for(idx = 0; idx < len; idx++) {
        if(idx != 0 && (idx) % 3 == mod) {
            comma_str[cidx++] = ',';
        }
        comma_str[cidx++] = str[idx];
    }
    
    comma_str[cidx] = 0x00;
    return comma_str;
}