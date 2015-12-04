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

void set_rank_and_world_size();


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
}

void start_even_process() {
	int query_id;
	int query_tag = 0;
	MPI_Request request;

	if (my_even_communicator_rank == MPI_UNDEFINED) {
		return;
	}

	while (1) {
		if (my_rank == PROCESS_ZERO) {
			//TODO
			//get_input();
		}
		if (my_even_communicator_rank == 0) {
			query_id = 10;
		} else {
			query_id = 20;
		}

		MPI_Bcast(&query_id, 1, MPI_INT, PROCESS_ZERO, EVEN_COMMUNICATOR);
		printf("Process: %d : Even_rank : %d : query id: %d\n", my_rank, my_even_communicator_rank, query_id);
		MPI_Isend(&query_id, 1, MPI_INT, my_odd_partner_rank, query_tag, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, MPI_STATUS_IGNORE);
		printf("Process: %d : send complete to : %d\n", my_rank, my_odd_partner_rank);
		++query_tag;
		//TODO refine to work when user enters zero
		break;

	}
}