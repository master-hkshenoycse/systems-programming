#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG_PROCS 64
volatile sig_atomic_t fg_pid = -1;
/* Splits the string by space and returns the array of tokens
*
*/

void sigint_handler(int signo)
{
    if (fg_pid > 0)
    {
        kill(fg_pid, SIGINT);
    }
}

char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];
	char cwd[256];
	pid_t bg_pids[MAX_BG_PROCS];
    int bg_count = 0;
	char  **tokens;              
	int i;

	signal(SIGINT, sigint_handler);

	while(1) {			
		int status;
        pid_t done;

        while ((done = waitpid(-1, &status, WNOHANG)) > 0)
        {
            printf("Shell: Background process finished\n");

            for (i = 0; i < bg_count; i++)
            {
                if (bg_pids[i] == done)
                {
                    bg_pids[i] = bg_pids[bg_count - 1];
                    bg_count--;
                    break;
                }
            }
        }

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));

		if (getcwd(cwd, sizeof(cwd)) != NULL)
			printf("%s $ ", cwd);
		else
			printf("$ ");

		scanf("%[^\n]", line);
		getchar();

		printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		if (tokens[0] == NULL)
        {
            free(tokens);
            continue;
        }

		/* ---------------- BUILT-IN: cd ---------------- */
		if (strcmp(tokens[0], "cd") == 0)
		{
			/* cd requires exactly one argument */
			if (tokens[1] == NULL || tokens[2] != NULL)
			{
				printf("cd: invalid usage\n");
			}
			else
			{
				if (chdir(tokens[1]) != 0)
				{
					perror("cd failed");
				}
			}

			for (i = 0; tokens[i] != NULL; i++)
				free(tokens[i]);

			free(tokens);
			continue;
		}

		if (strcmp(tokens[0], "exit") == 0)
        {
            /* kill all background processes */
            for (i = 0; i < bg_count; i++)
            {
                kill(bg_pids[i], SIGKILL);
            }

            /* reap all children */
            while (waitpid(-1, NULL, 0) > 0)
                ;

            free(tokens);
            break;
        }

		int bg = 0;

		/* find last token */
		for (i = 0; tokens[i] != NULL; i++);

		if (i > 0 && strcmp(tokens[i - 1], "&") == 0)
		{
			bg = 1;
			free(tokens[i - 1]);
			tokens[i - 1] = NULL;
		}

		pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
        }
        else if (pid == 0)
        {

			if (bg)
            {
                setpgid(0, 0);
            }

            /* Child process */
            execvp(tokens[0], tokens);

            /* If execvp returns, command failed */
            perror("exec failed");
            exit(1);
        }
        else
        {	
			if(bg==0){
				/* Parent process waits */
				int status;
				fg_pid = pid;

				waitpid(pid, &status, 0);

				fg_pid = -1;

				if (WIFEXITED(status))
					printf("EXITSTATUS: %d\n", WEXITSTATUS(status));

			}else{
				setpgid(0, 0);
				if (bg_count < MAX_BG_PROCS)
                    bg_pids[bg_count++] = pid;
			}
        }

		for(i=0;tokens[i]!=NULL;i++){
			printf("found token %s (remove this debug output later)\n", tokens[i]);
		}
       
		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}
