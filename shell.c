#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<fcntl.h>

#define MAXSIZE 1000

int create_arg(char [], char *[], int );
void call_exec(char []);
void set_lpipe(int * );
void set_rpipe(int * );
pid_t fork_and_chain(int *, int , int *, int , char *[]);
int get_line(char [], int );
void command_preprocessor(char []);

int main()
{	
	char command[MAXSIZE];
	while(1) {
		
		printf("linux ~ $ ");
		get_line(command, MAXSIZE);
		command_preprocessor(command);
		call_exec(command);
		
	}
	return 0;
}

void call_exec(char command[])
{
	int k = 0, pcount=0;
	int fd;
	int return_status;
	int lpipe[] = {0,1}, rpipe[] = {0,1};
	int lset = 0, rset = 0;
	char *argv[MAXSIZE], *argv2[MAXSIZE];
	pid_t childpid;
	
	while(1) {
		
		k = create_arg(command, argv,k);
		if(command[k-1] == '\0') {
			
			childpid = fork_and_chain(lpipe,lset,rpipe,rset, argv);
			if(lset) {
				close(lpipe[0]);
				close(lpipe[1]);
			}
			waitpid(childpid,&return_status,0);		
			return;
		}
		else if(command[k-1] == '>') {
			k = create_arg(command,argv2,k);
			fd = open(argv2[0], O_WRONLY|O_CREAT|O_TRUNC, 0644);
			rpipe[1] = fd;
			rset = 1;	
			childpid = fork_and_chain(lpipe,lset,rpipe,rset, argv);
			if(lset) {
				close(lpipe[0]);
				close(lpipe[1]);
			}
			rset = 0;
		}
		else if(command[k-1] == '<') {
			
			k = create_arg(command,argv2,k);
			fd = open(argv2[0], O_RDONLY, 0644);
			lpipe[0] = fd;
			lpipe[1] = fd;
			lset = 1;
			if(command[k-1] == '|') {
				pcount++;
				pipe(rpipe);
				rset = 1;
			}
			childpid = fork_and_chain(lpipe,lset,rpipe,rset, argv);
			if(pcount) {
				close(lpipe[0]); 
	   			close(lpipe[1]);
	   		}
			if(rset) {
				lpipe[0] = rpipe[0];
				lpipe[1] = rpipe[1];
	   			rset = 0;
	   		}
		}
		else if(command[k-1] == '|') {
			pipe(rpipe);
			rset = 1;
			childpid = fork_and_chain(lpipe,lset,rpipe,rset,argv);
			if(pcount) {
	    			close(lpipe[0]); 
	   			close(lpipe[1]);	
	    			
			}
			pcount++;
			lpipe[0] = rpipe[0]; 
	    		lpipe[1] = rpipe[1];
	    		lset  = 1;
			rset = 0;	
		}
		if(command[k-1] == '\0')
			return;
		
		
	}
}
	 
void set_lpipe(int * lpipe)
{	
	dup2(lpipe[0], STDIN_FILENO);
	close(lpipe[0]);
	close(lpipe[1]);
}

void set_rpipe(int *rpipe)
{
	dup2(rpipe[1], STDOUT_FILENO);
	close(rpipe[0]);
	close(rpipe[1]);
}

pid_t fork_and_chain(int *lpipe, int lset, int *rpipe, int rset, char *argv[])
{	
	pid_t childpid;
	if(!(childpid = fork())) {
		
		if(lset)
			set_lpipe(lpipe);
		if(rset)
			set_rpipe(rpipe);
		execvp(argv[0],argv);
	}
	return childpid;
}
int create_arg(char command[], char *argv[], int start)
{
	int i=start, j = 0, k = 0;
	char temp[100];
	char *arg;
	while(command[i] == ' ' ||command[i] == '\t') {
            		i++;
        }
	for(;command[i] != '\0' && command[i] != '|' && command[i] != '>' && command[i] != '<';i++) {
	
		if(command[i] == ' ') {
			temp[j] = '\0';
			arg = (char *)malloc(j*sizeof(char));
			strcpy(arg, temp);
			argv[k++] = arg;
			j = 0;
		}
		else if(command[i] == '\\'){
			temp[j++] = command[i+1];
			i++;
		}
		else{
			temp[j++] = command[i];
		}
		
		
	}
	if(j != 0) {
		temp[j] = '\0';
		arg = (char *)malloc(j*sizeof(char));
		strcpy(arg, temp);
		argv[k++] = arg;
	}
	argv[k] = NULL;
	return i+1;
}

int get_line(char s[], int lim)
{
    int c, i;

    for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; ++i)
        s[i] = c;
    s[i] = '\0';

    return i;
}

void command_preprocessor(char command[])
{
	int i = 0,j=0, len = strlen(command);
	int nb=0;
	char temp[MAXSIZE];
    	while(command[i] == ' ' ||command[i] == '\t') {
            		i++;
        }
        while(command[i]!= '\0' && i<=len) {
        	while(command[i] == ' ' ||command[i] == '\t') {
            		i++;
            		nb=1;
        	}
        	if(nb>0) {
        		if(command[i] != '\0') {
            			temp[j++] = ' ';
            			nb=0;
            		}
        	}
        	temp[j++] = command[i];
        	i++;
    	}
    	temp[j] = '\0';
    	strcpy(command,temp);
}				
