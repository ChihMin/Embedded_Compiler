#include <stdio.h>

int main(int argc, const char *argv[]){
    int A[20], B[20], C[20];
    int x = 10; 
    int y = 7;
    int z = 2;
    for (int i = 3; i < 20; ++i) {
        A[i] = B[i];
        C[i-2] = A[i];
    }
    return 0;
}
