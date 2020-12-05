
// Parameters
// cacheSize(power of 2) | cacheType(direct, assoc, or assoc:n where n is power of 2) | blockSize(power of 2) | replacementPolicy(LRU or FIFO) | textFile of inputs

// Return
// memoryWrites, memoryReads, cacheHits, cacheMisses


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void freeMatrix(unsigned long long int** mat, int rows){
    for(int i = 0; i < rows; i++){
        free(mat[i]);
    }
    free(mat);
}

unsigned long long int get(unsigned long long int x, unsigned long long int n){

    x = x >> n;
    return x % 2;

}
void printBinary(unsigned long long int x){
    
    int i = 0;
    while(x){

        printf("%llx", x % 2);
        x = x >> 1;
        i++;

    }
    printf("\n");
}

int onesInBitString(unsigned int num){

    int oneCount = 0;

    while(num){
        oneCount = oneCount + (num & 1);
        num = num >> 1;
    }

    return oneCount;
}

int main(int argc, char* argv[])
{

    int cacheSize = atoi(argv[1]);
    unsigned int temp = (unsigned int)cacheSize;
    if(onesInBitString(temp) != 1){
        printf("error\n");
        return 0;
    }

    char* associativity = argv[2];
    char copyAssoc[20];
    char* copyPtr = copyAssoc;
    strcpy(copyPtr, associativity);
    char* tok = strtok(copyAssoc,":");
    if(strcmp(tok, "direct") == 0){
        strcpy(associativity, "direct");
    }
    else if(strcmp(tok, "assoc") == 0){
        tok = strtok(NULL, ":");
        if(tok == NULL){
            strcpy(associativity, "assoc");
        }
        else{
            //printf("digit\n");
            unsigned int temp = atoi(tok);
            if(onesInBitString(temp) != 1){
                printf("error\n");
                return 0;
            }
            strcpy(associativity, tok);
        }
    }
    else{
        printf("error\n");
        return 0;
    }

    char* replacePolicy = argv[3];
    if(strcmp(replacePolicy, "lru") == 0 || strcmp(replacePolicy, "fifo") == 0){
        ;
    }
    else{
        printf("error\n");
        return 0;
    }

    int blockSize = atoi(argv[4]);
    unsigned int tempTwo = (unsigned int)blockSize;
    if(onesInBitString(tempTwo) != 1){
        printf("error\n");
        return 0;
    }

    FILE* fp = fopen(argv[5], "r");
    if(fp == NULL){
        printf("error\n");
        return 0;
    }

    //calculate E, S as in lecture slides

    int E = 0; //line
    int S = 0; //set
    
    if(strcmp(associativity, "direct") == 0){
        S = cacheSize / blockSize;
        E = 1; //E is 1 because there it is a direct mapping
    }
    else if(strcmp(associativity, "assoc") == 0){
        E = cacheSize / blockSize;
        S = 1; //Since it is associative, the properties are flipped
    }
    else{ //if it is assoc:n AKA n-way

        E = atoi(associativity); // convert to integer from char*
        S = cacheSize / (blockSize * E);

    }

    int tagBits;
    int setBits;
    int blockBits;

    blockBits = log2(blockSize);
    setBits = log2(S);
    tagBits = 48 - (blockBits + setBits);

    //cacheSize = S * E * B : where S = sets, E = lines per set, B = blockSize

    //box will hold tag set block and MAYBE valid bit values
    //each set will have E number of boxes
    //sets are similar to rows and there are E number of boxes per row

    // box* cache[S][E] --> box* cache[sets][linesPerSet] --> the values are pointers to a struct called box which hold tag, set, and block
    
    unsigned long long int** mainCache = malloc(S * sizeof(unsigned long long int*)); // make the array that holds setNodes that contain set string AND pointer to the head of a tagNode
    for(int i = 0; i < S; i++){
        mainCache[i] = malloc(E * sizeof(unsigned long long int));
    }

    for(int i = 0; i < S; i++){
        for(int j = 0; j < E; j++){
            mainCache[i][j] = -1;
        }
    }
    
    char programCounter[100];
    char action[2];
    unsigned long long int hex;

    int memoryRead = 0;
    int memoryWrite = 0;
    int cacheHit = 0;
    int cacheMiss = 0;

    while(!feof(fp)){
        
        char c = fgetc(fp);
        if(c == '#'){
            break;
        }
        
        fscanf(fp, "%s %s %llx\n", programCounter, action, &hex);
        //printf("%s %s %llx\n", programCounter, action, hex);
        
        unsigned long long int tempHex = hex;
        unsigned long long int curTag = tempHex >> (48 - tagBits);
        tempHex = hex;
        unsigned long long int set = (tempHex >> blockBits) & ~~((1 << setBits) - ((setBits + 1) - setBits));
         
        if(strcmp(replacePolicy, "lru") == 0){

            int actionInLoop = 1;
            int hitStatus = 1;
            int i;
            for(i = 0; i < E; i++){

                if(mainCache[set][i] == curTag){
                    if(strcmp(action, "W") == 0){
                        memoryWrite++;
                    }
                    else if(strcmp(action, "R") == 0){
                        ;
                    }
                    cacheHit++;
                    actionInLoop = 0;
                    hitStatus = 0;
                    break;
                }
                
                if(mainCache[set][i] == -1){
                    cacheMiss++;
                    if(strcmp(action, "W") == 0){
                        memoryWrite++;
                        memoryRead++;
                        
                    }
                    else if(strcmp(action, "R") == 0){
                        memoryRead++;
                    }
                    mainCache[set][i] = curTag;
                    actionInLoop = 0;
                    break;
                }
            }

            if(hitStatus == 0){

               

                  unsigned long long int tempTag = mainCache[set][i];

                  for(int j = i; j < E; j++){
                      if((j+1) < E){
                          mainCache[set][j] = mainCache[set][j+1];
                      }
                  }

                  //shift is done, now place back into most recent front
                  /*
                  int k;
                  for(k = E; k >= 0; k--){
                      if((k-1) >= 0){
                          if(mainCache[set][k-1] != -1){
                              break;
                          }
                      }
                  }
                  k++;
                  mainCache[set][k] = tempTag;
                  */


                  int k = E;

                  while(k >= 0){

                      if(k == 0){
                          mainCache[set][k] = tempTag;
                          break;
                      }

                      if( (k-1) >= 0){

                          if( mainCache[set][k-1] != -1){

                              if(k == E){
                                  mainCache[set][k-1] = tempTag;
                              }
                              else{
                                  mainCache[set][k] = tempTag;
                              }

                              break;
                          }

                      }

                      k = k - 1;

                  }


                
            }

            else if(actionInLoop == 1){ //if nothing happened in the for loop, there is already a miss and the cache is full
                cacheMiss++;
                if(strcmp(action, "W") == 0){
                    memoryWrite++;
                    memoryRead++;
                   
                }
                else if(strcmp(action, "R") == 0){
                    memoryRead++;
                }
                for(int j = 0; j < E; j++){
                    if((j+1) < E){
                        mainCache[set][j] = mainCache[set][j+1];
                    }
                }
                mainCache[set][E-1] = curTag;

            }

            
        }
        else if(strcmp(replacePolicy, "fifo") == 0){
            
            int actionInLoop = 1;
            //int hitStatus = 1;
            int i;
            for(i = 0; i < E; i++){

                if(mainCache[set][i] == curTag){
                    if(strcmp(action, "W") == 0){
                        memoryWrite++;
                    }
                    else if(strcmp(action, "R") == 0){
                        ;
                    }
                    cacheHit++;
                    actionInLoop = 0;
                    //hitStatus = 0;
                    break;
                }
                
                if(mainCache[set][i] == -1){
                    cacheMiss++;
                    if(strcmp(action, "W") == 0){
                        memoryWrite++;
                        memoryRead++;
                        
                    }
                    else if(strcmp(action, "R") == 0){
                        memoryRead++;
                    }
                    mainCache[set][i] = curTag;
                    actionInLoop = 0;
                    break;
                }
            }

            
            if(actionInLoop == 1){ //if nothing happened in the for loop, there is already a miss and the cache is full
                cacheMiss++;
                if(strcmp(action, "W") == 0){
                    memoryWrite++;
                    memoryRead++;
                   
                }
                else if(strcmp(action, "R") == 0){
                    memoryRead++;
                }
                for(int j = 0; j < E; j++){
                    if((j+1) < E){
                        mainCache[set][j] = mainCache[set][j+1];
                    }
                }
                mainCache[set][E-1] = curTag;

            }

            
            
        }
        else{
            ;
        }

    }

    fclose(fp);

    printf("Memory reads: %d\n", memoryRead);
    printf("Memory writes: %d\n", memoryWrite);
    printf("Cache hits: %d\n", cacheHit);
    printf("Cache misses: %d\n", cacheMiss);

    

    for(int i = 0; i < S; i++){
        free(mainCache[i]);
    }
    free(mainCache);





}
