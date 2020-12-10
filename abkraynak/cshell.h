#ifndef CSHELL
#define CSHELL

// Type represents both an myalias and myset command
struct aliset{
	char first[50]; // the variable or shortcut
	char second[50]; // the value or long command version
};

int find(struct aliset v[], char * c);

char * substring(char * src, int m, int n);

void reset_arr(char * arr[], int sz);

void new_cmd_text();

#endif
