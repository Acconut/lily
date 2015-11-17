#include "lily_alloc.h"
#include "lily_value.h"
#include "lily_vm.h"

/* This is to get their gc collection functions... :( */

#include "lily_cls_list.h"
#include "lily_cls_tuple.h"
#include "lily_cls_hash.h"
#include "lily_cls_any.h"
#include "lily_cls_function.h"

/*  lily_deref
    This function will check that the value is refcounted and that it is not
    nil/protected before giving it a deref. It is therefore safe to pass
    anything to this function as long as it's not a NULL value.
    If the value given falls to 0 refs, it is immediately destroyed, as well as
    whatever is inside of it.

    Note: This destroys the contents inside the value, NOT the value itself. */
void lily_deref(lily_value *value)
{
    if ((value->flags & VAL_IS_NOT_DEREFABLE) == 0) {
        value->value.generic->refcount--;
        if (value->value.generic->refcount == 0)
            value->type->cls->destroy_func(value);
    }
}

/*  lily_deref_raw
    This is a helper function for lily_deref. This function calls lily_deref
    with a proper value that has the given type and raw value inside. */
void lily_deref_raw(lily_type *type, lily_raw_value raw)
{
    lily_value v;
    v.type = type;
    v.flags = (type->cls->flags & VAL_IS_PRIMITIVE);
    v.value = raw;

    lily_deref(&v);
}

inline lily_value *lily_new_value(uint64_t flags, lily_type *type,
        lily_raw_value raw)
{
    lily_value *v = lily_malloc(sizeof(lily_value));
    v->flags = flags;
    v->type = type;
    v->value = raw;

    return v;
}

/*  lily_assign_value
    This assigns the value on the left side to the value on the right side. Any
    necessary ref/deref action is done. The flags and the value are copied over,
    but not the type (as the type is usually already set). */
void lily_assign_value(lily_value *left, lily_value *right)
{
    if ((right->flags & VAL_IS_NOT_DEREFABLE) == 0)
        right->value.generic->refcount++;

    if ((left->flags & VAL_IS_NOT_DEREFABLE) == 0)
        lily_deref(left);

    left->value = right->value;
    left->flags = right->flags;
}

/*  lily_move_raw_value
    Assign the value on the right side into the left side. Decrease the refcount
    of the left side (if applicable), but do not increase the refcount of the
    right side. This is useful in situations where the right side is a
    newly-made value (which starts at one ref), and assign would falsely give it
    two refs. */
void lily_move_raw_value(lily_value *left, lily_raw_value raw_right)
{
    if ((left->flags & VAL_IS_NOT_DEREFABLE) == 0)
        lily_deref(left);

    left->value = raw_right;
    left->flags = (left->type->cls->flags & VAL_IS_PRIMITIVE);
}

/*  lily_copy_value
    Create a copy of the value passed in. If applicable, the refcount of the
    value passed in is increased. The caller is responsible for putting the
    returned value somewhere that the vm can see it. */
lily_value *lily_copy_value(lily_value *input)
{
    if ((input->flags & VAL_IS_NOT_DEREFABLE) == 0)
        input->value.generic->refcount++;

    return lily_new_value(input->flags, input->type, input->value);
}

inline lily_instance_val *lily_new_instance_val()
{
    lily_instance_val *ival = lily_malloc(sizeof(lily_instance_val));
    ival->refcount = 1;
    ival->gc_entry = NULL;
    ival->values = NULL;
    ival->num_values = -1;
    ival->visited = 0;
    ival->true_type = NULL;

    return ival;
}

inline lily_instance_val *lily_new_instance_val_for(lily_type *t)
{
    lily_instance_val *ival = lily_new_instance_val();
    int num_values = t->cls->prop_count;
    ival->values = lily_malloc(num_values * sizeof(lily_value *));
    ival->num_values = num_values;
    ival->true_type = t;

    return ival;
}

int lily_generic_eq(lily_vm_state *vm, int *depth, lily_value *left,
        lily_value *right)
{
    return (left->value.generic == right->value.generic);
}

int lily_instance_eq(lily_vm_state *vm, int *depth, lily_value *left,
        lily_value *right)
{
    int ret;

    if (left->value.instance->true_type == right->value.instance->true_type) {
        int i;
        ret = 1;

        for (i = 0;i < left->value.instance->num_values;i++) {
            lily_value **left_values = left->value.instance->values;
            lily_value **right_values = right->value.instance->values;

            class_eq_func eq_func = left_values[i]->type->cls->eq_func;
            (*depth)++;
            if (eq_func(vm, depth, left_values[i], right_values[i]) == 0) {
                ret = 0;
                (*depth)--;
                break;
            }
            (*depth)--;
        }
    }
    else
        ret = 0;

    return ret;
}

void lily_gc_collect_value(lily_type *value_type,
        lily_raw_value value)
{
    int entry_cls_id = value_type->cls->id;

    if (entry_cls_id == SYM_CLASS_LIST)
        lily_gc_collect_list(value_type, value.list);
    else if (entry_cls_id == SYM_CLASS_HASH)
        lily_gc_collect_hash(value_type, value.hash);
    else if (value_type->cls->flags & CLS_IS_ENUM)
        lily_gc_collect_any(value.any);
    else if (entry_cls_id == SYM_CLASS_TUPLE ||
             entry_cls_id >= SYM_CLASS_EXCEPTION)
        lily_gc_collect_tuple(value_type, value.list);
    else if (entry_cls_id == SYM_CLASS_FUNCTION)
        lily_gc_collect_function(value_type, value.function);
    else
        lily_deref_raw(value_type, value);
}
