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

// dirent.h is the header file which is added
// for using the functions which are used 
// on the directories.

#include <dirent.h>

#include <unistd.h>
#include "diff.h"

//Macro to store the value of the green
//color. When printed along with the text
//will show the text on screen in green color.

#define GREEN "\x1B[32m"

//Macro to store the value of the blue
//color. When printed along with the text
//will show the text on screen in green color.

#define BLUE "\x1B[34m"

void findPatches(char *dirName);
int main()
{
	char cwd[1024];
	//Get the current working directory
	if((getcwd(cwd,sizeof(cwd))==NULL))
	{
		perror("getcwd() error!");
		exit(-1);
	}
	//Concatenate the current working directory with / at the end.
	strcat(cwd,"/");

	//Call the function findPatches()
	findPatches(cwd);
	return 0;
}

//
void findPatches(char *dirName)
{

	//DIR is a structure.
	DIR *d;

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

	//Open the directory.
	d=opendir(dirName);

	//If the directory is opened successfully.
	if(d)
	{
		//reading the contents of the directory
		while((dir=readdir(d))!=NULL)
		{
			//We assume that the files and the directories starting with .(dot) are
			//either temporary or are used for storing the meta data.
			if(dir->d_name[0]!='.')
			{
				//if the object accessed is a directory.
				if(dir->d_type==DT_DIR)
				{
					char dirPath[4096];

					//dirPath is used for storing the absolute path of the directory.
					//copy the parent directory name in dirPath.
					strcpy(dirPath,dirName);
					//concatenate the dirPath with the directory name.
					strcat(dirPath,dir->d_name);
					//concatenate the dirPath with a / at the end.
					strcat(dirPath,"/");
					
					//printf("%s%s\n",GREEN,dir->d_name);

					//Recursively call the function findPatches() and pass the path of the
					//directory accessed.
					findPatches(dirPath);
				}
				//if the object accessed is a file.
				else
				{
					/*filePath is the path of the directory inside .temp directory with the name of the 
					 *file which is accessed.
					 * */

					/*
					 * selectedFile is path of the file in the repository.
					 * */
					char filePath[4096],selectedFile[4096];

					//copy dirName in filePath
					strcpy(filePath,dirName);

					//copy dirName in selectedFile
					strcpy(selectedFile,dirName);

					//concatenate file path with .temp
					strcat(filePath,".temp/");

					//concatenate filePath with the name of the file accessed
					strcat(filePath,dir->d_name);

					//concatenate selectedFile with the name of the file accessed.
					strcat(selectedFile,dir->d_name);

					//concatenate filePath with / at the end.
					strcat(filePath,"/");

					//filePath will be of the form */.temp/file_name/
					//
					//selectedFile will be of the form */file_name
					
					//if the directory with the name of the accessed file is present in the .temp directory
					if(access(filePath,F_OK)!=-1)
					{
						printf("%sThe file %s was edited.\n",GREEN,dir->d_name);
						printf("%sThe versions available for the file %s are :\n",BLUE,dir->d_name);

						//printing the name of the recovery files present.
						DIR *tempPath;
						struct dirent *tempDir;

						//open the filePath
						tempPath=opendir(filePath);
						
						//if successfully accessed the directory
						if(tempPath)
						{
							//reading the contents of the directory
							while((tempDir=readdir(tempPath))!=NULL)
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
							//
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
										strcpy(verPath,filePath);

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
												printf("%s%c",GREEN,c);
											}
											choice=-1;
										}
										break;
									}

									//generating the patch.
									case 2:
									{
										printf("Select a version number : ");
										scanf("%d",&fileNum);
										char verPath[4096];
										strcpy(verPath,filePath);
										sprintf(verPath,"%s%d",verPath,fileNum);
										if(access(verPath,F_OK)==-1)
										{
											printf("Please select the appropriate file name.\n");
											choice=-1;
										}
										else
										{
											//calling the diff function to generate the path.
											diff(verPath,selectedFile);
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
			}
		}
	}
}
