#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <fcntl.h>
#include "HT.h"
#include "BF.h"

int HT_CreateIndex( char *fileName,char attrType,char* attrName,int attrLength,int buckets){
	int i,j;
	void* block;
	eyrethrio e,p;	
	i=BF_CreateFile(fileName);                       //create file
	if (i < 0) {
		BF_PrintError("Error creating file");
		exit(EXIT_FAILURE);
	}
	e.hti.attrType=attrType;
	strcpy(e.hti.attrName,attrName);                    //make ht_info and hash_table
	e.hti.attrLength=attrLength;
	e.hti.numBuckets=buckets;
	for(j=0;j<buckets;j++){
		e.p[j]=0;
	}
	if ((e.hti.fileDesc = BF_OpenFile(fileName)) < 0) {     //get filedescriptor
		BF_PrintError("Error opening file");
	}
	if (BF_AllocateBlock(e.hti.fileDesc) < 0) {
		BF_PrintError("Error allocating block");        //allocate first block
	}
	if (BF_ReadBlock(e.hti.fileDesc, 0, &block) < 0) {
		BF_PrintError("Error getting block");
	}
	memcpy(block,&e,sizeof(eyrethrio));                    //assign everything from one struct
	if (BF_WriteBlock(e.hti.fileDesc, 0) < 0){
		BF_PrintError("Error writing block back");   //write block to memory
	}
	BF_CloseFile(e.hti.fileDesc);
	return i;
}

HT_info* HT_OpenIndex( char *fileName){
	int fd;
	eyrethrio *p;
	void* block;
	HT_info *hi;
	 p=(eyrethrio*)malloc(sizeof(eyrethrio));
	if ((fd = BF_OpenFile(fileName)) < 0) {
		BF_PrintError("Error opening file");
		return NULL;
	}
	if (BF_ReadBlock(fd, 0, &block) < 0) {
		BF_PrintError("Error getting block");
		return NULL;
	}
	memcpy(p,block,sizeof(eyrethrio));                 //allocate everything to one struct
	if (BF_WriteBlock(fd, 0) < 0){
		BF_PrintError("Error writing block back");
		return NULL;
	}
	return &p->hti;
}

int HT_CloseIndex( HT_info* header_info ){
	int i;
	i=BF_CloseFile(header_info->fileDesc) ;             //close file
	if (i< 0) {
		BF_PrintError("Error closing file");
	}
	return i;
}


