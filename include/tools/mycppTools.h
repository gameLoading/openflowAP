#include <iostream>
#include <vector>
#include <cstdio>
#include <numeric>
#include <functional>
#include <time.h>
#include <queue>
#include <array>
#include <iterator>

using namespace std;

#define printLine() printf("FILENAME: %s,a FUNC:%s, LineNo:%d\n", __FILE__, __FUNCTION__, __LINE__)

inline void printTime() {
    time_t mtime = time(NULL);
    string str = ctime(&mtime);
    str.pop_back();
    cout << endl << "[" << str << "]" << endl;
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
void printQueue(T& q){
    cout << "[";
    for (auto e = q.begin();e != q.end();e++){
        cout << *e;
        if (e!=q.end()-1) cout << ", ";
    }
    cout << "]" << endl;
}

template <typename T>
string printArray(T arr[], int size){
    string str;
    str.append("[");
    for (int i=0;i < size;i++) {
        str.append(to_string(arr[i]));
        if (i!=size-1) str.append(", ");
    }
    str.append("]");
    return str;
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
