#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/*
This is a modification of prior shell program to add the
ability to redirect and pipe.
Use < and > to redirect and | to pipe
*/

// Type represents both an myalias and myset command
struct aliset{
	char first[50]; // the variable or shortcut
	char second[50]; // the value or long command version
};

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

void new_cmd_text(){
	printf("\n*** New Commands ***\n");
	printf("Use \"myset [var] = [value]\" to set variables\n");
	printf("  Call variables with \"@var\" to use with other commands\n");
	printf("Use \"myalias [shortcut] [longcommand]\" to make shortcuts\n\n");
}

int main(){
	int argc;
	int sz = 1000;

	char * argv[sz];
	char line[sz];

	struct aliset aliv[sz];
	struct aliset setv[sz];

	int av_index = 0; // count of aliases stored
	int sv_index = 0; // count of variables set

	new_cmd_text();

	while(1){
		int s;
		printf("~>>~ "); // print prompt

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
