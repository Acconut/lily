#ifndef LILY_SEED_SYMTAB_H
# define LILY_SEED_SYMTAB_H

# include "lily_cls_str.h"
# include "lily_cls_list.h"
# include "lily_vm.h"
# include "lily_gc.h"

/* Sync name order with SYM_CLASS_* #defines in lily_symtab.h */
typedef const struct {
    char *name;
    int is_refcounted;
    int template_count;
    int flags;
    class_setup_func setup_func;
    gc_marker_func gc_marker;
} class_seed;

/* Note: If CLS_VALID_HASH_KEY is added to other classes, the vm will need to be
         updated to hash those classes right. It will also need ErrNoSuchKey
         printing to be touched up for that. Other things may also need updating
         too. */
class_seed class_seeds[] =
{
    {"integer",  0, 0, CLS_VALID_HASH_KEY, NULL,            NULL},
    {"number",   0, 0, CLS_VALID_HASH_KEY, NULL,            NULL},
    {"str",      1, 0, CLS_VALID_HASH_KEY, lily_str_setup,  NULL},
    {"function", 0, 0, 0,                  NULL,            NULL},
    {"object",   1, 0, 0,                  NULL,            &lily_gc_object_marker},
    {"method",   1, 0, 0,                  NULL,            NULL},
    {"list",     1, 1, 0,                  lily_list_setup, &lily_gc_list_marker},
    {"hash",     1, 2, 0,                  NULL,            &lily_gc_hash_marker},
    /* * is the name of the template class. This was chosen because it's not a
       valid name so the user can't directly declare members of it. */
    {"*",        0, 0, 0,                  NULL,            NULL},
    {"package",  0, 0, 0,                  NULL,            NULL}
};

typedef const struct {
    char *name;
    uint64_t shorthash;
} keyword_seed;

keyword_seed keywords[] = {
    {"if",         26217},
    {"elif",       1718185061},
    {"else",       1702063205},
    {"return",     121437875889522},
    {"while",      435610544247},
    {"continue",   7310870969309884259},
    {"break",      461195539042},
    {"show",       2003789939},
    {"__line__",   6872323081280184159},
    {"__file__",   6872323072689856351},
    {"__method__", 7237117975334838111},
    {"for",        7499622},
    {"do",         28516}
};
void lily_builtin_print(lily_vm_state *, uintptr_t *, int);
void lily_builtin_printfmt(lily_vm_state *, uintptr_t *, int);

static const lily_func_seed print =
    {"print", lily_builtin_print, NULL,
        {SYM_CLASS_FUNCTION, 2, 0, -1, SYM_CLASS_STR}};
static const lily_func_seed printfmt =
    {"printfmt", lily_builtin_printfmt, &print,
        {SYM_CLASS_FUNCTION, 3, SIG_IS_VARARGS, -1, SYM_CLASS_STR, SYM_CLASS_OBJECT}};

/* This must always be set to the last func seed defined here. */
#define GLOBAL_SEED_START printfmt

#endif
