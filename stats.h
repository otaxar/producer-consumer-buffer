#ifndef STATS_H
#define STATS_H

typedef struct {
	int factory_num;
	int candy_made_ct;
	int candy_eaten_ct;
	double min_delay;
	double avg_delay;
	double max_delay;
	double total_delay;
} stat_record_t;

void stats_init( int num_producers );
void stats_cleanup( void );
void stats_record_produced( int factory_number );
void stats_record_consumed( int factory_number, double delay_in_ms );
void stats_display( void );

#endif
