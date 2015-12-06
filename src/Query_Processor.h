/*
 * Query_Processor.h
 *
 *  Created on: Dec 6, 2015
 *      Author: parth
 */

#ifndef QUERY_PROCESSOR_H_
#define QUERY_PROCESSOR_H_

#include "Query.h"
#include "Query_Result.h"

// Buffer related variables
extern int data_buffer_size;				// Size of Current Buffer
extern Record * data_buffer_begin;			// POinter to the beigining i.e the first element of the buffer
extern Record * data_buffer_current;		// Pointer the next empty location in the buffer(i.e place where next element will go)
extern int used_buffer_size;				// number of elements in the buffer

static int INITIAL_RESULT_SIZE = 1000;

void process_query(Query *query);

#endif /* QUERY_PROCESSOR_H_ */
