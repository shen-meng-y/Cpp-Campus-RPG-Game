#include "shopdialog.h"

#include <QAbstractItemView>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <algorithm>

// 为类内静态常量提供存储定义，避免 MinGW 链接时报 undefined reference。
const int ShopDialog::BackpackCapacity;

ShopDialog::ShopDialog(const Shop &shop,
                       Character &player,
                       std::vector<Item> &backpack,
                       QWidget *parent)
    : QDialog(parent),
    shop(shop),
    player(player),
    backpack(backpack),
    cartQuantities(static_cast<int>(shop.getGoods().size()), 0),
    goldLabel(nullptr),
    backpackLabel(nullptr),
    cartCountLabel(nullptr),
    totalPriceLabel(nullptr),
    emptyCartLabel(nullptr),
    cartTable(nullptr),
    checkoutButton(nullptr),
    clearCartButton(nullptr) {
    setWindowTitle("校园便利店");
    resize(1180, 720);
    setMinimumSize(980, 620);

    buildUi();
    updatePlayerSummary();
    refreshCartTable();
}

void ShopDialog::buildUi() {
    setStyleSheet(
        "QDialog {"
        "  background-color: #f7f3ff;"
        "  font-family: 'Microsoft YaHei';"
        "}"
        "QLabel { color: #55476f; }"
        "QPushButton {"
        "  min-height: 34px;"
        "  padding: 4px 14px;"
        "  color: #5d4b78;"
        "  background-color: #ffffff;"
        "  border: 2px solid #d8cdf0;"
        "  border-radius: 10px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background-color: #eee7ff;"
        "  border-color: #b9a4e3;"
        "}"
        "QPushButton:pressed { background-color: #e3d8fa; }"
        "QPushButton:disabled {"
        "  color: #aaa2b7;"
        "  background-color: #eeeaf4;"
        "  border-color: #ddd7e7;"
        "}"
        "QSpinBox {"
        "  min-height: 32px;"
        "  padding: 1px 28px 1px 8px;"
        "  background-color: white;"
        "  border: 2px solid #ddd2f2;"
        "  border-radius: 9px;"
        "  color: #55476f;"
        "  font-size: 14px;"
        "}"
        "QSpinBox::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 22px;"
        "  height: 16px;"
        "  border-left: 1px solid #d8cdf0;"
        "  border-bottom: 1px solid #e6def3;"
        "}"
        "QSpinBox::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 22px;"
        "  height: 16px;"
        "  border-left: 1px solid #d8cdf0;"
        "}"
        "QTableWidget {"
        "  background-color: #ffffff;"
        "  alternate-background-color: #faf8ff;"
        "  border: 1px solid #ded4f1;"
        "  border-radius: 10px;"
        "  gridline-color: #eee8f7;"
        "  color: #55476f;"
        "  font-size: 13px;"
        "}"
        "QHeaderView::section {"
        "  background-color: #eee7fb;"
        "  color: #66527f;"
        "  border: none;"
        "  border-bottom: 1px solid #d8cdf0;"
        "  padding: 8px;"
        "  font-weight: bold;"
        "}"
        );

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(22, 18, 22, 20);
    rootLayout->setSpacing(16);

    QFrame *header = new QFrame(this);
    header->setObjectName("shopHeader");
    header->setStyleSheet(
        "#shopHeader {"
        "  background-color: rgba(255, 255, 255, 235);"
        "  border: 2px solid #d8cdf0;"
        "  border-radius: 16px;"
        "}"
        );

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 14, 20, 14);

    QVBoxLayout *titleLayout = new QVBoxLayout;
    titleLayout->setSpacing(2);

    QLabel *titleLabel = new QLabel("校园便利店", header);
    titleLabel->setStyleSheet(
        "font-size: 28px; font-weight: 800; color: #684f8b;"
        );

    QLabel *subtitleLabel = new QLabel(
        "挑选商品、设置数量，加入购物车后统一结算。", header);
    subtitleLabel->setStyleSheet("font-size: 14px; color: #8a7a9d;");

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);

    QVBoxLayout *summaryLayout = new QVBoxLayout;
    summaryLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    goldLabel = new QLabel(header);
    goldLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #d38a2d;"
        );

    backpackLabel = new QLabel(header);
    backpackLabel->setStyleSheet("font-size: 14px; color: #76658a;");

    summaryLayout->addWidget(goldLabel, 0, Qt::AlignRight);
    summaryLayout->addWidget(backpackLabel, 0, Qt::AlignRight);

    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    headerLayout->addLayout(summaryLayout);
    rootLayout->addWidget(header);

    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(18);

    QScrollArea *productScrollArea = new QScrollArea(this);
    productScrollArea->setWidgetResizable(true);
    productScrollArea->setFrameShape(QFrame::NoFrame);
    productScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *productContainer = new QWidget(productScrollArea);
    QGridLayout *productGrid = new QGridLayout(productContainer);
    productGrid->setContentsMargins(2, 2, 8, 2);
    productGrid->setHorizontalSpacing(14);
    productGrid->setVerticalSpacing(14);

    const std::vector<Item> &goods = shop.getGoods();
    for (int i = 0; i < static_cast<int>(goods.size()); ++i) {
        productGrid->addWidget(createProductCard(i), i / 2, i % 2);
    }
    productGrid->setRowStretch((static_cast<int>(goods.size()) + 1) / 2, 1);

    productScrollArea->setWidget(productContainer);
    contentLayout->addWidget(productScrollArea, 3);

    QFrame *cartPanel = new QFrame(this);
    cartPanel->setObjectName("cartPanel");
    cartPanel->setMinimumWidth(380);
    cartPanel->setMaximumWidth(430);
    cartPanel->setStyleSheet(
        "#cartPanel {"
        "  background-color: rgba(255, 255, 255, 240);"
        "  border: 2px solid #d8cdf0;"
        "  border-radius: 16px;"
        "}"
        );

    QVBoxLayout *cartLayout = new QVBoxLayout(cartPanel);
    cartLayout->setContentsMargins(16, 16, 16, 16);
    cartLayout->setSpacing(12);

    QHBoxLayout *cartTitleLayout = new QHBoxLayout;
    QLabel *cartTitle = new QLabel("我的购物车", cartPanel);
    cartTitle->setStyleSheet(
        "font-size: 22px; font-weight: 800; color: #684f8b;"
        );

    cartCountLabel = new QLabel("0 件", cartPanel);
    cartCountLabel->setStyleSheet(
        "padding: 4px 9px; background-color: #eee7fb;"
        "border-radius: 9px; color: #7d65a0; font-weight: bold;"
        );

    cartTitleLayout->addWidget(cartTitle);
    cartTitleLayout->addStretch();
    cartTitleLayout->addWidget(cartCountLabel);
    cartLayout->addLayout(cartTitleLayout);

    emptyCartLabel = new QLabel(
        "购物车还是空的，先从左侧选择商品吧。", cartPanel);
    emptyCartLabel->setAlignment(Qt::AlignCenter);
    emptyCartLabel->setWordWrap(true);
    emptyCartLabel->setStyleSheet(
        "padding: 14px; color: #9a8cab; background-color: #faf8ff;"
        "border: 1px dashed #d8cdf0; border-radius: 10px;"
        );
    cartLayout->addWidget(emptyCartLabel);

    cartTable = new QTableWidget(0, 4, cartPanel);
    cartTable->setHorizontalHeaderLabels(
        QStringList() << "商品" << "数量" << "小计" << "操作");
    cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    cartTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    cartTable->setColumnWidth(1, 112);
    cartTable->setColumnWidth(2, 72);
    cartTable->setColumnWidth(3, 72);
    cartTable->verticalHeader()->setVisible(false);
    cartTable->setSelectionMode(QAbstractItemView::NoSelection);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable->setAlternatingRowColors(true);
    cartLayout->addWidget(cartTable, 1);

    totalPriceLabel = new QLabel("合计：0 金币", cartPanel);
    totalPriceLabel->setAlignment(Qt::AlignRight);
    totalPriceLabel->setStyleSheet(
        "font-size: 21px; font-weight: 800; color: #d18428;"
        );
    cartLayout->addWidget(totalPriceLabel);

    QHBoxLayout *smallButtonLayout = new QHBoxLayout;
    clearCartButton = new QPushButton("清空购物车", cartPanel);
    QPushButton *sellButton = new QPushButton("出售背包物品", cartPanel);
    smallButtonLayout->addWidget(clearCartButton);
    smallButtonLayout->addWidget(sellButton);
    cartLayout->addLayout(smallButtonLayout);

    checkoutButton = new QPushButton("确认购买", cartPanel);
    checkoutButton->setObjectName("checkoutButton");
    checkoutButton->setMinimumHeight(46);
    checkoutButton->setStyleSheet(
        "#checkoutButton {"
        "  color: white; background-color: #8d72d9;"
        "  border: 2px solid #8d72d9; border-radius: 12px;"
        "  font-size: 17px; font-weight: 800;"
        "}"
        "#checkoutButton:hover { background-color: #7d61c9; }"
        "#checkoutButton:disabled {"
        "  color: #aaa2b7; background-color: #e5e0ec;"
        "  border-color: #e5e0ec;"
        "}"
        );

    QPushButton *closeButton = new QPushButton("关闭商店", cartPanel);
    cartLayout->addWidget(checkoutButton);
    cartLayout->addWidget(closeButton);

    connect(clearCartButton, &QPushButton::clicked,
            this, &ShopDialog::clearCart);
    connect(sellButton, &QPushButton::clicked,
            this, &ShopDialog::sellBackpackItem);
    connect(checkoutButton, &QPushButton::clicked,
            this, &ShopDialog::checkout);
    connect(closeButton, &QPushButton::clicked,
            this, &ShopDialog::accept);

    contentLayout->addWidget(cartPanel, 2);
    rootLayout->addLayout(contentLayout, 1);
}

