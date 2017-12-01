/*This code is supposed to monitor the 	*
 *directories whose name is given as the*
 *command line arguement.		*
 * */

// dirent.h is the header file which is added
// for using the functions which are used 
// on the directories.
#include <dirent.h>

// stdio.h is a header file for standard input 
// output. Functions like printf() and scanf()
// are present in this header file.
#include <stdio.h>

//String handling functions like strcpy(),
//strcat(), strlen(), etc are present in
//the header file string.h
#include <string.h>

//It is a standard library header file 
//which contains various functions like atoi(),
//malloc(), etc.
#include <stdlib.h>

//Header file used for error handling.
#include <errno.h>
#include <sys/types.h>

//inotify is an API available in linux,
//which is used for monitoring the file 
//system events on the linux platform.
#include <sys/inotify.h>
#include <limits.h>
#include <unistd.h>

//Pthread is an API for performing operations
//on threads.
#include <pthread.h>

#include "VerzoneCreation.h"

//Maximum number for events which can wait
//in the queue for processing.

#define MAX_EVENTS 1024

#define LEN_NAME 16

//Size of the inotify_event structure which
//will store the occurrence of the events.

#define EVENT_SIZE (sizeof(struct inotify_event))

//Calculating the size of the buffer.
//each event size plus the length of the event
//name will be multiplied by the maximum number
//of events.

#define BUF_LEN (MAX_EVENTS *(EVENT_SIZE + LEN_NAME))


//Macro to store the value of the green
//color. When printed along with the text
//will show the text on screen in green color.

#define GREEN "\x1B[32m"


//Macro to store the value of the blue
//color. When printed along with the text
//will show the text on screen in green color.

#define BLUE "\x1B[34m"

//This function is used to list the name 
//of the directories which are present in 
//the directory name mentioned.
//On getting the name of the directory,
//the function is again called recursively 
//and the name of the subdirectory is now
//passed as the arguement.

void addDirectoryWatch(char *dirName);

//This function is used to add an inotify
//watch descriptor to the directory name
//passed as the arguement.

void* initializeInotify(void *dir);

FILE *fp;

//Main function.
//The name of the directory must be
//passed as the command line arguement.
//If the command line arguement is not 
//passed in that case it must take the 
//current working directory as the arguement
//and pass it to the addDirectoryWatch function.

int main(int argc,char **argv)
{
	//Charater array to store the path for the current working directory.
	char cwd[1024];

	//When the command line arguements are passed then the value of argc is greater than 1.
	if(argc>1)
	{
		//Check if the name given as the command line arguement really exists or not.
		if(access(argv[1],F_OK)!=-1)
		{
			DIR *dir;
			//Checking if the name provided is a directory or it is a file.
			//If it is a directory then it will be opened by the function opendir().
			dir=opendir(argv[1]);
			if(dir)
			{
				closedir(dir);
				//If it is a directory then pass the command line arguement to the function addDirectoryWatch.
				addDirectoryWatch(argv[1]);
			}
			else
			{
				//If it is not a directory then give error that the directory doesnot exists.
				printf("No such directory exists.\n");
				exit(-1);
			}
		}
		else
		{
			//If the file name passed doesnot exists then print the error and exit.
			char cmd[4096],dirName[256];
			strcpy(dirName,argv[1]);
			if(dirName[strlen(dirName)-1]!='/')
			{
				strcat(dirName,"/");
			}
			strcpy(cmd,"mkdir ");
			strcat(cmd,dirName);
			if(system(cmd))
			{
				perror("Not able to initialize the repository.");
			}
			else
			{
				createVerzone(dirName);
				addDirectoryWatch(dirName);
			}
		}
	}
	else
	{
		//When no command line arguement is passed then we have to get the path of the current working directory.
		//char *getcwd(char *buf, size_t size);
		//getcwd() fucntion will return the path of the current working directory and in the char array cwd.
        	if((getcwd(cwd,sizeof(cwd))==NULL))
		{
			//If it is not able to fetch the path of the current working directory.
			perror("getcwd() error!");
		        exit(-1);
	        }
		//Concatenate the path of the directory with / as the end.
	        strcat(cwd,"/");
		//Pass the path of the current working directory to the addDirectoryWatch().
		createVerzone(cwd);
		addDirectoryWatch(cwd);
	}
	//The function addDirectoryWatch using multi threading for adding watch to the directories and subdirectories.
	//We need that the daemon process must go on inifinitely. If the main thread will end, then it is going to end all the
	//subthreads which are generated from it.
	//Hence, this infinite loop is added.
	while(1);
	return 0;
}


