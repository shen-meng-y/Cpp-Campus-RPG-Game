#include "statisticsdialog.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStringList>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace {

QString formatDuration(qint64 totalSeconds)
{
    totalSeconds = std::max<qint64>(0, totalSeconds);
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1小时%2分钟").arg(hours).arg(minutes);
    }
    if (minutes > 0) {
        return QString("%1分钟%2秒").arg(minutes).arg(seconds);
    }
    return QString("%1秒").arg(seconds);
}

QString compactTime(qint64 seconds)
{
    seconds = std::max<qint64>(0, seconds);
    if (seconds >= 3600) {
        return QString::number(seconds / 3600.0, 'f', 1) + "h";
    }
    if (seconds >= 60) {
        return QString::number(seconds / 60.0, 'f', 0) + "m";
    }
    return QString::number(seconds) + "s";
}

QFrame *createMetricCard(const QString &title,
                         const QString &value,
                         const QString &note = QString())
{
    QFrame *card = new QFrame;
    card->setObjectName("metricCard");
    card->setMinimumHeight(112);

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(5);

    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("metricTitle");

    QLabel *valueLabel = new QLabel(value, card);
    valueLabel->setObjectName("metricValue");
    valueLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(valueLabel);

    if (!note.isEmpty()) {
        QLabel *noteLabel = new QLabel(note, card);
        noteLabel->setObjectName("metricNote");
        noteLabel->setWordWrap(true);
        layout->addWidget(noteLabel);
    }

    layout->addStretch();
    return card;
}

class GrowthChartWidget : public QWidget
{
public:
    enum Metric {
        Level,
        Experience,
        MaxHp,
        Attack
    };

    explicit GrowthChartWidget(const QVector<GrowthRecord> &records,
                               QWidget *parent = nullptr)
        : QWidget(parent),
        records(records)
    {
        setMinimumHeight(440);
    }

    void setMetric(Metric value)
    {
        metric = value;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor("#ffffff"));

        const QRectF plotRect(72.0,
                              38.0,
                              std::max(10.0, width() - 104.0),
                              std::max(10.0, height() - 105.0));

        painter.setPen(QPen(QColor("#d8e5f2"), 1));
        painter.drawRoundedRect(rect().adjusted(1, 1, -2, -2), 14, 14);

        if (records.isEmpty()) {
            painter.setPen(QColor("#718096"));
            painter.drawText(rect(), Qt::AlignCenter,
                             "暂无成长记录。完成训练、战斗或进化后会生成曲线数据。");
            return;
        }

        QVector<double> values;
        values.reserve(records.size());
        for (const GrowthRecord &record : records) {
            switch (metric) {
            case Level:
                values.append(record.level);
                break;
            case Experience:
                values.append(50.0 * record.level * (record.level - 1)
                              + record.experience);
                break;
            case MaxHp:
                values.append(record.maxHp);
                break;
            case Attack:
                values.append(record.attack);
                break;
            }
        }

        double maxValue = 1.0;
        for (double value : values) {
            maxValue = std::max(maxValue, value);
        }
        maxValue *= 1.12;

        const qint64 minSeconds = records.first().playSeconds;
        qint64 maxSeconds = records.last().playSeconds;
        if (maxSeconds <= minSeconds) {
            maxSeconds = minSeconds + std::max<qint64>(
                             1, static_cast<qint64>(records.size()) - 1);
        }

        painter.setFont(QFont("Microsoft YaHei", 9));
        for (int i = 0; i <= 5; ++i) {
            const double ratio = i / 5.0;
            const double y = plotRect.bottom() - ratio * plotRect.height();
            painter.setPen(QPen(QColor("#e8eef5"), 1));
            painter.drawLine(QPointF(plotRect.left(), y),
                             QPointF(plotRect.right(), y));

            painter.setPen(QColor("#6b7b8f"));
            const double labelValue = ratio * maxValue;
            painter.drawText(QRectF(5, y - 10, 58, 20),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(labelValue,
                                             metric == Level ? 'f' : 'f',
                                             metric == Level ? 0 : 0));
        }

        painter.setPen(QPen(QColor("#91a4b7"), 1.4));
        painter.drawLine(plotRect.bottomLeft(), plotRect.bottomRight());
        painter.drawLine(plotRect.bottomLeft(), plotRect.topLeft());

        QPainterPath path;
        QVector<QPointF> points;
        points.reserve(records.size());

