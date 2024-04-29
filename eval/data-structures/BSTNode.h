// Project: Binary_Search_Tree.cbp
// File   : BSTNode.h

//From: https://github.com/PacktPublishing/CPP-Data-Structures-and-Algorithms/tree/master/Chapter07/Binary_Search_Tree

#ifndef BSTNODE_H
#define BSTNODE_H

#include <iostream>

#define WRITE(m, k, v) (m.Insert(k))
#define READ(m, k) (m.Search(k))
#define TYPE BST

class BSTNode
{
public:
    int Key;
    BSTNode * Left;
    BSTNode * Right;
    BSTNode * Parent;
    int Height;
};

class BST
{
private:
    BSTNode * root;

protected:
    BSTNode * Insert(BSTNode * node, int key);
    void PrintTreeInOrder(BSTNode * node);
    BSTNode * Search(BSTNode * node, int key) const;
    int FindMin(BSTNode * node);
    int FindMax(BSTNode * node);
    int Successor(BSTNode * node);
    int Predecessor(BSTNode * node);
    BSTNode * Remove(BSTNode * node, int v);

public:
    BST();

    void Insert(int key);
    void PrintTreeInOrder();
    bool Search(int key) const;
    int FindMin();
    int FindMax();
    int Successor(int key);
    int Predecessor(int key);
    void Remove(int v);
};

#endif // BSTNODE_H
