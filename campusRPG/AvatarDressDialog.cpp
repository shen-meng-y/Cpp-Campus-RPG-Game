#include "AvatarDressDialog.h"
#include "AvatarRenderer.h"

#include <QListWidgetItem>
#include <QListView>
#include <QIcon>
#include <QSize>
#include <QPixmap>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>

static QString avatarAssetDir() {
    const QString appDir = QCoreApplication::applicationDirPath();

    // 优先从 exe 附近查找素材，换电脑后仍然有效。
    const QStringList candidates = {
        QDir(appDir).filePath("images/avatar"),
        QDir(appDir).filePath("images"),
        QDir(appDir).filePath("avatar"),
        QDir(appDir).filePath("assets/avatar"),
        QDir(appDir).filePath("assets"),
        appDir
    };

    for (const QString& dir : candidates) {
        if (QFileInfo::exists(QDir(dir).filePath("bg_room.png"))) {
            return QDir(dir).absolutePath();
        }
    }

#ifdef AVATAR_ASSET_DIR
    // 仅作为本机开发环境的兼容后备，不再依赖它进行部署。
    const QString configuredDir = QString::fromUtf8(AVATAR_ASSET_DIR);
    if (QFileInfo::exists(QDir(configuredDir).filePath("bg_room.png"))) {
        return QDir(configuredDir).absolutePath();
    }
#endif

    // 默认约定：素材放在 exe 同级的 images 文件夹中。
    return QDir(appDir).filePath("images");
}

static QString avatarFilePath(const QString& fileName) {
    return QDir(avatarAssetDir()).filePath(fileName);
}

AvatarDressDialog::AvatarDressDialog(QWidget* parent)
    : QDialog(parent),
    bgWidget(nullptr),
    backgroundLabel(nullptr),
    previewLabel(nullptr),
    btnPersonTab(nullptr),
    btnHairTab(nullptr),
    btnDressTab(nullptr),
    btnShoeTab(nullptr),
    btnOk(nullptr),
    btnCancel(nullptr),
    thumbList(nullptr),
    currentCategory(DressCategory::Person),
    personStyle(1),
    dressStyle(1),
    hairStyle(1),
    shoeStyle(1) {
    setupUi();

    qDebug() << "素材目录:" << avatarAssetDir();
    qDebug() << "bg_room exists:" << QFileInfo::exists(avatarFilePath("bg_room.png"));
    qDebug() << "person1 exists:" << QFileInfo::exists(avatarFilePath("person1.png"));
    qDebug() << "dress1 exists:" << QFileInfo::exists(avatarFilePath("dress1.png"));
    qDebug() << "dress1_thumb exists:" << QFileInfo::exists(avatarFilePath("dress1_thumb.png"));
    qDebug() << "hair1 exists:" << QFileInfo::exists(avatarFilePath("hair1.png"));
    qDebug() << "hair1_thumb exists:" << QFileInfo::exists(avatarFilePath("hair1_thumb.png"));
    qDebug() << "shoe1 exists:" << QFileInfo::exists(avatarFilePath("shoe1.png"));
    qDebug() << "shoe1_thumb exists:" << QFileInfo::exists(avatarFilePath("shoe1_thumb.png"));

    setupConnections();

    loadCategory(DressCategory::Person);
    updatePreview();
}

