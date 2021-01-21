#include "20181617.h"
//#include "progaddr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>

int main (){
	initmemory();						//시작하고 바로 메모리공간 초기화
	gethash();							//opcode hashtable 만들기
	for(;;){
		char input[100];	
		printf("sicsim> ");
		fgets(input, 50, stdin);			//shell명령어 입력받기
	
		char *p;
		if((p=strchr(input, '\n'))!=NULL)
			*p='\0';						//입력받은 명령어에서 엔터값은 삭제
		if(input[0]=='\0'){}
		else{
			option5(input);						//option함수  호출
		}
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

void option5( char* input){
	char array[4][50]={'\0', };
	int len=strlen(input);
	int i=0; int j=0; int k=0;
	int result=0;
	for(i=0; i<len; i++){					//input 공백으로 구분
		if(input[i]==' '){
			if(input[i+1]!=' '){
				j++; k=0;
			}
		}
		else{
			array[j][k]=input[i];
			k++;
		}		}
		
	for(i=20; i<24; i++){					//주어진 명령어와 비교
		if(strcmp(array[0], inst_[i])==0)
			break;
	}
	i=i-7;
	switch(i){
		case progaddr:						//progaddr 처리
			result = progaddr_(array[1]);
			if(result ==0)
				addlist(input);
			break;
		case loader:						//loader 처리
			result = loader_(array[1],array[2],array[3]);
			if(result==0)
				addlist(input);
			break;
		case run:
			result = run_();				//run 처리
			if(result ==0)
				addlist(input);
			break;
		case bp:							//bp 처리
			result = bp_(array[1]);
			addlist(input);
			break;
		default:							//그 외
			option(input);
			break;

	}
}

int progaddr_(char * address){				//progaddr 명령어처리
	prostart=strtol(address,NULL, 16);		//int 전환
	if((prostart<0) || (prostart >1048575)||address[0]=='\0'){	//범위 밖 에러
		prostart=0;
		printf("ERROR OUT OF RANGE, setting 0\n");
		return -1;
	}
	else									//범위 내
		return 0;
	//범위 제한하기!!
}

int loader_(char* file1, char* file2, char* file3){	//loader 명령어 처리
	if(file1[0]=='\0'){								//파일 입력이 없을 때 에러
		printf("NO FILE INPUT\n");
		return -1;
	}

	for(int k=0; k<3; k++){							//전역변수 초기화
		csaddr[k]=0;
		cslth[k]=0;
		estab[k]=NULL;
	}
	Areg=0;
	Xreg=0;											//레지스터 초기화
	Lreg=0;
	PCreg=0;
	Breg=0;
	Sreg=0;
	Treg=0;

	char file[3][50]={'\0', };
	int result=0;
	if(file1[0]!='\0'){								//파일이름 복사
		strcpy(file[0],file1);
	}
	if(file2[0]!='\0'){
		strcpy(file[1],file2);
	}
	if(file3[0]!='\0'){
		strcpy(file[2],file3);
	}

	for(int i=0; i<3;i++){							//파일 갯수만큼 pass1실행
		if(file[i][0]!='\0'){
			result=loaderpass1(file[i], i);
			//printf("있음");
			if(result==-1)							//실패
				return -1;
		}
	}
	for(int i=0; i<3; i++){							//파일 갯수만큼 pqss2실행
		if(file[i][0]!='\0'){
			result=loaderpass2(file[i],i);
			if(result==-1)							//실패
				return -1;
		}
	}

	printf("control	symbol	address	length\nsection	name\n-----------------------------------------\n");
	ESTABNODE*temp=estab[0];
	if(estab[0]!=NULL){								//성공시 출력
		PCreg=estab[0]->address;					//PCreg 값 설정
		Lreg=cslth[0];								//Lreg값 설정
		printf("%s\t\t%.4X\t%.4X\n",temp->name, temp->address,cslth[0]);
		temp=temp->next;
		while(temp!=NULL){
			printf("\t%s\t%.4X\n",temp->name, temp->address);
			temp=temp->next;
		}
	}
	temp=estab[1];
	if(estab[1]!=NULL){
		Lreg=Lreg+cslth[1];
		printf("%s\t\t%.4X\t%.4X\n",temp->name, temp->address, cslth[1]);
		temp=temp->next;
		while(temp!=NULL){
			printf("\t%s\t%.4X\n",temp->name, temp->address);
			temp=temp->next;
		}
	}
	temp=estab[2];
	if(estab[2]!=NULL){
		Lreg=Lreg+cslth[2];
		printf("%s\t\t%.4X\t%.4X\n",temp->name, temp->address, cslth[2]);
		temp=temp->next;
		while(temp!=NULL){
			printf("\t%s\t%.4X\n",temp->name, temp->address);
			temp=temp->next;
		}
	}
	printf("-----------------------------------------\n\ttotal length %.4X\n",Lreg);
	totallength=Lreg;								//totallength 전역변수 값 설정
	return 0;
}

int run_(){
		
	if(estab[0]==NULL){								//load된 프로그램이 없을 때 에러
		printf("NO PROGRAM\n");
		return -1;
	}
	int start=estab[0]->address;					//프로그램 시작
	int end = estab[0]->address+totallength;		//프로그램 끝
	int m;											//메모리 값 int 전환
	while(PCreg!=end){								//프로그램이 끝날때까지
		start=PCreg;
		int row=start/16;
		int col=start%16;
		m=(memory[row][col+1]%16)*16*16;				//opcode address3개
		m=m+(memory[row][col+2]);
		int value = memory[row][col];					//mnemonic opcode값
		int code=value/4;
		switch(code){	
			case 45:
														//printf("CLEAR\n");
				m=(memory[row][col+1]/16);
				PCreg=PCreg+2;
				if(m==A)
					Areg=0;
				if(m==X)
					Xreg=0;
				if(m==L)
					Lreg=0;
				if(m==B)
					Breg=0;
				if(m==S)
					Sreg=0;
				if(m==PC)
					PCreg=0;
				break;
			case 10:
														//printf("COMP\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					if(Areg==m)
						conditioncode='=';
					else if(Areg>m)
						conditioncode='>';
					else if(Areg<m)
						conditioncode='<';
				}
				break;
			case 40:
														//printf("COMPR\n");
				PCreg=PCreg+2;
				break;
			case 15:
														//printf("J\n");
				PCreg=PCreg+3;
				int indirect=m+PCreg;
				PCreg=memory[indirect/16][indirect%16]*16*16*16*16;
				PCreg=PCreg+memory[indirect/16][indirect%16+1]*16*16;
				PCreg=PCreg+memory[indirect/16][indirect%16+2];
				break;
			case 12:
														//printf("JEQ\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					m=m+PCreg;
					if(conditioncode=='='){
						if(value%4==3){				
							if((m-PCreg)/(16*16)==15){
								m=m-4096;
							}
							PCreg=m;
						}

					}
				}
				break;
			case 14:
														//printf("JLT\n");
				PCreg=PCreg+3;
				m=m+PCreg;
				if(conditioncode=='<'){
					if((m-PCreg)/(16*16)==15){
						m=m-4096;
					}
					PCreg=m;			
				}
				break;
			case 18:
														//printf("JSUB\n");
				if((memory[row][col+1]/16)%2!=1){
				}
				else{							
					PCreg=PCreg+4;
					if(value%4==1){

					}
					if(value%4==2){

					}
					if(value%4==3){				
						Lreg=PCreg;
						PCreg=m*16*16+memory[row][col+3];
					}
				}
				break;
			case 0:
														//printf("LDA\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					m=m+PCreg;
					if(value%4==1){
						if(memory[row][col+1]/16==0)
							Areg=m-PCreg;
						else
							Areg=m;
					}
					else if(value%4==2){
					}
					else if(value%4==3){
						Areg=memory[m/16][m%16]*(16*16*16*16);
						Areg=Areg+memory[m/16][m%16+1]*(16*16);
						Areg=Areg+memory[m/16][m%16+2];
					}
				}
				else{
				}
				break;
			case 26:
															//printf("LDB\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					m=m+PCreg;
					if(value%4==1){					
						if(memory[row][col+1]/16==0)
							Breg=m-PCreg;
						else					
							Breg=m;
					}
					else if(value%4==2){		

					}
					else if(value%4==3){
						Breg=memory[m/16][m%16]*(16*16*16*16);
						Breg=Breg+memory[m/16][m%16+1]*(16*16);
						Breg=Breg+memory[m/16][m%16+2];
					}
				}
				else{

				}
				break;
			case 20:
															//printf("LDCH\n");
				PCreg=PCreg+3;
				m=Breg+m+Xreg;
				Areg=memory[m/16][m%16];
				break;
			case 29:
															//printf("LDT\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					m=m+PCreg;
					if(value%4==3){
						if(memory[row][col+1]/16==4){
							Treg=memory[Breg/16][Breg%16]*(16*16*16*16);
							Treg=Treg+memory[Breg/16][Breg%16+1]*(16*16);
							Treg=Treg+memory[Breg/16][Breg%16+2];
						}
					}
						
				}
				else {							
					PCreg=PCreg+4;
					m=m+PCreg;
					if(value%4==1){				
						if(memory[row][col+1]/16==1)
							Treg=(m-PCreg)*16*16+memory[row][col+3];
						else{

						}
					}
				}
				break;
			case 54:
															//printf("RD\n");
				conditioncode='=';
				PCreg=PCreg+3;

				break;
			case 19:
															//printf("RSUB\n");
				PCreg=Lreg;
				break;
			case 3:
															//printf("STA\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					m=m+PCreg;
					if(value%4==3){
						memory[m/16][m%16]=Areg/(16*16*16*16);
						memory[m/16][m%16+1]=(Areg%(16*16*16*16))/(16*16);
						memory[m/16][m%16+2]=Areg%(16*16);
					}
				}
				break;
			case 5:
															//printf("STL\n");
				if((memory[row][col+1]/16)%2!=1){	
					PCreg=PCreg+3;
					m=m+PCreg;
					if(value%4==1){

					}
					else if(value%4==2){

					}
					else if(value%4==3){
						memory[m/16][m%16]=Lreg/(16*16*16*16);
						memory[m/16][m%16+1]=(Lreg%(16*16*16*16))/(16*16);
						memory[m/16][m%16+2]=Lreg%(16*16);
					}
				}
				else{

				}
				break;
			case 4:
																//printf("STX\n");
				if((memory[row][col+1]/16)%2!=1){
					PCreg=PCreg+3;
					m=m+PCreg;
					if(memory[row][col+1]/16==4){
						memory[Breg/16][Breg%16]=Xreg/(16*16*16*16);
						memory[Breg/16][Breg%16+1]=(Xreg%(16*16*16*16))/(16*16);
						memory[Breg/16][Breg%16+2]=Xreg%(16*16);
					}

				}
				break;
			case 56:
																//printf("TD\n");
				if((memory[row][col+1]/16)%2!=1){	
					PCreg=PCreg+3;
					m=m+PCreg;
					conditioncode='<';
				}
				break;
			case 46:
																//printf("TIXR\n");
				PCreg=PCreg+2;
				m=memory[row][col+1];
				Xreg=Xreg+1;
				if(Xreg==Treg)
					conditioncode='=';
				else if(Xreg>Treg)
					conditioncode='>';
				else if(Xreg<Treg)
					conditioncode='<';
				break;
			case 55:
																//printf("WD\n");
				PCreg=PCreg+3;
				break;
			default:
				break;
		}
		for(int i=0; i<bparray[0]; i++){						//bp와 일치할때
			if(PCreg==bparray[i+1]){							//출력
				printf("A : %.6X  X : %.6X\n",Areg, Xreg);
				printf("L : %.6X PC : %.6X\n",Lreg, PCreg);
				printf("B : %.6X  S : %.6X\n",Breg, Sreg);
				printf("\t\tStop at checkpoint[%X]\n",PCreg);
				return 0;
			}
		}

	}

	if(PCreg==end){												//프로그램 끝일때
		printf("A : %.6X  X : %.6X\n",Areg, Xreg);				//출력
		printf("L : %.6X PC : %.6X\n",Lreg, PCreg);
		printf("B : %.6X  S : %.6X\n",Breg, Sreg);
		printf("\t\tEnd Program\n");
		return 0;
	}



	return 0;
}

