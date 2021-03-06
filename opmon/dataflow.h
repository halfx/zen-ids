#ifndef DATAFLOW_H
#define DATAFLOW_H 1

#include "cfg.h"

typedef enum _dataflow_operand_index_t {
  DATAFLOW_OPERAND_RESULT, /* needs to be first (zero) to make sink.operand_index default to N/A */
  DATAFLOW_OPERAND_VALUE = DATAFLOW_OPERAND_RESULT,
  DATAFLOW_OPERAND_1,
  DATAFLOW_OPERAND_MAP = DATAFLOW_OPERAND_1,
  DATAFLOW_OPERAND_2,
  DATAFLOW_OPERAND_KEY = DATAFLOW_OPERAND_2,
  DATAFLOW_OPERAND_SOURCE,
  DATAFLOW_OPERAND_SINK
} dataflow_operand_index_t;

typedef struct _dataflow_operand_id_t {
  cfg_opcode_id_t opcode_id;
  dataflow_operand_index_t operand_index;
} dataflow_operand_id_t;

typedef struct _dataflow_predecessor_t {
  dataflow_operand_id_t operand_id;
  struct _dataflow_predecessor_t *next;
} dataflow_predecessor_t;

typedef struct _dataflow_source_include_t {
  uint routine_hash;
} dataflow_source_include_t;

typedef enum _dataflow_source_type_t {
  SOURCE_TYPE_NONE,      /* no source here */
  SOURCE_TYPE_INCLUDE,   /* include or require */ // kinda wonky having this in sources
  SOURCE_TYPE_HTTP,      /* HTTP parameters and cookies */
  SOURCE_TYPE_SESSION,   /* session variables */
  SOURCE_TYPE_SQL,       /* database query results */
  SOURCE_TYPE_FILE,      /* file access such as fread() */
  SOURCE_TYPE_SYSTEM,    /* server and environment configuration */
  SOURCE_TYPE_PARAMETER, /* function parameter */
  SOURCE_TYPE_RESULT,    /* function return value */
  SOURCE_TYPE_GLOBAL,    /* globally defined variables and constants */
} dataflow_source_type_t;

typedef struct _dataflow_source_t {
  cfg_opcode_id_t id;
  dataflow_source_type_t type;
  union {
    dataflow_source_include_t include;
    uint parameter_index;
  };
} dataflow_source_t;

typedef struct _dataflow_influence_t {
  dataflow_source_t source;
  struct _dataflow_influence_t *next;
} dataflow_influence_t;

typedef enum _dataflow_sink_type_t {
  SINK_TYPE_NONE,             /* no sink here */
  SINK_TYPE_VAR_NUM,          /* modifies an int or float variable */
  SINK_TYPE_VAR_BOOL,         /* modifies a bool variable */
  SINK_TYPE_VAR_STRING,       /* modifies a string */
  SINK_TYPE_VAR_COND,         /* can change a branch condition */
  SINK_TYPE_SESSION,          /* modifies a session variable */
  SINK_TYPE_OUTPUT,           /* specifies output text */
  SINK_TYPE_EDGE,             /* specifies an intra-procedural control flow target, e.g. jmp */
  SINK_TYPE_CALL,             /* specifies an inter-procedural control flow target, e.g. call */
  SINK_TYPE_RESULT,           /* specifies an inter-procedural control flow result, e.g. return */
  SINK_TYPE_SQL,              /* specifies an SQL insert or update (possibly a fragment) */
  /* Various functions like file_put_contents(), fopen(), fwrite(), etc. It might be a good idea
   * to monitor system calls in the process, though not sure how (hook?)
   */
  SINK_TYPE_FILE,
  SINK_TYPE_SYSTEM,
  SINK_TYPE_CODE,             /* eval() or create_function() */
  SINK_TYPE_CLASS_HIERARCHY,  /* add subclass, method, interface, trait */
  SINK_TYPE_INCLUDE,          /* include() or require() */
  SINK_TYPE_GLOBAL,           /* globally defined variables and constants */
  SINK_TYPE_EXIT_CODE,        /* the process exit code */
} dataflow_sink_type_t;

typedef struct _dataflow_sink_t {
  cfg_opcode_id_t id;
  dataflow_predecessor_t *predecessor;
  dataflow_influence_t *influence;
  dataflow_sink_type_t type;
} dataflow_sink_t;

typedef union _sink_identifier_t {
  const char *call_target;
} sink_identifier_t;

typedef enum _dataflow_condition_t {
  DATAFLOW_CONDITION_DIRECT,    /* the dataflow always occurs at this op */
  DATAFLOW_CONDITION_LOGICAL,   /* the dataflow depends on an immediate condition */
  DATAFLOW_CONDITION_INDIRECT,  /* the global state affects the dataflow */
} dataflow_condition_t;

