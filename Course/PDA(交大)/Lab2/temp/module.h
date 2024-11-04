#ifndef MODULE_H
#define MODULE_H
#include <string>
#include <vector>
using namespace std;

class Block
{
public:
    // constructor and destructor
    Block(string& name_, int w_, int h_) : name(name_), w(w_), h(h_) {}
    ~Block() {}

    string name;
    int w; // width        
    int h; // height       
    int area() const { return w * h; }
};

class Node
{
public:
	// constructor and destructor
	Node(string name_): name(name_), parent(nullptr), left(nullptr), right(nullptr) {}
    Node(string name_,Node *parent_, Node *left_, Node *right_): name(name_), parent(parent_), left(left_), right(right_) {}
	~Node() {}
	
	string name;
	Node* parent;
    Node* left;
	Node* right;
	int x, y;
};

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name_, int x_, int y_) : name(name_), x(x_), y(y_) {}
    ~Terminal()  {}

    string name;      
    int x, y;       
};

class Net
{
public:
    // constructor and destructor
    Net()   {}
    ~Net()  {}

    vector<string> cell_list;
};

class Contour
{
public:
    // constructor and destructor
    Contour(int x1_, int x2_, int y_) : x1(x1_), x2(x2_), y(y_) {} 
    ~Contour() {}

    int x1, x2, y;
};
#endif 