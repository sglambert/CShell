#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// Function declarations for builtin shell commands:
int cd(char **args);
int help(char **args);
int exit_shell(char **args);

// List of builtin commands and their respective functions.
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &cd,
    &help,
    &exit_shell
};

int num_of_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


// Builtin function implementations.

/**
    @brief Bultin command: change directory.
    @param args List of args:  args[0] is "cd", args[1] is the directory.
    @return returns 1 to continue executing.
*/
int cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "Error: Please specify a directory");
    } 
    else {
      if (chdir(args[1]) != 0) {
        perror("Error: Failed to change directory");
      }
    }
    return 1;
}


/**
    @brief Builtin command: help.
    @param args List of args.  Not examined.
    @return Always returns 1, to continue executing.
*/
int help(char **args)
{
    int i;
    printf("The following are builtin commands:\n");

    for (i = 0; i < num_of_builtins(); i++) {
      printf("  %s\n", builtin_str[i]);
    }
    return 1;
}


/**
    @brief Builtin command: exit_shell.
    @param args List of args.
    @return Always returns 0, to terminate execution.
*/
int exit_shell(char **args)
{
    return 0;
}


/**
    @brief Launch a program and wait for it to terminate.
    @param args Null terminated list of arguments (including program).
    @return Always returns 1, to continue execution.
*/
int launch(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
      // Child process
      if (execvp(args[0], args) == -1) {
        perror("Error: Child process failed");
      }
      exit(EXIT_FAILURE);
    } else if (pid < 0) {
      // Error forking
      perror("Error: Forking process failed");
    } else {
      // Parent process
      do {
        waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

/**
    @brief Execute program.
    @param args Null terminated list of arguments.
    @return 0 if the shell should terminate, 1 if it should continue
*/
int execute(char **args)
{
    int i;

    if (args[0] == NULL) {
      // An empty command was entered.
      return 1;
    }

    for (i = 0; i < num_of_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
          return (*builtin_func[i])(args);
        }
      }
    return launch(args);
}

char *read_line(void)
{
    char *line = NULL;
    ssize_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1){
      if (feof(stdin)) {
        exit(EXIT_SUCCESS);
      } else  {
        perror("Error: readline error");
        exit(EXIT_FAILURE);
      }
    }
    return line;
}

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"
/**
    @brief Split a line into tokens.
    @param line The line.
    @return Null-terminated array of tokens.
*/
char **split_line(char *line)
{
    int bufsize = TOKEN_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "Error: Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
      tokens[position] = token;
      position++;

      if (position >= bufsize) {
        bufsize += TOKEN_BUFSIZE;
        tokens_backup = tokens;
        tokens = realloc(tokens, bufsize * sizeof(char*));
        if (!tokens) {
      free(tokens_backup);
          fprintf(stderr, "Error: Allocation error\n");
          exit(EXIT_FAILURE);
        }
      }
      token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
    @brief Loop getting input and executing it.
*/
void loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = read_line();
        args = split_line(line);
        status = execute(args);

        free(line);
        free(args);
    } while (status);
}

/**
    @brief Main entry point
    @param argc Argument count
    @param argv Argument vector
    @return status code
*/
int main(int argc, char **argv)
{
    // Run command
    loop();

    return EXIT_SUCCESS;
}
