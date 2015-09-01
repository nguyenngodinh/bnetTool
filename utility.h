/*
 * utility.h
 *
 *  Created on: May 18, 2015
 *      Author: trungvv1
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <string>
#include <vector>
#include <sstream>

using namespace std;

template <typename T>
struct PtrLess // public std::binary_function<bool, const T*, const T*>
{
  bool operator()(const T* a, const T* b) const
  {
    // may want to check that the pointers aren't zero...
    return *a < *b;
  }
};

class Utility {
public:
    /// io utils
    static vector<vector<double>> readTsvData(const string & filename);
    static vector<vector<uint>> readTsvUintData(const string & filename);
    static void writeTsvData(const string & filename, const vector<vector<double>> & data);
    static void writeTsvUintData(const string & filename, const vector<vector<uint>> &data);
    static void writeString(const string & fileName, const string &content);

    /// bayes reshape
    static uint vectorToNumber(const vector<uint> &numValues,
            const vector<uint> &values);
    static vector<uint> numberToVector(const vector<uint> & numValues, uint value);
};

#endif /* UTILITY_H_ */