int HT_InsertEntry( HT_info header_info,Record record){
    int blocksnum,i,err=-1,j;       //keeps track of available blocks
    void *lastblock;
    void *newblock,*block;
	blockstruct *re,*re2;
	eyrethrio *p;
	p=(eyrethrio*)malloc(sizeof(eyrethrio));
	re=(blockstruct*)malloc(sizeof(blockstruct));
	re2=(blockstruct*)malloc(sizeof(blockstruct));
	i=Hashfunction(record.id);
	i=i%(int)header_info.numBuckets;   //find position in hash table
    blocksnum = BF_GetBlockCounter(header_info.fileDesc);
    BF_ReadBlock(header_info.fileDesc,0,&lastblock);               //read block of hash_table ,, always 0
    int max_records=MAXRECORDS;//(BLOCK_SIZE - 3*sizeof(int))/sizeof(record);    //finding how many records per block
	memcpy(p,lastblock,sizeof(eyrethrio));                          //assign lastblock to struct eyrethrio
	if(p->p[i]==0){                                                 //if I haven't assigned block to i position of hash_table
		BF_AllocateBlock(header_info.fileDesc);
		blocksnum = BF_GetBlockCounter(header_info.fileDesc);          //allocate block
		p->p[i]=blocksnum-1;
		re->recordcounter=1;
		re->maxrecsum=MAXRECORDS;
		re->rec[0]=record;
		re->blockpointer=0;											//and write record in
		BF_ReadBlock(header_info.fileDesc,p->p[i],&newblock);        //assign struct blockstruct into newblock
		memcpy(newblock,re,sizeof(blockstruct));                     
		BF_WriteBlock(header_info.fileDesc,p->p[i]);
		err=p->p[i];
		memcpy(lastblock,p,sizeof(eyrethrio));
		if (BF_WriteBlock(header_info.fileDesc,0) < 0){               //write changed hash_table block back to memory
			BF_PrintError("Error writing block back");

		}
	}
	else{
		BF_ReadBlock(header_info.fileDesc,p->p[i],&newblock);
		memcpy(re,newblock,sizeof(blockstruct));                    //assign to a struct blockstruct all bytes of newblock
		if(re->recordcounter<re->maxrecsum){                           //if there is space for new record
			re->rec[re->recordcounter]=record;
			re->recordcounter++;
			memcpy(newblock,re,sizeof(blockstruct));
			BF_WriteBlock(header_info.fileDesc,p->p[i]);
			err=p->p[i];
		}
		else{				//if there is no space for new record
			if(re->blockpointer==0){	//if block doesn't point to next block
				BF_AllocateBlock(header_info.fileDesc);                 //allocate new block
				blocksnum = BF_GetBlockCounter(header_info.fileDesc);   //and put record to it
				re->blockpointer=blocksnum-1;
				re2->recordcounter=1;
				re2->maxrecsum=MAXRECORDS;
				re2->rec[0]=record;
				re2->blockpointer=0;
				BF_ReadBlock(header_info.fileDesc,re->blockpointer,&block);
				memcpy(block,re2,sizeof(blockstruct));
				BF_WriteBlock(header_info.fileDesc,re->blockpointer);
				err=re->blockpointer;
				memcpy(newblock,re,sizeof(blockstruct));
				BF_WriteBlock(header_info.fileDesc,p->p[i]);
			}
			else{					//if block points to new block
				j=re->blockpointer;	
				while(re->recordcounter>=re->maxrecsum && re->blockpointer!=0){         //find last block in the "list"
					j=re->blockpointer;
					BF_ReadBlock(header_info.fileDesc,re->blockpointer,&newblock);
					memcpy(re,newblock,sizeof(blockstruct));
				}
				if( re->recordcounter<re->maxrecsum){                       //if last block has room for more
					
					re->rec[re->recordcounter]=record;
					re->recordcounter++;
					memcpy(newblock,re,sizeof(blockstruct));
					BF_WriteBlock(header_info.fileDesc,j);
					err=j;
					
					
				}
				else{
				
					BF_AllocateBlock(header_info.fileDesc);  //else if last block doesn't have room for a record
					blocksnum = BF_GetBlockCounter(header_info.fileDesc);          //allocate new block
					re->blockpointer=blocksnum-1;
					re2->recordcounter=1;
					re2->maxrecsum=MAXRECORDS;
					re2->rec[0]=record;
					re2->blockpointer=0;
					BF_ReadBlock(header_info.fileDesc,re->blockpointer,&block);
					memcpy(block,re2,sizeof(blockstruct));
					BF_WriteBlock(header_info.fileDesc,re->blockpointer);
					err=re->blockpointer;
					memcpy(newblock,re,sizeof(blockstruct));
					BF_WriteBlock(header_info.fileDesc,j);
				}
			}
		}
	}
    return err;
}