int bp_(char * input){								//bp명령어 처리
	if (input[0]=='\0'){							//bp입력시
		printf("\tbreakpoint\n");
		printf("\t----------\n");
		bpprint();									//bp값들 출력함수 호출
	}	
	else if(strcmp(input, "clear")==0){				//bp clear시
		bparray[0]=0;								//bp 갯수 0 초기화
		printf("[ok] clear all breakpoints\n");
	}
	else{
		int num=strtol(input,NULL, 16);				//bp address시
		bparray[0]++;								//갯수 +해준뒤 해당 순서에 저장
		int count=bparray[0];
		bparray[count]=num;
		printf("[ok] create breakpoint %X\n",num);
	}
	return 0;
}

void bpprint(){										//bp값들 출력하는 함수
	int count=bparray[0];
	for(int i=0; i<count; i++)						//0번째에 저장된 갯수만큼 출력
		printf("\t%X\n",bparray[i+1]);
	return;
}

int loaderpass1(char* filename, int filenum){		//pass1처리
	FILE *fp = fopen(filename, "r");
	if(fp==NULL){									//파일없을때 에러
		printf("NO FILE\n");
		return -1;
	}
	if(filenum==0)
		csaddr[filenum]=prostart;					//첫번째 파일 csaddr 저장
	char array[100];
	char ctemp[8];
	int pstart=0;
	int pend=0;
	int result;
	int linecount=0;
	fgets(array,100,fp);							//파일 읽기
	linecount++;
	for(int j=0; j<strlen(array); j++)
		if(array[j]=='\n')
			array[j]='\0';
	int i=1;
	char name[10];
	
	strncpy(name, array+i,6);						//프로그램이름 저장
	name[6]='\0';
	i=i+6;
	
	result=searchestab(name);						//이미 있으면 에러
	if(result!=-1){
		printf("%s line: %d PASS1 ERROR %s 중복\n", filename, linecount, name);
		fclose(fp);
		return -1;
	}
	addestab(name, csaddr[filenum],filenum);		//없으면 estab저장

	strncpy(ctemp,array+i,6);						//시작주소, 길이 각각 저장
	ctemp[6]='\0';
	pstart=strtol(ctemp, NULL, 16);
	i=i+6;
	strncpy(ctemp,array+i,6);
	ctemp[6]='\0';
	pend=strtol(ctemp, NULL, 16);
	cslth[filenum]=pend-pstart;

	fgets(array, 100, fp);							//파일 읽기
	linecount++;
	for(int j=0 ;j<strlen(array);j++)
		if(array[j]=='\n')
			array[j]='\0';
	while(array[0]!='E'){							//E가 올때까지 계속
		if(array[0]=='D'){							//D일 때
			for(int i=1; i<strlen(array); ){
				strncpy(name, array+i, 6);
				name[6]='\0';
				i=i+6;
				result=searchestab(name);			//symbol 이름 검색
				if(result!=-1){						//있으면 에러
					printf("%s line: %d PASS1 1ERROR %s 중복\n", filename, linecount, name);
					fclose(fp);
					return -1;
				}
				strncpy(ctemp, array+i,6);
				ctemp[6]='\0';
				i=i+6;
				pstart=strtol(ctemp, NULL, 16);
				addestab(name, csaddr[filenum]+pstart,filenum);//없으면 estab저장
			}
		}
		fgets(array, 100, fp);						//파일 읽기
		linecount++;
		for(int j=0; j<strlen(array); j++)
			if(array[j]=='\n')
				array[j]='\0';
	}
	if(filenum!=2)									//두세번째 파일 csaddr 값설정
		csaddr[filenum+1]=csaddr[filenum]+cslth[filenum];
	fclose(fp);
	return 0;
}

