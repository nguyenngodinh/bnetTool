/*
 * bayesnet.cpp
 *
 *  Created on: May 25, 2015
 *      Author: trungvv1
 */

#include "bayesnet.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>

#include "utility.h"

BayesNet *BayesNet::loadFromTextFile(const string & filename) {
    BayesNet* bn = nullptr;
    uint nNodes;                            // number of nodes in network
    vector<uint> nodeSizes;                 // number of the values each node can take (discrete variables)
    vector<vector<uint>> mMapParents;      // each node has a set of parent-nodes
    vector<vector<double>> mapCpt;        // each node has a conditional probability table (mMapCpt)
    vector<double> listJointProbabilities;  // use for pre-compute all joint probability over network
    uint value;
    double dValue;
    ifstream inf(filename);
    if (!inf.is_open()) {
        cout << "Open file error!" << "\n";
        return nullptr;
    }

    /// nodeSizes
    inf >> nNodes;
    for (uint i = 0; i < nNodes; ++i) {
        inf >> value;
        nodeSizes.push_back(value);
        mMapParents.push_back(vector<uint>());
        mapCpt.push_back(vector<double>());
    }

    /// mMapParents
    uint nodeIndex, nParents;
    for (uint i = 0; i < nNodes; ++i) {
        inf >> nodeIndex >> nParents;
        for (uint j = 0; j < nParents; ++j) {
            inf >> value;
            mMapParents[nodeIndex].push_back(value);
        }
    }

    /// CPTs
    for (uint i = 0; i < nNodes; ++i) {
        inf >> nodeIndex >> nParents;
        for (uint j = 0; j < nParents; ++j) {
            inf >> dValue;
            mapCpt[nodeIndex].push_back(dValue);
        }
    }
    bn = new BayesNet(nNodes, nodeSizes, mMapParents, mapCpt);

    /// jointProbabilities
    uint size = 0;
    inf >> size;
    if (size > 0) {
        for (uint i = 0; i < size; ++i) {
            inf >> dValue;
            listJointProbabilities.push_back(dValue);
        }
        bn->setJointDistribution(listJointProbabilities);
    } else {
        /// if info is not stored in file, recompute joint distribution
//        bn->setJointDistribution();
        bn->setIsModified(true);
    }

    inf.close();
    return bn;
}

BayesNet *BayesNet::loadFromBinFile(const string &filename)
{
    BayesNet* bn = nullptr;
    uint nNodes;                            // number of nodes in network
    vector<uint> nodeSizes;                 // number of the values each node can take (discrete variables)
    vector<vector<uint>> mMapParents;       // each node has a set of parent-nodes
    vector<vector<double>> mapCpt;          // each node has a conditional probability table (mMapCpt)
    vector<double> listJointProbabilities;  // use for pre-compute all joint probability over network
    uint i, uValue, size;
    double dValue;

    ifstream inf;
    inf.open(filename, ios::binary | ios::in);
    if (!inf.is_open()) {
        cout << "Open file error!" << "\n";
        return nullptr;
    }

    inf.read((char*)&nNodes, sizeof(nNodes));
    for (i = 0; i < nNodes; ++i) {
        inf.read((char*)&uValue, sizeof(uValue));
        nodeSizes.push_back(uValue);
    }
    for (i = 0; i < nNodes; ++i) {
        inf.read((char*)&size, sizeof(size));
        vector<uint> temp;
        while (size--) {
            inf.read((char*)&uValue, sizeof(uValue));
            temp.push_back(uValue);
        }
        mMapParents.push_back(temp);
    }
    for (i = 0; i < nNodes; ++i) {
        inf.read((char*)&size, sizeof(size));
        vector<double> temp;
        while (size--) {
            inf.read((char*)&dValue, sizeof(dValue));
            temp.push_back(dValue);
        }
        mapCpt.push_back(temp);
    }
    inf.read((char*)&size, sizeof(size));
    for (i = 0; i < size; ++i) {
        inf.read((char*)&dValue, sizeof(dValue));
        listJointProbabilities.push_back(dValue);
    }
    inf.close();

    bn = new BayesNet(nNodes, nodeSizes, mMapParents, mapCpt);
    if (listJointProbabilities.empty()) {
        bn->setJointDistribution();
    } else {
        bn->setJointDistribution(listJointProbabilities);
    }

    return bn;
}

BayesNet::BayesNet()
{
    mNumNodes = 0;
    mIsModified = false;
}

/**
 * @brief BayesNet::BayesNet
 * Initialize BayesNet, compute sizes of each CPT
 * @param numNodes
 * @param nodeSizes
 * @param mMapParents
 */
BayesNet::BayesNet(uint numNodes, const vector<uint> & nodeSizes,
                   const vector<vector<uint>> & parentNodes) {
    mNumNodes = numNodes;
    mMapNodeSizes = nodeSizes;
    mMapParents = parentNodes;

    for (uint i = 0; i < mNumNodes; ++i) {
        mMapCpt.push_back(vector<double>());
        /// sort mMapParents so as to satisfy topology order
        std::sort(mMapParents[i].begin(), mMapParents[i].end());
        initNodeCpt(i);
    }
    mIsModified = false;
}

/**
 * @brief BayesNet::BayesNet
 * only use when read from file
 * @param numNodes
 * @param nodeSizes
 * @param mMapParents
 * @param mapCpt
 */
BayesNet::BayesNet(uint numNodes, const vector<uint> & nodeSizes,
         const vector<vector<uint>> & parentNodes,
         const vector<vector<double>> & mapCpt) {
    mNumNodes = numNodes;
    mMapNodeSizes = nodeSizes;
    mMapParents = parentNodes;
    mMapCpt = mapCpt;
    mIsModified = false;

    ///------------------------------///
    for(uint i=0; i<mNumNodes; i++)
        mNodeId.push_back(i);
    initial();
    ///------------------------------///
}
///------------------------------///
/**
 * belief propagation
 * initial()
 * created on: August 21th, 2015
 * author: nguyennd
 */
