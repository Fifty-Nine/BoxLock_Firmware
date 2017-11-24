#include "utility.h"

#include <stddef.h>

static const char mapping[256] =
{
    ['a'] = '2',
    ['b'] = '2',
    ['c'] = '2',
    ['d'] = '3',
    ['e'] = '3',
    ['f'] = '3',
    ['g'] = '4',
    ['h'] = '4',
    ['i'] = '4',
    ['j'] = '5',
    ['k'] = '5',
    ['l'] = '5',
    ['m'] = '6',
    ['n'] = '6',
    ['o'] = '6',
    ['p'] = '7',
    ['q'] = '7',
    ['r'] = '7',
    ['s'] = '7',
    ['t'] = '8',
    ['u'] = '8',
    ['v'] = '8',
    ['w'] = '9',
    ['x'] = '9',
    ['y'] = '9',
    ['z'] = '9',
    ['+'] = '*',
    [' '] = '0',
    ['_'] = '0',
};

char mapToPhoneKeypad(const char c)
{
    return mapping[(size_t)c] ?: c;
}
