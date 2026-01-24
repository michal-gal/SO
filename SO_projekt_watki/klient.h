#ifndef KLIENT_H
#define KLIENT_H

#include "common.h"

#include <pthread.h>
#include <time.h>

struct MealCtx // kontekst posiłku
{
    struct Grupa *grupa;
    pthread_mutex_t lock;
    pthread_cond_t seated_cv;
    int seated;
    int done;
    int dania_do_pobrania;
    int dania_pobrane;
    int eaten_per_member[4];
    time_t czas_start_dania;
    time_t czas_ostatniego_dania;
    int timeout_dania;
    int allow_special;
};

struct MemberArg // argumenty wątku członka grupy
{
    struct MealCtx *ctx;
    int member_index; // 0=przedstawiciel, 1..N-1=pozostali
};

#endif // KLIENT_H
