#include "leptjson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* NULL */
#include <stdio.h>

lept_type lept_get_type(const lept_value *v)
{
    return v->type;
}

double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
typedef struct
{
    const char *json;
} lept_context; //json串

void lept_parse_white_space(lept_context *c)
{
    const char *chrp = c->json;
    while (*chrp == ' ' || *chrp == '\t' || *chrp == '\r' || *chrp == '\n')
        chrp++;
    c->json = chrp;
    return;
}
int lept_parse_null(lept_context *c, lept_value *v)
{
    assert(*c->json == 'n');
    if (c->json[0] != 'n' || c->json[1] != 'u' || c->json[2] != 'l' || c->json[3] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
int lept_parse_false(lept_context *c, lept_value *v)
{
    assert(*c->json == 'f');
    if (c->json[0] != 'f' || c->json[1] != 'a' || c->json[2] != 'l' || c->json[3] != 's' || c->json[4] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 5;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}
int lept_parse_true(lept_context *c, lept_value *v)
{
    assert(*c->json == 't');
    if (c->json[0] != 't' || c->json[1] != 'r' || c->json[2] != 'u' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

int lept_parse_value(lept_context *c, lept_value *v)
{
    switch (*c->json)
    {
    case 'n':
        return lept_parse_null(c, v);

    case 'f':
        return lept_parse_false(c, v);

    case 't':
        return lept_parse_true(c, v);

    case '\0':

        return LEPT_PARSE_EXPECT_VALUE;
    default:
        return LEPT_PARSE_INVALID_VALUE;
    }
}
int lept_parse(lept_value *v, const char *json) //解析函数
{
    lept_context c;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL; //所有解析函数若parse失败都直接返回失败码，没有置节点值，提前置好
    lept_parse_white_space(&c);
    int ret;
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
    {
        lept_parse_white_space(&c);
        if (c.json[0] != '\0')
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}
