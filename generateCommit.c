#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "diff.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "RecoverFile.h"
//Fetch data from header file


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

void fetchData(char *fileName, char *header)
{
	char *Data;
	Data=(char *)malloc(4096*sizeof(char));
	Data=NULL;
	FILE *fp;
	size_t len=0;
	ssize_t read;
	fp=fopen(fileName,"r");
	if(fp==NULL)
	{
		exit(-1);
	}
	read=getline(&Data,&len,fp);
	printf("%s\n",Data);
	printf("lenght = %ld\n",strlen(Data));
	strcpy(header,Data);
	free(Data);
	fclose(fp);
}

void readCommit(char *commit,char *Data1,char *type)
{
	char *Data,*typ;
	Data=(char *)malloc(4096*sizeof(char));
	typ=(char *)malloc(4096*sizeof(char));
	FILE *fp;
	Data=NULL;
	fp=fopen(commit,"r");
	if(fp==NULL)
	{
		printf("Not able to open the file 1\n");
		exit(-1);
	}
	size_t len=0;
	ssize_t read;
	read=getline(&Data,&len,fp);
	read=getline(&typ,&len,fp);
	strcpy(Data1,Data);
	strcpy(type,typ);
	free(Data);
	fclose(fp);
}

int  isEditted(char *repoPath)
{
	if(access(repoPath,F_OK)!=-1)
	{
		printf("The file is editted.\n");
		return 1;
	}
	else
	{
		//printf("The file is not editted.\n");
		return 0;
	}
}


void genFileName(char *filePath,char *fileName)
{
	char xyz[256];
	int i,l;
	l=strlen(filePath);
	i=l-1;
	strcpy(fileName,"\0");
	while(filePath[i]!='/')
	{
		strcpy(xyz,fileName);
		sprintf(fileName,"%c%s",filePath[i],xyz);
		i--;
	}
}


int findFileObject(char *Tree,char *filepath,char *objectpath,char *patchType)
{
	if(Tree[strlen(Tree)-1]=='\n')
	{
		Tree[strlen(Tree)-1]='\0';
	}
	char *filePath,*fileType,*objectPath,*n;
	size_t lent,lenft,lenop;
	ssize_t read;
	char fileName[256],temp[4096];
	filePath=(char *)malloc(4096*sizeof(char));
	fileType=(char *)malloc(15*sizeof(char));
	objectPath=(char *)malloc(4096*sizeof(char));
	n=(char *)malloc(sizeof(char));
	filePath=fileType=objectPath=NULL;
	FILE *fp;
	fp=fopen(Tree,"r");
	if(fp==NULL)
	{
		printf("Not able to open the file.\n");
		exit(-1);
	}
	while(!feof(fp))
	{
		read=getline(&filePath,&lent,fp);
		read=getline(&fileType,&lenft,fp);
		read=getline(&objectPath,&lenop,fp);
		read=getline(&n,&lenft,fp);
		filePath[strlen(filePath)-1]='\0';
		fileType[strlen(fileType)-1]='\0';
		objectPath[strlen(objectPath)-1]='\0';
		if((strcmp(fileType,"File")==0)||(strcmp(fileType,"Patch")==0))
		{
			if(strcmp(filePath,filepath)==0)
			{
				strcpy(objectpath,objectPath);
				strcpy(fileType,patchType);
				printf("object : %s\n",objectpath);
				return 1;
			}
		}
		else if(strcmp(fileType,"Directory")==0)
		{
			if(strncmp(filePath,filepath,strlen(filePath))==0)
			{
				return findFileObject(objectPath,filepath,objectpath,patchType);
			}
		}
	}
	return 0;
}

