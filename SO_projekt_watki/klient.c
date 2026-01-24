#include "klient.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t shutdown_requested = 0;

static int last_member_eaten[4] = {0};
static int last_member_count = 0;
static int last_member_valid = 0;

static void klient_obsluz_sigterm(int signo)
{
    (void)signo;
    shutdown_requested = 1;
}

static struct Grupa inicjalizuj_grupe(void)
{
    struct Grupa g;
    g.proces_id = getpid();
    g.osoby = rand() % 4 + 1;
    g.dorosli = rand() % g.osoby + 1;
    g.dzieci = g.osoby - g.dorosli;
    g.stolik_przydzielony = -1;
    g.vip = (rand() % 100 < 2);
    g.wejscie = time(NULL);
    memset(g.pobrane_dania, 0, sizeof(g.pobrane_dania));
    g.danie_specjalne = 0;
    return g;
}

static void usadz_grupe_vip(struct Grupa *g)
{
    int log_usadzono = 0;
    int log_numer_stolika = 0;
    int log_zajete = 0;
    int log_pojemnosc = 0;

    sem_operacja(SEM_STOLIKI, -1);
    int i = znajdz_stolik_dla_grupy_zablokowanej(g);
    if (i >= 0)
    {
        stoliki[i].grupy[stoliki[i].liczba_grup] = *g;
        stoliki[i].zajete_miejsca += g->osoby;
        stoliki[i].liczba_grup++;
        log_usadzono = 1;
        log_numer_stolika = stoliki[i].numer_stolika;
        log_zajete = stoliki[i].zajete_miejsca;
        log_pojemnosc = stoliki[i].pojemnosc;
        g->stolik_przydzielony = i;
    }
    sem_operacja(SEM_STOLIKI, 1);

    if (log_usadzono)
        LOGI("Grupa VIP %d usadzona: %d osób (dorosłych: %d, dzieci: %d) przy stoliku: %d (miejsc zajete: %d/%d)\n",
             g->proces_id,
             g->osoby,
             g->dorosli,
             g->dzieci,
             log_numer_stolika,
             log_zajete,
             log_pojemnosc);
}

static int czekaj_na_przydzial_stolika(struct Grupa *g)
{
    kolejka_dodaj(*g);
    LOGI("Grupa %d dodana do kolejki: %d osób (dorosłych: %d, dzieci: %d)%s\n",
         g->proces_id,
         g->osoby,
         g->dorosli,
         g->dzieci,
         g->vip ? " [VIP]" : "");

    pid_t moj_proces_id = g->proces_id;
    while (g->stolik_przydzielony == -1 && *restauracja_otwarta && !shutdown_requested)
    {
        int log_znaleziono = 0;
        int log_numer_stolika = 0;
        sem_operacja(SEM_STOLIKI, -1);
        for (int i = 0; i < MAX_STOLIKI; i++)
        {
            for (int j = 0; j < stoliki[i].liczba_grup; j++)
            {
                if (stoliki[i].grupy[j].proces_id == moj_proces_id)
                {
                    g->stolik_przydzielony = i;
                    log_znaleziono = 1;
                    log_numer_stolika = stoliki[i].numer_stolika;
                    break;
                }
            }
            if (g->stolik_przydzielony != -1)
                break;
        }
        sem_operacja(SEM_STOLIKI, 1);

        if (log_znaleziono)
            LOGI("Grupa %d znalazała swój stolik: %d\n", g->proces_id, log_numer_stolika);
        rest_sleep(1);
    }

    if (g->stolik_przydzielony == -1)
    {
        LOGI("Grupa %d opuszcza kolejkę - restauracja zamknięta\n", g->proces_id);
        return -1;
    }
    return 0;
}

static void zamow_specjalne_jesli_trzeba(struct Grupa *g, int *dania_do_pobrania, time_t *czas_start_dania, int timeout_dania)
{
    if (time(NULL) - *czas_start_dania <= timeout_dania)
        return;
    if (g->danie_specjalne != 0)
        return;

    int ceny[] = {p40, p50, p60};
    int c = ceny[rand() % 3];
    g->danie_specjalne = c;
    (*dania_do_pobrania)++;

    sem_operacja(SEM_STOLIKI, -1);
    for (int j = 0; j < stoliki[g->stolik_przydzielony].liczba_grup; j++)
    {
        if (stoliki[g->stolik_przydzielony].grupy[j].proces_id == g->proces_id)
        {
            stoliki[g->stolik_przydzielony].grupy[j].danie_specjalne = c;
            break;
        }
    }
    sem_operacja(SEM_STOLIKI, 1);

    LOGI("Grupa %d zamawia danie specjalne za: %d zł. \n", g->proces_id, g->danie_specjalne);
    *czas_start_dania = time(NULL);
}

