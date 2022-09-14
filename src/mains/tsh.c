/*
 * TODO: Include your name and Andrew ID here.
 */

/*
 * TODO: Delete this comment and replace it with your own.
 * tsh - A tiny shell program with job control
 * <The line above is not a sufficient documentation.
 *  You will need to write your program documentation.
 *  Follow the 15-213/18-213/15-513 style guide at
 *  http://www.cs.cmu.edu/~213/codeStyle.html.>
 */

#include "csapp.h"
#include "tsh_helper.h"

#include <sys/wait.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * If DEBUG is defined, enable contracts and printing on dbg_printf.
 */
#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif

/* Function prototypes */
void eval(const char *cmdline);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
void sigquit_handler(int sig);
void cleanup(void);

/*  Global flags to be shared between shell main thread and
    signal handler thread.
    The flags should write in one thread and read in the other.
 */
static sig_atomic_t fg_pid;
static sig_atomic_t fg_running;
static sig_atomic_t st_fgbg;

/*
 * TODO: Delete this comment and replace it with your own.
 * <Write main's function header documentation. What does main do?>
 * "Each function should be prefaced with a comment describing the purpose
 *  of the function (in a sentence or two), the function's arguments and
 *  return value, any error cases that are relevant to the caller,
 *  any pertinent side effects, and any assumptions that the function makes."
 */
int main(int argc, char **argv) {
    char c;
    char cmdline[MAXLINE_TSH];  // Cmdline for fgets
    bool emit_prompt = true;    // Emit prompt (default)

    // Redirect stderr to stdout (so that driver will get all output
    // on the pipe connected to stdout)
    if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
        perror("dup2 error");
        exit(1);
    }

    // Parse the command line
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':                   // Prints help message
            usage();
            break;
        case 'v':                   // Emits additional diagnostic info
            verbose = true;
            break;
        case 'p':                   // Disables prompt printing
            emit_prompt = false;
            break;
        default:
            usage();
        }
    }

    // Create environment variable
    if (putenv("MY_ENV=42") < 0) {
        perror("putenv");
        exit(1);
    }

    // Set buffering mode of stdout to line buffering.
    // This prevents lines from being printed in the wrong order.
    if (setvbuf(stdout, NULL, _IOLBF, 0) < 0) {
        perror("setvbuf");
        exit(1);
    }

    // Initialize the job list
    init_job_list();

    // Register a function to clean up the job list on program termination.
    // The function may not run in the case of abnormal termination (e.g. when
    // using exit or terminating due to a signal handler), so in those cases,
    // we trust that the OS will clean up any remaining resources.
    if (atexit(cleanup) < 0) {
        perror("atexit");
        exit(1);
    }

    // Install the signal handlers
    Signal(SIGINT,  sigint_handler);   // Handles Ctrl-C
    Signal(SIGTSTP, sigtstp_handler);  // Handles Ctrl-Z
    Signal(SIGCHLD, sigchld_handler);  // Handles terminated or stopped child

    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN);

    Signal(SIGQUIT, sigquit_handler);

    fg_pid = 0;
    fg_running = 0;
    st_fgbg = 0;

    // Execute the shell's read/eval loop
    while (true) {
        if (emit_prompt) {
            printf("%s", prompt);

            // We must flush stdout since we are not printing a full line.
            fflush(stdout);
        }

        if ((fgets(cmdline, MAXLINE_TSH, stdin) == NULL) && ferror(stdin)) {
            perror("fgets error");
            exit(1);
        }

        if (feof(stdin)) {
            // End of file (Ctrl-D)
            printf("\n");
            return 0;
        }

        // Remove any trailing newline
        char *newline = strchr(cmdline, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }

        // Evaluate the command line
        eval(cmdline);
    }

    return -1; // control never reaches here
}

/*  Create child process for Foreground/Background job
 */
