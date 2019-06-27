#pragma once
#ifndef __PARALLEL_H
#define __PARALLEL_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef void (*job_execute_t)(void *context);
typedef void (*job_completed_t)(void *context);

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void parallel_initialize(void);
void parallel_shutdown(void);
void parallel_process(void);

void parallel_submit_job(job_execute_t execute, job_completed_t completed, void *context);

END_DECLARATIONS;

#endif
