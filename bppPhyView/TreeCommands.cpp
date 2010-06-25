//
// File: TreeCommands.cpp
// Created by: Julien Dutheil
// Created on: Fri Oct 13 21:25 2006
//

/*
Copyright or © or Copr. CNRS, (November 16, 2004)

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

#include "TreeCommands.h"

using namespace std;

TranslateNodeNamesCommand::TranslateNodeNamesCommand(TreeDocument* doc, const DataTable& table, unsigned int from, unsigned int to):
  AbstractCommand(QtTools::toQt("Translates nodes names from " + table.getColumnName(from) + " to " + table.getColumnName(to) + "."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
  //Build translation:
  map<string, string> tln;
  for(unsigned int i = 0; i < table.getNumberOfRows(); ++i) {
    tln[table(i, from)] = table(i, to);
  }
  vector<Node*> nodes = new_->getNodes();
  for (unsigned int i = 0; i < nodes.size(); i++) {
    if (nodes[i]->hasName()) {
      map<string,string>::iterator it = tln.find(nodes[i]->getName());
      if (it != tln.end()) {
        nodes[i]->setName(it->second);
      }
    }
  }
}

AttachDataCommand::AttachDataCommand(TreeDocument* doc, const DataTable& data, unsigned int index, bool useNames):
  AbstractCommand(QtTools::toQt("Attach data to tree."), doc)
{
  new_ = new TreeTemplate<Node>(*old_);
  addProperties_(new_->getRootNode(), data, index, useNames);
}

void AttachDataCommand::addProperties_(Node* node, const DataTable& data, unsigned int index, bool useNames)
{
  if (!useNames) {
    //Use id
    string id = TextTools::toString(node->getId());
    for (unsigned int i = 0; i < data.getNumberOfRows(); ++i) {
      if (data(i, index) == id) {
        for (unsigned int j = 0; j < data.getNumberOfColumns(); ++j) {
          if (j != index) {
            node->setNodeProperty(data.getColumnName(j), BppString(data(i, j)));
          }
        }
      }
    }
  } else {
    //Use name:
    if (node->hasName()) {
      string name = node->getName();
      for (unsigned int i = 0; i < data.getNumberOfRows(); ++i) {
        if (data(i, index) == name) {
          for (unsigned int j = 0; j < data.getNumberOfColumns(); ++j) {
            if (j != index) {
              node->setNodeProperty(data.getColumnName(j), BppString(data(i, j)));
            }
          }
        }
      }
    }
  }
  for (unsigned int i = 0; i < node->getNumberOfSons(); ++i)
    addProperties_(node->getSon(i), data, index, useNames);
}

