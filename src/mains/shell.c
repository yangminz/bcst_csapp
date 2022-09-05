#define _XOPEN_SOURCE 600
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

// input line on stdin from user
// the input line should be parsed
// and shell invokes corresponding actions
#define MAX_LENGTH_USERINPUT (128)
#define MAX_ARG_COUNT (32)
#define REDSTR(STR)     "\033[31;1m"STR"\033[0m"
#define GREENSTR(STR)   "\033[32;1m"STR"\033[0m"
#define YELLOWSTR(STR)  "\033[33;1m"STR"\033[0m"
#define BLUESTR(STR)    "\033[34;1m"STR"\033[0m"

typedef enum
{
    EMPTY,
    // built-in command
    // "quit" to terminate shell
    // "jobs" to list all background jobs
    // "bg <pid>" to restart job in background by sending SIGCONT
    // "fg <pid>" to restart job in foreground by sending SIGCONT
    BUILTIN_QUIT,
    BUILTIN_JOBS,
    BUILTIN_FGCONT,
    BUILTIN_BGCONT,
    // foreground job taking the control flow
    // shell should wait for forground job to finish
    FOREGROUND_JOB,
    // background job leaving the control flow
    // background job are ending with '&', e.g.,
    // > /bin/sleep 5 &
    // shell should NOT wait for background job to finish
    BACKGROUND_JOB
} job_t;

typedef struct USERINPUT_STRUCT
{
    // the job type of current user input
    job_t type;
    // the count of arguments
    int argc;
    // the value of arguments
    char **argv;
    // the raw char pointer
    char *raw;
} userinput_t;

typedef enum
{
    NEW,
    FG_RUNNING,
    BG_RUNNING,
    STOPPED,
} job_state_t;

typedef struct JOB_TASK_STRUCT
{
    pid_t pid;
    job_state_t state;
    char argstr[MAX_LENGTH_USERINPUT];
} job_task_t;

#define MAX_NUM_JOBS (64)
job_task_t job_list[MAX_NUM_JOBS];

static int job_add(pid_t pid, job_state_t state, char *argstr);
static int job_delete(pid_t pid);
static void job_print();
static void job_init();

static void parse_userinput(
    const char *input,
    userinput_t *result);
static void evaluate(userinput_t *input);

// handler for SIGCHLD when a child process terminates
static void sigchld_handler(int sig);

// handler for SIGINT sent by Ctrl-C
// when Ctrl-C, the signal should be sent to foreground job group
// i.e., child process may create child process
// and shell process & other background processes 
// should not be interrupted
static void sigint_handler(int sig);

// handler for SIGTSTP sent by Ctrl-Z
// when Ctrl-Z, the signal should be sent to foreground job group
// i.e., child process may create child process
// and shell process & other background processes 
// should not be stopped
static void sigtstp_handler(int sig);

typedef void (sighandler_t)(int);
sighandler_t *register_sighandler(
    int sig, sighandler_t *handler, int flags)
{
    struct sigaction action, old_action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = flags;

    if (sigaction(sig, &action, &old_action) < 0) {
        write(1, "Signal error\n", 13);
        exit(1);
    }

    return old_action.sa_handler;
}

// flag indicating if sigchld has been invoked
// to wait for child process zombies
static int fg_reaped = 0;
static int fg_pid;

int main()
{
    userinput_t input;
    char input_chars[MAX_LENGTH_USERINPUT];

    // register handlers for signals
    // with this, this program can only run on *unix
    register_sighandler(SIGCHLD, sigchld_handler, SA_RESTART);
    register_sighandler(SIGINT,  sigint_handler,  SA_RESTART);
    register_sighandler(SIGTSTP, sigtstp_handler, SA_RESTART);

    job_init();
    pid_t shell_pid = getpid();

    while (1)
    {
        printf(GREENSTR("[%d]>>>>> "), shell_pid);

        // read user input
        fgets(input_chars, MAX_LENGTH_USERINPUT, stdin);
        parse_userinput(input_chars, &input);

        if (input.type == EMPTY)
        {
            continue;
        }
        else
        {
            evaluate(&input);
        }

        // release the arguments
        for (int i = 0; i < input.argc; ++ i)
        {
            free(input.argv[i]);
        }
        free(input.argv);
    }
    return 0;
}

