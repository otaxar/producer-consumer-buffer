#include "stats.h"
#include <stdlib.h>
#include <float.h>							// provides the DBL_MAX
#include <semaphore.h>
#include <stdio.h>

stat_record_t *stat_arr;
double total_delay;
int total_fact;								// needed for print out loop

sem_t * stat_mutex;

// PURPOSE:	Initialize the statistics array and variables
// PARAM:	num_producers - number of factories (rows in the stats)
// ----------------------------------------------------------------------------
void stats_init(int num_producers){

	total_fact = num_producers;
	stat_arr = malloc( sizeof(stat_record_t) * num_producers );

	stat_mutex = malloc( sizeof(*stat_mutex) * num_producers );

	for (int i = 0; i < num_producers; i++ ){

		sem_init(&stat_mutex[i], 0, 1);

		stat_arr[i].factory_num = i;
		stat_arr[i].candy_made_ct = 0;
		stat_arr[i].candy_eaten_ct = 0;
		stat_arr[i].min_delay = 0.0;
		stat_arr[i].avg_delay = 0.0;
		stat_arr[i].max_delay = 0.0;

		stat_arr[i].total_delay = 0.0;		//accumulator
	}


}

// POST:	Dynamically alocated memory is freed up
// ----------------------------------------------------------------------------
void stats_cleanup(){
	free( stat_arr );
	free( stat_mutex );
}

// POST:	Number of candies made by this factory is updated
// PARAM:	factory_number - factory id
// ----------------------------------------------------------------------------
void stats_record_produced( int factory_number ){

	sem_wait(&stat_mutex[factory_number]);
	stat_arr[factory_number].candy_made_ct++;	// CS
	sem_post(&stat_mutex[factory_number]);
}

// POST:	Statistics for this factory are updated upon candy consumption
// PARAM:	factory_number - factory id
//			delay_in_ms - difference between production and consumption time
// ----------------------------------------------------------------------------
void stats_record_consumed( int factory_number, double delay_in_ms ){

	sem_wait(&stat_mutex[factory_number]);

	stat_arr[factory_number].candy_eaten_ct++;
	stat_arr[factory_number].total_delay += delay_in_ms;

	// update the stats (w/ exception of avg, that one will be done at the end)
	// first time
	if( stat_arr[factory_number].candy_eaten_ct == 1 ){
		stat_arr[factory_number].min_delay = delay_in_ms;
		stat_arr[factory_number].max_delay = delay_in_ms;
	}
	// all other times
	else{

 		if( stat_arr[factory_number].min_delay > delay_in_ms )
			stat_arr[factory_number].min_delay = delay_in_ms;

		if( stat_arr[factory_number].max_delay < delay_in_ms )
			stat_arr[factory_number].max_delay = delay_in_ms;
	}

	sem_post(&stat_mutex[factory_number]);
}

// POST:	avg delay computed and statisctics displayed for all factories
// ----------------------------------------------------------------------------
void stats_display( void ){

	printf("\nStatistics:\n");
	printf("%s%8s%8s%16s%16s%16s\n", "Factory#", "#Made", "#Eaten",
			"Min Delay[ms]", "Avg Delay[ms]", "Max Delay[ms]");

	for( int i = 0; i < total_fact; i++ ) {
		printf("%4d%10d%7d%18f%16f%16f\n",
				stat_arr[i].factory_num,
				stat_arr[i].candy_made_ct,
				stat_arr[i].candy_eaten_ct,
				stat_arr[i].min_delay,
				stat_arr[i].total_delay / (double) stat_arr[i].candy_eaten_ct,
				stat_arr[i].max_delay);

		if( stat_arr[i].candy_made_ct != stat_arr[i].candy_eaten_ct )
			printf("\tERROR: Mismatch between number made and eaten.\n");
	}

	printf("\n");
}
