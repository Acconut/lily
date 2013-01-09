#include "lily_impl.h"
#include "lily_symtab.h"
#include "lily_opcode.h"

static char *typename(lily_sym *sym)
{
    char *ret;

    if (sym->flags & VAR_SYM)
        ret = "var";
    else if (sym->flags & LITERAL_SYM)
        ret = "literal";
    else if (sym->flags & STORAGE_SYM)
        ret = "storage";
    else
        ret = NULL;

    return ret;
}

static void print_one(uintptr_t *code, char *str, int i)
{
    lily_sym *left;

    lily_impl_debugf(str, i);
    left = ((lily_sym *)code[i+1]);
    lily_impl_debugf("[%s #%d]\n", typename((lily_sym *)left), left->id);
}

static void print_two(uintptr_t *code, char *str, int i)
{
    lily_sym *left, *right;

    lily_impl_debugf(str, i);
    left = ((lily_sym *)code[i+2]);
    right = ((lily_sym *)code[i+3]);
    lily_impl_debugf("[%s #%d] [%s #%d]\n",
                     typename((lily_sym *)left), left->id,
                     typename((lily_sym *)right), right->id);
}

static void print_three(uintptr_t *code, char *str, int i)
{
    lily_sym *left, *right, *result;

    lily_impl_debugf(str, i);
    left = ((lily_sym *)code[i+2]);
    right = ((lily_sym *)code[i+3]);
    result = ((lily_sym *)code[i+4]);
    lily_impl_debugf("[%s #%d] [%s #%d] [%s #%d]\n",
                     typename((lily_sym *)left), left->id,
                     typename((lily_sym *)right), right->id,
                     typename((lily_sym *)result), result->id);
}

static void print_call(uintptr_t *code, char *str, int i)
{
    int j;
    lily_sym *ret;
    lily_var *var;

    /* For methods, this is:   var, method_val, #args, ret, args
       For functions, this is: var, func_ptr,   #args, ret, args */

    var = (lily_var *)code[i+2];
    lily_impl_debugf(str, i);

    if (var->line_num == 0)
        lily_impl_debugf("name: (builtin) %s.\n        args:\n",
                         var->name, code[i+4]);
    else
        lily_impl_debugf("name: %s (from line %d).\n        args:\n",
                         var->name, var->line_num, code[i+4]);

    for (j = 0;j < code[i+4];j++) {
        lily_sym *sym = (lily_sym *)code[i+6+j];
        lily_impl_debugf("            #%d: %s #%d\n", j+1,
                         typename(sym), sym->id);
    }

    ret = (lily_sym *)code[i+5];
    if (ret != NULL)
        lily_impl_debugf("        return: %s #%d\n", typename(ret), ret->id);
    else
        lily_impl_debugf("        return: nil\n");
}

static void print_save(uintptr_t *code, char *str, int i)
{
    lily_impl_debugf(str, i, code[i+1]);
    int j, k;
    lily_var *v;

    for (j = 0, k = code[i+1];j < k;j++) {
        lily_sym *sym = (lily_sym *)code[i+2+j];
        if (sym->flags & VAR_SYM) {
            v = (lily_var *)sym;
            lily_impl_debugf("        #%d: var %s from line %d.\n", j+1,
                             v->name, v->line_num);
        }
        else
            lily_impl_debugf("        #%d: %s #%d.\n", j+1, typename(sym), sym->id);
    }
}