void fetchNextCommit(char *commit,char *nextValue)
{
	char *Data,*typ,*nv;
	char xyz[4096];
	Data=(char *)malloc(4096*sizeof(char));
	typ=(char *)malloc(15*sizeof(char));
	nv=(char *)malloc(4096*sizeof(char));
	FILE *fp;
	Data=NULL;
	fp=fopen(commit,"r");
	if(fp==NULL)
	{
		printf("Not able to open the file 3\n");
		exit(-1);
	}
	size_t len=0;
	ssize_t read;
	read=getline(&Data,&len,fp);
	read=getline(&typ,&len,fp);
	read=getline(&nv,&len,fp);
	if(nv[0]!='n')
	{
		read=getline(&nv,&len,fp);
	}
	int i;
	for(i=2;i<(strlen(nv));i++)
	{
		strcpy(xyz,nextValue);
		sprintf(nextValue,"%s%c",xyz,nv[i]);
	}
	free(Data);
	free(typ);
	free(nv);
	fclose(fp);
}
void generateDifference(char *header,char *checkpoint,char *filePath,char *tempPath,char *temp,char *fname,char *patchType)
{
	char tree1[4096],tree2[4096];
	char type1[15],type2[15];
	readCommit(header,tree1,type1);
	tree1[strlen(tree1)-1]='\0';
	type1[strlen(type1)-1]='\0';
	readCommit(checkpoint,tree2,type2);
	tree2[strlen(tree2)-1]='\0';
	type2[strlen(type2)-1]='\0';
	char objectPath[4096];
	char pt[6];
	
	printf("header = %s\ntree1 = %s\ntype1 = %s\n\n",header,tree1,type1);
	printf("header = %s\ntree2 = %s\ntype2 = %s\n\n",checkpoint,tree2,type2);
	
	if(strcmp(header,checkpoint)==0)
	{
		//Generation of the first commit after the checkpoint
		if((strcmp(type1,"checkpoint")==0)&&(strcmp(type2,"checkpoint")==0))
		{
			printf("Header = checkpoint = CHECKPOINT.\n");
			int flag;
			flag=findFileObject(tree1,filePath,objectPath,pt);
			if(flag==1)
			{
				printf("object : %s\n",objectPath);
				printf("tempPath : %s\n",tempPath);
				printf("The versions available for the file %s are :\n",filePath);
				//printing the name of the recovery files present.
				DIR *tmp;
				struct dirent *tempDir;
				//open the filePath
				tmp=opendir(temp);
				//if successfully accessed the directory
				if(tmp)
				{
					//reading the contents of the directory
					while((tempDir=readdir(tmp))!=NULL)
					{
						//if the object is not a directory
						if(tempDir->d_type!=DT_DIR)
						{
							//if it is not the count file then print it.
							if(strcmp(tempDir->d_name,"count")!=0)
							{
								printf("%s\n",tempDir->d_name);
							}
						}
					}
					//choice is used for getting the choice for the menu
					int choice;
					//initializing choice
					choice=-1;
					//while choice is equals to -1
					//The loop goes on untill the user doesn't generates any patch for all the edited files.
					while(choice==-1)
					{
						//printing the menu
						printf("\n\n1. View any version.");
						printf("\n2. Select a version to generate patch.\n");
						printf("\nEnter your choice : ");
						scanf("%d",&choice);
						switch(choice)
						{
							int fileNum;
							case 1:
							{
								printf("Enter the number of the version you want to view : ");
								scanf("%d",&fileNum);
								//verPath stores the absolute path of the version file
								char verPath[4096];
								//copy the filePath in verPath
								strcpy(verPath,temp);
								//customize the value verPath by concatenating the 
								//selected version number at the end.
								sprintf(verPath,"%s%d",verPath,fileNum);
								//if the version number enterred is not present.
								if(access(verPath,F_OK)==-1)
								{
									//display the warning message.
									printf("Please select the appropriate file name.\n");
									choice=-1;
								}
								//if the version number selected is present
								else
								{
									//print the contents of the version file
									FILE *fp;

									//reading the contents of the version file
									fp=fopen(verPath,"r");
									char c;
									while((c=fgetc(fp))!=EOF)
									{
										printf("%c",c);

										choice=-1;
									}
									break;
								}
							}
							//generating the patch.
							case 2:
							{
								printf("Select a version number : ");
								scanf("%d",&fileNum);
								char verPath[4096];
								strcpy(verPath,temp);
								sprintf(verPath,"%s%d",verPath,fileNum);
								if(access(verPath,F_OK)==-1)
								{
									printf("Please select the appropriate file name.\n");
									choice=-1;
								}
								else
								{
									//calling the diff function to generate the path.
									diff(filePath,verPath,fname);
									char cmd[8192];
									sprintf(cmd,"cp %s %s",verPath,filePath);
									system(cmd);
									sprintf(cmd,"rm -rf %s",temp);
									system(cmd);
									strcpy(patchType,"Patch");
								}
								break;
							}
							default:
							{
								printf("Please enter the appropriate choice.\n");
								choice=-1;
							}
						}
					}
				}
			}
			//The file is not present in the previous checkpoint
			else
			{
				printf("The versions available for the file %s are :\n",filePath);
				//printing the name of the recovery files present.
				DIR *tmp;
				struct dirent *tempDir;
				//open the filePath
				tmp=opendir(temp);
				//if successfully accessed the directory
				if(tmp)
				{
					//reading the contents of the directory
					while((tempDir=readdir(tmp))!=NULL)
					{
						//if the object is not a directory
						if(tempDir->d_type!=DT_DIR)
						{
							//if it is not the count file then print it.
							if(strcmp(tempDir->d_name,"count")!=0)
							{
								printf("%s\n",tempDir->d_name);
							}
						}
					}
					//choice is used for getting the choice for the menu
					int choice;
					//initializing choice
					choice=-1;
					//while choice is equals to -1
					//The loop goes on untill the user doesn't generates any patch for all the edited files.
					while(choice==-1)
					{
						//printing the menu
						printf("\n\n1. View any version.");
						printf("\n2. Select a version to generate patch.\n");
						printf("\nEnter your choice : ");
						scanf("%d",&choice);
						switch(choice)
						{
							int fileNum;
							case 1:
							{
								printf("Enter the number of the version you want to view : ");
								scanf("%d",&fileNum);
								//verPath stores the absolute path of the version file
								char verPath[4096];
								//copy the filePath in verPath
								strcpy(verPath,temp);
								//customize the value verPath by concatenating the 
								//selected version number at the end.
								sprintf(verPath,"%s%d",verPath,fileNum);
								//if the version number enterred is not present.
								if(access(verPath,F_OK)==-1)
								{
									//display the warning message.
									printf("Please select the appropriate file name.\n");
									choice=-1;
								}
								//if the version number selected is present
								else
								{
									//print the contents of the version file
									FILE *fp;

									//reading the contents of the version file
									fp=fopen(verPath,"r");
									char c;
									while((c=fgetc(fp))!=EOF)
									{
										printf("%c",c);

										choice=-1;
									}
									break;
								}
							}
							//generating the patch.
							case 2:
							{
								printf("Select a version number : ");
								scanf("%d",&fileNum);
								char verPath[4096];
								strcpy(verPath,temp);
								sprintf(verPath,"%s%d",verPath,fileNum);
								if(access(verPath,F_OK)==-1)
								{
									printf("Please select the appropriate file name.\n");
									choice=-1;
								}
								else
								{
									//calling the diff function to generate the path.
									char cmd[8192];
									sprintf(cmd,"cp %s %s",verPath,fname);
									system(cmd);
									sprintf(cmd,"cp %s %s",verPath,filePath);
									system(cmd);
									sprintf(cmd,"rm -rf %s",temp);
									system(cmd);
									strcpy(patchType,"File");
								}
								break;
							}
							default:
							{
								printf("Please enter the appropriate choice.\n");
								choice=-1;
							}
						}
					}
				}
				return;
			}
		}
		else if((strcmp(type1,"commit")==0)&&(strcmp(type2,"commit")==0))
		{
			printf("Header = checkpoint = commit\n");
			int flag;
			flag=findFileObject(tree2,filePath,objectPath,pt);
			//printf("Tree2 : %s\nFilePath : %s\n",tree2,filePath);
			//printf("fname : %s\nobjectpath : %s\nfilepath : %s\n",fname,filePath,objectPath);
			if(isEditted(fname))
			{
				printf("Hello");
				patch(fname,objectPath);
				printf("File successfully created with path %s.\n",fname);
			}
			else
			{
				if(flag==1)
				{
					char cmd[8192];
					sprintf(cmd,"cp %s %s",objectPath,fname);
					printf("cmd : %s\n",cmd);
					system(cmd);
				}
				else
				{
					
					printf("The versions available for the file %s are :\n",filePath);
					//printing the name of the recovery files present.
					DIR *tmp;
					struct dirent *tempDir;
					//open the filePath
					tmp=opendir(temp);
					//if successfully accessed the directory
					if(tmp)
					{
						//reading the contents of the directory
						while((tempDir=readdir(tmp))!=NULL)
						{
							//if the object is not a directory
							if(tempDir->d_type!=DT_DIR)
							{
								//if it is not the count file then print it.
								if(strcmp(tempDir->d_name,"count")!=0)
								{
									printf("%s\n",tempDir->d_name);
								}
							}
						}
						//choice is used for getting the choice for the menu
						int choice;
						//initializing choice
						choice=-1;
						//while choice is equals to -1
						//The loop goes on untill the user doesn't generates any patch for all the edited files.
						while(choice==-1)
						{
							//printing the menu
							printf("\n\n1. View any version.");
							printf("\n2. Select a version to generate patch.\n");
							printf("\nEnter your choice : ");
							scanf("%d",&choice);
							switch(choice)
							{
								int fileNum;
								case 1:
								{
									printf("Enter the number of the version you want to view : ");
									scanf("%d",&fileNum);
									//verPath stores the absolute path of the version file
									char verPath[4096];
									//copy the filePath in verPath
									strcpy(verPath,temp);
									//customize the value verPath by concatenating the 
									//selected version number at the end.
									sprintf(verPath,"%s%d",verPath,fileNum);
									//if the version number enterred is not present.
									if(access(verPath,F_OK)==-1)
									{
										//display the warning message.
										printf("Please select the appropriate file name.\n");
										choice=-1;
									}
									//if the version number selected is present
									else
									{
										//print the contents of the version file
										FILE *fp;
	
										//reading the contents of the version file
										fp=fopen(verPath,"r");
										char c;
										while((c=fgetc(fp))!=EOF)
										{
											printf("%c",c);
	
											choice=-1;
										}
										break;
									}
								}
								//generating the patch.
								case 2:
								{
									printf("Select a version number : ");
									scanf("%d",&fileNum);
									char verPath[4096];
									strcpy(verPath,temp);
									sprintf(verPath,"%s%d",verPath,fileNum);
									if(access(verPath,F_OK)==-1)
									{
										printf("Please select the appropriate file name.\n");
										choice=-1;
									}
									else
									{
										//calling the diff function to generate the path.
										char cmd[8192];
										sprintf(cmd,"cp %s %s",verPath,fname);
										system(cmd);
										sprintf(cmd,"cp %s %s",verPath,filePath);
										system(cmd);
										sprintf(cmd,"rm -rf %s",temp);
										system(cmd);
										strcpy(patchType,"File");
									}
									break;
								}
								default:
								{
									printf("Please enter the appropriate choice.\n");
									choice=-1;
								}
							}
						}
					}
					return;
				}
			}
			printf("The versions available for the file %s are :\n",filePath);
			//printing the name of the recovery files present.
			DIR *tmp;
			struct dirent *tempDir;
			//open the filePath
			tmp=opendir(temp);
			//if successfully accessed the directory
			if(tmp)
			{
				//reading the contents of the directory
				while((tempDir=readdir(tmp))!=NULL)
				{
					//if the object is not a directory
					if(tempDir->d_type!=DT_DIR)
					{
						//if it is not the count file then print it.
						if(strcmp(tempDir->d_name,"count")!=0)
						{
							printf("%s\n",tempDir->d_name);
						}
					}
				}
				//choice is used for getting the choice for the menu
				int choice;
				//initializing choice
				choice=-1;
				//while choice is equals to -1
				//The loop goes on untill the user doesn't generates any patch for all the edited files.
				while(choice==-1)
				{
					//printing the menu
					printf("\n\n1. View any version.");
					printf("\n2. Select a version to generate patch.\n");
					printf("\nEnter your choice : ");
					scanf("%d",&choice);
					switch(choice)
					{
						int fileNum;
						case 1:
						{
							printf("Enter the number of the version you want to view : ");
							scanf("%d",&fileNum);
							//verPath stores the absolute path of the version file
							char verPath[4096];
							//copy the filePath in verPath
							strcpy(verPath,temp);
							//customize the value verPath by concatenating the 
							//selected version number at the end.
							sprintf(verPath,"%s%d",verPath,fileNum);
							//if the version number enterred is not present.
							if(access(verPath,F_OK)==-1)
							{
								//display the warning message.
								printf("Please select the appropriate file name.\n");
								choice=-1;
							}
							//if the version number selected is present
							else
							{
								//print the contents of the version file
								FILE *fp;

								//reading the contents of the version file
								fp=fopen(verPath,"r");
								char c;
								while((c=fgetc(fp))!=EOF)
								{
									printf("%c",c);

									choice=-1;
								}
								break;
							}
						}
						//generating the patch.
						case 2:
						{
							printf("Select a version number : ");
							scanf("%d",&fileNum);
							char verPath[4096];
							strcpy(verPath,temp);
							sprintf(verPath,"%s%d",verPath,fileNum);
							if(access(verPath,F_OK)==-1)
							{
								printf("Please select the appropriate file name.\n");
								choice=-1;
							}
							else
							{
								getFileName(tempPath);
								//calling the diff function to generate the path.
								diff(fname,verPath,tempPath);
								char cmd[8192];
								sprintf(cmd,"cp %s %s",verPath,filePath);
								system(cmd);
								sprintf(cmd,"rm -rf %s",temp);
								system(cmd);
								sprintf(cmd,"mv %s %s",tempPath,fname);
								system(cmd);
								strcpy(patchType,"Patch");
							}
							break;
						}
						default:
						{
							printf("Please enter the appropriate choice.\n");
							choice=-1;
						}
					}
				}
			}
			
		}
	}
	else
	{
		//When header and the checkpoint are different
		if(strcmp(type2,"checkpoint")==0)
		{
			printf("Checkpoint = CHECKPOINT\n");
			int flag;
			flag=findFileObject(tree2,filePath,objectPath,pt);
			//Found the file in the hierarchy.
			if(flag==1)
			{
				char cmd[8192];
				sprintf(cmd,"cp %s %s",objectPath,fname);
				printf("cmd : %s\n",cmd);
				system(cmd);
			}
			char nextValue[4096];
			strcpy(nextValue,"\0");
			fetchNextCommit(checkpoint,nextValue);
			printf("Next Value : %s\n",nextValue);
			generateDifference(header,nextValue,filePath,tempPath,temp,fname,patchType);
		}
		else if(strcmp(type2,"commit")==0)
		{
			printf("Checkpoint = commit\n");
			int flag;
			flag=findFileObject(tree2,filePath,objectPath,pt);
			if(isEditted(fname))
			{
				patch(fname,objectPath);
				printf("File successfully created with path %s.\n",fname);
			}
			else
			{
				if(flag==1)
				{
					char cmd[8192];
					sprintf(cmd,"cp %s %s",objectPath,fname);
					printf("cmd : %s\n",cmd);
					system(cmd);
				}
			}
			char nextValue[4096];
			strcpy(nextValue,"\0");
			fetchNextCommit(checkpoint,nextValue);
			printf("Next Value : %s\n",nextValue);
			generateDifference(header,nextValue,filePath,tempPath,temp,fname,patchType);
		}
	}
}
/*
void readTree(char *Tree,char *cwd,char *headerAddress,char *checkpointAddress)
{
	char *filePath,*fileType,*objectPath,*n;
	size_t lent,lenft,lenop;
	ssize_t read;
	char fileName[256],temp[4096],tempPath[4096];
	filePath=(char *)malloc(4096*sizeof(char));
	fileType=(char *)malloc(15*sizeof(char));
	objectPath=(char *)malloc(4096*sizeof(char));
	n=(char *)malloc(sizeof(char));
	filePath=fileType=objectPath=NULL;
	FILE *fp;
	fp=fopen(Tree,"r");
	if(fp==NULL)
	{
		printf("Not able to open the file.\n");
		exit(-1);
	}
	while(!feof(fp))
	{
		read=getline(&filePath,&lent,fp);
		read=getline(&fileType,&lenft,fp);
		read=getline(&objectPath,&lenop,fp);
		read=getline(&n,&lenft,fp);
		filePath[strlen(filePath)-1]='\0';
		fileType[strlen(fileType)-1]='\0';
		objectPath[strlen(objectPath)-1]='\0';
		
		printf("Directory : %s\n",Tree);
		printf("File Path : %s\n",filePath);
		printf("File Type : %s\n",fileType);
		printf("Object Type : %s\n",objectPath);
		
		if(strcmp(fileType,"Directory")==0)
		{
			readTree(objectPath,filePath,headerAddress,checkpointAddress);
		}
		else if(strcmp(fileType,"File")==0)
		{
			genFileName(filePath,fileName);
			//printf("File Name : %s\n\n",fileName);
			sprintf(temp,"%s.temp/%s/",cwd,fileName);
			sprintf(tempPath,"%s.temp/",cwd);
			//printf();
			printf("temp : %s\n",temp);
			printf("tempPath : %s\n",tempPath);
			if (isEditted(temp))
			{
				generateDifference(headerAddress,checkpointAddress,filePath,tempPath,temp);
			}
		}
	}
	fclose(fp);
}
*/
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


