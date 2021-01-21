#include "20181617.h"

int main (){
	initmemory();						//시작하고 바로 메모리공간 초기화
	gethash();							//opcode hashtable 만들기
    for(;;){
	char input[50];	
	printf("sicsim> ");
	fgets(input, 50, stdin);			//shell명령어 입력받기

	char *p;
	if((p=strchr(input, '\n'))!=NULL)
		*p='\0';						//입력받은 명령어에서 엔터값은 삭제
	option(input);						//option함수  호출
    }
	
    return 0;
}

void initmemory(){						//메모리 초기화하는 함수
	int i,j;
	for(i=0;i<65535;i++){
		address[i+1]=address[i]+16;		//주소는 16씩 증가
	}

	for(i=0; i<65536; i++){
		for(j=0; j<16; j++){
			memory[i][j]=0;				//메모리는 0으로 모두 초기화
		}
	}
}

void gethash(){									//파일에서 데이터 읽는 함수
	FILE *fp = fopen("opcode.txt","r"); 		//"opcode.txt"파일 열기
	int value;
	char value2[5];
	char ins[50];
	char n[50];
	fscanf(fp, "%s %s %s",value2, ins, n); 		//value2, ins, n순서로 한줄씩 읽기
	while(!feof(fp)){							//파일끝까지 반복
		value=strtol(value2, NULL, 16);			//string의 value2를 16진수 int로 변경
		addhash(value, ins, n);					//읽은 데이터로  addhash함수 호출
		fscanf(fp, "%s %s %s",value2, ins, n);
	}
	return;
}

void addhash(int value, char* ins, char*n ){	//hashtable 만드는 함수
	int order=ins[0]%20;						//명령어의 첫알파벳의 ASCII코드를 20으로 나눈 나머지 값으로 table에 넣는다.
	node_*new=(node_*)malloc(sizeof(node_));	//node 할당
	new->value=value;
	strcpy(new->ins, ins);
	strcpy(new->n, n);
	new->next=NULL;
	if(table[order]==NULL){						//table을 linkedlist로 구성하는데 만약 head가 비어있다면 바로 node를 넣는다.
		table[order]=new;
	}
	else{
		node_* temp=table[order];				//table의 linkedlist head가 이미  있다면 가장 마지막 부분에 node 연결
		while(temp->next!=NULL)	
			temp=temp->next;
		temp->next=new;
	}
	return;
}

void option (char* input){						//help, dir, quit, history 명령어를 처리하는 함수
	char input2[50];
	strcpy(input2, input);
	int i;
	char *array;
	array=strtok(input2, " ");					//명령어 뒤에 공백이 들어올 경우 공백 앞까지 잘라준다.

	for(i=0; i<8; i++){
		if(strcmp(array, inst_[i])==0)
			break;								//입력받은 명령어가 무슨 명령어인지 판단
	}
	int i2=i/2;
    switch (i2){								//switch로 명령어 구분
		case help:								//help 명령어
			printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\n");
			addlist(input);						//올바른 명령어이므로 명령어 history list에 추가
	    	break;
		case dir:								//dir 명령어
			dirent_();							//dirent_함수 호출
			addlist(input);						//올바른 명령어이므로 명령어 history list에 추가
	    	break;
		case quit:								//quit 명령어

			exit(0);							//프로그램 종료					
	    	break;
		case history: 							//history 명령어
			addlist(input);						//올바른 명령어이므로 명령어 history list에 추가
			printlist();						//printlist함수 호출
	    	break;
	default:									//해당하지 않는 경우
	    option2(input);							//option2함수 호출
		break;	
    }
    return;
}

