#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <linux/limits.h>

#define MAX_COMMANDS    32
#define MAX_LINE 2048
#define MAXIMUM 512

static int x = 1; // using it as a counter for the background process
static int boolean = 0; // using it in order to know if we have the ampersand mark
static int add = 0; // if its equal to 1 we stop the x count
static int keep = 0, save = 0; //variables for num of commands and total words in all commands
static int noHupProcess = 0;// in order to check if we have nohup

void basicMethod();

void killZombies();

char *spaceRemover(char *str);

int pipeCount(const char *str);

int wordCounter(const char *str);

int charCounter(const char *str);

void onePipe(char *str, FILE *fp);

void charRemover(char *s, char c);

void twoPipes(char *str, FILE *fp);

void workPipe(char *str, FILE *fp);

int ampersandCount(const char *str);

void workNoPipe(char *str, FILE *fp);

char *stringTillPipe(const char *str);

int exclamationCount(const char *str);

char *stringAfterPipe(const char *str);

void basicCommands(char *str, FILE *fp);

char *stringBetweenPipe(const char *str);

char *exclamationMark(char *str, FILE *fp);

char *stringAfterTwoPipe(const char *str);

void fromFIle(char *str, char *fromFile, FILE *fp);

char *combine(char *str, char *fh, char *sh, FILE *fp);

void workOnePipe(char *str, char *fh, char *sh, FILE *fp);

static int execution(char *str, int input, int fd1, int fd2);

char *combine2(char *str, char *fh, char *sh, char *th, FILE *fp);

void workTwoPipes(char *str, char *fh, char *sh, char *th, FILE *fp);

int main() {
    signal(SIGCHLD, killZombies);
    basicMethod();
    return 0;
}