QWidget *ShopDialog::createProductCard(int goodsIndex) {
    const Item &item = shop.getGoods()[goodsIndex];
    const QString accentColor = typeColor(item);

    QFrame *card = new QFrame(this);
    card->setMinimumHeight(250);
    card->setObjectName(QString("productCard%1").arg(goodsIndex));
    card->setStyleSheet(
        QString(
            "QFrame#productCard%1 {"
            "  background-color: rgba(255, 255, 255, 245);"
            "  border: 2px solid %2;"
            "  border-radius: 16px;"
            "}"
            ).arg(goodsIndex).arg(accentColor)
        );

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(8);

    QHBoxLayout *topLayout = new QHBoxLayout;

    QLabel *iconLabel = new QLabel(itemIcon(item), card);
    iconLabel->setFixedSize(62, 62);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet(
        QString(
            "font-size: 34px; background-color: %1;"
            "border-radius: 15px; color: white;"
            ).arg(accentColor)
        );

    QVBoxLayout *nameLayout = new QVBoxLayout;
    QLabel *nameLabel = new QLabel(
        QString::fromStdString(item.getName()), card);
    nameLabel->setStyleSheet(
        "font-size: 21px; font-weight: 800; color: #59466f;"
        );

    QLabel *typeLabel = new QLabel(
        QString::fromStdString(Item::typeToString(item.getType())), card);
    typeLabel->setStyleSheet(
        QString(
            "color: %1; font-size: 13px; font-weight: bold;"
            ).arg(accentColor)
        );

    nameLayout->addWidget(nameLabel);
    nameLayout->addWidget(typeLabel);

    topLayout->addWidget(iconLabel);
    topLayout->addLayout(nameLayout);
    topLayout->addStretch();
    layout->addLayout(topLayout);

    QLabel *descriptionLabel = new QLabel(
        QString::fromStdString(item.getDescription()), card);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setStyleSheet(
        "font-size: 14px; color: #7c6d8e; min-height: 38px;"
        );
    layout->addWidget(descriptionLabel);

    QLabel *effectLabel = new QLabel(
        QString("效果值：%1").arg(item.getEffectValue()), card);
    effectLabel->setStyleSheet(
        "font-size: 14px; color: #6f5c84; font-weight: 600;"
        );
    layout->addWidget(effectLabel);

    QLabel *priceLabel = new QLabel(
        QString("%1 金币").arg(item.getPrice()), card);
    priceLabel->setStyleSheet(
        "font-size: 20px; color: #d18428; font-weight: 800;"
        );
    layout->addWidget(priceLabel);

    layout->addStretch();

    QHBoxLayout *actionLayout = new QHBoxLayout;
    QLabel *quantityLabel = new QLabel("数量", card);
    quantityLabel->setStyleSheet("font-weight: 600; color: #6f5c84;");

    QSpinBox *quantitySpin = new QSpinBox(card);
    quantitySpin->setRange(1, 99);
    quantitySpin->setValue(1);
    quantitySpin->setAlignment(Qt::AlignCenter);
    cardQuantitySpins.push_back(quantitySpin);

    QPushButton *addButton = new QPushButton("加入购物车", card);
    addButton->setStyleSheet(
        QString(
            "QPushButton { color: white; background-color: %1;"
            "border-color: %1; }"
            "QPushButton:hover { background-color: #7d61c9; }"
            ).arg(accentColor)
        );

    connect(addButton, &QPushButton::clicked, this,
            [this, goodsIndex, quantitySpin]() {
                addToCart(goodsIndex, quantitySpin->value());
                quantitySpin->setValue(1);
            });

    actionLayout->addWidget(quantityLabel);
    actionLayout->addWidget(quantitySpin);
    actionLayout->addWidget(addButton, 1);
    layout->addLayout(actionLayout);

    return card;
}