void child_process(parseline_return parse_result,
    struct cmdline_tokens *token,
    const char *cmdline)
{
    // prepare signal mask
    sigset_t mask, prev;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);

    // block SIGCHLD, SIGINT, SIGTSTP
    // to avoid RACE CONDITION between
    // shell main thread and handler thread on
    // global variables: flags and job_list
    sigprocmask(SIG_BLOCK, &mask, &prev);

    pid_t pid = fork();
    if (pid == 0)
    {
        // child process

        // unblock signals in case child process will use
        sigprocmask(SIG_SETMASK, &prev, NULL);

        // use pid of child process as group id
        // so the forwarded signal will go to the whole group.
        pid = getpid();
        setpgid(pid, pid);

        // execute the new program
        if (execve(token->argv[0], token->argv, environ) < 0)
        {
            printf("failed to execute: %s\n", cmdline);
            exit(-1);
        }
    }
    else
    {
        // shell process

        if (parse_result == PARSELINE_FG)
        {
            // foreground job

            // set the global variables: flags and job list
            fg_pid = pid;
            fg_running = 1;
            add_job(pid, FG, cmdline); 

            // wait for FG job no longer running
            // without RACE CONDITION and CPU waste
            while (fg_running == 1)
            {
                sigsuspend(&prev);
            }

            // optional: you can unblock or not
            sigprocmask(SIG_SETMASK, &prev, NULL);
        }
        else if (parse_result == PARSELINE_BG)
        {
            // background job

            // set the global variables: flags and job list
            add_job(pid, BG, cmdline); 
            jid_t jid = job_from_pid(pid);
            printf("[%d] (%d) %s\n", jid, pid, cmdline);

            // unblock is necessary in this case
            sigprocmask(SIG_SETMASK, &prev, NULL);
        }
    }
}

/*  Job state transfer:
    ST -> FG, ST -> BG, BG -> FG
 */
void fgbg_job(struct cmdline_tokens *token)
{
    // fg/bg, the first argument variable
    const char *btin = token->argv[0];

    // check the argument number
    if (token->argc != 2)
    {
        printf("%s command requires PID or %%jobid argument\n", btin);
        return;
    }

    // the jid/pid string
    const char *id = token->argv[1];
    pid_t pid = 0;  // process id
    jid_t jid = 0;  // job id

    sigset_t mask, prev;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);

    // block to avoid RACE CONDITION
    sigprocmask(SIG_BLOCK, &mask, &prev);

    // convert the input id string to pid and jid
    if (id == NULL || id[0] == '\0')
    {
        printf("%s command requires PID or %%jobid argument", btin);
    }
    else
    {
        if (id[0] == '%')
        {
            // jid
            jid = (jid_t)atoi(&id[1]);
            if (job_exists(jid) == false)
            {
                printf("%s: No such job\n", id);
                goto FGBG_UNBLOCK;
            }
            pid = job_get_pid(jid);
        }
        else
        {
            // pid
            pid = (pid_t)atoi(id);
            if (pid <= 0)
            {
                printf("%s: argument must be a PID or %%jobid\n", btin);
                goto FGBG_UNBLOCK;
            }
            jid = job_from_pid(pid);
        }
    }

    // get job info from jid
    const char *cmdline = job_get_cmdline(jid);
    job_state jstate = job_get_state(jid);

    if (token->builtin == BUILTIN_BG)
    {
        // ST -> BG
        st_fgbg = 2;    
        printf("[%d] (%d) %s\n", jid, pid, cmdline);
        killpg(pid, SIGCONT);
    }
    else
    {
        fg_pid = pid;
        fg_running = 1;

        if (jstate == ST)
        {
            // ST -> FG
            st_fgbg = 1;
            killpg(pid, SIGCONT);
        }
        else if (jstate == BG)
        {
            // BG -> FG
            st_fgbg = 0;
            job_set_state(jid, FG);
        }

        // wait for FG job finish running
        while (fg_running == 1)
        {
            sigsuspend(&prev);
        }
    }

FGBG_UNBLOCK:
    // unblock
    sigprocmask(SIG_SETMASK, &prev, NULL);
}

/*
 * TODO: Delete this comment and replace it with your own.
 * <What does eval do?>
 *
 * NOTE: The shell is supposed to be a long-running process, so this function
 *       (and its helpers) should avoid exiting on error.  This is not to say
 *       they shouldn't detect and print (or otherwise handle) errors!
 */