void basicMethod() {
    int i;
    FILE *fp;
    char path[PATH_MAX];
    getcwd(path, sizeof(path)); // get the file path
    while (1) {
        boolean = 0, add = 0, noHupProcess = 0;
        char *str = (char *) malloc(sizeof(char) * MAXIMUM); //allocate memory for user input
        if (str == NULL) {
            perror("Allocation Failed");
            exit(1);
        }
        printf("%s>", path);
        fgets(str, MAXIMUM, stdin);// get user input
        int input = 0, fd1 = 1;
        if ((strlen(str) > 0) && (str[strlen(str) - 1] == '\n')) {
            str[strlen(str) - 1] = '\0';
        }
        if (strlen(str) == 0 || charCounter(str) == 0) { // in case of empty string
            free(str);
            continue;
        } else {
            int check = pipeCount(str);
            save += wordCounter(str); // to get the total num of words on all commands
            keep += 1; // to get num of commands
            if (strncmp(str, "nohup", 5) == 0) {
                noHupProcess = 1; // changed to 1 if we have nohup at the start of the string
                signal(SIGHUP, SIG_IGN); // signal to ignore the exit and keep running after we exit
                execution(str, input, fd1, 0); //send it to execute
                free(str);
            } else if (str[0] == ' ') { // check for spaces
                save -= 1;
                printf("Spaces BEFORE/AFTER commands are not allowed\n");
                free(str);
            } else if (str[strlen(str) - 1] == ' ' && str[0] == ' ') {
                free(str);
                save -= 1;
            } else if (str[strlen(str) - 1] == ' ') {
                printf("Spaces BEFORE/AFTER commands are not allowed\n");
                free(str);
            } else if (ampersandCount(str) > 1) {// check for double ampersand
                printf("Not allowed to use double ampersand (Yet)\n");
                free(str);
                continue;
            } else if ((str[0] == '|') || (str[strlen(str) - 1] == '|')) { // check if we have a pipe at the beginning or at the end of the string
                printf("Can't start or end with a pipe\n");
                free(str);
            } else if (check > 2) { // can't use more than two pipes
                printf("One or Two pipes are allowed (ONLY)\n");
                free(str);
                continue;
            } else if (strcmp(str, "done") == 0) {// check for string = done
                printf("Num of commands: %d\n", keep);
                printf("Total number of words in all commands: %d !\n", save - 1);
                free(str);
                exit(0);
            } else if (strcmp(str, "cd") == 0) { // if we have cd then print this message
                printf("command not supported (Yet)\n");
                free(str);
            } else if (strcmp(str, "history") == 0) { // check for string = history
                fp = fopen("file.txt", "r+"); // open file to update read and write
                if (fp == NULL) {
                    printf("There is no HISTORY atm\n"); // if the file is NULL we print this message
                    fp = fopen("file.txt", "w"); // then we open the file for write
                    fprintf(fp, "%s\n", str);// we print the word history in the file
                } else {
                    i = 1; // start from 1
                    while (fscanf(fp, "%[^\n]%*c", str) != EOF) { // read from the file
                        // %[^\n]%*c , will help you to get an entire sentence
                        // until the next line isn’t encountered “\n” or enter is pressed
                        printf("%d: %s\n", i, str);// print strings from the history to the screen , (numbered)
                        i++;
                    }
                    fprintf(fp, "%s\n", "history");// here we print (history) in the file too
                }
                fclose(fp);
                free(str);
            } else if (str[0] == '!') { // if the first char is (!)
                char *fromFile = (char *) malloc(sizeof(char) * MAXIMUM); // allocate memory for the string that we get from the file
                if (fromFile == NULL) {// check if string is NULL
                    perror("Allocation Failed");
                    exit(1);
                }
                if (pipeCount(str) == 0 && exclamationCount(str) == 1) { // if we have one exclamation mark and zero pipe

                    fromFIle(str, fromFile, fp); // go here
                } else if (pipeCount(str) == 1) { // if we have one pipe
                    onePipe(str, fp);
                } else if (pipeCount(str) == 2) { // if we have two pipes
                    twoPipes(str, fp);
                }
            } else if (pipeCount(str) == 1 && str[0] != '!' && exclamationCount(str) != 0) {// also if the first string is not (!) but we have exclamation mark and on pipe
                onePipe(str, fp);
            } else if (pipeCount(str) == 2 && str[0] != '!' && exclamationCount(str) != 0) {//   = = = = = = = = = = = = = = = = = = = = = = = = = = == = = = = = two pipes
                twoPipes(str, fp);

            } else if (exclamationCount(str) == 0) {// if we have normal commands with no exclamation marks
                basicCommands(str, fp);

            }
        }
    }
}

