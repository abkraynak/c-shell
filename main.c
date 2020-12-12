#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "abkraynak/cshell.h"

/*
This is a modification of prior shell program to add the
ability to redirect and pipe.
Use < and > to redirect and | to pipe
*/

int main(){
	int argc;
	int sz = 1000;
	
	int redir_out = 0;
	int redir_in = 0;
	int pipes = 0;
	int fd;
	int link[2];

	char * argv[sz];
	char line[sz];

	struct aliset aliv[sz];
	struct aliset setv[sz];

	int av_index = 0; // count of aliases stored
	int sv_index = 0; // count of variables set

	char output_file[100];
	char input_file[100];
	char * prog1[sz];
	char * prog2[sz];

	// Save value of stdin stdout to use after redirect
	int orig_stdin = dup(0);
	int orig_stdout = dup(1);

	new_cmd_text();

	while(1){
		int s;
		printf("> "); // prompt
	
		// Reset all to 0 for each new command entered
		redir_out = 0;
		redir_in = 0;
		pipes = 0;
		reset_arr(prog1, sz);
		reset_arr(prog2, sz);		

		if(!fgets(line, 1000, stdin)){
			return 0;
		}
		if(line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';

		// exit command
		if(strcmp(line, "exit") == 0){
			exit(0);
		}

		char * tok;
		tok = strtok(line, " ");
		int i = 0;
		while(tok != NULL && i < 1000){
			argv[i] = tok;
			tok = strtok(NULL, " ");
			i += 1;
		}
		argc = i;
		argv[i] = NULL;

		// Find and replace any aliases on the command line with their values
		for(i = 0; i < argc; i += 1){		
			int r = find(aliv, argv[i]);
			if(r >= 0){ // if the argument was an alias
				argv[i] = aliv[r].second;
			}
		}		

		// Find and replace any variables on the command line with their values
		for(i = 0; i < argc; i += 1){
			if(argv[i][0] == '@'){ // if an argument was a myset variable
				int l = strlen(argv[i]);
				char * search = substring(argv[i], 1, l - 1);
				int r = find(setv, search);
				if(r >= 0){
					argv[i] = setv[r].second;
				}
				else{
					fprintf(stderr, "error: myset variable \"%s\" not declared\n", search);
				}
			}
		}

		// If > is present, redirect stdout and adjust the command line
		for(i = 0; i < argc; i += 1){
			if(strcmp(argv[i], ">") == 0){
				redir_out = 1;
				strcpy(output_file, argv[i+1]);
				// Remove > from command line and shift rest down
				int j;
				for(j = i; j < argc-2; j += 1){
					argv[j] = argv[j+2];
				}
				// Removes the "duplicates" left in the last 2 positions of the array
				argv[argc-2] = NULL;
				argv[argc-1] = NULL;
				argc -= 2;
			}
		}

		// If < is present, redirect stdin and adjust the command line
		for(i = 0; i < argc; i += 1){
			if(strcmp(argv[i], "<") == 0){
				redir_in = 1;
				strcpy(input_file, argv[i+1]);
				// Remove < from command line and shift rest down
				int j;
				for(j = i; j < argc-2; j += 1){
					argv[j] = argv[j+2];
				}
				// Removes the "duplicates" left in the last 2 positions of the array
				argv[argc-2] = NULL;
				argv[argc-1] = NULL;
				argc -= 2;
			}
		}

		// If | is present, redirect stdout and stdin and adjust the command line
		for(i = 0; i < argc; i += 1){
			if(strcmp(argv[i], "|") == 0){
				int j, k;
				pipes = 1;
				// Loop to build prog1 array to exec
				for(j = 0; j < i; j += 1){
					prog1[j] = argv[j];
				}
		

				// Loop to build prog2 array to exec
				for(j = i + 1, k = 0; j < argc; j += 1, k += 1){
					//strcpy(prog2[k], argv[j]);
					prog2[k] = argv[j];
				}
			}
		}

		if(strcmp(argv[0], "myalias") == 0){ // myalias command
			if((argc < 3) || (argc % 2 != 1)){
				fprintf(stderr, "usage: myalias [shortcut] [longcommand]\n");
				continue;
			}
			int i;
			for(i = 1; i < argc; i += 2){
				strcpy(aliv[av_index].first, argv[i]);
				strcpy(aliv[av_index].second, argv[i+1]);
				av_index += 1;
			}
			continue;
		}

		if(strcmp(argv[0], "myset") == 0){ // myset command
			if((argc < 4) || (argc % 3 != 1)){
				fprintf(stderr, "usage: myset [variable] = [value]\n");
				continue;
			}
			int i;
			for(i = 1; i < argc; i += 3){
				strcpy(setv[sv_index].first, argv[i]);
				strcpy(setv[sv_index].second, argv[i+2]);
				sv_index += 1;
			}
			continue;	
		}

		int p = fork();
		if(p == 0){ // child process running
			if(redir_out == 1){
				close(1);
				if((fd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0600)) == -1){
					fprintf(stderr, "could not open output file %s \n", output_file);
					continue;
				}
			}
			if(redir_in == 1){
				close(0);
				if((fd = open(input_file, O_RDONLY)) == -1){
					fprintf(stderr, "could not open input file %s \n", input_file);
					continue;
				}
			}
			if(pipes == 1){
				int p1, p2;
				pipe(link);
				p1 = fork();
				if(p1 == 0){
					close(1);
					dup(link[1]);
					execvp(prog1[0], prog1);
					fprintf(stderr, "Cannot execlp p1\n");
					exit(1);
				}
				close(link[1]);
				p2 = fork();
				if(p2 == 0){
					close(0);
					dup(link[0]);
					execvp(prog2[0], prog2);
					fprintf(stderr, "Cannot execlp p2\n");
					exit(1);					
				}
				p2 = wait(& s);
				if(WIFEXITED(s)){
					if(WEXITSTATUS(s) != 0){
						printf("Exit code %d\n", WEXITSTATUS(s));
					}
				}
				p1 = wait(& s);
				if(WIFEXITED(s)){
					if(WEXITSTATUS(s) != 0){
						printf("Exit code %d\n", WEXITSTATUS(s));
					}
				}
				exit(0);
				exit(0);
				continue;
			}
			execvp(argv[0], argv);
			printf("Could not execvp\n");
			exit(1);
		}

		p = wait(& s);
		if(WIFEXITED(s)){
			if(WEXITSTATUS(s) != 0){
				printf("Exit code %d\n", WEXITSTATUS(s));
			}
		}
	}
}

int find(struct aliset v[], char * c){
	// return the index of aliv[] if found, -1 if not
	int i;
	for(i = 0; i < 1000; i += 1){
		if(strcmp(v[i].first, c) == 0){
			return i;
		}
	}
	return -1;
}

char * substring(char * src, int m, int n){
	int len = n - m + 1;
	char * dst = (char*)malloc(sizeof(char) * (len));
	strncpy(dst, (src + m), len);
	return dst;
}

void reset_arr(char * arr[], int sz){
	int i;
	for(i = 0; i < sz; i += 1){
		arr[i] = NULL;
	}
}

void new_cmd_text(){
	printf("\n*** New Commands ***\n");
	printf("Use < and > to redirect inputs and outputs to different programs and files\n");
	printf("Use | to pipe the output to one program to be the input to another\n\n");
}
