#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG_PROCS 64
#define MAX_CMDS 64

volatile sig_atomic_t fg_pids[MAX_CMDS];
volatile sig_atomic_t fg_count = 0;

/* ---------------- SIGINT HANDLER ---------------- */
void sigint_handler(int signo)
{
    for (int i = 0; i < fg_count; i++)
    {
        if (fg_pids[i] > 0)
            kill(fg_pids[i], SIGINT);
    }
    write(STDOUT_FILENO, "\n", 1);
}

/* ---------------- TOKENIZER ---------------- */
char **tokenize(char *line)
{
    char **tokens = malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = malloc(MAX_TOKEN_SIZE);

    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++)
    {
        char c = line[i];

        if (c == ' ' || c == '\n' || c == '\t')
        {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0)
            {
                tokens[tokenNo] = malloc(MAX_TOKEN_SIZE);
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0;
            }
        }
        else
        {
            token[tokenIndex++] = c;
        }
    }

    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}

void free_tokens(char **tokens)
{
    for (int i = 0; tokens[i] != NULL; i++)
        free(tokens[i]);
    free(tokens);
}

/* ---------------- SPLIT COMMANDS ---------------- */
char ***split_commands(char *line, int *count, int mode)
{
    char ***cmds = malloc(MAX_CMDS * sizeof(char **));
    char *token;
    int i = 0;

    char *delim = (mode == 1) ? "&&" : "&&&";

    token = strtok(line, delim);

    while (token != NULL)
    {
        cmds[i++] = tokenize(token);
        token = strtok(NULL, delim);
    }

    cmds[i] = NULL;
    *count = i;
    return cmds;
}

/* ---------------- MAIN ---------------- */
int main()
{
    char line[MAX_INPUT_SIZE];
    char cwd[256];

    pid_t bg_pids[MAX_BG_PROCS];
    int bg_count = 0;

    signal(SIGINT, sigint_handler);

    while (1)
    {
        /* reap background */
        int status;
        pid_t done;
        while ((done = waitpid(-1, &status, WNOHANG)) > 0)
        {
            printf("Shell: Background process finished\n");

            for (int i = 0; i < bg_count; i++)
            {
                if (bg_pids[i] == done)
                {
                    bg_pids[i] = bg_pids[bg_count - 1];
                    bg_count--;
                    break;
                }
            }
        }

        /* prompt */
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s $ ", cwd);
        else
            printf("$ ");

        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        /* detect mode */
        int mode = 0;
        if (strstr(line, "&&&"))
            mode = 2;
        else if (strstr(line, "&&"))
            mode = 1;

        char ***commands;
        int cmd_count = 0;

        if (mode == 0)
        {
            commands = malloc(sizeof(char **));
            commands[0] = tokenize(line);
            commands[1] = NULL;
            cmd_count = 1;
        }
        else
        {
            commands = split_commands(line, &cmd_count, mode);
        }

        /* handle exit */
        if (commands[0][0] && strcmp(commands[0][0], "exit") == 0)
        {
            for (int i = 0; i < bg_count; i++)
                kill(bg_pids[i], SIGKILL);

            while (waitpid(-1, NULL, 0) > 0)
                ;

            for (int i = 0; i < cmd_count; i++)
                free_tokens(commands[i]);
            free(commands);
            break;
        }

        /* SERIAL */
        if (mode == 1)
        {
            for (int c = 0; c < cmd_count; c++)
            {
                if (commands[c][0] == NULL)
                    continue;

                if (strcmp(commands[c][0], "cd") == 0)
                {
                    if (commands[c][1] == NULL || commands[c][2] != NULL)
                        printf("cd: invalid usage\n");
                    else if (chdir(commands[c][1]) != 0)
                        perror("cd failed");
                    continue;
                }

                pid_t pid = fork();

                if (pid == 0)
                {
                    execvp(commands[c][0], commands[c]);
                    perror("exec failed");
                    exit(1);
                }
                else
                {
                    fg_pids[0] = pid;
                    fg_count = 1;

                    int status;
                    waitpid(pid, &status, 0);

                    fg_count = 0;

                    if (WIFEXITED(status))
                        printf("EXITSTATUS: %d\n", WEXITSTATUS(status));
                }
            }
        }

        /* PARALLEL */
        else if (mode == 2)
        {
            pid_t pids[MAX_CMDS];

            for (int c = 0; c < cmd_count; c++)
            {
                if (commands[c][0] == NULL)
                    continue;

                pid_t pid = fork();

                if (pid == 0)
                {
                    execvp(commands[c][0], commands[c]);
                    perror("exec failed");
                    exit(1);
                }
                else
                {
                    pids[c] = pid;
                }
            }

            fg_count = cmd_count;
            for (int i = 0; i < cmd_count; i++)
                fg_pids[i] = pids[i];

            for (int c = 0; c < cmd_count; c++)
            {
                int status;
                waitpid(pids[c], &status, 0);

                if (WIFEXITED(status))
                    printf("EXITSTATUS: %d\n", WEXITSTATUS(status));
            }

            fg_count = 0;
        }

        /* SINGLE COMMAND */
        else
        {
            char **tokens = commands[0];

            if (tokens[0] == NULL)
            {
                free_tokens(tokens);
                free(commands);
                continue;
            }

            if (strcmp(tokens[0], "cd") == 0)
            {
                if (tokens[1] == NULL || tokens[2] != NULL)
                    printf("cd: invalid usage\n");
                else if (chdir(tokens[1]) != 0)
                    perror("cd failed");

                free_tokens(tokens);
                free(commands);
                continue;
            }

            int bg = 0;
            int i;
            for (i = 0; tokens[i] != NULL; i++)
                ;

            if (i > 0 && strcmp(tokens[i - 1], "&") == 0)
            {
                bg = 1;
                free(tokens[i - 1]);
                tokens[i - 1] = NULL;
            }

            pid_t pid = fork();

            if (pid == 0)
            {
                if (bg)
                    setpgid(0, 0);

                execvp(tokens[0], tokens);
                perror("exec failed");
                exit(1);
            }
            else
            {
                if (bg)
                {
                    bg_pids[bg_count++] = pid;
                }
                else
                {
                    fg_pids[0] = pid;
                    fg_count = 1;

                    int status;
                    waitpid(pid, &status, 0);

                    fg_count = 0;

                    if (WIFEXITED(status))
                        printf("EXITSTATUS: %d\n", WEXITSTATUS(status));
                }
            }
        }

        /* free */
        for (int i = 0; i < cmd_count; i++)
            free_tokens(commands[i]);

        free(commands);
    }

    return 0;
}