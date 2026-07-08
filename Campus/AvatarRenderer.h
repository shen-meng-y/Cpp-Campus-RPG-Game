#ifndef AVATARRENDERER_H
#define AVATARRENDERER_H

#include <QPixmap>

class AvatarRenderer {
public:
    static QPixmap render(
        int personStyle,
        int dressStyle,
        int hairStyle,
        int shoeStyle,
        int width = 260,
        int height = 360
        );
};

#endif