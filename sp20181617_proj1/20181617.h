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
void printhash();					//hashtable에 있는 모든 데이터 출력(opcodelist명령어)
int searchhash(char*);				//hashtable에 해당 데이터가 있는지 검색(opcode명령어)
int check(char);					//16진수로 표현되었는지 검사
void dump_(int, int);				//메모리공간 출력(dump명령어)
void edit_(int, int);				//메모리공간 변경(edit명령어)
void fill_(int, int, int);			//메모리공간 변경(fill명령어)
void dirent_();						//현재디렉터리 내 파일 출력(dir명령어)
void addlist(char*);				//입력된 올바른 명령어 저장
void printlist();					//addlist로 linkedlist에 저장되어 있는 모든 명령어 출력(history명령어)


enum instruction { help, dir, quit, history, dump, edit, fill, reset, opcode, opcodelist };	//명령어 열거형
char* inst_[] = { "help", "h", "dir", "d", "quit", "q", "history", "hi", "dump", "du", "edit", "e", "fill", "f", "reset","opcode", "opcodelist" }; //입력될수 있는 모든 명령어 형태

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

int memory[65536][16] = { 0, };	//가상메모리공간 선언
int  address[65536] = { 0, };	//가상메모리공간의 주소 선언
int dumpstart = 0;				//dump명령어 사용시 마지막 출력된 주소번지+1 저장
