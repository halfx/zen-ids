#ifndef _TAINT_H_
#define _TAINT_H_ 1

/*

typedef struct _cfg_opcode_id_t {
  uint routine_hash;
  uint op_index;
} cfg_opcode_id_t;

typedef enum _dataflow_operand_index_t {
  DATAFLOW_OPERAND_RESULT,
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

typedef struct _dataflow_live_variable_t {
  bool is_temp;
  union {
    uint temp_id;
    const char *var_name;
  };
  dataflow_operand_id_t src;
  cfg_opcode_id_t global_id; // where the global was locally declared (mainly for logging)
} dataflow_live_variable_t;

  For static dataflow, local variables occur per stack frame and have a type { temp, var } and name.
  Currently using the memory address of zend_op_array->opcodes as a stack frame id.

  In dynamic dataflow, a variable needs 3 types: { temp, var, global } with name and frame id.
  It would be ideal to pack all these vars into one giant hashtable, so I don't need to keep
  tagging them on the stack frames or the corresponding cfm. Issues though:

   � how to hash this thing safely?
     � 4 upper bytes of the stack_frame_id are generally available
   � how to clear a stack frame from the hashtable?

  It may be easier to just put a list of variables on each stack frame, and keep a global list.
  Maybe there's no need for TAINT_VAR_GLOBAL, since those have a different scope anyway.
  A stack frame id looks like this: { 0000 7fff e865 9180 }. So the upper 4 bytes can be messed
  with, at the slight risk of collision. But removal is hard in a perfect hash scheme, because
  how do I know what variables have been tracked in a given frame? Also, I don't know when frames
  get cleared. Though hm, this is transitory per request--it gets flushed at request start anyway.
  So could I just let them accumulate and toss the bucket at the end? I dunno how far a single
  request can really go.
 */

typedef enum _taint_variable_type_t {
  TAINT_VAR_NONE,
  TAINT_VAR_TEMP,
  TAINT_VAR_LOCAL,
  TAINT_VAR_GLOBAL
} taint_variable_type_t;

typedef union _taint_variable_id_t {
  uint temp_id;
  const char *var_name;
} taint_variable_id_t;

typedef struct _taint_variable_t {
  taint_variable_type_t type;
  taint_variable_id_t id;
  zend_op *stack_frame_id;
  void *taint;
} taint_variable_t;

void taint_var_add(application_t *app, taint_variable_type_t type, taint_variable_id_t id,
                   zend_op *stack_frame_id, void *taint);

void *taint_var_get(taint_variable_type_t type, taint_variable_id_t id, zend_op *stack_frame_id);

void *taint_var_remove(taint_variable_type_t type, taint_variable_id_t id, zend_op *stack_frame_id);

#endif
