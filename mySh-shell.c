//
// MySh
// Autores: Lucas Baldin
// Linguagem: C
// Versão: 2.1
// Data: 05/2018
// Funcionalidades do programa:
// . Exibir um prompt e aguarda entrada do usuário 
// . O comando "exit" sai do processador de comandos 
// . O comando "clear" limpa a tela do processador de comandos
// . Quando a entrada do usuário é o nome de um programa, o mesmo é executado (ex: ./teste) 
// . Quando a entrada do usuário não é um programa ou comando válido, é mostrada uma mensagem de erro 
// . Os parâmetros digitados na linha de comando são passados ao programa que será executado 
// . O prompt é apresentado  no formato: [MySh] nome-de-usuario@hospedeiro:diretorioatual$ 
// . No prompt, o caminho do diretório home do usuário é substituído por ~, caso este faça parte do caminho atual. Exemplo: de /home/juquinha/aula para ~/aula. 
// . O comando cd muda para diretórios (quando recebe parâmetro) ou volta ao diretório home do usuário (sem parâmetros ou com o parâmetro ~) 
// . O programa mostra uma mensagem de erro adequada se cd falhar 
// . Ctrl+Z e Ctrl+C não causam a parada ou interrupção do shell (apenas ignora a linha com o comando)
// . Ctrl+D  sai do processador de comandos
//

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h> 
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <unistd.h>

#define sizeMax 100

int execptionProgram(char *Program, char **arg_list);
void handler(int signal_number);
void promptMySh();
char* str_replace(char* search, char* replace, char* subject);


char Buffer[sizeMax + 1];
char Command[100];
char *cwd;	
char Hostname[1024];
struct passwd *pwd;

void handler(int signal_number) {
	printf("\n");
}


char* str_replace(char* search, char* replace, char* subject) {
	int i, j, k;
	
	int searchSize = strlen(search);
	int replaceSize = strlen(replace);
	int size = strlen(subject);

	char* ret;

	if (!searchSize) {
		ret = malloc(size + 1);
		for (i = 0; i <= size; i++) {
			ret[i] = subject[i];
		}
		return ret;
	}
	
	int retAllocSize = (strlen(subject) + 1) * 2; 
	ret = malloc(retAllocSize);

	int bufferSize = 0; 
	char* foundBuffer = malloc(searchSize); 
	
	for (i = 0, j = 0; i <= size; i++) {
		
		if (retAllocSize <= j + replaceSize) {
			retAllocSize *= 2;
			ret = (char*) realloc(ret, retAllocSize);
		}
		
		else if (subject[i] == search[bufferSize]) {
			foundBuffer[bufferSize] = subject[i];
			bufferSize++;

			
			if (bufferSize == searchSize) {
				bufferSize = 0;
				for (k = 0; k < replaceSize; k++) {
					ret[j++] = replace[k];
				}
			}
		}
		
		else {
			for (k = 0; k < bufferSize; k++) {
				ret[j++] = foundBuffer[k];
			}

			bufferSize = 0;
			ret[j++] = subject[i];
		}
	}

	free(foundBuffer);
	return ret;
}


int execptionProgram(char *Program, char **arg_list) {
        pid_t child_pid;
        child_pid = fork ();
 
        if (child_pid != 0)
                return child_pid;
        else {
        				//Executar o programa
                execvp(Program, arg_list); 
                printf("-bash: %s: Command not found\n",Program);
                abort(); 
        }
}


void promptMySh() {
	Hostname[100] = '\0';
	gethostname(Hostname, 100);

	cwd = getcwd(Buffer, sizeMax + 1); 
	pwd = getpwuid(getuid()); 
	cwd = str_replace(pwd->pw_dir, "~", cwd); 

	printf("[MySh]%s@%s:%s$: ",pwd->pw_name, Hostname, cwd);
}

int main() {
	int child_status;
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &handler;

	//Tratamento Crtl+C 
	sigaction (SIGINT, &sa, NULL);
	//Tratamento Crtl+Z 
	sigaction (SIGTSTP, &sa, NULL);

	printf("\n");
	printf("Starting [MySh]...\n");
	printf("\n");

	while(1) {

		int i = 0; 
		char *arg_list[20], *program_name, *argument;		

		//Tratamento Ctrl+D
		if (feof(stdin)) {
				printf("\n");
				exit(0);
		}
		
		promptMySh();
		memset(&Command, 0x00, sizeof(Command)); 
		fgets(Command, 100, stdin); 

	
		if (strncmp(Command, "exit", 4) == 0) {
			printf("\n\nExiting [MySh]...\n\n");
			return 0;
		} 

		if (strncmp(Command, "clear", 1) == 0) {
			printf("\e[H\e[2J");		
 		} 

		
		if (strlen(Command) > 1) {
			argument = strtok(Command, " "); 
			program_name = arg_list[i] = argument; 
			i++; 
			while(argument != NULL){ 
				argument = strtok(NULL, " ");
				arg_list[i] = argument; 
				i++;
			}

			arg_list[i-2][strlen(arg_list[i-2]) - 1] = '\0';
			if (strcmp(program_name, "cd") == 0) {
				if (arg_list[1] == NULL || arg_list[1][0] == '~') 
					chdir(pwd->pw_dir); 
				else if (chdir(arg_list[1]))
					printf("-bash: cd: %s: No such file or directory\n",arg_list[1]);
			} else {
				//Execute the program
				execptionProgram(program_name, arg_list);
				wait(&child_status);
			}
		}
		else {
		 	//Execute pipe { 
		 	int fds[2];
			pid_t pid;
			pipe (fds);

			pid = fork ();
			if (pid == (pid_t) 0) {
				FILE* stream;
				close (fds[1]);
				stream = fdopen (fds[0], "r");
				close (fds[0]);
			}
			else {
				FILE* stream;
				close (fds[0]);
				stream = fdopen (fds[1], "w");
				close (fds[1]);
			}
		}
	}
}
