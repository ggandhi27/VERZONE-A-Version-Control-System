#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

void readDirectory(char *cwd)
{
	DIR *d;
	struct dirent *sir;
	d=opendir(cwd);
	while((dir=readdir(d)!=NULL))
	{
		if(dir->d_type==DT_DIR)
		{
			if(!((strcmp(dir->d_name,".")==0)||(strcmp(dir->d_name,"..")==0)||(dir->d_name[0]=='.')))
			{
				char dirName[4096];
				sprintf(dirName,"%s%s/",cwd,dir->d_name);
				readDirectory(dirName);
			}
			else
			{

			}
		}
	}
}

int main()
{
	char cwd[4096];
       	if((getcwd(cwd,sizeof(cwd))==NULL))
	{
		//If it is not able to fetch the path of the current working directory.
		perror("getcwd() error!");
	        exit(-1);
        }
 	//Concatenate the path of the directory with / as the end.
        strcat(cwd,"/");

	return 0;
}
