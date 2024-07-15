// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#include "TreeCommands.h"

using namespace std;

TranslateNodeNamesCommand::TranslateNodeNamesCommand(
    std::shared_ptr<TreeDocument> doc,
    const DataTable& table,
    unsigned int from, unsigned int to) :
  AbstractCommand(QtTools::toQt("Translates nodes names from " + table.getColumnName(from) + " to " + table.getColumnName(to) + "."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  // Build translation:
  map<string, string> tln;
  for (unsigned int i = 0; i < table.getNumberOfRows(); ++i)
  {
    tln[table(i, from)] = table(i, to);
  }
  vector<Node*> nodes = new_->getNodes();
  for (unsigned int i = 0; i < nodes.size(); i++)
  {
    if (nodes[i]->hasName())
    {
      map<string, string>::iterator it = tln.find(nodes[i]->getName());
      if (it != tln.end())
      {
        nodes[i]->setName(it->second);
      }
    }
  }
}

AttachDataCommand::AttachDataCommand(
    std::shared_ptr<TreeDocument> doc,
    const DataTable& data,
    unsigned int index, bool useNames) :
  AbstractCommand(QtTools::toQt("Attach data to tree."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  addProperties_(new_->getRootNode(), data, index, useNames);
}

void AttachDataCommand::addProperties_(Node* node, const DataTable& data, unsigned int index, bool useNames)
{
  if (!useNames)
  {
    // Use id
    string id = TextTools::toString(node->getId());
    for (unsigned int i = 0; i < data.getNumberOfRows(); ++i)
    {
      if (data(i, index) == id)
      {
        for (unsigned int j = 0; j < data.getNumberOfColumns(); ++j)
        {
          if (j != index)
          {
            node->setNodeProperty(data.getColumnName(j), BppString(data(i, j)));
          }
        }
      }
    }
  }
  else
  {
    // Use name:
    if (node->hasName())
    {
      string name = node->getName();
      for (unsigned int i = 0; i < data.getNumberOfRows(); ++i)
      {
        if (data(i, index) == name)
        {
          for (unsigned int j = 0; j < data.getNumberOfColumns(); ++j)
          {
            if (j != index)
            {
              node->setNodeProperty(data.getColumnName(j), BppString(data(i, j)));
            }
          }
        }
      }
    }
  }
  for (unsigned int i = 0; i < node->getNumberOfSons(); ++i)
  {
    addProperties_(node->getSon(i), data, index, useNames);
  }
}

AddDataCommand::AddDataCommand(
    std::shared_ptr<TreeDocument> doc,
    const QString& name) :
  AbstractCommand(QString("Add data '") + name + QString("' to tree."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  addProperty_(new_->getRootNode(), name);
}

void AddDataCommand::addProperty_(Node* node, const QString& name)
{
  node->setNodeProperty(name.toStdString(), BppString(""));
  for (unsigned int i = 0; i < node->getNumberOfSons(); ++i)
  {
    addProperty_(node->getSon(i), name);
  }
}

RemoveDataCommand::RemoveDataCommand(
    std::shared_ptr<TreeDocument> doc,
    const QString& name) :
  AbstractCommand(QString("Remove data '") + name + QString("' from tree."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  removeProperty_(new_->getRootNode(), name);
}

void RemoveDataCommand::removeProperty_(Node* node, const QString& name)
{
  node->deleteNodeProperty(name.toStdString());
  for (unsigned int i = 0; i < node->getNumberOfSons(); ++i)
  {
    removeProperty_(node->getSon(i), name);
  }
}

RenameDataCommand::RenameDataCommand(
    std::shared_ptr<TreeDocument> doc,
    const QString& oldName,
    const QString& newName) :
  AbstractCommand(QString("Rename data '") + oldName + QString("' to '" + newName + "' from tree."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  renameProperty_(new_->getRootNode(), oldName, newName);
}

void RenameDataCommand::renameProperty_(Node* node, const QString& oldName, const QString& newName)
{
  if (node->hasNodeProperty(oldName.toStdString()))
  {
    Clonable* property = node->removeNodeProperty(oldName.toStdString());
    node->setNodeProperty(newName.toStdString(), *property);
    delete property;
  }
  for (unsigned int i = 0; i < node->getNumberOfSons(); ++i)
  {
    renameProperty_(node->getSon(i), oldName, newName);
  }
}

NaiveAsrCommand::NaiveAsrCommand(
    std::shared_ptr<TreeDocument> doc,
    const string& name) :
  AbstractCommand(QString("Naive Ancestral State Reconstruction of variable '") + QString(name.c_str()) + QString("'."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  auto state = asr_(new_->rootNode(), name);
  new_->rootNode().setNodeProperty(name, BppString(state));
}

string NaiveAsrCommand::asr_(Node& node, const string& name)
{
  if (node.isLeaf()) {
    if (node.hasNodeProperty(name)) {
      return dynamic_cast<const BppString*>(node.getNodeProperty(name))->toSTL();
    } else {
      return "";
    }
  } else {
    vector<string> states;
    //We first call the function recursively on all subtrees:
    for (size_t i = 0; i < node.getNumberOfSons(); ++i) {
      auto state = asr_(node.son(i), name);
      node.son(i).setNodeProperty(name, BppString(state));
      states.push_back(state);
    }
    string ancestor = "";
    for (const auto& state : states) {
      if (state == "") return "";
      if (ancestor == "") ancestor = state;
      else if (ancestor != state) return "";
    }
    return ancestor;  
  }
}

SetNamesFromDataCommand::SetNamesFromDataCommand(
    std::shared_ptr<TreeDocument> doc,
    const string& propertyName,
    bool innerNodesOnly) :
  AbstractCommand(QString("Set names from variable '") + QString(propertyName.c_str()) + QString("'."), doc)
{
  new_.reset(new TreeTemplate<Node>(*old_));
  auto nodes = new_->getNodes();
  for (auto* node : nodes) {
    if (node->hasNodeProperty(propertyName)) {
      if (node->isLeaf()) {
	if (!innerNodesOnly) {
	  string name = dynamic_cast<BppString*>(node->getNodeProperty(propertyName))->toSTL();
	  node->setName(name);	
        } // else do nothing
      } else {
	string name = dynamic_cast<BppString*>(node->getNodeProperty(propertyName))->toSTL();
	node->setName(name);	
      }
    }
  }
}