        for (int i = 0; i < records.size(); ++i) {
            double xRatio = 0.0;
            if (records.size() == 1) {
                xRatio = 0.5;
            } else if (records.last().playSeconds > records.first().playSeconds) {
                xRatio = static_cast<double>(records[i].playSeconds - minSeconds)
                / static_cast<double>(maxSeconds - minSeconds);
            } else {
                xRatio = static_cast<double>(i)
                / static_cast<double>(records.size() - 1);
            }

            const double yRatio = values[i] / maxValue;
            const QPointF point(plotRect.left() + xRatio * plotRect.width(),
                                plotRect.bottom() - yRatio * plotRect.height());
            points.append(point);

            if (i == 0) {
                path.moveTo(point);
            } else {
                path.lineTo(point);
            }
        }

        painter.setPen(QPen(QColor("#5b8fd6"), 3,
                            Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(path);

        painter.setPen(QPen(QColor("#ffffff"), 2));
        painter.setBrush(QColor("#5b8fd6"));
        for (const QPointF &point : points) {
            painter.drawEllipse(point, 5, 5);
        }

        painter.setPen(QColor("#526779"));
        const QString startText = compactTime(records.first().playSeconds);
        const QString middleText = compactTime(
            records[records.size() / 2].playSeconds);
        const QString endText = compactTime(records.last().playSeconds);

        painter.drawText(QRectF(plotRect.left() - 25,
                                plotRect.bottom() + 12,
                                70,
                                22),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         startText);
        painter.drawText(QRectF(plotRect.center().x() - 35,
                                plotRect.bottom() + 12,
                                70,
                                22),
                         Qt::AlignCenter,
                         middleText);
        painter.drawText(QRectF(plotRect.right() - 45,
                                plotRect.bottom() + 12,
                                70,
                                22),
                         Qt::AlignRight | Qt::AlignVCenter,
                         endText);

        painter.setFont(QFont("Microsoft YaHei", 10, QFont::DemiBold));
        painter.setPen(QColor("#334e68"));
        painter.drawText(QRectF(plotRect.left(), 8, plotRect.width(), 24),
                         Qt::AlignCenter,
                         metricTitle());
        painter.drawText(QRectF(plotRect.left(),
                                height() - 34,
                                plotRect.width(),
                                22),
                         Qt::AlignCenter,
                         "累计游戏时间");
    }

private:
    QString metricTitle() const
    {
        switch (metric) {
        case Level:
            return "等级成长曲线";
        case Experience:
            return "累计经验成长曲线";
        case MaxHp:
            return "最大生命值成长曲线";
        case Attack:
            return "攻击力成长曲线";
        }
        return "成长曲线";
    }

    QVector<GrowthRecord> records;
    Metric metric = Level;
};

class EnemyBarChartWidget : public QWidget
{
public:
    explicit EnemyBarChartWidget(const QMap<QString, int> &data,
                                 QWidget *parent = nullptr)
        : QWidget(parent),
        data(data)
    {
        setMinimumHeight(360);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor("#ffffff"));
        painter.setPen(QPen(QColor("#d8e5f2"), 1));
        painter.drawRoundedRect(rect().adjusted(1, 1, -2, -2), 14, 14);

        painter.setFont(QFont("Microsoft YaHei", 11, QFont::DemiBold));
        painter.setPen(QColor("#334e68"));
        painter.drawText(QRectF(20, 16, width() - 40, 28),
                         Qt::AlignCenter,
                         "怪物击杀数量");

        if (data.isEmpty()) {
            painter.setFont(QFont("Microsoft YaHei", 10));
            painter.setPen(QColor("#718096"));
            painter.drawText(rect().adjusted(20, 50, -20, -20),
                             Qt::AlignCenter,
                             "暂无击杀数据");
            return;
        }

        int maxValue = 1;
        for (int count : data) {
            maxValue = std::max(maxValue, count);
        }

        const int left = 132;
        const int right = 55;
        const int top = 62;
        const int bottom = 28;
        const int count = data.size();
        const double rowHeight =
            static_cast<double>(height() - top - bottom) / count;

        int index = 0;
        painter.setFont(QFont("Microsoft YaHei", 9));
        for (auto it = data.constBegin(); it != data.constEnd(); ++it, ++index) {
            const double centerY = top + rowHeight * (index + 0.5);
            const QRectF track(left,
                               centerY - 10,
                               std::max(20, width() - left - right),
                               20);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor("#eef4fa"));
            painter.drawRoundedRect(track, 8, 8);

            QRectF bar = track;
            bar.setWidth(track.width()
                         * static_cast<double>(it.value())
                         / static_cast<double>(maxValue));
            painter.setBrush(QColor("#6aa4df"));
            painter.drawRoundedRect(bar, 8, 8);

