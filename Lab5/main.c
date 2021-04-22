#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define PROC_COUNT 9
#define STARTING_PROC_ID 1
#define MAX_CHILDS_COUNT 3
#define MAX_USR_COUNT 101

const unsigned char CHILDS_COUNT[PROC_COUNT] =
    {1, 3, 2, 0, 0, 0, 1, 1, 0};

const unsigned char CHILDS_IDS[PROC_COUNT][MAX_CHILDS_COUNT] =
    {{1}, {2, 3, 4}, {5, 6}, {0}, {0}, {0}, {7}, {8}, {0}};

const unsigned char GROUP_TYPE[PROC_COUNT] =
    {0, 1, 0, 2, 2, 0, 2, 0, 0};

const char SEND_TO[PROC_COUNT] =
    {0, -2, -5, 0, 0, 0, 7, 8, 1};

const int SEND_SIGNALS[PROC_COUNT] =
    {0, SIGUSR1, SIGUSR2, 0, 0, 0, SIGUSR1, SIGUSR1, SIGUSR2};

volatile int usr_amount[2][2] =
    {{0, 0}, {0, 0}};

volatile int usr_recv[2] = {0, 0};

const char RECV_SIGNALS_COUNT[2][PROC_COUNT] =
    {{0, 0, 1, 1, 1, 0, 0, 1, 1},
     {0, 1, 0, 0, 0, 1, 1, 0, 0}};

pid_t *pids_list = NULL;
char *exec_name = NULL;
int proc_id = 0;
char *tmp_file_name = "/tmp/pids.log";

void forker(int curr_number, int childs_count);
void print_error_exit(const char *s_name, const char *msg, const int proc_num);
void set_sig_handler(void (*handler)(void *), int sig_no, int flags);
void kill_wait_for_children();
long long current_time();
void wait_for_children();
void sig_handler(int signum);

int main(int argc, char *argv[])
{
    exec_name = basename(argv[0]);
    pids_list = (pid_t *)mmap(pids_list, (2 * PROC_COUNT) * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int i = 0;
    for (i = 0; i < PROC_COUNT; i++)
    {
        pids_list[i] = 0;
    }
    forker(0, CHILDS_COUNT[0]);
    set_sig_handler(&kill_wait_for_children, SIGTERM, 0);

    if (proc_id == 0)
    {
        wait_for_children();
        munmap(pids_list, (2 * PROC_COUNT) * sizeof(pid_t));
        return 0;
    }

    on_exit(&wait_for_children, NULL);
    pids_list[proc_id] = getpid();
    fflush(stdout);

    if (proc_id == STARTING_PROC_ID)
    {
        do
        {
            for (i = 1; (i <= PROC_COUNT) && (pids_list[i] != 0); i++)
            {

                if (pids_list[i] == -1)
                {
                    print_error_exit(exec_name, "Error: not all processes forked or initialized.\nHalting.", 0);
                    exit(1);
                }
            }
        } while (i < PROC_COUNT);

        FILE *tmp = fopen(tmp_file_name, "wt");
        if (tmp == NULL)
        {
            print_error_exit(exec_name, "Can't create temp file", 0);
        }

        for (i = 1; i < PROC_COUNT; ++i)
        {
            fprintf(tmp, "%d\n", pids_list[i]);
        }

        fclose(tmp);

        pids_list[0] = 1;

        set_sig_handler(&sig_handler, 0, 0);

        do
        {
            for (i = 1 + PROC_COUNT; (i < 2 * PROC_COUNT) && (pids_list[i] != 0); ++i)
            {

                if (pids_list[i] == -1)
                {
                    print_error_exit(exec_name, "Error: not all processes forked or initialized.\nHalting.", 0);
                    exit(1);
                }
            }
        } while (i < 2 * PROC_COUNT);

        for (i = PROC_COUNT + 1; i < 2 * PROC_COUNT; ++i)
        {
            pids_list[i] = 0;
        }

        sig_handler(0);
    }
    else
    {
        do
        {
        } while (pids_list[0] == 0);

        set_sig_handler(&sig_handler, 0, 0);
    }

    while (1)
    {
        pause();
    }

    return 0;
}

void forker(int curr_number, int childs_count)
{
    pid_t pid = 0;
    int i = 0;

    for (i = 0; i < childs_count; i++)
    {

        int chld_id = CHILDS_IDS[curr_number][i];
        if ((pid = fork()) == -1)
        {
            print_error_exit(exec_name, "Can't fork!", chld_id);
        }
        else if (pid == 0)
        {
            proc_id = chld_id;
            if (CHILDS_COUNT[proc_id] != 0)
            {
                forker(proc_id, CHILDS_COUNT[proc_id]);
            }
            break;
        }
        else
        {
            static int prev_chld_grp = 0;

            int grp_type = GROUP_TYPE[chld_id];

            if (grp_type == 0)
            {
                if (setpgid(pid, pid) == -1)
                {
                    print_error_exit(exec_name, "Can't set group", chld_id);
                }
                else
                {
                    prev_chld_grp = pid;
                }
            }
            else if (grp_type == 1)
            {
                if (setpgid(pid, getpgid(0)) == -1)
                {
                    print_error_exit(exec_name, "Can't set group", chld_id);
                }
            }
            else if (grp_type == 2)
            {
                if (setpgid(pid, prev_chld_grp) == -1)
                {
                    print_error_exit(exec_name, "Can't set group", chld_id);
                }
            }
        }
    }
}

void print_error_exit(const char *s_name, const char *msg, const int proc_num)
{
    fprintf(stderr, "%s: %s %d\n", s_name, msg, proc_num);
    fflush(stderr);

    pids_list[proc_num] = -1;

    exit(1);
}

void set_sig_handler(void (*handler)(void *), int sig_no, int flags)
{

    struct sigaction sa, oldsa;

    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, SIGUSR2);

    sa.sa_mask = block_mask;
    sa.sa_flags = flags;
    sa.sa_handler = handler;

    if (sig_no != 0)
    {
        sigaction(sig_no, &sa, &oldsa);
        return;
    }
    int i = 0;
    for (i = 0; i < PROC_COUNT; i++)
    {
        char to = SEND_TO[i];

        if (((to > 0) && (to == proc_id)) ||
            ((to < 0) && (getpgid(pids_list[-to]) == getpgid(0))))
        {
            if (SEND_SIGNALS[i] != 0)
            {
                if (sigaction(SEND_SIGNALS[i], &sa, &oldsa) == -1)
                {
                    print_error_exit(exec_name, "Can't set sighandler!", proc_id);
                }
            }
        }
    }

    pids_list[proc_id + PROC_COUNT] = 1;
}

