#define MAXRECORDS 6


typedef struct{
	int id;
	char name[15];
	char surname[20];
	char address[40];
}Record;


typedef struct{
	int recordcounter;
	Record rec[MAXRECORDS];
	int maxrecsum;
	int blockpointer;
}blockstruct;

typedef struct {
	int fileDesc;
	char attrType; 
	char attrName[20];
	int attrLength;
	long int numBuckets;
} HT_info;

typedef struct{
	char name[20];
	int id;
	int blockId; 
}mySecondaryRecord;

typedef struct{
	int recordcounter;
	mySecondaryRecord rec[MAXRECORDS];
	int maxrecsum;
	int blockpointer;
}secondaryblockstruct;



typedef struct{
	Record record;
	int blockId; 
}SecondaryRecord;

typedef struct {
	int fileDesc;
	char attrName[20];
	int attrLength;
	long int numBuckets;
	char fileName[20];
}SHT_info;

typedef struct{
	HT_info hti;
	int p[60];
}eyrethrio;

typedef struct{
	SHT_info shti;
	int p[60];
}SecIndex;

int HT_CreateIndex(char*,char,char*,int,int);
HT_info* HT_OpenIndex(char*);
int HT_CloseIndex(HT_info*);
int HT_InsertEntry( HT_info,Record );
int HT_DeleteEntry ( HT_info ,void *);
int HT_GetAllEntries(HT_info ,void*);
int SHT_CreateSecondaryIndex( char *,char* ,int ,int ,char* );
SHT_info* SHT_OpenSecondaryIndex(char*);
int SHT_CloseSecondaryIndex(SHT_info*);
int SHT_SecondaryInsertEntry(SHT_info,SecondaryRecord );
int SHT_SecondaryGetAllEntries(SHT_info,HT_info ,void *);

int HashStatistics( char* );
int Hashfunction(int);
int Hashfunctionstring(char* );

