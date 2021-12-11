#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>

//#define BUFFER_SIZE 2000000000L
#define BUFFER_SIZE 20000000L
#define THREADS 90
#define MAX_WIDTH_STRING 1000

int dictionary_size;
char* dict[100000];
int lenword;
int hasLen;
int lens[1000];
int countLenList;
int countThreads;
char* buffer[THREADS];
char* distinctList[THREADS];
long totalBinarySearchTime;
int paramTime;
char* listWords[BUFFER_SIZE];
int indexListWords=0;
int verbose;
int sorted;
int distinct;
int countDistinct;
pthread_mutex_t lock;
long timer;
int currentLen=0;


static long get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
	return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;

    /*
    struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * 1000000000 + currentTime.tv_usec;
    */
}

int binsearch(char* dictionaryWords[],int listSize,char* keyword){
    int keyLen = strlen(keyword);
    if((long)keyLen != lenword)
        return -1; //not found by len
	int mid, low = 0, high = listSize - 1;
	while (high >= low) {
		mid = (high + low) / 2;
		if (strcmp(dictionaryWords[mid],keyword) < 0)
			low = mid + 1;
		else if (strcmp(dictionaryWords[mid],keyword)>0)
			high = mid - 1;
		else
			return mid;
	}
	return -1; //not found
}

void *word_puzzle_solver(void* id){
    //lock
    pthread_mutex_lock(&lock);
    int nthread=(int)id;

    if(verbose)
        fprintf(stderr, "Note: Thread #%d: %s\n", nthread, "started!");

    for(currentLen=0; currentLen < countLenList; currentLen++)
    {
        lenword = lens[currentLen];
        char c = 0;
        int i, n = BUFFER_SIZE/countThreads-lenword;
        char* buf = buffer[nthread];

        for(i = 0; i < n; i++){
            if(c)
                buf[i + lenword - 1] = c;
            c = buf[i + lenword];
            buf[i + lenword] = '\0';

            timer = get_nanos();

            if(binsearch(dict, dictionary_size, buf + i) + 1){ //if search is successful!
                listWords[indexListWords] = (char*) malloc(sizeof(buf)+1);
                //add the word found in listWords
                strcpy(listWords[indexListWords],buf+i);
                indexListWords++;
            }

            timer = get_nanos() - timer;

            totalBinarySearchTime += timer;
        }
    }

    if(paramTime){
        float timeSeconds = (float)totalBinarySearchTime / 1000000000;
        fprintf(stderr, "Note: Total time spent on binary search: %.2f seconds in thread #%d\n", timeSeconds, (int)id);
    }

    //unlock
    pthread_mutex_unlock(&lock);
}

void printListWords(){
    int i=0;

    fprintf(stderr, "Original list\n");
    MostrarLista(listWords, indexListWords);

    if(sorted){
        int longitud = sizeof(listWords) / sizeof(char*);
        OrdenarLista(listWords, indexListWords);
        fprintf(stderr, "Sorted list\n");
        MostrarLista(listWords, indexListWords);
    }

    if(distinct){
        distinctLista(listWords, indexListWords);
        fprintf(stderr, "Distinct list\n");
        MostrarLista(listWords, indexListWords);
    }
}

void OrdenarLista(char *data[],const int tamano){
    char *temp;
    int i,j;

    for(i=0; i<tamano-1; i++){
        for(j=i+1; j<tamano; j++){
            if(strcmp(data[i], data[j]) > 0){
                temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }
        }
    }
}

void distinctLista(char *data[], int tamano){
    char *temp;
    char *actual;
    int i,j,k;

    for(i = 0; i < tamano; i++)
	{
        actual=data[i];

        for(j = i+1; j < tamano; j++)
		{
            temp=data[j];

            if(!strcmp(temp, actual))
            {
                k = j;

                while(k < tamano)//Mientras k sea menor que la cantidad de elementos
				{
                    data[k] = data[k+1];
                    ++k;
				}

                --tamano;
				--indexListWords;//Disminuimos el tama�o del vector ya que se eliminaron elementos repetidos.
				--j;
            }
		}
	}
}

void MostrarLista(char *data[],const int tamano){//muestra la lista
    int i;
    for(i=0;i<tamano;i++){
        fprintf(stderr, "Word found #%d: %s\n", i ,data[i]);
    }
}

void main(int argc, char** argv){
    //fprintf(stderr, "Cantidad de Argumentos: %d", argc);

    if(argc == 1){
        fprintf(stderr, "no arguments found!\n");
        exit(1);
    }

    countThreads=1;
    paramTime=0;
    indexListWords=0;
    hasLen=0;
    verbose=0;
    sorted=0;
    distinct=0;
    countDistinct=0;
    timer=0;

    char *str1, *token;
    int i=0, tokenIndex=0;
    countLenList=0;
    for(i=0; i<argc; i++){
        if(!strcmp(argv[i], "-len")){
            str1 = argv[i+1];
            token = strtok(str1, ",");
            lens[tokenIndex] = atoi(token);
            if(token != NULL){
                while(token != NULL){
                    lens[tokenIndex++] = atoi(token);
                    countLenList++;
                    token = strtok(NULL, ",");
                }
            }
            i++;
            hasLen=1;
        } else if(!strcmp(argv[i], "-nthreads")){
            countThreads=atoi(argv[i+1]);

            if(countThreads > THREADS){
                fprintf(stderr, "Error! You cant enter a value greater than 90 threads.\n");
                exit(1);
            }

            i++;
        } else if(!strcmp(argv[i], "-time")){
            paramTime = 1;
        } else if(!strcmp(argv[i], "-verbose")){
            verbose=1;
        } else if(!strcmp(argv[i], "-sorted")){
            sorted=1;
        } else if(!strcmp(argv[i], "-distinct")){
            distinct=1;
        }
    }

    if(hasLen == 0){
        lens[0] = 8;
        lens[1] = 9;
        countLenList=2;
    }

	timer = get_nanos();
	int thread_number, j, size = BUFFER_SIZE/countThreads;
	char temp[100];
	pthread_t threadID[countThreads];
	char line[MAX_WIDTH_STRING];
	char *pointer;
	FILE* f = fopen("dict.txt", "r");
	dictionary_size = 0;

	while(fgets(line, MAX_WIDTH_STRING, f)){
		sscanf(line, "%s\n", temp);
		if(strlen(temp) == 0)
			continue;
		dict[dictionary_size] = (char*) malloc(sizeof(temp)+1);
		strcpy(dict[dictionary_size++], temp);
	}
	fclose(f);

	for(thread_number = 0; thread_number < countThreads;thread_number++){
		buffer[thread_number] = (char*) malloc(size + 1);
		if(!fgets(buffer[thread_number], size + 1, stdin)){
            if(verbose)
                fprintf(stderr, "thread number: %d, Error: can't read the input stream!\n", thread_number);
			break;
		}
		if(verbose)
            fprintf(stderr, "Note: Thread #%d: %s\n", thread_number, "created!");
		if(pthread_create(threadID + thread_number, NULL, word_puzzle_solver, (void *) thread_number)){
			fprintf(stderr, "Error: Too many threads are created!\n");
			break;
		}
	}

	for(j = 0; j < countThreads; j++){
		pthread_join(threadID[j], NULL);
		if(verbose)
            fprintf(stderr, "Note: Thread %d joined!\n", j);
	}

	printListWords();

	if(verbose)
    {
        float timeSeconds = (float)totalBinarySearchTime / 1000000000;
        fprintf(stderr, "Note: Total time: %.2f seconds using %d threads!\n", timeSeconds, thread_number);
    }
}