void option2(char* input){						//dump, edit, fill명령어 처리
	char array[4][50]={'\0', };					//명령어가 ,로 구분될수 있기 때문에 최대 4개로 구분하기 위함
	int len = strlen(input);
	int i=0; int j=0; int k=0;
	int c=0;

	for(i=0; i<len; i++){
		if(input[i]==' '){						//명령어에서 공백이 들어올경우 모든 공백은 인식안한다.
			if(input[i+1]!=' '){
				j++; k=0;
			}
		}
		else{
			if(input[i]==','){					// , 가 들어온 경우 명령어 구분한다. 다음  array에 값 저장
				j++; i++; k=0; c++;
				if(c==3){						// , 가 3개이상 들어온 경우 ERROR 처리
					printf("ERROR!\n");
					return;
				}
			}
			else{
				if(((input[i]>='0')&&(input[i]<='9'))||((input[i]>='a')&&(input[i]<='z'))||((input[i]>='A')&&(input[i]<='Z'))){	//명령어가 숫자, 알파벳만 있어야 된다.
					array[j][k]=input[i];		
					k++;
				}
				else{
					//숫자 알파벳이 아닌경우 ERROR처리
					return;
				}
			}
		}		
	}

	int start, end, address, value;

	for(i=8; i<14; i++){
		if(strcmp(array[0], inst_[i])==0)				//입력받은 명령어가 무슨 명령어인지 판단
			break;	
	}
	int i2=i/2;
	switch(i2){											//switch로 명령어 구분
		case dump:										//dump명령
			if(array[3][0]!='\0'){
				printf("ERROR\n");						//인자가 3개 들어온경우 ERROR처리
				return;
			}
			if (array[1][0]!='\0'){						//start가 입력되었을 때
				int len1=strlen(array[1]);
				for(j=0; j<len1; j++){
					if(check(array[1][j])==-1){			//16진수로 표현안되었을 경우 ERROR처리
						printf("ERROR!\n");
						return;
					}
				}
				start=strtol(array[1], NULL, 16);		//start 16진수로 바꿔서 int로 저장

				if((start<0)||(start>1048575)){			//주소 범위를 벗어난 경우 ERROR처리
					printf("ERROR!\n");
					return;
				}

				if(array[2][0]!='\0'){					//end가 입력되었을 때
					int len2=strlen(array[2]);
					for(j=0; j<len2; j++){
						if(check(array[2][j])==-1){		//16진수로 표현안되었을 경우 ERROR처리
							printf("ERROR!\n");
							return;
						}
					}
					end=strtol(array[2], NULL, 16);		//end 16진수로 바꿔서 int로 저장
					if((start>end)||(end>1048575)){		//주소범위를 벗어나거나 start보다 작을 때 ERROR처리
						printf("ERROR!\n");
						return;
					}
				}
				else
					end = start+159;					//start만 입력되었을 경우 end값 설정
			}
			else{ 										//start, end모두 입력되지 않았을 때
				if (dumpstart==1048576)					//이전에 메모리 끝까지 출력했을 경우 처음부터 다시 출력 start=0으로 설정
					start=0;
				else
					start = dumpstart; 					//메모리끝까지 출력안했을 때는 그 다음부터 출력
				end = start+159;						//end값 설정
			}

			if(end>=1048576)							//end가 주소범위보다 클때 끝까지만 출력
				end = 1048575;
			dump_(start, end);							//dump_함수 호출
			dumpstart=end+1;							//출력한 마지막 주소 저장
			addlist(input);								//올바른 명령어이므로 history linkedlist에 저장
			break;
		case edit:										//edit명령어
			if(array[3][0]!='\0'){						//인자가 3개들어온 경우 ERROR처리
				printf("ERROR!\n");
				return;
			}
			if(array[1][0]!='\0'){
				int len1=strlen(array[1]);	
				for(j=0; j<len1; j++){
					if(check(array[1][j])==-1){			//16진수로 표현안되었을 경우 ERROR처리
						printf("ERROR!\n");
						return;
					}
				}
				address=strtol(array[1],NULL, 16);		//address 16진수로 바꿔서 int로 저장
				if((address<0)||(address>1048575)){		//주소범위를 벗어났을 경우 ERROR처리
					printf("ERROR!\n");
					return;
				}

				if(array[2][0]!='\0'){
					int len2=strlen(array[2]);
					for(j=0; j<len2; j++){
						if(check(array[2][j])==-1){		//16진수로 표현안되었을 경우 ERROR처리
							printf("ERROR!\n");
							return;
						}
					}
					value=strtol(array[2],NULL, 16);	//value 16진수로 바꿔서 int로 저장
					if((value<0)||(value>4095)){		//0~ff범위 벗어난 경우 ERROR처리
						printf("ERROR!\n");
						return;			
					}
				}
				else return;							//인자 2개아닌 경우 ERROR처리
			}
			else return;								//인자 2개아닌 경우 ERROR처리
			
			edit_(address,value);						//edit_함수 호출
			addlist(input);								//올바른 명령어이므로 history linkedlist 추가
			break;
		case fill:										//fill명령어
			if(array[1][0]!='\0'){
				int len1=strlen(array[1]);
				for(j=0; j<len1; j++){
					if(check(array[1][j]==-1)){			//16진수로 표현 안되었을 경우 ERROR처리
						printf("ERROR!\n");
						return;
					}
					start=strtol(array[1], NULL, 16);	//start 16진수로 바꿔서 int로 저장
					
					if((start<0)||(start>1048575)){		//주소 범위 벗어난 경우 ERROR처리
						printf("ERROR!\n");
						return;
					}

					if(array[2][0]!='\0'){
						int len2=strlen(array[2]);
						for(j=0; j<len2; j++){
							if(check(array[2][j])==-1){	//end 16진수로 바꿔서 int로 저장
								printf("ERROR!\n");
								return;
							}
						}
						end=strtol(array[2], NULL, 16);	//end 16진수로 바꿔서 int로 저장
						if((start>end)||(end>1048575)){	//주소범위 벗어났거나 start보다 작으면 ERROR처리
							printf("ERROR!\n");
							return;
						}
						
						if(array[3][0]!='\0'){			
							int len3=strlen(array[3]);
							for(j=0; j<len3; j++){
								if(check(array[3][j])==-1){	//16진수로 표현안되었을 경우 ERROR처리
									printf("ERROR\n");
									return;
								}
							}
							value=strtol(array[3],NULL, 16)	;//value 16진수로 바꿔서 int로 저장
							if((value<0)||(value>4095)){	//0~ff범위 벗어난 경우 ERROR처리
								printf("ERROR!\n");
								return;
							}
						}
						else return;						//인자 3개 아닌 경우 ERROR 처리
					}
					else return;							//인자 3개 아닌 경우 ERROR처리
				}
			}
			else return;									//인자 3개 아닌 경우 ERROR처리
			fill_(start, end, value);						//fill_함수 호출
			addlist(input);									//올바른명령어이므로 history linkedlist에 추가
			break;
		default:											//해당하지않는 경우
			option3(input);									//option3함수 호출
			break;
	}
	return;
}

