#pragma once
#include <algorithm>


template<typename T>
T clamp(T val, T mn, T mx){
return std::max(std::min(val, mx), mn);
}

template<typename T, uint8_t LEN>
void set_all_values_in_array(T (&arr)[LEN], T val) {
    for (T& value : arr) {
        value = val;
    }
}
