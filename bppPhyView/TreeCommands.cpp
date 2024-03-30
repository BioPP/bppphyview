// SPDX-FileCopyrightText: The Bio++ Development Group
//
// SPDX-License-Identifier: CECILL-2.1

#include "TreeCommands.h"

using namespace std;

TranslateNodeNamesCommand::TranslateNodeNamesCommand(TreeDocument* doc, const DataTable& table, unsigned int from, unsigned int to) :
  AbstractCommand(QtTools::toQt("Translates nodes names from " + table.getColumnName(from) + " to " + table.getColumnName(to) + "."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
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

AttachDataCommand::AttachDataCommand(TreeDocument* doc, const DataTable& data, unsigned int index, bool useNames) :
  AbstractCommand(QtTools::toQt("Attach data to tree."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
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

AddDataCommand::AddDataCommand(TreeDocument* doc, const QString& name) :
  AbstractCommand(QString("Add data '") + name + QString("' to tree."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
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

RemoveDataCommand::RemoveDataCommand(TreeDocument* doc, const QString& name) :
  AbstractCommand(QString("Remove data '") + name + QString("' from tree."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
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

RenameDataCommand::RenameDataCommand(TreeDocument* doc, const QString& oldName, const QString& newName) :
  AbstractCommand(QString("Rename data '") + oldName + QString("' to '" + newName + "' from tree."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
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