void option3(char* input){						//reset, opcode, opcodelist 명령어 처리
	char array[2][50]={'\0', };
	int len=strlen(input);
	int i=0; int j=0; int k=0;
	int result;
	for(i=0; i<len; i++){						//opcode명령어 경우 공백으로 구분되어 input이 들어오므로 각각 구분해서 array에 저장한다.
		if(input[i]==' '){
			if(input[i+1]!=' ')
				j++; k=0;
		}
		else{
			array[j][k]=input[i];
			k++;
		}
	}

	for(i=14; i<17; i++){
		if(strcmp(array[0], inst_[i])==0)		//입력받은 명령어가 무슨 명령어인지 판단
			break;
	}
	i=i-7;
	switch(i){									//switch로 명령어 구분
		case reset:								//reset명령어
			initmemory();						//initmemory함수 호출
			addlist(input);						//올바른 명령어이므로 history linkedlist에  추가
			break;
		case opcode:							//opcode명령어
			result= searchhash(array[1]);		//searchhash함수 호출후 return 값 저장
			if(result==0)						//있었으면 올바른 명령어이므로 history linkedlist에 추가
				addlist(input);
			break;								//없었으면 무시
		case opcodelist:						//opcodelist명령어

			printhash();						//printhash함수 호출
			addlist(input);						//올바른 명령어이므로 history linkedlist에 추가
			break;
		default:								//해당하지 않는 경우
			break;								//명령어 무시
	}
	return;
}