void ShopDialog::addToCart(int goodsIndex, int quantity) {
    if (goodsIndex < 0 || goodsIndex >= cartQuantities.size() || quantity <= 0) {
        return;
    }

    cartQuantities[goodsIndex] = std::min(99,
                                          cartQuantities[goodsIndex] + quantity);
    refreshCartTable();

    const Item &item = shop.getGoods()[goodsIndex];
    emit logRequested(
        QString("已将 %1 ×%2 加入购物车。")
            .arg(QString::fromStdString(item.getName()))
            .arg(quantity));
}

void ShopDialog::refreshCartTable() {
    cartTable->setRowCount(0);

    const std::vector<Item> &goods = shop.getGoods();
    int row = 0;

    for (int i = 0; i < cartQuantities.size(); ++i) {
        if (cartQuantities[i] <= 0) {
            continue;
        }

        cartTable->insertRow(row);
        const Item &item = goods[i];

        QTableWidgetItem *nameItem = new QTableWidgetItem(
            QString::fromStdString(item.getName()));
        nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        cartTable->setItem(row, 0, nameItem);

        QSpinBox *quantitySpin = new QSpinBox(cartTable);
        quantitySpin->setRange(1, 99);
        quantitySpin->setValue(cartQuantities[i]);
        quantitySpin->setAlignment(Qt::AlignCenter);
        quantitySpin->setFixedSize(92, 38);
        quantitySpin->setStyleSheet(
            "QSpinBox {"
            "  padding-left: 8px;"
            "  padding-right: 26px;"
            "  font-size: 15px;"
            "  font-weight: 700;"
            "  color: #4f3d6c;"
            "  background-color: #ffffff;"
            "}"
            );

        QWidget *spinContainer = new QWidget(cartTable);
        QHBoxLayout *spinLayout = new QHBoxLayout(spinContainer);
        spinLayout->setContentsMargins(3, 2, 3, 2);
        spinLayout->addWidget(quantitySpin, 0, Qt::AlignCenter);
        cartTable->setCellWidget(row, 1, spinContainer);
        cartTable->setRowHeight(row, 48);

        QTableWidgetItem *subtotalItem = new QTableWidgetItem(
            QString::number(item.getPrice() * cartQuantities[i]));
        subtotalItem->setTextAlignment(Qt::AlignCenter);
        cartTable->setItem(row, 2, subtotalItem);

        QPushButton *removeButton = new QPushButton("移除", cartTable);
        removeButton->setFixedHeight(30);
        removeButton->setStyleSheet(
            "QPushButton { padding: 2px 8px; color: #a85c6c;"
            "border-color: #e7bcc5; background-color: #fff7f8; }"
            "QPushButton:hover { background-color: #ffecef; }"
            );
        cartTable->setCellWidget(row, 3, removeButton);

        connect(quantitySpin,
                QOverload<int>::of(&QSpinBox::valueChanged),
                this,
                [this, i, row](int value) {
                    cartQuantities[i] = value;
                    const Item &changedItem = shop.getGoods()[i];
                    QTableWidgetItem *subtotal = cartTable->item(row, 2);
                    if (subtotal != nullptr) {
                        subtotal->setText(
                            QString::number(changedItem.getPrice() * value));
                    }
                    updateCartTotals();
                });

        connect(removeButton, &QPushButton::clicked, this, [this, i]() {
            cartQuantities[i] = 0;
            refreshCartTable();
        });

        cartTable->setRowHeight(row, 46);
        ++row;
    }

    emptyCartLabel->setVisible(row == 0);
    cartTable->setVisible(row > 0);
    updateCartTotals();
}

