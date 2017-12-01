
// stdio.h is a header file for standard input 
// output. Functions like printf() and scanf()
// are present in this header file.
#include <stdio.h>


//It is a standard library header file 
//which contains various functions like atoi(),
//malloc(), etc.
#include <stdlib.h>

//String handling functions like strcpy(),
//strcat(), strlen(), etc are present in
//the header file string.h

#include <string.h>

/*It is the header file which contains the definition
of the function InitializeInotify().
*/
#include "inotify.h"

int main(int argc,char **argv)

{
	//struct node is defined inside inotify.h
	struct node *head;
	//creating a linked list
	head=NULL;
	head=(struct node *)malloc(sizeof(struct node));
	head->next=NULL;
	if(argc==1)
	{

	}
	else
	{
		strcpy(head->dir_name,argv[1]);
	}
	printf("%s\n",head->dir_name);
	initializeInotify(head);
	return 0;
}
