#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void setup_environment();
void shell();
void parse_input(char *command[1000]);
char get_command_type(char command[]);
void execute_shell_bultin(char* command[1000]);
void remove_quotes(char input[]);
void evaluate_expression(char *command[1000]);
void export(char *command[1000]);
void echo(char *command[1000]);
void cd(char * command[1000]);
void execute_command(char *command[1000]);
void on_child_exit();
void register_child_signal(void (*on_child_exit)(int));
void get_args(char *command[1000], char *args[1000]);

int main() {
    register_child_signal(on_child_exit);
    setup_environment();
    shell();
    return 0;
}

void setup_environment(){
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("Setup Error");
        exit(1);
    }
    if(chdir(cwd) == -1){
        perror("Change Dir Error");
        exit(1);
    }
}
void shell(){
    char *command[1000];
    do{
        parse_input(command);
        evaluate_expression(command);
        switch(get_command_type(command[0])){
            case '1':
                execute_shell_bultin(command); break;
            case '2':
                execute_command(command); break;
        }
    } while (strcmp(command[0],"exit") != 0);
}

void evaluate_expression(char *command[1000]) {
    for(int i = 1; command[i] != NULL; i++){
        if (*command[i] == '$') {
            *command[i] = '\0';
            command[i]++;
            command[i] = getenv(command[i]);
        }
    }
}

void parse_input(char* command[1000]){
    char input[1000];
    int index = 0;

    fgets(input,1000,stdin);
    input[strcspn(input,"\n")] = '\0';
    remove_quotes(input);
    char *token = strtok(input, " ");

    while(token!=NULL){
        command[index++] = token;
        token = strtok(NULL, " ");
    }
    for(int i = index; i <1000; i++){
        command[i] =NULL;
    }
}

char get_command_type(char command[]){
    if(!strcmp(command,"exit")){
        return '3';
    }else if(strcmp(command,"cd")&&strcmp(command,"echo")&&strcmp(command,"export")){
        return '2';
    }
        return '1';
}

void execute_shell_bultin(char* command[1000]){
    if(strcmp(command[0],"cd") == 0){
        cd(command);
    }else if(strcmp(command[0],"echo") == 0){
        echo(command);
    }else if(strcmp(command[0],"export") == 0){
        export(command);
    }else{
        perror("Command doesnt exist");
        return;
    }
}

void export(char *command[1000]){
    char *name, *index;
    index = strchr(command[1], '=');
    *index = '\0';
    name = command[1];
    index++;
    char value[1000];

    strcpy(value,index);
    strcat(value," ");
    for(int i=2;command[i]!=NULL;i++){
        strcat(value,command[i]);
        strcat(value," ");
    }
    setenv(name, value, 1);
}

void remove_quotes(char input[]){
    char* p= strchr(input, '\"');
    if(p==NULL){ return;}
    memmove(p, p+1, strlen(p));
    input[strlen(input) - 1] = '\0';
}

void echo(char *command[1000]){
    for (int i = 1; command[i] != NULL; ++i){
        printf("%s ", command[i]);
    }
    printf("\n");
}

void cd(char *command[1000]){
    static char last_dir[1000];
    char *dir,current_dir[1000];

    if (command[1] == NULL || strcmp(command[1], "~") == 0 ){
        dir = getenv("HOME");
    }else if (strcmp(command[1], "..") == 0) {
        dir = "..";
    }  else {
        dir = command[1];
    }

    if(getcwd(current_dir, sizeof(current_dir)) == NULL){
        perror("Unreachable dir");
        return;
    }

    if(chdir(dir) == -1){
        perror("Change Dir Error");
        return;
    }
    memcpy(last_dir, current_dir, sizeof(last_dir));
}

void execute_command(char *command[1000]) {
    pid_t childPid = fork();

    if (childPid < 0) {
        perror("Child was not created");
        exit(1);
    } else if (childPid == 0) {
        if (command[1] && strcmp(command[1], "&") == 0) {
            command[1] = '\0';
        }
        char *args[1000];
        get_args(command,args);
        execvp(command[0], command);
        perror("Error");
        exit(1);
    } else {
        if (command[1] && strcmp(command[1], "&") == 0) {return;}
        waitpid(childPid, 0, 0);
    }
}

void reap_child_zombie(){
    while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

void write_to_log_file(char line[]){
    FILE *log;

    log = fopen("/home/mahmoud/CLionProjects/os/log.txt", "a");
    if (log == NULL)
        return;

    fprintf(log, "%s", line);
    fclose(log);
}

void on_child_exit(){
    reap_child_zombie();
    write_to_log_file("Child terminated\n");
}

void register_child_signal(void (*on_child_exit)(int)) {
    signal(SIGCHLD, on_child_exit);
}

void get_args(char *command[1000], char *args[1000]) {
    int index = 1;
    char *ptr;
    args[0] = command[0];
    for (int i = 1; command[i]!=NULL; i++) {
        ptr = strtok(command[i], " ");
        while (ptr != NULL) {
            args[index++] = ptr;
            ptr = strtok(NULL, " ");
        }
    }
}