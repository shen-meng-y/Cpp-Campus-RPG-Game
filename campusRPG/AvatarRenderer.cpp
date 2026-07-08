#include "AvatarRenderer.h"

#include <QPainter>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

static QString avatarFilePath(const QString& fileName) {
    QString dir = QString::fromUtf8(AVATAR_ASSET_DIR);
    return QDir(dir).filePath(fileName);
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