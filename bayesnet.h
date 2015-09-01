/*
 * bayesnet.h
 *
 *  Created on: May 25, 2015
 *      Author: trungvv1
 */

#ifndef BAYESNET_H_
#define BAYESNET_H_

#include <vector>
#include <map>
#include <string>
#include <sys/types.h>

using namespace std;

const uint NONE_NODE_INDEX = -1;        /// = 4294967295

class BayesNet {
    friend class LearnBayes;
public:
    static BayesNet* loadFromTextFile(const string & filename);
    static BayesNet* loadFromBinFile(const string & filename);

    explicit BayesNet();
    explicit BayesNet(uint numNodes, const vector<uint> & nodeSizes,
            const vector<vector<uint>> & parentNodes);
    explicit BayesNet(uint numNodes, const vector<uint> & nodeSizes,
             const vector<vector<uint>> & parentNodes,
             const vector<vector<double>> & mapCpt);


    ///----------------------------------------------------------///
    /**
     * belief propagation
     * initial()
     * created on: August 21th, 2015
     * author: nguyennd
     */
    BayesNet(uint numNodes, const vector<uint> & nodeSizes,
             const vector<vector<uint>> & parentNodes,
             const vector<vector<double>> & mapCpt,
             const vector<uint> & nodeId,
             const vector<vector<double>> & lamdaValue,
             const vector<vector<double>> & piValue,
             const vector<vector<vector<double>>> & lamdaMessage,
             const vector<vector<vector<double>>> & piMessage,
             const vector<vector<double>> & beliefs,
             const vector<int> & evidences
             );
    void initial();
    void update(uint nodeIndex, int value);

    void initialBelief();
    void updateMul(vector<uint>evidenceIndex, vector<int>evidence);

    vector<uint> predecessorsOf(uint nodeIndex);
    bool isLoopCutInPredecessors(uint nodeIndex);

    void sendPiMessage(uint parentIn, uint childIn);
    void sendLamdaMessage(uint childIn, uint parentIn);

    vector<uint> getChildrenIndice(uint nodeIndex)const;
    vector<vector<uint>> getInstances(uint nodeIndex, uint xOption);
    vector<uint> cptIndex2Vector(uint nodeIndex, uint cptIndex)const;
    uint cptVector2Index(uint nodeIndex, vector<uint> cptVector)const;
    void show();
    static double flcTime;
    vector<uint> findLoopCutSet()const;

    vector<vector<double>> getBeliefs()const;
    void setBeliefs(vector<vector<double>> beliefs);
    ///----------------------------------------------------------///



    ~BayesNet();

    void    initNodeCpt(uint nodeIndex);
    void    setCpt(const vector<vector<double>> & mapCpt);
    void    setJointDistribution();
    void    setJointDistribution(const vector<double> & listJointProbabilities);

    const   vector<double>& getJointDistribution() const;
    double  getJointProbability(const vector<uint> & values);
    double  getConditionalProbability(const vector<uint> & mapEvidences, const vector<uint> & values);

    double  predictValue(uint preIndex, vector<uint> values);
    vector<double> predictValues(uint nodeIndex, vector<uint> values);

    /// additional util
    void addNode(uint nodeSize);
    void removeNode(uint nodeIndex);
    void addEdge(uint parentIndex, uint childIndex);
    void removeEdge(uint parentIndex, uint childIndex);

    void setNodeCpt(int nodeIndex, const vector<uint> & assignment, const vector<double> & probs);
    void setNodeSize(int nodeIndex, int nodeSize);
    void setIsModified(bool value) { mIsModified = value; }

    uint getNumNodes() const;
    uint getNumParents(uint nodeIndex) const;
    uint getNumNodeValues(uint nodeIndex) const;
    uint getNumCptValues(uint nodeIndex) const;
    uint getNumCases() const;
    vector<uint> getParentIndice(uint nodeIndex) const;
    double getNodeCpt(uint nodeIndex, uint cptIndex) const;
    bool isConnected() const;
    bool hasNode(uint nodeIndex) const;
    bool isModified() const;

    vector<double> getPosteriorProbabilities(uint nodeIndex, const vector<uint> & evidences);
    vector<double> getPosteriorProbabilitiesByBP(uint nodeIndex, const vector<uint> & evidences);

private:
    uint mNumNodes;                             // number of nodes in network
    vector<uint> mMapNodeSizes;                 // number of the values each node can take (discrete variables)
    vector<vector<uint>> mMapParents;          // each node has a set of parent-nodes
    vector<vector<double>> mMapCpt;            // each node has a conditional probability table (CPT)
    vector<double> mListJointProbabilities;     // use for pre-compute all joint probability over network
    bool mIsModified;


    ///----------------------------------------------------------///
    /**
     * belief propagation
     * initial()
     * created on: August 21th, 2015
     * author: nguyennd
     */
    vector<uint> mNodeId;                           // node identifier
    vector<vector<double>> mLamdaValue;             // node's lamda value
    vector<vector<double>> mPiValue;                // node's pi value
    vector<vector<vector<double>>> mLamdaMessage;   // node's lamda message
    vector<vector<vector<double>>> mPiMessage;      // node's pi message
    vector<vector<double>> mBeliefs;                 // node's beliefs
    vector<int> mEvidences;                          // evidences;

    ///----------------------------------------------------------///

public:
    /// debug purposes
    string getCPT(uint nodeId) const;
    string toString() const;
    void save(const string &filename) const;
};

#endif /* BAYESNET_H_ */
