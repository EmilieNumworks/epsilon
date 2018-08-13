#ifndef POINCARE_TREE_POOL_H
#define POINCARE_TREE_POOL_H

#include "tree_node.h"
#include <poincare/ghost_node.h>
#include <stddef.h>
#include <string.h>
#include <new>
#if POINCARE_TREE_LOG
#include <ostream>
#endif

namespace Poincare {

class TreeByReference;

class TreePool {
  friend class TreeNode;
  friend class TreeByReference;
public:
  static constexpr int NoNodeIdentifier = -1;
  static constexpr int FirstStaticNodeIdentifier = -2;

  static TreePool * sharedPool();

  // Node
  TreeNode * node(int identifier) const;
  TreeNode * last() const { return reinterpret_cast<TreeNode *>(const_cast<char *>(m_cursor)); }

  template <typename T>
  T * createTreeNode(size_t size = sizeof(T)) {
    int nodeIdentifier = generateIdentifier();
    if (nodeIdentifier == -1) {
      T::FailedAllocationStaticNode()->retain();
      return T::FailedAllocationStaticNode();
    }
    void * ptr = alloc(size);
    if (ptr == nullptr) {
      T::FailedAllocationStaticNode()->retain();
      return T::FailedAllocationStaticNode();
    }
    T * node = new(ptr) T();
    node->rename(nodeIdentifier, false);

    // Ensure the pool is syntactially correct by creating ghost children if needed.
    // It's needed for children that have a fixed, non-zero number of children.
    for (int i = 0; i < node->numberOfChildren(); i++) {
      TreeNode * ghost = createTreeNode<GhostNode>();
      if (ghost->isAllocationFailure()) {
        /* There is no room to create the node and all of its children, so
         * delete the node, the children already built and return an allocation
         * failure node. */
        // Discard the node and its first children
        discardTreeNode(node);
        for (int j = 0; j < i; j++) {
          /* The pool has been compacted, so the next child to discard is at the
           * address "node" */
          discardTreeNode(node);
        }
        // Return an allocation failure node
        T::FailedAllocationStaticNode()->retain();
        return T::FailedAllocationStaticNode();
      }
      ghost->retain();
      move(node->next(), ghost, 0);
    }
    return node;
  }

  void move(TreeNode * destination, TreeNode * source, int realNumberOfSourceChildren);
  void moveChildren(TreeNode * destination, TreeNode * sourceParent);

  TreeNode * deepCopy(TreeNode * node) {
    size_t size = node->deepSize(-1);
    void * ptr = alloc(size);
    if (ptr == nullptr) {
      node->failedAllocationStaticNode()->retain();
      return node->failedAllocationStaticNode();
    }
    memcpy(ptr, static_cast<void *>(node), size);
    TreeNode * copy = reinterpret_cast<TreeNode *>(ptr);
    renameNode(copy, false);
    for (TreeNode * child : copy->depthFirstChildren()) {
      renameNode(child, false);
      child->retain();
    }
    return copy;
  }

  void registerStaticNodeIfRequired(TreeNode * node);

  int registerStaticNode(TreeNode * node) {
    int nodeID = 0;
    while (m_staticNodes[nodeID] != nullptr && nodeID < MaxNumberOfStaticNodes) {
      nodeID++;
    }
    assert(nodeID < MaxNumberOfStaticNodes);
    m_staticNodes[nodeID] = node;
    return identifierOfStaticNodeAtIndex(nodeID);
  }

#if POINCARE_TREE_LOG
  void flatLog(std::ostream & stream);
  void treeLog(std::ostream & stream);
#endif

  int numberOfNodes() const {
    int count = 0;
    TreeNode * firstNode = first();
    TreeNode * lastNode = last();
    while (firstNode != lastNode) {
      count++;
      firstNode = firstNode->next();
    }
    return count;
  }

private:
  constexpr static int BufferSize = 1024;//2048;
  constexpr static int MaxNumberOfNodes = BufferSize/sizeof(TreeNode);
  constexpr static int MaxNumberOfStaticNodes = 200; // TODO: count how may are needed

