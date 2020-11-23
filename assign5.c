#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/*
This is a modification of prior shell program to add the
ability to redirect and pipe.
Use < and > to redirect and | to pipe

A hashtable has been added to store set variables and
aliases as an improvement over the simple array from the
last iteration.
*/

// Type represents both an myalias and myset command
struct aliset{
	char first[50]; // the variable or shortcut
	char second[50]; // the value or long command version
};

struct hashentry{
	struct aliset a;
	struct hashentry * next;
};

unsigned int hash(char * s){
	int i;
	int h = 12361;
	while((i = *s++) != 0){
		h = h * 691 + i;
	}
	return h;
}

struct aliset make_aliset(const char * s1, const char * s2){
	struct aliset A;
	strcpy(A.first, s1);
	strcpy(A.second, s2);
	return A;
}

struct hashentry * make_hashentry(struct aliset A){
	struct hashentry * H = (struct hashentry *)malloc(sizeof(struct hashentry));
	strcpy(H->a.first, A.first);
	strcpy(H->a.second, A.second);
	H->next = NULL;
	return H;
}

void insert(struct hashentry * ht[], struct aliset A, int size){
	struct hashentry * H = make_hashentry(A);
	int pos = hash(A.first) % size;
	printf("position of %s is %d \n", A.first, pos);
	H->next = ht[pos];
	ht[pos] = H;
}

struct hashentry * find2(struct hashentry * ht[], char * c, int size){
	int pos = hash(c) % size;
	struct hashentry * H = ht[pos];
	while(H != NULL){
		if(strcmp(H->a.first, c) == 0){
			return H;
		}
		H = H -> next;
	}
	return NULL;
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

	struct hashentry * aliv_ht[sz];
	struct hashentry * setv_ht[sz];

	// Hash function tests:
	unsigned int x = hash("cat");
	printf("Hash of 'cat' is %u \n", x);
	unsigned int y = hash("tac");
	printf("Hash of 'tac' is %u \n", y);
	unsigned int z = hash("longtest");
	printf("Hash of 'longtest' is %u \n", z);
	unsigned int w = hash("cat");
	printf("Hash of 'cat' is %u \n", w);

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
			struct hashentry * H = find2(aliv_ht, argv[i], sz);
			if(H != NULL){
				argv[i] = H -> a.second;
			}

		
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
				struct aliset new_alias = make_aliset(argv[i], argv[i+1]);
				//printf("new_alias : first = %s \n", new_alias.first);
				//printf("new_alias : second = %s \n", new_alias.second);

				insert(aliv_ht, new_alias, sz);

				//strcpy(aliv[av_index].first, argv[i]);
				//strcpy(aliv[av_index].second, argv[i+1]);
				//av_index += 1;
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