static void show_code(lily_var *var)
{
    int i, len;
    uintptr_t *code;
    lily_method_val *m;
    lily_sym *left;

    m = (lily_method_val *)var->value.ptr;
    i = 0;
    code = m->code;
    len = m->pos;

    while (i < len) {
        switch (code[i]) {
            case o_assign:
                print_two(code, "    [%d] assign        ", i);
                i += 4;
                break;
            case o_obj_assign:
                print_two(code, "    [%d] obj_assign    ", i);
                i += 4;
                break;
            case o_str_assign:
                print_two(code, "    [%d] str_assign    ", i);
                i += 4;
                break;
            case o_integer_add:
                print_three(code, "    [%d] integer_add   ", i);
                i += 5;
                break;
            case o_integer_minus:
                print_three(code, "    [%d] integer_minus ", i);
                i += 5;
                break;
            case o_number_add:
                print_three(code, "    [%d] number_add    ", i);
                i += 5;
                break;
            case o_number_minus:
                print_three(code, "    [%d] number_minus  ", i);
                i += 5;
                break;
            case o_less:
                print_three(code, "    [%d] less          ", i);
                i += 5;
                break;
            case o_less_eq:
                print_three(code, "    [%d] less_equal    ", i);
                i += 5;
                break;
            case o_is_equal:
                print_three(code, "    [%d] is_equal      ", i);
                i += 5;
                break;
            case o_greater:
                print_three(code, "    [%d] greater       ", i);
                i += 5;
                break;
            case o_greater_eq:
                print_three(code, "    [%d] greater_equal ", i);
                i += 5;
                break;
            case o_not_eq:
                print_three(code, "    [%d] not_equal     ", i);
                i += 5;
                break;
            case o_jump:
                lily_impl_debugf("    [%d] jump          ", i);
                lily_impl_debugf("[%d].\n", code[i+1]);
                i += 2;
                break;
            case o_jump_if:
                left = ((lily_sym *)code[i+2]);
                lily_impl_debugf("    [%d] jump_if_", i);
                if (code[i+1] == 0)
                    lily_impl_debugf("false ");
                else
                    lily_impl_debugf("true  ");
                lily_impl_debugf("%s #%d -> [%d].\n",
                                 typename((lily_sym *)left), left->id,
                                 code[i+3]);
                i += 4;
                break;
            case o_func_call:
                /* For both functions and methods, [i+3] is the # of args. */
                print_call(code, "    [%d] func_call  \n        ", i);
                i += 6 + code[i+4];
                break;
            case o_method_call:
                print_call(code, "    [%d] method_call\n        ", i);
                i += 6 + code[i+4];
                break;
            case o_save:
                print_save(code, "    [%d] save (%d)\n", i);
                i += code[i+1] + 2;
                break;
            case o_restore:
                lily_impl_debugf("    [%d] restore (%d)\n", i, code[i+1]);
                i += 2;
                break;
            case o_return_val:
                print_one(code, "    [%d] return_value  ", i);
                i += 2;
                break;
            case o_return_noval:
                lily_impl_debugf("    [%d] return_noval\n", i);
                i++;
                break;
            case o_unary_minus:
                print_two(code, "    [%d] unary_minus   ", i);
                i += 4;
                break;
            case o_unary_not:
                print_two(code, "    [%d] unary_not     ", i);
                i += 4;
                break;
            case o_vm_return:
                lily_impl_debugf("    [%d] vm_return\n", i);
                return;
            default:
                lily_impl_debugf("    [%d] bad opcode %d.\n", i, code[i]);
                return;
        }
    }
}

static void show_sym(lily_sym *sym)
{
    if (!(sym->flags & S_IS_NIL)) {
        int cls_id;
        if (sym->sig->cls->id != SYM_CLASS_OBJECT)
            cls_id = sym->sig->cls->id;
        else
            cls_id = sym->sig->node.value_sig->cls->id;

        if (cls_id == SYM_CLASS_STR) {
            lily_impl_debugf("str(%-0.50s)\n",
                            ((lily_strval *)sym->value.ptr)->str);
        }
        else if (cls_id == SYM_CLASS_INTEGER)
            lily_impl_debugf("integer(%d)\n", sym->value.integer);
        else if (cls_id == SYM_CLASS_NUMBER)
            lily_impl_debugf("number(%f)\n", sym->value.number);
    }
    else
        lily_impl_debugf("(nil)\n");
}

void lily_show_symtab(lily_symtab *symtab)
{
    lily_var *var = symtab->var_start;
    lily_literal *lit = symtab->lit_start;

    lily_impl_debugf("Literals:\n");
    /* Show literal values first. */
    while (lit != NULL) {
        lily_impl_debugf("#%d: ", lit->id);
        show_sym((lily_sym *)lit);
        lit = lit->next;
    }

    lily_impl_debugf("Vars:\n");
    while (var != NULL) {
        lily_impl_debugf("#%d: ", var->id);
        if (var->line_num == 0) {
            /* This is a builtin symbol. */
            lily_impl_debugf("(builtin) %s %s\n", var->sig->cls->name,
                             var->name);
        }
        else {
            lily_impl_debugf("%s %s @ line %d\n", var->sig->cls->name,
                             var->name, var->line_num);
        }
        if (var->sig->cls->id == SYM_CLASS_METHOD)
            show_code(var);
        var = var->next;
    }
}