void addDirectoryWatch(char *dirName)
{
	//DIR is a structure.
	DIR *d;

	int len;
	len=strlen(dirName);
	if(dirName[len-1]!='/')
	{
		strcat(dirName,"/");
	}


	/*
	struct dirent 
	{
		ino_t          d_ino;        Inode number 
	      	off_t          d_off;        Not an offset; see below 
	      	unsigned short d_reclen;     Length of this record 
	      	unsigned char  d_type;       Type of file; not supported by all filesystem types 
	      	char           d_name[256];  Null-terminated filename 
	};*/
	struct dirent *dir;
	d=opendir(dirName);
	
	//pthread_t is type similar to that of int.
	pthread_t thread_ID;

	//int pthread_create(pthread_t * thread, 
	//                   const pthread_attr_t * attr,
	//                   void * (*start_routine)(void *), 
	//                   void *arg);
	//                                                                         
	//This function is used to create a thread.
	//
	//thread is the pointer to the pthread_t type variable which is used to store the thread id for the thread which is generated.
	//attr is the pointer to the structure for storing the attributes for the thread to be generated.
	//start_routine is the name of the function/routine which is needed to be executed by the thread.
	//arg is the pointer to the arguements which are needed to be passed to the function which is to be executed by the thread.
	
	//Generate a thread and store the id for the thread in the location pointed by the pointer variable thread_ID.
	//The thread to be generated doesnot requires any attribute modification hence pass NULL.
	//initializeInotify function is to be executed by the thread.
	//dirName is the name of the directory on which we want to set the inotify watch. We have to pass the name of the directory
	//to the function inotifyInitialize.
	pthread_create(&thread_ID,NULL,initializeInotify,(void*)dirName);
	
	//If the directory name given as arguement to the function is opened successfully, then executed the following set of statements.
	if(d)
	{
		//readdir(d) is used to read the names of the files and the directories which are present in that particular directory.
		//It will return a struct dirent * type of a pointer which will store the properties of the directory or file read.
		while((dir=readdir(d))!=NULL)
		{
			//Checking if the object retrieved is a directory or not.
			if(dir->d_type==DT_DIR)
			{
				//Here we have assumed that the names of the directories starting with a .(dot) are temporary directory
				//or it is a directory storing the meta data for the version controlling system (VERZONE).
				//Hence, all those directories are not be monitored.
				if(!((strcmp(dir->d_name,".")==0)||(strcmp(dir->d_name,"..")==0)||(dir->d_name[0]=='.')))
				{
					//subDirPath is an array of char type which is used to store the absolute path of the subdirectories.
					//subDirName is an array of char type which is used to store the name of the subdirectory.
					char subDirPath[4096],subDirName[4096];
					//Copy the name of subdirectory into the char array subDirName.
					strcpy(subDirName,dir->d_name);
					//Concatenate the name of the subdirectory with a / at the end.
					strcat(subDirName,"/");
					//Copy the directory path into the subDirPath.
					strcpy(subDirPath,dirName);
					//Concatenate the directory path with the subdirectory name, so that we can get the absolute path
					//for the subdirectory.
					strcat(subDirPath,subDirName);
					//Print the name of the parent directory in blue.
					printf("Parent Directory : %s%s\n",BLUE,dirName);
					//print the name of the subdirectory in green.
					printf("%s%s\n",GREEN,dir->d_name);
					//Recursively call the function addDirectoryWatch by passing the absolute path of the subdirectory.
					addDirectoryWatch(subDirPath);
				}
			}
		}
		//Close the directory which was opened using the function opendir.
		closedir(d);
	}
}

