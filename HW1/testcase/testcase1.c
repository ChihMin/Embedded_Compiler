#include <stdio.h>

int main(int argc, const char *argv[]){
    int A[20], B[20], C[20];
    int x = 10; 
    int y = 7;
    for (int i = 3; i < 20; ++i) {
        B[i] = A[i-1];
        A[i-2] = B[i-3];
        C[i-2] = A[i-1];
        B[i-2] = C[i-3];
    }
    return 0;
}