void AvatarDressDialog::setupUi() {
    setWindowTitle("角色换装");
    setFixedSize(1200, 760);

    bgWidget = new QWidget(this);
    bgWidget->setGeometry(0, 0, 1200, 760);

    backgroundLabel = new QLabel(bgWidget);
    backgroundLabel->setGeometry(0, 0, 1200, 760);
    backgroundLabel->setScaledContents(true);

    QString bgPath = avatarFilePath("bg_room.png");
    QPixmap bgPixmap(bgPath);

    if (bgPixmap.isNull()) {
        qDebug() << "背景图片加载失败:" << bgPath;
        backgroundLabel->setStyleSheet("background-color: #eef5ff;");
    } else {
        backgroundLabel->setPixmap(
            bgPixmap.scaled(
                1200,
                760,
                Qt::IgnoreAspectRatio,
                Qt::SmoothTransformation
                )
            );
    }

    backgroundLabel->lower();

    // 左侧地毯上的角色预览区域
    previewLabel = new QLabel(bgWidget);
    previewLabel->setGeometry(240, 255, 300, 400);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setScaledContents(false);
    previewLabel->setStyleSheet("background: transparent;");

    // 顶部分类按钮
    btnPersonTab = new QPushButton("人物", bgWidget);
    btnPersonTab->setGeometry(585, 35, 100, 70);

    btnHairTab = new QPushButton("发饰", bgWidget);
    btnHairTab->setGeometry(695, 35, 100, 70);

    btnDressTab = new QPushButton("裙子", bgWidget);
    btnDressTab->setGeometry(805, 35, 100, 70);

    btnShoeTab = new QPushButton("鞋子", bgWidget);
    btnShoeTab->setGeometry(915, 35, 100, 70);

    thumbList = new QListWidget(bgWidget);

    // 右侧可滚动选择区域
    thumbList->setGeometry(660, 150, 340, 430);

    // 图标模式，但改成纵向单列排列
    thumbList->setViewMode(QListView::IconMode);
    thumbList->setFlow(QListView::TopToBottom);
    thumbList->setWrapping(false);

    // 图片放大
    thumbList->setIconSize(QSize(150, 150));

    // 每个格子的大小：宽度接近列表宽度，高度大一点
    thumbList->setGridSize(QSize(300, 185));

    thumbList->setResizeMode(QListView::Adjust);
    thumbList->setMovement(QListView::Static);
    thumbList->setSpacing(12);
    thumbList->setUniformItemSizes(true);

    // 开启上下滚动，关闭左右滚动
    thumbList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    thumbList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    thumbList->setStyleSheet(
        "QListWidget {"
        "   background: rgba(255,255,255,0);"
        "   border: none;"
        "}"

        "QListWidget::item {"
        "   background: transparent;"
        "   padding: 4px;"
        "}"

        "QListWidget::item:selected {"
        "   background: rgba(120, 170, 230, 90);"
        "   border-radius: 12px;"
        "}"

        "QScrollBar:vertical {"
        "   background: rgba(220,235,255,120);"
        "   width: 14px;"
        "   margin: 4px 0 4px 0;"
        "   border-radius: 7px;"
        "}"

        "QScrollBar::handle:vertical {"
        "   background: rgba(110,160,220,180);"
        "   min-height: 30px;"
        "   border-radius: 7px;"
        "}"

        "QScrollBar::handle:vertical:hover {"
        "   background: rgba(80,140,210,220);"
        "}"

        "QScrollBar::add-line:vertical,"
        "QScrollBar::sub-line:vertical {"
        "   height: 0px;"
        "}"

        "QScrollBar::add-page:vertical,"
        "QScrollBar::sub-page:vertical {"
        "   background: transparent;"
        "}"
        );

    // 底部确定 / 取消
    btnOk = new QPushButton("确定", bgWidget);
    btnOk->setGeometry(620, 650, 120, 45);

    btnCancel = new QPushButton("取消", bgWidget);
    btnCancel->setGeometry(860, 650, 120, 45);

    QString tabStyle =
        "QPushButton {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #2f70bd;"
        "   background-color: rgba(255,255,255,210);"
        "   border: 3px solid #9fc0e8;"
        "   border-radius: 18px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(230, 242, 255, 240);"
        "}";

    btnPersonTab->setStyleSheet(tabStyle);
    btnHairTab->setStyleSheet(tabStyle);
    btnDressTab->setStyleSheet(tabStyle);
    btnShoeTab->setStyleSheet(tabStyle);

    QString bottomButtonStyle =
        "QPushButton {"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   color: #2f70bd;"
        "   background-color: rgba(255,255,255,220);"
        "   border: 3px solid #8fb7e8;"
        "   border-radius: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(230, 242, 255, 240);"
        "}";

    btnOk->setStyleSheet(bottomButtonStyle);
    btnCancel->setStyleSheet(bottomButtonStyle);
}