//Function to initialize the thread which is going to monitor all the filesystem events which are to be executed on the directory name passed.
void* initializeInotify(void *dir)
{
	//Buffer is the array which is going to queue all the events occuring in the directory, untill those events are processed.
	//dirName is a char array which is going to store the name of the directory in the dir arguement.
	char buffer[BUF_LEN],dirName[4096];
	//Copying the name of the directory into the variable dirName.
	strcpy(dirName,(char*)dir);
	//RepositoryName is an array which is used for storing the absolute path for the .temp directory.
	//.temp directory will be created in each and every subdirectory of the repository.
	char RepositoryName[4096];
	int length,i=0;
	//Generating the absolute path for the .temp directory of that particular directory.
	strcpy(RepositoryName,dirName);
	strcat(RepositoryName,".temp/");
	//cmd will store the command which will create the .temp directory
	char cmd[4150];
	/*mkdir is a linux system command which is used for the creation of a directory.
	-p is an option of the mkdir command which will create the complete hierarchy of the directory passed along with it
	and if the directory is already mentioned then  it will not give any error and the directory structure remains intact.
	*/
	strcpy(cmd,"mkdir -p ");
	strcat(cmd,RepositoryName);
	printf("%s\n",RepositoryName);
	//Executing the command on the linux system for the creation of the .temp directory.
	system(cmd);
	/* fd id the variable used for storing the file descriptor.
	 * when the inotify is initialized using the function inotify_init() then it returns a file descriptor. 
	*/
	
	//wd is used for storing the watch descriptor.
	int fd,wd;

	//initializing inotify.
	fd=inotify_init();

	//If the system is not able to initialize inotify.
	if(fd<0)
	{
		perror("Couldn't initalize inotify");
	}

	//Add a watch descriptor on the dirName.
	//This watch descriptor is going to monitor the directory for create, modify and delete events.
	wd=inotify_add_watch(fd,dirName,IN_CREATE|IN_MODIFY|IN_DELETE);

	//When not able to set the watch descriptor.
	if(wd ==-1)
	{
		printf("Couldn't add watch to %s.\n",dirName);
		exit(-1);
	}
	else
	{
		printf("Watching :: %s\n",dirName);
	}
	while(1)
	{
		//i is used as iterator to iterate in the buffer array.
		i=0;
		//The events occuring will be stored in the file descriptor.
		//read function will read the content of the file descriptor into the buffer.
		//length variable will store the number events that have occurred.
		length=read(fd,buffer,BUF_LEN);
		
		//If not able to read the file descriptor.
		if(length<0)
		{
			perror("Not able to read the file descriptor");
		}

		//This loop is read the buffer array.
		while(i<length)
		{
			//type casting the content of the buffer into the inotify_event type.
			struct inotify_event *event=(struct inotify_event *)&buffer[i];
			if(event->len)
			{
				//If any file or a directory in created in the directory we are watching.
				if(event->mask & IN_CREATE)
				{
					//If a directory is created.
					if(event->mask & IN_ISDIR)
					{
						printf("The directory %s was created.\n",event->name);
						char abc[4096];
						//Generating the absolute path for the new subdirectory.
						strcpy(abc,dirName);
						strcpy(dirName,event->name);
						strcat(dirName,"/");
						strcat(abc,dirName);
						strcpy(dirName,abc);
						pthread_t thread_ID;

						//Creating a new thread which will again call the initializeInotify
						//function for the new subdirectory and starts monitoring the new subdirectory.
						pthread_create(&thread_ID,NULL,initializeInotify,(void*)dirName);
						printf("Started monitoring the directory %s.\n",dirName);
					}

					//If a new file is created.
					else
					{
						printf("The file %s was created with wd %d\n",event->name,event->wd);
						char fileName[4096],fullFileName[4096];
						//fileName will simply store the name of the file which is created.
						strcpy(fileName,event->name);
						//fullFileName will store the absolute path for the newly created file.
						strcpy(fullFileName,dirName);
						strcat(fullFileName,fileName);
						strcat(fileName,"/");

						//checking if the new file is not a temporary file.
						if((fileName[0]!='.')&&(strcmp(fileName,"4913/")!=0))
						{
							//fileDir will store the absolute value of the directory created inside the .temp 
							//directory with the name of the file which is created.
							char fileDir[4150],fname[4150];
							//copying the absolute path of the .temp directory into fileDir.
							strcpy(fileDir,RepositoryName);
							//Concatenating the fileDir with the name of the created.
							strcat(fileDir,fileName);
							char cmd[4150];
							//creating the command to create the directory with the name of the created file,
							//inside the .temp directory.
							strcpy(cmd,"mkdir -p ");
							strcat(cmd,fileDir);
							system(cmd);
							//fname will store the */.temp/filename/count
							//count is the file which will store the number of the latest version of the recovery file.
							strcpy(fname,fileDir);
							strcat(fname,"count");
							//checking if the file count is already created or not.
							if(access(fname,F_OK)!=-1) 
							{
								FILE *fp;
								//open file count in read mode
								fp=fopen(fname,"r");
								//char array to store the data read from the file count.
								char count[3];
								//reading data from the file count
								fgets(count,3,fp);
								//closing the file
								fclose(fp);
								int num;
								//ascii to interger. Converting the number string to integer type
								num=atoi(count);
								//incrementing the number
								num++;
								//open the file count in write mode.
								fp=fopen(fname,"w");
								//writing the incremented value to the file count.
								fprintf(fp,"%d",num);
								//closing the file
								fclose(fp);
								//strcat(fileDir,"V");
								//modifying the value of string fileDir as */.temp/filename/version_number
								sprintf(fileDir,"%s%d",fileDir,num);
								//copy the contents of the file in the main repository into the 
								//file with latest version_number inside the directory 
								// */.temp/filename/version_number
								strcpy(cmd,"cp ");
								strcat(cmd,fullFileName);
								strcat(cmd," ");
								strcat(cmd,fileDir);
								system(cmd);
							}
							//When the file count is not present
							else
							{
								FILE *fp;
								//open the file in write mode
								fp=fopen(fname,"w");
								//writing 1 to the file it is the latest file number as the
								//file is edited for the first time. 
								fprintf(fp,"%d",1);
								fclose(fp);
								//strcat(fileDir,"V");
								sprintf(fileDir,"%s1",fileDir);
								strcpy(cmd,"cp ");
								strcat(cmd,fullFileName);
								strcat(cmd," ");
								strcat(cmd,fileDir);
								system(cmd);
							}
						}
					}
				}
				//When the modification to a file or the directory occurs.
				if(event->mask & IN_MODIFY)
				{
					//if the directory is modified
					if(event->mask & IN_ISDIR) 
					{ 
						printf("The directory %s was modified.\n",event->name); 
					} 
					//if the file is modified
					else
					{
						printf("The file %s was modified with wd %d\n",event->name,event->wd);
					}

				}
				//the deletion of a file or directory occurs.
				if(event->mask & IN_DELETE)
				{
					//If the directory is deleted
					if(event->mask & IN_ISDIR) 
					{ 
						printf("The directory %s was deleted.\n",event->name); 
					} 
					//if the file is deleted.
					else
					{
						printf("The file %s was deleted with wd %d\n",event->name,event->wd);
					}
				}
				//Incrementing the value of i
				//i = i + EVENT_SIZE + event->len.
				i+=EVENT_SIZE+event->len;
			}
		}
	}
	//removing the watch from the directory.
	inotify_rm_watch(fd,wd);
}
