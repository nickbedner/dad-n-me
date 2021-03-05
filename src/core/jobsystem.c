#include "core/jobsystem.h"

int job_worker_init(struct JobWorker *job_worker, int thread_num, struct Game *game_handle) {
  queue_init(&job_worker->task_queue, 2048);
  job_worker->game_handle = game_handle;
  job_worker->thread_num = thread_num;

  mtx_init(&job_worker->mutex, mtx_plain);
  cnd_init(&job_worker->condition);
  thrd_create(&job_worker->thread, job_worker_process_tasks, (void *)job_worker);

  return 0;
}

void job_worker_delete(struct JobWorker *job_worker) {
  queue_delete(&job_worker->task_queue);
}

int job_worker_process_tasks(void *a_arg) {
  struct JobWorker *job_worker = (struct JobWorker *)a_arg;
  for (;;) {
    struct Queue *task_queue = &job_worker->task_queue;
    if (queue_empty(task_queue) == 0) {
      struct Job *job = queue_pop(task_queue);
      (job->job_func)(job->job_type, job->job_data);
      printf("Did job in thread: %d!\n", job_worker->thread_num);
      free(job);
    } else
      job_worker_stop_thread(job_worker);
  }

  return 0;
}

void job_worker_start_thread(struct JobWorker *job_worker) {
  mtx_lock(&job_worker->mutex);
  job_worker->run_thread = 1;
  cnd_signal(&job_worker->condition);
  mtx_unlock(&job_worker->mutex);
}

void job_worker_stop_thread(struct JobWorker *job_worker) {
  mtx_lock(&job_worker->mutex);
  while (job_worker->run_thread == 0)
    cnd_wait(&job_worker->condition, &job_worker->mutex);
  job_worker->run_thread = 0;
  mtx_unlock(&job_worker->mutex);
}

/////////////////////////////////////////////////////////////////////////////

int job_system_init(struct JobSystem *job_system, struct Game *game_handle) {
  job_system->num_workers = engine_get_max_omp_threads();
  job_system->workers = calloc(job_system->num_workers, sizeof(struct JobWorker));
  job_system->round_robin = 0;

  for (int worker_num = 0; worker_num < job_system->num_workers; worker_num++)
    job_worker_init(&job_system->workers[worker_num], worker_num, game_handle);

  return 0;
}

int job_system_delete(struct JobSystem *job_system) {
  for (int worker_num = 0; worker_num < job_system->num_workers; worker_num++)
    job_worker_delete(&job_system->workers[worker_num]);

  free(job_system->workers);
}

void job_system_enqueue(struct JobSystem *job_system, struct Job *job) {
  queue_push(&job_system->workers[job_system->round_robin++].task_queue, job);

  if (job_system->round_robin >= job_system->num_workers)
    job_system->round_robin = 0;
}

void job_system_start_threads(struct JobSystem *job_system) {
  for (int worker_num = 0; worker_num < job_system->num_workers; worker_num++) {
    if (queue_empty(&job_system->workers[worker_num].task_queue) == 0)
      job_worker_start_thread(&job_system->workers[worker_num]);
  }
}

void job_system_wait(struct JobSystem *job_system) {
  for (int worker_num = 0; worker_num < job_system->num_workers; worker_num++) {
    if (queue_size(&job_system->workers[worker_num].task_queue) > 0) {
      sleep(1);
      worker_num = 0;
    }
  }
}
