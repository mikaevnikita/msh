#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MSH_BUFSIZE 255
#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELIM " \t\r\n\a"

/*char* msh_read_line(void)
{
    int bufsize = MSH_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "msh: ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        else
            buffer[position] = c;
        position++;

        if (position >= bufsize) {
            bufsize += MSH_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "msh: ошибка выделения памяти\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}*/

char* msh_read_line(void)
{
    char *line = NULL;
    ssize_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char** msh_split_line(char* line)
{
    int bufsize = MSH_TOK_BUFSIZE;
    int position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;

    if (!tokens) {
        fprintf(stderr, "msh: ошибка выделения памяти\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "msh: ошибка выделения памяти\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


int msh_cd(char** args);
int msh_help(char** args);
int msh_exit(char** args);


char* builtin_str[] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[]) (char **) = {
        &msh_cd,
        &msh_help,
        &msh_exit
};

int msh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}


int msh_cd(char** args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "msh: ожидается аргумент для \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("msh");
        }
    }
    return 1;
}

int msh_help(char** args)
{
    int i;
    printf("Наберите название программы и её аргументы и нажмите enter.\n");
    printf("Вот список встроенных команд:\n");

    for (i = 0; i < msh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Используйте команду man для получения информации по другим программам.\n");
    return 1;
}

int msh_exit(char** args)
{
    return 0;
}

int msh_launch(char** args)
{
    pid_t pid;
    pid_t wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if(execvp(args[0], args) == -1) {
            perror("msh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("msh");
    } else {
        // Родительский процесс
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


int msh_execute(char** args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < msh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return msh_launch(args);
}

void msh_loop(void)
{
    char* line;
    char** args;
    int status;

    do {
        printf("msh> ");
        line = msh_read_line();
        args = msh_split_line(line);
        status = msh_execute(args);

        free(line);
        free(args);
    } while (status);
}



int main(int argc, char** argv)
{
    msh_loop();
    return 0;
}
