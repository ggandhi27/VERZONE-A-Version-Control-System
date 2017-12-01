#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void getFileName(char *fileName)
{
	static int i=0;
	char temp[12];
	sprintf(temp,".temp%d.txt",i++);
	strcat(fileName,temp);
	return;
}

long int getInode(char *fileName)
{
	int fd;
	fd=open(fileName,O_RDONLY);
	if(fd<0)
	{
		exit(-3);
	}
	struct stat file_stat;
	int ret;
	ret=fstat(fd,&file_stat);
	if(ret<0)
	{
		exit(-4);
	}
	return file_stat.st_ino;
	
}


void changeNameToInode(char *fileName,char *verzonePath)
{
	long int inode;
	inode = getInode(fileName);
	char inodeName[4096],cmd[8192];
	sprintf(inodeName,"%s%ld",verzonePath,inode);
	sprintf(cmd,"mv %s %s",fileName,inodeName);
	system(cmd);
	strcpy(fileName,inodeName);
}

char* genFile(char *dirName,char *verzonePath,char *filepath)
{
	DIR *dir;
	dir=opendir(dirName);
	char fileName[4096];
	strcpy(fileName,verzonePath);
	getFileName(fileName);
	struct dirent *d;
	FILE *fp;
	printf("Dir Name = %s\n",dirName);
	printf("FileName = %s\n",fileName);
	fp=fopen(fileName,"a+");
	if(dir)
	{
		while((d=readdir(dir))!=NULL)
		{
			printf("%s\n",d->d_name);
			if(d->d_type==DT_DIR)
			{
				if(d->d_name[0]!='.')
				{
					char dirPath[4096];
					strcpy(dirPath,dirName);
					strcat(dirPath,d->d_name);
					strcat(dirPath,"/");
					char filePath[4096];
					printf("dirPath = %s\n",dirPath);
					genFile(dirPath,verzonePath,filePath);
					printf("File Path = %s\n",filePath);
					char str[4096];
					int len;
					/*len=len+(int)(log(len)+1);
					sprintf(str,"%ld",d->d_ino);
					len=strlen(str);
					len++;
					len=len+(log(len));
					sprintf(str,"%d%s\n",len,str);
					len=strlen(filePath);
					len++;
					len=len+(log(len));
					sprintf(str,"%d%s\n",len,str);*/
					fprintf(fp,"%s\nDirectory\n%s\n\n",dirPath,filePath);
				}
			}
			else
			{
				//when we come accross a file
				char fname[4096],cmd[8192];
				strcpy(fname,verzonePath);
				getFileName(fname);
				char dirPath[4096];
				strcpy(dirPath,dirName);
				strcat(dirPath,d->d_name);
				sprintf(cmd,"cp %s %s",dirPath,fname);
				system(cmd);
				changeNameToInode(fname,verzonePath);
				fprintf(fp,"%s\nFile\n%s\n\n",dirPath,fname);
			}
		}
	}
	fclose(fp);
	changeNameToInode(fileName,verzonePath);
	//char fileName2[4096];
	strcpy(filepath,fileName);
}

void updateLastCheckPoint(char *checkpointPath,char *filePath)
{
	FILE *fp;
	fp=fopen(checkpointPath,"w");
	fprintf(fp,"%s",filePath);
	fclose(fp);
}

void readHeader(char *headPath,char *head)
{
	char *data;
	data=(char *)malloc(4096*sizeof(char));
	FILE *fp;
	fp=fopen(headPath,"r");
	size_t len=0;
	ssize_t read;
	read=getline(&data,&len,fp);
	if(data[strlen(data)-1]!='\0')
	{
		data[strlen(data)-1]='\0';
	}
	strcpy(data,head);
	fclose(fp);
	free(data);
}

void updateHeader(char *headPath,char *filePath)
{
	FILE *fp;
	fp=fopen(headPath,"w");
	fprintf(fp,"%s",filePath);
	fclose(fp);
}
void updateHeader1(char *headPath,char *filePath,char *head)
{
	FILE *fp;
	fp=fopen(head,"a+");
	fprintf(fp,"n:%s",filePath);
	fclose(fp);
	fp=fopen(headPath,"w");
	fprintf(fp,"%s",filePath);
	fclose(fp);
}

void updateCheckPointNum(char *checkpointNum)
{
	FILE *fp;
	fp=fopen(checkpointNum,"w");
	fprintf(fp,"1");
	fclose(fp);
}

void createCheckpoint(char *treePath,char *headPath,char *verzonePath,char *checkpointPath,char *checkpointNum,int n)
{
	char tempPath[4096];
	strcpy(tempPath,verzonePath);
	getFileName(tempPath);
	FILE *fp;
	fp=fopen(tempPath,"w");
	if(n==0)
	{
		fprintf(fp,"%s\ncheckpoint\n",treePath);
		changeNameToInode(tempPath,verzonePath);
		updateHeader(headPath,tempPath);
	}
	else
	{
		char head[4096];
		readHeader(headPath,head);
		fprintf(fp,"%s\ncheckpoint\np:%s",treePath,head);
		changeNameToInode(tempPath,verzonePath);
		updateHeader1(headPath,tempPath,head);
	}
	updateLastCheckPoint(checkpointPath,tempPath);
	updateCheckPointNum(checkpointNum);
	fclose(fp);
}

int main(int argc,char **argv)
{
	char cwd[4096],verzonePath[4096],headPath[4096],checkpointPath[4096],checkpointNum[4096];
	FILE *fp;
        //When no command line arguement is passed then we have to get the path of the current working directory.
        //char *getcwd(char *buf, size_t size);
	//getcwd() fucntion will return the path of the current working directory and in the char array cwd.
	if(!(getcwd(cwd,sizeof(cwd))))
	{
                //If it is not able to fetch the path of the current working directory.
	        perror("getcwd() error!");
	        exit(-1);
	}
	//Concatenate the path of the directory with / as the end.
	strcat(cwd,"/");
	strcpy(verzonePath,cwd);
	strcat(verzonePath,".VERZONE/");
	if(access(verzonePath,F_OK)==-1)
	{
		printf("This directory is not an initlized VERZONE repository.\n");
		exit(-1);
	}
	sprintf(headPath,"%s/header",verzonePath);
	sprintf(checkpointPath,"%scheckpoint",verzonePath);
	sprintf(checkpointNum,"%scheckpointnum",verzonePath);
	strcat(verzonePath,"objects/");
	char filePath[4096];
	if(access(headPath,F_OK)==-1)
	{
		genFile(cwd,verzonePath,filePath);
		createCheckpoint(filePath,headPath,verzonePath,checkpointPath,checkpointNum,0);
		//This means that it is the first commit.
	}
	else
	{
		fp=fopen(checkpointNum,"r");
		int num;
		char c;
		c=fgetc(fp);
		num=atoi(&c);
		fclose(fp);
		if(num%5==0)
		{
			genFile(cwd,verzonePath,filePath);
			createCheckpoint(filePath,headPath,verzonePath,checkpointPath,checkpointNum,1);
			//It's time to create checkpoint.
		}
		else
		{
			//We have to generate commit.
			system("generateCommit");
		}
	}
	printf("%s\n",filePath);
	return 0;
}