int HT_DeleteEntry ( HT_info header_info,void * value){
	int i,hf,err=-1,j,pre,pre2,pre3;
	void* block,*block2,*block3;
	eyrethrio e;
	blockstruct *b,*b2;
	b=(blockstruct*)malloc(sizeof(blockstruct));
	b2=(blockstruct*)malloc(sizeof(blockstruct));
	memcpy(&i,value,sizeof(int));
	hf=Hashfunction(i);
	hf=hf%header_info.numBuckets;                             //find hash table position
	BF_ReadBlock(header_info.fileDesc,0,&block);
	memcpy(&e,block,sizeof(eyrethrio));
	if(e.p[hf]!=0){                                               //if there are blocks assigned
		BF_ReadBlock(header_info.fileDesc,e.p[hf],&block2);
		memcpy(b,block2,sizeof(blockstruct));
		for(j=0;j<b->recordcounter;j++){                       //check every position of record array of block
			if(i==b->rec[j].id){
				err++;                        //if i find it i have to find last record of last block
				b->rec[j].id=-1;             //pseudo delete
				strcpy(b->rec[j].name,"\0");
				strcpy(b->rec[j].surname,"\0");
				strcpy(b->rec[j].address,"\0");
				memcpy(block2,b,sizeof(blockstruct));
				BF_WriteBlock(header_info.fileDesc,e.p[hf]);
				return err;
			}
		}
		while(b->blockpointer!=0){  //the same
			pre2=b->blockpointer;  //but first search every block of list for value
			BF_ReadBlock(header_info.fileDesc,b->blockpointer,&block2);
			memcpy(b,block2,sizeof(blockstruct));
			for(j=0;j<b->recordcounter;j++){
				if(i==b->rec[j].id){
					err++;
					b->rec[j].id=-1;
					strcpy(b->rec[j].name,"\0");
					strcpy(b->rec[j].surname,"\0");
					strcpy(b->rec[j].address,"\0");
					memcpy(block2,b,sizeof(blockstruct));
					BF_WriteBlock(header_info.fileDesc,pre2);
					return err;
				}
			}
		}	
	}
	return err;
}
int HT_GetAllEntries(HT_info header_info,void *value){
	int i,hf,err=-1,j,flag=0;
	void* block,*block2,*newblock;
	eyrethrio *e;
	blockstruct *b,*re2;
	e=(eyrethrio*)malloc(sizeof(eyrethrio));
	b=(blockstruct*)malloc(sizeof(blockstruct));
	re2=(blockstruct*)malloc(sizeof(blockstruct));
	memcpy(&i,value,sizeof(int));
	hf=Hashfunction(i);                //in what bucket my value could be
	hf=hf%header_info.numBuckets;
	BF_ReadBlock(header_info.fileDesc,0,&block);
	memcpy(e,block,sizeof(eyrethrio));
	err++;
	if(e->p[hf]!=0){         //search the bucket
		BF_ReadBlock(header_info.fileDesc,e->p[hf],&block2);
		memcpy(b,block2,sizeof(blockstruct));
		err++;
		for(j=0;j<b->recordcounter;j++){   //check every record
			if(i==b->rec[j].id){
				flag++;
				printf("id: %d ,name: %s ,surname: %s, address: %s \n",b->rec[j].id,b->rec[j].name,b->rec[j].surname,b->rec[j].address);
				return err;
			}
		}
		while(b->blockpointer!=0){      //search in every block of bucket
			BF_ReadBlock(header_info.fileDesc,b->blockpointer,&block2);
			memcpy(b,block2,sizeof(blockstruct));
			err++;
			for(j=0;j<b->recordcounter;j++){
				if(i==b->rec[j].id){
					flag++;
					printf("id: %d ,name: %s ,surname: %s, address: %s\n",b->rec[j].id,b->rec[j].name,b->rec[j].surname,b->rec[j].address);return err;
				}
			}
		}	
	}
	if(flag==0){   //did not find it
		return -1;
	}
	return err;
}
int SHT_CreateSecondaryIndex( char *sfileName,char* attrName,int attrLength,int buckets,char* fileName)
{
	int i,j,k,pre;
	void* block,*block2,*block0;
	SecIndex e,p;
	HT_info* hi;
	SecondaryRecord sRecord;
	eyrethrio *e2;
	blockstruct *b;
	b=(blockstruct*)malloc(sizeof(blockstruct));
	e2=(eyrethrio*)malloc(sizeof(eyrethrio));
	i=BF_CreateFile(sfileName);
	if (i < 0) {
		BF_PrintError("Error creating file");
		exit(EXIT_FAILURE);
	}
	strcpy(e.shti.attrName,attrName);
	e.shti.attrLength=attrLength;
	e.shti.numBuckets=buckets;
	for(j=0;j<buckets;j++){
		e.p[j]=0;
	}
	strcpy(e.shti.fileName,fileName);
	if ((e.shti.fileDesc = BF_OpenFile(sfileName)) < 0) {
		BF_PrintError("Error opening file");
	}
	if (BF_AllocateBlock(e.shti.fileDesc) < 0) {
		BF_PrintError("Error allocating block");
	}
	if (BF_ReadBlock(e.shti.fileDesc, 0, &block) < 0) {
		BF_PrintError("Error getting block");
	}
	memcpy(block,&e,sizeof(SecIndex)); //allocate everything from one struct
	if (BF_WriteBlock(e.shti.fileDesc, 0) < 0){
		BF_PrintError("Error writing block back");
	}
	hi=(HT_info*)malloc(sizeof(HT_info));
	hi=HT_OpenIndex(fileName);
	BF_ReadBlock(hi->fileDesc, 0, &block0);
	memcpy(e2,block0,sizeof(eyrethrio));
	for(k=0;k<hi->numBuckets;k++){         //search the bucket
		BF_ReadBlock(hi->fileDesc,e2->p[k],&block2);
		memcpy(b,block2,sizeof(blockstruct));
		for(j=0;j<b->recordcounter;j++){   //check every record
			if(b->rec[j].id>=0){
				
				sRecord.record=b->rec[j];
				sRecord.blockId=e2->p[k];
				SHT_SecondaryInsertEntry(e.shti,sRecord);
			}
		}
		while(b->blockpointer!=0){      //search in every block of bucket
			pre=b->blockpointer;
			BF_ReadBlock(hi->fileDesc,b->blockpointer,&block2);
			memcpy(b,block2,sizeof(blockstruct));
			for(j=0;j<b->recordcounter;j++){
				if(b->rec[j].id>=0){
					sRecord.record=b->rec[j];
					sRecord.blockId=pre;
					SHT_SecondaryInsertEntry(e.shti,sRecord);
				}
			}
		}	
	}
	BF_CloseFile(e.shti.fileDesc);
	return i;
}
SHT_info* SHT_OpenSecondaryIndex( char *sfileName)
{
	int fd;
	SecIndex *p;
	void* block;
	SHT_info *shi;
	 p=(SecIndex*)malloc(sizeof(SecIndex));
	if ((fd = BF_OpenFile(sfileName)) < 0) {
		BF_PrintError("Error opening file");
		return NULL;
	}
	if (BF_ReadBlock(fd, 0, &block) < 0) {
		BF_PrintError("Error getting block");
		return NULL;
	}
	memcpy(p,block,sizeof(SecIndex));                 //allocate everything to one struct
	if (BF_WriteBlock(fd, 0) < 0){
		BF_PrintError("Error writing block back");
		return NULL;
	}
	
	return &p->shti;
}
int SHT_CloseSecondaryIndex( SHT_info* header_info)
{
	int i;
	i=BF_CloseFile(header_info->fileDesc) ;
	if (i< 0) {
		BF_PrintError("Error closing file");
	}
	return i;
}
int SHT_SecondaryInsertEntry( SHT_info header_info,SecondaryRecord record)
{
	int blocksnum,i,err=-1,j,fd,blockid,s;       //keeps track of available blocks
    void *lastblock;
    void *newblock,*block,*block56;
	HT_info* hi;
	secondaryblockstruct *re,*re2;
	SecIndex *p;
	p=(SecIndex*)malloc(sizeof(SecIndex));
	/*hi=(HT_info*)malloc(sizeof(HT_info));
	hi=HT_OpenIndex(header_info.fileName);         //search if it exists in primary
	s=HT_GetAllEntries(*hi,(void*)&record.record.id);
	if (s<0)
	{
		return err;
	}*/
	re=(secondaryblockstruct*)malloc(sizeof(secondaryblockstruct));
	re2=(secondaryblockstruct*)malloc(sizeof(secondaryblockstruct));
	i=Hashfunctionstring(record.record.name);
	i=i%(int)header_info.numBuckets;   //find position in hash table
    blocksnum = BF_GetBlockCounter(header_info.fileDesc);
    BF_ReadBlock(header_info.fileDesc,0,&lastblock);               //read block of hash_table ,, always 0
    int max_records=MAXRECORDS;//(BLOCK_SIZE - 3*sizeof(int))/sizeof(record);    //finding how many records per block
	memcpy(p,lastblock,sizeof(SecIndex));                          //assign lastblock to struct eyrethrio
	if(p->p[i]==0){                                                 //if I haven't assigned block to i position of hash_table
		BF_AllocateBlock(header_info.fileDesc);
		blocksnum = BF_GetBlockCounter(header_info.fileDesc);          //allocate block
		p->p[i]=blocksnum-1;
		re->recordcounter=1;
		re->maxrecsum=MAXRECORDS;
		strcpy(re->rec[0].name,record.record.name);
		re->rec[0].id=record.record.id;
		re->rec[0].blockId=record.blockId;
		re->blockpointer=0;											//and write record in
		BF_ReadBlock(header_info.fileDesc,p->p[i],&newblock);        //assign struct blockstruct into newblock
		memcpy(newblock,re,sizeof(secondaryblockstruct));                     
		BF_WriteBlock(header_info.fileDesc,p->p[i]);
		err=p->p[i];
		memcpy(lastblock,p,sizeof(SecIndex));
		if (BF_WriteBlock(header_info.fileDesc,0) < 0){               //write changed hash_table block back to memory
			BF_PrintError("Error writing block back");

		}
	}
	else{
		BF_ReadBlock(header_info.fileDesc,p->p[i],&newblock);
		memcpy(re,newblock,sizeof(secondaryblockstruct));                    //assign to a struct blockstruct all bytes of newblock
		if(re->recordcounter<re->maxrecsum){                           //if there is space for new record
			strcpy(re->rec[re->recordcounter].name,record.record.name);
			re->rec[re->recordcounter].id=record.record.id;
			re->rec[re->recordcounter].blockId=record.blockId;
			re->recordcounter++;
			memcpy(newblock,re,sizeof(secondaryblockstruct));
			BF_WriteBlock(header_info.fileDesc,p->p[i]);
			err=p->p[i];
		}
		else{				//if there is no space for new record
			if(re->blockpointer==0){	//if block doesn't point to next block
				BF_AllocateBlock(header_info.fileDesc);                 //allocate new block
				blocksnum = BF_GetBlockCounter(header_info.fileDesc);   //and put record to it
				re->blockpointer=blocksnum-1;
				re2->recordcounter=1;
				re2->maxrecsum=MAXRECORDS;
				strcpy(re2->rec[0].name,record.record.name);
				re2->rec[0].id=record.record.id;
				re2->rec[0].blockId=record.blockId;
				re2->blockpointer=0;
				BF_ReadBlock(header_info.fileDesc,re->blockpointer,&block);
				memcpy(block,re2,sizeof(secondaryblockstruct));
				BF_WriteBlock(header_info.fileDesc,re->blockpointer);
				err=re->blockpointer;
				memcpy(newblock,re,sizeof(secondaryblockstruct));
				BF_WriteBlock(header_info.fileDesc,p->p[i]);
			}
			else{					//if block points to new block
				j=re->blockpointer;	
				while(re->recordcounter>=re->maxrecsum && re->blockpointer!=0){         //find last block in the "list"
					j=re->blockpointer;
					BF_ReadBlock(header_info.fileDesc,re->blockpointer,&newblock);

					memcpy(re,newblock,sizeof(secondaryblockstruct));
				}
				if( re->recordcounter<re->maxrecsum){                       //if last block has room for more
					
					strcpy(re->rec[re->recordcounter].name,record.record.name);
					re->rec[re->recordcounter].id=record.record.id;
					re->rec[re->recordcounter].blockId=record.blockId;
					re->recordcounter++;
					memcpy(newblock,re,sizeof(secondaryblockstruct));
					BF_WriteBlock(header_info.fileDesc,j);
					err=j;
					
				}
				else{
				
					BF_AllocateBlock(header_info.fileDesc);  //else if last block doesn't have room for a record
					blocksnum = BF_GetBlockCounter(header_info.fileDesc);          //allocate new block
					re->blockpointer=blocksnum-1;
					re2->recordcounter=1;
					re2->maxrecsum=MAXRECORDS;
					strcpy(re2->rec[0].name,record.record.name);
					re2->rec[0].id=record.record.id;
					re2->rec[0].blockId=record.blockId;
					re2->blockpointer=0;
					BF_ReadBlock(header_info.fileDesc,re->blockpointer,&block);
					memcpy(block,re2,sizeof(secondaryblockstruct));
					BF_WriteBlock(header_info.fileDesc,re->blockpointer);
					err=re->blockpointer;
					memcpy(newblock,re,sizeof(secondaryblockstruct));
					BF_WriteBlock(header_info.fileDesc,j);
				}
			}
		}
	}
    return err;
}
int SHT_SecondaryGetAllEntries(SHT_info header_info_sht,HT_info header_info_ht,void *value){
	int hf,err=-1,j,flag=0,k;
	void* block,*block2,*newblock,*block3,*block4;
	char i[50];
	SecIndex *e;
	secondaryblockstruct *b,*re2;
	blockstruct *b2;
	e=(SecIndex*)malloc(sizeof(SecIndex));
	b=(secondaryblockstruct*)malloc(sizeof(secondaryblockstruct));
	b2=(blockstruct*)malloc(sizeof(blockstruct));
	re2=(secondaryblockstruct*)malloc(sizeof(secondaryblockstruct));
	memcpy(&i,value,50*sizeof(char));
	hf=Hashfunctionstring(i);                //in what bucket my value could be
	hf=hf%header_info_sht.numBuckets;
	BF_ReadBlock(header_info_sht.fileDesc,0,&block);
	memcpy(e,block,sizeof(SecIndex));
	err++;
	if(e->p[hf]!=0){         //search the bucket
		BF_ReadBlock(header_info_sht.fileDesc,e->p[hf],&block2);
		memcpy(b,block2,sizeof(secondaryblockstruct));
		err++;
		for(j=0;j<b->recordcounter;j++){   //check every record
			if(strcmp(i,b->rec[j].name)==0){
				BF_ReadBlock(header_info_ht.fileDesc,b->rec[j].blockId,&block3);
				memcpy(b2,block3,sizeof(blockstruct));
				err++;
				for(k=0;k<b2->recordcounter;k++){   //check every record
					if(b2->rec[k].id==b->rec[j].id){
						flag++;
						printf("id: %d ,name: %s ,surname: %s, address: %s \n",b2->rec[k].id,b2->rec[k].name,b2->rec[k].surname,b2->rec[k].address);
					}
				}
			}
		}
		while(b->blockpointer!=0){      //search in every block of bucket
			BF_ReadBlock(header_info_sht.fileDesc,b->blockpointer,&block2);
			memcpy(b,block2,sizeof(secondaryblockstruct));
			err++;
			for(j=0;j<b->recordcounter;j++){
				if(strcmp(i,b->rec[j].name)==0){
					BF_ReadBlock(header_info_ht.fileDesc,b->rec[j].blockId,&block4);
					memcpy(b2,block4,sizeof(blockstruct));
					err++;
					for(k=0;k<b2->recordcounter;k++){
						if(b2->rec[k].id==b->rec[j].id){
							flag++;
							printf("id: %d ,name: %s ,surname: %s, address: %s\n",b2->rec[k].id,b2->rec[k].name,b2->rec[k].surname,b2->rec[k].address);
						}
					}
				}
			}
		}	
	}
	if(flag==0){   //did not find it
		return -1;
	}
	return err;
}