static TakeDishResult sprobuj_pobrac_danie(struct Grupa *g, int *dania_pobrane, int dania_do_pobrania, time_t *czas_start_dania)
{
    int log_pobrano = 0;
    int log_cena = 0;
    int log_numer_stolika = g->stolik_przydzielony + 1;
    int log_pobrane = 0;
    int log_do_pobrania = dania_do_pobrania;
    pid_t log_pid = g->proces_id;

    sem_operacja(SEM_TASMA, -1);
    if (tasma[g->stolik_przydzielony].cena != 0)
    {
        int numer_stolika = g->stolik_przydzielony + 1;
        if (tasma[g->stolik_przydzielony].stolik_specjalny != 0 &&
            tasma[g->stolik_przydzielony].stolik_specjalny != numer_stolika)
        {
            sem_operacja(SEM_TASMA, 1);
            return TAKE_SKIPPED_OTHER_TABLE;
        }

        int cena = tasma[g->stolik_przydzielony].cena;
        int idx = cena_na_indeks(cena);
        if (idx >= 0)
            g->pobrane_dania[idx]++;

        (*dania_pobrane)++;
        log_pobrano = 1;
        log_cena = cena;
        log_pobrane = *dania_pobrane;

        tasma[g->stolik_przydzielony].cena = 0;
        tasma[g->stolik_przydzielony].stolik_specjalny = 0;
        *czas_start_dania = time(NULL);

        sem_operacja(SEM_TASMA, 1);

        if (log_pobrano)
            LOGI("Grupa %d przy stoliku %d pobrała danie za %d zł (pobrane: %d/%d)\n",
                 log_pid,
                 log_numer_stolika,
                 log_cena,
                 log_pobrane,
                 log_do_pobrania);
        return TAKE_TAKEN;
    }

    sem_operacja(SEM_TASMA, 1);
    return TAKE_NONE;
}

static void meal_ctx_init(struct MealCtx *ctx, struct Grupa *g, int dania_do_pobrania, int timeout_dania, int allow_special)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->grupa = g;
    pthread_mutex_init(&ctx->lock, NULL);
    pthread_cond_init(&ctx->seated_cv, NULL);
    ctx->seated = 0;
    ctx->done = 0;
    ctx->dania_do_pobrania = dania_do_pobrania;
    ctx->dania_pobrane = 0;
    memset(ctx->eaten_per_member, 0, sizeof(ctx->eaten_per_member));
    ctx->czas_start_dania = time(NULL);
    ctx->czas_ostatniego_dania = ctx->czas_start_dania;
    ctx->timeout_dania = timeout_dania;
    ctx->allow_special = allow_special;
}

static void meal_ctx_destroy(struct MealCtx *ctx)
{
    pthread_cond_destroy(&ctx->seated_cv);
    pthread_mutex_destroy(&ctx->lock);
}

static void meal_mark_done(struct MealCtx *ctx)
{
    pthread_mutex_lock(&ctx->lock);
    ctx->done = 1;
    pthread_mutex_unlock(&ctx->lock);
}

static int meal_is_done(struct MealCtx *ctx)
{
    pthread_mutex_lock(&ctx->lock);
    int done = ctx->done || (ctx->dania_pobrane >= ctx->dania_do_pobrania);
    pthread_mutex_unlock(&ctx->lock);
    return done;
}

static void *czlonek_grupy_thread(void *arg) // wątek członka grupy klientów
{
    struct MemberArg *marg = (struct MemberArg *)arg; // argumenty wątku
    struct MealCtx *ctx = marg->ctx;
    int member_index = marg->member_index;

    pthread_mutex_lock(&ctx->lock);
    while (!ctx->seated && !shutdown_requested && *restauracja_otwarta)
    {
        pthread_cond_wait(&ctx->seated_cv, &ctx->lock);
    }
    pthread_mutex_unlock(&ctx->lock);

    while (!shutdown_requested && *restauracja_otwarta)
    {
        if (meal_is_done(ctx))
            break;

        // Wątki-członkowie pobierają dania; zamówienia specjalne zostają po stronie przedstawiciela.
        int dania_pobrane_local = 0;
        time_t czas_start_local = time(NULL);
        TakeDishResult take_res = sprobuj_pobrac_danie(ctx->grupa, &dania_pobrane_local, 1, &czas_start_local);
        if (take_res == TAKE_TAKEN)
        {
            pthread_mutex_lock(&ctx->lock);
            ctx->dania_pobrane++;
            ctx->czas_ostatniego_dania = time(NULL);
            ctx->czas_start_dania = ctx->czas_ostatniego_dania;
            if (member_index >= 0 && member_index < 4)
                ctx->eaten_per_member[member_index]++;
            int local_cnt = (member_index >= 0 && member_index < 4) ? ctx->eaten_per_member[member_index] : 0;
            pthread_mutex_unlock(&ctx->lock);

            // Lokalny log wątku członka (niezależny od grupowego logu w sprobuj_pobrac_danie()).
            LOGI("Grupa %d: członek %d pobrał danie (łącznie członka: %d)\n",
                 ctx->grupa->proces_id,
                 member_index + 1,
                 local_cnt);
        }

        rest_sleep(1);
    }

    return NULL;
}

