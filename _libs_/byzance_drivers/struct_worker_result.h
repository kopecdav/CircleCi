#ifndef STRUCT_WORKER_RESULT_H
#define STRUCT_WORKER_RESULT_H
 
#include <stdint.h>

struct worker_result {
  uint32_t			rc;
  //string*			topic;
  picojson::object*	json_out;
};

#endif /* STRUCT_WORKER_RESULT_H */ 
