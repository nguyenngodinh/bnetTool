/*
 * utility.cpp
 *
 *  Created on: May 19, 2015
 *      Author: trungvv1
 */

#include "utility.h"

#include <fstream>
#include <iostream>
#include <cstdio>
#include <cmath>

/**
 * @brief Utility::readTsvData
 * TSV file with first line storing the number of rows and columns
 * @param filename
 * @return
 */
vector<vector<double>> Utility::readTsvData(const string & filename) {
    vector<vector<double>> data;

    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Open file error!" << endl;
        return data;
    }

    uint m, n;
    double value;
    infile >> m >> n;
    for (uint i = 0; i < m; i++) {
        vector<double> row;
        for (uint j = 0; j < n; j++) {
            infile >> value;
            //			cout << value << endl;
            row.push_back(value);
        }
        data.push_back(row);
    }

    infile.close();
    //	cout << data[1][4];
    cout << "Read " << filename << " data successfully." << endl;
    return data;
}

vector<vector<uint>> Utility::readTsvUintData(const string &filename)
{
    vector<vector<uint>> data;

    ifstream infile(filename);
    if (!infile.is_open()) {
        cout << "Open file error!" << endl;
        return data;
    }

    uint m, n;
    uint value;
    infile >> m >> n;
    for (uint i = 0; i < m; i++) {
        vector<uint> row;
        for (uint j = 0; j < n; j++) {
            infile >> value;
            //			cout << value << endl;
            row.push_back(value);
        }
        data.push_back(row);
    }

    infile.close();
    cout << "Read " << filename << " data successfully." << endl;
    return data;
}

void Utility::writeTsvData(const string & filename, const vector<vector<double>> & data) {
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Write file error!" << endl;
        return;
    }
    if (data.empty())
        return;

    outfile << data.size() << "\t" << data[0].size() << endl;
    for (const auto & row : data) {
        for (const auto & value : row) {
            outfile << value << "\t";
        }
        outfile << endl;
    }
    outfile.close();
    cout << "Write " << filename << " data successfully."<< endl;
}

void Utility::writeTsvUintData(const string &filename, const vector<vector<uint>> &data)
{
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cout << "Write file error!" << endl;
        return;
    }

    outfile << data.size() << "\t" << data[0].size() << endl;
    for (const auto & row : data) {
        for (const auto & value : row) {
            outfile << value << "\t";
        }
        outfile << endl;
    }
    outfile.close();
    cout << "Write " << filename << " uint data successfully."<< endl;
}

void Utility::writeString(const string& filename, const string &content) {
    ifstream infile(filename);
    if (infile.good())
    {
        cout << "file " << filename <<" has already existed! removing file..."<< endl;
        if (std::remove(filename.c_str())) {
            cout << "cannot remove " << filename<< endl;
        }
    }
    infile.close();

    ofstream outfile(filename);
    if (!outfile.is_open())
    {
        cout << "Open file error!"<< endl;
        return;
    }

    // Start writing data
    outfile << content;

    outfile.close();
    cout << "Write " << filename << " file successfully."<< endl;
}

uint Utility::vectorToNumber(const vector<uint>& numValues, const vector<uint>& values) {
    uint result = 0;
    if (!numValues.empty())
    {
        uint n = numValues.size();
        uint multiplier = 1;

        for (uint i = n; i >= 1; --i) {
            result += values.at(i-1) * multiplier;
            multiplier *= numValues.at(i-1);
        }
    }

    return result;
}

vector<uint> Utility::numberToVector(const vector<uint>& numValues, uint value) {
    vector<uint> results;
    if (!numValues.empty())
    {
        uint n = numValues.size();
        uint multiplier = 1;

        for (uint i = n - 1; i >= 1; --i) {
            multiplier *= numValues.at(i);
        }
        for (uint i = 1; i < n; ++i) {
            uint d = value % multiplier;
            results.push_back((value - d) / multiplier);
            value = d;
            multiplier /= numValues.at(i);
        }
        results.push_back(value);
    }

    return results;
}
