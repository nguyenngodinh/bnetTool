#include "learnbayes.h"
#include "bayesnet.h"
#include "utility.h"

#include <iostream>
#include <fstream>
#include <algorithm>

LearnBayes::LearnBayes(BayesNet *bn, const string &learnFile)
{
    mBayesNet = bn;
    loadFromTextFile(learnFile);
}

LearnBayes::LearnBayes(BayesNet *bn, double prior, double cptWeight)
{
    mBayesNet = bn;
    initCaseCounterBasedOnPriorCpts(prior, cptWeight);
}

/**
 * @brief LearnBayes::learnParams
 * learn CPTs for each node from complete sample data
 * @param samples
 */
void LearnBayes::learn(const vector<vector<uint>> & samples)
{
    if (mCaseCounter.size() != mBayesNet->mListJointProbabilities.size())
        initCaseCounterBasedOnPriorCpts();

    /// count the number of times each case appears
    uint index;
    vector<uint> maxValues;
    for (const auto & sample : samples) {
        if (std::find(sample.begin(), sample.end(), NONE_NODE_INDEX) == sample.end()) {
            /// complete data
            index = Utility::vectorToNumber(mBayesNet->mMapNodeSizes, sample);
            mCaseCounter[index] += 1;
        } else {
            /// EM
            vector<uint> mapEvidences, postValues, indexPreValues, maxPreValues;
            uint size = 1;
            postValues = sample;
            for (uint i=0; i < sample.size(); ++i) {
                if (sample[i] != NONE_NODE_INDEX) {
                    mapEvidences.push_back(i);
                } else {
                    size *= mBayesNet->mMapNodeSizes[i];
                    maxPreValues.push_back(mBayesNet->mMapNodeSizes[i]);
                    indexPreValues.push_back(i);
                }
            }
            for (uint i = 0; i < size; ++i) {
                auto v = Utility::numberToVector(maxPreValues, i);
                for (uint j = 0; j < v.size(); ++j) {
                    postValues[indexPreValues[j]] = v[j];
                }
                auto p = mBayesNet->getConditionalProbability(mapEvidences, postValues);
                ///@
//                vector<uint> eIndice;
//                vector<int> eVals;
//                for(uint j=0; j<postValues.size()-1; j++)
//                {
//                    eIndice.push_back(j);
//                    eVals.push_back(postValues.at(j));
//                }
//                mBayesNet->updateMul(eIndice, eVals);
                ///@
                index = Utility::vectorToNumber(mBayesNet->mMapNodeSizes, postValues);
                mCaseCounter[index] += p;
            }

        }
    }
    ///

    /// count the number of time a group of values related to a node appears
    vector<vector<double>> nodeCounter;
    vector<vector<double>> parentCounter;

    /// init counters if not defined
    uint size;
    for (uint nodeId = 0; nodeId < mBayesNet->mNumNodes; ++nodeId) {
        size = 1;
        for (const auto & paNode : mBayesNet->mMapParents[nodeId]) {
            size *= mBayesNet->mMapNodeSizes[paNode];
        }
        nodeCounter.push_back(vector<double>(size*mBayesNet->mMapNodeSizes[nodeId], 0));
        parentCounter.push_back(vector<double>(size, 0));
    }

    for (uint nodeId = 0; nodeId < mBayesNet->mNumNodes; ++nodeId) {
        maxValues.clear();
        for (const auto & paNode : mBayesNet->mMapParents[nodeId]) {
            maxValues.push_back(mBayesNet->mMapNodeSizes[paNode]);
        }
        maxValues.push_back(mBayesNet->mMapNodeSizes[nodeId]);

        for (uint i = 0; i < mCaseCounter.size(); ++i) {
            vector<uint> values;
            for (const auto & paNode : mBayesNet->mMapParents[nodeId]) {
                values.push_back(mMapIndex2Case[i][paNode]);
            }
            values.push_back(mMapIndex2Case[i][nodeId]);
            index = Utility::vectorToNumber(maxValues, values);

            nodeCounter[nodeId][index] += mCaseCounter[i];
            parentCounter[nodeId][ index / mBayesNet->mMapNodeSizes[nodeId] ] += mCaseCounter[i];
        }
    }
    ///

    /// setup CPT for BayesNet
    vector<vector<double>> cpt;
    for (uint nodeId = 0; nodeId < mBayesNet->mNumNodes; ++nodeId) {
        cpt.push_back(vector<double>());
        size = nodeCounter[nodeId].size();
        for (uint i = 0; i < size; ++i) {
            index = i / mBayesNet->mMapNodeSizes[nodeId];
            cpt[nodeId].push_back( nodeCounter[nodeId][i] / parentCounter[nodeId][index] );
        }
    }
    mBayesNet->setCpt(cpt);
}

