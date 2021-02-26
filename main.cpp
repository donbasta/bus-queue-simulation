#include <bits/stdc++.h>
#pragma GCC target ("avx2")
#pragma GCC optimization ("O3")
#pragma GCC optimization ("unroll-loops")


using namespace std;

// For random seeds


// Define Constants
const int NUM_OF_STATION = 3;
const int MAX_NUM_OF_PASSENGER = 20;
const int AVG_ARRIVAL_TIME_STATION_1 = 14;
const int AVG_ARRIVAL_TIME_STATION_2 = 10;
const int AVG_ARRIVAL_TIME_STATION_3 = 24;
const double MAX_TIME = 80;


Station station[NUM_OF_STATION + 1];

int get_uniform_int_distribution(int low, int high) {
    default_random_engine generator;
    uniform_int_distribution<int> distribution(low, high);
    return distribution(generator);
}

mt19937 rng32(chrono::steady_clock::now().time_since_epoch().count());
uniform_real_distribution<> dis(0, 1);

double expon(double mean) {
    return min(-mean * log(dis(rng32)), MAX_TIME);
}

double simclock = 0.0; // current time in simulation

// Define Data Structures with structs
struct Person {
    double arrival_time; // di generate pake expon
    int arrival_location;
    int destination; // (kalau arrival_location = 3, generate destination = 1 atau 2 pake probability)
    
    Person() : arrival_time(0.0), arrival_location(0), destination(0) {}
    Person(double arrTime, int loc, int dest) : arrival_time(arrTime), arrival_location(loc), destination(dest) {}
};

struct Bus {
    deque<person> passengers[3];
    int num_of_passenger;
    int place; // where the bus is at the moment (ex. at terminal 3)
    double min_time_at_place[3];
    double max_time_at_place[3];
    double avg_time_at_place[3];
    double num_at_place[3];
    
    double unload() {
        double timer = 0.0;
        while (timer < 5.0) {
            if (passengers[place].empty()) break;
            int unload_time = get_uniform_int_distribution(16, 24);
            simclock += unload_time;
            passengers[place].pop_front();
            timer += unload_time;
            num_of_passenger--;
        }
        return timer;
    }

    double load(double timer) {
        while (num_of_passenger < MAX_NUM_OF_PASSENGER) {
            if (timer >= 5.0) break;
            if (station[place].empty()) break;
            Person currentPerson = station[place].front();
            double delay = simclock - currentPerson.arrival_time;
            total_delay[place] += delay;
            max_delay[place] = max(max_delay[place], delay);

            int load_time = get_uniform_int_distribution(15, 25);
            simclock += load_time;
            timer += load_time;
            passengers.pop_front();

            num_of_passenger++;
        }
        return timer;
    }

    void stop() {
        double timer = unload();
        if (timer < 5.0) {
            timer = load(timer);
        }
        timer = max(5.0, timer);
        min_time_at_place[place] = min(min_time_at_place[place], timer);
        max_time_at_place[place] = max(min_time_at_place[place], timer);
        avg_time_at_place[place] = (avg_time_at_place[place] * num_at_place[place] + timer) / (num_at_place[place] + 1);
        num_at_place[place]++;
    }
};

struct Station {
    deque<Person> queue_line;
};

void initialize() {
    // create queue_line of station 1
    {
        double last_time = 0.0;
        while (1) {
            double inter = expon(AVG_ARRIVAL_TIME_STATION_1);
            if (last_time + inter > MAX_TIME) break;
            last_time += inter;
            int loc = 1;
            int dest = 3;

            station[1].push_back(Person(last_time, loc, dest));
        }
    }
    // create queue_line of station 2
    {
        double last_time = 0.0;
        while (1) {
            double inter = expon(AVG_ARRIVAL_TIME_STATION_2);
            if (last_time + inter > MAX_TIME) break;
            last_time += inter;
            int loc = 2;
            int dest = 3;

            station[2].push_back(Person(last_time, loc, dest));
        }
    }
    // create queue_line of station 3
    {
        double last_time = 0.0;
        while (1) {
            double inter = expon(AVG_ARRIVAL_TIME_STATION_3);
            if (last_time + inter > MAX_TIME) break;
            last_time += inter;
            int loc = 3;
            int dest;
            {
                int temp = get_uniform_int_distribution(1, 1000);
                if (temp <= 583) { // 0.583 chance to go to station 1
                    dest = 1;
                } else {
                    dest = 2;
                }
            }
            station[3].push_back(Person(last_time, loc, dest));
        }
    }
}

// void initialize() {
//     simclock = 0.0;
    
//     num_q_in_terminal_1 = 0;
//     num_q_in_terminal_2 = 0;
//     num_q_in_terminal_3 = 0;
//     time_last_event = 0.0;

//     total_delay

//     num_custs_delayed = 0;
//     total_of_delays = 0.0;
//     area_num_in_q = 0.0;
//     area_server_status = 0.0;
    
//     time_next_event[1] = simclock + expon(mean_interarrival);
//     time_next_event[2] = INT_MAX;
// }

// void timing() {
//     int i;
//     float min_time_next_event = INT_MAX;
//     next_event_type = 0;
//     /* Determine the event type of the next event to occur */
//     for (i=1; i<= num_events; i++) {
//         if (time_next_event[i] < min_time_next_event) {
//             min_time_next_event = time_next_event[i];
//             next_event_type = i;
//         }
//     }
//     if (next_event_type == 0) {
//     /* The event list is empty so stop the simulation */
//         exit(1);
//     }
//     simclock = min_time_next_event;
// }

int main() {


    return 0;    
}