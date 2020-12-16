#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include "longest_word_search.h"
#include "queue_ids.h"

//Global Variables
char **prefixArr; // Stores all prefixes
sem_t *statusTracker; //Semaphore Array to Track Progress
int countOfPrefix; 
int passageCount;

void sig_handler(int signo)
{
  signal(SIGINT, sig_handler); 
  if (signo == SIGINT){
      int *compare = (int*) malloc(sizeof(int));
      for(int i = 0; i < countOfPrefix; i++){
            printf(" %s - ", prefixArr[i]);
            sem_getvalue(&statusTracker[i], compare);
            
            if(*compare == 0){
                printf("pending\n");
            } else if(*compare == passageCount){
                printf("done\n");
            } else {
                printf("%d of %d\n",*compare,passageCount);
            }
        }
    }
}

size_t strlcpy(char *dst, const char *src, size_t size){
    size_t srclen;
    size--;
    srclen = strlen(src);
    if(srclen > size)
        srclen = size;

    memcpy(dst, src, srclen);
    dst[srclen] = '\0';

    return (srclen);
}

int main(int argc, char**argv){

    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    prefix_buf sbuf;
    response_buf rbuf;
    size_t buf_length;

    if(argc <= 1){
        printf("Error: please provide prefix of at least two characters for search\n");
        printf("Usage: %s <prefix>\n", argv[0]);
        exit(-1);
    }

    countOfPrefix = 0;
    for(int  i = 2; i < argc; i++){
        if(strlen(argv[i]) < 3 || strlen(argv[i]) > 20){
            printf("Error: please provide prefix of at least three characters for search\n");
            ("Usage: %s <prefix>\n", argv[0]);
            argv[i] = "";
            countOfPrefix --;
        } 
        countOfPrefix++;
    }

    prefixArr = (char **)malloc(countOfPrefix * sizeof(char *)); 
    for(int i = 0; i < countOfPrefix; i++){
        prefixArr[i] = (char *)malloc(20 * sizeof(char));
    }
    statusTracker = (sem_t*)malloc(countOfPrefix *  sizeof(sem_t)); 

    for(int i = 2; i < argc; i++){
        sem_init(&statusTracker[i-2], 0, 0);
        if(argv[i] == ""){
            while(argv[i+1] != ""){
                i++;
            }
            prefixArr[i - 2] = argv[i+1];
            i++;
        } else {
            prefixArr[i - 2] = argv[i];
        }
    }
    
    key = ftok(CRIMSON_ID, QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0) {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
    }
    else {
        for(int i = 0; i < countOfPrefix; i++){     
            sbuf.mtype = 1;
            strlcpy(sbuf.prefix, prefixArr[i], WORD_LENGTH);
            sbuf.id = i+1;
            buf_length = strlen(sbuf.prefix) + sizeof(int) + 1;

            if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
                int errnum = errno;
                fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
                perror("(msgsnd)");
                fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
                exit(1);
            }
            else
                fprintf(stderr,"Message(%d): \"%s\" Sent (%d bytes)\n", sbuf.id, sbuf.prefix,(int)buf_length);

            sleep(atoi(argv[1]));       

            int responseCount = 0;
            key = ftok(CRIMSON_ID,QUEUE_NUMBER);
            if ((msqid = msgget(key, msgflg)) < 0) {
                int errnum = errno;
                fprintf(stderr, "Value of errno: %d\n", errno);
                perror("(msgget)");
                fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
            }

            int ret;
            do {
                ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
                int errnum = errno;
                if (ret < 0 && errno !=EINTR){
                    fprintf(stderr, "Value of errno: %d\n", errno);
                    perror("Error printed by perror");
                    fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
                }
            } while ((ret < 0 ) && (errno == 4));
            
            passageCount = rbuf.count;
            response_buf resultsArr[passageCount] ;
            sem_post(&statusTracker[i]);
            resultsArr[rbuf.index] = rbuf;
            responseCount++;    
            
            while(responseCount != passageCount){
                key = ftok(CRIMSON_ID,QUEUE_NUMBER);
                if ((msqid = msgget(key, msgflg)) < 0) {
                    int errnum = errno;
                    fprintf(stderr, "Value of errno: %d\n", errno);
                    perror("(msgget)");
                    fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
                }

                int ret;
                do {
                    ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
                    int errnum = errno;
                    if (ret < 0 && errno !=EINTR){
                        fprintf(stderr, "Value of errno: %d\n", errno);
                        perror("Error printed by perror");
                        fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
                    }
                } while ((ret < 0 ) && (errno == 4));
                    
                sem_post(&statusTracker[i]);
                resultsArr[rbuf.index] = rbuf;     
                responseCount++;
            }

            for(int j = 0; j < passageCount; j++){
                if(resultsArr[j].present == 1){
                    printf("Passage %d - %s - %s\n", j, resultsArr[j].location_description, resultsArr[j].longest_word);
                }
                else
                    printf("Passage %d - %s - no word found\n",j, resultsArr[j].location_description);      
            }
          
        }

        sbuf.mtype = 1;
        strlcpy(sbuf.prefix, "   ", WORD_LENGTH);
        sbuf.id = 0;
        buf_length = strlen(sbuf.prefix) + sizeof(int) + 1;
        if((msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT)) < 0) {
            int errnum = errno;
            fprintf(stderr,"%d, %ld, %s, %d\n", msqid, sbuf.mtype, sbuf.prefix, (int)buf_length);
            perror("(msgsnd)");
            fprintf(stderr, "Error sending msg: %s\n", strerror( errnum ));
            exit(1);
        }
        else
            fprintf(stderr,"Message(%d): \"%s\" Sent (%d bytes)\n", sbuf.id, sbuf.prefix,(int)buf_length);
        
        key = ftok(CRIMSON_ID,QUEUE_NUMBER);
        if ((msqid = msgget(key, msgflg)) < 0) {
            int errnum = errno;
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("(msgget)");
            fprintf(stderr, "Error msgget: %s\n", strerror( errnum ));
        }

        // msgrcv to receive message
        int ret;
        do {
            ret = msgrcv(msqid, &rbuf, sizeof(response_buf), 2, 0);//receive type 2 message
            int errnum = errno;
            if (ret < 0 && errno !=EINTR){
                fprintf(stderr, "Value of errno: %d\n", errno);
                perror("Error printed by perror");
                fprintf(stderr, "Error receiving msg: %s\n", strerror( errnum ));
            }
        } while ((ret < 0 ) && (errno == 4));

        if(rbuf.index == 0){
            printf("Exiting ...\n");
        }
    }
}