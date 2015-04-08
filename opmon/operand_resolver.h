#ifndef _OPERAND_RESOLVER_H_
#define _OPERAND_RESOLVER_H_ 1

void destroy_operand_resolver();
const char *resolve_constant_include(zend_op *op);
char *resolve_eval_body(zend_op *op);
const char *locate_site_root(const char *filename /*absolute path*/);

#endif