void twoPipes(char *str, FILE *fp) {
    if (exclamationCount(str) == 3) { // 3 exclamation marks
        char *fh = exclamationMark(stringTillPipe(str), fp);// get the first command from the history file
        char *sh = exclamationMark(stringBetweenPipe(str), fp);// get second command from history file
        char *th = exclamationMark(stringAfterTwoPipe(str), fp);// get third command from history file
        workTwoPipes(str, fh, sh, th, fp); // send them to workTwoPipes
    } else if (exclamationCount(str) == 2) {// 2 exclamation marks
        if (str[0] != '!') {// if first char is not (!)
            if (exclamationCount(stringBetweenPipe(str)) == 1) {  // if the middle command is with exclamation mark
                char *fh1 = stringTillPipe(str); // we get first command from user input
                char *sh = exclamationMark(stringBetweenPipe(str), fp);// we get second command from history file
                char *th = exclamationMark(stringAfterTwoPipe(str), fp);// we get third command from history file
                workTwoPipes(str, fh1, sh, th, fp);
            }
        } else {//if first char is (!)
            if (exclamationCount(stringBetweenPipe(str)) == 1) {// if the middle command has an exclamation mark
                char *fh = exclamationMark(stringTillPipe(str), fp);// get first command from history file
                char *sh = exclamationMark(stringBetweenPipe(str), fp);// from history file
                char *th1 = stringAfterTwoPipe(str);// from user input
                workTwoPipes(str, fh, sh, th1, fp);
            } else if (exclamationCount(stringAfterTwoPipe(str)) == 1) {// last command has an exclamation mark
                char *fh = exclamationMark(stringTillPipe(str), fp); // from file
                char *sh1 = stringBetweenPipe(str);// user input
                char *th = exclamationMark(stringAfterTwoPipe(str), fp);//from file
                workTwoPipes(str, fh, sh1, th, fp);
            }
        }
    } else if (exclamationCount(str) == 1) { // we do the same process if we have one exclamation mark
        if (str[0] != '!') {
            if (exclamationCount(stringBetweenPipe(str)) == 1) {
                char *fh1 = stringTillPipe(str);
                char *sh = exclamationMark(stringBetweenPipe(str), fp);
                char *th1 = stringAfterTwoPipe(str);
                workTwoPipes(str, fh1, sh, th1, fp);
            } else {
                char *fh1 = stringTillPipe(str);
                char *sh1 = stringBetweenPipe(str);
                char *th = exclamationMark(stringAfterTwoPipe(str), fp);
                workTwoPipes(str, fh1, sh1, th, fp);
            }
        } else {
            char *fh = exclamationMark(stringTillPipe(str), fp);
            char *sh1 = stringBetweenPipe(str);
            char *th1 = stringAfterTwoPipe(str);
            workTwoPipes(str, fh, sh1, th1, fp);
        }
    }
}

void onePipe(char *str, FILE *fp) {// same process as twwPipes but we only have 2 commands
    if (exclamationCount(str) == 2) {
        char *fh = exclamationMark(stringTillPipe(str), fp);
        char *sh = exclamationMark(stringAfterPipe(str), fp);
        workOnePipe(str, fh, sh, fp);
    } else if (exclamationCount(str) == 1) {
        if (str[0] != '!') {
            char *fh1 = stringTillPipe(str);
            char *sh = exclamationMark(stringAfterPipe(str), fp);
            workOnePipe(str, fh1, sh, fp);
        } else {
            char *fh = exclamationMark(stringTillPipe(str), fp);
            char *sh1 = stringAfterPipe(str);
            workOnePipe(str, fh, sh1, fp);
        }
    }
    strcpy(stringTillPipe(str), "");
    strcpy(stringAfterPipe(str), "");
}

void workTwoPipes(char *str, char *fh, char *sh, char *th, FILE *fp) {
    char *both = combine2(str, fh, sh, th, fp); // combine all three commands
    workPipe(both, fp);
}

void workOnePipe(char *str, char *fh, char *sh, FILE *fp) {
    char *both = combine(str, fh, sh, fp); // combine both commands
    workPipe(both, fp);
}

void workPipe(char *str, FILE *fp) { // if we have a pipe and exclamation marks
    int input = 0;
    int fd1 = 1;
    if (str[strlen(str) - 1] == '&' && pipeCount(str) <= 2) { // if last char is ampersand
        fp = fopen("file.txt", "a"); // open the file and print the string with the ampersand in the file
        fprintf(fp, "%s\n", str);
        fclose(fp);
        printf("%s\n", str);// print the string on the screen
        str[strlen(str) - 2] = '\0';// delete the ampersand before passing the command to execvp
        boolean = 1;
    } else if (str[strlen(str) - 1] != '&' && pipeCount(str) <= 2) { // if we don't have ampersand
        fp = fopen("file.txt", "a");
        fprintf(fp, "%s\n", str);
        fclose(fp);
        printf("%s\n", str);
    }
    if (pipeCount(str) <= 2) { // other than that
        charRemover(str, '"'); // delete all """" marks
        char *pipe = strchr(str, '|'); // using strChr to get the string starting from the first pipe till the end of the string if str = A|B then  pipe = |B

        while (pipe != NULL) {// as long as pipe is not NULL
            *pipe = '\0';//point to \0
            input = execution(str, input, fd1, 0);// input will be the output of the first command

            strcpy(str, pipe + 1); // copy pipe +1 which is (B) in the example into str
            pipe = strchr(str, '|'); // then continue looking for another pipe
            fd1 = 0; // fd1 is back to zero , in order not to go on the first command again
        }
        input = execution(str, input, fd1, 1); // execute last command after all the pipes

    } else { // if the line in the file had more than two pipes
        strcpy(stringTillPipe(str), "");
        strcpy(stringAfterPipe(str), "");
        printf("You are calling a line with more than two pipes!\n");
        basicMethod();
    }
    free(str);

}

