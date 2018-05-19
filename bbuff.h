#ifndef BBUFF_H
#define BBUFF_H

#define BUFFER_SIZE 10 

// Candy def
typedef struct{
    int factory_number;             // Candy factory ID
    double time_stamp_in_ms;        // Time of production
} candy_t;



// FN. Prototypes
void bbuff_init( void );
void bbuff_blocking_insert( void* item);
void* bbuff_blocking_extract( void );
_Bool bbuff_is_empty( void );

void print_bbbuff( void );
int bbuff_get_items_ct();


#endif