BayesNet::BayesNet(uint numNodes, const vector<uint> &nodeSizes,
                   const vector<vector<uint> > &parentNodes,
                   const vector<vector<double> > &mapCpt,
                   const vector<uint> & nodeId, const vector<vector<double> > & lamdaValue,
                   const vector<vector<double> > & piValue, const vector<vector<vector<double> > > & lamdaMessage,
                   const vector<vector<vector<double> > > & piMessage, const vector<vector<double> > & beliefs,
                   const vector<int> & evidences)
{
    mNumNodes = numNodes;
    mMapNodeSizes = nodeSizes;
    mMapParents = parentNodes;
    mMapCpt = mapCpt;
    mNodeId = nodeId;
    mLamdaValue = lamdaValue;
    mPiValue = piValue;
    mLamdaMessage = lamdaMessage;
    mPiMessage = piMessage;
    mBeliefs = beliefs;
    mEvidences = evidences;
}

void BayesNet::initial()
{
//    clock_t startingTime = clock();
    uint i;
//    cout << "<initial>\n";
//    cout << "1.empty evidences" << endl;s
    for(i=0; i<mNumNodes; i++)
        mEvidences.push_back(-1);

//    cout << "2.init lamda values at each node as 1" << endl;
    for(i=0; i<mNumNodes; i++)
    {
        vector<double> lamdaValues;
        for(uint j=0; j<mMapNodeSizes.at(i); j++)
        {
            lamdaValues.push_back(1);
        }
        mLamdaValue.push_back(lamdaValues);
    }
//    cout << "3.init lamda message at each parent of node" << endl;
    for(i=0; i<mNumNodes; i++)
    {
        vector<uint> parents = mMapParents.at(i);
        vector<vector<double>> lamdaMsg;
        for(uint j=0; j<parents.size(); j++)
        {
            uint parent = parents.at(j);
            vector<double> msg;
            for(uint k=0; k<mMapNodeSizes.at(parent); k++)
            {
                msg.push_back(1);
            }
            lamdaMsg.push_back(msg);
        }
        mLamdaMessage.push_back(lamdaMsg);
    }
//    cout << "4.init pi message at each child of node" << endl;
    for(i=0; i<mNumNodes; i++)
    {
        vector<uint> children = getChildrenIndice(i);
        vector<vector<double>> piMsg;
        for(uint j=0; j<children.size(); j++)
        {
            vector<double> msg;
            for(uint k=0; k<mMapNodeSizes.at(i); k++)
            {
                msg.push_back(1);
            }
            piMsg.push_back(msg);
        }
        mPiMessage.push_back(piMsg);
    }
//    cout << "5.init belief and pi values at roots" << endl;
    for(i=0; i<mNumNodes; i++)
    {
        vector<double> bels;
        vector<double> piVals;
        for(uint j=0; j<mMapNodeSizes.at(i); j++)
        {
            bels.push_back(0);
            piVals.push_back(0);
        }
        if(mMapParents.at(i).size() ==0)//is root?
        {
            bels = mMapCpt.at(i);
            piVals = mMapCpt.at(i);
        }
        mBeliefs.push_back(bels);
        mPiValue.push_back(piVals);
    }
//    vector<uint> loop = findLoopCutSet();
//    cout << "loopCutSets:";
//    for(uint i=0; i<loop.size(); i++)
//    {
//        cout << loop.at(i) << "\t";
//    }
//    cout << endl;
//    if(findLoopCutSet().size()==0)
//    {
    for(i=0; i<mNumNodes; i++)
    {
        if(mMapParents.at(i).size() ==0)//is root?
        {
            vector<uint> children = getChildrenIndice(i);
            for(uint j=0; j<children.size(); j++)
            {
                uint child = children.at(j);
                //                cout << "sendPiMessage(" << i << ", " << child << ")\n";
                sendPiMessage(i, child);
            }
        }
    }
//    }
//    else
//    {
//        initialBelief();
//    }

//    cout << "6.send pi message from roots" << endl;//done in 5
//    cout << "</inital>\n";
//    float diffTime = clock() - startingTime;
//    cout << "initial() time: " << diffTime/CLOCKS_PER_SEC << endl;
}

void BayesNet::update(uint nodeIndex, int value)
{
//    cout << "<update>\n";
//    cout << "1.enter evidence" << endl;
    mEvidences.at(nodeIndex) = value;
    uint i;
    for(i=0; i<mMapNodeSizes.at(nodeIndex); i++)
    {
        if(value==-1)
            return;
        if(i==value)
        {
            mLamdaValue.at(nodeIndex).at(i) = 1.0;
            mPiValue.at(nodeIndex).at(i) = 1.0;
            mBeliefs.at(nodeIndex).at(i) = 1.0;
        }
        else
        {
            mLamdaValue.at(nodeIndex).at(i) = 0.0;
            mPiValue.at(nodeIndex).at(i) = 0.0;
            mBeliefs.at(nodeIndex).at(i) = 0.0;
        }
    }

//    cout << "2.message propagate" << endl;
    vector<uint> parents = getParentIndice(nodeIndex);
    vector<uint> children = getChildrenIndice(nodeIndex);

    for(i=0; i<parents.size(); i++)
    {
        if(mEvidences.at(parents.at(i)) ==-1)
            sendLamdaMessage(nodeIndex, parents.at(i));
    }
    for(i=0; i<children.size(); i++)
    {
        sendPiMessage(nodeIndex, children.at(i));
    }
//    cout << "<\\update>" << endl;
}

void BayesNet::initialBelief()
{
    uint i;
    for(i=0; i<mNumNodes; i++)
    {
        if(!isLoopCutInPredecessors(i))
        {
            if(mMapParents.at(i).size() == 0)
            {
                mBeliefs.at(i) = mMapCpt.at(i);
            }
            else
            {
                for(uint j=0; j<mMapNodeSizes.at(i); j++)
                {
                    vector<vector<uint>> instances = getInstances(i, j);
                    double bel = 0.0;
                    for(uint k=0; k<instances.size(); k++)
                    {
                        double productOfBel = 1.0;
                        for(uint l=0; l<mMapParents.at(i).size(); l++)
                            productOfBel *= mBeliefs.at(mMapParents.at(i).at(l)).at(instances.at(k).at(l));
                        uint cptIndex = cptVector2Index(i, instances.at(k));
                        double cpt = mMapCpt.at(i).at(cptIndex);
                        bel += cpt*productOfBel;
                    }
                    mBeliefs.at(i).at(j) = bel;
                }
            }
        }
    }
}