void workNoPipe(char *str, FILE *fp) { // If we have exclamation marks with no pipes or the opposite or a normal command

    int input = 0;
    int fd1 = 1;
    if (str[strlen(str) - 2] == '&') { // check for & and do the same as above
        fp = fopen("file.txt", "a");
        fprintf(fp, "%s", str);
        fclose(fp);
        str[strlen(str) - 2] = '\0';
        boolean = 1;
    } else if (str[strlen(str) - 1] == '&') { // check for & special case
        fp = fopen("file.txt", "a");
        fprintf(fp, "%s\n", str);
        fclose(fp);
        str[strlen(str) - 1] = '\0';
        boolean = 1;
    } else {// we have no &
        fp = fopen("file.txt", "a");
        fprintf(fp, "%s\n", str);
        fclose(fp);
    }
    charRemover(str, '"'); // remove all """" marks
    char *pipe = strchr(str, '|'); // same as the code in workPipe method

    while (pipe != NULL) {
        *pipe = '\0';
        input = execution(str, input, fd1, 0);
        strcpy(str, pipe + 1);
        pipe = strchr(str, '|');
        fd1 = 0;
    }
    input = execution(str, input, fd1, 1);
    free(str);

}

static int execution(char *str, int input, int fd1, int fd2) {
    int arrOfPipes[2]; // array that has fd1 , fd2 and fd2 can be read from fd1
    pipe(arrOfPipes); // calling pipe
    pid_t pid = fork();
    char *command;
    char *argv[MAX_COMMANDS]; // array to pass to execvp
    int i = 0;

    command = strtok(str, " \n"); // using strtok to split the commands
    while (command != NULL) { // as long as it's not null
        argv[i] = command; // pass the command to argv in index i
        ++i;
        command = strtok(NULL, " \n");//in order to get all the separated words in the str we pass NULL to strtok , if there are no more words it returns NULL
    }
    { argv[i] = NULL; }// last index is NULL
    int fd;
    if (noHupProcess == 1) {// if we find nohup
        fd = open("noHup.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // open this file
    }
    if (pid < 0) { // child was not entered
        perror("Child process was not entered");
        exit(1);
    } else if (pid == 0) { // child process
        prctl(PR_SET_PDEATHSIG, SIGHUP); // in order to stop the background running commands after we exit
        if (fd1 == 1 && fd2 == 0 && input == 0) { // this means that we are linking the first command to STDOUT
            dup2(arrOfPipes[1], STDOUT_FILENO);

        } else if (fd1 == 0 && fd2 == 0 && input != 0) { // then we go to the middle command between the pipes
            dup2(input, STDIN_FILENO); // its input is the STDOUT of the first command
            dup2(arrOfPipes[1], STDOUT_FILENO); // then we link its output to STDOUT

        } else {

            dup2(input, STDIN_FILENO); // last command will get the input from the middle command
        }
        if (noHupProcess == 1) { // if we have nohup
            dup2(fd, STDOUT_FILENO); // link the df to the output ,so it's printed in the file
            if (execvp(argv[1], &argv[1]) <= -1) // do execvp to every command other starting after nohup
                perror("Execvp error");
            exit(0);
        } else if (execvp(argv[0], argv) <= -1)// else we execute normally
            perror("Execvp error");
        exit(0);
    } else if (boolean != 1) { // if we don't have ampersand , father waits for the child
        waitpid(pid, NULL, 0);
    }
    if (boolean == 1 && add == 0) { // if we have ampersand we print the number of the command running in the background and its pid
        printf("[%d] %d\n", x, pid);
        x++;
        add = 1;//when add is equal to one we stop printing
    }
    close(arrOfPipes[1]);// close the array
    return arrOfPipes[0];
}

void fromFIle(char *str, char *fromFile, FILE *fp) { // this method is for the exclamation mark without pipe

    int i;
    fp = fopen("file.txt", "r+");// open the file for read/write
    if (fp == NULL) {
        printf("There are no commands in the history atm\n");
        basicMethod();

    } else {
        int notDone = 1;// a boolean variable used as int
        int cur = 1;// a variable to run on the lines of file
        char curToChar[MAX_LINE];// an array with length of MAx_line
        //the cur will be converted into a string and saved in the curToChar
        while (notDone == 1) {// as long as we have lines in the file
            fgets(fromFile, MAX_LINE, fp); // read from the file and store into fromFile
            sprintf(curToChar, "%d", cur);// used sprintf in order to convert cur to string then store the string in curToChar
            if (feof(fp)) {// if we are out of lines in the file
                fclose(fp);
                notDone = 0;
                printf("The line number %s was not found in the file \n", str + 1);

            } else if (strcmp(curToChar, str + 1) == 0) {// if we find the line we are looking for
                notDone = 0;
                if (pipeCount(fromFile) > 2) {
                    printf("You are calling a line with more than two pipes!\n");// more than two pipes
                    basicMethod();
                } else {
                    notDone = 0;
                    printf("%s", fromFile); // print the line

                    if (strncmp(fromFile, "history", 7) == 0 && strlen(fromFile) - 1 == 7) { // if line is history we do the same as above

                        fp = fopen("file.txt", "r+");
                        i = 1;
                        while (fscanf(fp, "%[^\n]%*c", str) != EOF) {

                            printf("%d: %s\n", i, str);
                            i++;
                        }
                        fprintf(fp, "%s", fromFile);
                        fclose(fp);

                    } else {
                        charRemover(fromFile, '\n'); // remove \n
                        workNoPipe(fromFile, fp);
                    }
                }
            }
            cur++;
        }
    }
}

void basicCommands(char *str, FILE *fp) { // for normal methods
    {
        workNoPipe(str, fp);
    }
}

char *exclamationMark(char *str, FILE *fp) { // this method will convert the number with the exclamation mark into the command line in the file in that line

    char *fromFile = (char *) malloc(sizeof(char) * MAXIMUM);
    if (fromFile == NULL) {
        perror("Allocation Failed");
        exit(1);
    }
    fp = fopen("file.txt", "r+"); // open file for read and write

    if (fp == NULL) {
        printf("There are no commands in the history atm\n");
        basicMethod();
    }
    int notDone = 1; // doing the same as fromFIle method, but we don't check for history in here
    int cur = 1;
    char curToChar[MAX_LINE];
    while (notDone == 1) {
        fgets(fromFile, MAX_LINE, fp);
        sprintf(curToChar, "%d", cur);
        if (feof(fp)) {
            notDone = 0;
            printf("One or More of the lines are not in the file\n");
            basicMethod();

        } else if (strcmp(curToChar, str + 1) == 0) {
            notDone = 0;
        }
        cur++;
    }
    fclose(fp);
    return fromFile;
}

int wordCounter(const char *str) { // counts the words in each entered string
    int counter = 0;
    int i = 0;
    while (str[i] != '\0') { // till we reach the end of the string
        if (str[i + 1] != ' ' && str[i] == ' ') {
            counter++;
        }
        if (str[i + 1] == '\0' && str[i] == ' ') {
            return counter;
        }
        i++;
    }
    return counter + 1;
}

int pipeCount(const char *str) { // each time we find a pipe we increase the pipeCount
    int numPipes = 0;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '|') {
            numPipes++;
        }
        i++;
    }
    return numPipes;
}

