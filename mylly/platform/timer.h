#pragma once
#ifndef __TIMER_H
#define __TIMER_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

typedef uint64_t timeout_t;

static INLINE bool timer_has_started(timeout_t *timer) { return (*timer != 0); }
bool timer_has_expired(timeout_t *timer, uint64_t interval_ms);
void timer_reset(timeout_t *timer, uint64_t interval_ms);

float timer_get_factor(timeout_t *timer, uint64_t interval_ms);

uint64_t timer_get_ticks(void);

END_DECLARATIONS;

#endif
