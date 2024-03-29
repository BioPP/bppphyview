//
// File: TreeCommands.h
// Created by: Julien Dutheil
// Created on: Fri Oct 13 21:25 2006
//

/*
Copyright or � or Copr. Bio++ Development Team, (November 16, 2004)

This software is a computer program whose purpose is to provide classes
for phylogenetic data analysis.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "TreeDocument.h"

#include <Bpp/Text/TextTools.h>
#include <Bpp/Numeric/DataTable.h>

//From bpp-phyl:
#include <Bpp/Phyl/Tree/TreeTools.h>

//From Qt:
#include <QUndoCommand>
#include <QTime>

//From bpp-qt:
#include <Bpp/Qt/QtTools.h>

//From the STL:
#include <vector>

class AbstractCommand: public QUndoCommand
{
  protected:
    TreeDocument* doc_;
    TreeTemplate<Node>* old_;
    TreeTemplate<Node>* new_;

  public:
    AbstractCommand(const QString& name, TreeDocument* doc):
      QUndoCommand(name),
      doc_(doc),
      old_(new TreeTemplate<Node>(*doc->getTree())),
      new_(0)
    {}

    virtual ~AbstractCommand()
    {
      if (old_) delete old_;
      if (new_) delete new_;
    }

  public:
    void redo() { doOrUndo(); }
    void undo() { doOrUndo(); }
    
    virtual void doOrUndo()
    {
      doc_->setTree(*new_);
      doc_->modified(true);
      doc_->updateAllViews();
      TreeTemplate<Node>* tmp = new_;
      new_ = old_;
      old_ = tmp;
    }  
};

class SetLengthCommand: public AbstractCommand
{
  public:
    SetLengthCommand(TreeDocument* doc, double length):
      AbstractCommand(QtTools::toQt("Set all lengths to " + TextTools::toString(length) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      new_->setBranchLengths(length);
    }
};

class DeleteLengthCommand: public AbstractCommand
{
  public:
    DeleteLengthCommand(TreeDocument* doc):
      AbstractCommand(QtTools::toQt("Delete all branch lengths."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      TreeTemplateTools::deleteBranchLengths(*new_->getRootNode());
    }
};

class DeleteSupportValuesCommand: public AbstractCommand
{
  public:
    DeleteSupportValuesCommand(TreeDocument* doc):
      AbstractCommand(QtTools::toQt("Delete all support values."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      std::vector<std::string> properties;
      properties.push_back(TreeTools::BOOTSTRAP);
      TreeTemplateTools::deleteBranchProperties(*new_->getRootNode(), properties);
    }
};


class InitGrafenCommand: public AbstractCommand
{
  public:
    InitGrafenCommand(TreeDocument* doc):
      AbstractCommand("Init branch lengths (Grafen)", doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      TreeTools::initBranchLengthsGrafen(*new_);
    }
};

class ComputeGrafenCommand: public AbstractCommand
{
  public:
    ComputeGrafenCommand(TreeDocument* doc, double power):
      AbstractCommand(QtTools::toQt("Compute branch lengths (Grafen), power=" + TextTools::toString(power) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      TreeTools::computeBranchLengthsGrafen(*new_, power, false);
    }
};

class ConvertToClockTreeCommand: public AbstractCommand
{
  public:
    ConvertToClockTreeCommand(TreeDocument* doc):
      AbstractCommand(QtTools::toQt("Convert to clock tree"), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      TreeTools::convertToClockTree(*new_, new_->getRootId(), true);
    }
};

class SwapCommand: public AbstractCommand
{
  public:
    SwapCommand(TreeDocument* doc, int nodeId, unsigned int i1, unsigned int i2, int id1, int id2):
      AbstractCommand(QtTools::toQt("Swap nodes " + TextTools::toString(id1) + " and " + TextTools::toString(id2) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      new_->swapNodes(nodeId, i1, i2);
    }
};

class OrderCommand: public AbstractCommand
{
  public:
    OrderCommand(TreeDocument* doc, int nodeId, bool downward):
      AbstractCommand(QtTools::toQt("Order nodes in subtree " + TextTools::toString(nodeId) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      TreeTemplateTools::orderTree(*new_->getNode(nodeId), downward);
    }
};

class RerootCommand: public AbstractCommand
{
  public:
    RerootCommand(TreeDocument* doc, int nodeId):
      AbstractCommand(QtTools::toQt("Reroot at " + TextTools::toString(nodeId) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      new_->rootAt(nodeId);
    }
};

class OutgroupCommand: public AbstractCommand
{
  public:
    OutgroupCommand(TreeDocument* doc, int nodeId):
      AbstractCommand(QtTools::toQt("New outgroup: " + TextTools::toString(nodeId) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      new_->newOutGroup(nodeId);
    }
};

class MidpointRootingCommand: public AbstractCommand
{
  public:
    MidpointRootingCommand(TreeDocument* doc, const string& criterion) :
      AbstractCommand(QtTools::toQt("Midpoint rooting (" + criterion + ")."), doc)
    {
      short crit = 0;
      if (criterion == "Variance")
        crit = TreeTemplateTools::MIDROOT_VARIANCE;
      else if (criterion == "Sum of squares")
        crit = TreeTemplateTools::MIDROOT_SUM_OF_SQUARES;
      new_ = new TreeTemplate<Node>(*old_);
      TreeTemplateTools::midRoot(*new_, crit, true);
    }
};

class UnresolveUnsupportedNodesCommand: public AbstractCommand
{
  public:
    UnresolveUnsupportedNodesCommand(TreeDocument* doc, double threshold) :
      AbstractCommand(QtTools::toQt("Unresolve nodes with bootstrap < " + TextTools::toString(threshold) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      TreeTemplateTools::unresolveUncertainNodes(*new_->getRootNode(), threshold, TreeTools::BOOTSTRAP);
    }
};

class DeleteSubtreeCommand: public AbstractCommand
{
  public:
    DeleteSubtreeCommand(TreeDocument* doc, int nodeId):
      AbstractCommand(QtTools::toQt("Delete substree " + TextTools::toString(nodeId) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      Node* node = new_->getNode(nodeId);
      TreeTemplateTools::dropSubtree(*new_, node);
    }
};

class InsertSubtreeAtNodeCommand: public AbstractCommand
{
  public:
    InsertSubtreeAtNodeCommand(TreeDocument* doc, int nodeId, Node* subtree):
      AbstractCommand(QtTools::toQt("Insert substree at " + TextTools::toString(nodeId) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      Node* node = new_->getNode(nodeId);
      node->addSon(subtree);
      new_->resetNodesId();
    }
};

class InsertSubtreeOnBranchCommand: public AbstractCommand
{
  public:
    InsertSubtreeOnBranchCommand(TreeDocument* doc, int nodeId, Node* subtree):
      AbstractCommand(QtTools::toQt("Insert substree below " + TextTools::toString(nodeId) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      Node* node = new_->getNode(nodeId);
      if (!node->hasFather())
      {
        //Need to change root:
        Node* father = new Node();
        node->addSon(new_->getRootNode());
        node->addSon(subtree);
        new_->setRootNode(father);
      }
      else
      {
        Node* father = node->getFather();
        father->removeSon(node);
        Node* base = new Node();
        base->addSon(node);
        base->addSon(subtree);
        father->addSon(base);
      }
      new_->resetNodesId();
    }
};

class ChangeBranchLengthCommand: public AbstractCommand
{
  public:
    ChangeBranchLengthCommand(TreeDocument* doc, int nodeId, double newLength):
      AbstractCommand(QtTools::toQt("Change length of node " + TextTools::toString(nodeId) + " to " + TextTools::toString(newLength) + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      Node* node = new_->getNode(nodeId);
      node->setDistanceToFather(newLength);
    }
};

class ChangeNodeNameCommand: public AbstractCommand
{
  public:
    ChangeNodeNameCommand(TreeDocument* doc, int nodeId, const string& newName):
      AbstractCommand(QtTools::toQt("Change name of node " + TextTools::toString(nodeId) + " to " + newName + "."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      Node* node = new_->getNode(nodeId);
      node->setName(newName);
    }
};

class TranslateNodeNamesCommand: public AbstractCommand
{
  public:
    TranslateNodeNamesCommand(TreeDocument* doc, const DataTable& table, unsigned int from, unsigned int to);
};

class AttachDataCommand: public AbstractCommand
{
  public:
    AttachDataCommand(TreeDocument* doc, const DataTable& data, unsigned int index, bool useNames);

  private:
    static void addProperties_(Node* node, const DataTable& data, unsigned int index, bool useNames);
};

class AddDataCommand: public AbstractCommand
{
  public:
    AddDataCommand(TreeDocument* doc, const QString& name);

  private:
    static void addProperty_(Node* node, const QString& name);
};

class RemoveDataCommand: public AbstractCommand
{
  public:
    RemoveDataCommand(TreeDocument* doc, const QString& name);

  private:
    static void removeProperty_(Node* node, const QString& name);
};

class RenameDataCommand: public AbstractCommand
{
  public:
    RenameDataCommand(TreeDocument* doc, const QString& oldName, const QString& newName);

  private:
    static void renameProperty_(Node* node, const QString& oldName, const QString& newName);
};

class SampleSubtreeCommand: public AbstractCommand
{
  public:
    SampleSubtreeCommand(TreeDocument* doc, int nodeId, unsigned int size):
      AbstractCommand(QtTools::toQt("Sample subtree " + TextTools::toString(nodeId) + " to " + TextTools::toString(size) + " leaves."), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
      Node* node = new_->getNode(nodeId);
      TreeTemplateTools::sampleSubtree(*new_, TreeTemplateTools::getLeavesNames(*node), size);
    }
};

class SnapCommand: public AbstractCommand
{
  public:
    SnapCommand(TreeDocument* doc):
      AbstractCommand(QString("Tree snapshot (saved at ") + QTime::currentTime().toString("hh:mm:ss") + QString(")"), doc)
    {
      new_ = new TreeTemplate<Node>(*old_);
    }
};

#endif //_COMMANDS_H_

