#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "../../include/globals.h"
#include "../../include/logging/data_logger.h"
#include "../../include/match/events.h"
#include "../../include/utils/time_utils.h"

static FILE        *data_file  = NULL;
static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_data_logger(void)
{
    data_file = fopen("data/events.csv", "w");
    if (!data_file)
    {
        perror("Could not open data/events.csv");
        return;
    }
    fprintf(data_file,
            "over,ball,bowler,batsman,event,runs_scored,"
            "score_after,wickets_after,timestamp_ms\n");
    fflush(data_file);
}

void log_csv_event(const char *ev_name)
{
    if (!data_file) return;

    int runs = 0;
    switch (ball_event)
    {
        case SINGLE:  runs = 1; break;
        case DOUBLE:  runs = 2; break;
        case FOUR:    runs = 4; break;
        case SIX:     runs = 6; break;
        case WIDE:    runs = 1; break;
        case NO_BALL: runs = 1; break;
        default:      runs = 0; break;
    }

    long ts = get_time_ms() - match_start_ms;

    pthread_mutex_lock(&data_mutex);
    fprintf(data_file,
            "%d,%d,%d,%d,%s,%d,%d,%d,%ld\n",
            current_over,
            current_ball,
            current_bowler,
            striker,
            ev_name,
            runs,
            global_score,
            wickets,
            ts);
    fflush(data_file);
    pthread_mutex_unlock(&data_mutex);
}

void close_data_logger(void)
{
    if (data_file)
    {
        fflush(data_file);
        fclose(data_file);
        data_file = NULL;
    }
}

void write_gantt_csv(void)
{
    int first_innings = (current_innings == 1);
    FILE *f = fopen("data/gantt_log.csv", first_innings ? "w" : "a");
    if (!f)
    {
        perror("Could not open data/gantt_log.csv");
        return;
    }

    if (first_innings)
    {
        fprintf(f,
                "innings,batting_team,"
                "ball_number,over,ball_in_over,bowler,striker,"
                "event,score_after,wickets_after,timestamp_ms\n");
    }

    for (int i = 0; i < gantt_count; i++)
    {
        BallRecord *r = &gantt_log[i];
        fprintf(f,
                "%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%ld\n",
                r->innings,
                r->batting_team_id,
                r->ball_number,
                r->over,
                r->ball_in_over,
                r->bowler,
                r->striker_id,
                event_name(r->event),
                r->score_after,
                r->wickets_after,
                r->timestamp_ms);
    }

    fflush(f);
    fclose(f);
    printf("[LOG] Gantt data written → data/gantt_log.csv (innings %d)\n",
           current_innings);
}
