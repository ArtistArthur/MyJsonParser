#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
};
typedef enum {
    LEPT_NULL,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT,
    LEPT_PARSE_STRING_BAD_ESCAPE,
} lept_type;

typedef struct {
    union{
        double n;//add number member for value
        struct {
            char* s;
            size_t len;
        };
    };
    lept_type type;
}lept_value;

typedef struct {
    const char *json;
    char* stack;
    size_t size;
    size_t top;
} lept_context; //json串

#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

int lept_parse(lept_value* v, const char* json);
lept_type lept_get_type(const lept_value* v);
double lept_get_number(const lept_value *v);
void lept_set_number(lept_value* v, double n);
int lept_get_boolean(lept_value* v);
void lept_set_boolean(lept_value* v, lept_type type);
size_t lept_get_string_length(lept_value* v);
const char* lept_get_string(lept_value* v);
void lept_free(lept_value* v);

#endif /* LEPTJSON_H__ */
