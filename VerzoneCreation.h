void createVerzone(char *dirname)
{
	char dirPath[4096];
	strcpy(dirPath,dirname);
	strcat(dirPath,".VERZONE/");
	if(access(dirPath,F_OK)!=-1)
	{
		printf(".VERZONE is already existing.\nStarting monitorin g the directory %s.\n",dirPath);
	}
	else
	{
		strcat(dirPath,"objects");
		char cmd[4096];
		strcpy(cmd,"mkdir -p ");
		strcat(cmd,dirPath);
		system(cmd);
		printf(".VERZONE is initialized in the directory %s.\n",dirPath);
		printf("Command : %s.\n",cmd);
	}
	return;
}