int loaderpass2(char* filename, int filenum){		//pass2 처리
	FILE * fp=fopen(filename, "r");
	char array[100]={'\0', };
	int i=1;
	char ctemp[8]={'\0', };
	int num;
	int result;
	int value;
	char r[10][6]={'\0', };
	int linecount=0;
	while(!feof(fp)){								//파일 끝까지
		fgets(array,100, fp);						//파일 읽기
		linecount++;
		for(int j=0; j<strlen(array);j++)
			if(array[j]=='\n')
				array[j]='\0';
		while(array[0]!='E'){						//E가 나올때까지
			i=1;
			if(array[0]=='T'){						//T일 경우
				strncpy(ctemp,array+i,6);			//앞 6글자 시작 address
				ctemp[6]='\0';
				num=strtol(ctemp, NULL, 16);
				num=num+csaddr[filenum];
				i=i+8;
				char ccnum[3];
				for(;i<strlen(array);){				//2글자씩 읽어서 memory에 저장
					strncpy(ccnum, array+i, 2);
					ccnum[2]='\0';
					value=strtol(ccnum, NULL, 16);
					edit_(num, value);
					num++;
					i=i+2;
				}
			}
			if (array[0]=='R'){						//R일 경우
				for(;i<strlen(array);){
					char ccnum[3];
					ccnum[0]=array[i];				//2글자 symbol 순서
					ccnum[1]=array[i+1];
					ccnum[2]='\0';

					num=strtol(ccnum, NULL, 16);
					i=i+2;
					strncpy(ctemp, array+i, 6);		//6글자 symbol 이름
					char* toktemp;
					toktemp= strtok(ctemp, " ");
					strcpy(r[num],toktemp);			//r에 순서별로 저장
					strcpy(r[1],estab[filenum]->name);
					i=i+6;
				}
			}
			else if (array[0]=='M'){				//M일 경우
				strncpy(ctemp, array+i,6);
				ctemp[6]='\0';						//6글자 address
				num=strtol(ctemp, NULL, 16);
				num=num+csaddr[filenum];
				i=i+6;
				char cctemp[3];
				cctemp[0]=array[7];					//2글자 메모리 변경 갯수
				cctemp[1]=array[8];
				cctemp[2]='\0';
				strncpy(ctemp, array+i, 2);
				ctemp[3]='\0';
				int num2 = strtol(cctemp, NULL, 16);
				i=i+2;
				int row=num/16;
				int col=num%16;
				int irow=row;
				int icol=col;
				if(num2==5){						//5일때
					int a = (memory[row][col]/16)*16;
					int b = (memory[row][col]%16)*16*16*16*16;
					col++;
					if(col>=16){
						row=row+1; 
						col=0;
					}
					
					
					b = b + (memory[row][col])*16*16;;
					col++;
					if(col>=16){
						row=row+1;
						col=0;
					}
					
					b = b + (memory[row][col]);		//각 메모리 5개읽어서 더한뒤 b에 int로 저장
					
					cctemp[0]=array[10];
					cctemp[1]=array[11];
					cctemp[2]='\0';
					num=strtol(cctemp, NULL, 16);

					result = searchestab(r[num]);	//reference symbol 이름 검색

					if(result==-1){					//없으면 에러
						printf("%s line : %d PASS2 ERROR NO SYMBOL %s\n",filename, linecount, r[num]);
						fclose(fp);
						return -1;
					}
					
					if(array[9]=='+')				//부호따라 계산
						b=b+result;
					else if (array[9]=='-')
						b=b-result;
					b=b%(16*16*16*16*16);
					
					memory[irow][icol]=a+(b/(16*16*16*16));//메모리에 다시 b값 저장
					icol++;
					if(icol>=16){
						irow=irow+1;
						icol=0;
					}
					memory[irow][icol]=(b%(16*16*16*16))/(16*16);
					icol++;
					if(icol>=16){
						irow=irow+1;
						icol=0;
					}
					memory[irow][icol]=(b%(16*16));
				}
				else if (num2==6){							//6일때
					int a=memory[row][col]*16*16*16*16;		//각 메모리 6개 읽어서 저장
					col++;									//밑으로는 5일때와 같음
					if(col>=16){
						row++;
						col=0;
					}
					a=a+memory[row][col]*16*16;
					col++;
					if(col>=16){
						row++;
						col=0;
					}
					a=a+memory[row][col];

					cctemp[0]=array[10];
					cctemp[1]=array[11];
					cctemp[2]='\0';
					num=strtol(cctemp, NULL, 16);

					result = searchestab(r[num]);
					if(result==-1){
						printf("%s line : %d PASS2 ERROR NO SYMBOL %s\n",filename, linecount, r[num]);
						fclose(fp);
						return -1;
					}

					if(array[9]=='+')
						a=a+result;
					else if(array[9]=='-')
						a=a-result;
					a=a%(16*16*16*16*16);

					memory[irow][icol]=a/(16*16*16*16);
					icol++;
					if(icol>=16){
						irow++;
						icol=0;
					}
					memory[irow][icol]=(a%(16*16*16*16))/(16*16);
					icol++;
					if(icol>=16){
						irow++;
						icol=0;
					}
					memory[irow][icol]=(a%(16*16));
				}
			}
			fgets(array, 100,fp);									//파일 읽기
			linecount++;
			for(int j=0; j<strlen(array);j++)
				if(array[j]=='\n')
					array[j]='\0';
		}
	}

	fclose(fp);
	return 0;
}


