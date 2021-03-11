/* External definitions for single-server queueing system using simlib. */

#include "simlib.h"             /* Required for use of simlib.c. */

#define STREAM_INTERARRIVAL  1  /* Random-number stream for interarrivals. */
#define STREAM_UNLOAD 2 /* Random-number stream for unloading time. */
#define STREAM_LOAD 3 /* Random-number stream for loading time. */
#define STREAM_GEN_DEST 4
#define EVENT_AT_STATION_1 1 /* Event type for arrival Station 1. */
#define EVENT_AT_STATION_2 2 /* Event type for arrival Station 2. */
#define EVENT_AT_STATION_3 3 /* Event type for arrival Station 3. */
#define EVENT_LOAD 4
#define EVENT_UNLOAD 5
#define STATION_1 1
#define STATION_2 2
#define STATION_3 3
#define NUM_STATION 3
#define LIST_DESTINATION_1 1
#define LIST_DESTINATION_2 2
#define LIST_DESTINATION_3 3

#define MAX_QUEUE 10000
 

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

/* SAMPST event declaration */
const int SAMPST_DELAY_STATION[NUM_STATION + 1] = {0, 9, 10, 11};
const int SAMPST_BUS_TRANSIT[NUM_STATION + 1] = {0, 5, 6, 7};
const int SAMPST_BUS_LOOP = 8;
const int SAMPST_TIME_PEOPLE_IN_SYSTEM[NUM_STATION + 1] = {0, 12, 13, 14};

/* TIMEST event declaration */
const int TIMEST_PEOPLE_STATION[NUM_STATION + 1] = {0, 1, 2, 3};
const int TIMEST_BUS_CAPACITY = 4;

typedef struct Station_Struct {
    // double last_arrival_time;
    double queue_line[MAX_QUEUE]; // arrival time
    double queue_line_out[MAX_QUEUE]; // waktu naik bis
    int length;
    int frontid;
} Station;


const int next_station[NUM_STATION + 1] = {0, 2, 3, 1};

const double distance_to_station[NUM_STATION + 1] = {
    0.0,
    DISTANCE_31,
    DISTANCE_12,
    DISTANCE_23
};

const int station_event[NUM_STATION + 1] = {
    -1,
    EVENT_AT_STATION_1,
    EVENT_AT_STATION_2,
    EVENT_AT_STATION_3
};

const double avg_arrival_time_stations[NUM_STATION + 1] = {
    0.0,
    AVG_ARRIVAL_TIME_STATION_1,
    AVG_ARRIVAL_TIME_STATION_2,
    AVG_ARRIVAL_TIME_STATION_3
};


/* Declare Global Variables */
Station station[NUM_STATION + 1];
double last_depart_from_station_3 = 0.0;


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
    maxatr = 10;  /* NEVER SET maxatr TO BE SMALLER THAN 4. */

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

void report(void)  /* Report generator function. */
{
    // SOAL 1: Rata-rata dan maksimal orang pada queue untuk setiap station
    fprintf(outfile, "Number of People in Station Queue Statistics\n");
    for (int st_num = 1; st_num <= 3; st_num++) {
        int len = station[st_num].length;
        int id_in = 0;
        int id_out = 0;
        int cur_queue_size = 0;
        while (id_in < len || id_out < len) {
            double time_of_change;
            if (id_out >= len || station[st_num].queue_line[id_in] <= station[st_num].queue_line_out[id_out]) {
                cur_queue_size++;
                time_of_change = station[st_num].queue_line[id_in];
                id_in++;
            } else {
                cur_queue_size--;
                time_of_change = station[st_num].queue_line_out[id_out];
                id_out++;
            }
            double temp = sim_time;
            sim_time = time_of_change;
            timest(cur_queue_size, TIMEST_PEOPLE_STATION[st_num]);
            sim_time = temp;
        }
    }
    out_timest(outfile, TIMEST_PEOPLE_STATION[1], TIMEST_PEOPLE_STATION[NUM_STATION]);

    // SOAL 2: Rata-rata dan maksimal delay pada queue untuk setiap station
    fprintf(outfile, "Delay in Stations Statistics (in minutes)\n");
    out_sampst(outfile, SAMPST_DELAY_STATION[1], SAMPST_DELAY_STATION[NUM_STATION]);

    // SOAL 3: Rata-rata dan maksimal orang di bus
    fprintf(outfile, "Bus Passengers Statistics\n");
    out_timest(outfile, TIMEST_BUS_CAPACITY, TIMEST_BUS_CAPACITY);

    // SOAL 4: Rata-rata, maksimal, dan minimal waktu bus berhenti di setiap station
    fprintf(outfile, "Average Time Bus Transit at Stations (in minutes)\n");
    out_sampst(outfile, SAMPST_BUS_TRANSIT[1], SAMPST_BUS_TRANSIT[NUM_STATION]);

    // SOAL 5: Rata-rata, maksimal, dan minimal waktu bus melakukan putaran
    fprintf(outfile, "Bus Loop (From Car Rental to Car Rental again) Statistics (in minutes)\n");
    out_sampst(outfile, SAMPST_BUS_LOOP, SAMPST_BUS_LOOP);

    // SOAL 6: Rata-rata, maksimal, dan minimal setiap orang berada dalam sistem (mulai dari datang di lokasi awal hingga tiba di lokasi tujuan)
    fprintf(outfile, "Duration people in the system by departure station (in minutes)\n");
    out_sampst(outfile, SAMPST_TIME_PEOPLE_IN_SYSTEM[1], SAMPST_TIME_PEOPLE_IN_SYSTEM[NUM_STATION]);
}

