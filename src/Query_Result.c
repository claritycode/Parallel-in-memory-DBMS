/*
 * Query_Result.c
 *
 *  Created on: Dec 6, 2015
 *      Author: parth
 */


#include "Query_Result.h"
#include <stdlib.h>

void get_company_sale_result_buffer(int no_of_elements, company_sale_result **buffer, company_sale_result **buffer_current) {
	*buffer = (company_sale_result *) malloc(no_of_elements * sizeof(company_sale_result));
	if (*buffer == NULL) {
		fprintf(stderr, "A buffer creation process failed at : process: %d\n", my_rank);
	} else {
		*buffer_current = *buffer;
	}
}

void expand_company_sale_result_buffer(int *no_of_elements, company_sale_result **buffer_begin,
		company_sale_result ** buffer_current) {
	int new_no_of_elements = (*no_of_elements) * 2;
	*buffer_begin = (company_sale_result *) realloc(*buffer_begin,
			(new_no_of_elements * sizeof(company_sale_result)));
	if (*buffer_begin == NULL) {
			fprintf(stderr, "Buffer Creation error at process: %d\nSystem will now terminate\n", my_rank);
			//TODO
			//terminate_system();
	}
	*buffer_current = ((*buffer_begin) + (*no_of_elements));
	*no_of_elements = new_no_of_elements;
}

void get_sale_by_date_result_buffer(int no_of_elements, sale_by_date_result **buffer, sale_by_date_result **buffer_current) {
	*buffer = (sale_by_date_result *) malloc(no_of_elements * sizeof(sale_by_date_result));
	if (*buffer == NULL) {
		fprintf(stderr, "A buffer creation process failed at : process: %d\n", my_rank);
	} else {
		*buffer_current = *buffer;
	}
}

void expand_sale_by_date_result_buffer(int *no_of_elements, sale_by_date_result **buffer_begin,
		sale_by_date_result ** buffer_current) {
	int new_no_of_elements = (*no_of_elements) * 2;
	*buffer_begin = (sale_by_date_result *) realloc(*buffer_begin,
			(new_no_of_elements * sizeof(sale_by_date_result)));
	if (*buffer_begin == NULL) {
			fprintf(stderr, "Buffer Creation error at process: %d\nSystem will now terminate\n", my_rank);
			//TODO
			//terminate_system();
	}
	*buffer_current = ((*buffer_begin) + (*no_of_elements));
	*no_of_elements = new_no_of_elements;
}
