#ifndef LEARNBAYES_H
#define LEARNBAYES_H

#include <vector>
#include <string>
#include <sys/types.h>

using namespace std;

class BayesNet;

/**
 * @brief The LearnBayes class - using ".learn" file
 * Used for learning with one data corpus, using uniform prior is available.
 * Update or online-learning have not yet supported.
 */
class LearnBayes
{
public:
    explicit LearnBayes(BayesNet *bn, const string & learnFile);
    explicit LearnBayes(BayesNet *bn, double prior = 0.0, double cptWeight = 0.0);

    void learn(const vector<vector<uint>> & samples);
    void learn(const string & datafile, uint iter=1);

    void save(const string &learnFile) const;
    string toString() const;

private:
    BayesNet *mBayesNet;
    vector<double> mCaseCounter;                       // count the number of time a case appears
    vector<vector<uint>> mMapIndex2Case;              // map an index (a number) to the case value vector (multi-dim array)

    void loadFromTextFile(const string & learnFile);
    void loadFromBinFile(const string & learnFile);
    void initCaseCounterBasedOnPriorCpts(double prior = 0.0, double cptWeight = 0.0);

};

#endif // LEARNBAYES_H