int exclamationCount(const char *str) {// same as pipeCount but for exclamation mark
    int numEX = 0;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '!') {
            numEX++;
        }
        i++;
    }
    return numEX;
}

int ampersandCount(const char *str) {// same as above but for ampersand
    int ampersand = 0;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '&') {
            ampersand++;
        }
        i++;
    }
    return ampersand;
}

int charCounter(const char *str) { // a method to counts chars except of spaces
    int counter = 0;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] != ' ') {
            counter++;
        }
        i++;
    }
    return counter;
}

char *stringTillPipe(const char *str) { // this method will give me the string before the first pipe
    char *fh = (char *) malloc(sizeof(char) * MAXIMUM);
    if (fh == NULL) {
        perror("Allocation Failed");
        exit(1);
    }
    int i = 0;
    while (str[i] != '|' && pipeCount(str) <= 2) {
        fh[i] = str[i];
        i++;
    }
    spaceRemover(fh);// remove white spaces
    return fh;
}

char *stringAfterPipe(const char *str) {// this will give me the string after the first pipe
    char *pipe = strchr(str, '|');
    char *sh = pipe + 1;
    spaceRemover(sh);
    return sh;
}

char *stringBetweenPipe(const char *str) { // this will give the string between the two pipes
    char *sh = (char *) malloc(sizeof(char) * MAXIMUM);
    if (sh == NULL) {
        perror("Allocation Failed");
        exit(1);
    }
    sh = stringAfterPipe(str);
    sh = stringTillPipe(sh);
    spaceRemover(sh);
    return sh;
}

