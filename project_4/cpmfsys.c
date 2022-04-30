#include <stdint.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <string.h> 
#include <stdio.h>
#include "diskSimulator.h"
#include "cpmfsys.h"

//Global variables
uint8_t Buffer[BLOCK_SIZE];
bool FreeList[256]; 


struct dirStruct *mkDirStruct(int index,uint8_t *e){
  int row, col;
  blockRead(e,index); //Read blocks from the disk
  struct dirStruct *d = malloc(32*sizeof(DirStructType)); 
  for(row=0;row<32;row++){

	(d+row)->status = *(e+(row*32)+0);   //status

      for(col=0; col<8;col++){             //filename
		(d+row)->name[col] = (char)*(e+(row*32)+col+1);
	} 
	(d+row)->name[8] = '\0';
	
	for(col=0; col<3;col++){            //extension
		(d+row)->extension[col] = (char)*(e+(row*32)+col+9); 
	}
	(d+row)->extension[3] = '\0';

      (d+row)->XL = *(e+(row*32)+12);
	(d+row)->BC = *(e+(row*32)+13);
	(d+row)->XH = *(e+(row*32)+14);
	(d+row)->RC = *(e+(row*32)+15);
	
	for(col=0; col<16;col++){        //blocks
		(d+row)->blocks[col] = *(e+(row*32)+16+col); 
	}
	
  }
  return d;
}


void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e){
  int row, col;
   
  for(row=0;row<32;row++){

	 *(e+(row*32)+0) = (d+row)->status ;   //status

      for(col=0; col<8;col++){             //filename
		*(e+(row*32)+col+1) = (uint8_t)(d+row)->name[col];
	} 
	
	
	for(col=0; col<3;col++){            //extension
		*(e+(row*32)+col+9) = (uint8_t)(d+row)->extension[col]; 
	}
	

      *(e+(row*32)+12) = (d+row)->XL;
	*(e+(row*32)+13) = (d+row)->BC;
	*(e+(row*32)+14) = (d+row)->XH;
	*(e+(row*32)+15) = (d+row)->RC;
	
	for(col=0; col<16;col++){        //blocks
		*(e+(row*32)+16+col) = (d+row)->blocks[col]; 
	}
	
  }
  blockWrite(e,index);          //Write buffer back to the Disk
  free(d);                    //Free memory
}



 
void makeFreeList(){
  int i,j;
  DirStructType *d;
  d = mkDirStruct(0,Buffer);

  for(i=0;i<NUM_BLOCKS;i++){FreeList[i]=false;} 

  for(i=0;i<32;i++){
	if((d+i)->status != 229){
		for(j=0;j<16;j++){	
			FreeList[(d+i)->blocks[j]]=true; 		
		}
 	}
  }
  writeDirStruct(d,0,Buffer);  	
}



void printFreeList(){
  int i,j,k=0;
  printf("FREE BLOCK LIST: (* means in-use) \n");
  for(i=0;i<16;i++){
	printf("%2x : ",i<<4); 
  	for(j=0;j<16;j++){
		if(FreeList[k]==true) //Print marks of FreeList
			printf(" * ");
		else
			printf(" . ");
		k++;
	}
	printf("\n");
  }
  printf("\n");
}



int findExtentWithName(char *name, uint8_t *block0){
  int i,j,k;
  char name_cpy[15];
  DirStructType *d;
  d = mkDirStruct(0,block0); //Load directory structure
  for(i=0;i<32;i++){
	j=0;
	k=0;
	if((d+i)->status != 229){
		while(j!=8 && (d+i)->name[j]!=' '){
			name_cpy[k] = (d+i)->name[j]; //Concate name for name.extension
			j++;
			k++;
		}
		
		name_cpy[k] = '.'; 
		k++;
		j=0;
		while(j!=4 && (d+i)->extension[j]!=' '){
			name_cpy[k] = (d+i)->extension[j]; 
			j++;
			k++;
		}
		if(j==0)
			name_cpy[k-1]='\0'; 
		
  		if(strcmp(name_cpy,name)==0){  
			free(d);
			return i; 
		}
		
	}
  }
  free(d);
  return -1;
}


bool checkLegalName(char *name){

  int i=0,length,name_len=0,ext=0;
  
  length = strlen(name);

// The first character should be between A to Z

  if(!(name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z')){
  	return false;	
  }

while(i<length && name[i]!='.'){
  name_len++;
i++;
}
i++;

while(i<length){
ext++;
i++;

}
  if(name_len>9 || ext>4) {
	return false;}
 
 return true;	
}


void cpmDir(){
  int i,j=0,k,s;
  DirStructType *d;
  int NumBlocks=0;
  int TotalBytes;
  d = mkDirStruct(0,Buffer);
  printf("*DIRECTORY LISTING*\n");
  for(i=0;i<32;i++){
      k=0;
	j=0;
	NumBlocks = 0;
	if((d+i)->status != 229){ 
		while(j!=8 && (d+i)->name[j]!=' '){
			printf("%c",(d+i)->name[j]);
			j++;
		}
		printf(".");
		j=0;
		while(j!=4 && (d+i)->extension[j]!=' '){
			printf("%c",(d+i)->extension[j]); 
			j++;
		}
		//name_cpy[k]='\0';
		
		for(s=0;s<16;s++){
			if((d+i)->blocks[s]>0) 
			   NumBlocks++;	
		}
		TotalBytes = (1024*(NumBlocks-1)) + (128*(d+i)->RC) + (d+i)->BC; 
		 
		printf("  %d \n",TotalBytes);
	}
  }
  writeDirStruct(d,0,Buffer);
  printf("\n");
}



int cpmRename(char *oldName, char *newName){
  int extent_index;
  
  char name[9];
  char extension[4];
  int i=0,length,k=0;
  
  length = strlen(newName);
  DirStructType *d;
  d = mkDirStruct(0,Buffer);
  if(checkLegalName(newName)==0)
	return -2;
  if((extent_index = findExtentWithName(oldName,Buffer)) == -1){
      printf("%s doesn't exsist\n",oldName);
	return -1;
	}
  if(findExtentWithName(newName,Buffer) != -1){
	printf("%s already exsists\n",newName);
	return -3;
	}

while(i<length && newName[i]!='.'){
  name[k++] = newName[i];
i++;
}
name[k]='\0';
i++;
k=0;
while(i<length){
extension[k++] = newName[i]; 
i++;
}
extension[k]='\0';

  printf("RENAMING file \"%s\" to \"%s\"...",oldName,newName);
  strcpy((d+extent_index)->name,name);
  strcpy((d+extent_index)->extension,extension);
  writeDirStruct(d,0,Buffer);
  printf("Done\n");
  return 0;
}


int  cpmDelete(char * name){
  int extent_index;
  DirStructType *d;
  if((extent_index = findExtentWithName(name,Buffer)) == -1){
      printf("%s File Not Found To Delete\n",name);
	return -1;}
  else
 	d = mkDirStruct(0,Buffer);
  	(d+extent_index)->status = 229;
  	printf("File \"%s\" is deleted.\n",name);
  	writeDirStruct(d,0,Buffer);
  	makeFreeList();
  return 0;
}
