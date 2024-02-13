//
// Created by vladislav on 21.01.24.
//

#include "Logger.h"
#include "iostream"
using namespace std;
template <typename T>
void info(T log) {
    cout << "[\x1B[94mINFO\x1B[37m] " << log << '\n';}
template <typename T>
void warn(T log) {
    cout << "[\x1B[93mWARN\x1B[37m] " << log << '\n';}
template <typename T>
void fatal(T log) {
    cout << "[\x1B[91mFATAL\x1B[37m] " << log << '\n';
    exit(1);}
template <typename T>
void sucess(T log) {
    cout << "[\x1B[92mSUCCESS\x1B[37m] " << log << '\n';}
template <typename T>
void debug(T log) {
    cout << "[\x1B[95mDEBUG\x1B[37m] " << log << '\n';
}
