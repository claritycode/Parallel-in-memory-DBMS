/*
 * even.c
 *
 *  Created on: Dec 4, 2015
 *      Author: parth
 */

#include "even.h"
#include "BufferCreator.h"
#include <stdio.h>
#include <stdlib.h>
#include "Query_Result.h"
#include "Parallel_Bucket_Sort.h"
#include "Shared_Memory.h"

//TODO For testing
#include "Query_Processor.h"
#include <string.h>

void process_result(Query *query, int result_size);
void set_rank_and_world_size();
void process_company_sale(int result_size);
void process_sale_by_date(Query * query, int result_size);

void merge_total_sale_by_date(sale_by_date_result *query_result, int query_result_size,
		sale_by_date_result ** final_result, int *final_result_size);
void merge_total_company_sale(company_sale_result *query_result, int query_result_size,
		company_sale_result  **final_result, int *final_result_size );

void process_delete_record(int result_size);


void set_even_process_communicator() {
	int *even_process_ranks;
	int i = 0, j = 0;
	MPI_Group my_group;
	MPI_Group my_added_group;


	if(world_size % 2 != 0) {
		fprintf(stderr,"Process: %d Error: The total number of processes should be an even number.\n"
				"The program can not continue in this state.\n"
				"Hence Terminating", my_rank);
		//TODO
		//terminate_system();
	}

	get_int_buffer((world_size/2), &even_process_ranks);

	while (i < world_size - 1) {
		even_process_ranks[j] = i;
		++j;
		i += 2;
	}

	MPI_Comm_group(MPI_COMM_WORLD, &my_group);
	MPI_Group_incl(my_group, (world_size/2), even_process_ranks, &my_added_group);
	MPI_Comm_create(MPI_COMM_WORLD, my_added_group, &EVEN_COMMUNICATOR);

	//TODO
	//Destroy EVEN_Communicator and COrresponding group

	free(even_process_ranks);

	set_rank_and_world_size();

}

void set_rank_and_world_size() {

	// For odd processes the EVEN_COMMUNICATOR will be set to MPI_COMM_NULL by the MPI_Comm_create call
	if (EVEN_COMMUNICATOR == MPI_COMM_NULL) {
		my_even_communicator_rank = MPI_UNDEFINED;
		even_communicator_world_size = MPI_UNDEFINED;
		my_odd_partner_rank = MPI_UNDEFINED;
	} else {
		MPI_Comm_size(EVEN_COMMUNICATOR, &even_communicator_world_size);
		MPI_Comm_rank(EVEN_COMMUNICATOR, &my_even_communicator_rank);
		my_odd_partner_rank = my_rank + 1;
	}

	INITIAL_MERGE_TOTAL_BUFFER_SIZE = 1000;

}

void start_even_process() {
	get_MPI_Type_for_query(&query_type);

	Query user_query;
	MPI_Request request;
	int result_size;

	if (my_even_communicator_rank == MPI_UNDEFINED) {
		return;
	}

	while (1) {
		if (my_rank == PROCESS_ZERO) {
			get_input(&user_query);
		}

		MPI_Bcast(&user_query, 1, query_type, PROCESS_ZERO, EVEN_COMMUNICATOR);

		MPI_Isend(&user_query, 1, query_type, my_odd_partner_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, MPI_STATUS_IGNORE);
		if (user_query.query_id == 4) {
			return;
		}

		// Note: This receive just communicates the size of the shared memory to the veen process.
		// The actual data is residing in the shared memory segmenet created with shmget and even process will retreive it from there
		MPI_Irecv(&result_size, 1, MPI_INT, my_odd_partner_rank, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, MPI_STATUS_IGNORE);

		process_result(&user_query, result_size);
	}
}

void process_result(Query *query, int result_size) {
	if (query -> query_id == 1) {
		process_company_sale(result_size);
	}
		else if (query -> query_id == 2) {
		process_sale_by_date(query, result_size);
	}
		else if (query -> query_id == 3) {
		process_delete_record(result_size);
	}
}

void process_delete_record(int result_size){
	int final_result;
	MPI_Reduce(&result_size, &final_result, 1, MPI_INT, MPI_SUM, 0, EVEN_COMMUNICATOR);
	if (my_rank == 0) {
		printf("\n%d number of records deleted from the database\n", final_result);
	}
}

