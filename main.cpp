#include <bits/stdc++.h>

using namespace std;

// Define Constants
const int NUM_OF_STATION = 3;
const int MAX_CAPACITY = 20;
const int AVG_ARRIVAL_TIME_STATION_1 = 14; // avg per hour
const int AVG_ARRIVAL_TIME_STATION_2 = 10; // avg per hour
const int AVG_ARRIVAL_TIME_STATION_3 = 24; // avg per hour
const double MAX_TIME = 80 * 60.0; // minutes
const double BUS_SPEED = 30.0 / 60.0; // 30 miles per hour --> speed per minutes

// For Random Functions
mt19937 rng32(chrono::steady_clock::now().time_since_epoch().count());
unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
default_random_engine generator(seed);
uniform_real_distribution<> dis(0, 1);

int get_uniform_int_distribution(int low, int high) {
    uniform_int_distribution<int> distribution(low, high);
    return distribution(generator);
}

double expon(double mean) {
    return min(-mean * log(dis(rng32)), 80.0);
}

// Simulation Clock
double simclock = 0.0; // current time in simulation (minutes)

// Define Data Structures with structs
struct Person {
    double arrival_time; // generated with expon
    double departure_time = MAX_TIME;

    double bus_in_time;
    double bus_out_time = MAX_TIME;

    int arrival_location;
    int destination; // (if arrival_location = 3, generate destination = 1 or 2 with probability)
    
    Person() : arrival_time(0.0), arrival_location(0), destination(0) {}
    Person(double arrTime, int loc, int dest) : arrival_time(arrTime), arrival_location(loc), destination(dest) {}
};

struct Station {
    int id;
    deque<Person> queue_line;
    deque<Person> departed;

    Person depart(double time_of_departure) {
        assert(!queue_line.empty());
        Person temp = queue_line.front();
        queue_line.pop_front();
        temp.departure_time = time_of_departure;
        departed.push_back(temp);

        return temp;
    }
} station[NUM_OF_STATION + 1];

struct Bus {
    deque<Person> passengers[NUM_OF_STATION + 1];
    deque<Person> arrived;
    int place; // where the bus is at the moment (ex. at terminal 3)
    double min_time_at_station[NUM_OF_STATION + 1];
    double max_time_at_station[NUM_OF_STATION + 1];
    double avg_time_at_station[NUM_OF_STATION + 1];
    double num_at_station[NUM_OF_STATION + 1];

    Bus() {
        for (int i = 1; i <= NUM_OF_STATION; i++) {
            min_time_at_station[i] = INT_MAX;
            max_time_at_station[i] = num_at_station[i] = avg_time_at_station[i] = 0;
        }
    }

    int get_num_of_passengers() {
        int ret = 0;
        for (int i = 1; i <= NUM_OF_STATION; i++) {
            ret += passengers[i].size();
        }
        return ret;
    }

    void unload(Person& temp, double arrived_time){
        temp.bus_out_time = arrived_time;
        arrived.push_back(temp);
    }
    
    void transit(Station& st) {
        double initialclock = simclock;
        double mxclock = simclock + 5.0;
        int st_num = st.id;
        
        // unload
        while (!passengers[st_num].empty() && simclock < MAX_TIME) {
            double unload_time = ((double) get_uniform_int_distribution(16, 24)) / 60.0;
            unload(passengers[st_num].front(), simclock);
            passengers[st_num].pop_front();

            simclock += unload_time;
            mxclock = max(mxclock, simclock);
        }
        // load
        while (get_num_of_passengers() < MAX_CAPACITY && !st.queue_line.empty() && st.queue_line.front().arrival_time <= mxclock && simclock < MAX_TIME) {
            double load_time = ((double) get_uniform_int_distribution(15, 25)) / 60.0;
            Person temp = st.depart(simclock);
            simclock += load_time;
            mxclock = max(mxclock, simclock);

            passengers[temp.destination].push_back(temp);
        }

        simclock = mxclock;

        double time_in_station = simclock - initialclock;
        min_time_at_station[st_num] = min(min_time_at_station[st_num], time_in_station);
        max_time_at_station[st_num] = max(min_time_at_station[st_num], time_in_station);
        avg_time_at_station[st_num] = (avg_time_at_station[st_num] * num_at_station[st_num] + time_in_station) / (num_at_station[st_num] + 1);
        num_at_station[st_num]++;
    }
} bus;