void addestab(char* input,int address ,int filenum){			//estab에 symbol 저장
	ESTABNODE* new=(ESTABNODE*)malloc(sizeof(ESTABNODE));
	char *result;
	result = strtok(input, " ");
	strcpy(new->name, result);
	new->address=address;
	new->next=NULL;
	if(estab[filenum]==NULL){									//head NULL 일때 head에 저장
		estab[filenum]=new;
	}
	else{
		ESTABNODE* temp=estab[filenum];							//아닐때 제일 끝 node에 저장
		while(temp->next!=NULL)
			temp=temp->next;
		temp->next=new;
	}
	return;
}

int searchestab(char* input){				//symbol 검색
	ESTABNODE* temp=estab[0];
	while(temp!=NULL){						//0번째 검색
		if(strcmp(input, temp->name)==0)
			return temp->address;
		temp=temp->next;
	}
	temp=estab[1];
	while(temp!=NULL){						//1번째 검색
		if(strcmp(input, temp->name)==0)
			return temp->address;
		temp=temp->next;
	}
	temp=estab[2];	
	while(temp!=NULL){						//2번째 검색
		if(strcmp(input, temp->name)==0)
			return temp->address;
		temp=temp->next;
	}
	return -1;								//잇으면 address리턴 없으면 -1리턴
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
			printf("h[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\nassemble filename\ntype filename\nsymbol\nprogaddr\nloader\nrun\nbp\n");
			addlist(input);						//올바른 명령어이므로 명령어 history list에 추가
	    	break;
		case dir:								//dir 명령어
			dirent_();							//dirent_함수 호출
			addlist(input);						//올바른 명령어이므로 명령어 history list에 추가
	    	break;
		case quit:								//quit 명령어
			for(int j=0; j<filenums; j++){
				remove(filenames[j]);			//생성된 파일제거
			}
			node *target=head;					//free
			node *temp=target;
			while(target!=NULL){
				temp=temp->next;
				free(target);
				target=temp;
			}
			snode *target2=shead;
			snode *temp2 = target2;
			while(target2!=NULL){
				temp2=temp2->next;
				free(target2);
				target2=temp2;
			}
			symnode *target3=symhead;
			symnode *temp3=target3;
			while(target3!=NULL){
				temp3=temp3->next;
				free(target3);
				target3=temp3;
			}
			exit(0);							//프로그램 종료					
	    	break;
		case history: 							//history 명령어
			addlist(input);						//올바른 명령어이므로 명령어 history list에 추가
			printlist();						//printlist함수 호출
	    	break;
	default:									//해당하지 않는 경우
	    option4(input);							//option2함수 호출
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
			if(input[i+1]!=' '){
				j++; k=0;
			}
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
			//option5(input);
			break;								//명령어 무시
	}
	return;
}

void option4(char* input){						//type, assemble, symbol명령어 처리
	char array[2][50]={'\0', };
	int len = strlen(input);
	int i=0; int j=0; int k=0;
	int result;
	for(i=0; i<len; i++){						//input을 공백으로 구분하는 for문 각각 array[0],[1]에 저장
		if(input[i]==' '){						
			if(input[i+1]!=' '){
				j++; k=0;
			}
		}
		else{
			array[j][k]=input[i];
			k++;
		}
	}
	for(i=17; i<20; i++){						//inst_의 값 들과 비교
		if(strcmp(array[0], inst_[i])==0)
			break;
	}
	i=i-7;
	switch(i){									//명령어 구분하는 switch문
		case type:								//type명령어
			result = type_(array[1]);			//type_함수 성공하면 return 0
			if(result==0)						//성공시 history list에 저장
				addlist(input);
			break;
		case assemble:							//assemble 명령어
			result=assemble_(array[1]);			//assemble_함수 성공시 return 0
			if(result==0)						//성공시 history list에 저장
				addlist(input);
			break;
		case symbol:							//symbol명령어
			symbol_();							//symbol_함수 호출
			addlist(input);						//history list에 저장
			break;
		default:								//이외의 경우
			option2(input);						//option2호출
			break;
	}
	return;
}

void symbol_(){									//symbol파일 출력
	FILE *fp=fopen("symbol","r");				//symbol 파일 포인터
	if(fp==NULL){								//없을 때는 종료
		return;
	}
	else{
		while(!feof(fp)){						//파일 끝까지
			char symbol[10];
			int loc;
			fscanf(fp, "%s %d\n",symbol, &loc);	//파일 읽어서
			printf("\t%s\t%.4X\n",symbol, loc);	//16진수로 출력

		}
	}
	return;

}