  // TreeNode
  void discardTreeNode(TreeNode * node) {
    int nodeIdentifier = node->identifier();
    size_t size = node->size();
    node->~TreeNode();
    dealloc(node, size);
    freeIdentifier(nodeIdentifier);
  }

  void registerNode(TreeNode * node) {
    int nodeID = node->identifier();
    if (nodeID >= 0 && nodeID < MaxNumberOfNodes) {
      m_nodeForIdentifier[nodeID] = node;
    }
  }

  void unregisterNode(TreeNode * node) {
    freeIdentifier(node->identifier());
  }

  void renameNode(TreeNode * node, bool unregisterPreviousIdentifier = true) {
    node->rename(generateIdentifier(), unregisterPreviousIdentifier);
  }

  int identifierOfStaticNodeAtIndex(int index) const { return FirstStaticNodeIdentifier-index;} // We do not want positive indexes that are reserved for pool nodes, and -1 is reserved for node initialisation.
  int indexOfStaticNode(int id) const { return -(id - FirstStaticNodeIdentifier);}

  // Iterators

  TreeNode * first() const { return reinterpret_cast<TreeNode *>(const_cast<char *>(m_buffer)); }

  class Nodes {
  public:
    Nodes(TreeNode * node) : m_node(node) {}
    class Iterator : public TreeNode::Iterator<TreeNode> {
    public:
      using TreeNode::Iterator<TreeNode>::Iterator;
      Iterator & operator++() {
        m_node = m_node->next();
        return *this;
      }
    };
    Iterator begin() const { return Iterator(m_node); }
    Iterator end() const { return Iterator(TreePool::sharedPool()->last()); }
  private:
    TreeNode * m_node;
  };
  Nodes allNodes() { return Nodes(*(begin())); }

  class RootNodes {
  public:
    RootNodes(TreeNode * node) : m_node(node) {}
    class Iterator : public TreeNode::Iterator<TreeNode> {
    public:
      using TreeNode::Iterator<TreeNode>::Iterator;
      Iterator & operator++() {
        m_node = m_node->nextSibling();
        return *this;
      }
    };
    Iterator begin() const { return Iterator(m_node); }
    Iterator end() const { return Iterator(TreePool::sharedPool()->last()); }
  private:
    TreeNode * m_node;
  };
  RootNodes roots() { return RootNodes(*(begin())); }

  TreeNode::DepthFirst<TreeNode>::Iterator begin() const { return TreeNode::DepthFirst<TreeNode>::Iterator(first()); }
  TreeNode::DepthFirst<TreeNode>::Iterator end() const { return TreeNode::DepthFirst<TreeNode>::Iterator(last()); }

  TreePool() : m_cursor(m_buffer) { }

  // Pool memory
  void * alloc(size_t size);
  void dealloc(TreeNode * ptr, size_t size);
  bool insert(char * destination, char * source, size_t length);
  void moveNodes(TreeNode * destination, TreeNode * source, size_t moveLength);

  // Identifiers
  int generateIdentifier() {
    int newIdentifier = -1;
    for (int i = 0; i < MaxNumberOfNodes; i++) {
      if (m_nodeForIdentifier[i] == nullptr) {
        newIdentifier = i;
        break;
      }
    }
    return newIdentifier;
  }

  void freeIdentifier(int identifier) {
    if (identifier >= 0 && identifier < MaxNumberOfNodes) {
      m_nodeForIdentifier[identifier] = nullptr;
    }
  }

  // Debug
  void logNodeForIdentifierArray();

  char * m_cursor;
  char m_buffer[BufferSize];
  TreeNode * m_nodeForIdentifier[MaxNumberOfNodes];
  TreeNode * m_staticNodes[MaxNumberOfStaticNodes];
};

}

#endif