            painter.setPen(QColor("#425466"));
            painter.drawText(QRectF(10, centerY - 16, left - 20, 32),
                             Qt::AlignRight | Qt::AlignVCenter,
                             it.key());
            painter.drawText(QRectF(track.right() + 8,
                                    centerY - 16,
                                    right - 8,
                                    32),
                             Qt::AlignLeft | Qt::AlignVCenter,
                             QString::number(it.value()));
        }
    }

private:
    QMap<QString, int> data;
};

class BattlePieChartWidget : public QWidget
{
public:
    BattlePieChartWidget(int wins,
                         int defeats,
                         int escapes,
                         QWidget *parent = nullptr)
        : QWidget(parent),
        wins(std::max(0, wins)),
        defeats(std::max(0, defeats)),
        escapes(std::max(0, escapes))
    {
        setMinimumHeight(360);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor("#ffffff"));
        painter.setPen(QPen(QColor("#d8e5f2"), 1));
        painter.drawRoundedRect(rect().adjusted(1, 1, -2, -2), 14, 14);

        painter.setFont(QFont("Microsoft YaHei", 11, QFont::DemiBold));
        painter.setPen(QColor("#334e68"));
        painter.drawText(QRectF(20, 16, width() - 40, 28),
                         Qt::AlignCenter,
                         "战斗结果构成");

        const int total = wins + defeats + escapes;
        if (total <= 0) {
            painter.setFont(QFont("Microsoft YaHei", 10));
            painter.setPen(QColor("#718096"));
            painter.drawText(rect().adjusted(20, 50, -20, -20),
                             Qt::AlignCenter,
                             "暂无战斗数据");
            return;
        }

        const int diameter = std::min(width() - 190, height() - 100);
        const QRectF pieRect(38,
                             70,
                             std::max(120, diameter),
                             std::max(120, diameter));

        const QVector<int> values{wins, defeats, escapes};
        const QVector<QColor> colors{
            QColor("#69b98a"),
            QColor("#e88989"),
            QColor("#e8b96a")
        };
        const QStringList names{"胜利", "失败", "逃跑"};

        int startAngle = 90 * 16;
        for (int i = 0; i < values.size(); ++i) {
            const int spanAngle = qRound(-360.0 * 16.0
                                         * values[i]
                                         / static_cast<double>(total));
            painter.setPen(QPen(QColor("#ffffff"), 2));
            painter.setBrush(colors[i]);
            painter.drawPie(pieRect, startAngle, spanAngle);
            startAngle += spanAngle;
        }

        const int legendX = static_cast<int>(pieRect.right()) + 28;
        int legendY = 100;
        painter.setFont(QFont("Microsoft YaHei", 10));
        for (int i = 0; i < values.size(); ++i) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(colors[i]);
            painter.drawRoundedRect(QRectF(legendX, legendY, 16, 16), 4, 4);

            painter.setPen(QColor("#425466"));
            const double percentage = values[i] * 100.0 / total;
            painter.drawText(QRectF(legendX + 24,
                                    legendY - 4,
                                    width() - legendX - 30,
                                    26),
                             Qt::AlignLeft | Qt::AlignVCenter,
                             QString("%1：%2 次（%3%）")
                                 .arg(names[i])
                                 .arg(values[i])
                                 .arg(percentage, 0, 'f', 1));
            legendY += 50;
        }
    }

private:
    int wins;
    int defeats;
    int escapes;
};

QString buildAnalysis(const GameStatistics &statistics,
                      const Character &player)
{
    QStringList lines;
    lines << QString("当前角色等级为 %1，攻击力为 %2，最大生命值为 %3。")
                 .arg(player.getLevel())
                 .arg(player.getAttack())
                 .arg(player.getMaxHp());

    if (statistics.totalBattles > 0) {
        lines << QString("共进行 %1 场战斗，胜率为 %2%。")
                     .arg(statistics.totalBattles)
                     .arg(statistics.winRate(), 0, 'f', 1);

        if (statistics.winRate() >= 80.0) {
            lines << "战斗表现稳定，当前胜率处于较高水平。";
        } else if (statistics.winRate() >= 50.0) {
            lines << "战斗表现基本稳定，可继续通过训练提升攻击力和生命值。";
        } else {
            lines << "当前战斗胜率偏低，建议先训练、进化或补充恢复道具。";
        }
    } else {
        lines << "尚未产生战斗记录，完成校园试炼后可分析胜率。";
    }

    const QString favoriteEnemy = statistics.favoriteEnemy();
    if (!favoriteEnemy.isEmpty()) {
        lines << QString("击败次数最多的怪物是“%1”。").arg(favoriteEnemy);
    }

    const int netGold = statistics.goldEarned - statistics.goldSpent;
    lines << QString("累计获得 %1 金币，累计支出 %2 金币，净变化为 %3 金币。")
                 .arg(statistics.goldEarned)
                 .arg(statistics.goldSpent)
                 .arg(netGold);

    return lines.join("\n");
}

} // namespace