void initialize() {
    // create queue_line of station 1
    station[1].id = 1;
    station[2].id = 2;
    station[3].id = 3;
    {
        double last_time = 0.0;
        while (1) {
            double inter = expon((double) 1.0 / AVG_ARRIVAL_TIME_STATION_1) * 60.0;
            if (last_time + inter > MAX_TIME) break;
            last_time += inter;
            int loc = 1;
            int dest = 3;

            station[1].queue_line.push_back(Person(last_time, loc, dest));
        }
    }
    // create queue_line of station 2
    {
        double last_time = 0.0;
        while (1) {
            double inter = expon((double) 1.0 / AVG_ARRIVAL_TIME_STATION_2) * 60.0;
            if (last_time + inter > MAX_TIME) break;
            last_time += inter;
            int loc = 2;
            int dest = 3;

            station[2].queue_line.push_back(Person(last_time, loc, dest));
        }
    }
    // create queue_line of station 3
    {
        double last_time = 0.0;
        while (1) {
            double inter = expon((double) 1.0 / AVG_ARRIVAL_TIME_STATION_3) * 60.0;
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
            station[3].queue_line.push_back(Person(last_time, loc, dest));
        }
    }
    for (int i = 1; i <= NUM_OF_STATION; i++) {
        cerr << "-----\n";
        cerr << "[S] " << i << " => " << station[i].queue_line.size() << '\n';
        // for (Person& p : station[i].queue_line) {
        //     cerr << "[P] " << p.destination << '\n';
        // }
        // cerr << "-----\n";
    }
}

int main() {
    initialize();
    while (simclock < MAX_TIME) {
        // depart from station 3
        // distance 3 - 1 = 4.5 miles
        {
            double need = 4.5 / BUS_SPEED;
            simclock += need;
        }
        if (simclock >= MAX_TIME) break;
        // arrived at station 1
        bus.transit(station[1]);
        if (simclock >= MAX_TIME) break;

        // depart from station 1
        // distance 1 - 2 = 1 miles
        {
            double need = 1.0 / BUS_SPEED;
            simclock += need;
        }
        if (simclock >= MAX_TIME) break;

        // arrived at station 2
        bus.transit(station[2]);
        if (simclock >= MAX_TIME) break;

        // depart from station 2
        // distance 2 - 3 = 4.5 miles
        {
            double need = 4.5 / BUS_SPEED;
            simclock += need;
        }
        if (simclock >= MAX_TIME) break;
        //arrived at station 3
        bus.transit(station[3]);
        if (simclock >= MAX_TIME) break;
        // cerr << "[+] " << simclock << '\n';
    }
    cerr << "FINISHED";
    // TODO: Add Statistics

    // a1. average number in each queue
    // a2. maximum number in each queue
    // b1. average delay in each queue
    // b2. maximum delay in each queue
    // c1. average number on the bus
    // c2. maximum number on the bus
    // d1. average time the bus is stopped at each location 
    // d2. maximum time the bus is stopped at each location 
    // d3. minimum time the bus is stopped at each location 
    // e1. average time for the bus to make a loop
    // e2. maximum time for the bus to make a loop
    // e3. minimum time for the bus to make a loop
    // f1. average time a person is in the system by arrival location
    // f2. maximum time a person is in the system by arrival location
    // f3. minimum time a person is in the system by arrival location
    


    return 0;    
}