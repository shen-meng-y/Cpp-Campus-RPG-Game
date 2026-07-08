#ifndef BACKPACKDIALOG_H
#define BACKPACKDIALOG_H

#include <QDialog>
#include <QPoint>
#include <QPushButton>
#include <QString>
#include <QVector>
#include <vector>

#include "Character.h"
#include "Item.h"

class QLabel;
class QMouseEvent;
class QDragEnterEvent;
class QDropEvent;

class BackpackCellButton : public QPushButton {
    Q_OBJECT

public:
    explicit BackpackCellButton(int slotIndex, QWidget *parent = nullptr);

    int getSlotIndex() const;
    void setFilled(bool filled);

signals:
    void cellClicked(int slotIndex);
    void itemDropped(int fromSlot, int toSlot);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    int slotIndex;
    bool filled;
    QPoint dragStartPosition;
};

class BackpackDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackpackDialog(std::vector<Item> &backpack, Character &player, QWidget *parent = nullptr);

signals:
    void backpackChanged();
    void logRequested(const QString &message);

private slots:
    void selectSlot(int slotIndex);
    void moveItem(int fromSlot, int toSlot);
    void useSelectedItem();

private:
    enum { SlotCount = 20 };

    std::vector<Item> &backpack;
    Character &player;
    QVector<Item> slotItems;
    QVector<bool> slotFilled;
    QVector<BackpackCellButton *> cells;
    int selectedSlot;

    QLabel *nameLabel;
    QLabel *typeLabel;
    QLabel *priceLabel;
    QLabel *effectLabel;
    QLabel *descLabel;
    QPushButton *useButton;

    void buildUi();
    void loadItemsToSlots();
    void writeSlotsToBackpack();
    void refreshSlots();
    void refreshDetail();
    QString itemIcon(const Item &item) const;
    QString cellStyle(bool selected, bool filled) const;
};

#endif
