#ifndef TREEINDEX_H
#define TREEINDEX_H
#include "flight.h"

// Simple BST, for printing callsigns in sorted order.
struct TreeNode
{
    Flight* flight;
    TreeNode* left;
    TreeNode* right;
    int h;
    TreeNode(Flight* f)
    {
        flight = f;
        left = right = NULL;
        h = 1;
    }
};
