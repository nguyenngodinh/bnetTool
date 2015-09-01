/**
 * \author trungvv1
 *
 * \date 4/6/2015
 * \class
 *
 * \brief write something about your class
 *
 *
 */

#include "mainwindow.h"

#include <QApplication>


#include <iostream>
#include "bayesnet.h"
#include <time.h>
#include <math.h>
using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    double BayesNet::flcTime = 0.0;
//    clock_t startingTime = clock();
    //debug
//    cout << "hello world!" << endl;
//    BayesNet * bnet = BayesNet::loadFromTextFile("multiply-3.bayesnet");
//    BayesNet * bnet = BayesNet::loadFromTextFile("multiply-new.bayesnet");
//    BayesNet * bnet = BayesNet::loadFromTextFile("loop.bayesnet");
//    clock_t init = clock();
//    BayesNet * bnet = BayesNet::loadFromTextFile("threat3.bayesnet");
//    float initTime = clock()-init;
//    cout << "initTime:" << initTime/CLOCKS_PER_SEC << endl;
//    BayesNet * bnet = BayesNet::loadFromTextFile("singly.bayesnet");
//    BayesNet * bnet = BayesNet::loadFromTextFile("tree.bayesnet");
//    bnet->update(0, 0);
//    bnet->update(2, 0);
//    vector<uint> eIndice;   vector<int> eVals;
//    eIndice.push_back(1);     eVals.push_back(0);
//    eIndice.push_back(3);     eVals.push_back(0);
//    eIndice.push_back(4);     eVals.push_back(1);
//    clock_t updateMul = clock();
//    bnet->updateMul(eIndice, eVals);
//    float updateMulTime = clock()-updateMul;
//    cout << "updateMulTime:" << updateMulTime/CLOCKS_PER_SEC << endl;
//    float diffTime = clock() - startingTime;
//    cout << "running time: " << diffTime/CLOCKS_PER_SEC << endl;

//    bnet->show();
//    end of debug

    return a.exec();
}
