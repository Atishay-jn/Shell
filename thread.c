#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
// threads
char* remove_space(char *input) {
	int flag = 0;
	int k=0;
	while(input[k] == ' ')
		k++;
	int x = k;
	for(;input[x]!='\0';x++) {
		input[x-k] = input[x];
	}
	input[x-k] = '\0';
	do {
		flag = 0;
		int j=0;
		for(int i=0;input[i]!='\0';i++,j++) {
			input[j] = input[i];
			if(input[i] == ' ' && (input[i+1] == ' ' || input[i+1] == '\0')) {
				j--;
				flag = 1;
			}
		}
		input[j] = '\0';
	}
	while(flag == 1);
	return input;
}

int read_command() {
	char *input = malloc(sizeof(char) *500);
	gets(input);
	input = remove_space(input);
	int counter = 1, fd[2];
	char *trueinput = malloc(sizeof(char) *500);
	for(int i=0;input[i]!='\0';i++) {
		if(input[i] == '|')
			counter++;
	}
	int trueidx = 0;
	if(trueidx == 0) {
		int i=0;
		for(;input[i]!='\0';i++)
			trueinput[i] = input[i];
		trueinput[i] = '\0';
	}
	if(trueinput[0] == 'e' && trueinput[1]!='\0' && trueinput[1] == 'x' && trueinput[2]!='\0' && trueinput[2] == 'i' && trueinput[3]!='\0' && trueinput[3] == 't') {
		return 0;
	}
	else if(trueinput[0] == 'c' && trueinput[1]!='\0' && trueinput[1] == 'd' && trueinput[2]!='\0' && trueinput[2] == ' ' && counter == 1) {
		char cd[100];
		int i=3;
		for(;trueinput[i]!='\0';i++)
			cd[i-3] = trueinput[i];
		cd[i-3] = '\0';
		if(chdir(cd)!=0) {
			printf("\033[0;31m");
			printf("Failed\n");
			printf("\033[0m");
		}
		return 1;
	}
	else if(trueinput[0]=='\0')
		return 1;
	int process = fork();
	if(process != 0) {
		wait(NULL);
		return 1;
	}
	if(counter>1) {
		for(;input[trueidx]!='|';trueidx++)
			input[trueidx] = trueinput[trueidx];
		input[trueidx-1] = '\0';
		trueidx++;
	}
	pipe(fd);
	while(counter-->1) {
		pipe(fd);
		process = fork();
		if(process == 0) {
			if(counter!=0) {
				close(fd[0]);
				close(1);
				dup(fd[1]);
				close(fd[1]);
			}
			break;
		}
		else {
			close(fd[1]);
			close(0);
			dup(fd[0]);
			close(fd[0]);
			int i = 0;
			trueidx++;
			for(;trueinput[trueidx]!='|' && trueinput[trueidx]!='\0';trueidx++,i++)
				input[i] = trueinput[trueidx];
			trueidx++;
			input[i] = '\0';
			if(input[i-1] == ' ')
				input[i-1] = '\0';
		}
	}
	char *cmd;
	char **arg;
	cmd = malloc(sizeof(char) * 500);
	arg = malloc(sizeof(char *) * 128);
	arg[0] = malloc(sizeof(char) * 500);
	for(int i=1;i<128;i++)
		arg[i] = NULL;
	int j = 0, k = 0;
	for(int i=0;input[i]!='\0';i++,k++) {
		if(input[i] == ' ') {
			arg[j][k] = '\0';
			k = -1;
			j++;
			arg[j] = malloc(sizeof(char) * 500);
		}
		else
			arg[j][k] = input[i];
	}
	arg[j][k] = '\0';
	int cnt = 0;
	arg[j+1] = NULL;
	if(arg[0][0] == 'e' && arg[0][1] == 'x' && arg[0][2] == 'i' && arg[0][3] == 't')
		return 0;
	if(arg[0][0] == 'c' && arg[0][1]!='\0' && arg[0][1] == 'd' && arg[0][2]=='\0')
		exit(0);
	int flag = 0;
	int index = -1;
	for(int i=0;i<=j;i++) {
		if(j>1 && strcmp(arg[i],"<")==0) {
			close(0);
			fopen(arg[i+1],"r");
			for(int x=i+2;x<=j;x++)
				arg[x-2] = arg[x];
			arg[j-1] = NULL;
			j-=2;
		}
	}
	for(int i=0;i<=j;i++)
		if(strcmp(arg[i],">")==0) {
			flag = 1;
			arg[i] = NULL;
			index = i+1;
			break;
		}
		else if(strcmp(arg[i],">>")==0) {
			flag = 2;
			arg[i] = NULL;
			index = i+1;
			break;
		}
		else if(arg[i][0]=='1' && arg[i][1]!='\0' && arg[i][1]=='>' && arg[i][2]!='\0' && arg[i][2] != '&') {
			char *temp = malloc(sizeof(char) * 100);
			for(int x=2;arg[i][x]!='\0';x++)
				temp[x-2] = arg[i][x];
			close(1);
			FILE *file = fopen(temp, "w");
			for(int x=i;x<j;x++)
				arg[x] = arg[x+1];
			arg[j] = NULL;
			j--;
			i--;
		}
		else if(arg[i][0]=='2' && arg[i][1]!='\0' && arg[i][1]=='>' && arg[i][2]!='\0' && arg[i][2] != '&') {
			char *temp = malloc(sizeof(char) * 100);
			for(int x=2;arg[i][x]!='\0';x++)
				temp[x-2] = arg[i][x];
			close(2);
			FILE *file = fopen(temp, "w");
			for(int x=i;x<j;x++)
				arg[x] = arg[x+1];
			arg[j] = NULL;
			j--;
			i--;
		}
		else if(strcmp(arg[i],"2>&1")==0) {
			close(2);
			dup(1);
			for(int x=i;x<j;x++)
				arg[x] = arg[x+1];
			arg[j] = NULL;
			j--;
			i--;
		}
		else if(strcmp(arg[i],"1>&2")==0) {
			close(1);
			dup(2);
			for(int x=i;x<j;x++)
				arg[x] = arg[x+1];
			arg[j] = NULL;
			j--;
			i--;
		}
		else
			continue;
	if(flag) {
		int tem = index-1;
		for(;tem+2<=j;tem++)
			arg[tem] = arg[tem+2];
		arg[j-1] = NULL;
		j-=2;
		FILE *file;
		close(1);
		if(flag == 1)
			file = fopen(arg[index], "w");
		else
			file = fopen(arg[index], "a");
	}
	execvp(arg[0],arg);
	printf("\033[0;31m");
	printf("Command not found / Invalid command\n");
	printf("\033[0m");
	exit(0);
}

int main() {
	int flag = 1;
	char pwd[100];
	while(flag) {
		getcwd(pwd,sizeof(pwd));
		printf("\033[33;1mAtishay %s ",pwd);
		printf("$ \033[0m");
		flag = read_command();
	}
}
