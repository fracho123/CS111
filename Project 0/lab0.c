#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    	char *buf = NULL;
    	char *temp = NULL;
    	size_t size = 0, index = 1;
    	int ch = EOF;
	
	//Signal Handler
	void sig_handler(int sigNum)
        {
                if (sigNum == SIGSEGV)
                {
                        write(2, "Error: Segmentation Fault Caught\n", 33);
                        _exit(3);
                }
        }

	/* --input and --output implementation*/
    	char *input = NULL;
	char *output = NULL;
    	int optVar; 
	char *ptr = NULL; //segfault pointer
	
	//option indicator variables
	int segFlag = 0;
	int inFlag = 0;
	int outFlag = 0;	
    	while (1) {
        	int this_option_optind = optind ? optind : 1;
        	int option_index = 0;
        	static struct option long_opts[] = {
        	    {"input",  required_argument, 0,  'i' },
        	    {"output", required_argument, 0,  'o' },
		    {"segfault", no_argument, 0, 's' },
		    {"catch", no_argument, 0, 'c' },	
		    {0,0,0,0}
        	};
	
	       optVar = getopt_long(argc, argv, "i:o:s:",
	                 long_opts, &option_index);
	        if (optVar == -1)
	            break;

	       switch (optVar){
	      	case 'i': //input option used
			input = optarg;
			inFlag = 1;
			break;
	 	case 'o': //output option used
			output = optarg;
			outFlag = 1;
			break;
		case 'c': //catch option used
                        signal(SIGSEGV, sig_handler);
                        break;
		case 's': //segfault option used
			
			segFlag = 1; 
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
		
	//SEG FAULT FORCE
	if (segFlag)
		*ptr = 'a'; //write to NULL pointer
	
	//input redirection, fd 0 will be the newly opened file
	if (inFlag)
	{
		int ifd = open(input, O_RDONLY);
  		
		//check if file was opened correctly	
       		if (ifd >= 0){
			close(0);
			dup(ifd);
			close(ifd);
		}
		else
		{
			perror("Error");
			_exit(1);
		}
	} 
	
	//output redirection, fd 1 will now be the newly created file
	if (outFlag)
	{
		int ofd =  creat(output, 0666);
		
		//check if file was created correctly	
		if (ofd >= 0){
			close(1);
			dup(ofd);
			close(ofd);
		}
		else
		{
			perror("Error");
			_exit(2);
		}
	}
	
  	//main body to read and write
    	while (ch) 
	{
               	//Check for more memory needed
        	if (size <= index) 
		{
            		size += 4096; //works with 8
            		temp = realloc(buf, size);
            		if (!temp) 
			{
                		free(buf);
                		buf = NULL;
                		break;
            		}
            		buf = temp;
        	}
		
		int check = read(0, buf, 1);	
		if (!check)
			break;
		ch = buf[0];

        	// Store character into buffer
        	buf[index] = ch;
		index++;
    	}

	//realign buffer
	int i = 0 ;
	for (i = 0; i < index-1; i++)
		buf[i] = buf[i+1];


	//write and exit
	write(1, buf, index-1);
	
	free(buf);
	_exit(0); 
}

