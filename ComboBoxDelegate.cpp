/*******************************************************************
This file is part of OpticsBenchUI.

OpticsBenchUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "ComboBoxDelegate.h"

ComboBoxDelegate::ComboBoxDelegate(QObject *parent, QStringList *_itemlist)
{
 itemlist = _itemlist;
}

ComboBoxDelegate::~ComboBoxDelegate()
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
     const QStyleOptionViewItem &,
     const QModelIndex &index) const
{ 
      QComboBox *editor = new QComboBox(parent);
      editor->addItems(*itemlist);
      return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);
  QString item = index.model()->data(index, Qt::EditRole).toString();
  for ( int i = 0 ; i < combobox->count();i++ ) {
   if ( combobox->itemText(i) == item ) {
    combobox->setCurrentIndex(i);
    break;
   }
  }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const
{
     QComboBox *combobox = static_cast<QComboBox*>(editor);
     QString item = combobox->itemText(combobox->currentIndex());
     model->setData(index,item, Qt::EditRole);
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
       const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QComboBox *combobox = static_cast<QComboBox*>(editor);
  editor->setGeometry(option.rect);
  int maxsize = 0;
  for ( int i = 0 ; i < combobox->count();i++ ) {
    if ( maxsize < combobox->itemText(i).size() )
      maxsize = combobox->itemText(i).size();   
  }
  editor->setMinimumWidth(maxsize * 10);
}

