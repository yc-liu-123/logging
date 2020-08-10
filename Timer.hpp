#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <ctime>

struct UTC_timer {
    UTC_timer() : past(0) {
        struct timespec tw;
        clock_gettime(CLOCK_MONOTONIC, &tw);
        base_sec = tw.tv_sec;
        base_nsec = tw.tv_nsec;
        renew();
    }
    void get_curr_time(double& pass) {
        double tmp_past(past), tmp_rel_past(rel_past);
        struct timespec tw;
        clock_gettime(CLOCK_MONOTONIC, &tw);
        double cur_past((tw.tv_sec + tw.tv_nsec * 1e-9) - (base_sec + base_nsec * 1e-9));
        tmp_past += cur_past;
        pass = tmp_past;
        tmp_rel_past += cur_past;
        int int_past(tmp_rel_past);
        if (int_past > 0) {
            int m(incr(&sec, int_past, 60));
            tmp_rel_past -= int_past;
            if (m > 0)
                renew();
            else
                reset_utc_fmt_sufix();
        }
        base_nsec = tw.tv_nsec;
        base_sec = tw.tv_sec;
        rel_past = tmp_rel_past;
        past = tmp_past;
    }
    double past;
    char utc_fmt[20];
private:
    void renew() {
        time_t t = time(NULL);
        struct tm cur_tm;
        localtime_r(&t, &cur_tm);
        year = cur_tm.tm_year + 1900;
        mon = cur_tm.tm_mon + 1;
        day = cur_tm.tm_mday;
        hr = cur_tm.tm_hour;
        min = cur_tm.tm_min;
        sec = cur_tm.tm_sec;
        rel_past = 0;
        reset_utc_fmt();
    }
    int incr(unsigned int* orig, const int passed, const int itv) {
        int w(*orig + passed);
        *orig = w % itv;
        return w / itv;
    }
    void reset_utc_fmt() {
        snprintf(utc_fmt, 20, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", year, mon, day, hr, min, sec);
    }
    void reset_utc_fmt_sufix() {
        snprintf(utc_fmt + 17, 3, "%.2d", sec);
    }
    unsigned int year, mon, day, hr, min, sec;
    uint64_t base_sec, base_nsec;
    double rel_past;
};

#endif