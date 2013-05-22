///////////////////////////////////////////////////////////////////////////////
// name: pwdgen
// function: generate password
// author: Eric Knox
// usage: pwdgen -p pattern [-t times]
// pattern:
//      1. between "[]" randomly generates one character, with "*?" defines
//         repetitional times.
//      2. between "()" group a sub-pattern, with "*?" defines repetitional
//         times.
//      3. "-" in "[]" as A to B.
//      4. rest of the characters in pattern printed directly.
// example:
//      ~$ pwdgen -p ([A-Za-z][0-9])*3
//      s6B3i8
// caution:
//      1. repeat 0 time is not allowed, it will print itself as ordinary
//         characters.
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/*
 * function: check
 * description: check if password definition is legal or not.
 * parameters:  in ipt: pointer to string
 * return: 1 for legal, or 0
 */
int check (char *ipt)
{
        int in = 0, level = 0;

        for (; *ipt != '\0'; ++ipt) {
                if (*ipt < '!' || *ipt > '~')
                        return 0;
                switch (*ipt) {
                        case '[':
                                if (in != 0) return 0;
                                else in = 1;
                                break;
                        case ']':
                                if (in != 1 || *(ipt - 1) == ']') return 0;
                                else in = 0;
                                break;
                        case '(':
                                if (in == 1) return 0;
                                ++level;
                                break;
                        case ')':
                                if (in != 0 || level <= 0) return 0;
                                --level;
                                break;
                        case '-':
                                if (in != 0 && (*(ipt - 1) == '[' || *(ipt + 1) == ']'
                                        || *(ipt + 1) < *(ipt - 1))) return 0;
                                break;
                        default:
                                break;
                }
        }
        return in == 0 && level == 0;
}

/*
 * function: strfind
 * description: find target character.
 * parameters:  in tar: target character
 *              in lst: pointer to the starting point of string
 *              in len: search length
 * return: 1 for found, or 0
 */
int strfind(char tar, char *lst, int len)
{
        char *lst_origin = lst;
        
        for (; lst - lst_origin <= len && *lst != tar; ++lst);
        return lst - lst_origin <= len;
}

/*
 * function: randchar
 * description: generate one random character based on definition.
 * parameters: in_out ipt: pointer to string
 * return: length processed(-1)
 */
int randchar(char *ipt)
{
        int c, len = 0;
        char chars[256] = {0};
        char* ipt_origin = ipt;
        
        for (; *ipt != ']'; ++ipt) {
                if (*ipt == '-') {
                        for (c = *(ipt - 1) + 1; c <= *(ipt + 1); ++c)
                                if (!strfind(c, chars, len))
                                        chars[len++] = c;
                        ++ipt;
                }
                else
                        if (!strfind(*ipt, chars, len))
                                chars[len++] = *ipt;
        }

        if (*(ipt + 1) == '*' && *(ipt + 2) > '0' && *(ipt + 2) <= '9')
                c = strtol(ipt += 2, &ipt, 10);
        else {
                c = 1;
                ++ipt;
        }
        
        while (c--) putchar(chars[rand() % len]);
        return ipt - ipt_origin - 1;
}

/*
 * function: generate
 * description: generate a password string.
 * parameters: in_out ipt: pointer to string
 * return: length processed(-1)
 */
int generate(char *ipt)
{
        int t = 0, time = 0;
        char *ipt_origin = ipt, *ipt_exit = 0;
        
        for (; *ipt != '\0'; ++ipt) {
                switch (*ipt) {
                        case '(':
                                ++ipt;
                                ipt += generate(ipt);
                                break;
                        case ')':
                                if (time == 0)
                                        if (*(ipt + 1) == '*' && *(ipt + 2) > '0' && *(ipt + 2) <= '9')
                                                time = strtol(ipt += 2, &ipt_exit, 10);
                                        else {
                                                ++ipt;
                                                return ipt - ipt_origin;
                                        }
                                if (++t < time) ipt = ipt_origin - 1;
                                else return ipt_exit - ipt_origin - 1;
                                break;
                        case '[':
                                ipt += randchar(++ipt);
                                break;
                        default:
                                putchar(*ipt);
                                break;
                }
        }
        return ipt - ipt_origin;
}

// entry point
#define FLAG_DEFINITION         0x1
#define FLAG_TIME               0x2
int main(int argc, char **argv)
{
        int flags = 0, times = 0, c = 0;
        char *ipt, *ipt_origin;
        
        while ((c = getopt(argc, argv, "p:t:")) != -1)
                switch (c) {
                        case 'p':
                                flags |= FLAG_DEFINITION;
                                ipt_origin = optarg;
                                ipt = ipt_origin;
                                break;
                        case 't':
                                flags |= FLAG_TIME;
                                times = strtol(optarg, NULL, 10);
                                break;
                        default: break;
                }
        
        if (!(flags & FLAG_DEFINITION)) {
                printf("Need password definition.\n");
                return -1;
        }
        if (!(flags & FLAG_TIME) || times == 0)
                times = 1;

        if (!check(ipt)) {
                printf("Password definition error.\n");
                return -2;
        }
        
        srand((int)time(NULL));
        while (times--) {
                ipt = ipt_origin;
                generate(ipt);
                putchar('\n');
        }
        
        return 0;
}