static void run_child(userinput_t *input)
{
    char *envp[] = { NULL };

    // block before fork to eliminate the race condition between
    // parent (shell) and the handler threads on the 
    // global resource -- job list add/delete
    // handler should delete the job from the list after it's added
    sigset_t mask, prev;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &mask, &prev);

    pid_t pid = fork();
    if (pid == 0)
    {
        // new created child process will inherit the signal
        // pending vector, blocking vector from parent.
        // unblock and reset in case further execve will fork
        sigprocmask(SIG_SETMASK, &prev, NULL);

        // do not trigger copy on write here
        if (execve(input->argv[0], input->argv, envp) < 0)
        {
            printf(REDSTR("failed to execute %s\n"), input->argv[0]);
            exit(-1);
        }
    }
    else
    {
        // critical section
        // when SIGCHLD is blocked
        // the code in this section is executed before handler
        job_add(pid, FG_RUNNING, input->raw);

        if (input->type == FOREGROUND_JOB)
        {
            // fg_pid is also a critical global variable
            fg_reaped = 0;
            fg_pid = pid;

            // unblock -- recover the previous signal block vector
            sigprocmask(SIG_SETMASK, &prev, NULL);

            // wait for hanlder being invoked
            while (fg_reaped == 0)
            {
                sleep(1);
            }
        }
        else
        {
            // background job
        }
    }
}

static void evaluate(userinput_t *input)
{

    switch (input->type)
    {
    case BUILTIN_QUIT:
        printf(YELLOWSTR("Shell terminating ...\n"));
        exit(0);
    case BUILTIN_JOBS:
        break;
    case BUILTIN_FGCONT:
        break;
    case BUILTIN_BGCONT:
        break;
    case FOREGROUND_JOB:
        run_child(input);
        break;
    case BACKGROUND_JOB:
        break;
    default:
        break;
    }
}

static void sigchld_handler(int sig)
{
    // async-signal-safe system functions may set errno
    // when return with error. E.g., printf.
    // So we need to save errno on stack and restore it.
    int _errno = errno;
    int status;

    // waitpid(-1) -- match any process
    pid_t pid = waitpid(-1, &status, WUNTRACED | WNOHANG);
    while (pid > 0)
    {
        // when this handler is invoked,
        // SIGCHLD is unblocked.
        // so it's safe to operate on global resource
        if (pid == fg_pid)
        {
            fg_reaped = 1;
        }

        if (WIFEXITED(status))
        {
            job_delete(pid);
        }

        pid = waitpid(-1, &status, WUNTRACED | WNOHANG);
    }

    // restore errno
    errno = _errno;
}

static void sigint_handler(int sig)
{
    int _errno = errno;
    // proxy SIGINT to foreground process group:
    // FG job may create child processes.
    // These processes are all grouped by FG job pid
    if (fg_pid > 0)
    {
        killpg(fg_pid, SIGINT);
        fg_pid = 0;
    }
    errno = _errno;
}

static void sigtstp_handler(int sig)
{
    int _errno = errno;
    // proxy SIGINT to foreground process group:
    // FG job may create child processes.
    // These processes are all grouped by FG job pid
    if (fg_pid > 0)
    {
        killpg(fg_pid, SIGTSTP);
        fg_pid = 0;
    }
    errno = _errno;
}

static void job_init()
{
    for (int i = 0; i < MAX_NUM_JOBS; ++ i)
    {
        job_list[i].state = NEW;
        job_list[i].pid = -1;
        memset(job_list[i].argstr, '\0', sizeof(job_list[i].argstr));
    }
}

