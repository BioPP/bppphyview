// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "TreeDocument.h"

#include <Bpp/Text/TextTools.h>
#include <Bpp/Numeric/DataTable.h>

// From bpp-phyl:
#include <Bpp/Phyl/Tree/TreeTools.h>

// From Qt:
#include <QUndoCommand>
#include <QTime>

// From bpp-qt:
#include <Bpp/Qt/QtTools.h>

// From the STL:
#include <vector>

class AbstractCommand : public QUndoCommand
{
protected:
  std::shared_ptr<TreeDocument> doc_;
  std::shared_ptr<TreeTemplate<Node>> old_;
  std::shared_ptr<TreeTemplate<Node>> new_;

public:
  AbstractCommand(const QString& name, std::shared_ptr<TreeDocument> doc) :
    QUndoCommand(name),
    doc_(doc),
    old_(new TreeTemplate<Node>(doc->tree())),
    new_(nullptr)
  {}

  virtual ~AbstractCommand() = default;

public:
  void redo() { doOrUndo(); }
  void undo() { doOrUndo(); }

  virtual void doOrUndo()
  {
    doc_->setTree(*new_);
    doc_->modified(true);
    doc_->updateAllViews();
    new_.swap(old_);
  }
};

class SetLengthCommand : public AbstractCommand
{
public:
  SetLengthCommand(std::shared_ptr<TreeDocument> doc, double length) :
    AbstractCommand(QtTools::toQt("Set all lengths to " + TextTools::toString(length) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    new_->setBranchLengths(length);
  }
};

class DeleteLengthCommand : public AbstractCommand
{
public:
  DeleteLengthCommand(std::shared_ptr<TreeDocument> doc) :
    AbstractCommand(QtTools::toQt("Delete all branch lengths."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTemplateTools::deleteBranchLengths(new_->rootNode());
  }
};

class DeleteSupportValuesCommand : public AbstractCommand
{
public:
  DeleteSupportValuesCommand(std::shared_ptr<TreeDocument> doc) :
    AbstractCommand(QtTools::toQt("Delete all support values."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    std::vector<std::string> properties;
    properties.push_back(TreeTools::BOOTSTRAP);
    TreeTemplateTools::deleteBranchProperties(new_->rootNode(), properties);
  }
};


class InitGrafenCommand : public AbstractCommand
{
public:
  InitGrafenCommand(std::shared_ptr<TreeDocument> doc) :
    AbstractCommand("Init branch lengths (Grafen)", doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTools::initBranchLengthsGrafen(*new_);
  }
};

class ComputeGrafenCommand : public AbstractCommand
{
public:
  ComputeGrafenCommand(std::shared_ptr<TreeDocument> doc, double power) :
    AbstractCommand(QtTools::toQt("Compute branch lengths (Grafen), power=" + TextTools::toString(power) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTools::computeBranchLengthsGrafen(*new_, power, false);
  }
};

class ConvertToClockTreeCommand : public AbstractCommand
{
public:
  ConvertToClockTreeCommand(std::shared_ptr<TreeDocument> doc) :
    AbstractCommand(QtTools::toQt("Convert to clock tree"), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTools::convertToClockTree(*new_, new_->getRootId(), true);
  }
};

class SwapCommand : public AbstractCommand
{
public:
  SwapCommand(std::shared_ptr<TreeDocument> doc,
      int nodeId, unsigned int i1, unsigned int i2, int id1, int id2) :
    AbstractCommand(QtTools::toQt("Swap nodes " + TextTools::toString(id1) + " and " + TextTools::toString(id2) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    new_->swapNodes(nodeId, i1, i2);
  }
};

class OrderCommand : public AbstractCommand
{
public:
  OrderCommand(std::shared_ptr<TreeDocument> doc, int nodeId, bool downward) :
    AbstractCommand(QtTools::toQt("Order nodes in subtree " + TextTools::toString(nodeId) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTemplateTools::orderTree(*new_->getNode(nodeId), downward);
  }
};

class RerootCommand : public AbstractCommand
{
public:
  RerootCommand(std::shared_ptr<TreeDocument> doc, int nodeId) :
    AbstractCommand(QtTools::toQt("Reroot at " + TextTools::toString(nodeId) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    new_->rootAt(nodeId);
  }
};

class OutgroupCommand : public AbstractCommand
{
public:
  OutgroupCommand(std::shared_ptr<TreeDocument> doc, int nodeId) :
    AbstractCommand(QtTools::toQt("New outgroup: " + TextTools::toString(nodeId) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    new_->newOutGroup(nodeId);
  }
};

class MidpointRootingCommand : public AbstractCommand
{
public:
  MidpointRootingCommand(std::shared_ptr<TreeDocument> doc, const string& criterion) :
    AbstractCommand(QtTools::toQt("Midpoint rooting (" + criterion + ")."), doc)
  {
    short crit = 0;
    if (criterion == "Variance")
      crit = TreeTemplateTools::MIDROOT_VARIANCE;
    else if (criterion == "Sum of squares")
      crit = TreeTemplateTools::MIDROOT_SUM_OF_SQUARES;
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTemplateTools::midRoot(*new_, crit, true);
  }
};

class UnresolveUnsupportedNodesCommand : public AbstractCommand
{
public:
  UnresolveUnsupportedNodesCommand(std::shared_ptr<TreeDocument> doc, double threshold) :
    AbstractCommand(QtTools::toQt("Unresolve nodes with bootstrap < " + TextTools::toString(threshold) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    TreeTemplateTools::unresolveUncertainNodes(new_->rootNode(), threshold, TreeTools::BOOTSTRAP);
  }
};

class DeleteSubtreeCommand : public AbstractCommand
{
public:
  DeleteSubtreeCommand(std::shared_ptr<TreeDocument> doc, int nodeId) :
    AbstractCommand(QtTools::toQt("Delete substree " + TextTools::toString(nodeId) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    Node* node = new_->getNode(nodeId);
    TreeTemplateTools::dropSubtree(*new_, node);
  }
};

class InsertSubtreeAtNodeCommand : public AbstractCommand
{
public:
  InsertSubtreeAtNodeCommand(std::shared_ptr<TreeDocument> doc, int nodeId, Node* subtree) :
    AbstractCommand(QtTools::toQt("Insert substree at " + TextTools::toString(nodeId) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    Node* node = new_->getNode(nodeId);
    node->addSon(subtree);
    new_->resetNodesId();
  }
};

class InsertSubtreeOnBranchCommand : public AbstractCommand
{
public:
  InsertSubtreeOnBranchCommand(std::shared_ptr<TreeDocument> doc, int nodeId, Node* subtree) :
    AbstractCommand(QtTools::toQt("Insert substree below " + TextTools::toString(nodeId) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    Node* node = new_->getNode(nodeId);
    if (!node->hasFather())
    {
      // Need to change root:
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

class ChangeBranchLengthCommand : public AbstractCommand
{
public:
  ChangeBranchLengthCommand(std::shared_ptr<TreeDocument> doc, int nodeId, double newLength) :
    AbstractCommand(QtTools::toQt("Change length of node " + TextTools::toString(nodeId) + " to " + TextTools::toString(newLength) + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    Node* node = new_->getNode(nodeId);
    node->setDistanceToFather(newLength);
  }
};

class ChangeNodeNameCommand : public AbstractCommand
{
public:
  ChangeNodeNameCommand(std::shared_ptr<TreeDocument> doc, int nodeId, const string& newName) :
    AbstractCommand(QtTools::toQt("Change name of node " + TextTools::toString(nodeId) + " to " + newName + "."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    Node* node = new_->getNode(nodeId);
    node->setName(newName);
  }
};

class TranslateNodeNamesCommand : public AbstractCommand
{
public:
  TranslateNodeNamesCommand(
      std::shared_ptr<TreeDocument> doc,
      const DataTable& table,
      unsigned int from, unsigned int to);
};

class AttachDataCommand : public AbstractCommand
{
public:
  AttachDataCommand(
      std::shared_ptr<TreeDocument> doc,
      const DataTable& data,
      unsigned int index, bool useNames);

private:
  static void addProperties_(Node* node, const DataTable& data, unsigned int index, bool useNames);
};

class AddDataCommand : public AbstractCommand
{
public:
  AddDataCommand(std::shared_ptr<TreeDocument> doc, const QString& name);

private:
  static void addProperty_(Node* node, const QString& name);
};

class RemoveDataCommand : public AbstractCommand
{
public:
  RemoveDataCommand(std::shared_ptr<TreeDocument> doc, const QString& name);

private:
  static void removeProperty_(Node* node, const QString& name);
};

class RenameDataCommand : public AbstractCommand
{
public:
  RenameDataCommand(std::shared_ptr<TreeDocument> doc, const QString& oldName, const QString& newName);

private:
  static void renameProperty_(Node* node, const QString& oldName, const QString& newName);
};

class SampleSubtreeCommand : public AbstractCommand
{
public:
  SampleSubtreeCommand(std::shared_ptr<TreeDocument> doc, int nodeId, unsigned int size) :
    AbstractCommand(QtTools::toQt("Sample subtree " + TextTools::toString(nodeId) + " to " + TextTools::toString(size) + " leaves."), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
    Node* node = new_->getNode(nodeId);
    TreeTemplateTools::sampleSubtree(*new_, TreeTemplateTools::getLeavesNames(*node), size);
  }
};

class SnapCommand : public AbstractCommand
{
public:
  SnapCommand(std::shared_ptr<TreeDocument> doc) :
    AbstractCommand(QString("Tree snapshot (saved at ") + QTime::currentTime().toString("hh:mm:ss") + QString(")"), doc)
  {
    new_.reset(new TreeTemplate<Node>(*old_));
  }
};

class NaiveAsrCommand : public AbstractCommand
{
public:
  NaiveAsrCommand(std::shared_ptr<TreeDocument> doc, const string& name);

private:
  static std::string asr_(Node& node, const string& name);
};

class SetNamesFromDataCommand : public AbstractCommand
{
public:
  SetNamesFromDataCommand(std::shared_ptr<TreeDocument> doc, const string& propertyName, bool innerNodesOnly);
};


#endif // _COMMANDS_H_
