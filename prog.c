#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/statvfs.h>

struct Action{
	unsigned char rand;
	unsigned int zeros;
	unsigned char shread;
	unsigned char error;
	char** shreadFileNames;
	unsigned int shreadFileNum;
}action;

void init(){
	action.error = 0;
	action.rand = 0;
	action.shread = 0;
	action.zeros = 3;
	action.shreadFileNum = 0;
}



void check(int argc, char* arg){
	if(strcmp(arg, "-r") == 0){
		action.rand = 1;
	}
	else if(strncmp(arg, "-z=", 3) == 0){
		action.zeros = atoi((char*)(arg + 3));
	}
	else if(action.shread){
		strcpy(action.shreadFileNames[action.shreadFileNum++], arg);
	}
	else if((!action.shread) && (strcmp(arg, "-s") == 0)){
		action.shreadFileNames = (char **)malloc(argc*sizeof(char*));
		for(int i=0;i<argc;++i)
			action.shreadFileNames[i] = (char*) malloc(10);
		action.shread = 1;
	}
	else{
		action.error = 1;
	}
}

void parseArguments(int argc, char* argv[]){
	for(int i=1;i<argc;++i){
		check(argc - i, argv[i]);
		printf("arg %d : %s\n", i, argv[i]);
		if(action.error)
			break;
	}
}


void printHelp(){
	printf("\nUsage: USB-Guard [OPTION]... [FILE]...\n");
	printf("Makes it harder for any forensics expert to retrive data from the disk.\n\n");
	printf("  -r\t\tWrites random data on memory after all parses are over.\n");
	printf("  -s\t\tEnables the shreading mode (used to delete files).\n");
	printf("  -z=<number>\tUsed to set the number of parses of writing null on disk.\n\n");
}

void shread(){
	for(int i=0;i<action.shreadFileNum;++i)
		processFile(action.shreadFileNames[i], (char)0);
}

void wipePart(){
	processFile(".diskwiper", 1);
}

void processFile(char* fileName, char fullPartFlag){
	printf("File: %s\n",fileName);
	for(int i=0;i<action.zeros;++i)
		writeZeros(fileName, fullPartFlag);
	if(action.rand)
		writeRand(fileName, fullPartFlag);
	remove(fileName);
}

void updateProgressBar(double percentCompleted){
	printf("\r\t[");
	for(int i=0;i<100;++i){
		if(percentCompleted > i)
			printf("#");
		else
			printf(" ");
	}
	printf("]\t(%.4f completed)", percentCompleted);
}

void writeZeros(char* fileName, char fullPartFlag){
	unsigned long long int size;
	if(fullPartFlag){
		char path[] = "./";
		struct statvfs stat;
		if(statvfs(path, &stat) != 0){
			printf("\r\nERROR : Failed to get free space in drive.\n");
			return;
		}
		size = stat.f_bsize * stat.f_bavail;
	}
	else{
		FILE* file;
		if((file = fopen(fileName, "rb")) == NULL){
			printf("\r\nERROR : Couldnot get FileSize.\n");
			return;
		}
		fseek(file, 0L, SEEK_END);
		size = ftello(file);
		fclose(file);
	}
	printf("Writting NULL\n");
	char null = 0;
	FILE* file;
	if((file = fopen(fileName, "wb+")) == NULL){
		printf("\r\nERROR : Couldnot open File\n");
		return;
	}
	fseek(file, 0L, SEEK_SET);
	double percentCompleted = 0;
	double deltaPercent = ((double)100.00/size);
	for(long long int writen=0;size>=writen;++writen, percentCompleted+=deltaPercent){
		fwrite(&null, 1, 1, file);
		if(writen%262144 == 0)
			updateProgressBar(percentCompleted);
	}
	printf("\r\033[K");
	fclose(file);
}

void writeRand(char* fileName, char fullPartFlag){
	printf("\rWritting Rand\n");
}

void printSelections(){
	printf("\nZeros : %d\n", action.zeros);
	printf("Rand : %d\n", action.rand);
}

void actionTaker(){
	if(action.error){
		printHelp();
	}
	else if(action.shread){
		printSelections();
		shread();
	}
	else{
		printSelections();
		wipePart();
	}
}

int main(int argc, char* argv[]){
	init();
	parseArguments(argc, argv);
	actionTaker();
	return 0;
}
