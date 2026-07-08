#include "backpackdialog.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>

BackpackCellButton::BackpackCellButton(int slotIndex, QWidget *parent)
    : QPushButton(parent),
    slotIndex(slotIndex),
    filled(false) {
    setAcceptDrops(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(88, 76);
}

int BackpackCellButton::getSlotIndex() const {
    return slotIndex;
}

void BackpackCellButton::setFilled(bool filled) {
    this->filled = filled;
}

void BackpackCellButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
        emit cellClicked(slotIndex);
    }

    QPushButton::mousePressEvent(event);
}

void BackpackCellButton::mouseMoveEvent(QMouseEvent *event) {
    if (!(event->buttons() & Qt::LeftButton) || !filled) {
        return;
    }

    int distance = (event->pos() - dragStartPosition).manhattanLength();
    if (distance < QApplication::startDragDistance()) {
        return;
    }

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-backpack-slot", QByteArray::number(slotIndex));

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}

void BackpackCellButton::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasFormat("application/x-backpack-slot")) {
        event->acceptProposedAction();
    }
}

void BackpackCellButton::dropEvent(QDropEvent *event) {
    QByteArray data = event->mimeData()->data("application/x-backpack-slot");
    bool ok = false;
    int fromSlot = data.toInt(&ok);

    if (ok) {
        emit itemDropped(fromSlot, slotIndex);
        event->acceptProposedAction();
    }
}

BackpackDialog::BackpackDialog(std::vector<Item> &backpack, Character &player, QWidget *parent)
    : QDialog(parent),
    backpack(backpack),
    player(player),
    slotItems(SlotCount),
    slotFilled(SlotCount, false),
    selectedSlot(-1),
    nameLabel(nullptr),
    typeLabel(nullptr),
    priceLabel(nullptr),
    effectLabel(nullptr),
    descLabel(nullptr),
    useButton(nullptr) {
    setWindowTitle("拖拽格子背包");
    resize(820, 520);

    buildUi();
    loadItemsToSlots();
    refreshSlots();
    refreshDetail();
}

void BackpackDialog::buildUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(18, 18, 18, 18);
    mainLayout->setSpacing(18);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setSpacing(10);

    for (int i = 0; i < SlotCount; ++i) {
        BackpackCellButton *cell = new BackpackCellButton(i, this);
        connect(cell, &BackpackCellButton::cellClicked, this, &BackpackDialog::selectSlot);
        connect(cell, &BackpackCellButton::itemDropped, this, &BackpackDialog::moveItem);

        cells.push_back(cell);
        gridLayout->addWidget(cell, i / 5, i % 5);
    }

    QWidget *gridWidget = new QWidget(this);
    gridWidget->setLayout(gridLayout);
    mainLayout->addWidget(gridWidget, 3);

    QWidget *detailWidget = new QWidget(this);
    detailWidget->setObjectName("detailWidget");
    QVBoxLayout *detailLayout = new QVBoxLayout(detailWidget);
    detailLayout->setContentsMargins(18, 18, 18, 18);
    detailLayout->setSpacing(12);

    QLabel *titleLabel = new QLabel("物品详情", detailWidget);
    titleLabel->setObjectName("titleLabel");

    nameLabel = new QLabel(detailWidget);
    typeLabel = new QLabel(detailWidget);
    priceLabel = new QLabel(detailWidget);
    effectLabel = new QLabel(detailWidget);
    descLabel = new QLabel(detailWidget);
    descLabel->setWordWrap(true);

    useButton = new QPushButton("使用物品", detailWidget);
    QPushButton *closeButton = new QPushButton("关闭", detailWidget);

    connect(useButton, &QPushButton::clicked, this, &BackpackDialog::useSelectedItem);
    connect(closeButton, &QPushButton::clicked, this, &BackpackDialog::accept);

    detailLayout->addWidget(titleLabel);
    detailLayout->addWidget(nameLabel);
    detailLayout->addWidget(typeLabel);
    detailLayout->addWidget(priceLabel);
    detailLayout->addWidget(effectLabel);
    detailLayout->addWidget(descLabel);
    detailLayout->addStretch();
    detailLayout->addWidget(useButton);
    detailLayout->addWidget(closeButton);

    mainLayout->addWidget(detailWidget, 2);

    setStyleSheet(
        "QDialog {"
        "   background-color: #f8f5ff;"
        "   font-family: 'Microsoft YaHei';"
        "}"
        "QPushButton {"
        "   font-size: 15px;"
        "   font-weight: 600;"
        "   color: #5a4a7a;"
        "   background-color: #ffffff;"
        "   border: 2px solid #d7c8f2;"
        "   border-radius: 12px;"
        "   padding: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #f0e9ff;"
        "   border-color: #bda7ea;"
        "}"
        "#detailWidget {"
        "   background-color: rgba(255, 255, 255, 230);"
        "   border: 2px solid #d7c8f2;"
        "   border-radius: 14px;"
        "}"
        "#titleLabel {"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "   color: #6b4f8a;"
        "}"
        "QLabel {"
        "   font-size: 15px;"
        "   color: #5a4a7a;"
        "}"
        );
}

