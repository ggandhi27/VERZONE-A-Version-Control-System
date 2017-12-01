#include<stdio.h>
#include<string.h>
#include<stdlib.h>

char *pos(char *str1){
        int c=0,i;
        for(i=0;str1[i]!='\0';i++){
                if(str1[i]==':'){
                        c++;
                }
                if(c==2)
                        break;
        }
        return str1+i+1;

}
int linenum(char *str)
{
        char c,d;
        int n=0;
        c=*(str+26);
        d=*(str+27);
        if(d==' '){
                return c-48;
        }
        else{
                n=c-48;
                n*=10;
                n+=d-48;
				 }
        return n;
}

void patch(char *f1,char*f2){
        FILE *src=fopen(f1,"r");
        FILE *pth=fopen(f2,"r");
        char *f3=(char*)malloc(sizeof(char)*50);
	strcpy(f3,".tempFile.txt");
        FILE *dst=fopen(f3,"w");
        char *str1=(char*)malloc(sizeof(src));
        char *str2=(char*)malloc(sizeof(pth));
        char *str3=(char*)malloc(sizeof(char)*200);
        int c,d,i=1,j=0,len,arr[50];
        while(!feof(pth)){
                fgets(str2,200,pth);
                arr[j]=ftell(pth);
                j++;
        }
        rewind(pth);
        j=0;
        while(!feof(src)){
                fgets(str1,100,src);
                fgets(str2,200,pth);
                printf("%s\n",str2);
                c=linenum(str2);
                printf("%d\n",c);
                if(c==i){
                        if(str2[9]=='A'){
                                fputs(pos(str2),dst);
                                j++;
                        }
                        else if(str2[9]=='D'){
                                continue;
                        }
                }
                else{
                        fputs(str1,dst);
                        fseek(pth,arr[j],SEEK_SET);
                }
                i++;
        }
		fclose(src);
        fclose(pth);
        fclose(dst);
	char cmd[8192];
	sprintf(cmd,"mv %s %s",f3,f1);
	system(cmd);
        free(f3);
}