/**
 * @brief LearnBayes::learn
 * EM learning
 * @param datafile
 * @param iter - the likelihood must increase after each iteration
 */
void LearnBayes::learn(const string &datafile, uint iter)
{
    auto samples = Utility::readTsvUintData(datafile);
    while (iter--)
        learn(samples);
}

void LearnBayes::save(const string &learnFile) const
{
    ofstream of;
    of.open(learnFile, ios::binary | ios::out);
    if (!of.is_open()) {
        cout << "Write file error!" << "\n";
        return;
    }

    uint size = mCaseCounter.size();   /// caseCounter
    of.write((char*)&size, sizeof(size));
    for (const auto & p : mCaseCounter) {
        of.write((char*)&p, sizeof(p));
    }

    of.close();
    cout << "Save " << learnFile << " successfully."<< "\n";
}

string LearnBayes::toString() const
{
    stringstream content;
    content << mCaseCounter.size() << "\n";
    for (const auto & c : mCaseCounter) {
        content << c << "\n";
    }
    return content.str();
}

void LearnBayes::loadFromTextFile(const string &learnFile)
{    
    ifstream stream(learnFile);
    if (!stream.is_open()) {
        cout << "Open file error!" << "\n";
        return;
    }

    double dValue;
    uint size = 0;
    stream >> size;
    if (size > 0) {
        /// node counter
        for (uint i = 0; i < size; ++i) {
            stream >> dValue;
            mCaseCounter.push_back(dValue);
            mMapIndex2Case.push_back(Utility::numberToVector(mBayesNet->mMapNodeSizes, i));
        }
    }
    stream.close();
    cout << "Read " << learnFile << " successfully."<< "\n";
}

void LearnBayes::loadFromBinFile(const string &learnFile)
{
    ifstream inf;
    inf.open(learnFile, ios::binary | ios::in);
    if (!inf.is_open()) {
        cout << "Open file error!" << "\n";
        return;
    }

    /// read caseCounter
    uint size = 0;
    double dValue;
    inf.read((char*)&size, sizeof(size));
    if (size > 0) {
        /// node counter
        for (uint i = 0; i < size; ++i) {
            inf.read((char*)&dValue, sizeof(dValue));
            mCaseCounter.push_back(dValue);
            mMapIndex2Case.push_back(Utility::numberToVector(mBayesNet->mMapNodeSizes, i));
        }
    }
    inf.close();
    cout << "Read " << learnFile << " successfully."<< "\n";
}

void LearnBayes::initCaseCounterBasedOnPriorCpts(double prior, double cptWeight)
{
    if (mBayesNet->mListJointProbabilities.empty())
        mBayesNet->setJointDistribution();

    auto size = mBayesNet->mListJointProbabilities.size();
    for (uint i = 0; i < size; ++i) {
        mMapIndex2Case.push_back(Utility::numberToVector(mBayesNet->mMapNodeSizes, i));
        mCaseCounter.push_back(0.0);
        mCaseCounter[i] += prior / size;
        mCaseCounter[i] += cptWeight * mBayesNet->mListJointProbabilities[i] * size;
    }
    cout << "init prior CPTs ok";
}

