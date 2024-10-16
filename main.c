#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/types.h>
 		#include <unistd.h>
#include "BF.h"
#include "HT.h"


int main(char argc,char** argv)
{
	FILE* f;
	Record test;//={2,"fred","fucks","add"};
	int i;
	char x[100],y[100],z[50];

	char* fileName="primary.index";
	char attrType='i';
	char* attrName="id";
	int attrLength=4;
	int buckets=10;
	char* sfileName="secondary.index";
	char sAttrType='c';
	char* sAttrName="name";
	int sAttrLength=20;
	int sBuckets=10;
	int blockid[100000];
	HT_info* hi;
	int a=10,b=2,c=3,d=4,e=7,g=10;
	BF_Init();
	HT_CreateIndex(fileName,attrType,attrName,attrLength,buckets);
	hi=HT_OpenIndex(fileName);
	f=fopen(argv[1], "r");
	for(i=0;i<a;i++){
		//fgets(x, 2, f);
		if(fgets(x, 2, f)==NULL){
			break;
		}
		fgets(x,b,f);
		test.id=atoi(x);
		fgets(x, 3, f);
		fgets(x, e, f);
		strcpy(test.name,x);
		fgets(x, 4, f);
		fgets(x, g, f);
		strcpy(test.surname,x);
		fgets(x, 4, f);
		fgets(x, g, f);
		strcpy(test.address,x);
		fgets(x, 4, f);
		blockid[i]=HT_InsertEntry( *hi,test );
		if(i==9 || i==99 || i==999 || i==9999){
			a=a*10;
			b++;
			e++;
			g++;
		}
	}
	fclose(f);
	for (i=0;i<1000*2;i++)
	{
		Record record;
		record.id=i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.address,"address_%d",i);
		int err=HT_GetAllEntries(*hi,(void*)&record.id);
	}
	SHT_CreateSecondaryIndex(sfileName,sAttrName,sAttrLength,sBuckets,fileName);
	SHT_info* shi=SHT_OpenSecondaryIndex(sfileName);
	SecondaryRecord test2;
	f=fopen(argv[1], "r");
	for(i=0;i<a;i++){
		//fgets(x, 2, f);
		if(fgets(x, 2, f)==NULL){
			break;
		}
		fgets(x,b,f);
		test2.record.id=atoi(x);
		fgets(x, 3, f);
		fgets(x, e, f);
		strcpy(test2.record.name,x);
		fgets(x, 4, f);
		fgets(x, g, f);
		strcpy(test2.record.surname,x);
		fgets(x, 4, f);
		fgets(x, g, f);
		strcpy(test2.record.address,x);
		fgets(x, 4, f);
		test2.blockId=blockid[i];
		SHT_SecondaryInsertEntry(*shi,test2);
		if(i==9 || i==99 || i==999 || i==9999){
			a=a*10;
			b++;
			e++;
			g++;
		}
	}
	fclose(f);
		for (int i=0;i<1000*2;i++)
	{
		Record record;
		record.id=i;
		sprintf(record.name,"name_%d",i);
		sprintf(record.surname,"surname_%d",i);
		sprintf(record.address,"address_%d",i);
		SHT_SecondaryGetAllEntries(*shi,*hi,(void*)record.name);
	}
		SHT_CloseSecondaryIndex(shi);
		HT_CloseIndex(hi);
		HashStatistics(fileName);
    		
}
