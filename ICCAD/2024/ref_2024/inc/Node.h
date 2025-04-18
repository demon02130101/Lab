#ifndef _NODE_H_
#define _NODE_H_

#include "Coor.h"
#include "Cell.h"
#include "FF.h"
#include <string.h>
#include <cmath>

// Node class used in legalizer code, 
// represent  the ff and the comb cell(obstacle)

class Node{
private:
    std::string name;
    Coor gpCoor;    // Global Placement Coordinate
    Coor lgCoor;    // Legalized Coordinate
    Cell *cell;
    double w;
    double h;
    bool isPlace;
    double TNS;
    size_t closest_rowidx;
    size_t place_rowidx;
    
    // Used in detail placement
    std::vector<Node *> NeighborCells;
    FF *ffptr;

public:
    Node();
    ~Node();
    
    // Setters
    void setName(const std::string &name);
    void setGPCoor(const Coor &gpCoor);
    void setLGCoor(const Coor &lgCoor);
    void setCell(Cell *cell);
    void setW(double w);
    void setH(double h);
    void setIsPlace(bool isPlace);
    void setTNS(double TNS);
    void setClosestRowIdx(size_t closest_rowidx);
    void setPlaceRowIdx(size_t place_rowidx);
    void setFFPtr(FF *ffptr);

    // Getters
    const std::string &getName()const;
    const Coor &getGPCoor()const;
    const Coor &getLGCoor()const;
    Cell *getCell()const;
    double getW()const;
    double getH()const;
    bool getIsPlace()const;
    double getTNS()const;
    size_t getClosestRowIdx()const;
    size_t getPlaceRowIdx()const;
    FF *getFFPtr()const;

    // Helper method
    double getDisplacement()const;                      // displacement from lgCoor to gpCoor
    double getDisplacement(const Coor &otherCoor)const; // displacement from otherCoor to gpCoor

    // Stream Insertion
    friend std::ostream &operator<<(std::ostream &os, const Node &node);
};

#endif