void readDirectory(char *dirName,char *verzonePath,char *filePath,char *header,char *checkpoint)
{
	printf("In readDirectory\n");
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
					readDirectory(dirPath,verzonePath,filePath,header,checkpoint);
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
				if(d->d_name[0]!='.')
				{
					char fname[4096];
					char temp[4096];
					char tempPath[4096];
					char patchType[6];
					strcpy(fname,verzonePath);
					getFileName(fname);
					char dirPath[4096];
					strcpy(dirPath,dirName);
					strcat(dirPath,d->d_name);
					sprintf(temp,"%s.temp/%s/",dirName,d->d_name);
					sprintf(tempPath,"%s.temp/",dirName);
					//printf();
					//printf("temp : %s\n",temp);
					//printf("tempPath : %s\n",tempPath);
					if (isEditted(temp))
					{
						generateDifference(header,checkpoint,dirPath,tempPath,temp,fname,patchType);
					}
					else
					{
						char tree1[4096];
						char type1[15];
						readCommit(header,tree1,type1);
						int flag=findFileObject(tree1,dirPath,fname,patchType);
						
					}
					changeNameToInode(fname,verzonePath);
					fprintf(fp,"%s\n%s\n%s\n\n",dirPath,patchType,fname);
				}
			}
		}
	}
	fclose(fp);
	changeNameToInode(fileName,verzonePath);
	//char fileName2[4096];
	strcpy(filePath,fileName);


}