void BayesNet::updateMul(vector<uint> eIndice, vector<int> eVals)
{

    vector<uint> loopCut = findLoopCutSet();
    uint numberOfJointIns = 1;
    uint i;
    for(i=0; i<loopCut.size(); i++)
    {
        numberOfJointIns *=mMapNodeSizes.at(loopCut.at(i));
    }

    vector<vector<uint>> jointInses;
    for(i=0; i<numberOfJointIns; i++)
    {
        int ins = i;
        int factor = numberOfJointIns;
        vector<uint> jointIns;
        for(uint j=0; j<loopCut.size(); j++)
        {
            factor = factor/mMapNodeSizes.at(loopCut.at(j));
            uint index = ins/factor;
            ins %= factor;
            jointIns.push_back(index);
        }
        jointInses.push_back(jointIns);
    }

    ///---------
    vector<double> jointProbs;
    double normalConstant = 0.0;
    vector<BayesNet *> bayesNets;
    ///---------init
//    cout << "init:\n";
    for(i=0; i<jointInses.size(); i++)
    {
//        cout << i << "." << endl;
        double jointProb = 1.0;
        vector<uint> jointIns = jointInses.at(i);

        //BayesNet * bayesNet = new BayesNet(mNumNodes, mMapNodeSizes, mMapParents, mMapCpt);
        BayesNet * bayesNet = new BayesNet(mNumNodes, mMapNodeSizes, mMapParents, mMapCpt,
                                           mNodeId, mLamdaValue, mPiValue, mLamdaMessage, mPiMessage,
                                           mBeliefs, mEvidences);
        for(uint j=0; j<loopCut.size(); j++)
        {
//            cout << "\tloopCut:" << loopCut.at(j) << endl;
            vector<double> bel = bayesNet->getBeliefs().at(loopCut.at(j));
            jointProb *= bel.at(jointIns.at(j));

            bayesNet->update(loopCut.at(j), jointIns.at(j));

        }

        normalConstant += jointProb;
        jointProbs.push_back(jointProb);
        bayesNets.push_back(bayesNet);
//        bayesNets.at(i)->show();
    }

    for(i=0; i<jointInses.size(); i++)
        jointProbs.at(i) /= normalConstant;
    ///---------set evidence
//    cout << "evi:\n";
    for(i=0; i<eIndice.size(); i++)
    {
        double normalConstant = 0.0;
        uint j;

        for(j=0; j<jointInses.size(); j++)
        {
            jointProbs.at(j) *= bayesNets.at(j)->getBeliefs().at(eIndice.at(i)).at(eVals.at(i));
            normalConstant += jointProbs.at(j);
            bayesNets.at(j)->update(eIndice.at(i), eVals.at(i));
        }

        for(j=0; j<jointInses.size(); j++)
            jointProbs.at(j) /= normalConstant;
    }

    ///---------agg bel
    vector<vector<double>> beliefs;
    for(i=0; i<mNumNodes; i++)
    {
        vector<double> belief;
        for(uint j=0; j<mMapNodeSizes.at(i); j++)
        {
            belief.push_back(0.0);
        }
        beliefs.push_back(belief);
    }

    for(i=0; i<bayesNets.size(); i++)
    {
        for(uint j=0; j<mNumNodes; j++)
        {
            for(uint k=0; k<mMapNodeSizes.at(j); k++)
            {
                beliefs.at(j).at(k) += bayesNets.at(i)->getBeliefs().at(j).at(k) *jointProbs.at(i);
            }
        }
    }

    mBeliefs = beliefs;


}