static void zaplac_za_dania(const struct Grupa *g)
{
    LOGI("Grupa %d przy stoliku %d gotowa do płatności\n", g->proces_id, g->stolik_przydzielony + 1);

    int log_ilosc[6] = {0};
    int log_cena[6] = {0};
    int log_kwota[6] = {0};

    sem_operacja(SEM_TASMA, -1);
    for (int i = 0; i < 6; i++)
    {
        if (g->pobrane_dania[i] == 0)
            continue;
        int kwota = g->pobrane_dania[i] * CENY_DAN[i];
        kasa_dania_sprzedane[i] += g->pobrane_dania[i];

        log_ilosc[i] = g->pobrane_dania[i];
        log_cena[i] = CENY_DAN[i];
        log_kwota[i] = kwota;
    }
    sem_operacja(SEM_TASMA, 1);

    for (int i = 0; i < 6; i++)
    {
        if (log_ilosc[i] == 0)
            continue;
        LOGI("Grupa %d płaci za %d dań za %d zł każde, łącznie: %d zł\n",
             g->proces_id,
             log_ilosc[i],
             log_cena[i],
             log_kwota[i]);
    }
}

static void opusc_stolik(const struct Grupa *g)
{
    pid_t log_pid = g->proces_id;
    int log_numer_stolika = g->stolik_przydzielony + 1;

    sem_operacja(SEM_STOLIKI, -1);
    for (int j = 0; j < stoliki[g->stolik_przydzielony].liczba_grup; j++)
    {
        if (stoliki[g->stolik_przydzielony].grupy[j].proces_id == g->proces_id)
        {
            for (int k = j; k < stoliki[g->stolik_przydzielony].liczba_grup - 1; k++)
            {
                stoliki[g->stolik_przydzielony].grupy[k] = stoliki[g->stolik_przydzielony].grupy[k + 1];
            }
            memset(&stoliki[g->stolik_przydzielony].grupy[stoliki[g->stolik_przydzielony].liczba_grup - 1], 0, sizeof(struct Grupa));
            stoliki[g->stolik_przydzielony].liczba_grup--;
            stoliki[g->stolik_przydzielony].zajete_miejsca -= g->osoby;
            break;
        }
    }
    sem_operacja(SEM_STOLIKI, 1);

    LOGI("Grupa %d przy stoliku %d opuszcza restaurację.\n", log_pid, log_numer_stolika);
}

