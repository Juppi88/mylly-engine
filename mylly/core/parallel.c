#include "parallel.h"
#include "mylly.h"
#include "memory.h"
#include "collections/list.h"
#include "platform/thread.h"

// -------------------------------------------------------------------------------------------------

typedef struct job_t {

	list_entry(job_t);

	job_execute_t execute;
	job_completed_t completed;
	void *context;

} job_t;

// -------------------------------------------------------------------------------------------------

static lock_t job_sync; // Used to control access to the parallel job list
static list_t(job_t) jobs; // A linked list of parallel jobs to be completed
static list_t(job_t) completed; // A list of completed jobs
static bool is_thread_running; // Status flag for the worker thread

// -------------------------------------------------------------------------------------------------

THREAD(parallel_worker_thread);

// -------------------------------------------------------------------------------------------------

void parallel_initialize(void)
{
	list_init(jobs);
	list_init(completed);

	// Initialize a sync object.
	thread_init_lock(&job_sync);

	// Start a worker thread.
	is_thread_running = true;
	thread_create(parallel_worker_thread, NULL);
}

void parallel_shutdown(void)
{
	is_thread_running = false;

	// Give the worker thread a while to shut down.
	thread_sleep(100);

	// Destroy the sync object.
	thread_lock(&job_sync);
	thread_destroy_lock(&job_sync);

	// Remove all remaining jobs from the list.
	job_t *job;

	list_foreach(jobs, job) {
		mem_free(job);
	}

	list_foreach(completed, job) {
		mem_free(job);
	}
}

void parallel_process(void)
{
	// Dispatch and destroy all completed jobs.
	thread_lock(&job_sync);
	{
		job_t *job, *tmp;
		list_foreach_safe(completed, job, tmp) {

			if (job->completed != NULL) {
				job->completed(job->context);
			}

			list_remove(completed, job);
			mem_free(job);
		}
	}
	thread_unlock(&job_sync);
}

void parallel_submit_job(job_execute_t execute, job_completed_t completed, void *context)
{
	NEW(job_t, job);
	list_entry_init(job);

	job->execute = execute;
	job->completed = completed;
	job->context = context;

	// Push the new job to the work queue.
	thread_lock(&job_sync);
	{
		list_push(jobs, job);
	}
	thread_unlock(&job_sync);
}

THREAD(parallel_worker_thread)
{
	UNUSED(args);

	for (;;) {
		
		job_t *job;

		do {
			// Pop the first uncompleted job from the queue.
			thread_lock(&job_sync);
			{
				job = jobs.first;

				if (job != NULL) {
					jobs.first = list_next(job);
				}
				else {
					jobs.first = NULL;
				}
			}
			thread_unlock(&job_sync);

			if (job != NULL) {
			
				// Execute the job.
				if (job->execute != NULL) {
					job->execute(job->context);
				}

				// Push the job to the completed queue.
				thread_lock(&job_sync);
				{
					list_push(completed, job);
				}
				thread_unlock(&job_sync);
			}

		} while (job != NULL);

		thread_sleep(1);
	}

	return 0;
}