char *stringAfterTwoPipe(const char *str) {// this will give the string after the second pipe
    char *th = stringAfterPipe(str);
    th = stringAfterPipe(th);
    spaceRemover(th);// clean white spaces in the string
    return th;
}

char *spaceRemover(char *str) {
    int i, j;
    for (i = 0; str[i] == ' '; i++);

    for (j = 0; str[i]; i++) {
        str[j++] = str[i];
    }
    str[j] = '\0';
    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ')
            j = i;
    }
    str[j + 1] = '\0';
    return str;
}

void charRemover(char *str, char c) {// this method will delete the specified character
    int A = 0, B = 0;
    while (str[B]) {
        if (str[B] != c) {
            str[A++] = str[B];
        }
        B++;
    }
    str[A] = 0;
}

char *combine(char *str, char *fh, char *sh, FILE *fp) {// this method will combine the two strings that we get from the file and add a pipe between them
    int i;
    for (i = 0; fh[i] != '\0'; i++); // pass on the fh without doing anything
    fh[i] = '|';// add the pipe
    for (int j = 0; sh[j] != '\0'; j++) { // then add the sh to it
        fh[i + 1] = sh[j];
        i++;
    }
    fh[i + 2] = '\0';
    charRemover(fh, '\n');// remove \n
    return fh;
}

char *combine2(char *str, char *fh, char *sh, char *th, FILE *fp) {// same as combine but with two pipes and three strings to combine
    int i;
    for (i = 0; fh[i] != '\0'; i++);
    fh[i] = '|';
    for (int j = 0; sh[j] != '\0'; j++) {
        fh[i + 1] = sh[j];
        i++;
    }
    fh[i + 1] = '|';
    for (int k = 0; th[k] != '\0'; k++) {
        fh[i + 2] = th[k];
        i++;
    }
    fh[i + 2] = '\0';
    charRemover(fh, '\n');
    return fh;
}

void killZombies() { // this method will make sure that we don't have the background commands as zombie process
    int status;
    waitpid(-1, &status, WNOHANG);
}