void printhash(){								//hashtable에 있는 모든 데이터 출력
	int order=0;
	node_* temp;
	for(order=0; order<20; order++){			//사이즈가 20
		temp=table[order];
		printf("%d : ", order);
		if(temp==NULL)							//table에 아무것도 없으면 다음으로 넘어감
			printf("\n");
		else{
			while(temp!=NULL){					//linkedlist끝까지 출력
				printf("[%s, %.2X] ", temp->ins, temp->value);
				temp=temp->next;
			}

			printf("\n");
		}
	}
	return;
}

int searchhash(char* array){					//hashtable에 해당 데이터가 있는지 검색
	int order=array[0]%20;						//첫알파벳의 ASCIIcode를 20으로 나눈 나머지로 어느 table에 있을지 판단
	node_* temp = table[order];
	while(temp!=NULL){
		int len1=strlen(array);					//명령어 길이가 일치하는지 판단
		int len2=strlen(temp->ins);
		if(len1==len2){
			if(strcmp(array, temp->ins)==0){	//명령어가 일치하는지 판단
				printf("opcode is %.2X\n", temp->value);//있으면 출력 후 return 0
				return 0;
			}
		}
		temp=temp->next;
	}
	printf("NONE\n");							//없으면 NONE출력 후 return 1
	return 1;
}

int check(char c){								//16진수로 표현되었는지 검사
	if(((c>'f')&&(c<='z'))||((c>'F')&&(c<='Z')))//f~z F~Z이면 return -1
		return -1;
	else
		return 0;								//아니면 0
}

void dump_(int start, int end){								//메모리 공간 출력하는 함수
	int start_a=start/16;									//start 주소 줄
	int start_b=start%16;									//start 주소 칸
	int end_a=end/16;										//end 주소 줄
	int end_b=end%16;										//end 주소 칸
	int i, j;
	if(start_a==end_a){										//같은 줄일 때
		i=start_a;
		printf("%.5X ",address[i]);							//주소 한줄출력
		for(j=0; j<start_b; j++)							//start까지 공백출력
			printf("   ");
		for(j=start_b; j<=end_b; j++)
			printf("%.2X ",memory[i][j]);					//start부터 end까지 16진수 출력
		for(j=end_b+1; j<16; j++)
			printf("   ");									//끝까지 공백 출력
		printf("; ");
		//
		for(j=0; j<start_b; j++)
			printf(".");									//start까지 . 출력
		for(j=start_b; j<=end_b; j++){
			if((memory[i][j]>=32)&&(memory[i][j]<=126))		//start부터 end까지 20~7E이면 ASCII CODE출력, 아니면 . 출력
				printf("%c",memory[i][j]);
			else
				printf(".");								
		}
		for(j=end_b+1; j<16; j++)							//끝까지 . 출력
			printf(".");
		printf("\n");
	}
	else{													//같은 줄 아닐 때
		for(i=start_a; i<=end_a; i++){
			printf("%.5X ",address[i]);						//주소 출력
			if(i==start_a){									//start주소 줄일때
				for(j=0; j<start_b; j++)					//start까지 공백 출력
					printf("   ");
				for(j=start_b;j<16;j++)						//끝까지 16진수 출력
					printf("%.2X ",memory[i][j]);
				printf("; ");
				//
				for(j=0; j<start_b; j++)					//start까지 . 출력
					printf(".");
				for(j=start_b;j<16; j++){					//끝까지 20~7E이면 ASCII CODE출력, 아니면 . 출력
					if((memory[i][j]>=32)&&(memory[i][j]<=126))
						printf("%c",memory[i][j]);
					else
						printf(".");
				}
				printf("\n");
			}
			else if(i==end_a){								//end주소 줄일때
				for(j=0; j<=end_b; j++)						//end까지 16진수 출력
					printf("%.2X ",memory[i][j]);
				for(j=end_b+1; j<16; j++)					//끝까지 공백 출력
					printf("   ");
				printf("; ");
				//
				for(j=0; j<=end_b; j++){					//end까지 20~7E이면 ASCII CODE출력, 아니면 . 출력
					if((memory[i][j]>=32)&&(memory[i][j]<=126))
						printf("%c",memory[i][j]);
					else
						printf(".");
				}
				for(j=end_b+1;j<16; j++)					//끝까지 . 출력
					printf(".");
				printf("\n");
			}
			else{											//start~end사이의 주소 줄일때
				for(j=0; j<16; j++)							//16진수 출력
					printf("%.2X ",memory[i][j]);
				printf("; ");
				//	
				for(j=0; j<16; j++){						//20~7E이면 ASCII CODE 출력, 아니면 . 출력
					if((memory[i][j]>=32)&&(memory[i][j]<=126))
						printf("%c",memory[i][j]);
					else
						printf(".");
				}
				printf("\n");
			}
		}
	}
	
}

