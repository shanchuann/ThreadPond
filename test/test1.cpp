#include "FixedThreadPool.hpp"
#include <vector>
#include <algorithm>
#include <assert.h>
#include <iostream>

const int ROWSIZE = 10000;
const int COLSIZE = 1000;

void random(std::vector<int> *pvec){
    assert(pvec != nullptr);
    pvec->clear();
    pvec->reserve(COLSIZE);
    for(int i = 0; i < COLSIZE; ++i){
        pvec->push_back(rand() % 10000);
    }
}
void my_sort(std::vector<int> &vec){
    std::sort(vec.begin(), vec.end());
}
void print_vec(const std::vector<int> &vec){
    for(size_t i = 0; i < vec.size(); ++i){
        printf("%5d ", vec[i]);
        if((i + 1) % 20 == 0){
            printf("\n");
        }
    }
    printf("------------------------------------------------------------------------------------------\n");
}
int main()
{
    // 使用块作用域，使线程池在退出作用域时被析构，确保所有任务完成后再打印结果
    std::vector<std::vector<int>> matrix(ROWSIZE);
    for(int i = 0; i < ROWSIZE; ++i){
        random(&matrix[i]);
    }

    {
        shanchuan::FixedThreadPool pool; // 默认使用硬件并发数
        for(int i = 0; i < ROWSIZE; ++i){
            // 按值捕获 i，按引用捕获 matrix
            pool.add_task([&matrix, i]() {
                my_sort(matrix[i]);
            });
            // debug: 标记已提交任务
        }
        // 在此块结束时 pool 被销毁，析构函数会等待任务完成并 join 线程
    }
    std::cout << "pool scope ended\n";

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