typedef enum _dataflow_source_scope_t {
  /* Only the operands themselves can affect the op result. */
  DATAFLOW_SOURCE_IMMEDIATE,
  /* Only the containing routine can affect the op result. */
  DATAFLOW_SOURCE_ROUTINE,
  /* Dataflow from other routines may affect the op result. */
  DATAFLOW_SOURCE_PROGRAM,
  /* Specific hypothetical values */
  DATAFLOW_SOURCE_HYPOTHETICAL,
  /* Nothing is known (yet) about the scope of potential dataflow to the operands. */
  DATAFLOW_SOURCE_UNRESOLVED
} dataflow_source_scope_t;

/*
typedef enum _dataflow_source_scope_t {
  / * Only the immediate operands can affect the op result. * /
  DATAFLOW_SOURCE_SCOPE_IMMEDIATE,
  / * Dataflow to the op result may potentially be limited to local variables of any routine. * /
  DATAFLOW_SOURCE_SCOPE_LOCAL,
  / * Global state always flows to the op result. * /
  DATAFLOW_SOURCE_SCOPE_GLOBAL,
} dataflow_source_scope_t;
*/

/*
typedef enum _dataflow_const_type_t {
  DATAFLOW_CONST_LONG,
  DATAFLOW_CONST_DOUBLE,
  DATAFLOW_CONST_STRING
} dataflow_const_type_t;

typedef struct _dataflow_const_t {
  dataflow_const_type_t type;
  union {
    zend_ulong lval;
    zend_double dval;
    const char *string;
  };
} dataflow_const_t;
*/

typedef enum _dataflow_value_type_t {
  DATAFLOW_VALUE_TYPE_NONE,
  DATAFLOW_VALUE_TYPE_CONST,
  DATAFLOW_VALUE_TYPE_JMP,
  DATAFLOW_VALUE_TYPE_VAR,
  DATAFLOW_VALUE_TYPE_THIS,
  DATAFLOW_VALUE_TYPE_TEMP,
  DATAFLOW_VALUE_TYPE_INCLUDE,
  DATAFLOW_VALUE_TYPE_FCALL,
} dataflow_value_type_t;

typedef struct _dataflow_value_t {
  dataflow_value_type_t type;
  union {
    uint temp_id;
    uint jmp_target;
    const char *var_name;
    zval *constant;
    zend_class_entry *class;
  };
} dataflow_value_t;

/* also need:
 *   � include path
 *   � fcall target
 */
typedef struct _dataflow_operand_t {
  dataflow_value_t value;
  dataflow_predecessor_t *predecessor;
  dataflow_influence_t *influence;
} dataflow_operand_t;

typedef struct _dataflow_opcode_t {
  cfg_opcode_id_t id;
  dataflow_source_t source;
  dataflow_sink_t sink;
  union {
    dataflow_operand_t op1;
    dataflow_operand_t map;
  };
  union {
    dataflow_operand_t op2;
    dataflow_operand_t key;
  };
  union {
    dataflow_operand_t result;
    dataflow_operand_t value;
  };
} dataflow_opcode_t;

void init_dataflow_analysis();
void set_opcode_dump_file(FILE *file);
void dump_script_header(application_t *app, const char *routine_name, uint function_hash);
void dump_function_header(application_t *app, const char *unit_path, const char *routine_name,
                          uint function_hash);
void dump_operand(application_t *app, char index, zend_op_array *ops, znode_op *operand,
                  zend_uchar type);
void dump_fcall_opcode(application_t *app, zend_op_array *ops, zend_op *op,
                       const char *routine_name);
void dump_fcall_arg(application_t *app, zend_op_array *ops, zend_op *op, const char *routine_name);
void dump_map_assignment(application_t *app, zend_op_array *ops, zend_op *op, zend_op *next_op);
void dump_foreach_fetch(application_t *app, zend_op_array *ops, zend_op *op, zend_op *next_op);
void dump_opcode(application_t *app, zend_op_array *ops, zend_op *op);
void identify_sink_operands(application_t *app, zend_op_array *ops, zend_op *op, sink_identifier_t id);

bool is_db_source_function(const char *type, const char *name);
bool is_db_sink_function(const char *type, const char *name);
bool is_file_source_function(const char *name);
bool is_file_sink_function(const char *name);
bool is_system_source_function(const char *name);
bool is_system_sink_function(const char *name);

int analyze_dataflow(application_t *app, zend_file_handle *file);
void destroy_dataflow_analysis();
void add_dataflow_routine(application_t *app, uint routine_hash, zend_op_array *zops,
                          bool is_function);
void add_dataflow_opcode(uint routine_hash, uint index, zend_op_array *zops);
void push_dataflow_fcall_init(application_t *app, uint target_hash, const char *routine_name);
void add_dataflow_fcall(application_t *app, uint routine_hash, uint index, zend_op_array *zops,
                        const char *routine_name);
void add_dataflow_fcall_arg(uint routine_hash, uint index, zend_op_array *zops,
                        const char *routine_name);
void add_dataflow_map_assignment(uint routine_hash, uint index, zend_op_array *zops);
void add_dataflow_foreach_fetch(uint routine_hash, uint index, zend_op_array *zops);
void add_dataflow_include(uint routine_hash, uint index, zend_op_array *zops,
                          const char *include_path, uint include_hash);

#endif
