#ifndef AVATARDRESSDIALOG_H
#define AVATARDRESSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

enum class DressCategory {
    Person,
    Hair,
    Dress,
    Shoe
};

class AvatarDressDialog : public QDialog {
    Q_OBJECT

public:
    explicit AvatarDressDialog(QWidget* parent = nullptr);

    int getPersonStyle() const;
    int getDressStyle() const;
    int getHairStyle() const;
    int getShoeStyle() const;

private:
    QWidget* bgWidget;
    QLabel* backgroundLabel;
    QLabel* previewLabel;

    QPushButton* btnPersonTab;
    QPushButton* btnHairTab;
    QPushButton* btnDressTab;
    QPushButton* btnShoeTab;
    QPushButton* btnOk;
    QPushButton* btnCancel;

    QListWidget* thumbList;

    DressCategory currentCategory;

    int personStyle;
    int dressStyle;
    int hairStyle;
    int shoeStyle;

private:
    void setupUi();
    void setupConnections();
    void loadCategory(DressCategory category);
    void updatePreview();

    QString getImagePath(DressCategory category, int index) const;
    QString getThumbPath(DressCategory category, int index) const;
    int getCount(DressCategory category) const;
};

#endif