#ifndef BPLUS_TYPES_H
#define BPLUS_TYPES_H

#include <string.h>
#include <cstring>
#include <algorithm>

namespace bplus{
    #define BPLUS_ORDER 50

    struct value_t{
        char name[256];
        int value;
        char data[256];
    };

    struct key_t {
        char k[16];
    
        key_t(const char *str = "")
        {
            memset(k, 0, sizeof(k));
            strncpy(k, str, sizeof(k));
        }
    };
    
    inline int keyCmp(const key_t& a, const key_t& b){
        int diff = strlen(a.k) - strlen(b.k);
        return diff == 0 ? strcmp(a.k, b.k) : diff;
    }

    #define OPERATOR_RELOAD(type) \
        bool operator< (const key_t &l, const type &r) {\
            return keyCmp(l, r.key) < 0;\
        }\
        bool operator< (const type &l, const key_t &r) {\
            return keyCmp(l.key, r) < 0;\
        }\
        bool operator== (const key_t &l, const type &r) {\
            return keyCmp(l, r.key) == 0;\
        }\
        bool operator== (const type &l, const key_t &r) {\
            return keyCmp(l.key, r) == 0;\
        }

    // overload operator key_t compare key_t
    #define KEYCMP_OPERATOR_RELOAD \
        bool operator< (key_t& left, const key_t& right){ \
            return keyCmp(left, right) < 0; \
        } \
        bool operator< (const key_t& left, key_t& right){ \
            return keyCmp(left, right) < 0; \
        } \
        bool operator== (key_t& left, const key_t& right){ \
            return keyCmp(left, right) == 0; \
        } \
        bool operator== (const key_t& left, key_t& right){ \
            return keyCmp(left, right) == 0; \
        }
}

#endif