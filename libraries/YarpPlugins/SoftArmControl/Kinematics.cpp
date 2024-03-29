// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

#include "SoftArmControl.hpp"

#include <cmath>

#include <Eigen/Dense>

using namespace sofia;

// -----------------------------------------------------------------------------

bool SoftArmControl::computeIk(double incl, double orien, std::vector<double> & lengths)
{
    theta=incl*M_PI/180;
    psi=orien*M_PI/180;

    if (theta!=0)
    {
        R=geomL0/theta;

        phi1= M_PI/2-psi;

        phi2= 7*M_PI/6-psi;

        phi3= 11*M_PI/6-psi;

        lengths[0]=geomL0 - theta * geomA * cos(phi1);
        lengths[1]=geomL0 - theta * geomA * cos(phi2);
        lengths[2]=geomL0 - theta * geomA * cos(phi3);
    }
    else
    {
        lengths[0]=geomL0;
        lengths[1]=geomL0;
        lengths[2]=geomL0;
    }

    lengths[0] = geomLg0 - lengths[0];
    lengths[1] = geomLg0 - lengths[1];
    lengths[2] = geomLg0 - lengths[2];

    return true;
}

// -----------------------------------------------------------------------------

int SoftArmControl::initTableIk(string csvfileName, vector<int> tableDimensions)
{
    ifstream csv;
       csv.open(csvfileName);
       csv.seekg(0);

       string line;
       double i1,i2;
       double l1,l2,l3;

       getline(csv,line);
       lookupIndex.resize(tableDimensions[0]);

       for (int i=0; i<lookupIndex.size(); i++)
       {
           lookupIndex[i].resize(tableDimensions[1]);

           for (int j=0; j<lookupIndex[i].size(); j++)
           {
               //get line from file
               getline(csv,line);
               istringstream ss(line);

               //compare index
               ss >> i1;
               ss >> i2;
               ss >> l1;
               ss >> l2;
               ss >> l3;

               //store index
               lookupIndex[i][j]=lookupTable.size();
               //add another line to table
               lookupTable.push_back(vector<double>{l1,l2,l3});
           }
       }
       return true;
}

// -----------------------------------------------------------------------------

int SoftArmControl::readTableIk(double incl, double orien, std::vector<double> & lengths)
{
     long index = lookupIndex[incl][orien];

     if (lengths.size()!=lookupTable[index].size())
     {
         cout << "Wrong size." << endl;
         return -1;
     }

     lengths = lookupTable[index];

     return true;
}
