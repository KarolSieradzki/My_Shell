#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <windows.h>
#include <string.h>
#include <direct.h>
#include <sstream>

#define SH_BUFF_SIZE 1024
char* sh_get_line(){
    char* buffer;
    int buff_size = SH_BUFF_SIZE;
    int position = 0;

    buffer = (char*)malloc(buff_size);
    if(NULL == buffer){
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    int c;
    while(1){
        c = getchar();
        if(EOF == c || '\n' == c){
            buffer[position] = '\0';
            return buffer;
        }
        else{
            buffer[position] = c;
        }
        position++;

        if(position>=buff_size){
            buff_size += SH_BUFF_SIZE;
            buffer = (char*) realloc(buffer, buff_size);

            if(NULL == buffer){
                fprintf(stderr, "Allocation Error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define SH_ARG_BUFF_SIZE 64
char** sh_split_line(char* line){
    int buff_size = SH_ARG_BUFF_SIZE;
    int position = 0;
    char** args = (char**) malloc(buff_size*sizeof(char*));
    char* arg;

    if(!args){
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
    }

    arg = strtok(line, " \t\r\n\a");
    while(NULL != arg){
        args[position] = arg;
        position++;

        if(position>=buff_size){
            buff_size += SH_ARG_BUFF_SIZE;
            args = (char**) realloc(args, buff_size * sizeof(char*));

            if(!args){
                fprintf(stderr, "Allocation Error\n");
                exit(EXIT_FAILURE);
            }
        }
        arg = strtok(NULL, " \t\r\n\a");
    }
    args[position] = NULL;

    return args;
}

int sh_exit(char** args);
int sh_help(char** args);
int sh_chdir(char** args);
int sh_cls(char** args);
int sh_ch_console_colors(char** args);
int sh_gotoxy(char** args);

const char* builtin_str[]={
    "exit",
    "help",
    "chdir",
    "cls",
    "set_color",
    "gotoxy"
};

int (*builtin_func[])(char**)={
    &sh_exit,
    &sh_help,
    &sh_chdir,
    &sh_cls,
    &sh_ch_console_colors,
    &sh_gotoxy
};

int num_of_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}

int sh_launch(char** args){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if(NULL == args[1])
    {
        printf("Usage: %s [cmdline]\n", args[0]);
        return 1;
    }

    if(!CreateProcess(
        NULL, args[1], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi
    )){
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return 1;
    }
    //wait until child process exits
    WaitForSingleObject(pi.hProcess, INFINITE);
    //close process and threads handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 1;
}

int sh_execute(char** args){
    if(args[0] == NULL){
        return 1;
    }

    for(int i=0;i<num_of_builtins();i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

void sh_loop(){
    char* line;
    char** args;
    int status;

    do{
        printf("> ");
        line = sh_get_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    }while(status);

}

int main(int argc, char** argv){

    sh_loop();
    return EXIT_SUCCESS;
}

int sh_exit(char** args){
    if(args[1] != NULL)
        printf("%s \n", args[1]);

    return 0;
}

int sh_help(char** args){
    printf("wpisujesz polecenia i klikasz enter\n\n");
    printf("do programow z zewnatrz dopisujesz cmd(np cmd notepad)\n\n");
    printf("wbudowane polecenia:\n");
    for(int i = 0;i<num_of_builtins();i++){
        printf("%s\n", builtin_str[i]);
    }

    return 1;
}

int sh_chdir(char** args){
    if(args[1] == NULL){
        fprintf(stderr, "expected argument to \"chdir\"\n");
    }
    else{
        if(_chdir(args[1])){
            switch (errno){
                case ENOENT:
                    printf( "Unable to locate the directory: %s\n", args[1] );
                    break;
                case EINVAL:
                    printf( "Invalid buffer.\n");
                    break;
                default:
                    printf( "Unknown error.\n");
            }
        }
    }
    return 1;
}

int sh_cls(char** args){
    system("cls");

    return 1;
}

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int sh_ch_console_colors(char** args){

    if(args[1] == NULL){
        fprintf(stderr, "expected argument to \"set_color\"\n");
    }else{
        std::stringstream s(args[1]);
        int color = 0;
        s>>color;
        SetConsoleTextAttribute(hConsole, color);
    }

    return 1;
}

int sh_gotoxy(char** args){
    if(args[2] == NULL){
        fprintf(stderr, "expected arguments to \"gotoxy\"\n");
    }else{
        std::stringstream x(args[1]);
        std::stringstream y(args[2]);
        int _x = 0, _y = 0;
        x>>_x; y>>_y;

        COORD c;
        c.X = _x - 1;
        c.Y = _y - 1;
        SetConsoleCursorPosition( hConsole, c );
    }

    return 1;
}