void process_sale_by_date(Query * query, int result_size) {
	// Sorted result
	sale_by_date_result *sbd_result;
	unsigned long sale_by_date_result_size;

	// Final merged result
	sale_by_date_result * final_result;
	int final_result_size;

	// Result to be printed. Relevant only at PZERO
	sale_by_date_result *print_data;
	int print_data_size;

	// Result obtained from odd partner
	sale_by_date_result * my_result;
	int no_of_elements = result_size;
	sale_by_date_result *temp_ptr;

	get_sale_by_date_result_buffer(result_size, &my_result, &temp_ptr);

	receive_sbd_from_sm(my_result, result_size);

	parallel_bucket_sort_sale_by_date(*query, my_result, no_of_elements, &sbd_result, &sale_by_date_result_size);

	merge_total_sale_by_date(sbd_result, sale_by_date_result_size, &final_result, &final_result_size);

	send_sale_by_date_result_to_pzero(final_result, final_result_size, &print_data, &print_data_size);

	if (my_rank == 0) {
		print_sale(print_data, print_data_size);
	}
}

void process_company_sale(int result_size) {
	company_sale_result *cs_result;
	unsigned long company_sale_result_size;

	company_sale_result *final_result;
	int final_result_size;

	company_sale_result *print_data;
	int print_data_size;

	// Result obtained from partner
	company_sale_result * my_result;
	int no_of_elements = result_size;
	company_sale_result *temp_ptr;

	get_company_sale_result_buffer(result_size, &my_result, &temp_ptr);

	receive_cs_from_sm(my_result, result_size);

	parallel_bucket_sort_company_sale(my_result, no_of_elements, &cs_result, &company_sale_result_size);

	merge_total_company_sale(cs_result, company_sale_result_size, &final_result, &final_result_size);

	send_company_sale_result_to_pzero(final_result, final_result_size, &print_data, &print_data_size);

	if (my_rank == 0) {
		print_company_name(print_data, print_data_size);
	}
}


void merge_total_sale_by_date(sale_by_date_result *query_result, int query_result_size,
		sale_by_date_result ** final_result, int *final_result_size) {
	int i;
	int result;
	int no_of_elements = INITIAL_MERGE_TOTAL_BUFFER_SIZE;
	int used_final_result_space = 0;
	sale_by_date_result *final_result_current;

	if (query_result_size == 0) {
		return;
	}

	get_sale_by_date_result_buffer(INITIAL_MERGE_TOTAL_BUFFER_SIZE, final_result, &final_result_current);


	final_result_current = *final_result;

	// Used_final_result_space points to the last element in the buffer
	final_result_current[used_final_result_space] = query_result[0];

	for (i = 1; i != query_result_size; ++i) {
		result = compare_sale_by_date_result(&(final_result_current[used_final_result_space]), &(query_result[i]));
		if (result == 0) {
			final_result_current[used_final_result_space].sales_total += query_result[i].sales_total;
		} else {
			if ((used_final_result_space + 1) == no_of_elements) {
				expand_sale_by_date_result_buffer(&no_of_elements, final_result, &final_result_current);
			}
			++ used_final_result_space;
			final_result_current[used_final_result_space] = query_result[i];
		}
	}

	++used_final_result_space;		// Increment after the last element entered in the loop
	collapse_sale_by_date_result_buffer(used_final_result_space, final_result);

	*final_result_size = used_final_result_space;

}

void merge_total_company_sale(company_sale_result *query_result, int query_result_size,
		company_sale_result  **final_result, int *final_result_size ) {
	int i;
	int result;
	int no_of_elements = INITIAL_MERGE_TOTAL_BUFFER_SIZE;
	int used_final_result_space = 0;
	company_sale_result *final_result_current;

	if (query_result_size == 0) {
		return;
	}

	get_company_sale_result_buffer(INITIAL_MERGE_TOTAL_BUFFER_SIZE, final_result, &final_result_current);


	final_result_current = *final_result;

	// Used_final_result_space points to the last element in the buffer
	final_result_current[used_final_result_space] = query_result[0];

	for (i = 1; i != query_result_size; ++i) {
		result = compare_company_sale_result(&(final_result_current[used_final_result_space]), &(query_result[i]));
		if (result == 0) {
			final_result_current[used_final_result_space].sales_total += query_result[i].sales_total;
		} else {
			if ((used_final_result_space + 1) == no_of_elements) {
				expand_company_sale_result_buffer(&no_of_elements, final_result, &final_result_current);
			}
			++ used_final_result_space;
			final_result_current[used_final_result_space] = query_result[i];
		}
	}

	++used_final_result_space;		// Increment after the last element entered in the loop
	collapse_company_sale_result_buffer(used_final_result_space, final_result);

	*final_result_size = used_final_result_space;
}
