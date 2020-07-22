#include "leptjson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* NULL */
#include <stdio.h>

#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(c->json[0] == ch); \
        c->json++;                \
    }while(0)
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
        chrp++;//消除空白符，有空格，制表符，回车符，换行符，是json标准规定的四个空白值
    c->json = chrp;
    return;
}

int lept_parse_literal(lept_context *c,lept_value *v,const char *literal,lept_type type)
{
    EXPECT(c, literal[0]);
    for (int i = 0; literal[i+1]!='\0';i++)
    {
        if(c->json[0]!=literal[i+1])
        {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c->json++;
    }
    v->type = type;
    return LEPT_PARSE_OK;
}

int lept_parse_value(lept_context *c, lept_value *v)
{
    switch (*c->json)
    {
    case 'n':
        return lept_parse_literal(c,v,"null",LEPT_NULL);

    case 'f':
        return lept_parse_literal(c, v, "false", LEPT_FALSE);

    case 't':
        return lept_parse_literal(c, v,"true",LEPT_TRUE);

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
