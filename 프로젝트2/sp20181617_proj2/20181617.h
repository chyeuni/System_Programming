#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

void initmemory();					//메모리 초기화
void gethash();						//opcode.txt 파일에서 데이터 읽기
void addhash(int, char*, char*);	//hashtable 만들기
void option(char*);					//help,dir,quit,history 명령어처리
void option2(char*);				//dump,edit,fill 명령어처리
void option3(char*);				//reset,opcode,opcodelist 명령어처리
void option4(char*);				//type, assemble, symbol명령어 처리
void symbol_();						//symbol table파일을 읽는 함수 symbol 명령어 처리
int assemble_(char*);				//asm파일 이름을 저장하는 함수 assemble 명령어 처리
void makeobject(char*);				//obj파일 생성
void makelst(char*);				//lst파일 생성
void makesymbol();					//symbol파일 생성
int sourcelist(char*);				//asm파일 linkedlist 저장
void addsourcelist(int, char*, int,int,char*, char*,char*,char*, int);	//linkedlist에 add
int searchsource(char*);			//올바른 mnemonic인지 판단
int pass1();						//패스1
void change(char*, int);			//16진수로 변환
int pass2();						//패스2
int symbolcheck(char*);				//symboltable에 symbol있는지 확인
void insertsymbol(char* , int);		//symboltable에 symbol 저장
int type_(char*);					//type명령
void printhash();					//hashtable에 있는 모든 데이터 출력(opcodelist명령어)
int searchhash(char*);				//hashtable에 해당 데이터가 있는지 검색(opcode명령어)
int check(char);					//16진수로 표현되었는지 검사
void dump_(int, int);				//메모리공간 출력(dump명령어)
void edit_(int, int);				//메모리공간 변경(edit명령어)
void fill_(int, int, int);			//메모리공간 변경(fill명령어)
void dirent_();						//현재디렉터리 내 파일 출력(dir명령어)
void addlist(char*);				//입력된 올바른 명령어 저장
void printlist();					//addlist로 linkedlist에 저장되어 있는 모든 명령어 출력(history명령어)


enum instruction { help, dir, quit, history, dump, edit, fill, reset, opcode, opcodelist ,assemble, type, symbol};	//명령어 열거형
enum reg {A, X, L ,B, S, T, F, PC=8, SW};
char* inst_[] = { "help", "h", "dir", "d", "quit", "q", "history", "hi", "dump", "du", "edit", "e", "fill", "f", "reset","opcode", "opcodelist" ,"assemble", "type", "symbol"}; //입력될수 있는 모든 명령어 형태
char* source_[]={"START", "BASE", "END"};
//char* register_[]={"A", "X", "L", "PC", "SW", "B", "S", "T", "F"};

typedef struct NODE {
	int num;
	char data[50];
	struct NODE* next;
}node;				//history명령어 처리를 위한 linkedlist의 node
node* head = NULL; 	//node의 head

typedef struct NODE_ {
	int value;
	char ins[50];
	char n[50];
	struct NODE_* next;
}node_;							//hashtable을 구성하는 node_
node_* table[20] = { NULL, };	//hashtable의 사이즈 20설정

typedef struct NODE_2 {
	int type;					//주석문이면 2 아니면 1
	char comment[100];			//주석문 저장
	int len;					//op길이
	int num;					//행
	int loc;					//loc
	char op[8];					//목적코드
	char s[4][10];				//asm파일 나눠서 저장
	struct NODE_2* next;
}snode;							//asm파일의 linkedlist의 node		
snode* shead = NULL;			//snode의 head

typedef struct NODE_3{
	char name[10];
	int loc;
	struct NODE_3* next;		//symbol파일의 linkedlist의 노드
}symnode;
symnode* symhead=NULL;			//symnode의 symhead

int memory[65536][16] = { 0, };	//가상메모리공간 선언
int  address[65536] = { 0, };	//가상메모리공간의 주소 선언
int dumpstart = 0;				//dump명령어 사용시 마지막 출력된 주소번지+1 저장
int locctr = 0;					//loc저장
int startn=0;					//프로그램시작주소
char startname[10]={'\0'};		//프로그램이름
int tempop=0;					//명령어의 opcode
char filenames[100][20]={'\0',};//생성된 파일이름저장
int filenums=0;					//생성된 파일갯수
int fileend;					//프로그램 끝주소
int mod[100];					//modification record 값
int modnum=0;					//modification record 갯수
