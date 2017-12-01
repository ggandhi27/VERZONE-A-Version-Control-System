#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int lcs(char *a,char *b,int m,int n,int q,int w)
{
        int i,j,k=0;
	for(j=m-1;j>=0;j--)
               	for(i=n-1;i>=0;i--){
                       	if(*(a+j)==*(b+i)){
                               	n--;
                               	j--;
                       	}
                        else{
				return 1;
			}
                }
	return 0;
}

void diff(char *file1, char *file2,char *file3){
	FILE *fp1=fopen(file1,"r");
        FILE *fp2=fopen(file2,"r");
        int count1=0,count2=0,temp;
        char *str1=(char*)malloc(100);
        char *str2=(char*)malloc(100);
        int m,n,i=0,j;
        while(feof(fp1)==0){
                fgets(str1,100,fp1);
                count1++;
        }
        while(feof(fp2)==0){
                fgets(str2,100,fp2);
                count2++;
        }
        FILE *fp=fopen(file3,"w");
        rewind(fp1);    rewind(fp2);
        for(;i<count1-1;i++){
                fgets(str1,100,fp1);
                m=strlen(str1);
                for(j=0;j<count2;j++){
                        fgets(str2,100,fp2);
                        n=strlen(str2);
                        temp=lcs(str1,str2,m,n,i,j);
                        if(temp==0)
                                break;
                }
                rewind(fp2);
                if(temp==1){
                        fprintf(fp,"File 1 : Deleted line no. - %d :",i+1);
                        fputs(str1,fp);
                }
        }

        rewind(fp1);    rewind(fp2);
        for(i=0;i<count1;i++){
                fgets(str2,100,fp2);
                m=strlen(str2);
                for(j=0;j<count2;j++){
                        fgets(str1,100,fp1);
                        n=strlen(str1);
                        temp=lcs(str1,str2,m,n,i,j);
			if(temp==0)
                                break;
                }
                rewind(fp1);
                if(temp==1){
                        fprintf(fp,"File 2 : Added line no. - %d :",i+1);
                        fputs(str2,fp);
                }
        }
        free(str1);
        free(str2);
        fclose(fp);
        fclose(fp1);
        fclose(fp2);
}

