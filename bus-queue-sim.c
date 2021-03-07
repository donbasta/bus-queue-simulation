/* External definitions for single-server queueing system using simlib. */

#include "simlib.h"             /* Required for use of simlib.c. */

#define STREAM_INTERARRIVAL  1  /* Random-number stream for interarrivals. */
#define STREAM_UNLOAD 2 /* Random-number stream for unloading time. */
#define STREAM_LOAD 3 /* Random-number stream for loading time. */
#define STREAM_GEN_DEST 4
#define EVENT_AT_STATION_1 1 /* Event type for arrival Station 1. */
#define EVENT_AT_STATION_2 2 /* Event type for arrival Station 2. */
#define EVENT_AT_STATION_3 3 /* Event type for arrival Station 3. */
#define SAMPST_BUS_CAPACITY 1
#define EVENT_LOAD 4
#define EVENT_UNLOAD 5
#define STATION_1 1
#define STATION_2 2
#define STATION_3 3
#define NUM_STATION 3
#define LIST_DESTINATION_1 1
#define LIST_DESTINATION_2 2
#define LIST_DESTINATION_3 3
 

/* Declare non-simlib global variables. */
FILE  *outfile;

/* Declare Constant Variables */
const int AVG_ARRIVAL_TIME_STATION_1 = 14; // avg per hour
const int AVG_ARRIVAL_TIME_STATION_2 = 10; // avg peconst r hour
const int AVG_ARRIVAL_TIME_STATION_3 = 24; // avg per hour
const int MAX_CAPACITY = 20;
const double MAX_TIME = 80.0 * 60; // in minutes
const double SPEED_PER_MINUTE = 0.5; // 30 miles per hour
const double DISTANCE_31 = 4.5; // miles
const double DISTANCE_12 = 1; // miles
const double DISTANCE_23 = 4.5; // miles

typedef struct Station_Struct {
    double last_arrival_time;
} Station;

Station station[NUM_STATION + 1];
int next_station[NUM_STATION + 1] = {0, 2, 3, 1};

double distance_to_station[NUM_STATION + 1] = {
    0.0,
    DISTANCE_12,
    DISTANCE_23,
    DISTANCE_31,
};

int station_event[NUM_STATION + 1] = {
    -1,
    EVENT_AT_STATION_1,
    EVENT_AT_STATION_2,
    EVENT_AT_STATION_3
};

double avg_arrival_time_stations[NUM_STATION + 1] = {
    0.0,
    AVG_ARRIVAL_TIME_STATION_1,
    AVG_ARRIVAL_TIME_STATION_2,
    AVG_ARRIVAL_TIME_STATION_3
};

/* Declare non-simlib functions. */
double max(double, double);
void init_model(void);
void arrive(void);
void depart(void);
void report(void);
void transit(int);
int get_bus_size(void);
int generate_destination_from_station(int st_num);

int main()  /* Main function. */
{
    /* Open output files. */
    outfile = fopen("bus-sim.out", "w");

    /* Initialize simlib */
    init_simlib();

    /* Set maxatr = max(maximum number of attributes per record, 4) */
    maxatr = 4;  /* NEVER SET maxatr TO BE SMALLER THAN 4. */

    /* Initialize the model. */
    init_model();

    /* Run the simulation while more delays are still needed. */
    while (sim_time <= MAX_TIME) {

        /* Determine the next event. */
        timing();
        if (sim_time > MAX_TIME) break;

        /* Invoke the appropriate event function. */
        switch (next_event_type) {
            case EVENT_AT_STATION_1:
                transit(1);
                break;
            case EVENT_AT_STATION_2:
                transit(2);
                break;
            case EVENT_AT_STATION_3:
                transit(3);
                break;
        }
    }

    /* Invoke the report generator and end the simulation. */
    report();
    fclose(outfile);
    return 0;
}

void transit(int st_num) {
    double dep_time = sim_time + 5.0; // departure time
    // unload
    while (list_size[st_num] > 0 && sim_time <= MAX_TIME) {
        double unload_time = uniform(16, 24, STREAM_UNLOAD) / 60.0;
        event_schedule(sim_time + unload_time, EVENT_UNLOAD);
        timing();
        dep_time = max(dep_time, sim_time);
        list_remove(FIRST, st_num);
        timest(get_bus_size(), SAMPST_BUS_CAPACITY);
    }
    

    // load
    while (get_bus_size() < MAX_CAPACITY && station[st_num].last_arrival_time <= dep_time && sim_time <= MAX_TIME) {
        double load_time = uniform(15, 25, STREAM_LOAD) / 60.0;
        event_schedule(sim_time + load_time, EVENT_LOAD);
        timing();
        dep_time = max(dep_time, sim_time);
        transfer[1] = sim_time; // waktu naik
        transfer[2] = station[st_num].last_arrival_time; // waktu nyampe station
        list_file(LAST, generate_destination_from_station(st_num));
        station[st_num].last_arrival_time += expon((double) 1.0 / avg_arrival_time_stations[st_num], STREAM_INTERARRIVAL);
        timest(get_bus_size(), SAMPST_BUS_CAPACITY);
    }

    

    sim_time = dep_time;

    event_schedule(sim_time + distance_to_station[next_station[st_num]] / SPEED_PER_MINUTE,
                   station_event[next_station[st_num]]);
}

void init_model(void)  /* Initialization function. */
{
    event_schedule(sim_time + DISTANCE_31 / SPEED_PER_MINUTE, EVENT_AT_STATION_1);
    station[1].last_arrival_time = expon((double) 1.0 / AVG_ARRIVAL_TIME_STATION_1, STREAM_INTERARRIVAL);
    station[2].last_arrival_time = expon((double) 1.0 / AVG_ARRIVAL_TIME_STATION_2, STREAM_INTERARRIVAL);
    station[3].last_arrival_time = expon((double) 1.0 / AVG_ARRIVAL_TIME_STATION_3, STREAM_INTERARRIVAL);
}

int generate_destination_from_station(int st_num) {
    if (st_num != STATION_3)
        return STATION_3;
    double prob = lcgrand(STREAM_GEN_DEST);
    return (prob <= 0.583 ? STATION_1 : STATION_2);
}

void report(void)  /* Report generator function. */
{
    fprintf(outfile, "Bus Capacity Statistics\n");
    out_timest(outfile, SAMPST_BUS_CAPACITY, SAMPST_BUS_CAPACITY);
    /* Get and write out estimates of desired measures of performance. */
    // fprintf(outfile, "\nDelays in queue, in minutes:\n");
    // printf("\nDelays in queue, in minutes:\n");
    // out_sampst(outfile, SAMPST_DELAYS, SAMPST_DELAYS);
    // fprintf(outfile, "\nQueue length (1) and server utilization (2):\n");
    // printf("\nQueue length (1) and server utilization (2):\n");
    // out_filest(outfile, LIST_QUEUE, LIST_SERVER);
    // fprintf(outfile, "\nTime simulation ended:%12.3f minutes\n", sim_time);
    // printf("\nTime simulation ended:%12.3f minutes\n", sim_time);
}

int get_bus_size() {
    int ret = 0;
    for (int i = 1; i <= NUM_STATION; i++) {
        ret += list_size[i];
    }
    return ret;
}

double max(double a, double b) {
    return (a > b ? a : b);
}