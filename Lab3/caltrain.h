#include <pthread.h>

struct station {
    int emptySeats;
    int trainPassengers;
    int waitingPassengers;
    pthread_mutex_t lockStation;
    pthread_cond_t noPassengers;
    pthread_cond_t train;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);