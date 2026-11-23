#pragma once

#include <QMainWindow>
#include <QImage>
#include <QStack>
#include <QTranslator>
#include <QString>

class QLabel;
class QScrollArea;
class QSlider;
class QSpinBox;
class QAction;
class QMenu;
class QToolBar;
class QDockWidget;
class QCloseEvent;
class QEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // 檔案流程
    void onNewBlankImage();
    void onOpenImage();
    bool onSaveImage();
    bool onSaveImageAs();

    // 編輯流程 - 復原與重設
    void onUndo();
    void onResetImage();

    // 編輯流程 - 更改顏色（即時預覽）
    void onColorPreviewChanged();
    void onApplyColorAdjustments();

    // 編輯流程 - 更改像素
    void onApplyPixelChange();

    // 系統與輔助
    void onSwitchLanguage(const QString &langCode);
    void onOpenSettingsDialog();
    void onOpenTutorialDialog();

private:
    void createActions();
    void createMenusAndToolBars();
    void createCentralCanvas();
    void createColorDock();
    void createPixelDock();
    void retranslateUi();

    void loadSettings();
    void saveSettings();

    void setWorkingImage(const QImage &img, bool pushUndo = true, bool markModified = true);
    void refreshCanvasDisplay();
    void updateActionStates();
    bool maybeSaveChanges();
    void updateStatusPixelInfo(const QPoint &widgetPos);

    // 影像資料狀態
    QImage m_originalImage;      // 原始影像（供 Reset 與亮度/對比基準計算）
    QImage m_baseColorImage;     // 調整顏色前的基準圖（避免滑桿累加失真）
    QImage m_currentImage;       // 目前畫布顯示／編輯中的影像
    QString m_currentFilePath;
    bool m_isModified = false;
    bool m_isAdjustingColor = false;

    QStack<QImage> m_undoStack;
    int m_maxUndoSteps = 20;

    // 偏好設定
    QString m_language = QStringLiteral("zh_TW");
    QString m_defaultSaveFormat = QStringLiteral("PNG");
    QTranslator m_translator;

    // UI 元件
    QScrollArea *m_scrollArea = nullptr;
    QLabel *m_canvasLabel = nullptr;
    QLabel *m_statusPixelLabel = nullptr;

    QDockWidget *m_colorDock = nullptr;
    QSlider *m_brightnessSlider = nullptr;
    QSpinBox *m_brightnessSpin = nullptr;
    QSlider *m_contrastSlider = nullptr;
    QSpinBox *m_contrastSpin = nullptr;

    QDockWidget *m_pixelDock = nullptr;
    QSpinBox *m_pixelXSpin = nullptr;
    QSpinBox *m_pixelYSpin = nullptr;
    QSpinBox *m_pixelRSpin = nullptr;
    QSpinBox *m_pixelGSpin = nullptr;
    QSpinBox *m_pixelBSpin = nullptr;

    // 選單與動作
    QMenu *m_fileMenu = nullptr;
    QMenu *m_editMenu = nullptr;
    QMenu *m_imageMenu = nullptr;
    QMenu *m_langMenu = nullptr;
    QMenu *m_helpMenu = nullptr;
    QToolBar *m_mainToolBar = nullptr;

    QAction *m_newAct = nullptr;
    QAction *m_openAct = nullptr;
    QAction *m_saveAct = nullptr;
    QAction *m_saveAsAct = nullptr;
    QAction *m_exitAct = nullptr;

    QAction *m_undoAct = nullptr;
    QAction *m_resetAct = nullptr;
    QAction *m_settingsAct = nullptr;

    QAction *m_colorDockAct = nullptr;
    QAction *m_pixelDockAct = nullptr;

    QAction *m_langZhTwAct = nullptr;
    QAction *m_langEnUsAct = nullptr;
    QAction *m_tutorialAct = nullptr;
};