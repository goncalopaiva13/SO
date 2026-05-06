#include "defs.h"
#include <sys/types.h>


int main (int argc, char * argv[]){

	unlink(FIFO_CONTROLLER);
	if(mkfifo(FIFO_CONTROLLER, 0600) != 0){
		perror("fifo");
	}

	int max_paralelo = atoi(argv[1]);
	int em_execucao = 0;

	Comando fila_espera[100];
	int fila_size = 0;


	Comando em_exec[100];
	int em_exec_size = 0;


	int fifo_server = open(FIFO_CONTROLLER,O_RDONLY);
	int fd_dummy __attribute__((unused))= open(FIFO_CONTROLLER, O_WRONLY); // mantém o FIFO aberto atributte unused porque ele não é utilizado no resto do controler

	Msg msg;


	int shutdown = 0; // tem haver com a operacao == 's'
	int shutdown_pid = -1;

	
	while(read(fifo_server, &msg, sizeof(Msg))>0){


	printf("Recebi mensagem do runner%d\n",msg.runner_pid);
	// agr precisamos de outro fifo em q o server escreva la a respota(pq o server ta ai em cima como reader e so pode ser reader nesse fifo dai precisarmos de outro fifo para ele ser writer)
	
	
	
	
	if(msg.operacao == 's'){ //  pedir a terminação do programa controller
    	
    	shutdown = 1;
    	shutdown_pid = msg.runner_pid;

    	if(em_execucao == 0 && fila_size == 0){
        	break;
    	}

		// se ainda há runners a executar, não faz nada
		// o loop continua a receber 'f' dos runners
		// quando o último 'f' chegar e em_execucao ficar 0, o controller termina
	
	}
		
	
		




	
	else if(msg.operacao == 'e'){ //pedir para executar um comando


		if(em_execucao < max_paralelo){
			em_execucao++;
		

			//para calcular a duração no log e para o -c, utiliza-se a struct Comando

			
			Comando c;
			c.runner_pid = msg.runner_pid;
			strcpy(c.user_id, msg.user_id);
			strcpy(c.comando, msg.comando);
			gettimeofday(&c.inicio, NULL);
			em_exec[em_exec_size++] = c;


			//abre fifo de resposta ao cliente para o autorizar

			char fifo_client_name[64];
			sprintf(fifo_client_name, FIFO_RUNNER, msg.runner_pid);
			
			int fifo_client = open(fifo_client_name, O_WRONLY);
			
			Resposta r;
			r.autorizado = 1;
			r.tipo = 1;

			write(fifo_client, &r, sizeof(Resposta));
			close(fifo_client);
		} 

		else{ // em_execucao >= max_paralelo

			Comando c;
			c.runner_pid = msg.runner_pid;
			strcpy(c.user_id, msg.user_id);
			strcpy(c.comando, msg.comando);
			gettimeofday(&c.inicio, NULL);
			fila_espera[fila_size++] = c;
		}


	}
	


	else if(msg.operacao == 'c'){ // pedir para  consultar os comandos em execução
    	

		char resposta[4096] = "";
		// adiciona os em execução
		strcat(resposta, "---\nExecuting\n");

		for(int i = 0; i < em_exec_size; i++){
			char linha[512];
			sprintf(linha, "user-id %s - command %s\n", em_exec[i].user_id, em_exec[i].comando);
			strcat(resposta, linha);
		}
	
	
		// adiciona os em espera
		strcat(resposta, "---\nScheduled\n");
		for(int i = 0; i < fila_size; i++){
			char linha[512];
			sprintf(linha, "user-id %s - command %s\n", fila_espera[i].user_id, fila_espera[i].comando);
			strcat(resposta, linha);
		}
		
		
		// envia ao runner
		char fifo_client_name[64];
		sprintf(fifo_client_name, FIFO_RUNNER, msg.runner_pid);
		int fifo_client = open(fifo_client_name, O_WRONLY);
		write(fifo_client, resposta, strlen(resposta)+1);
		close(fifo_client);


		}	







	else if(msg.operacao == 'f'){ //avisar controller que terminou
			em_execucao--;
			


			//tratar do comando que terminou tem de ser fora do if(fila_size>0) ja que um comando pode terminar mesmo que haja ou não haja ninguém na fila de espera.
			
			
			// encontra o comando na lista em_exec pelo runner_pid
			int indice = -1;
			for(int i = 0; i < em_exec_size; i++){
				if(em_exec[i].runner_pid == msg.runner_pid){
					indice = i;
					break;
				}
			}
			
			if(indice != -1){
				// guarda o inicio para calcular duração e tambem as outras componentes antes de remover
				struct timeval inicio = em_exec[indice].inicio;
				char user_id[32];
				char comando[256];
				strcpy(user_id, em_exec[indice].user_id);
				strcpy(comando, em_exec[indice].comando);


				
				// remove o comando da lista em_exec
				for(int i = indice + 1; i < em_exec_size; i++){
					em_exec[i-1] = em_exec[i];
				}
				em_exec_size--;


				
				// calcula duração (em milissegundosssssss)
				struct timeval fim;
				gettimeofday(&fim, NULL);
				
				long duracao_ms = (fim.tv_sec - inicio.tv_sec) * 1000 + 
				(fim.tv_usec - inicio.tv_usec) / 1000;
				
				// escrever no ficheiro de log


				char log_linha[512];
				int length = sprintf(log_linha, "user:%s comando:%s duracao:%ld ms\n", 
                     user_id,   
                     comando,    
                     duracao_ms);


				int fd_log = open("log.txt", O_WRONLY | O_CREAT | O_APPEND, 0600);
				write(fd_log, log_linha, length);
				close(fd_log);

			}



			if(fila_size > 0){

				
				// retirar o primeiro runner da fila de espera
				
				Comando primeiro = fila_espera[0];
				int primeirorunner_pid = primeiro.runner_pid;

				for (int i = 1; i < fila_size; i++) {
					fila_espera[i - 1] = fila_espera[i];
				}


				// autoriza esse runner

				char fifo_client_name[64];
				sprintf(fifo_client_name, FIFO_RUNNER, primeirorunner_pid);
				
				int fifo_client = open(fifo_client_name, O_WRONLY);
				
				Resposta r;
				r.tipo = 1;
				r.autorizado = 1;

				write(fifo_client, &r, sizeof(Resposta));
				close(fifo_client);

				// adiciona o runner autorizado agora para a lista em_exec e aumenta a variavel em_execucao


				em_exec[em_exec_size++] = primeiro;

				em_execucao++;

				// Decrementa o fila_size

				fila_size--;
				
			}

			
			// se as condicoes em baixo estiverem estabelcidas , acaba o ciclo e o programa controller termina
			if(shutdown && em_execucao == 0 && fila_size == 0){
    			break;
			}


	}

	
}
// Envia resposta ao runner que pediu para o controler terminar
	if (shutdown_pid != -1) {
    	char fifo_client_name[64];
    	sprintf(fifo_client_name, FIFO_RUNNER, shutdown_pid);

    	int fifo_client = open(fifo_client_name, O_WRONLY);

    	Resposta r;
    	r.tipo = 2;          
    	r.autorizado = 0;    // não interessa aqui

    	write(fifo_client, &r, sizeof(Resposta));
    	close(fifo_client);
	}
	
	
	return 0;

}

