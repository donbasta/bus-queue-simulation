#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#define Q_LIMIT 100
#define BUSY 1
#define IDLE 0

int next_event_type, num_custs_delayed, num_delays_required, num_events, num_in_q,server_status;
float area_num_in_q,area_server_status, mean_interarrival, mean_service, simclock, time_arrival[Q_LIMIT + 1], time_last_event, time_next_event[3], total_of_delays;
void initialize(void);
void timing(void);
void arrive(void);
void depart(void);
void update_time_avg_stats(void);
void report(void);
float expon(float mean);

int main() {

    num_events = 2;

    /* baca input parameter */
    printf("Mean inter arrival : ");scanf("%f",&mean_interarrival);
    printf("Mean service time : ");scanf("%f",&mean_service);
    printf("Number customer : ");scanf("%d",&num_delays_required);
    initialize();
    printf("\ntime \tEvent Type\tArrival \tDepart \t\t Queue\n");
    while (num_custs_delayed < num_delays_required) {
        timing();
        printf("%7.3f\t\t%d\t%8.3f\t%8.3f\t%d\n", simclock,next_event_type,time_next_event[1],time_next_event[2],num_in_q);
        update_time_avg_stats();
        switch (next_event_type) {
            case 1:
                arrive();
                break;
            case 2:
                depart();
                break;
        }
    }
    report();
    return 0;
}

void initialize(void) {
    simclock = 0.0;
    /* Variable state */
    server_status = IDLE;
    num_in_q = 0;
    time_last_event = 0.0;
    /* Variable statistic */
    num_custs_delayed = 0;
    total_of_delays = 0.0;
    area_num_in_q = 0.0;
    area_server_status = 0.0;
    time_next_event[1] = simclock + expon(mean_interarrival);
    time_next_event[2] = INT_MAX;
}

void timing(void) {
    int i;
    float min_time_next_event = INT_MAX;
    next_event_type = 0;
    /* Determine the event type of the next event to occur */
    for (i=1; i<= num_events; i++) {
        if (time_next_event[i] < min_time_next_event) {
            min_time_next_event = time_next_event[i];
            next_event_type = i;
        }
    }
    if (next_event_type == 0) {
    /* The event list is empty so stop the simulation */
        exit(1);
    }
    simclock = min_time_next_event;
}

void arrive(void) {
    float delay;
    time_next_event[1] = simclock + expon(mean_interarrival);
    if (server_status == BUSY) {
        ++num_in_q;
        if (num_in_q > Q_LIMIT) {
        /* The Queue has overflowed, stop simulation */
            exit(1);
        }
        time_arrival[num_in_q] = simclock;
    } else {
    /* server is idle, delay = 0.0 */
        delay = 0.0;
        total_of_delays += delay;
        ++num_custs_delayed;
        server_status = BUSY;
        time_next_event[2] = simclock + expon(mean_service);
    }
}

void depart(void) {
    int i;
    float delay ;
    if (num_in_q == 0) {
        server_status = IDLE;
        time_next_event[2] = INT_MAX;
    } else {
        --num_in_q;
        delay = simclock - time_arrival[1];
        total_of_delays +=delay;
        ++num_custs_delayed;
        time_next_event[2] = simclock + expon(mean_service);
        for (i=1; i <= num_in_q; ++i)
            time_arrival[i] = time_arrival[i+1];
        }
}

void update_time_avg_stats(void) {
    float time_since_last_event;
    time_since_last_event = simclock - time_last_event;
    time_last_event = simclock;
    area_num_in_q += num_in_q * time_since_last_event;
    area_server_status += server_status * time_since_last_event;
}

void report(void) {
    printf("\n\nAverage delay in Queue \t%11.3f\n",total_of_delays / num_custs_delayed);
    printf("Average number in Queue\t%11.3f\n",area_num_in_q / simclock);
    printf("Server utilization \t%11.3f \n",area_server_status/ simclock);
    printf("End of simulation \t%11.3f \n",simclock);
}

float expon(float mean) {
    float u;
    u = (float) rand()/ (float) INT_MAX;
    return (-mean * log(1-u));
}