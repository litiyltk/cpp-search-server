// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 271

// Закомитьте изменения и отправьте их в свой репозиторий.

// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?

#include <iostream>

using namespace std;

int count_three_in_number(int x){
    int count = 0;
    while (x > 0){
        if (x%10 == 3){
            ++count;
        }
        x /= 10;
    }
    return count;
}

int main()
{
    int count = 0;
    for (int x=1; x<=1000; ++x){
        if (count_three_in_number(x) >= 1){
            ++count;
        }
    }
    cout << count << endl;
    return 0;
}