/**
library dis

This package provides a function that is able to peek into the bytecode of a
Lily function. This can be installed using Lily's `garden` via:

`garden install github FascinatedBox/dis`
*/

#include "lily_int_opcode.h"
#include "lily_int_code_iter.h"

#include "lily.h"

/** Begin autogen section. **/
const char *lily_dis_table[] = {
    "\0\0"
    ,"F\0dis\0(Function($1)):String"
    ,"Z"
};
#define toplevel_OFFSET 1
void lily_dis__dis(lily_state *);
void *lily_dis_loader(lily_state *s, int id)
{
    switch (id) {
        case toplevel_OFFSET + 0: return lily_dis__dis;
        default: return NULL;
    }
}
/** End autogen section. **/

static const char *opcode_names[] =
{
    "o_assign",
    "o_assign_noref",
    "o_int_add",
    "o_int_minus",
    "o_int_modulo",
    "o_int_multiply",
    "o_int_divide",
    "o_int_left_shift",
    "o_int_right_shift",
    "o_int_bitwise_and",
    "o_int_bitwise_or",
    "o_int_bitwise_xor",
    "o_number_add",
    "o_number_minus",
    "o_number_multiply",
    "o_number_divide",
    "o_compare_eq",
    "o_compare_not_eq",
    "o_compare_greater",
    "o_compare_greater_eq",
    "o_unary_not",
    "o_unary_minus",
    "o_unary_bitwise_not",
    "o_jump",
    "o_jump_if",
    "o_jump_if_not_class",
    "o_for_integer",
    "o_for_setup",
    "o_call_foreign",
    "o_call_native",
    "o_call_register",
    "o_return_value",
    "o_return_unit",
    "o_build_list",
    "o_build_tuple",
    "o_build_hash",
    "o_build_enum",
    "o_subscript_get",
    "o_subscript_set",
    "o_global_get",
    "o_global_set",
    "o_load_readonly",
    "o_load_integer",
    "o_load_boolean",
    "o_load_byte",
    "o_load_empty_variant",
    "o_instance_new",
    "o_get_property_get",
    "o_set_property_set",
    "o_catch_push",
    "o_catch_pop",
    "o_exception_catch",
    "o_exception_store",
    "o_exception_raise",
    "o_closure_get",
    "o_closure_set",
    "o_closure_new",
    "o_closure_function",
    "o_interpolation",
    "o_vm_exit"
};

static void dis(lily_msgbuf *msgbuf, uint16_t *buffer, uint16_t *pos, const char *str)
{
    *pos = *pos + 1;
    char temp[64];
    sprintf(temp, "    [%4.d] %-9s%d\n", *pos, str, buffer[*pos]);
    lily_mb_add(msgbuf, temp);
}

#define ADD_FMT(...) sprintf(fmt_buffer, __VA_ARGS__); lily_mb_add(msgbuf, fmt_buffer);

#define T_VALUE   ADD_FMT("    [%4.d] value:   %d\n",  pos, (int16_t)buffer[pos]);
#define T_INPUT   ADD_FMT("    [%4.d] <------  #%d\n", pos, buffer[pos]);
#define T_OUTPUT  ADD_FMT("    [%4.d] ======>  #%d\n", pos, buffer[pos]);
#define T_JUMP    ADD_FMT("    [%4.d] +>    |  [%d] (%d)\n", pos, ci.offset + (int16_t)buffer[pos], (int16_t)buffer[pos]);
#define T_G_IN    ADD_FMT("    [%4.d] <------  #%d (G)\n", pos, buffer[pos]);
#define T_G_OUT   ADD_FMT("    [%4.d] ======>  #%d (G)\n", pos, buffer[pos]);
#define T_U_IN    ADD_FMT("    [%4.d] <------  #%d (up)\n", pos, buffer[pos]);
#define T_U_OUT   ADD_FMT("    [%4.d] ======>  #%d (up)\n", pos, buffer[pos]);

static void dump_code(lily_msgbuf *msgbuf, lily_function_val *fv)
{
    lily_code_iter ci;
    lily_ci_from_native(&ci, fv);
    char fmt_buffer[64];
    int first_pass = 1;

    while (lily_ci_next(&ci)) {
        uint16_t *buffer = ci.buffer;
        uint16_t pos = ci.offset;
        lily_opcode op = ci.buffer[pos];
        int i;

        if (first_pass == 0)
            lily_mb_add(msgbuf, "\n");
        else
            first_pass = 0;

        lily_mb_add_fmt(msgbuf, "[%d-%d] (%d) %s\n", pos,
                pos + ci.round_total - 1, buffer[pos],
                opcode_names[buffer[pos]]);

        if (ci.special_1) {
            switch (op) {
                case o_jump_if:
                    dis(msgbuf, buffer, &pos, "truthy:");
                    break;
                case o_load_integer:
                case o_load_boolean:
                case o_load_byte:
                    dis(msgbuf, buffer, &pos, "value:");
                    break;
                case o_global_set:
                    pos++;
                    T_G_IN
                    break;
                case o_global_get:
                    pos++;
                    T_G_OUT
                    break;
                case o_call_register:
                    pos++;
                    T_INPUT
                    break;
                case o_call_foreign:
                case o_call_native:
                    dis(msgbuf, buffer, &pos, "func:");
                    break;
                case o_build_variant:
                case o_load_empty_variant:
                    dis(msgbuf, buffer, &pos, "variant:");
                    break;
                case o_exception_catch:
                case o_instance_new:
                    dis(msgbuf, buffer, &pos, "class:");
                    break;
                case o_load_readonly:
                    dis(msgbuf, buffer, &pos, "literal:");
                    break;
                case o_property_get:
                case o_property_set:
                    dis(msgbuf, buffer, &pos, "index:");
                    break;
                case o_closure_set:
                    pos++;
                    T_U_IN
                    break;
                case o_closure_get:
                    pos++;
                    T_U_OUT
                    break;
                case o_closure_new:
                    dis(msgbuf, buffer, &pos, "size:");
                    break;
                default:
                    pos += ci.special_1;
                    break;
            }
        }

        if (ci.counter_2)
            dis(msgbuf, buffer, &pos, "count:");

        for (i = 0;i < ci.inputs_3;i++) {
            pos++;
            T_INPUT
        }

        for (i = 0;i < ci.outputs_4;i++) {
            pos++;
            T_OUTPUT
        }

        for (i = 0;i < ci.jumps_5;i++) {
            pos++;
            T_JUMP
        }
    }
}

/**
define dis(f: Function($1)): String

This receives a function that takes any number of input arguments, and returns a
`String` containing the disassembly of that function. If 'f' is not a native
function, then `"<foreign function>"` is returned instead.
*/
void lily_dis__dis(lily_state *s)
{
    lily_msgbuf *msgbuf = lily_msgbuf_get(s);

    lily_function_val *fv = lily_arg_function(s, 0);

    if (lily_function_is_foreign(fv))
        lily_push_string(s,"<foreign function>");
    else {
        dump_code(msgbuf, fv);
        lily_push_string(s, lily_mb_raw(msgbuf));
    }

    lily_return_top(s);
}
