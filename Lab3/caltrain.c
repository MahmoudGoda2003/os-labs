#include <pthread.h>
#include "caltrain.h"



void
station_init(struct station *station)
{
    station->trainPassengers = 0;
    station->waitingPassengers = 0;
    pthread_mutex_init(&station->lockStation, NULL);
    pthread_cond_init(&station->noPassengers, NULL);
    pthread_cond_init(&station->train, NULL);
}

void
station_load_train(struct station *station, int count)
{
    pthread_mutex_lock(&station->lockStation);
    station->emptySeats = count;
    while(station->emptySeats > 0 && station->waitingPassengers > 0){
        pthread_cond_broadcast(&station->train);
        pthread_cond_wait(&station->noPassengers,&station->lockStation);
    }
    station->emptySeats = 0;
    pthread_mutex_unlock(&station->lockStation);
}

void
station_wait_for_train(struct station *station)
{
    pthread_mutex_lock(&station->lockStation);
    station->waitingPassengers++;
    while(station->emptySeats == station->trainPassengers){
        pthread_cond_wait(&station->train,&station->lockStation);
    }
    station->waitingPassengers--;
    station->trainPassengers++;
    pthread_mutex_unlock(&station->lockStation);
}

void
station_on_board(struct station *station)
{
    pthread_mutex_lock(&station->lockStation);
    station->emptySeats--;
    station->trainPassengers--;
    if(station->emptySeats == 0 || (station->waitingPassengers == 0 && station->trainPassengers == 0)){
        pthread_cond_signal(&station->noPassengers);
    }
    pthread_mutex_unlock(&station->lockStation);
}