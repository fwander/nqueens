// Finn Wander

//compile with: gcc -O6 -flto -fopenmp nqueens.c
//to get solution:
//for a in {4..36}; do echo $a; time ./a.out $a 1; done 2>&1 | tee outBoards.txt
//to get number of solutions:
//for a in {4..16}; do echo $a; time ./a.out $a 0; done 2>&1 | tee outNumbers.txt

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef struct{
  int* board;
  int size;
  int row;
  int back;
  char** attacked;
} B;
int size;
int stn(char* in){  // string to number
  int result = 0;


  while(isdigit(*in)){
    result *= 10;
    result += (int) (*in - '0');
    in++;

  }

  in--;
  return result;
}

void printBoard(B* b){
  for(int i = 0; i < size; i++){
    for(int s = 0; s < size; s++){
      if(s == *(b->board+i)){
        printf("Q");
      }
      else{
        printf(".");
      }
    }
    printf("%d\n", *(b->board+i));
  }
  printf("---\n" );

  for(int i = 0; i < size; i++){
    for(int s = 0; s < size; s++){
      printf("%d", b->attacked[i][s]);
    }
    printf("\n");
  }
}

void setRow(B* b){
  if(b->back || b->row == 0){

    b->back = 0;
    return;
  }
  int Q;
  unsigned int dist;
  int right;
  int left;

  memset(b->attacked[b->row], 0, sizeof(char) * size);

  for(int i = 0; i != b->row - 1; i++){
      Q = b->board[i];
      b->attacked[b->row][Q] = 1;
      dist = (b->row - i);
      right = Q + dist;
      left = Q - dist;
      *(b->attacked[b->row]+right) = 1;
      *(b->attacked[b->row]+left) = 1;
  }
  //if there are more than one queens attacking here, it matters only if it doubles up in the last row
  Q = b->board[b->row-1];
  b->attacked[b->row][Q]++;
  (*(b->attacked[b->row]+(Q-1)))++;
  (*(b->attacked[b->row]+(Q+1)))++;

}

int okmove(int placeLocation,B* b){
    return placeLocation >= size? 1 : !b->attacked[b->row][placeLocation];
}



void nextmove(B* b){ //places the  queen in the next legal position, and updates attacked
    if(b->back && b->row != size-1){
      int Q = b->board[b->row]-1;
      b->attacked[b->row+1][Q]--;
      (*(b->attacked[b->row+1]+(Q-1)))--;
      (*(b->attacked[b->row+1]+(Q+1)))--;
      while(!okmove(b->board[b->row],b)){b->board[b->row]+=strlen(b->attacked[b->row]+b->board[b->row]);}
      Q = b->board[b->row];
      if (Q >= size) return;
      b->attacked[b->row+1][Q]++;
      (*(b->attacked[b->row+1]+(Q-1)))++;
      (*(b->attacked[b->row+1]+(Q+1)))++;
      return;
    }
  while(!okmove(b->board[b->row],b)){b->board[b->row]+=strlen(b->attacked[b->row]+b->board[b->row]);}

}

void nextboard(B* b){ //get next legal board backtracking necessary
  nextmove(b);
  if(b->board[b->row] >= size){
      b->board[b->row] = 0;
      b->row--;
      b->board[b->row]++;
      b->back = 1;
      nextboard(b);
  }
}

void solve(B* b){ //keep on setting up the attacked spaces and getting the next legal board until you've got a legal board at row == size
    while(b->row != size) {
        setRow(b);
        nextboard(b);
        b->row++;
    }
}

int count(B* b, int pos){ //keep solving at column pos
  int back = 0;

  b->board[0] = pos;
  while(b->board[0] == pos){
    solve(b);
    b->row--;
    back++;
    b->board[b->row]++;
  }
  back--;

  return back;
}

void setup(B** in, int numBoards){ //set up the boards so the multiple cores dont step on each other
  for(int i = 0; i < numBoards; i++){
    B* b = (B*)malloc(sizeof(B));
    b->board = (int*) calloc(size,sizeof(int));
    b->attacked = (char**)(calloc(size,sizeof(char*)));
    for (int i = 0; i < size; i++){
      b->attacked[i] = (char*)(calloc(size+i,sizeof(char)));
    }
    b->row = 0;
    *(in+i) = b;
  }
}

int main(int argc, char** argv){
  if(argc != 3){
    printf("use: ./a.out [board size] [1 for first solution, 0 for all solutions]\n");
    return 0;
  }
  size = stn(argv[1]);
  int option = stn(argv[2]);
  B** list = (B**) malloc((size/2+1)*sizeof(B**));

  if (option){
    setup(list,1);
    solve(*list);
    printBoard(*list);
  }
  else{
    setup(list,(size/2+1));
    long long unsigned int* total = (long long int*)calloc(size/2+1,sizeof(long long int));
    int n = size/2 + (size%2 == 0? 0 : 1);
    #pragma omp parallel for
    for(int i = 0 ; i < n; i++){
      *(total+i) += count(*(list+i),i);
    }
    int totaltotal = 0;
    for (int i = 0 ; i < size/2; i++){
      totaltotal += *(total+i);
    }
    totaltotal += *(total+size/2) >> 1;
    printf("%d\n", totaltotal + totaltotal);
  }
  free(list);
}