int assemble_(char* name){					//assemble_명령어 처리
	char filename[50]={'\0', };				//입력된 filename에서 .asm을 뺀 파일 이름
	int i;
	int len = strlen(name);
	for(i=0; i<len; i++){					//.이전까지 filename저장
		if(name[i]=='.')
			break;
		filename[i]=name[i];
	}
	int result;
	result=sourcelist(name);				//sourcelist함수 성공시 return 0
	//printf("성궁\n");
	if(result==-1)							//실패시 return -1
		return -1;
	result = pass1();//pass1함수 성공시 return 0
	//printf("성공\n");
	if(result ==-1)							//실패시 return -1
		return -1;
	result=pass2();							//pass2함수 성공시 return 0
	if(result==-1)							//실패시 return 0-1
		return -1;

	makeobject(filename);					//object파일 생성함수호출
	makelst(filename);						//lst파일 생성함수 호출
	makesymbol();							//symbol파일 생성함수호출

	return 0;
}

void makelst(char* filename){						//lst파일 생성함수
	char name[50]={'\0', };							
	int i=0;
	strcpy(name, filename);							
	i=strlen(filename);			
	name[i]='.';
	name[i+1]='l';
	name[i+2]='s';
	name[i+3]='t';									//filename 끝에 .lst 복사
	FILE *fp=fopen(name, "w");
	snode* temp=shead;
	while(temp!=NULL){								//temp가 NULL일 때까지
		if(temp->type!=1){							//주석문일때
			fprintf(fp, "%d\t\t",temp->num);		//행, 주석문 출력
			fprintf(fp, "%s\n",temp->comment);
		}
	else{											//주석문아닐때

		fprintf(fp, "%d\t",temp->num);				//행출력
		if((strcmp(temp->s[1],"BASE")==0)||(strcmp(temp->s[1],"END")==0))		//BASE나 END일 때 
			fprintf(fp, "\t");													//공백
		else
			fprintf(fp, "%.4X\t",temp->loc);									//BASE나 END아닐때 loc출력
		fprintf(fp, "%s\t%s\t%s",temp->s[0],temp->s[1],temp->s[2]);				//입력된 label, mnemonic, operand순서대로 출력
		if(temp->s[3][0]!='\0')													//,X있으면  출력
			fprintf(fp,", %s\t",temp->s[3]);
		else
			fprintf(fp,"\t\t");
		if((strcmp(temp->s[1],"BASE")==0)||(strcmp(temp->s[1],"END")==0)||(strcmp(temp->s[1],"START")==0))	//BASE나 END, START일때
			fprintf(fp,"\t\n");																				//공백
		else{
			for(i=0; i<temp->len;i++)											//아닐때는 len만큼 opcode출력
				fprintf(fp,"%c",temp->op[i]);
			fprintf(fp, "\n");
		}
	}
		temp=temp->next;														//다음 linkedlist
	
	}
	fclose(fp);
	strcpy(filenames[filenums],name);											//filename복사
	filenums++;																	//filenums 1증가
	return;
}

void makeobject(char* filename){												//object파일 생성
	snode* temp=shead;
	char name[50]={'\0',};
	char array[60]={'\0',};
	int i=0;
	int len=0;
	int start=0;
	int a;
	strcpy(name, filename);	
	i=strlen(filename);
	name[i]='.';
	name[i+1]='o';
	name[i+2]='b';	
	name[i+3]='j';																//filename에 .lst 복사
	FILE *fp=fopen(name, "w");
	fprintf(fp, "H%s",startname);												
	for(i=0; i<7-strlen(startname)-1; i++)
		fprintf(fp," ");		
	fprintf(fp, "%.6X%.6X\n",startn, fileend-temp->loc);						//H파일이름 주소 출력
	a=temp->loc;																//시작 loc저장
	if(strcmp(temp->s[1],"START")==0)
		temp=temp->next;														//temp가 start이면 다음 linkedlist
	start=startn;																//시작 주소 저장
	while(temp->next!=NULL){
		if((temp->type==1)&&(strcmp(temp->s[1],"BASE")!=0)){					//주석문이아니고 BASE가아닐때
			len=len+temp->len;													//len에 temp의 len더하기
			if((temp->len==0)){													//temp의 len이 0이면 변수가 나올경우이다.
				fprintf(fp,"T%.6X%.2X%s\n",start,temp->loc-start,array);		//T시작주소,길이, array 출력
				for(i=0; i<60; i++)												//초기화
					array[i]='\0';
				while(temp->len==0)												//이후 len이 0인경우는 모두 무시
					temp=temp->next;
				start=temp->loc;												//start 재설정
				len=temp->len;													//len재설정
			}
			else if(len>60){													//len이 60보다 클때
				len=len-temp->len;
				fprintf(fp,"T%.6X%.2X%s\n",start, temp->loc-start, array);
				for(i=0; i<60; i++)
					array[i]='\0';
				start=temp->loc;
				len=temp->len;													//위와 똑같이 출력
			}
			strncpy(array+(len-temp->len),temp->op,temp->len);					//array에 temp opcode 복사
		}
		temp=temp->next;
	}
	fprintf(fp, "T%.6X%.2X%s\n",start, fileend-start, array);					//위와 똑같이 출력
	for(i=0; i<modnum; i++)														//modnum만큼 modification record출력
		fprintf(fp, "M%.6X05\n", mod[i]);

	fprintf(fp, "E%.6X\n",a);													//E 시작 loc 출력
	fclose(fp);
	strcpy(filenames[filenums],name);											//파일이름 저장하고 +1
	filenums++;
	return;

}

void makesymbol(){																//symbol파일 생성
	symnode* temp=symhead;	
	FILE *fp =fopen("symbol","w");
	while(temp!=NULL){
		fprintf(fp, "%s %d\n",temp->name, temp->loc);							//symbol name과 loc순서대로 한줄씩 저장
		temp=temp->next;
	}
	fclose(fp);
	strcpy(filenames[filenums],"symbol");										//파일이름 저장하고 +1
	filenums++;
	return;
}