void ShopDialog::updateCartTotals() {
    const int count = cartItemCount();
    const int total = cartTotalPrice();

    cartCountLabel->setText(QString("%1 件").arg(count));
    totalPriceLabel->setText(QString("合计：%1 金币").arg(total));

    checkoutButton->setEnabled(count > 0);
    clearCartButton->setEnabled(count > 0);
}

void ShopDialog::updatePlayerSummary() {
    const int usedSlots = static_cast<int>(backpack.size());
    const int visibleUsedSlots = std::min(usedSlots, BackpackCapacity);

    goldLabel->setText(QString("金币：%1").arg(player.getGold()));
    backpackLabel->setText(
        QString("背包：%1 / %2 格")
            .arg(visibleUsedSlots)
            .arg(BackpackCapacity));
}

int ShopDialog::cartItemCount() const {
    int count = 0;
    for (int quantity : cartQuantities) {
        count += quantity;
    }
    return count;
}

int ShopDialog::cartTotalPrice() const {
    int total = 0;
    const std::vector<Item> &goods = shop.getGoods();

    for (int i = 0; i < cartQuantities.size(); ++i) {
        total += cartQuantities[i] * goods[i].getPrice();
    }

    return total;
}

void ShopDialog::clearCart() {
    if (cartItemCount() <= 0) {
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "清空购物车",
        "确定要移除购物车中的全部商品吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    std::fill(cartQuantities.begin(), cartQuantities.end(), 0);
    refreshCartTable();
    emit logRequested("已清空购物车。");
}

void ShopDialog::checkout() {
    const int count = cartItemCount();
    const int total = cartTotalPrice();

    if (count <= 0) {
        QMessageBox::information(this, "购物车", "购物车中还没有商品。");
        return;
    }

    const int freeSlots = std::max(
        0, BackpackCapacity - static_cast<int>(backpack.size()));
    if (count > freeSlots) {
        QMessageBox::warning(
            this,
            "背包空间不足",
            QString("购物车中共有 %1 件商品，但背包只剩 %2 个空格。\n"
                    "请减少购买数量，或先整理背包。")
                .arg(count)
                .arg(freeSlots));
        return;
    }

    if (player.getGold() < total) {
        QMessageBox::warning(
            this,
            "金币不足",
            QString("本次结算需要 %1 金币，你当前有 %2 金币，"
                    "还差 %3 金币。")
                .arg(total)
                .arg(player.getGold())
                .arg(total - player.getGold()));
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "确认结算",
        QString("确定购买 %1 件商品并支付 %2 金币吗？")
            .arg(count)
            .arg(total),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!player.spendGold(total)) {
        QMessageBox::warning(this, "购买失败", "金币扣除失败，请重试。");
        return;
    }

    QStringList purchaseSummary;
    const std::vector<Item> &goods = shop.getGoods();

    for (int i = 0; i < cartQuantities.size(); ++i) {
        const int quantity = cartQuantities[i];
        if (quantity <= 0) {
            continue;
        }

        for (int j = 0; j < quantity; ++j) {
            backpack.push_back(goods[i]);
        }

        purchaseSummary << QString("%1 ×%2")
                               .arg(QString::fromStdString(goods[i].getName()))
                               .arg(quantity);
    }

    std::fill(cartQuantities.begin(), cartQuantities.end(), 0);
    updatePlayerSummary();
    refreshCartTable();

    emit shopChanged();
    emit goldChangedBy(-total);
    emit logRequested(
        QString("购物车结算成功：%1，共支付 %2 金币。")
            .arg(purchaseSummary.join("、"))
            .arg(total));

    QMessageBox::information(
        this,
        "购买成功",
        QString("商品已放入背包。\n共购买 %1 件，支付 %2 金币。")
            .arg(count)
            .arg(total));
}

void ShopDialog::sellBackpackItem() {
    if (backpack.empty()) {
        QMessageBox::information(this, "出售失败", "背包为空，无法出售物品。");
        return;
    }

    QStringList itemList;
    for (int i = 0; i < static_cast<int>(backpack.size()); ++i) {
        const Item &item = backpack[i];
        itemList << QString("%1. %2 | 售价：%3 金币")
                        .arg(i + 1)
                        .arg(QString::fromStdString(item.getName()))
                        .arg(item.getPrice() / 2);
    }

    bool ok = false;
    const QString selected = QInputDialog::getItem(
        this,
        "出售背包物品",
        "请选择要出售的物品：",
        itemList,
        0,
        false,
        &ok);

    if (!ok || selected.isEmpty()) {
        return;
    }

    const int index = itemList.indexOf(selected);
    if (index < 0 || index >= static_cast<int>(backpack.size())) {
        return;
    }

    const Item item = backpack[index];
    const int sellPrice = item.getPrice() / 2;

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        "确认出售",
        QString("确定出售“%1”并获得 %2 金币吗？")
            .arg(QString::fromStdString(item.getName()))
            .arg(sellPrice),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    backpack.erase(backpack.begin() + index);
    player.addGold(sellPrice);
    updatePlayerSummary();

    emit shopChanged();
    emit goldChangedBy(sellPrice);
    emit logRequested(
        QString("出售物品：%1，获得 %2 金币。")
            .arg(QString::fromStdString(item.getName()))
            .arg(sellPrice));

    QMessageBox::information(
        this,
        "出售成功",
        QString("已获得 %1 金币。").arg(sellPrice));
}

QString ShopDialog::itemIcon(const Item &item) const {
    const QString name = QString::fromStdString(item.getName());

    if (name == "面包") {
        return "🍞";
    }
    if (name == "牛奶") {
        return "🥛";
    }
    if (name == "急救药水") {
        return "💊";
    }
    if (name == "木剑") {
        return "⚔";
    }
    if (name == "学习宝典") {
        return "📘";
    }

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

QString ShopDialog::typeColor(const Item &item) const {
    switch (item.getType()) {
    case ItemType::Food:
        return "#e6a15d";
    case ItemType::Medicine:
        return "#63b98e";
    case ItemType::Equipment:
        return "#9276d3";
    default:
        return "#8a7a9d";
    }
}
