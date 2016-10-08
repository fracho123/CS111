#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>

struct termios saved_attr;
pid_t pid = 0;
pthread_t tid = 0;
int shellFlag = 0;
int pipe_toshell[2], pipe_fromshell[2];

//returns to saved terminal modes
void reset_to_saved()
{
        tcsetattr (STDIN_FILENO, TCSANOW, &saved_attr);
}

//prints the shell's exit status when the program closes
void getExitStatus()
{
	if (shellFlag)
	{
		reset_to_saved();
        	int status;
        	waitpid(pid, &status, 0);
        	if (WIFEXITED(status))
        		{printf("shell exited with status: %d\n", WEXITSTATUS(status));}
        	else if (WIFSIGNALED(status))
        		{printf("shell exited with signal: %d\n", WTERMSIG(status));}
		else
			{printf("Shell Exited Normally");}
	}
}

//sets the desired terminal modes
void set_terminal_mode()
{
	struct termios attr;
	
	//save curernt terminal modes
	tcgetattr (STDIN_FILENO, &saved_attr);
	atexit(getExitStatus);

	//set terminal modes
	tcgetattr (STDIN_FILENO, &attr);
	attr.c_lflag &= ~(ICANON|ECHO);
	attr.c_cc[VMIN] = 1;
	attr.c_cc[VTIME] = 0;
	tcsetattr (STDIN_FILENO, TCSAFLUSH, &attr); //possibly TCSANOW
}

/*void int_handler(int signum)
{
        kill(pid, SIGINT);
}*/

void pipe_handler(int signum)
{
	//pthread_cancel(tid);
	//kill(pid, SIGPIPE);	
	close(pipe_toshell[0]);
	close(pipe_fromshell[0]);	
	close(pipe_toshell[1]);
	close(pipe_fromshell[1]);
        exit(1);
}

int main(int argc, char *argv[])
{
	/*OPTION HANDLING*/
        int optVar;
	while (1)
	{
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_opts[] = 
		{
			{"shell", no_argument, 0, 's'},
			{0, 0, 0, 0}
		};
		
		optVar = getopt_long(argc, argv, "s", long_opts, &option_index);
		if (optVar == -1)
			break;
		switch (optVar)
		{
			case 's': //shell option used
				shellFlag = 1;
				//signal(SIGINT, int_handler);
				signal(SIGPIPE, pipe_handler);
				break;
			default:
				fprintf(stderr, "Error: getOpt returned 0%o ??\n", optVar);
		}
	}	
	
	if (optind < argc)
	{
		printf("non-option elements passed: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}

	/*PART1*/
	int nchar = 0;
	char buf[100];
	int i = 0;
	set_terminal_mode();

	if (!shellFlag) //reads from stdin and echoes to stdout
	{
		while (nchar = read(STDIN_FILENO, buf, sizeof(buf)))
		{
			for (i = 0; i < nchar; i++)
			{
				if (buf[i] == '\n' || buf[i] == '\r')
				{
					char tmp[2] = {'\r', '\n'};
					write (STDOUT_FILENO, tmp, 2);	
				}
				else if (buf[i] == '\004')
				{
					reset_to_saved();
					exit(0);
				}
				else
					write (STDOUT_FILENO, buf + i, 1);
			}
		}
	}

	/*PART2*/
	if(shellFlag){

	pipe(pipe_toshell);
	pipe(pipe_fromshell);
	//sigset_t set;
	//sigemptyset(&set);
	//sigaddset(&set, SIGPIPE);
	//int s = pthread_sigmask(SIG_BLOCK, &set, NULL);
	char buf2[100];
	char buf3[100];	

	//read from shell function
	void *read_from_shell(void *arg)
	{
		close(pipe_toshell[0]);
		close(pipe_fromshell[1]);
		while (nchar = read(pipe_fromshell[0], buf2, sizeof(buf2)))
		{
			for (i = 0; i < nchar; i++)
                        {
                                if (buf2[i] == '\004')
                                {
                                        reset_to_saved();
                                        exit(1);
                                }
                                else 
                                        write (STDOUT_FILENO, buf2 + i, 1); 
                                
                        }
		}
		exit(1);
	//	pthread_exit(NULL);
	}
		
	//fork two processes
	
	pid = fork(); 
	/*if (pid < 0)
	{
		printf("Error in creating fork.\n");
		exit(3);
	}*/
	if (!pid)
	{
		close(0);
		dup(pipe_toshell[0]);
		close(1);
		dup(pipe_fromshell[1]);
		close(2);
		dup(pipe_fromshell[1]);
		execlp("/bin/bash", "/bin/bash", NULL);
	}	
	else
	{
		close(pipe_toshell[0]);
		close(pipe_fromshell[1]);		
		pthread_create (&tid, NULL, read_from_shell, /*(void *) &set*/ NULL); //commented out prevents SIGPIPE
		//reads from stdin and echoes to stdout and forwards to shell pipe
		while (nchar = read(STDIN_FILENO, buf3, sizeof(buf3))) 
		{
			//#3
			for (i = 0; i < nchar; i++)
                	{
                        	if (buf3[i] == '\n' || buf3[i] == '\r')
                        	{
                                	char tmp[2] = {'\r', '\n'};
                                	write (STDOUT_FILENO, tmp, 2);
					write (pipe_toshell[1], "\n", 1);
                        	}
                        	else if (buf3[i] == '\004')
                        	{
					pthread_cancel(tid);
                                     	close(pipe_toshell[0]);
                			close(pipe_fromshell[0]);
                			close(pipe_toshell[1]);
                			close(pipe_fromshell[1]);
                			kill (pid, SIGHUP);
                	                reset_to_saved();
                                        exit(0);
                        	}
				else if (buf3[i] == '\003') //^C is caught
				{	
					kill (pid, SIGINT);
				}
				else                        	
				{
                                	write (STDOUT_FILENO, buf3 + i, 1);
					write (pipe_toshell[1], buf3 + i, 1);
				}
                	}

		}
	}

	}
	exit(0);
}
