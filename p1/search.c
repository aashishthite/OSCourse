//Author: Aashish Thite
//

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
struct node
{
	char * fileName;
	int occurances;
	struct node * next;
};

int main(int argc, char** argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"Usage: search <input-number> <key-word> <input-list> <output>\n");
		exit(1);
	}
	int inputNumber = atoi(argv[1]);
	if(inputNumber == 0)
	{
		fprintf(stderr,"Usage: search <input-number> <key-word> <input-list> <output>\n");
		exit(1);
	}
	//printf("%d",argc);
	//*
	if(inputNumber != argc - 3 && inputNumber != argc - 4)
	{
		fprintf(stderr,"Usage: search <input-number> <key-word> <input-list> <output>\n");
		exit(1);
	}//*/
		
	FILE * outStream;
	if(inputNumber == argc - 4)
	{
		outStream = fopen(argv[3+inputNumber],"w");
		if(outStream == NULL)
		{
			fprintf(stderr,"Error: Cannot open file '%s'\n",argv[3+inputNumber]);
			exit(1);
		}
	}
	else
	{
		outStream = stdout;
	}
	char * keyWord = argv[2];
	int keyWordLength = strlen(keyWord);	
	
	//Head of the list
	struct node * head = (struct node *) malloc( sizeof(struct node) );
	head-> next = NULL;
	struct node * tail;
	tail = head;
	//do this for multiple files
	unsigned int i;
	for(i = 3; i < 3+inputNumber; i++)
	{
		char * inputFileName = argv[i];	
		FILE * fp;
		char * fileBuffer;
		fp = fopen(inputFileName,"r");
		if(fp == NULL)
		{
			fprintf(stderr,"Error: Cannot open file '%s'\n",inputFileName);
			exit(1);
		}
		//get file size
		fseek(fp, 0, SEEK_END);
		long long int fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);	
		fileBuffer = (char *) malloc(sizeof(char) * fileSize);
		if(fileBuffer == NULL)
		{
			fprintf(stderr, "Malloc failed");
			exit(1);	
		}
		size_t result = fread(fileBuffer,1,fileSize,fp);
		if(result != fileSize)
		{
			fprintf(stderr,"Reading error");
			exit(1);
		}
		char * temp = strstr(fileBuffer,keyWord);
		int count=0;
		while(temp != NULL)
		{
			count++;
			temp = strstr(temp+keyWordLength,keyWord);
		}
		free(fileBuffer);
		//printf("count of keyWord: %s is %d\n",keyWord,count);

		//tail = (struct node *) malloc(sizeof(sturct node));
		tail->fileName = inputFileName;
		tail->occurances = count;
		tail->next = (struct node *) malloc(sizeof(struct node));
		tail = tail->next;
		tail->next = NULL;
		fclose(fp);

	}
	//Print the files and occurances
	for(i = 0; i <inputNumber; ++i)
	{
		struct node * temp = head;
		struct node * maxSoFar = temp;
		while(temp->next)
		{
			if(temp->occurances > maxSoFar->occurances)
				maxSoFar = temp;
			//printf("%d %s\n",temp->occurances,temp->fileName);
			temp = temp->next;
		}
		fprintf(outStream,"%d %s\n",maxSoFar->occurances,maxSoFar->fileName);
		if(head == maxSoFar)
		{
			head = head-> next;
			free(maxSoFar);
			maxSoFar = NULL;
			continue;
		}
		temp = head;
		while(temp->next != maxSoFar)
		{
			temp = temp->next;
		}
		temp->next = maxSoFar->next;
		free(maxSoFar);		
	}
	if(outStream != stdout)
	{
		fclose(outStream);
	}
	//free resourses
	/*
	while(head)
	{
		struct node * temp = head;
		head = head-> next;
		free(temp);
	}*/
	//printf("Hello World \n");
	return(0);
}
