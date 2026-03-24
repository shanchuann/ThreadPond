#include "FixedThreadPool.hpp"
#include <vector>
#include <algorithm>
#include <assert.h>
const int ROWSIZE = 10000;
const int COLSIZE = 1000;

void random(std::vector<int> *pvec){
    assert(pvec != nullptr);
    for(int i = 0; i < ROWSIZE * COLSIZE; ++i){
        pvec->push_back(rand() % 10000);
    }
}
void my_sort(std::vector<int> &vec){
    std::sort(vec.begin(), vec.end());
}
void print_vec(const std::vector<int> &vec){
    for(int i = 0; i < vec.size(); ++i){
        printf("%5d ", vec[i]);
        if((i + 1) % 20 == 0){
            printf("\n");
        }
    }
    printf("-----------------------------------------------------\n");
}
int main()
{
    std::vector<std::vector<int>> matrix(ROWSIZE);
    for(int i = 0; i < ROWSIZE; ++i){
        matrix[i].resize(COLSIZE);
        random(&matrix[i]);
    }
    for(int i = 0; i < ROWSIZE; ++i){
        my_sort(matrix[i]);
    }
    for(int i = 0; i < ROWSIZE; ++i){
        print_vec(matrix[i]);
    }
    return 0;
}
#if 0
void funa()
{
    std::cout << "Hello from funa!" << std::endl;
}
void funb(int n)
{
    for(int i = 0; i < n; ++i)
    {
        std::cout << "Hello from funb!" << std::endl;
    }
}
int main()
{
    shanchuan::FixedThreadPool pool;
    pool.add_task(funa);
    pool.add_task(std::bind(funb, 5));
    
    return 0;
}
#endif