StatisticsDialog::StatisticsDialog(const GameStatistics &statistics,
                                   const Character &player,
                                   QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("游戏数据中心");
    resize(1180, 780);
    setMinimumSize(960, 680);

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 18, 20, 18);
    rootLayout->setSpacing(14);

    QLabel *heading = new QLabel(
        QString("%1 的游戏数据中心")
            .arg(QString::fromStdString(player.getName())),
        this);
    heading->setObjectName("heading");

    QLabel *subheading = new QLabel(
        "统计数据会随战斗、训练、任务、商店和游戏时间自动更新。",
        this);
    subheading->setObjectName("subheading");

    rootLayout->addWidget(heading);
    rootLayout->addWidget(subheading);

    QTabWidget *tabs = new QTabWidget(this);
    tabs->setDocumentMode(true);
    rootLayout->addWidget(tabs, 1);

    QWidget *overviewPage = new QWidget(tabs);
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewPage);
    overviewLayout->setContentsMargins(16, 18, 16, 16);
    overviewLayout->setSpacing(16);

    QGridLayout *metricGrid = new QGridLayout;
    metricGrid->setHorizontalSpacing(14);
    metricGrid->setVerticalSpacing(14);

    metricGrid->addWidget(createMetricCard(
                              "累计游玩时长",
                              formatDuration(statistics.totalPlaySeconds),
                              "按角色独立累计"),
                          0, 0);
    metricGrid->addWidget(createMetricCard(
                              "当前等级",
                              QString::number(player.getLevel()),
                              QString::fromStdString(
                                  player.getEvolutionStageName())),
                          0, 1);
    metricGrid->addWidget(createMetricCard(
                              "战斗场次",
                              QString::number(statistics.totalBattles),
                              QString("胜率 %1%")
                                  .arg(statistics.winRate(), 0, 'f', 1)),
                          0, 2);
    metricGrid->addWidget(createMetricCard(
                              "击败怪物",
                              QString::number(statistics.totalKills),
                              "累计击杀数量"),
                          0, 3);
    metricGrid->addWidget(createMetricCard(
                              "完成任务",
                              QString::number(statistics.questsCompleted),
                              "任务状态达到已完成"),
                          1, 0);
    metricGrid->addWidget(createMetricCard(
                              "累计获得金币",
                              QString::number(statistics.goldEarned),
                              "战斗、任务、出售及事件"),
                          1, 1);
    metricGrid->addWidget(createMetricCard(
                              "累计支出金币",
                              QString::number(statistics.goldSpent),
                              "商店、治疗及事件"),
                          1, 2);
    metricGrid->addWidget(createMetricCard(
                              "净金币变化",
                              QString::number(statistics.goldEarned
                                              - statistics.goldSpent),
                              "累计收入减累计支出"),
                          1, 3);

    overviewLayout->addLayout(metricGrid);

    QFrame *analysisFrame = new QFrame(overviewPage);
    analysisFrame->setObjectName("analysisFrame");
    QVBoxLayout *analysisLayout = new QVBoxLayout(analysisFrame);
    analysisLayout->setContentsMargins(20, 17, 20, 17);

    QLabel *analysisTitle = new QLabel("数据统计分析", analysisFrame);
    analysisTitle->setObjectName("sectionTitle");
    QLabel *analysisText = new QLabel(
        buildAnalysis(statistics, player), analysisFrame);
    analysisText->setObjectName("analysisText");
    analysisText->setWordWrap(true);

    analysisLayout->addWidget(analysisTitle);
    analysisLayout->addWidget(analysisText);
    overviewLayout->addWidget(analysisFrame);
    overviewLayout->addStretch();

    tabs->addTab(overviewPage, "数据总览");

    QWidget *growthPage = new QWidget(tabs);
    QVBoxLayout *growthLayout = new QVBoxLayout(growthPage);
    growthLayout->setContentsMargins(16, 18, 16, 16);
    growthLayout->setSpacing(12);

    QHBoxLayout *growthControls = new QHBoxLayout;
    QLabel *metricLabel = new QLabel("成长指标：", growthPage);
    QComboBox *metricCombo = new QComboBox(growthPage);
    metricCombo->addItems({"等级", "累计经验", "最大生命值", "攻击力"});
    metricCombo->setMinimumWidth(180);
    growthControls->addWidget(metricLabel);
    growthControls->addWidget(metricCombo);
    growthControls->addStretch();

    QLabel *growthHint = new QLabel(
        QString("当前共有 %1 个有效成长记录点。")
            .arg(statistics.growthHistory.size()),
        growthPage);
    growthHint->setObjectName("hint");
    growthControls->addWidget(growthHint);

    GrowthChartWidget *growthChart =
        new GrowthChartWidget(statistics.growthHistory, growthPage);

    QObject::connect(metricCombo,
                     qOverload<int>(&QComboBox::currentIndexChanged),
                     growthChart,
                     [growthChart](int index) {
                         growthChart->setMetric(
                             static_cast<GrowthChartWidget::Metric>(index));
                     });

    growthLayout->addLayout(growthControls);
    growthLayout->addWidget(growthChart, 1);
    tabs->addTab(growthPage, "成长曲线");

    QWidget *battlePage = new QWidget(tabs);
    QVBoxLayout *battleLayout = new QVBoxLayout(battlePage);
    battleLayout->setContentsMargins(16, 18, 16, 16);
    battleLayout->setSpacing(14);

    QHBoxLayout *battleCharts = new QHBoxLayout;
    battleCharts->setSpacing(14);
    battleCharts->addWidget(
        new EnemyBarChartWidget(statistics.enemyKillCounts, battlePage), 1);
    battleCharts->addWidget(
        new BattlePieChartWidget(statistics.battleWins,
                                 statistics.battleDefeats,
                                 statistics.battleEscapes,
                                 battlePage),
        1);
    battleLayout->addLayout(battleCharts, 1);

    QLabel *battleSummary = new QLabel(
        QString("胜利 %1 次｜失败 %2 次｜逃跑 %3 次｜总战斗 %4 次")
            .arg(statistics.battleWins)
            .arg(statistics.battleDefeats)
            .arg(statistics.battleEscapes)
            .arg(statistics.totalBattles),
        battlePage);
    battleSummary->setObjectName("battleSummary");
    battleSummary->setAlignment(Qt::AlignCenter);
    battleLayout->addWidget(battleSummary);

    tabs->addTab(battlePage, "战斗分析");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    QPushButton *closeButton = new QPushButton("关闭", this);
    closeButton->setMinimumWidth(120);
    QObject::connect(closeButton, &QPushButton::clicked,
                     this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);
    rootLayout->addLayout(buttonLayout);

    setStyleSheet(
        "QDialog { background: #f4f8fc; font-family: 'Microsoft YaHei'; }"
        "#heading { color: #244a6a; font-size: 24px; font-weight: 700; }"
        "#subheading, #hint { color: #6b7b8f; font-size: 13px; }"
        "QTabWidget::pane { border: 1px solid #d6e2ee; border-radius: 12px;"
        "                  background: #f9fbfd; top: -1px; }"
        "QTabBar::tab { background: #eaf1f7; color: #536b7e;"
        "               padding: 10px 24px; margin-right: 4px;"
        "               border-top-left-radius: 9px;"
        "               border-top-right-radius: 9px; }"
        "QTabBar::tab:selected { background: #ffffff; color: #2f6da4;"
        "                        font-weight: 700; }"
        "#metricCard { background: #ffffff; border: 1px solid #dce7f1;"
        "              border-radius: 13px; }"
        "#metricTitle { color: #708397; font-size: 13px; }"
        "#metricValue { color: #2d5575; font-size: 23px; font-weight: 700; }"
        "#metricNote { color: #8898a7; font-size: 11px; }"
        "#analysisFrame { background: #eef6fd; border: 1px solid #cfe1f1;"
        "                 border-radius: 13px; }"
        "#sectionTitle { color: #315f83; font-size: 17px; font-weight: 700; }"
        "#analysisText { color: #415a70; font-size: 14px; line-height: 1.5; }"
        "#battleSummary { color: #405d73; font-size: 14px;"
        "                 background: #ffffff; border: 1px solid #dce7f1;"
        "                 border-radius: 10px; padding: 10px; }"
        "QComboBox { min-height: 34px; padding: 3px 10px;"
        "            border: 1px solid #bfd1e1; border-radius: 8px;"
        "            background: #ffffff; color: #405d73; }"
        "QPushButton { min-height: 38px; padding: 4px 20px;"
        "              border: 1px solid #6c9dcc; border-radius: 10px;"
        "              background: #6c9dcc; color: white; font-weight: 700; }"
        "QPushButton:hover { background: #5d8fbe; }"
        );
}