void BackpackDialog::loadItemsToSlots() {
    for (int i = 0; i < SlotCount; ++i) {
        slotFilled[i] = false;
    }

    int count = static_cast<int>(backpack.size());
    if (count > SlotCount) {
        count = SlotCount;
    }
    for (int i = 0; i < count; ++i) {
        slotItems[i] = backpack[i];
        slotFilled[i] = true;
    }
}

void BackpackDialog::writeSlotsToBackpack() {
    backpack.clear();

    for (int i = 0; i < SlotCount; ++i) {
        if (slotFilled[i]) {
            backpack.push_back(slotItems[i]);
        }
    }
}

void BackpackDialog::refreshSlots() {
    for (int i = 0; i < SlotCount; ++i) {
        BackpackCellButton *cell = cells[i];
        cell->setFilled(slotFilled[i]);

        if (slotFilled[i]) {
            const Item &item = slotItems[i];
            cell->setText(itemIcon(item) + "\n" + QString::fromStdString(item.getName()));
        } else {
            cell->setText("空");
        }

        cell->setStyleSheet(cellStyle(i == selectedSlot, slotFilled[i]));
    }
}

void BackpackDialog::refreshDetail() {
    bool hasItem = selectedSlot >= 0
                   && selectedSlot < SlotCount
                   && slotFilled[selectedSlot];

    useButton->setEnabled(hasItem);

    if (!hasItem) {
        nameLabel->setText("名称：未选择");
        typeLabel->setText("类型：-");
        priceLabel->setText("价格：-");
        effectLabel->setText("效果：-");
        descLabel->setText("说明：点击左侧物品格子查看详情。");
        return;
    }

    const Item &item = slotItems[selectedSlot];
    nameLabel->setText("名称：" + QString::fromStdString(item.getName()));
    typeLabel->setText("类型：" + QString::fromStdString(Item::typeToString(item.getType())));
    priceLabel->setText("价格：" + QString::number(item.getPrice()));
    effectLabel->setText("效果：" + QString::number(item.getEffectValue()));
    descLabel->setText("说明：" + QString::fromStdString(item.getDescription()));
}

void BackpackDialog::selectSlot(int slotIndex) {
    selectedSlot = slotIndex;
    refreshSlots();
    refreshDetail();
}

void BackpackDialog::moveItem(int fromSlot, int toSlot) {
    if (fromSlot < 0 || fromSlot >= SlotCount || toSlot < 0 || toSlot >= SlotCount) {
        return;
    }

    if (fromSlot == toSlot || !slotFilled[fromSlot]) {
        return;
    }

    std::swap(slotItems[fromSlot], slotItems[toSlot]);
    std::swap(slotFilled[fromSlot], slotFilled[toSlot]);

    selectedSlot = toSlot;
    writeSlotsToBackpack();
    refreshSlots();
    refreshDetail();

    emit backpackChanged();
}

void BackpackDialog::useSelectedItem() {
    if (selectedSlot < 0 || selectedSlot >= SlotCount || !slotFilled[selectedSlot]) {
        QMessageBox::information(this, "提示", "请先选择一个物品。");
        return;
    }

    Item item = slotItems[selectedSlot];
    player.useItem(item);

    slotFilled[selectedSlot] = false;
    writeSlotsToBackpack();

    emit logRequested("使用物品：" + QString::fromStdString(item.getName()));
    emit backpackChanged();

    refreshSlots();
    refreshDetail();
}

QString BackpackDialog::itemIcon(const Item &item) const {
    switch (item.getType()) {
    case ItemType::Food:
        return "食";
    case ItemType::Medicine:
        return "药";
    case ItemType::Equipment:
        return "装";
    default:
        return "物";
    }
}

QString BackpackDialog::cellStyle(bool selected, bool filled) const {
    QString borderColor = selected ? "#8d72d9" : "#d7c8f2";
    QString backgroundColor = filled ? "#ffffff" : "#f1ecfb";
    QString textColor = filled ? "#5a4a7a" : "#b0a4c6";

    return QString(
               "QPushButton {"
               "   font-size: 14px;"
               "   font-weight: 600;"
               "   color: %1;"
               "   background-color: %2;"
               "   border: 2px solid %3;"
               "   border-radius: 12px;"
               "   padding: 6px;"
               "}"
               "QPushButton:hover {"
               "   background-color: #f8f3ff;"
               "   border-color: #bda7ea;"
               "}"
               ).arg(textColor, backgroundColor, borderColor);
}
