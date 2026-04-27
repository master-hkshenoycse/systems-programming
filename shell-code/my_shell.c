#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
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
	char  **tokens;              
	int i;


	while(1) {			
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
		
		pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
        }
        else if (pid == 0)
        {
            /* Child process */
            execvp(tokens[0], tokens);

            /* If execvp returns, command failed */
            perror("exec failed");
            exit(1);
        }
        else
        {
            /* Parent process waits */
            waitpid(pid, NULL, 0);
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
