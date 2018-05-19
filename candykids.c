// SFU - CMPT 300 - PRJ3
// Fall 2017
// Otakar Valek (ovalek, 301 187 105)

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <stdint.h>

#include "bbuff.h"
#include "stats.h"

#define MAX_FACTORY_WAIT 4			// 1 more than the actual limit
#define MAX_KID_WAIT 2				// 1 more than the actual limit

_Bool stop_thread = false;
long total_factories = 0, total_kids = 0, total_seconds = 0;

int res = 0;

// PURPOSE: Validates if:
//           1) the correct number of arguments
//           2) that all argument numeric values are greater than 0
//           If problems found, error message is displayed and program is
//           terminated.
// PARAMS:  argc - number of arguments
//          argv[] - array with the argument values
// POST:    N/A
// ----------------------------------------------------------------------------
void validate_arguments( int argc, char *argv[] ){

    // check that there are exactly 3 arguments
    if ( argc != 4 ){
        printf("\nExactly 3 numeric arguments are required: <#factories> <#kids> <#seconds>\n");
        exit(-1);
    }
    else{
        // If yes, convert to numbers and check they are >0

        total_factories = strtol( argv[1], NULL, 0);
        total_kids = strtol( argv[2], NULL, 0);
        total_seconds = strtol( argv[3], NULL, 0);

        if ( total_factories <= 0 || total_kids <= 0 || total_seconds <= 0 ){
            printf("\nAll Arguments <#factories> <#kids> <#seconds> must be numbers greater than 0\n" );
            exit(-1);
        }
    }
}

// PARAMS:  N/A
// POST:    Returns current time
// ----------------------------------------------------------------------------
double current_time_in_ms( void ){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

// PURPOSE: Defines what a single Factory thread should do
// PARAM:   *arg - default argument, in this case candy factory id
//
// POST:    Factory thread produced candy and added it to the buffer,
//          Statistics for this factory) have been updated
// ----------------------------------------------------------------------------
void* factory_runner( void * arg ){

	int fact_id = (intptr_t) arg;
	int fact_wait;

	while(!stop_thread) {

		fact_wait = rand() % MAX_FACTORY_WAIT ;
		printf("\tFactory %d ships candy & waits %ds, \n", fact_id, fact_wait);

		//create new candy, insert into buffer
		candy_t * candy = malloc( sizeof(candy_t) );
		candy->factory_number = fact_id;
		candy->time_stamp_in_ms = current_time_in_ms();
		bbuff_blocking_insert( candy );
		stats_record_produced( fact_id );  	// update candy ct for this factory
		sleep( fact_wait );
	}

	printf("Candy-factory %d done\n", fact_id);

	pthread_exit(0);
}

// PURPOSE:	Defines what a single Kid thread should do
// PARAM:	*arg - default argument, in this case kid id (used only if status
//				   update about eating the candy is displayed)
// POST:	Kid thread removed candy from the buffer and consumer it
//			Statistics for candy producer have been updated
// ----------------------------------------------------------------------------
void * kids_runner (void * arg){

	//int kid_id = (intptr_t) arg;												// DEBUG line
 	int kid_wait;

	while (true){

		kid_wait = rand() % MAX_KID_WAIT;

		candy_t * candy = (candy_t *) bbuff_blocking_extract();
		stats_record_consumed( candy->factory_number,
							   current_time_in_ms() - candy->time_stamp_in_ms );

		//printf("\tKid %d eats candy & waits %ds\n", kid_id, kid_wait);		//DEBUG line

		free(candy);
		sleep(kid_wait);
	}

	pthread_exit(0);
}

// PURPOSE:	Start all Factory threads
// PARAM:	*f_tids - factory thread ids array will be later used to stop the threads
// POST:	Number of factory threads defined in command line argument is created
// ----------------------------------------------------------------------------
void runFactories( pthread_t * f_tids ){

	for ( int i = 0; i < total_factories; i++ ){

		res = pthread_create( &f_tids[i], NULL, factory_runner, (void*) (intptr_t) i );

		if (res != 0){
			printf("\nERR while creating factory thread" );
			exit(-1);
		}
	}
}

// PURPOSE:	Start all Kid threads
// PARAM:	*k_tids - kid thread ids array will be later used to stop the threads
// POST:	Number of kid threads defined in command line argument is created
// ----------------------------------------------------------------------------
void runKids( pthread_t *k_tids){

	for ( int i = 0; i < total_kids; i++ ){

		res = pthread_create( &k_tids[i], NULL, kids_runner, (void*) (intptr_t) i );

		if (res != 0){
			printf("\nERR while creating kid thread" );
			exit(-1);
		}
	}
}

// PURPOSE:	Emulates the flow of time
// POST:	Returns control to main flow after the number of seconds defined in
//          command argument
// ----------------------------------------------------------------------------
void emulateTimeLimit(){

	for ( int i = 0; i < total_seconds; i++ ){
		sleep(1);
		printf("Time: %ds\n", i+1);
	}
	stop_thread = true;
	printf("\n");
}

// PURPOSE:	Stops all factry threads
// PARAM:	*f_tids	- pointer to thread id array
// POST:	All Factory threads are stopped
// ----------------------------------------------------------------------------
void stopFactories( pthread_t *f_tids ){

printf("\nStopping candy factories...\n" );
	for ( int i = 0; i < total_factories; i++ ){
		pthread_join(f_tids[i], NULL);
	}
}

// PURPOSE:	Allow time to the kid threads to run and eat up all candies
// POST:	Candies are eaten up
void allowKidsToFinishCandies(){

		while( !bbuff_is_empty()  ){
			printf("\nCandy left: %d, Waiting for all candy to be consumed...", bbuff_get_items_ct() );
			sleep(1);
		}
}

// PURPOSE:	Stops all kid threads
// PARAM:	*k_tids	- pointer to thread id array
// POST:	All kid threads are cancelled/stopped
// ----------------------------------------------------------------------------
void stopKids( pthread_t *k_tids ){
	printf("\n\nStopping kids...\n" );
	for ( int i = 0; i < total_kids; i++ ){
		pthread_cancel(k_tids[i]);
		pthread_join(k_tids[i], NULL);
	}
}

// ============================================================================
int main( int argc, char ** argv ){

    validate_arguments( argc, argv );

	// init
    bbuff_init();
    stats_init( total_factories );
    srand( time( NULL ) );
	pthread_t *factory_tids = malloc( sizeof(pthread_t) * total_factories );
	pthread_t *kid_tids = malloc( sizeof(pthread_t) * total_kids );

	printf("Time: 0s\n");

	// Start threads
	runFactories( factory_tids );
	runKids( kid_tids );

	emulateTimeLimit();

	// Stop threads
	stopFactories( factory_tids );

	allowKidsToFinishCandies();

	stopKids( kid_tids );

	stats_display();

	// release allocated memory
	free(factory_tids);
	free(kid_tids);
	stats_cleanup();

	return 0;
}