void kill_wait_for_children()
{
    int i = 0;

    for (i = 0; i < CHILDS_COUNT[proc_id]; ++i)
    {
        kill(pids_list[CHILDS_IDS[proc_id][i]], SIGTERM);
    }

    wait_for_children();

    if (proc_id != 0)
        printf("%d %d was terminated after %d SIGUSR1 and %d SIGUSR2\n", getpid(), getppid(), usr_amount[0][0], usr_amount[1][0]);
    fflush(stdout);

    exit(0);
}

long long current_time()
{
    struct timeval time;
    gettimeofday(&time, NULL);

    return time.tv_usec;
}

void wait_for_children()
{
    int i = CHILDS_COUNT[proc_id];
    while (i > 0)
    {
        wait(NULL);
        i--;
    }
}

void sig_handler(int signum)
{

    if (signum == SIGUSR1)
    {
        signum = 0;
    }
    else if (signum == SIGUSR2)
    {
        signum = 1;
    }
    else
    {
        signum = -1;
    }

    if (signum != -1)
    {
        ++usr_amount[signum][0];
        ++usr_recv[signum];
        printf("%d %d %d received %s%d %lld\n", proc_id, getpid(), getppid(), "SIGUSR", signum + 1, current_time());
        fflush(stdout);

        if (proc_id == 1)
        {
            if (usr_amount[0][0] + usr_amount[1][0] == MAX_USR_COUNT)
            {
                kill_wait_for_children();
            }

            pids_list[PROC_COUNT + 6] = pids_list[PROC_COUNT + 4] = 0;
        }

        if (!((usr_recv[0] == RECV_SIGNALS_COUNT[0][proc_id]) &&
              (usr_recv[1] == RECV_SIGNALS_COUNT[1][proc_id])))
        {
            if ((proc_id == 4) && (signum == 0))
            {
                pids_list[PROC_COUNT + 4] = 1;
            }

            return;
        }
        usr_recv[0] = usr_recv[1] = 0;
    }

    char to = SEND_TO[proc_id];

    if (to != 0)
    {
        signum = ((SEND_SIGNALS[proc_id] == SIGUSR1) ? 1 : 2);
        ++usr_amount[signum - 1][1];
    }
    printf("%d %d %d sent %s%d %lld\n", proc_id, getpid(), getppid(), "SIGUSR", signum, current_time());
    fflush(stdout);

    if (to > 0)
    {
        kill(pids_list[to], SEND_SIGNALS[proc_id]);
    }
    else if (to < 0)
    {
        kill(-getpgid(pids_list[-to]), SEND_SIGNALS[proc_id]);
    }
    else
    {
        return;
    }
}