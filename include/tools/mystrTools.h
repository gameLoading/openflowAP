int strlen_safe(char *str, int maxlen);
int strcat_safe(char *buffer, char *dest, int max_len, int buf_len);
int copyString_m(char *buffer, char *dest, int max_len);
int copyString_a(char buffer[], char *dest, int max_len, int buf_len);
char** freeStrings(char **str, int len);
const char *number2comma(long n);