static void petla_czekania_na_dania(struct Grupa *g)
{
    // Jeden proces = jedna grupa. Każdy członek grupy jest osobnym wątkiem.
    // Przedstawiciel (wątek główny) zarządza: wejściem do kolejki, zamówieniami specjalnymi i płatnością.
    int dania_do_pobrania = rand() % 8 + 3;
    int timeout_dania = 5;
    struct MealCtx ctx;
    meal_ctx_init(&ctx, g, dania_do_pobrania, timeout_dania, 1);

    int liczba_osob = g->osoby;
    int liczba_watkow_pobocznych = (liczba_osob > 1) ? (liczba_osob - 1) : 0;
    pthread_t watki[4];
    struct MemberArg args[4];
    for (int i = 0; i < liczba_watkow_pobocznych; i++)
    {
        args[i].ctx = &ctx;
        args[i].member_index = i + 1; // 1..N-1
        if (pthread_create(&watki[i], NULL, czlonek_grupy_thread, &args[i]) != 0)
        {
            // Jeśli nie udało się stworzyć wątku, kontynuuj w trybie bez-wątkowym.
            liczba_watkow_pobocznych = i;
            break;
        }
    }

    // Sygnalizuj członkom, że stolik jest przydzielony.
    pthread_mutex_lock(&ctx.lock);
    ctx.seated = 1;
    pthread_cond_broadcast(&ctx.seated_cv);
    pthread_mutex_unlock(&ctx.lock);

    while (!shutdown_requested && *restauracja_otwarta)
    {
        pthread_mutex_lock(&ctx.lock);
        int done = ctx.done || (ctx.dania_pobrane >= ctx.dania_do_pobrania);
        time_t last_dish = ctx.czas_ostatniego_dania;
        time_t start_wait = ctx.czas_start_dania;
        int current_target = ctx.dania_do_pobrania;
        pthread_mutex_unlock(&ctx.lock);

        if (done)
            break;

        // Timeout całej grupy, jeśli długo nie udało się pobrać żadnego dania.
        if (time(NULL) - last_dish > timeout_dania * 4)
        {
            LOGI("Grupa %d timeout czekania na dania - kończy się\n", g->proces_id);
            meal_mark_done(&ctx);
            break;
        }

        // Przedstawiciel zamawia danie specjalne (tylko w wątku głównym).
        time_t ts = start_wait;
        zamow_specjalne_jesli_trzeba(g, &current_target, &ts, timeout_dania);
        if (current_target != dania_do_pobrania)
        {
            pthread_mutex_lock(&ctx.lock);
            ctx.dania_do_pobrania = current_target;
            ctx.czas_start_dania = ts;
            pthread_mutex_unlock(&ctx.lock);
            dania_do_pobrania = current_target;
        }

        // Przedstawiciel również „uczestniczy” w pobieraniu dań.
        int pobrano_dummy = 0;
        time_t czas_dummy = time(NULL);
        TakeDishResult take_res = sprobuj_pobrac_danie(g, &pobrano_dummy, 1, &czas_dummy);
        if (take_res == TAKE_TAKEN)
        {
            pthread_mutex_lock(&ctx.lock);
            ctx.dania_pobrane++;
            ctx.czas_ostatniego_dania = time(NULL);
            ctx.czas_start_dania = ctx.czas_ostatniego_dania;
            ctx.eaten_per_member[0]++;
            int local_cnt = ctx.eaten_per_member[0];
            pthread_mutex_unlock(&ctx.lock);

            // Lokalny log przedstawiciela.
            LOGI("Grupa %d: przedstawiciel pobrał danie (łącznie przedstawiciela: %d)\n",
                 g->proces_id,
                 local_cnt);
        }

        rest_sleep(1);
    }

    meal_mark_done(&ctx);
    for (int i = 0; i < liczba_watkow_pobocznych; i++)
    {
        (void)pthread_join(watki[i], NULL);
    }

    // Zachowaj wynik per-członek do późniejszego wypisania przy opuszczaniu restauracji.
    pthread_mutex_lock(&ctx.lock);
    last_member_count = g->osoby;
    for (int i = 0; i < 4; i++)
        last_member_eaten[i] = ctx.eaten_per_member[i];
    last_member_valid = 1;
    pthread_mutex_unlock(&ctx.lock);

    meal_ctx_destroy(&ctx);
}

static void wypisz_kto_ile_zjadl(const struct Grupa *g)
{
    if (!last_member_valid || last_member_count <= 0)
        return;

    LOGI("Grupa %d: podsumowanie spożycia (kto ile zjadł)\n", g->proces_id);
    int suma = 0;
    for (int i = 0; i < last_member_count && i < 4; i++)
    {
        suma += last_member_eaten[i];
        if (i == 0)
            LOGI("  przedstawiciel: %d\n", last_member_eaten[i]);
        else
            LOGI("  członek %d: %d\n", i + 1, last_member_eaten[i]);
    }
    LOGI("  razem: %d\n", suma);
}

void klient(void)
{
    if (signal(SIGTERM, klient_obsluz_sigterm) == SIG_ERR)
        LOGE_ERRNO("signal(SIGTERM)");

    struct Grupa g = inicjalizuj_grupe();

    if (g.vip)
    {
        usadz_grupe_vip(&g);
    }
    else
    {
        if (czekaj_na_przydzial_stolika(&g) != 0)
            exit(0);
    }

    if (shutdown_requested || !*restauracja_otwarta)
    {
        if (g.stolik_przydzielony != -1)
        {
            wypisz_kto_ile_zjadl(&g);
            opusc_stolik(&g);
        }
        exit(0);
    }

    petla_czekania_na_dania(&g);

    if (shutdown_requested || !*restauracja_otwarta)
    {
        wypisz_kto_ile_zjadl(&g);
        opusc_stolik(&g);
        exit(0);
    }

    zaplac_za_dania(&g);
    wypisz_kto_ile_zjadl(&g);
    opusc_stolik(&g);

    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        LOGE("Użycie: %s <shm_id> <sem_id> <msgq_id>\n", argv[0]);
        return 1;
    }

    int shm = parsuj_int_lub_zakoncz("shm_id", argv[1]);
    int sem = parsuj_int_lub_zakoncz("sem_id", argv[2]);
    msgq_id = parsuj_int_lub_zakoncz("msgq_id", argv[3]);
    dolacz_ipc(shm, sem);
    zainicjuj_losowosc();
    klient();
    return 0;
}
