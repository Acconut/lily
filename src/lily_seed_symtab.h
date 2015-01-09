#ifndef LILY_SEED_SYMTAB_H
# define LILY_SEED_SYMTAB_H

# include "lily_cls_integer.h"
# include "lily_cls_double.h"
# include "lily_cls_string.h"
# include "lily_cls_list.h"
# include "lily_cls_hash.h"
# include "lily_cls_all_exceptions.h"
# include "lily_vm.h"
# include "lily_gc.h"
# include "lily_class_funcs.h"

/* Sync name order with SYM_CLASS_* #defines in lily_symtab.h */
typedef const struct {
    char *name;
    uint16_t is_refcounted;
    uint16_t template_count;
    uint32_t flags;
    char *parent_name;
    const lily_prop_seed_t *prop_seeds;
    class_setup_func setup_func;
    gc_marker_func gc_marker;
    class_eq_func eq_func;
} class_seed;

/* Exception properties. [0] is message, [1] is traceback. */
static const lily_prop_seed_t traceback =
    {"traceback", NULL,
        {SYM_CLASS_LIST, SYM_CLASS_TUPLE, 2, SYM_CLASS_STRING, SYM_CLASS_INTEGER}};

static const lily_prop_seed_t message =
    {"message", &traceback,
        {SYM_CLASS_STRING}};

/* Note: If CLS_VALID_HASH_KEY is added to other classes, the vm will need to be
         updated to hash those classes right. It will also need KeyError
         printing to be touched up for that. Other things may also need updating
         too. */
class_seed class_seeds[] =
{
    {"integer",                 /* name */
     0,                         /* is_refcounted */
     0,                         /* template_count */
     CLS_VALID_HASH_KEY,        /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     lily_integer_setup,        /* setup_func */
     NULL,                      /* gc_marker */
     &lily_integer_eq           /* eq_func */
    },

    {"double",                  /* name */
     0,                         /* is_refcounted */
     0,                         /* template_count */
     CLS_VALID_HASH_KEY,        /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     lily_double_setup,         /* setup_func */
     NULL,                      /* gc_marker */
     &lily_double_eq            /* eq_func */
    },

    {"string",                  /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     CLS_VALID_HASH_KEY,        /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     lily_string_setup,         /* setup_func */
     NULL,                      /* gc_marker */
     &lily_string_eq},          /* eq_func */

    {"function",                /* name */
     0,                         /* is_refcounted */
     -1,                        /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     NULL,                      /* setup_func */
     NULL,                      /* gc_marker */
     &lily_generic_eq},         /* eq_func */

    {"any",                     /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     /* 'any' is treated as an enum class that has all classes ever defined
        within it. */
     CLS_ENUM_CLASS,            /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     NULL,                      /* setup_func */
     &lily_gc_any_marker,       /* gc_marker */
     &lily_any_eq},             /* eq_func */

    {"list",                    /* name */
     1,                         /* is_refcounted */
     1,                         /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     lily_list_setup,           /* setup_func */
     &lily_gc_list_marker,      /* gc_marker */
     &lily_list_eq},            /* eq_func */

    {"hash",                    /* name */
     1,                         /* is_refcounted */
     2,                         /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     lily_hash_setup,           /* setup_func */
     &lily_gc_hash_marker,      /* gc_marker */
     &lily_hash_eq},            /* eq_func */

    {"tuple",                   /* name */
     1,                         /* is_refcounted */
     -1,                        /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     NULL,                      /* setup_func */
     &lily_gc_tuple_marker,     /* gc_marker */
     &lily_tuple_eq},           /* eq_func */

    {"",                        /* name */
     0,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     NULL,                      /* setup_func */
     NULL,                      /* gc_marker */
     NULL},                     /* eq_func */

    {"package",                 /* name */
     0,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     NULL,                      /* property seeds */
     NULL,                      /* setup_func */
     NULL,                      /* gc_marker */
     NULL},                     /* eq_func */

    {"Exception",               /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     NULL,                      /* parent name */
     &message,                  /* property seeds */
     NULL,                      /* setup_func */
     NULL,                      /* gc_marker */
     NULL},                     /* eq_func */

    /* SyntaxError, ImportError, and EncodingError are intentionally missing
       because they cannot be raised while the vm is running. */

    {"NoMemoryError",           /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_nomemoryerror_setup,  /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"DivisionByZeroError",     /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_dbzerror_setup,       /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"IndexError",              /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_indexerror_setup,     /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"BadTypecastError",        /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_badtcerror_setup,     /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"NoReturnError",           /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_noreturnerror_setup,  /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"ValueError",              /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_valueerror_setup,     /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"RecursionError",          /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_recursionerror_setup, /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"KeyError",                /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_keyerror_setup,       /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */

    {"FormatError",             /* name */
     1,                         /* is_refcounted */
     0,                         /* template_count */
     0,                         /* flags */
     "Exception",               /* parent name */
     NULL,                      /* property seeds */
     lily_formaterror_setup,    /* setup_func */
     NULL,                      /* gc_marker */
     lily_instance_eq},         /* eq_func */
};

void lily_builtin_print(lily_vm_state *, lily_function_val *, uint16_t *);
void lily_builtin_show(lily_vm_state *, lily_function_val *, uint16_t *);
void lily_builtin_printfmt(lily_vm_state *, lily_function_val *, uint16_t *);
void lily_builtin_calltrace(lily_vm_state *, lily_function_val *, uint16_t *);

static const lily_func_seed calltrace = 
    {"calltrace", "function calltrace( => list[tuple[string, integer]])", lily_builtin_calltrace, NULL};
static const lily_func_seed show =
    {"show", "function show[A](A)", lily_builtin_show, &calltrace};
static const lily_func_seed print =
    {"print", "function print(string)", lily_builtin_print, &show};
static const lily_func_seed printfmt =
    {"printfmt", "function printfmt(string, list[any]...)", lily_builtin_printfmt, &print};

/* This must always be set to the last func seed defined here. */
#define GLOBAL_SEED_START printfmt

#endif