int HashStatistics( char* filename){
	HT_info* hi;
	void *block,*block2,*testblock;
	eyrethrio p;
	blockstruct *b;
	FILE* f;
	int blocknum=0,minrecord=9999999,maxrecord=0,records=0,i,j,sz,fd;
	double morecord=0,moblock=0;
	int largebuckets=0,*largebucket;
	hi=HT_OpenIndex(filename);
	b=(blockstruct*)malloc(sizeof(blockstruct));
	largebucket=(int*)malloc(hi->numBuckets*sizeof(int));
	for(i=0;i<hi->numBuckets;i++){
		largebucket[i]=0;
	}
	blocknum=BF_GetBlockCounter(hi->fileDesc);               //number of blocks
	if (BF_ReadBlock(hi->fileDesc, 0, &block) < 0) {
		BF_PrintError("Error getting block");
		return -1;
	}
	memcpy(&p,block,sizeof(eyrethrio));
	for(i=0;i<hi->numBuckets;i++){
		if(p.p[i]==0){
			
		}
		else{
			records=0;
			if (BF_ReadBlock(hi->fileDesc, p.p[i], &block2) < 0) {
				BF_PrintError("Error getting block");
				return -1;
			}
			memcpy(b,block2,sizeof(blockstruct));
			for(j=0;j<b->recordcounter;j++){
				if(b->rec[j].id!=-1){
					records++;
				}
			}
			while(b->blockpointer!=0){
				largebucket[i]++;
				if (BF_ReadBlock(hi->fileDesc, b->blockpointer, &block2) < 0) {
					BF_PrintError("Error getting block");
					return -1;
				}
				memcpy(b,block2,sizeof(blockstruct));
				for(j=0;j<b->recordcounter;j++){
					if(b->rec[j].id!=-1){
						records++;
					}
				}
			}
			morecord+=(double)records;
			if(minrecord>records){
				minrecord=records;
			}
			if(maxrecord<records){
				maxrecord=records;
			}
		}
	}
	morecord=(double)morecord/hi->numBuckets;
	moblock=(double)(blocknum-1)/hi->numBuckets;
	
	printf("File has %d blocks. \n",blocknum);
	printf("The minimum of records in a bucket is %d, the maximum is %d and the average %lf.\n",minrecord,maxrecord,morecord);
	printf("Average of blocks in every bucket = %lf\n",moblock);
	for(i=0;i<hi->numBuckets;i++){
		if(largebucket[i]!=0){
			largebuckets++;
		}
		printf("Bucket %d has %d full blocks.\n",i,largebucket[i]);
	}
	printf("Number of buckets that have filled up blocks are %d.\n",largebuckets);
	return 0;
}
int Hashfunction(int i){
	int a=11;
	int b=23;
	long int p=  16769023;
	return (a*i+b)%p;
}

int Hashfunctionstring(char* i){
	int a=33;
	int b=5381;
	long int p=16769023;
	for(int j=0;j<strlen(i);j++){
		b = ((b*a) + i[j])%p;
	}
	return b;
}