int sourcelist(char* name){														//파일읽어서 label, mnemonic, operand, x구분
	char source[100];
	FILE *fp = fopen(name, "r");
	int i=5;
	if(fp==NULL){																//파일없을때 return -1
		printf("No FILE\n");
		return -1;
	}
	while(!feof(fp)){
		int j=0;	int k=0;	int t=0;
		char s[4][10]={'\0', };
		char s1[6]={'\0',};
		fgets(source, 100, fp);													//파일 한줄씩 읽기
		for(j=0; j<strlen(source);j++)
			if(source[j]=='\n')													//엔터 제거
				source[j]='\0';
		if(source[0]!='\0'){

		if(source[0]!='.'){														//주석문이 아닐때
			for(j=0; j<strlen(source);j++){										//입력받은 source 공백이나 , 를 기준으로 s에 순서대로 저장
				if((source[j]==' ')||source[j]==','){
					if(source[j+1]!=' ')
						k++; t=0;
				}
				else{
					s[k][t]=source[j];
					t++;
				}
			}
			int result = searchsource(s[0]);									//첫번째 입력이 mnemonic인지판단  -1이면 아님
			if(result!=-1)														
				addsourcelist(1,s1, i, 0, s1,s[0],s[1],s[2],0);					//addsourcelist에 한칸을 건너뛰고 넘김
			else
				addsourcelist(1,s1, i,0,s[0],s[1],s[2],s[3],0);					//mnemonic이기때문에 그대로 넘김
			i=i+5;																//행을 +5씩
		}
		else{
			addsourcelist(2, source,i, 0,s1,s1,s1,s1,0);						//주석문일때는 type값 2, source 넘김
			i=i+5;
		}
		}
	}
	fclose(fp);
	return 0;
}

int searchsource(char* array){													//mnemonic인지 판단
	int order;
	if(array[0]=='+')															//+있을땐 그다음 글자로 order
		order=array[1]%20;
	else
		order=array[0]%20;														//아닐땐 첫글자로 order
	node_*temp = table[order];
	while(temp!=NULL){
		if(strcmp(array, temp->ins)==0){										//일치하면 format 값 return
			tempop=temp->value;
			return (temp->n[0]-'0');
		}
			
		else if (array[0]=='+'){												//+이고 일치하면 4 return
			if(strcmp(array+1,temp->ins)==0){
				tempop=temp->value;
				return 4;
			}
		}
		temp=temp->next;
	}
	for(int i=0; i<3; i++){														//START, BASE, END이면 0리턴
		if(strcmp(array, source_[i])==0){
			return 0;
		}
	}
	return -1;																	//없으면 -1 리턴

}

void addsourcelist(int type, char* comment, int num, int loc, char* s0, char *s1, char*s2, char*s3, int op){		//링크드리스트 생성
	snode* new = (snode*)malloc(sizeof(snode));					//노드 할당
	new->type=type;	
	strcpy(new->comment, comment);
	new->num = num;
	new->len=0;
	new->loc=loc;
	strcpy(new->s[0],s0);
	strcpy(new->s[1],s1);
	strcpy(new->s[2],s2);
	strcpy(new->s[3],s3);
	printf("%s	%s	%s	%s\n", s0,s1,s2,s3);
	for(int i=0; i<8; i++)
		new->op[i]='\0';
	new->next=NULL;												//값 저장
	snode* temp = shead;
	if(shead==NULL){											//shead 널이면 new가 shead
		shead=new;
	}
	else{
		while(temp->next!=NULL)									//아니면 NULL찾고 연결
			temp=temp->next;
		temp->next=new;
	}
	return;
}

int pass1(){													//패스1
	int endn=0;		
	int num;
	int i;
	char comma='\'';
	snode* temp = shead;
	if(strcmp(temp->s[1],"START")==0){							//temp가 START이면 loc을 입력값으로 설정
		temp->loc=atoi(temp->s[2]);
		locctr=temp->loc;										//locctr, startn에도 저장
		startn=locctr;
		strcpy(startname,temp->s[0]);							//파일이름 복사
		temp=temp->next;										//다음 linkedlist
	}
	else{
		startn=0;												//START없으면 startn, locctr은 0
		for(i=0; i<10; i++)										//파일이름 널
			startname[i]='\0';
		locctr=0;	
	}
	while(strcmp(temp->s[1],"END")!=0){							//END까지
		//printf("%s\n",temp->s[1]);
		endn++;
		temp->loc=locctr;										//temp의 loc은 locctr
		if(temp->type==1){										//주석문아닐때
		int byte=0;
		int result = 5;
		if(temp->s[0][0]!='\0'){								//label있을때
			//printf("symbolcheck\n");
			result = symbolcheck(temp->s[0]);					//symbolcheck
			//printf("symbolcheck done\n");
			if (result !=-1){									//있으면 에러 출력
				printf("겹치는 symbol ERROR line %d\n", temp->num);
				return -1;
			}
			else{
				insertsymbol(temp->s[0],temp->loc);				//없으면 insert symbol
			}
			//printf("symbolcheck\n");
		}
		result = searchsource(temp->s[1]);						//searchsource 리턴 저장
		if(result!=-1){											//올바른 명령어일때
			locctr=locctr+result;								//result값 locctr에 더하기	
		}
		else if (strcmp(temp->s[1],"WORD")==0){					//WORD이면 3
			locctr=locctr+3;
		}
		else if(strcmp(temp->s[1],"RESW")==0){					//RESW이면 입력된 값*3
			num=atoi(temp->s[2]);
			locctr=locctr+(num*3);
		}
		else if (strcmp(temp->s[1],"RESB")==0){					//RESB이면 입력된 값
			num=atoi(temp->s[2]);
			locctr=locctr+num;
		}
		else if(strcmp(temp->s[1],"BYTE")==0){					//BYTE일때
			if(temp->s[2][0]=='C'){								//CHAR
				for(i=2; i<strlen(temp->s[2])-1; i++)	{
					if(temp->s[2][i]!=comma)
						byte++;		
				}
				locctr=locctr+byte;								//입력된 값 갯수
			}
			else if (temp->s[2][0]=='X'){						//16진수
				for(i=2; i<strlen(temp->s[2])-1; i++){
					if(temp->s[2][i]!=comma)
						byte++;
				}
				locctr=locctr+(byte/2);							//입력된 값갯수/2
			}
		}
		else{
			printf("ERROR line %d\n",temp->num);				//이외에는 에러
			return -1;
		}
		//j=0;
		byte=0;
	}
		temp=temp->next;
		fileend=locctr;											//파일 끝 loc fileend에 저장
		if(temp==NULL){											//END가 없을때 ERROR
			printf("ERROR NO END\n");
			return -1;
		}
	}

	
	
	
	return 0;
}