vector<uint> BayesNet::predecessorsOf(uint nodeIndex)
{
    vector<uint> predecessors;
    vector<uint> fParents = mMapParents.at(nodeIndex);
    while(!fParents.empty())
    {
        predecessors.push_back(fParents.back());
        vector<uint> iParents = mMapParents.at(fParents.back());
        fParents.pop_back();
        for(uint i=0; i<iParents.size(); i++)
        {
            bool found =false;
            for(uint j=0; j<predecessors.size(); j++)
            {
                if(predecessors.at(j) == iParents.at(i))
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                fParents.push_back(iParents.at(i));
        }
    }
    return predecessors;
}

bool BayesNet::isLoopCutInPredecessors(uint nodeIndex)
{
    vector<uint> loopCut = findLoopCutSet();
    vector<uint> predecessors = predecessorsOf(nodeIndex);
    for(uint i=0; i<loopCut.size(); i++)
    {
        int found = false;
        for(uint j=0; j<predecessors.size(); j++)
        {
            if(predecessors.at(j) == loopCut.at(i))
            {
                found = true;
                break;
            }
        }
        if(found)
            return true;
    }
    return false;
}

void BayesNet::sendPiMessage(uint parentIn, uint childIn)
{
    uint i;
//    cout << "sendPiMsg(parent=" << parentIn << ", child=" << childIn << ")\n";
//    cout << "1.update pi message" << endl;

    vector<double> piMsg;

    for(i=0; i<mMapNodeSizes.at(parentIn); i++)
    {
        vector<uint> fChildren = getChildrenIndice(parentIn);
        double productOfLamdaMsg  = 1.0;
        for(uint j=0; j<fChildren.size(); j++)
        {
            uint fChild = fChildren.at(j);
            if(fChild != childIn)
            {
                uint k;
                for(k=0; k<getParentIndice(fChild).size(); k++)
                {
                    if(getParentIndice(fChild).at(k) == parentIn)
                    {
                        break;
                    }
                }
                vector<vector<double>> lamdaMsg = mLamdaMessage.at(fChild);
                productOfLamdaMsg *= lamdaMsg.at(k).at(i);
            }
        }
        double piVal = mPiValue.at(parentIn).at(i);
        double msg = productOfLamdaMsg * piVal;
        piMsg.push_back(msg);
    }

    for(i=0; i<getChildrenIndice(parentIn).size(); i++)
    {
        if(getChildrenIndice(parentIn).at(i) == childIn)
            break;
    }
    mPiMessage.at(parentIn).at(i) = piMsg;
//    cout << parentIn << ":";
//    for(i=0; i<piMsg.size(); i++)
//        cout << piMsg.at(i) << "\t";
//    cout << endl;

//    cout << "2.update pi value" << endl;
    if(mEvidences.at(childIn)==-1)
    {
        double normalConstant = 0.0;
//        cout << "piValue " << childIn << ":\n";
        for(i=0; i<mMapNodeSizes.at(childIn); i++)
        {
            double piVal=0.0;
            vector<vector<uint>> instances = getInstances(childIn, i);
//            cout << "cptIndex={";
//            for(uint j=0; j<instances.size(); j++)
//            {
//                cout << cptVector2Index(childIn, instances.at(j)) << ",";
//            }
//            cout << "}\n";
//            cout << "\tval:" << i << ":\n";
            for(uint j=0; j<instances.size(); j++)
            {
                vector<uint> instance = instances.at(j);
                vector<uint> parents = getParentIndice(childIn);
                double productOfPiMsg = 1.0;
//                cout << "\t\tPiMsg:";
                for(uint k=0; k<instance.size()-1; k++)
                {
                    uint parent = parents.at(k);
                    uint l;
                    for(l=0; l<getChildrenIndice(parent).size(); l++)
                    {
                        if(getChildrenIndice(parent).at(l) == childIn)
                            break;
                    }

//                    cout << mPiMessage.at(parent).at(l).at(instance.at(k)) << "|";
                    productOfPiMsg *= mPiMessage.at(parent).at(l).at(instance.at(k));
                }
//                cout << endl;
                uint cptIndex = cptVector2Index(childIn, instance);
                double cpt = mMapCpt.at(childIn).at(cptIndex);
//                cout << "\t\tcptIndex:" << cptIndex << endl;
//                cout << "\t\tcpt:" << cpt << endl;
                piVal += cpt*productOfPiMsg;
            }
//            cout << "\t" << piVal << endl;
            mPiValue.at(childIn).at(i) = piVal;
            mBeliefs.at(childIn).at(i) = mPiValue.at(childIn).at(i) * mLamdaValue.at(childIn).at(i);
            normalConstant += mBeliefs.at(childIn).at(i);
        }
//        cout << endl;
//        cout << "normalConstant:" << normalConstant << endl;
        for(i=0; i<mMapNodeSizes.at(childIn); i++)
        {
            mBeliefs.at(childIn).at(i) /= normalConstant;
        }
        for(i=0; i<getChildrenIndice(childIn).size(); i++)
        {
            bool found = false;
            for(uint j=0; j<findLoopCutSet().size(); j++)
                if(childIn == findLoopCutSet().at(j))
                {
                    found = true;
                    break;
                }

            if(found)
                break;

            sendPiMessage(childIn, getChildrenIndice(childIn).at(i));
        }
    }
//    cout << "3.udpate belief" << endl;//done in 2
//    cout << "4.send pi message" << endl;//done in 2
//    cout << "5.send lamda message" << endl;
    bool isInstanced = false;
    for(i=0; i<mLamdaValue.at(childIn).size(); i++)
    {
        if(mLamdaValue.at(childIn).at(i) == 1)
        {
            isInstanced = true;
            break;
        }
    }

    if(isInstanced)
    {
        vector<uint> parents = getParentIndice(childIn);
        for(i=0; i<parents.size(); i++)
        {
            uint parent = parents.at(i);
            if(parent !=  parentIn && mEvidences.at(parent) == -1)
            {
//                cout << "sendLamdaMessage(child=" << childIn << ",parent=" << parent << ")\n";
                sendLamdaMessage(childIn, parent);
            }
        }
    }
}

void BayesNet::sendLamdaMessage(uint childIn, uint parentIn)
{
//    cout << "<sendLamdaMessage>\n";
//    cout << "sendLamdaMessage(child=" << childIn << ",parent=" << parentIn << ")\n";
//    cout << "1.update lamdaMessage" << endl;
    uint i;
    double normalConstant = 0.0;
    for(i=0; i<mMapNodeSizes.at(parentIn); i++)
    {
        uint j;
        double lamdaMsg = 0.0;
//        cout << "val = " << i << endl;
        for(j=0; j<mMapNodeSizes.at(childIn); j++)
        {
//            cout << "\tSumOfProducOfPiMsg:" << endl;
            double sumOfProduct = 0.0;
            vector<vector<uint>> instances = getInstances(childIn, j);
            uint k;
            for(k=0; k<instances.size(); k++)
            {
                vector<uint> instance = instances.at(k);
//                cout << "\t\tproductOfPiMsg:" << endl;
                double productOfPiMsg = 1.0;
                uint l;
                for(l=0; l<mMapParents.at(childIn).size(); l++)
                {
                    if(mMapParents.at(childIn).at(l) == parentIn)
                        break;
                }
//                cout << "\t\t\tpiMsg:";
                if(instance.at(l) == i)
                {
                    for(uint m=0; m<mMapParents.at(childIn).size(); m++)
                    {
                        uint parent = mMapParents.at(childIn).at(m);
                        if(parent==parentIn)
                            continue;
                        uint n;
                        vector<uint> fChildren = getChildrenIndice(parent);
                        for(n=0; n<fChildren.size(); n++)
                        {
                            if(fChildren.at(n) == childIn)
                                break;
                        }
                        double pMsg = mPiMessage.at(parent).at(n).at(instance.at(m));
//                        cout << pMsg << "|";
                        productOfPiMsg *= pMsg;
                    }
                }
                else
                {
                    productOfPiMsg = 0.0;
                }
//                cout << endl;
//                cout << "\t\tcpt:" << endl;
                uint cptIndex = cptVector2Index(childIn, instances.at(k));
//                cout << "\t\t\tcptIndex:" << cptIndex << endl;
                double cpt = mMapCpt.at(childIn).at(cptIndex);

//                cout << "\t\tcpt*productOfPiMsg:" << cpt << "*" << productOfPiMsg << endl;
                sumOfProduct += cpt*productOfPiMsg;
            }
//            cout << "\tsumOfProduct:" << sumOfProduct << endl;
//            cout << "\t" << "lamdaValue:" << mLamdaValue.at(childIn).at(j) << endl;
            lamdaMsg += sumOfProduct*mLamdaValue.at(childIn).at(j);
        }
        for(j=0; j<mMapParents.at(childIn).size(); j++)
            if(mMapParents.at(childIn).at(j) == parentIn)
                break;
        mLamdaMessage.at(childIn).at(j).at(i) = lamdaMsg;
//        cout << "lamdaMsg:" << lamdaMsg << endl;
        //2.
        vector<uint> children = getChildrenIndice(parentIn);
        double lamdaVal = 1.0;
        for(j=0; j<children.size(); j++)
        {
            uint child = children.at(j);
            uint j;
            for(j=0; j<mMapParents.at(child).size(); j++)
                if(mMapParents.at(child).at(j) == parentIn)
                    break;
            lamdaVal *= mLamdaMessage.at(child).at(j).at(i);
        }
        mLamdaValue.at(parentIn).at(i) = lamdaVal;
        //3.
        mBeliefs.at(parentIn).at(i) = mLamdaValue.at(parentIn).at(i) * mPiValue.at(parentIn).at(i);
        normalConstant += mBeliefs.at(parentIn).at(i);
    }
//    cout << "2.compute lamdaValue" << endl;//done in 1;
//    cout << "3.update belief" << endl;//done in 1;
//    cout << "3.1.normalize belief" << endl;
    for(i=0; i<mMapNodeSizes.at(parentIn); i++)
        mBeliefs.at(parentIn).at(i) /= normalConstant;
//    cout << "4.send lamda message" << endl;
    for(i=0; i<mMapParents.at(parentIn).size(); i++)
    {
        bool found = false;
        for(uint j=0; j<findLoopCutSet().size(); j++)
            if(parentIn == findLoopCutSet().at(j))
            {
                found = true;
                break;
            }
        if(found)
            break;
        if(mEvidences.at(mMapParents.at(parentIn).at(i)) ==-1)
            sendLamdaMessage(parentIn, mMapParents.at(parentIn).at(i));
    }

//    cout << "5.send pi message" << endl;
    vector<uint> fChildren = getChildrenIndice(parentIn);
    for(i=0; i<fChildren.size(); i++)
    {
        bool found = false;
        for(uint j=0; j<findLoopCutSet().size(); j++)
            if(parentIn == findLoopCutSet().at(j))
            {
                found = true;
                break;
            }
        if(found)
            break;
        if(fChildren.at(i) != childIn)
            sendPiMessage(parentIn, fChildren.at(i));
    }
//    cout << "</sendLamdaMessage>\n";
}

vector<uint> BayesNet::getChildrenIndice(uint nodeIndex) const
{
    vector<uint> childrenIndice;
    for(uint i=0; i<mNumNodes; i++)
    {
        vector<uint> parents = mMapParents.at(i);
        for(uint j=0; j<parents.size(); j++)
        {
            uint parent = parents.at(j);
            if(parent == nodeIndex)
            {
                childrenIndice.push_back(i);
                break;
            }
        }
    }
    return childrenIndice;
}

vector<vector<uint>> BayesNet::getInstances(uint nodeIndex, uint xOption)
{
//    cout << "cptIndex={";
    vector<vector<uint>> instances;
    for(uint i=0; i<mMapCpt.at(nodeIndex).size(); i++)
    {
        vector<uint> instance = cptIndex2Vector(nodeIndex, i);
        if(instance.back() == xOption)
        {
            instances.push_back(instance);
//            cout << i << ",";
        }
    }
//    cout << "}\n";
    return instances;
}

vector<uint> BayesNet::cptIndex2Vector(uint nodeIndex, uint cptIndex) const
{
    uint fCptIndex = cptIndex;
    vector<uint> cptVector;
    uint factor = mMapCpt.at(nodeIndex).size();
    vector<uint> parents = getParentIndice(nodeIndex);
    int index;
    for(uint i = 0; i<parents.size(); i++)
    {
        factor = factor/mMapNodeSizes.at(parents.at(i));
        index = fCptIndex/factor;
        fCptIndex = fCptIndex%factor;
        cptVector.push_back(index);
    }
    index = fCptIndex;
    cptVector.push_back(index);
    return cptVector;
}

uint BayesNet::cptVector2Index(uint nodeIndex, vector<uint> cptVector) const
{
    vector<uint> fCptVector = cptVector;
    uint cptIndex =0.0;
    vector<uint> parents = mMapParents.at(nodeIndex);
    uint factor = mMapCpt.at(nodeIndex).size();//mMapNodeSizes.at(nodeIndex);
    for(uint i=0; i<parents.size(); i++)
    {
        uint parent = parents.at(i);
        factor = factor/mMapNodeSizes.at(parent);
        cptIndex += factor * fCptVector.at(i);
    }
    cptIndex += fCptVector.back();

    return cptIndex;

}

void BayesNet::show()
{
    /*
    cout << "<====info=====>" << endl;
    cout << "mNumNodes:" << mNumNodes << endl;
    cout << "mMapNodeSizes:";
    for(uint i=0; i<mNumNodes; i++)
    {
        cout << "Node(" << i << ")=";
        cout << mMapNodeSizes.at(i) << "\t";
    }
    cout << endl;
    cout << "mMapParents:";
    for(uint i=0; i<mNumNodes; i++)
    {
        cout << "ParentsOf(" << i << ")=" << mMapParents.at(i).size() << "|";
        for(uint j=0; j<mMapParents.at(i).size(); j++)
        {
            cout << mMapParents.at(i).at(j) << "\t";
        }
        cout << endl;
    }
    cout << "mMapCpts:\n";
    for(uint i=0; i<mNumNodes; i++)
    {
        cout << "mMapCpts(" << i << ")=" << mMapCpt.at(i).size() << "|";
        for(uint j=0; j<mMapCpt.at(i).size(); j++)
        {
            cout << mMapCpt.at(i).at(j) << "\t";
        }
        cout << endl;
    }
    cout << "<====\info====>" << endl;
    */
    cout << "<====bp====>" << endl;
    for(uint i=0; i<mNumNodes; i++)
    {
        cout << "Bel(" << i << "):";
        for(uint j=0; j<mBeliefs.at(i).size(); j++)
        {
            cout << mBeliefs.at(i).at(j) << "\t";
        }
        cout << endl;
        /*
        cout << "PiValue(" <<i << "):";
        for(uint j=0; j<mPiValue.at(i).size(); j++)
        {
            cout << mPiValue.at(i).at(j) << "\t";
        }
        cout << endl;
        cout << "LamdaValue(" << i << "):";
        for(uint j=0; j<mLamdaValue.at(i).size(); j++)
        {
            cout << mLamdaValue.at(i).at(j) << "\t";
        }
        cout << endl;
*/
    }

    cout << "<====\\bp====>" << endl;

}

vector<uint> BayesNet::findLoopCutSet() const
{
    vector<uint> fNodeId = mNodeId;
    vector<uint> fMapNodeSizes = mMapNodeSizes;
    vector<vector<uint>> fMapParents = mMapParents;
    uint fNumNodes = mNumNodes;
    vector<uint> loopCutSet;
    while(fNumNodes !=0)
    {
        vector<uint> removeList;
        uint i;
        for(i=0; i<fNumNodes; i++)
        {
            vector<uint> neighbors;
            //find neighbors = childrens + parents
            uint j;
            //children
            for(j=0; j<fNumNodes; j++)
            {
//                vector<uint> jParents = fMapParents.at(j);
                for(uint k=0; k<fMapParents.at(j).size(); k++)
                {
                    uint kParent = fMapParents.at(j).at(k);
                    if(kParent == fNodeId.at(i))
                    {
                        neighbors.push_back(fNodeId.at(j));
                        break;
                    }
                }
            }
            //parent
            vector<uint> iParents = fMapParents.at(i);
            for(j=0; j<iParents.size(); j++)
            {
                bool found = false;
                for(uint k=0; k<neighbors.size(); k++)
                {
                    if(neighbors.at(k) == iParents.at(j))
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                    neighbors.push_back(iParents.at(j));
            }

            if(neighbors.size() <= 1)
            {
                removeList.push_back(i);
            }
        }
        //find node that sastisfy heuristic algorithm find minimal loop cut-set
        if(removeList.size() == 0)
        {
            for(i=0; i<fNumNodes; i++)
            {
                uint iNumberOfParents = fMapParents.at(i).size();
                if(iNumberOfParents == 0)
                {
                    removeList.push_back(i);
                    loopCutSet.push_back(fNodeId.at(i));
                }
            }
        }
        vector<uint> tempRemoveList = removeList;
        vector<uint> tempNodeId = fNodeId;
        while(!tempRemoveList.empty())
        {
            fNodeId.erase(fNodeId.begin()+tempRemoveList.back());
            fMapNodeSizes.erase(fMapNodeSizes.begin()+tempRemoveList.back());
            fMapParents.erase(fMapParents.begin()+tempRemoveList.back());
            tempRemoveList.pop_back();
            fNumNodes--;
        }
        for(i=0; i<fNumNodes; i++)
        {
            for(uint j=0; j<removeList.size(); j++)
            {
                uint remove = tempNodeId.at(removeList.at(j));
                int index=-1;
                for(uint k=0; k<fMapParents.at(i).size(); k++)
                {
                    if(fMapParents.at(i).at(k) == remove)
                    {
                        index = k;
                        break;
                    }
                }
                if(index!=-1)
                    fMapParents.at(i).erase(fMapParents.at(i).begin()+index);
            }
        }

    }

    return loopCutSet;
}

vector<vector<double> > BayesNet::getBeliefs() const
{
    return mBeliefs;
}

void BayesNet::setBeliefs(vector<vector<double> > beliefs)
{
    mBeliefs = beliefs;
}

///------------------------------///
BayesNet::~BayesNet() {
    mMapNodeSizes.clear();
    mListJointProbabilities.clear();
    for (uint i = 0; i < mNumNodes; ++i) {
        mMapParents[i].clear();
        mMapCpt[i].clear();
    }
    mMapParents.clear();
    mMapCpt.clear();
}

/**
 * @brief BayesNet::initNodeCpt
 * uniform init
 * @param nodeIndex
 */
void BayesNet::initNodeCpt(uint nodeIndex) {
    auto cptSize = mMapNodeSizes[nodeIndex];
    for (const auto & paNode : mMapParents[nodeIndex]) {
        cptSize *= mMapNodeSizes[paNode];
    }
    mMapCpt[nodeIndex].clear();
    for (uint i = 0; i < cptSize; ++i) {
//        if (i % mMapNodeSizes[nodeIndex] == 0) {
//            mMapCpt[nodeIndex].push_back(1.0);
//        } else {
//            mMapCpt[nodeIndex].push_back(0.0);
//        }
        mMapCpt[nodeIndex].push_back( 1.0 / mMapNodeSizes[nodeIndex] );
    }
}

void BayesNet::setCpt(const vector<vector<double>> & mapCpt) {
    for (auto & cpt : mMapCpt) {
        cpt.clear();
    }
    mMapCpt.clear();
    mMapCpt = mapCpt;
    setJointDistribution();
}

/**
 * @brief BayesNet::setJointDistribution
 * pre-compute all complete joint probabilities if no infomation is available
 */
void BayesNet::setJointDistribution() {
//    mListJointProbabilities.clear();
//    uint ncases = 1;
//    for (uint i = 0; i < mNumNodes; ++i) {
//        ncases *= mMapNodeSizes[i];
//    }

//    double p;
//    uint index;
//    for (uint i = 0; i < ncases; ++i) {
//        vector<uint> case_i = Utility::numberToVector(mMapNodeSizes, i);
//        p = 1.0;
//        for (uint nodeId = 0; nodeId < mNumNodes; ++nodeId) {
//            if (mMapParents[nodeId].empty()) {
//                // no parents --> straight-forward
//                index = case_i[nodeId];
//            } else {
//                vector<uint> maxValues, values;
//                for (const auto& paNode : mMapParents[nodeId]) {
//                    maxValues.push_back(mMapNodeSizes[paNode]);
//                    values.push_back(case_i[paNode]);
//                }
//                maxValues.push_back(mMapNodeSizes[nodeId]);
//                values.push_back(case_i[nodeId]);
//                index = Utility::vectorToNumber(maxValues, values);
//            }
//            p *= mMapCpt[nodeId][index];
//        }
//        mListJointProbabilities.push_back(p);
//    }
    mIsModified = false;
}

void BayesNet::setJointDistribution(const vector<double>& listJointProbabilities) {
    mListJointProbabilities.clear();
    mListJointProbabilities = listJointProbabilities;
    mIsModified = false;
}

const vector<double> &BayesNet::getJointDistribution() const
{
    return mListJointProbabilities;
}

/**
 * @brief getJointProbability
 * compute joint probability from a subset of nodes
 * e.g. P(A,C) = sum_BD { P(A,B,C,D) }
 * @param values value of each node in network,
 * NONE_NODE_INDEX if node doesn't appear in the joint probability
 * @return
 */
double BayesNet::getJointProbability(const vector<uint> & values) {
    if (mIsModified) {
        setJointDistribution();
    }

    double prob = 0;
    vector<uint> maxValues;
    uint ncases = 1;
    for (uint nodeId = 0; nodeId < mNumNodes; ++nodeId) {
        if (values[nodeId] == NONE_NODE_INDEX) {
            // node doesn't appear in the joint probability
            // --> iterate through it's possible values
            maxValues.push_back(mMapNodeSizes[nodeId]);
            ncases *= mMapNodeSizes[nodeId];
        }
    }

    /// sum over all cases
    for (uint i = 0; i < ncases; ++i) {
        vector<uint> indice = Utility::numberToVector(maxValues, i);
        for (uint j = 0; j < mNumNodes; ++j) {
            if (values[j] != NONE_NODE_INDEX) {
                // node appears in the joint probability
                // --> value not change
                indice.insert(indice.begin() + j, values[j]);
            }
        }
        auto index = Utility::vectorToNumber(mMapNodeSizes, indice);
        prob += mListJointProbabilities[index];
    }

    return prob;
}

/**
 * @brief BayesNet::getConditionalProbability
 * e.g. P(A|BC) = P(ABC) / P(BC)
 * where A is prediction, B,C is evidences
 * @param evidences - list index of evidences
 * @param values - values of evidences and predictions
 * value = NONE_NODE_INDEX for other nodes
 * @return
 */
double BayesNet::getConditionalProbability(const vector<uint> & mapEvidences,
                                           const vector<uint> & values) {
    double jointProb, margProb, condProb=0;

    /// compute marginal probability
    vector<uint> margValues;
    for (uint i = 0; i < mNumNodes; ++i) {
        margValues.push_back(NONE_NODE_INDEX);
    }
    for (const auto& evidence : mapEvidences) {
        margValues[evidence] = values[evidence];
    }
    margProb = getJointProbability(margValues);

    /// compute conditional probability
    jointProb = getJointProbability(values);
    if (margProb)
        condProb = jointProb / margProb;

    return condProb;
}

/**
 * @brief BayesNet::predictValue
 * Predict node value based on a vector of network values,
 * value = NONE_NODE_INDEX if the evidence is not observed
 * @param preIndex - index of prediction node
 * @param values - all nodes in network must be specified by labeled values
 * @return value - calculated based on probability as weight of each level of threat
 */
double BayesNet::predictValue(uint preIndex, vector<uint> values) {
    vector<uint> mapEvidences;
    for (uint i = 0; i < mNumNodes; ++i) {
        if (values[i] != NONE_NODE_INDEX && i != preIndex) {
            mapEvidences.push_back(i);
        }
    }

    double preValue = 0.0;
    for (uint i = 0; i < mMapNodeSizes[preIndex]; ++i) {
        values[preIndex] = i;
        double temp = getConditionalProbability(mapEvidences, values);
        preValue += temp * i;
    }
    return preValue;
}

/**
 * @brief BayesNet::predictValues
 * @param nodeIndex
 * @param values - all P(nodeIndex)
 * @return
 */
vector<double> BayesNet::predictValues(uint nodeIndex, vector<uint> values)
{
    vector<uint> evidences;
    for (int i = 0; i < mNumNodes; ++i) {
        if (i != nodeIndex) {
            evidences.push_back(i);
        }
    }

    vector<double> predictValues;
    double checkSum = 0.0;
    for (uint i = 0; i < mMapNodeSizes[nodeIndex]; ++i) {
        values[nodeIndex] = i;
        double temp = getConditionalProbability(evidences, values);
        predictValues.push_back(temp);
        checkSum += temp;
    }

    if (std::fabs(checkSum-1.0) > 0.01) {
        for (auto& pv : predictValues)
            pv = 1.0 / mMapNodeSizes[nodeIndex];
    }
    return predictValues;
}

void BayesNet::addNode(uint nodeSize)
{
    if (nodeSize >= 2) {
        mMapNodeSizes.push_back(nodeSize);
        mMapParents.push_back(vector<uint>());
        vector<double> newCpt(nodeSize, 0.0);
        newCpt[0] = 1.0;
        for (int i = 1; i < nodeSize; ++i) {
            newCpt[i] = 0.0;
        }
        mMapCpt.push_back(newCpt);
        ++mNumNodes;
        mIsModified = true;
    }
}

void BayesNet::removeNode(uint nodeIndex)
{
    mMapNodeSizes.erase(mMapNodeSizes.begin() + nodeIndex);

    /// change index of other node
    for (int otherIndex=0; otherIndex<mNumNodes; ++otherIndex) {
        auto findIndex = std::find(mMapParents[otherIndex].begin(), mMapParents[otherIndex].end(), nodeIndex);
        if (findIndex != mMapParents[otherIndex].end()) {       /// children
            mMapParents[otherIndex].erase(findIndex);
            for (int i = mMapParents[otherIndex].size()-1; i > -1; --i) {
                if (mMapParents[otherIndex][i] > nodeIndex) {
                    --mMapParents[otherIndex][i];
                }
            }
            initNodeCpt(otherIndex);
        } else {
            for (int i = mMapParents[otherIndex].size()-1; i > -1; --i) {
                if (mMapParents[otherIndex][i] > nodeIndex) {
                    --mMapParents[otherIndex][i];
                }
            }
        }
    }
    mMapParents[nodeIndex].clear();
    mMapParents.erase(mMapParents.begin() + nodeIndex);

    mMapCpt[nodeIndex].clear();
    mMapCpt.erase(mMapCpt.begin() + nodeIndex);

    --mNumNodes;
    mIsModified = true;
}

void BayesNet::addEdge(uint parentIndex, uint childIndex)
{
    if (std::find(mMapParents[childIndex].begin(), mMapParents[childIndex].end(), parentIndex) != mMapParents[childIndex].end())
        return;

    int i;
    for (i = mMapParents[childIndex].size()-1; i >= 0 && mMapParents[childIndex][i] > parentIndex; --i)
        ;
    mMapParents[childIndex].insert(mMapParents[childIndex].begin()+i+1, parentIndex);
    initNodeCpt(childIndex);
    mIsModified = true;
}

void BayesNet::removeEdge(uint parentIndex, uint childIndex)
{
    auto findIndex = std::find(mMapParents[childIndex].begin(), mMapParents[childIndex].end(), parentIndex);
    if (findIndex != mMapParents[childIndex].end()) {
        mMapParents[childIndex].erase(findIndex);
    }
    initNodeCpt(childIndex);
    mIsModified = true;
}

void BayesNet::setNodeCpt(int nodeIndex, const vector<uint> &assignment, const vector<double> &probs)
{
    vector<uint> numValues, values;
    int i = 0;
    for (const auto & paNodeIndex : mMapParents[nodeIndex]) {
        numValues.push_back(mMapNodeSizes[paNodeIndex]);
        values.push_back(assignment[i++]);
    }
    auto base = Utility::vectorToNumber(numValues, values);
    for (i = 0; i < mMapNodeSizes[nodeIndex]; ++i) {
        mMapCpt[nodeIndex][base*mMapNodeSizes[nodeIndex]+i] = probs[i];
    }
    mIsModified = true;
}

void BayesNet::setNodeSize(int nodeIndex, int nodeSize)
{
    mMapNodeSizes[nodeIndex] = nodeSize;
    initNodeCpt(nodeIndex);
    for (int i = 0; i < mNumNodes; ++i) {
        if (std::find(mMapParents[i].begin(), mMapParents[i].end(), nodeIndex) != mMapParents[i].end()) {
            initNodeCpt(i);
        }
    }
}

uint BayesNet::getNumNodes() const
{
    return mNumNodes;
}

uint BayesNet::getNumParents(uint nodeIndex) const
{
    return mMapParents[nodeIndex].size();
}

uint BayesNet::getNumNodeValues(uint nodeIndex) const
{
    return mMapNodeSizes[nodeIndex];
}

uint BayesNet::getNumCptValues(uint nodeIndex) const
{
    return mMapCpt[nodeIndex].size();
}

uint BayesNet::getNumCases() const
{
    return mListJointProbabilities.size();
}

vector<uint> BayesNet::getParentIndice(uint nodeIndex) const
{
    return mMapParents[nodeIndex];
}

double BayesNet::getNodeCpt(uint nodeIndex, uint cptIndex) const
{
    return mMapCpt[nodeIndex][cptIndex];
}

bool BayesNet::isConnected() const
{
    set<uint> connectedNodes;
    for (const auto & list : mMapParents) {
        for (const auto & index : list) {
            connectedNodes.insert(index);
        }
    }
    return (connectedNodes.size() < mNumNodes);
}

bool BayesNet::hasNode(uint nodeIndex) const
{
    return nodeIndex < mNumNodes;
}

bool BayesNet::isModified() const
{
    return mIsModified;
}

/**
 * @brief BayesNet::getPosteriorProbabilities
 * P(nodeIndex)
 * @param nodeIndex
 * @param evidences - evidence value != NONE_NODE_INDEX
 * @return
 */
vector<double> BayesNet::getPosteriorProbabilities(uint nodeIndex, const vector<uint> & evidences)
{
    vector<double> probs;
    double jointProb, margProb;

    auto values = evidences;
    if (values[nodeIndex] != NONE_NODE_INDEX) {
        /// if node in evidences
        for (uint i = 0; i < mMapNodeSizes[nodeIndex]; ++i) {
            probs.push_back(0);
        }
        probs[values[nodeIndex]] = 1.0;
    } else {
        /// if node not in evidences
        for (uint i = 0; i < mMapNodeSizes[nodeIndex]; ++i) {
            values[nodeIndex] = i;
            jointProb = getJointProbability(values);
            margProb += jointProb;
            probs.push_back(jointProb);
        }
        for (uint i = 0; i < mMapNodeSizes[nodeIndex]; ++i) {
            probs[i] /= margProb;
        }
    }

    return probs;
}

/**
 * @brief BayesNet::getCPT
 * @param nodeId
 * @return String represents <pa, x> : <p(x|pa)>
 */
string BayesNet::getCPT(uint nodeId) const {
    stringstream content;

    vector<uint> maxValues;
    for (const auto& paNode : mMapParents[nodeId]) {
        maxValues.push_back(mMapNodeSizes[paNode]);
    }
    maxValues.push_back(mMapNodeSizes[nodeId]);

    uint count = 1;
    for (uint i = 0; i < mMapCpt[nodeId].size(); ++i) {
        vector<uint> indice = Utility::numberToVector(maxValues, i);
        for (const auto& index : indice) {
            content << index << "\t";
        }
        content << mMapCpt[nodeId][i];
        if (count < mMapNodeSizes[nodeId]) {
            content << "\n";
            ++count;
        } else {
            content << "\n" << "\n";
            count = 1;
        }

    }

    return content.str();
}

/**
 * @brief BayesNet::toString
 * 1. number of nodes
 * 2. size of each node
 * 3. network's structure (infomation on parent nodes)
 * 4. CPTs
 * 5. [pre-compute] joint probabilities (optional)
 * 6. [learning] node counter & its parent counter (optional)
 *
 * @param filename
 */
string BayesNet::toString() const {
    stringstream content;
    uint i;

    content << mNumNodes << "\n";
    for (i = 0; i < mNumNodes; ++i) {
        content << mMapNodeSizes[i] << "\t";
    }
    content << "\n" << "\n";

    for (i = 0; i < mNumNodes; ++i) {
        content << i << "\t";
        content << mMapParents[i].size() << "\t";
        for (const auto& value : mMapParents[i]) {
            content << value << "\t";
        }
        content << "\n";
    }
    content << "\n";

    for (i = 0; i < mNumNodes; ++i) {
        content << i << "\t";
        content << mMapCpt[i].size() << "\t";
        for (const auto& value : mMapCpt[i]) {
            content << value << "\t";
        }
        content << "\n";
    }
    content << "\n";

//    content << mListJointProbabilities.size() << "\n";
//    for (const auto& value : mListJointProbabilities) {
//        content << value << "\n";
//    }
//    content << "\n";

    return content.str();
}

void BayesNet::save(const string & filename) const {
    ofstream of;
    of.open(filename, ios::binary | ios::out);
    if (!of.is_open()) {
        cout << "Write file error!" << "\n";
        return;
    }

    uint size;
    of.write((char*)&mNumNodes, sizeof(mNumNodes));
    for (const auto & size : mMapNodeSizes) {
        of.write((char*)&size, sizeof(size));
    }
    for (const auto & parents : mMapParents) {
        size = parents.size();
        of.write((char*)&size, sizeof(size));
        for (const auto & pa : parents) {
            of.write((char*)&pa, sizeof(pa));
        }
    }
    for (const auto & cpt : mMapCpt) {
        size = cpt.size();
        of.write((char*)&size, sizeof(size));
        for (const auto & p : cpt) {
            of.write((char*)&p, sizeof(p));
        }
    }
    size = mListJointProbabilities.size();
    of.write((char*)&size, sizeof(size));
    for (const auto & p : mListJointProbabilities) {
        of.write((char*)&p, sizeof(p));
    }

    of.close();
}
