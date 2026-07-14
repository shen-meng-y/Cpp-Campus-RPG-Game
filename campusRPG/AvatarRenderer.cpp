#include "AvatarRenderer.h"

#include <QPainter>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStringList>
#include <QDebug>

static QString avatarAssetDir() {
    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList candidates = {
        QDir(appDir).filePath("images/avatar"),
        QDir(appDir).filePath("images"),
        QDir(appDir).filePath("avatar"),
        QDir(appDir).filePath("assets/avatar"),
        QDir(appDir).filePath("assets"),
        appDir
    };

    for (const QString& dir : candidates) {
        if (QFileInfo::exists(QDir(dir).filePath("person1.png"))) {
            return QDir(dir).absolutePath();
        }
    }

#ifdef AVATAR_ASSET_DIR
    const QString configuredDir = QString::fromUtf8(AVATAR_ASSET_DIR);
    if (QFileInfo::exists(QDir(configuredDir).filePath("person1.png"))) {
        return QDir(configuredDir).absolutePath();
    }
#endif

    return QDir(appDir).filePath("images");
}

static QString avatarFilePath(const QString& fileName) {
    return QDir(avatarAssetDir()).filePath(fileName);
}

QPixmap AvatarRenderer::render(
    int personStyle,
    int dressStyle,
    int hairStyle,
    int shoeStyle,
    int width,
    int height
    ) {
    QPixmap result(width, height);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    auto drawLayer = [&](const QString& fileName) {
        QString path = avatarFilePath(fileName);
        QPixmap layer(path);

        if (layer.isNull()) {
            qDebug() << "图片加载失败:" << path;
            return;
        }

        painter.drawPixmap(0, 0, width, height, layer);
    };

    drawLayer(QString("person%1.png").arg(personStyle));
    drawLayer(QString("shoe%1.png").arg(shoeStyle));
    drawLayer(QString("dress%1.png").arg(dressStyle));
    drawLayer(QString("hair%1.png").arg(hairStyle));

    painter.end();

    return result;
}