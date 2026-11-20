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

class FlightBST
{
    TreeNode* root;
    TreeNode* add(TreeNode* n, Flight* f)
    {
        if(n == NULL) return new TreeNode(f);
        if(f->callSign < n->flight->callSign) n->left = add(n->left,f);
        else if(f->callSign > n->flight->callSign) n->right = add(n->right,f);
        return n;
    }
    TreeNode* remove(TreeNode* n, string id)
    {
        if(n == NULL) return NULL;
        if(id < n->flight->callSign) n->left = remove(n->left,id);
        else if(id > n->flight->callSign) n->right = remove(n->right,id);
        else
        {
            if(n->left == NULL || n->right == NULL)
            {
                TreeNode* child = n->left == NULL ? n->right : n->left;
                delete n;
                return child;
            }
            TreeNode* p = n->right;
            while(p->left != NULL) p = p->left;
            n->flight = p->flight;
            n->right = remove(n->right,p->flight->callSign);
        }
        return n;
    }
    int getHeight(TreeNode* n)
    {
        if(n == NULL) return 0;
        int a=getHeight(n->left), b=getHeight(n->right);
        return 1+(a>b?a:b);
    }
    void show(TreeNode* n)
    {
        if(n == NULL) return;
        show(n->left);
        cout << n->flight->callSign << " ";
        show(n->right);
    }
    void clear(TreeNode* n)
    {
        if(n==NULL) return;
        clear(n->left); clear(n->right); delete n;
    }
public:
    FlightBST() { root=NULL; }
    ~FlightBST() { clear(root); }
    void insert(Flight* f) { if(f!=NULL) root=add(root,f); }
    void erase(string id) { root=remove(root,id); }
    Flight* search(string id)
    {
        TreeNode* p=root;
        while(p!=NULL)
        {
            if(id==p->flight->callSign) return p->flight;
            p=id<p->flight->callSign?p->left:p->right;
        }
        return NULL;
    }
    int height() { return getHeight(root); }
    void inorderTraversal() { show(root); cout << endl; }
};

// AVL with basic left and right rotations after insertion/removal.
class FlightAVL
{
    class FlightAVL
    {
        TreeNode* root;
        int h(TreeNode* n) { return n==NULL?0:n->h; }
        void fix(TreeNode* n)
        {
            int a=h(n->left), b=h(n->right);
            n->h=1+(a>b?a:b);
        }
        int balance(TreeNode* n) { return n==NULL?0:h(n->left)-h(n->right); }
        TreeNode* rotateRight(TreeNode* y)
        {
            TreeNode* x=y->left;
            TreeNode* middle=x->right;
            x->right=y;
            y->left=middle;
            fix(y); fix(x);
            return x;
        }
        TreeNode* rotateLeft(TreeNode* x)
        {
            TreeNode* y=x->right;
            TreeNode* middle=y->left;
            y->left=x;
            x->right=middle;
            fix(x); fix(y);
            return y;
        }
        TreeNode* balanceNode(TreeNode* n)
        {
            if(n==NULL) return NULL;
            fix(n);
            if(balance(n)>1)
            {
                if(balance(n->left)<0) n->left=rotateLeft(n->left);
                return rotateRight(n);
            }
            if(balance(n)<-1)
            {
                if(balance(n->right)>0) n->right=rotateRight(n->right);
                return rotateLeft(n);
            }
            return n;
        }
        TreeNode* add(TreeNode* n, Flight* f)
        {
            if(n==NULL) return new TreeNode(f);
            if(f->callSign<n->flight->callSign) n->left=add(n->left,f);
            else if(f->callSign>n->flight->callSign) n->right=add(n->right,f);
            return balanceNode(n);
        }
        TreeNode* remove(TreeNode* n, string id)
        {
            if(n==NULL) return NULL;
            if(id<n->flight->callSign) n->left=remove(n->left,id);
            else if(id>n->flight->callSign) n->right=remove(n->right,id);
            else
            {
                if(n->left==NULL || n->right==NULL)
                {
                    TreeNode* c=n->left==NULL?n->right:n->left;
                    delete n;
                    return c;
                }
                TreeNode* p=n->right;
                while(p->left!=NULL) p=p->left;
                n->flight=p->flight;
                n->right=remove(n->right,p->flight->callSign);
            }
            return balanceNode(n);
        }
        void show(TreeNode* n)
        {
            if(n==NULL) return;
            show(n->left); cout<<n->flight->callSign<<" "; show(n->right);
        }
        void clear(TreeNode* n)
        {
            if(n==NULL) return;
            clear(n->left); clear(n->right); delete n;
        }
    public:
        FlightAVL() { root=NULL; }
        ~FlightAVL() { clear(root); }

};
#endif
