#define _XOPEN_SOURCE 600
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

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
} userinput_t;

typedef struct JOB_TASK_STRUCT
{
    pid_t pid;
    struct JOB_TASK_STRUCT *prev;
    struct JOB_TASK_STRUCT *next;
} job_task_t;

typedef struct JOB_LIST_STRUCT
{
    int count;
    job_task_t *head;
} job_list_t;

static job_list_t job_list;
static void joblist_add(job_list_t *list, pid_t pid);
static int joblist_del(job_list_t *list, pid_t pid);
static void joblist_print(job_list_t *list);

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

int main()
{
    userinput_t input;
    char input_chars[MAX_LENGTH_USERINPUT];

    // register handlers for signals
    // with this, this program can only run on *unix
    register_sighandler(SIGCHLD, sigchld_handler, SA_RESTART);
    register_sighandler(SIGINT,  sigint_handler,  SA_RESTART);
    register_sighandler(SIGTSTP, sigtstp_handler, SA_RESTART);

    while (1)
    {
        printf(GREENSTR(">>>>> "));

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
    pid_t pid = fork();
    if (pid == 0)
    {
        // set the foreground job group
        pid = getpid();
        setpgid(pid, pid);
        joblist_add(&job_list, pid);

        // new created child process
        // do not trigger copy on write here
        if (execve(input->argv[0], input->argv, envp) < 0)
        {
            printf(REDSTR("failed to execute %s\n"), input->argv[0]);
            exit(-1);
        }
    }
    else
    {
        // parent should wait for the child process to terminate
        waitpid(pid, NULL, WUNTRACED);
        printf("%d terminated\n", pid);
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
        joblist_print(&job_list);
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
    if (input == NULL || *input == '\0')
    {
        return;
    }

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

static void sigchld_handler(int sig)
{
    // when this handler is called,
    // at least one child has terminated and
    // SIGCHLD signal is not blocked
    printf("catched sig child\n");
}

static void sigint_handler(int sig)
{
    write(1, "hello\n", 6);
}

static void sigtstp_handler(int sig)
{}

// Insert a new pid into background job list
static void joblist_add(job_list_t *list, pid_t pid)
{
    if (pid == -1)
    {
        return;
    }

    job_task_t *task = malloc(sizeof(job_task_t));
    task->pid = pid;
    task->prev = task;
    task->next = task;

    if (list->count == 0)
    {
        list->head = task;
        list->count = 1;
    }
    else
    {
        task->next = list->head;
        list->head->prev->next = task;
        task->prev = list->head->prev;
        list->head->prev = task;
        list->count += 1;
    }
}

// delete an existing pid from background job list
static int joblist_del(job_list_t *list, pid_t pid)
{
    if (pid == -1)
    {
        return 0;
    }

    if (list->head->pid == pid)
    {
        list->head = list->head->next;
    }

    if (list->count == 1)
    {
        free(list->head);
        list->count = 0;
        list->head = NULL;
        return 1;
    }

    job_task_t *task = list->head;
    for (int i = 0; i < list->count; ++ i)
    {
        if (task->pid == pid)
        {
            // the job to deleted
            task->prev->next = task->next;
            task->next->prev = task->prev;
            free(task);
            list->count -= 1;
            return 1;
        }
    }

    return 0;
}

static void joblist_print(job_list_t *list)
{
    job_task_t *task = list->head;
    for (int i = 0; i < list->count; ++ i)
    {
        printf("%d\n", task->pid);
    }
}