#ifndef INTERP_H
#define INTERP_H

#include <pico/stdlib.h>

typedef uint64_t InterpCounter;

typedef struct {
    float start, end;
    InterpCounter t, tend;
    bool running;
} Interp;

static inline void interp_start(Interp *i, float start, float end, InterpCounter tend) {
    i->start   = start;
    i->end     = end;
    i->tend    = tend;
    i->t       = 0;
    i->running = true;
}

static inline bool interp_tick(Interp *i, InterpCounter dt) {
    if (!i->running) return false;
    if (i->t >= i->tend) {
        i->running = false;
        return false;
    }

    i->t += dt;

    if (i->t >= i->tend) {
        i->t = i->tend;
    }

    return true;
}

static inline float interp_value(Interp *i) {
    float t = (float)i->t / (float)i->tend;
    return i->start + t * (i->end - i->start);

}

#endif // INTERP_H
