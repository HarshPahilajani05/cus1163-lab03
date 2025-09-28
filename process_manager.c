#include "process_manager.h"

int run_basic_demo(void) {
    int pipe_fd[2];
    pid_t producer_pid, consumer_pid;
    int status;
    printf("\nParent process (PID: %d) creating children...\n", getpid());
    if (pipe(pipe_fd) == -1) {
    perror("pipe");
    return -1;
}


    producer_pid = fork();
if (producer_pid == -1) {
    perror("fork (producer)");
    close(pipe_fd[0]); close(pipe_fd[1]);
    return -1;
}
if (producer_pid == 0) {
    close(pipe_fd[0]);
    producer_process(pipe_fd[1], 1);
}
printf("Producer process started (PID: %d)\n", producer_pid);

    consumer_pid = fork();
if (consumer_pid == -1) {
    perror("fork (consumer)");
    close(pipe_fd[0]); close(pipe_fd[1]);
    return -1;
}
if (consumer_pid == 0) {
    close(pipe_fd[1]);
    consumer_process(pipe_fd[0], 0);
}
printf("Consumer process started (PID: %d)\n", consumer_pid);


close(pipe_fd[0]);
close(pipe_fd[1]);

if (waitpid(producer_pid, &status, 0) > 0) {
    printf("Producer %d finished with code %d\n",
           producer_pid, WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}
if (waitpid(consumer_pid, &status, 0) > 0) {
    printf("Consumer %d finished with code %d\n",
           consumer_pid, WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}


    return 0;
}
int run_multiple_pairs(int num_pairs) {
    pid_t pids[10];
    int pid_count = 0;
    int status;

    printf("\nParent creating %d producer-consumer pairs...\n", num_pairs);

    for (int i = 0; i < num_pairs; i++) {
    printf("\n--- Starting pair %d ---\n", i + 1);

    int fds[2];
    if (pipe(fds) == -1) { perror("pipe"); return -1; }

    pid_t p = fork();
    if (p == -1) { perror("fork (producer)"); close(fds[0]); close(fds[1]); return -1; }
    if (p == 0) {
        close(fds[0]);
        int start = i * NUM_VALUES + 1;
        producer_process(fds[1], start);
    }
    pids[pid_count++] = p;

    pid_t c = fork();
    if (c == -1) { perror("fork (consumer)"); close(fds[0]); close(fds[1]); return -1; }
    if (c == 0) {
        close(fds[1]);
        consumer_process(fds[0], i + 1);
    }
    pids[pid_count++] = c;

    close(fds[0]);
    close(fds[1]);
}


    for (int i = 0; i < pid_count; i++) {
    if (waitpid(pids[i], &status, 0) > 0) {
        printf("Child %d has completed (exit code %d)\n",
       pids[i], WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    }
}
printf("\nAll pairs completed successfully!\n");

    return 0;
}


void producer_process(int write_fd, int start_num) {
    printf("[Producer %d] launched\n", getpid());


    for (int i = 0; i < NUM_VALUES; i++) {
        int number = start_num + i;

        if (write(write_fd, &number, sizeof(number)) != sizeof(number)) {
            perror("write");
            exit(1);
        }

       printf("[Producer %d] wrote %d into the pipe\n", getpid(), number);
        usleep(100000);
    }

    printf("[Producer %d] done sending values\n", NUM_VALUES);
    close(write_fd);
    exit(0);
}

void consumer_process(int read_fd, int pair_id) {
    int number;
    int count = 0;
    int sum = 0;

    printf("[Consumer %d] running\n", getpid());


    while (read(read_fd, &number, sizeof(number)) > 0) {
        count++;
        sum += number;
        printf("[Consumer %d] got %d -> sum now %d\n", getpid(), number, sum);
    }

    printf("[Consumer %d] finished, total = %d\n", getpid(), sum);
    close(read_fd);
    exit(0);
}
