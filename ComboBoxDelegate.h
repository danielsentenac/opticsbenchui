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

#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QtSql>
#include <QtWidgets>

/// \ingroup ui
/// Item delegate providing a combo box editor.
class ComboBoxDelegate : public QItemDelegate
{
   Q_OBJECT

public:
  /// Construct a combo box delegate with an optional item list.
  /// \param parent Parent object.
  /// \param itemlist List of items for the combo box.
  ComboBoxDelegate(QObject *parent = nullptr, QStringList *itemlist = nullptr);
  /// Destructor.
  ~ComboBoxDelegate() override;
  /// Create the editor widget.
  /// \param parent Parent widget.
  /// \param option Style options.
  /// \param index Model index.
  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const override;

  /// Populate the editor with model data.
  /// \param editor Editor widget.
  /// \param index Model index.
  void setEditorData(QWidget *editor, const QModelIndex &index) const override;
  /// Commit editor data back to the model.
  /// \param editor Editor widget.
  /// \param model Model to update.
  /// \param index Model index.
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override;

  /// Update editor geometry to fit the item.
  /// \param editor Editor widget.
  /// \param option Style options.
  /// \param index Model index.
  void updateEditorGeometry(QWidget *editor,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const override;

private:
  QStringList *itemlist;
};
#endif