void createCommit(char *header,char *checkpoint,char *filePath,char *verzonePath,char *headerAddress,char *checkpointnum)
{
	char tempFile[4096],c;
	FILE *fp;
	int n;
	strcpy(tempFile,verzonePath);
	getFileName(tempFile);
	fp=fopen(tempFile,"w");
	fprintf(fp,"%s\ncommit\np:%s\n",filePath,header);
	fclose(fp);
	changeNameToInode(tempFile,verzonePath);
	fp=fopen(header,"a+");
	fprintf(fp,"n:%s",tempFile);
	fclose(fp);

	fp=fopen(headerAddress,"w");
	fprintf(fp,"%s",tempFile);
	fclose(fp);

	fp=fopen(checkpointnum,"r");
	c=fgetc(fp);
	n=atoi(&c);
	n=(n+1)%5;
	fclose(fp);

	fp=fopen(checkpointnum,"w");
	fprintf(fp,"%d",n);
	fclose(fp);

}

int main()
{
	char header[4096],checkpoint[4096],checkpointnum[4096];
	char Data[4096],headerfile[4096];
	char cwd[4096],type[15];
	char headerAddress[4096],checkpointAddress[4096];
	char verzonePath[4096],filePath[4096];
	if((getcwd(cwd,sizeof(cwd))==NULL))
	{
		//If it is not able to fetch the path of the current working director
		perror("getcwd() error!");
		exit(-1);
	}

	strcat(cwd,"/");	
	strcpy(headerAddress,cwd);
	strcpy(checkpointAddress,cwd);
	sprintf(verzonePath,"%s.VERZONE/objects/",cwd);
	strcat(headerAddress,".VERZONE/header");
	strcat(checkpointAddress,".VERZONE/checkpoint");
	sprintf(checkpointnum,"%s.VERZONE/checkpointnum",cwd);
	fetchData(headerAddress,header);
	fetchData(checkpointAddress,checkpoint);
	readCommit(header,Data,type);
	printf("%s\n",Data);
	Data[strlen(Data)-1]='\0';
	//readTree(Data,cwd,header,checkpoint);
	readDirectory(cwd,verzonePath,filePath,header,checkpoint);
	createCommit(header,checkpoint,filePath,verzonePath,headerAddress,checkpointnum);
	return 0;
}
//Fetch data from checkpoint file
//Traverse the files in the header commit.
//Check if the file is edited or not by checking for the file in the .temp directory.
//If the file is present in the .temp directory then generate the state of the file till the last commit.
//Else add the details about the file as it is from the previous commit.
//___________________________________________________________________________________________________________
//
//For generation of the file.
//_____________________________
//
//Search for the file in the commit and trees
//On finding the files generate a file. 
//Keep on traversing the files and generating the files utill you reach the current header.
//
//I am able to find that which file is editted and which file is not.
//Now I need to check for the file from the checkpoint and traverse the commit list upto the header.
//
//So I need to insert the meta data properly in the commit files and have to traverse in accordance with that.
//
//_______________________________________________________________________________________________________________
//
//Conditions:
//_______________
//
// 1. Both header and checkpoint are same values and are pointing to the checkpoint.
//
// In this case we need to perform the difference operation between the object to which the file entry in the tree is pointing to 
// and the version from the .temp directory which the user has opted for.
//
// 2. When the header is pointing to a commit file. 
//
// In this case a variable will be given the value of the checkpoint and we have to traverse the commit list upto the header file data.
// We need to keep on generating the file and finally when the value of the header and checkpoint becomes equal we have to generate the 
// file for the last time and then perform the difference operation between the file generated and the version from the .temp directory which
// the user has opted for.
//
// If the object created is commit and not a checkpoint
// in that case, as soon as the new commit or a checkpoint is created we need to update the path of the newly created commit file
// in  the previous commit file and update the header file data.
//
//