void change(char *a, int b){									//int -> 16진수 char
	if(b<10)
		*a=b+'0';												//0~9
	else{
		if(b==10)
			*a='A';												//A
		if(b==11)
			*a='B';												//B
		if(b==12)
			*a='C';												//C
		if(b==13)
			*a='D';												//D
		if(b==14)
			*a='E';												//E
		if(b==15)
			*a='F';												//F
	}
	return;
}

int pass2(){					//pass2함수
	//printf("pass2\n");
	snode * temp=shead;
	int n, i,x,b,p,e;
	int q;
	int k=0;
	int middle;
	int op1, op2;
	int address1;
	int address;
	int base;
	char comma='\'';
	if(strcmp(temp->s[1],"START")==0){						//temp가 START이면 다음 linkedlist
		//printf("NO START\n");
		temp=temp->next;
	}
	//temp=temp->next;
	while(strcmp(temp->s[1], "END")!=0){					//END전까지
		int result2;
		int opcode4[4]={0, };
		int opcode6[6]={0, };
		if(temp->type==1){									//주석문이 아닐때
			if(strcmp(temp->s[1],"BYTE")==0){				//BYTE이면
				if(temp->s[2][0]=='C'){						//C일때
					k=2;
					while(temp->s[2][k]!=comma){
						opcode6[(k-2)*2]=temp->s[2][k]/16;	//''사이내용 opcode6에 저장
						opcode6[(k-2)*2+1]=temp->s[2][k]%16;
						k++;
					}
					temp->len=2*(k-2);						//len은 ''사이 길이 *2
					for(q=0; q<temp->len; q++)
						change(&temp->op[q],opcode6[q]);	//16진수 변환후 op저장
				}
				else if (temp->s[2][0]=='X'){			//X일때
					k=2;
					while(temp->s[2][k]!=comma){			//''사이 내용 opcode6저장
						if((temp->s[2][k]>='0')&&(temp->s[2][k]<='9'))
							opcode6[k-2]=temp->s[2][k]-'0';
						else if((temp->s[2][k]>='A')&&(temp->s[2][k]<='F'))
							opcode6[k-2]=temp->s[2][k]-'A'+10;
						else{
							printf("16진수X line %d\n",temp->num);	//16진수아닐땐 에러
							return -1;
						}
						k++;
					}
					temp->len=k-2;							//len은 ''사이 길이
					for(q=0; q<temp->len; q++)
						change(&temp->op[q],opcode6[q]);		//16진수 변환후 op저장
				}
			}
			else if(strcmp(temp->s[1],"RESW")==0||(strcmp(temp->s[1],"RESB")==0))	//RESW나 RESB일경우 무시
			{}
			else if (strcmp(temp->s[1],"RSUB")==0){						//RSUB일경우
				temp->op[0]='4';										//4F0000저장
				temp->op[1]='F';
				temp->len=6;
				for(q=2; q<temp->len; q++)
					temp->op[q]='0';
			}
			else{
				int result=searchsource(temp->s[1]);					//searchsource값 저장
				if(result==1){											//format 1
					temp->len=2;
					change(&temp->op[0],tempop/16);						//16진수 변환후 len길이2 op
					change(&temp->op[1],tempop%16);
				}
				else if(result==2){										//format 2
					opcode4[0]=tempop/16;								//2글자는 명령어 op로 저장
					opcode4[1]=tempop%16;
					switch(temp->s[2][0]){
					case 'A': opcode4[2]=A;	break;
					case 'X': opcode4[2]=X;	break;
					case 'L': opcode4[2]=L;	break;
					case 'B': opcode4[2]=B;	break;
					case 'S': opcode4[2]=S;	break;
					case 'T': opcode4[2]=T;	break;
					case 'F': opcode4[2]=F;	break;
					default: if(strcmp(temp->s[2], "PC")==0)
								 opcode4[2]=PC;
						 	else if(strcmp(temp->s[2], "SW")==0)
								 opcode4[2]=SW;
						 	else{/*
								printf("없는 register line %d\n",temp->num);
								return -1;*/
								opcode4[3]=temp->s[3][0]-'0';
							 }
					break;
					}
					if(temp->s[3][0]!='\0'){
						switch(temp->s[3][0]){
						case 'A':opcode4[3]=A;	break;
						case 'X':opcode4[3]=X;	break;
						case 'L':opcode4[3]=L;	break;
						case 'B':opcode4[3]=B;	break;
						case 'S':opcode4[3]=S;	break;
						case 'T':opcode4[3]=T;	break;
						case 'F':opcode4[3]=F;	break;
						default: if(strcmp(temp->s[3], "PC")==0)
									 opcode4[3]=PC;
								 else if(strcmp(temp->s[3],"SW")==0)
							 		opcode4[3]=SW;
								 else{/*
									printf("없는 register line %d\n",temp->num);	//다음 2글자는 레지스터 값으로 저장 없는 레지스터의 경우 에러출력, 프로그램 종료
									return -1;*/
									 opcode4[3]=temp->s[3][0]-'0';
								 }
							break;
						}
					}
					else
						opcode4[3]=0;
					temp->len=4;				//목적코드길이는 4
					for(q=0; q<temp->len; q++)
						change(&temp->op[q],opcode4[q]);		//16진수 변환후 저장
				}
				else if (strcmp(temp->s[1],"BASE")==0){								//BASE의 경우
					result2=symbolcheck(temp->s[2]);								//있는symbol인지 체크
					if (result2==-1){
						printf("NO SYMBOL line %d\n",temp->num);					//없으면 에러출력후 종료
						return -1;
					}
					base=result2;													//있으면 symbol의 loc, base에 저장
					temp->len=0;													//len은 0
				}
				else{																//format 3,4
					op1=tempop/16;	op2=tempop%16;
					result2=symbolcheck(temp->s[2]);
					if(temp->s[2][0]=='#'){											//n,i값 지정 #경우
						result2=symbolcheck(temp->s[2]+1);						//result2다시 다음글자부터 판단
						n=0; i=1;
					}
					else if (temp->s[2][0]=='@'){									//@경우
						result2=symbolcheck(temp->s[2]+1);							//result2다시 다음글자부터 판단
						n=1; i=0;
					}
					else{															//이외경우
						n=1; i=1;								
					}
					
					op2=op2+n*2+i;													//2진수를 다시 int 10진수로
					if(result2==-1){												//symbol없을 때
						if(temp->s[2][0]=='#'){										
							if((temp->s[2][1]<='9')&&(temp->s[2][1]>='0')){}		//#숫자일때
							else{													//#숫자아닐때 에러출력후 종료
								printf("NO SYMBOL line %d\n",temp->num);
								return -1;
							}
						}
						else{
							printf("NO SYMBOL line %d\n",temp->num);
							return -1;
						}
					}

					if(temp-> s[3][0]=='X')	x=1;						//,X있으면 x는 1, 없으면 0
					else	x=0;
		
					if(result==4)	e=1;								//4형식이면 e는 1, 아니면 0
					else	e=0;

					if(temp->s[3][0]=='X'){								//address, b, p 값 지정
						address=result2-temp->next->loc;				//,X가있을때 주소값
						address1=abs(address);							//주소값 절대값
						if(address1>4095){								//4095보다 절대값크면
							b=1; p=0;									
							address=result2-base;						//주소값 재설정
						}
						else{
							b=0; p=1;
						}
					}
					else{												//X없을때
						if(temp->s[2][0]=='#'){							//#이있을때
							if(result2!=-1){							//#symbol 일 때	
								address=result2-temp->next->loc;
								address1=abs(address);
								if(address1>4095){						//위와 같음
									b=1; p=0;
									address=result2-base;
								}
								else{
									b=0; p=1;
								}	
							}
							else{										//#숫자 일때
								b=0; p=0;
								char aa[10];
								strcpy(aa,temp->s[2]+1);				//숫자값 주소에 저장
								address=atoi(aa);
							}

						}
						else if(temp->s[2][0]=='@'){				//@일때
							address=result2-temp->next->loc;		//위와 같음
							address1=abs(address);
							if(address1>4095){
								b=1; p=0; address=result2-base;
							}
							else{
								b=0; p=1;
							}
						}
						else if(result==4){										//format4이면
							b=0; p=0;								//b, p모두 0
							address=result2;
						}
						else{
							address=result2-temp->next->loc;		//이외의 경우 위와 같음
							address1=abs(address);
							if(address1>4095){
								b=1; p=0;
								address=result2-base;
							}
							else{
								b=0; p=1;
							}

						}
					}

					
					middle=x*2*2*2+b*2*2+p*2+e;						//2진수 10진수int변환

					if(result==4){									//4형식일때
						if((temp->s[2][0]=='#')&&(temp->s[2][1]>='0')&&(temp->s[2][1]<='9')){}
						else{										//#숫자형식아닐때
							mod[modnum]=temp->loc+1;				//modification record 저장
							modnum++;
						}
						temp->len=8;								//len 8로 설정후 int값 16진수형식 char로 8개 저장
						change(&temp->op[0],op1);
						change(&temp->op[1],op2);
						change(&temp->op[2],middle);
						change(&temp->op[3],address/(16*16*16*16));
						change(&temp->op[4],(address%(16*16*16*16))/(16*16*16));
						change(&temp->op[5],(address%(16*16*16))/(16*16));
						change(&temp->op[6],(address%(16*16))/16);
						change(&temp->op[7],address%16);
					}
					else{											//3형식일때
						temp->len=6;								//len 6으로 설정후 int값 16진수 형식 char로 6개 저장
						change(&temp->op[0],op1);
						change(&temp->op[1],op2);
						change(&temp->op[2],middle);	
						if(address>=0){
							change(&temp->op[3],address/(16*16*16));
							change(&temp->op[4],(address%(16*16))/16);
							change(&temp->op[5],address%16);
						}
						else{										//주소 음수일때
							int ee[3];
							ee[0]=15-address1/16/16;				//2의보수법이용 15에서 빼면서 반전시킴
							ee[1]=15-(address1%(16*16))/16;
							ee[2]=15-(address1%16)+1;
							if(ee[2]==16){							//1더해줌
								ee[1]++;
								ee[2]=0;
							}
							if(ee[1]==16){
								ee[0]++;
								ee[1]=0;
							}
							change(&temp->op[3],ee[0]);				//저
							change(&temp->op[4],ee[1]);
							change(&temp->op[5],ee[2]);
						}
				
					}
				}
			}
		}
		address=0;
		address1=0;
		temp=temp->next;
	}
	return 0;
}