void edit_(int address, int value){			//메모리 공간 변경하는 함수
	int start_a=address/16;					//address 줄
	int start_b=address%16;					//adress 칸

	memory[start_a][start_b]=value;			//해당 adress의 memory value로 변경
	return;
}

void fill_(int start, int end, int value){	//메모리 공간 변경하는 함수
	int start_a=start/16;					//start 줄
	int start_b=start%16;					//start 칸
	int end_a=end/16;						//end 줄
	int end_b=end%16;						//end 칸
	int i,j;
	if(start_a==end_a){						//start와 end가 같은 줄 일때
		i=start_a;
		for(j=start_b; j<=end_b; j++)		//start~end까지 value로 변경
			memory[i][j]=value;
	}
	else{									//다른 줄일 때
		for(i=start_a; i<=end_a; i++){		
			if(i==start_a){					//start 줄일때
				for(j=start_b; j<16; j++)	//start부터 끝까지 value로 변경
					memory[i][j]=value;
			}	
			else if(i==end_a){				//end 줄일때
				for(j=0; j<=end_b; j++)		//end까지 value로 변경
					memory[i][j]=value;
			}
			else{
				for(j=0; j<16; j++)			//start, end사이의 줄일때
					memory[i][j]=value;		//모두 value로 변경
			}
		}
	}
	return;
}

void dirent_(){								//현재 디렉터리 내 파일 출력하는 함수
	DIR *dir;
	struct dirent *ent;
	dir = opendir("./");
	struct stat st;

	if(dir!=NULL){
		while((ent=readdir(dir))!=NULL){
			stat(ent->d_name ,&st);
			
			switch(st.st_mode & S_IFMT){
				case S_IFDIR:
					printf("%s/	", ent->d_name); break;		//디렉터리인 경우 /출력
				default:
					if((st.st_mode & S_IXUSR) && (st.st_mode & S_IXGRP)&&(st.st_mode & S_IXOTH))
						printf("%s*	", ent->d_name);		//실행파일일 경우 *출력
					else
						printf("%s	", ent->d_name); break;	//이외의 경우 이름만 출력
			}
		}
		printf("\n");
		closedir(dir);
	}

	return;
}


void addlist(char* input){					//입력된 올바른 명령어를 저장하는 함수
	int i=1;
	node* new=(node*)malloc(sizeof(node));	//node할당
	strcpy(new->data, input);
	new->next=NULL;
	node* temp=head;

	if(head==NULL){							//head가 null일때
		new->num=i;							//순서는 첫번째, head를 new로 한다.
		head=new;
	}
	else{									//head가 null이 아닐 때
		while(temp->next!=NULL){			//마지막 node까지
			i++;							//순서 한개씩 증가
			temp=temp->next;
		}
		i++;
		new->num=i;							//마지막 노드에 연결
		temp->next=new;
	}
	return;
}

	
void printlist(){									//linkedlist에 저장되어 있는 모든 명령어를 출력하는 함수
	node*temp = head;
	while(temp!=NULL){								//linkedlist끝까지
		printf("%d	%s\n", temp->num, temp->data);	//순서와 명령어 출력
		temp=temp->next;
	}
	return;
}
