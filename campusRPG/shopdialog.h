#ifndef SHOPDIALOG_H
#define SHOPDIALOG_H

#include <QDialog>
#include <QVector>
#include <vector>

#include "Shop.h"

class QLabel;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QWidget;

class ShopDialog : public QDialog {
    Q_OBJECT

public:
    explicit ShopDialog(const Shop &shop,
                        Character &player,
                        std::vector<Item> &backpack,
                        QWidget *parent = nullptr);

signals:
    void shopChanged();
    void logRequested(const QString &message);
    void goldChangedBy(int amount);

private:
    static const int BackpackCapacity = 20;

    const Shop &shop;
    Character &player;
    std::vector<Item> &backpack;

    QVector<int> cartQuantities;
    QVector<QSpinBox *> cardQuantitySpins;

    QLabel *goldLabel;
    QLabel *backpackLabel;
    QLabel *cartCountLabel;
    QLabel *totalPriceLabel;
    QLabel *emptyCartLabel;
    QTableWidget *cartTable;
    QPushButton *checkoutButton;
    QPushButton *clearCartButton;

    void buildUi();
    QWidget *createProductCard(int goodsIndex);

    void addToCart(int goodsIndex, int quantity);
    void refreshCartTable();
    void updateCartTotals();
    void updatePlayerSummary();

    int cartItemCount() const;
    int cartTotalPrice() const;

    void clearCart();
    void checkout();
    void sellBackpackItem();

    QString itemIcon(const Item &item) const;
    QString typeColor(const Item &item) const;
};

#endif