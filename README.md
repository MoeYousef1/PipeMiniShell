Ex3

==Description==

 This program does the same as (ex2.b) and in addition we added the pipe to the program in thes ex, the user can enter commands with up tp two pipes only, and each time, the output 

of the first command will act as the input of the second command, using the pipe , also similer to ex2.b the user was able to use !num to call a command from the file and re execute it

and in this program the user will be able to call two or three commands fromm the file and do a pipe between them in this way >>>> !num|!num.

if the user entered nohup before the any of the commands the output of the command will  be 
sent to a file with the name of noHup.txt , at the same time if we had a command running in 
the background with nohup, if we exit the program, the command will keep on running.

if the user entered a command with ampersand at the end of it , ot means that the command will run in the background , and we added a method to kill it inorder not to end up with zombie process.

the user cant use more than two pipes , and can't start or end with a pipe.
i added a statment to prevent the user from entering double ampersand marks.

every other thing is the same as ex2.b , cd is not supported yet , history will display the command lines that we used before, done will exit and print num of commands and total words in all commands, also we can't start or end with a space.

In my program everything counts as a COMMAND except (only enter, only spaces).

=Program DATABASE=

History: it contains the strings which were entered by the user before, and we can get to them just by entering the string (history).
Array of chars (str):  it’s used to store all the characters of each string, it must be up to 512 chars, it’s also used to read the strings from the history file.
Command Array: it’s used with strtok to pass the words of each command into the (argv Array).
Argv Array: stores the commands, each word in a different index.
Array path: to store the current dir.
Array arrOfPipes: it contains the fd1 , fd2 in order to pass it to pipe.
fromFile: stores the lines that we get from the history.
fh,sh,th,fh1,sh1,th1= all are parts of the string devided by the pipes.
file.txt: stores the commands 
noHup.txt: stores the output of the commands if we wrtie it before them.
char *pipe : used it to devide the string by the pipes.

Functions:
25 main functions 

1)	wordCounter: this method receives a string and counts the number of words the string has
2)	charCounter: this method receives a string and counts the number of chars the string has, skipping spaces.
3)basicMethod: it does all the things mentioned above
4)killZombies:to kill the background process
5)spaceRemover:removes white spaces from the beginning and the end of the string
6)pipeCount: counts Pipes
7)ampersandCount: counts ampersandCounts
8)exclamationCount: counts exclamation marks
9)onePipe:gets first and second command either from the file or from the user input and sends them to workOnepipe
10)twoPipes:gets first, second and third command either from the file or from the user input and sends them to workTwopipe
11)charRemover: this method will delete the specified character
12)workOnePipe: combines the the strings that we get from the file and sends the combined string to workPipe
13)workTwoPipe: same as workOnePipe but with two pipes.
14)workPipe:if we have pipes with exclamation marks, it checks if we have ampersand also, and it splits the string by the pipe and sends the commands to excute, also it prints the commands in the history file.
15)workNoPipe:same as workPipe but it works if we have only pipes or only one exclamation mark and no pipe.
16)executes every command including noHup and the pipes, it contains the child,father process.
17)fromFile: works on the string with one execlamation mark and zero pipe, prings it from the history, re executes it, reprints it in the history file.
18)basicCommands: works on normal commands , no execlamation marks. does the same as above
but on user input and not from the file.
19)exclamationMark:gets an exclemation mark with a number and returns the command in that number line from the history file.
20)stringTillPipe:returns string before the first pipe
21)stringAfterPipe:returns string after first pipe
22)stringBetweenPipe:returns string between the pipes
23)stringAfterTwoPipes:returns the string after the two pipes
24)combie: returns the combined string that we get form methods 20,21 , and adds pipe  between them.
25)combine2:returns the combined string the we get from methods 20,22,23, and adds pipe between them.

==Program Files==
PipeShell.c      contains the main and the functions

==How to compile==
Run: PipeShell.c 
Or through the Terminal using gcc PipeShell.c -o PipeShell
Then running it by typing  ./PipeShell

==Input==
A string (command)

==Output==
If the string is history, the output will be the history containing all the previous commands that were entered, if there were no previous commands then the output will be NO HISTORY YET.
If the sting is done, the output will be the NUM OF COMMANDS and the TOTAL NUM OF WORDS IN ALL COMMANDS and the program will be exited.
If the string is cd, the output will be COMMAND NOT SUPPORTED (YET)
If the string was entered with spaces before or after it, the output will be NO SPACE ALLOWD
If the user pressed only enter or spaces then enter, there will be no output.
In every time enter is pressed the path will be printed again.
if the user enterd double && it will print a message saying its not allowed
if the user used mor ethan two pipes it will say its not allowed
prints the output of linux commands
if the user wrote nohup first the output will be printed into the noHup.txt
and other error messages if any error was noticed
