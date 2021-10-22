#include "leptjson.h"
#include <assert.h> /* assert() */
#include <stdlib.h> /* NULL strtod() */
#include <stdio.h>
#include<string.h>
#include<errno.h>//errno ERANGE
#include<math.h>//HUGE_VAL

#define EXPECT(c, ch)             \
    do                            \
    {                             \
        assert(c->json[0] == ch); \
        c->json++;                \
    }while(0)
#define ISDIGIT(ch) ((ch)>='0'&&(ch)<='9')


lept_type lept_get_type(const lept_value *v)
{
    return v->type;
}


void lept_free(lept_value* v)
{
    if(v == NULL) {
        return;
    }
    if(v->type != LEPT_STRING)
    {
        return;
    }
    if(v->s == NULL)
    {
        return;
    }
    free(v->s);
    v->type = LEPT_NULL;
}

void* lept_context_push(lept_context* ctx, size_t size) {
    assert(ctx != NULL);
    assert(size > 0);
    if(ctx->top + size >= ctx->size) {
        ctx->size = LEPT_PARSE_STACK_INIT_SIZE;
        while(ctx->top + size >= ctx->size) {
            ctx->size += ctx->size >> 1;
        }
        //realloc会cpy原数据
        ctx->stack = (char*)realloc(ctx->stack, ctx->size);
    }
    void *ret = ctx->stack + ctx->top;
    ctx->top += size;
    return ret;
}

void* lept_context_pop(lept_context* ctx, size_t size) {
    assert(ctx->top >= size);
    ctx->top -= size;
    return ctx->stack + ctx->top;
}

void lept_put(lept_context* c, char ch) {
    char* p = (char*)lept_context_push(c, sizeof(ch));
    *p = ch;
    return;
}

size_t lept_get_string_length(lept_value* v) {
    assert(v != NULL);
    assert(v->type == LEPT_STRING);
    return v->len;
}

const char* lept_get_string(lept_value* v) {
    assert(v != NULL);
    assert(v->type == LEPT_STRING);
    return v->s;
}

void lept_set_string(lept_value* v, const char* s, size_t len)
{
    assert(v != NULL&& (s!=NULL || len == 0));
    lept_free(v);
    v->s = (char*)malloc(len + 1);
    memcpy(v->s, s, len);
    v->s[len] = '\0';
    v->len = len;
    v->type = LEPT_STRING;
}

double lept_get_number(const lept_value *v)
{
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

void lept_set_number(lept_value* v, double n) {
    assert(v != NULL);
    assert(v->type == LEPT_NUMBER);
    v->n = n;
}

int lept_get_boolean(lept_value* v) {
    assert(v != NULL);
    return v->type == LEPT_TRUE ? LEPT_TRUE : LEPT_FALSE;
}

void lept_set_boolean(lept_value* v, lept_type type) {
    assert(v != NULL);
    v->type = type;
    return;
}


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

int lept_parse_string(lept_context* c, lept_value* v) {
    assert(v != NULL);
    const char* p = c->json;
    c->json++;
    size_t head = c->top;
    size_t len;
    assert(*p == '\"');
    p++;
    while(1) {
        
    char ch = *p++; 
    switch(ch) {
        case '\\':
            ch = *p++;
            switch (ch)
            {
            case 'b':
                lept_put(c, '\b');
                break;
            case 't':
                lept_put(c, '\t');
                break;
            case 'n':
                lept_put(c, '\n');
                break;
            case '\\':
                lept_put(c, '\\');
                break;
            case 'r':
                lept_put(c, '\r');
                break;
            case '/':
                lept_put(c, '/');
                break;
            case 'f':
                lept_put(c, '\f');
                break;
            case '\"':
                lept_put(c, '\"');
                break;
            default:
                c->top = head;
                return LEPT_PARSE_STRING_BAD_ESCAPE;
            }
            break;
        case '\"':
            c->json = p;
            len = c->top - head;
            lept_set_string(v, (const char*)lept_context_pop(c, len), len);
            return  LEPT_PARSE_OK;
        case '\0':
            return LEPT_PARSE_MISS_QUOTATION_MARK;
        default:
            lept_put(c, ch);
    }
    }
}
int lept_parse_number(lept_context*c, lept_value*v)
{
    assert(v != NULL);
    const char *p = c->json;
    if(p[0]=='-')p++;
    if(!ISDIGIT(p[0])) return LEPT_PARSE_INVALID_VALUE;
    if(p[0]=='0')p++;
    else {
        for (p++; ISDIGIT(*p);p++)
            ;
    }
    if(p[0]=='.')
    {
        p++;
        if(!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p);p++)
            ;
           
    }

    if (*p == 'e' || *p == 'E')
    {
        p++;
        if(*p=='+'||*p=='-')
            p++;
        if(!ISDIGIT(*p))
            return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p);p++)
            ;
    }
    errno = 0;
    v->n = strtod(c->json, NULL);
    if(errno==ERANGE&&v->n==HUGE_VAL)//strtod()会在遇到溢出错误时把errno置为ERANGE，并返回HUGE_VAL
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = p;
    v->type = LEPT_NUMBER;
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
    case '\"':
        return lept_parse_string(c, v);
    default:
        return lept_parse_number(c,v);
    }
}

int lept_parse(lept_value *v, const char *json) //解析函数
{
    assert(v != NULL);
    lept_context c;
    c.json = json;
    c.stack = NULL;
    c.top = 0;
    c.size = 0;
    lept_init(v);
    lept_parse_white_space(&c);
    int ret;
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK)
    {
        lept_parse_white_space(&c);
        if (c.json[0] != '\0')
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}