// Insert a new pid into background job list
static int job_add(pid_t pid, job_state_t state, char *argstr)
{
    if (pid == -1)
    {
        return -1;
    }
    
    for (int i = 0; i < MAX_NUM_JOBS; ++i)
    {
        if (job_list[i].state == NEW)
        {
            // insert to this job
            job_list[i].pid = pid;
            job_list[i].state = state;
            strcpy(job_list[i].argstr, argstr);
            return 1;
        }
    }

    printf(REDSTR("job_add::no free slot for new job\n"));
    return 0;
}

// delete an existing pid from background job list
static int job_delete(pid_t pid)
{
    if (pid == -1)
    {
        return -1;
    }

    for (int i = 0; i < MAX_NUM_JOBS; ++ i)
    {
        if (job_list[i].pid == pid)
        {
            job_list[i].pid = 0;
            job_list[i].state = NEW;
            memset(job_list[i].argstr, '\0', sizeof(job_list[i].argstr));
            return 1;
        }
    }

    printf(REDSTR("job_delete::pid {%d} not found from job list\n"), pid);
    return 0;
}

static void job_print()
{
    for (int i = 0; i < MAX_NUM_JOBS; ++ i)
    {
        if (job_list[i].state == NEW)
        {
            continue;
        }
        printf("[%d]\t%d\t%s\n",
            job_list[i].pid,
            job_list[i].state,
            job_list[i].argstr);
    }
}


// parse the user input
static void parse_userinput(
    const char *input, 
    userinput_t *result)
{
    if (result == NULL)
    {
        printf(REDSTR("Failed to fetch space for user input parsing result\n"));
        exit(-1);
    }
    
    // empty as default
    result->type = EMPTY;
    result->argc = 0;
    result->argv = NULL;
    result->raw = NULL;
    if (input == NULL || *input == '\0')
    {
        return;
    }
    result->raw = (char *)input;

    // the previous char
    int p = -1;

    // first we count the size of argv
    for (int i = 0; i < MAX_LENGTH_USERINPUT; ++ i)
    {
        if (input[i] == '\0' || input[i] == '\n')
            break;

        if (input[i] != ' ' && (p == -1 || input[p] == ' '))
        {
            result->argc += 1;
        }
        p = i;
    }

    result->argv = malloc(sizeof(char *) * (result->argc + 1));
    result->argv[result->argc] = NULL;

    // the current parsed argument char
    int j = 0, w = 0;
    p = -1;
    for (int i = 0; i < MAX_LENGTH_USERINPUT; ++ i)
    {
        if ((input[i] == ' ' || input[i] == '\0' || input[i] == '\n') && 
            (p != -1 || input[p] != ' '))
        {
            result->argv[j] = malloc(sizeof(char) * (i - w + 1));
            strncpy(result->argv[j], &input[w], i - w);
            result->argv[j][i - w] = '\0';
            
            // reset to next word
            j += 1;
        }
        else if (input[i] != ' ' && (p == -1 || input[p] == ' '))
        {
            w = i;
        }

        // go to next input char
        p = i;
        
        if (input[i] == '\0' || input[i] == '\n')
        {
            break;
        }
    }

    if (result->argc >= 2 &&
        strcmp(result->argv[result->argc - 1], "&") == 0)
    {
        result->type = BACKGROUND_JOB;
    }
    else if (result->argc >= 1)
    {
        result->type = FOREGROUND_JOB;
    }

    if (result->argc == 1)
    {
        if (strcmp(result->argv[0], "quit") == 0)
        {
            result->type = BUILTIN_QUIT;
        }
        else if (strcmp(result->argv[0], "jobs") == 0)
        {
            result->type = BUILTIN_JOBS;
        }
    }
    else if (result->argc == 2)
    {
        if (strcmp(result->argv[0], "fg") == 0)
        {
            result->type = BUILTIN_FGCONT;
        }
        else if (strcmp(result->argv[0], "bg") == 0)
        {
            result->type = BUILTIN_BGCONT;
        }
    }
}