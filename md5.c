// gcc -o md5 -Werror -Wall -Wextra -lcrypto -lncurses term.c md5.c

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include <openssl/evp.h>

#include "md5.h"

#define PASSWORD_MAX_LENGTH 128
#define CHAR_DEL 127

static bool read_password(char *dest);

static void str2md5(const char *str, char *out)
{
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    const EVP_MD *md = EVP_md5();
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;

    EVP_DigestInit_ex2(context, md, NULL);
    EVP_DigestUpdate(context, str, strlen(str));
    EVP_DigestFinal_ex(context, md_value, &md_len);
    EVP_MD_CTX_free(context);

    for (unsigned int n = 0; n < md_len; n++)
        snprintf(&(out[n * 2]), 16 * 2, "%02x", md_value[n]);
}

static bool ask_restart(char *dest)
{
    printf("Press ENTER to leave or BACKSPACE to restart.\n");
    int c = blocked_read();

    if (c == '\b' || c == CHAR_DEL)
        return read_password(dest);
    else if (c == '\n')
        return false;
    else
        return ask_restart(dest);
}

static bool read_password(char *dest)
{
    printf("Enter the password:\n");

    for (int i = 0; i < PASSWORD_MAX_LENGTH; i++)
    {
        const char c = blocked_read();

        if (c == 0)
            return true;

        if (c == '\b' || c == CHAR_DEL)
        {
            if (i > 0)
            {
                dest[i - 1] = '\0';
                i -= (i > 1) + 1;
                printf("\b \b");
            }
        }
        else if (c != '\n')
        {
            dest[i] = c;
            printf("*");
        }
        else
        {
            dest[i] = '\0';
            printf("\n");
            return true;
        }

        if (fflush(stdout))
            return false;
    }

    printf("\nPassword too long !\n");
    return ask_restart(dest);
}

static size_t get_words_size(int size, const char **list)
{
    size_t len = size - 1;

    for (int i = 0; i < size; i++)
        len += strlen(list[i]);
    return len;
}

static char *tab_to_str(int size, const char **tab)
{
    size_t len = get_words_size(size, tab);
    if (len == 0)
        return NULL;
    char *str = malloc(sizeof(char) * len + 1);

    if (!str)
        return NULL;
    str[0] = '\0';

    for (int i = 0; i < size; i++)
    {
        strcat(str, tab[i]);
        if (i != size - 1)
            strcat(str, " ");
    }
    return str;
}

int main(const int argc, const char **argv)
{
    char output[33];

    if (argc <= 1)
    {
        char password[PASSWORD_MAX_LENGTH] = "\0";

        if (!select_canonical_mode(true))
            return 1;
        if (!read_password(password))
        {
            select_canonical_mode(false);
            return 1;
        }
        if (!select_canonical_mode(false))
            return 1;

        if (password[0] == '\0')
        {
            printf("Empty password !\n");
            return 2;
        }
        str2md5(password, output);
    }
    else
    {
        char *password = tab_to_str(argc - 1, argv + 1);

        if (!password)
            return 1;
        str2md5(password, output);
        free(password);
    }

    printf("%s\n", output);
    return 0;
}