void eval(const char *cmdline) {
    parseline_return parse_result;
    struct cmdline_tokens token;

    // Parse command line
    parse_result = parseline(cmdline, &token);

    if (parse_result == PARSELINE_ERROR || parse_result == PARSELINE_EMPTY) {
        return;
    }

    sigset_t mask, prev;
    sigemptyset(&mask);
    sigaddset(&mask, SIGCHLD);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);

    switch (token.builtin)
    {
    case BUILTIN_NONE:
        // must be a FG or BG job
        child_process(parse_result, &token, cmdline);
        break;
    case BUILTIN_QUIT:
        exit(0);
    case BUILTIN_JOBS:
        // block
        sigprocmask(SIG_BLOCK, &mask, &prev);
        // access global job list
        list_jobs(STDOUT_FILENO);
        // unblock
        sigprocmask(SIG_SETMASK, &prev, NULL);
        break;
    case BUILTIN_BG:
    case BUILTIN_FG:
        fgbg_job(&token);
        break;
    default:
        break;
    }

}

/*****************
 * Signal handlers
 *****************/

/*
 * TODO: Delete this comment and replace it with your own.
 * <What does sigchld_handler do?>
 */
void sigchld_handler(int sig) {
    int _errno = errno;

    sigset_t mask, prev;    
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTSTP);

    // block
    sigprocmask(SIG_BLOCK, &mask, &prev);

    int wstate;
    pid_t pid = waitpid(-1, &wstate, WUNTRACED | WNOHANG | WCONTINUED);
    while (pid > 0)
    {
        jid_t jid = job_from_pid(pid);

        if (WIFCONTINUED(wstate))
        {
            job_state jstate = job_get_state(jid);

            if (jstate == ST)
            {
                if (st_fgbg == 1)
                {
                    job_set_state(jid, FG);
                }
                else if (st_fgbg == 2)
                {
                    job_set_state(jid, BG);
                }
            }
        }
        else
        {
            if (pid == fg_pid)
            {
                fg_running = 0;
            }

            if (WIFEXITED(wstate))
            {
                // exit() or return from main()
                delete_job(jid);
            }

            if (WIFSIGNALED(wstate))
            {
                // SIGINT
                delete_job(jid);
                int signum = WTERMSIG(wstate);
                sio_printf("Job [%d] (%d) terminated by signal %d\n",
                    jid, pid, signum);
            }

            if (WIFSTOPPED(wstate))
            {
                // SIGTSTP
                job_set_state(jid, ST);
                int signum = WSTOPSIG(wstate);
                sio_printf("Job [%d] (%d) stopped by signal %d\n",
                    jid, pid, signum);
            }
        }
        

        pid = waitpid(-1, &wstate, WUNTRACED | WNOHANG | WCONTINUED);
    }

    sigprocmask(SIG_SETMASK, &prev, NULL);

    errno = _errno;
}

/*
 * TODO: Delete this comment and replace it with your own.
 * <What does sigint_handler do?>
 */
void sigint_handler(int sig) {
    int _errno = errno;

    if (fg_pid > 0)
    {
        killpg(fg_pid, sig);
    }

    errno = _errno;
}

/*
 * TODO: Delete this comment and replace it with your own.
 * <What does sigtstp_handler do?>
 */
void sigtstp_handler(int sig) {
    int _errno = errno;

    if (fg_pid > 0)
    {
        killpg(fg_pid, sig);
    }

    errno = _errno;
}

/*
 * cleanup - Attempt to clean up global resources when the program exits. In
 * particular, the job list must be freed at this time, since it may contain
 * leftover buffers from existing or even deleted jobs.
 */
void cleanup(void) {
    // Signals handlers need to be removed before destroying the joblist
    Signal(SIGINT,  SIG_DFL);  // Handles Ctrl-C
    Signal(SIGTSTP, SIG_DFL);  // Handles Ctrl-Z
    Signal(SIGCHLD, SIG_DFL);  // Handles terminated or stopped child

    destroy_job_list();
}