void AvatarDressDialog::setupConnections() {
    connect(btnPersonTab, &QPushButton::clicked, this, [this]() {
        loadCategory(DressCategory::Person);
    });

    connect(btnHairTab, &QPushButton::clicked, this, [this]() {
        loadCategory(DressCategory::Hair);
    });

    connect(btnDressTab, &QPushButton::clicked, this, [this]() {
        loadCategory(DressCategory::Dress);
    });

    connect(btnShoeTab, &QPushButton::clicked, this, [this]() {
        loadCategory(DressCategory::Shoe);
    });

    connect(thumbList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        int index = item->data(Qt::UserRole).toInt();

        if (index <= 0) {
            return;
        }

        if (currentCategory == DressCategory::Person) {
            personStyle = index;
        } else if (currentCategory == DressCategory::Hair) {
            hairStyle = index;
        } else if (currentCategory == DressCategory::Dress) {
            dressStyle = index;
        } else if (currentCategory == DressCategory::Shoe) {
            shoeStyle = index;
        }

        updatePreview();
    });

    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void AvatarDressDialog::loadCategory(DressCategory category) {
    currentCategory = category;
    thumbList->clear();

    int count = getCount(category);

    for (int i = 1; i <= count; ++i) {
        QString thumbPath = getThumbPath(category, i);
        QString imagePath = getImagePath(category, i);

        QListWidgetItem* item = new QListWidgetItem;
        item->setData(Qt::UserRole, i);
        item->setSizeHint(QSize(300, 185));
        QPixmap thumbPixmap(thumbPath);

        if (!thumbPixmap.isNull()) {
            item->setIcon(QIcon(thumbPixmap));
        } else {
            QPixmap imagePixmap(imagePath);

            if (!imagePixmap.isNull()) {
                item->setIcon(QIcon(imagePixmap));
            } else {
                qDebug() << "缩略图和原图都加载失败:" << thumbPath << imagePath;
                item->setText(QString::number(i));
            }
        }

        thumbList->addItem(item);
    }

    if (thumbList->count() > 0) {
        thumbList->setCurrentRow(0);
    }
}

void AvatarDressDialog::updatePreview() {
    QPixmap avatar = AvatarRenderer::render(
        personStyle,
        dressStyle,
        hairStyle,
        shoeStyle,
        previewLabel->width(),
        previewLabel->height()
        );

    if (avatar.isNull()) {
        qDebug() << "角色预览图生成失败";
        return;
    }

    previewLabel->setPixmap(avatar);
}

QString AvatarDressDialog::getImagePath(DressCategory category, int index) const {
    if (category == DressCategory::Person) {
        return avatarFilePath(QString("person%1.png").arg(index));
    }

    if (category == DressCategory::Hair) {
        return avatarFilePath(QString("hair%1.png").arg(index));
    }

    if (category == DressCategory::Dress) {
        return avatarFilePath(QString("dress%1.png").arg(index));
    }

    return avatarFilePath(QString("shoe%1.png").arg(index));
}

QString AvatarDressDialog::getThumbPath(DressCategory category, int index) const {
    if (category == DressCategory::Person) {
        return avatarFilePath(QString("person%1.png").arg(index));
    }

    if (category == DressCategory::Hair) {
        return avatarFilePath(QString("hair%1_thumb.png").arg(index));
    }

    if (category == DressCategory::Dress) {
        return avatarFilePath(QString("dress%1_thumb.png").arg(index));
    }

    return avatarFilePath(QString("shoe%1_thumb.png").arg(index));
}

int AvatarDressDialog::getCount(DressCategory category) const {
    if (category == DressCategory::Person) {
        return 3;
    }

    if (category == DressCategory::Hair) {
        return 9;
    }

    if (category == DressCategory::Dress) {
        return 10;
    }

    return 4;
}

int AvatarDressDialog::getPersonStyle() const {
    return personStyle;
}

int AvatarDressDialog::getDressStyle() const {
    return dressStyle;
}

int AvatarDressDialog::getHairStyle() const {
    return hairStyle;
}

int AvatarDressDialog::getShoeStyle() const {
    return shoeStyle;
}