void transit(int st_num) {
    double transit_arrive_time = sim_time;
    double dep_time = sim_time + 5.0; // departure time
    // unload
    while (list_size[st_num] > 0 && sim_time <= MAX_TIME) {
        // time people in system
        list_remove(FIRST, st_num);
        int station_arrival = transfer[1];
        int system_arrival = transfer[2];

        // bus statistics
        timest(get_bus_size(), TIMEST_BUS_CAPACITY);
        double unload_time = uniform(16, 24, STREAM_UNLOAD) / 60.0;
        event_schedule(sim_time + unload_time, EVENT_UNLOAD);
        timing();
        sampst(sim_time - system_arrival, SAMPST_TIME_PEOPLE_IN_SYSTEM[station_arrival]);
        dep_time = max(dep_time, sim_time);
    }
    // load
    while (get_bus_size() < MAX_CAPACITY && station[st_num].queue_line[station[st_num].frontid] <= dep_time && sim_time <= MAX_TIME) {
        if (sim_time < station[st_num].queue_line[station[st_num].frontid]) {
            event_schedule(station[st_num].queue_line[station[st_num].frontid], EVENT_LOAD);
            timing();
        }
        transfer[1] = st_num; // stasiun keberangkatan
        transfer[2] = station[st_num].queue_line[station[st_num].frontid]; // waktu nyampe di sistem
        transfer[3] = sim_time; // waktu dia naik
        list_file(LAST, generate_destination_from_station(st_num));
        station[st_num].queue_line_out[station[st_num].frontid] = sim_time;
        
        // waktu naik bus - waktu dateng
        double delay = station[st_num].queue_line_out[station[st_num].frontid] - station[st_num].queue_line[station[st_num].frontid];
        sampst(delay, SAMPST_DELAY_STATION[st_num]);
        station[st_num].frontid++;
        
        // getting bus capacity statistic
        timest(get_bus_size(), TIMEST_BUS_CAPACITY);
        
        double load_time = uniform(15, 25, STREAM_LOAD) / 60.0;
        event_schedule(sim_time + load_time, EVENT_LOAD);
        timing();
        dep_time = max(dep_time, sim_time);
    }

    sim_time = dep_time;
    double transit_depart_time = sim_time;
    
    sampst(transit_depart_time - transit_arrive_time, SAMPST_BUS_TRANSIT[st_num]);
    
    /* Bus Loop */
    if (st_num == 3) {
        sampst(sim_time - last_depart_from_station_3, SAMPST_BUS_LOOP);
        last_depart_from_station_3 = sim_time;
    }

    event_schedule(sim_time + distance_to_station[next_station[st_num]] / SPEED_PER_MINUTE,
                   station_event[next_station[st_num]]);
}

void init_model(void)  /* Initialization function. */
{
    event_schedule(sim_time + DISTANCE_31 / SPEED_PER_MINUTE, EVENT_AT_STATION_1);
    for (int st_num = 1; st_num <= NUM_STATION; st_num++) {
        double cur_time = 0;
        station[st_num].length = 0;
        while (cur_time <= MAX_TIME) {
            double nperson = cur_time + expon((double) 1.0 / avg_arrival_time_stations[st_num], STREAM_INTERARRIVAL) * 60.0;
            if (nperson <= MAX_TIME) {
                station[st_num].queue_line_out[station[st_num].length] = MAX_TIME;
                station[st_num].queue_line[station[st_num].length++] = nperson;
            }
            cur_time = nperson;
        }
        station[st_num].queue_line[station[st_num].length] = MAX_TIME + MAX_TIME; // barrier
        station[st_num].frontid = 0;
    }
}

int generate_destination_from_station(int st_num) {
    if (st_num != STATION_3)
        return STATION_3;
    double prob = lcgrand(STREAM_GEN_DEST);
    return (prob <= 0.583 ? STATION_1 : STATION_2);
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