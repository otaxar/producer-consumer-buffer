#include "bbuff.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>                  	// for debug prints only
#include <semaphore.h>

void* bbuff[BUFFER_SIZE];

int     item_ct,                        // # of items in the buffer
        lo,                             // next buff index for consumers
        hi;                             // next buff index for producers

sem_t   mutex,                          // lock for CS
        full,                           // keep track of inserted items
        empty;                          // keep track of available slots

// PURPOSE:	Initialize variables needed in bounded buffer
// POST:    item counter, producer & consumer indeces and semaphores initialized
// ----------------------------------------------------------------------------
void bbuff_init( void ){

    item_ct = 0;
 	lo = 0;
	hi = 0;

    // INIT semaphores
    // init params: name, 0 = shared between threads, initial value
    sem_init(&mutex, 0, 1 );
    sem_init(&full, 0, 0);
    sem_init(&empty, 0, BUFFER_SIZE);

}

// PURPOSE:	Insert item to bounded buffer
// PARAM:	*item - pointer to the candy object
// POST:	item is added to the candy buffer,
//			item counter updated
// ----------------------------------------------------------------------------
void bbuff_blocking_insert( void* item ){

    sem_wait(&empty);
    sem_wait(&mutex);

    bbuff[hi] = item;                       // Critical section
    hi = (hi + 1) % BUFFER_SIZE;            //
    item_ct++;                              //

    sem_post(&mutex);
    sem_post(&full);

    // printf("\ninserted 1 candy, (hi= %d, lo=%d )\n", hi, lo );        				// DEBUG line
}

// PURPOSE:	Extract(remove) item from the bounded buffer
// POST:	candy object is removed from the buffer and returned,
//			item counter is updated
// ----------------------------------------------------------------------------
void* bbuff_blocking_extract( void ){

    sem_wait(&full);
    sem_wait(&mutex);

    void * item = bbuff[lo];                // Critical section
    bbuff[lo] = NULL;                       //
    lo = (lo + 1) % BUFFER_SIZE;            //
    item_ct--;

    sem_post(&mutex);
    sem_post(&empty);

    //printf("\nextracted 1 candy, (hi= %d, lo=%d )\n", hi, lo );       		// DEBUG line
    return item;
}

// PURPOSE:	Check if bounded buffer is empty
// POST:	Return true if buffer is empty, false otherwise
// ----------------------------------------------------------------------------
_Bool bbuff_is_empty( void ){

    if ( item_ct == 0 )
        return true;
    else
        return false;
}

// PURPOSE: Provides the number of items in buffer
// POST:	Number of items in the buffer returned
// ----------------------------------------------------------------------------
int bbuff_get_items_ct(){
    return item_ct;
}