int symbolcheck(char *array){				//symbol있는지 판단
	symnode* temp = symhead;
	while(temp!=NULL){
		if(strcmp(array, temp->name)==0)	//있으면 그 symbol 의 loc 리턴
			return temp->loc;
		temp=temp->next;
	}
	return -1;
}

void insertsymbol(char *array, int loc){				//symbol linkedlist에 저장
	symnode* new = (symnode*)malloc(sizeof(symnode));	//노드 할당
	strcpy(new->name, array);
	new->loc=loc;
	new->next=NULL;										//값저장
	symnode* temp=symhead;			

	if(symhead==NULL){									//head널이면 head가 new
		symhead=new;
		return;
	}
	else{
		if(strcmp(temp->name,array)>0){					//head랑 비교 new가 알파벳순 앞이면 앞으로 정렬
			new->next=temp;
			symhead=new;
			return;
		}
		if (temp->next==NULL){
			temp->next=new;
			return;
		}
		else if (temp->next!=NULL){
			while(temp->next->next!=NULL){					//마지막 linkedlist 전 node까지 new랑 비교후 알파벳순으로 정렬
				if(strcmp(temp->next->name,array)>0){
					new->next=temp->next;
					temp->next=new;
					return;
				}
				temp=temp->next;	
			}
		}
		if(strcmp(temp->next->name, array)>0){			//마지막 linkedlist랑 비교
			new->next=temp->next;
			temp->next=new;
			return;
		}
		else{
			temp=temp->next;							//new가 맨마지막인경우
			temp->next=new;
			return;
		}
	}
	return;
}

int type_(char* array){									//type 명령어
	DIR *dir = opendir("./");
	struct dirent *ent;
	struct stat st;
	char buffer[5]={0, } ;

	if(dir!=NULL){
		while((ent=readdir(dir))!=NULL){
			if(strcmp(ent->d_name,array)==0){
				stat(ent->d_name, &st);
				if((st.st_mode&S_IFMT)!=S_IFDIR){				//디렉터리 파일이 아닐 때
					closedir(dir);
					FILE *fp = fopen(array, "r");
					while(feof(fp)==0){							//파일 끝까지 출력
						fread(buffer,sizeof(char),4,fp);	
						printf("%s",buffer);
						memset(buffer, 0, 5);
					}
					fclose(fp);
					return 0;									//0리
				}
				else{
				}
			}
		}
	}
	printf("NONE!\n");										//파일 없으면 에러 -1리턴
	closedir(dir);
	return 1;
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
