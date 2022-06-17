#include <iostream>
#include <vector>
#include <cstdio>
#include <numeric>
#include <functional>
#include <time.h>

using namespace std;

#define printLine() printf("FILENAME: %s, FUNC:%s, LineNo:%d\n", __FILE__, __FUNCTION__, __LINE__)

inline void printTime() {
    time_t mtime = time(NULL);
    cout << "[" << ctime(&mtime)<< "]" << endl;
}

template <class T>
void printVector(T& vector){
    cout << "[";
    for (size_t i=0;i < vector.size();i++){
        cout << vector.at(i);
        if (i!=vector.size()-1) cout << ", ";
    }
    cout << "]" << endl;
}

template <class T>
T max_v(vector<T> vector){
    return *max_element(vector.begin(), vector.end());
}

template <class T>
T min_v(vector<T> vector){
    return *min_element(vector.begin(), vector.end());
}

template <class T>
double mean_v(vector<T> vector){
    return accumulate(vector.begin(), vector.end(), 0)/(float)vector.size();
}