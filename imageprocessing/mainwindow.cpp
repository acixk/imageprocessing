#include "MainWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QStyle>
#include <QTabWidget>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    loadSettings();
    onSwitchLanguage(m_language);

    createActions();
    createMenusAndToolBars();
    createCentralCanvas();
    createColorDock();
    createPixelDock();

    m_statusPixelLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statusPixelLabel);

    retranslateUi();
    updateActionStates();
    resize(1080, 700);
}

void MainWindow::loadSettings() {
    QSettings settings(QStringLiteral("MyStudio"), QStringLiteral("QtImageEditorV1"));
    m_language = settings.value(QStringLiteral("language"), QStringLiteral("zh_TW")).toString();
    m_defaultSaveFormat = settings.value(QStringLiteral("defaultSaveFormat"), QStringLiteral("PNG")).toString();
    m_maxUndoSteps = settings.value(QStringLiteral("maxUndoSteps"), 20).toInt();
}

void MainWindow::saveSettings() {
    QSettings settings(QStringLiteral("MyStudio"), QStringLiteral("QtImageEditorV1"));
    settings.setValue(QStringLiteral("language"), m_language);
    settings.setValue(QStringLiteral("defaultSaveFormat"), m_defaultSaveFormat);
    settings.setValue(QStringLiteral("maxUndoSteps"), m_maxUndoSteps);
}

void MainWindow::onSwitchLanguage(const QString &langCode) {
    m_language = langCode;
    qApp->removeTranslator(&m_translator);

    // 若已準備 .qm 翻譯檔（例如 translations/app_en_US.qm），可在此載入
    const QString qmPath = QStringLiteral(":/i18n/app_%1.qm").arg(langCode);
    if (m_translator.load(qmPath)) {
        qApp->installTranslator(&m_translator);
    }

    retranslateUi();
    saveSettings();
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::createActions() {
    QStyle *st = style();

    m_newAct = new QAction(st->standardIcon(QStyle::SP_FileIcon), QString(), this);
    m_newAct->setShortcut(QKeySequence::New);
    connect(m_newAct, &QAction::triggered, this, &MainWindow::onNewBlankImage);

    m_openAct = new QAction(st->standardIcon(QStyle::SP_DialogOpenButton), QString(), this);
    m_openAct->setShortcut(QKeySequence::Open);
    connect(m_openAct, &QAction::triggered, this, &MainWindow::onOpenImage);

    m_saveAct = new QAction(st->standardIcon(QStyle::SP_DialogSaveButton), QString(), this);
    m_saveAct->setShortcut(QKeySequence::Save);
    connect(m_saveAct, &QAction::triggered, this, &MainWindow::onSaveImage);

    m_saveAsAct = new QAction(st->standardIcon(QStyle::SP_DriveFDIcon), QString(), this);
    m_saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAct, &QAction::triggered, this, &MainWindow::onSaveImageAs);

    m_exitAct = new QAction(st->standardIcon(QStyle::SP_DialogCloseButton), QString(), this);
    m_exitAct->setShortcut(QKeySequence::Quit);
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    m_undoAct = new QAction(st->standardIcon(QStyle::SP_ArrowBack), QString(), this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    connect(m_undoAct, &QAction::triggered, this, &MainWindow::onUndo);

    m_resetAct = new QAction(st->standardIcon(QStyle::SP_BrowserReload), QString(), this);
    connect(m_resetAct, &QAction::triggered, this, &MainWindow::onResetImage);

    m_settingsAct = new QAction(st->standardIcon(QStyle::SP_ComputerIcon), QString(), this);
    connect(m_settingsAct, &QAction::triggered, this, &MainWindow::onOpenSettingsDialog);

    m_langZhTwAct = new QAction(QStringLiteral("繁體中文"), this);
    m_langZhTwAct->setCheckable(true);
    connect(m_langZhTwAct, &QAction::triggered, this, [this]() { onSwitchLanguage(QStringLiteral("zh_TW")); });

    m_langEnUsAct = new QAction(QStringLiteral("English"), this);
    m_langEnUsAct->setCheckable(true);
    connect(m_langEnUsAct, &QAction::triggered, this, [this]() { onSwitchLanguage(QStringLiteral("en_US")); });

    auto *langGroup = new QActionGroup(this);
    langGroup->addAction(m_langZhTwAct);
    langGroup->addAction(m_langEnUsAct);
    langGroup->setExclusive(true);
    m_langZhTwAct->setChecked(m_language == QStringLiteral("zh_TW"));
    m_langEnUsAct->setChecked(m_language == QStringLiteral("en_US"));

    m_tutorialAct = new QAction(st->standardIcon(QStyle::SP_DialogHelpButton), QString(), this);
    m_tutorialAct->setShortcut(QKeySequence::HelpContents);
    connect(m_tutorialAct, &QAction::triggered, this, &MainWindow::onOpenTutorialDialog);
}

void MainWindow::createMenusAndToolBars() {
    m_fileMenu = menuBar()->addMenu(QString());
    m_fileMenu->addAction(m_newAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    m_editMenu = menuBar()->addMenu(QString());
    m_editMenu->addAction(m_undoAct);
    m_editMenu->addAction(m_resetAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_settingsAct);

    m_imageMenu = menuBar()->addMenu(QString());
    // 面板開關動作於 createColorDock / createPixelDock 建立後加入

    m_langMenu = menuBar()->addMenu(QString());
    m_langMenu->addAction(m_langZhTwAct);
    m_langMenu->addAction(m_langEnUsAct);

    m_helpMenu = menuBar()->addMenu(QString());
    m_helpMenu->addAction(m_tutorialAct);

    m_mainToolBar = addToolBar(QString());
    m_mainToolBar->setObjectName(QStringLiteral("MainToolBar"));
    m_mainToolBar->addAction(m_newAct);
    m_mainToolBar->addAction(m_openAct);
    m_mainToolBar->addAction(m_saveAct);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_undoAct);
    m_mainToolBar->addAction(m_resetAct);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_settingsAct);
    m_mainToolBar->addAction(m_tutorialAct);
}

void MainWindow::createCentralCanvas() {
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setAlignment(Qt::AlignCenter);

    m_canvasLabel = new QLabel(this);
    m_canvasLabel->setAlignment(Qt::AlignCenter);
    m_canvasLabel->setMouseTracking(true);
    m_canvasLabel->installEventFilter(this);

    m_scrollArea->setWidget(m_canvasLabel);
    setCentralWidget(m_scrollArea);
}

void MainWindow::createColorDock() {
    m_colorDock = new QDockWidget(this);
    m_colorDock->setObjectName(QStringLiteral("ColorDock"));
    m_colorDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *panel = new QWidget(m_colorDock);
    auto *layout = new QVBoxLayout(panel);

    // 亮度控制
    auto *brightRow = new QHBoxLayout();
    m_brightnessSlider = new QSlider(Qt::Horizontal, panel);
    m_brightnessSlider->setRange(-100, 100);
    m_brightnessSpin = new QSpinBox(panel);
    m_brightnessSpin->setRange(-100, 100);
    brightRow->addWidget(m_brightnessSlider);
    brightRow->addWidget(m_brightnessSpin);

    // 對比控制
    auto *contrastRow = new QHBoxLayout();
    m_contrastSlider = new QSlider(Qt::Horizontal, panel);
    m_contrastSlider->setRange(-100, 100);
    m_contrastSpin = new QSpinBox(panel);
    m_contrastSpin->setRange(-100, 100);
    contrastRow->addWidget(m_contrastSlider);
    contrastRow->addWidget(m_contrastSpin);

    auto *applyColorBtn = new QPushButton(panel);
    applyColorBtn->setObjectName(QStringLiteral("ApplyColorBtn"));

    layout->addWidget(new QLabel(QStringLiteral("亮度 (Brightness)"), panel));
    layout->addLayout(brightRow);
    layout->addWidget(new QLabel(QStringLiteral("對比 (Contrast)"), panel));
    layout->addLayout(contrastRow);
    layout->addWidget(applyColorBtn);
    layout->addStretch();

    m_colorDock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_colorDock);

    m_colorDockAct = m_colorDock->toggleViewAction();
    m_imageMenu->addAction(m_colorDockAct);

    // 滑桿與 SpinBox 雙向同步，並觸發即時預覽
    connect(m_brightnessSlider, &QSlider::valueChanged, m_brightnessSpin, &QSpinBox::setValue);
    connect(m_brightnessSpin, qOverload<int>(&QSpinBox::valueChanged), m_brightnessSlider, &QSlider::setValue);
    connect(m_contrastSlider, &QSlider::valueChanged, m_contrastSpin, &QSpinBox::setValue);
    connect(m_contrastSpin, qOverload<int>(&QSpinBox::valueChanged), m_contrastSlider, &QSlider::setValue);

    connect(m_brightnessSlider, &QSlider::valueChanged, this, &MainWindow::onColorPreviewChanged);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &MainWindow::onColorPreviewChanged);
    connect(applyColorBtn, &QPushButton::clicked, this, &MainWindow::onApplyColorAdjustments);
}

void MainWindow::createPixelDock() {
    m_pixelDock = new QDockWidget(this);
    m_pixelDock->setObjectName(QStringLiteral("PixelDock"));
    m_pixelDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto *panel = new QWidget(m_pixelDock);
    auto *layout = new QFormLayout(panel);

    m_pixelXSpin = new QSpinBox(panel);
    m_pixelXSpin->setRange(0, 99999);
    m_pixelYSpin = new QSpinBox(panel);
    m_pixelYSpin->setRange(0, 99999);

    m_pixelRSpin = new QSpinBox(panel);
    m_pixelRSpin->setRange(0, 255);
    m_pixelGSpin = new QSpinBox(panel);
    m_pixelGSpin->setRange(0, 255);
    m_pixelBSpin = new QSpinBox(panel);
    m_pixelBSpin->setRange(0, 255);

    auto *applyPixelBtn = new QPushButton(panel);
    applyPixelBtn->setObjectName(QStringLiteral("ApplyPixelBtn"));

    layout->addRow(QStringLiteral("X:"), m_pixelXSpin);
    layout->addRow(QStringLiteral("Y:"), m_pixelYSpin);
    layout->addRow(QStringLiteral("R:"), m_pixelRSpin);
    layout->addRow(QStringLiteral("G:"), m_pixelGSpin);
    layout->addRow(QStringLiteral("B:"), m_pixelBSpin);
    layout->addRow(applyPixelBtn);

    m_pixelDock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, m_pixelDock);

    m_pixelDockAct = m_pixelDock->toggleViewAction();
    m_imageMenu->addAction(m_pixelDockAct);

    connect(applyPixelBtn, &QPushButton::clicked, this, &MainWindow::onApplyPixelChange);
}

void MainWindow::retranslateUi() {
    const bool isZh = (m_language == QStringLiteral("zh_TW"));

    setWindowTitle(isZh ? QStringLiteral("桌面影像處理程式 v1") : QStringLiteral("Desktop Image Editor v1"));

    if (m_fileMenu) m_fileMenu->setTitle(isZh ? QStringLiteral("檔案(&F)") : QStringLiteral("File(&F)"));
    if (m_editMenu) m_editMenu->setTitle(isZh ? QStringLiteral("編輯(&E)") : QStringLiteral("Edit(&E)"));
    if (m_imageMenu) m_imageMenu->setTitle(isZh ? QStringLiteral("影像(&I)") : QStringLiteral("Image(&I)"));
    if (m_langMenu) m_langMenu->setTitle(isZh ? QStringLiteral("語言(&L)") : QStringLiteral("Language(&L)"));
    if (m_helpMenu) m_helpMenu->setTitle(isZh ? QStringLiteral("說明(&H)") : QStringLiteral("Help(&H)"));
    if (m_mainToolBar) m_mainToolBar->setWindowTitle(isZh ? QStringLiteral("主工具列") : QStringLiteral("Main Toolbar"));

    if (m_newAct) m_newAct->setText(isZh ? QStringLiteral("新增空白影像") : QStringLiteral("New Blank Image"));
    if (m_openAct) m_openAct->setText(isZh ? QStringLiteral("開啟影像...") : QStringLiteral("Open Image..."));
    if (m_saveAct) m_saveAct->setText(isZh ? QStringLiteral("儲存") : QStringLiteral("Save"));
    if (m_saveAsAct) m_saveAsAct->setText(isZh ? QStringLiteral("另存新檔...") : QStringLiteral("Save As..."));
    if (m_exitAct) m_exitAct->setText(isZh ? QStringLiteral("離開") : QStringLiteral("Exit"));

    if (m_undoAct) m_undoAct->setText(isZh ? QStringLiteral("復原") : QStringLiteral("Undo"));
    if (m_resetAct) m_resetAct->setText(isZh ? QStringLiteral("重設為原圖") : QStringLiteral("Reset to Original"));
    if (m_settingsAct) m_settingsAct->setText(isZh ? QStringLiteral("設定選單...") : QStringLiteral("Settings..."));

    if (m_colorDock) m_colorDock->setWindowTitle(isZh ? QStringLiteral("更改顏色") : QStringLiteral("Color Adjustment"));
    if (m_pixelDock) m_pixelDock->setWindowTitle(isZh ? QStringLiteral("更改像素") : QStringLiteral("Pixel Edit"));

    if (auto *btn = findChild<QPushButton *>(QStringLiteral("ApplyColorBtn"))) {
        btn->setText(isZh ? QStringLiteral("套用色彩調整") : QStringLiteral("Apply Color"));
    }
    if (auto *btn = findChild<QPushButton *>(QStringLiteral("ApplyPixelBtn"))) {
        btn->setText(isZh ? QStringLiteral("套用單點像素") : QStringLiteral("Apply Pixel"));
    }

    if (m_tutorialAct) m_tutorialAct->setText(isZh ? QStringLiteral("使用教學") : QStringLiteral("Tutorial"));

    if (m_currentImage.isNull() && m_canvasLabel) {
        m_canvasLabel->setText(isZh
                                   ? QStringLiteral("目前尚無影像\n請由「檔案」開啟圖片或建立空白影像")
                                   : QStringLiteral("No image loaded\nOpen an image or create a blank one from File"));
    }

    if (m_statusPixelLabel && m_statusPixelLabel->text().isEmpty()) {
        m_statusPixelLabel->setText(isZh ? QStringLiteral("座標: (-, -) | RGB: (-, -, -)")
                                         : QStringLiteral("Pos: (-, -) | RGB: (-, -, -)"));
    }
}

void MainWindow::onNewBlankImage() {
    if (!maybeSaveChanges()) {
        return;
    }

    QImage blank(640, 480, QImage::Format_RGB32);
    blank.fill(Qt::white);

    m_originalImage = blank;
    m_currentFilePath.clear();
    m_undoStack.clear();
    m_isAdjustingColor = false;

    m_brightnessSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_brightnessSpin->setValue(0);
    m_contrastSpin->setValue(0);
    m_brightnessSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);

    setWorkingImage(blank, false, false);
    statusBar()->showMessage(QStringLiteral("已建立 640x480 空白影像"), 3000);
}

void MainWindow::onOpenImage() {
    if (!maybeSaveChanges()) {
        return;
    }

    const QString filePath = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("開啟影像"),
        QString(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp);;All Files (*)")
        );
    if (filePath.isEmpty()) {
        return;
    }

    QImage loaded(filePath);
    if (loaded.isNull()) {
        QMessageBox::warning(this, QStringLiteral("開啟失敗"), QStringLiteral("無法讀取此影像檔案。"));
        return;
    }

    m_originalImage = loaded.convertToFormat(QImage::Format_RGB32);
    m_currentFilePath = filePath;
    m_undoStack.clear();
    m_isAdjustingColor = false;

    m_brightnessSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_brightnessSpin->setValue(0);
    m_contrastSpin->setValue(0);
    m_brightnessSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);

    setWorkingImage(m_originalImage, false, false);
    statusBar()->showMessage(QStringLiteral("已載入：%1").arg(filePath), 3000);
}

bool MainWindow::onSaveImage() {
    if (m_currentImage.isNull()) {
        return false;
    }
    if (m_currentFilePath.isEmpty()) {
        return onSaveImageAs();
    }
    if (!m_currentImage.save(m_currentFilePath)) {
        QMessageBox::warning(this, QStringLiteral("儲存失敗"), QStringLiteral("寫入檔案時發生錯誤。"));
        return false;
    }
    m_isModified = false;
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已儲存至：%1").arg(m_currentFilePath), 3000);
    return true;
}

bool MainWindow::onSaveImageAs() {
    if (m_currentImage.isNull()) {
        return false;
    }
    const QString filter = QStringLiteral("PNG (*.png);;JPEG (*.jpg);;BMP (*.bmp)");
    QString filePath = QFileDialog::getSaveFileName(this, QStringLiteral("另存新檔"), QString(), filter);
    if (filePath.isEmpty()) {
        return false;
    }

    if (!filePath.contains(QLatin1Char('.'))) {
        filePath += QStringLiteral(".%1").arg(m_defaultSaveFormat.toLower());
    }

    if (!m_currentImage.save(filePath)) {
        QMessageBox::warning(this, QStringLiteral("另存失敗"), QStringLiteral("無法儲存影像檔案。"));
        return false;
    }

    m_currentFilePath = filePath;
    m_isModified = false;
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已另存為：%1").arg(filePath), 3000);
    return true;
}

bool MainWindow::maybeSaveChanges() {
    if (!m_isModified || m_currentImage.isNull()) {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        this,
        QStringLiteral("尚未儲存變更"),
        QStringLiteral("目前影像尚有未儲存的修改，是否要先儲存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
        );

    if (ret == QMessageBox::Save) {
        return onSaveImage();
    }
    if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (maybeSaveChanges()) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::setWorkingImage(const QImage &img, bool pushUndo, bool markModified) {
    if (pushUndo && !m_currentImage.isNull()) {
        if (m_undoStack.size() >= m_maxUndoSteps) {
            m_undoStack.removeFirst();
        }
        m_undoStack.push(m_currentImage);
    }

    m_currentImage = img;
    if (markModified) {
        m_isModified = true;
    } else if (!pushUndo) {
        m_isModified = false;
    }

    refreshCanvasDisplay();
    updateActionStates();
}

void MainWindow::refreshCanvasDisplay() {
    if (m_currentImage.isNull()) {
        m_canvasLabel->setPixmap(QPixmap());
        retranslateUi();
        return;
    }

    const QPixmap pix = QPixmap::fromImage(m_currentImage);
    m_canvasLabel->setPixmap(pix);
    m_canvasLabel->resize(pix.size());
}

void MainWindow::onUndo() {
    if (m_undoStack.isEmpty()) {
        return;
    }
    m_currentImage = m_undoStack.pop();
    m_isModified = true;
    refreshCanvasDisplay();
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已復原一步"), 2000);
}

void MainWindow::onResetImage() {
    if (m_originalImage.isNull()) {
        return;
    }

    m_brightnessSlider->blockSignals(true);
    m_contrastSlider->blockSignals(true);
    m_brightnessSlider->setValue(0);
    m_contrastSlider->setValue(0);
    m_brightnessSpin->setValue(0);
    m_contrastSpin->setValue(0);
    m_brightnessSlider->blockSignals(false);
    m_contrastSlider->blockSignals(false);
    m_isAdjustingColor = false;

    setWorkingImage(m_originalImage, true, true);
    statusBar()->showMessage(QStringLiteral("已重設為原始影像"), 2000);
}

void MainWindow::onColorPreviewChanged() {
    if (m_currentImage.isNull()) {
        return;
    }

    if (!m_isAdjustingColor) {
        m_baseColorImage = m_currentImage;
        m_isAdjustingColor = true;
    }

    const int brightness = m_brightnessSlider->value();
    const int contrast = m_contrastSlider->value();
    const double factor = (100.0 + contrast) / 100.0;

    QImage preview = m_baseColorImage.copy();
    for (int y = 0; y < preview.height(); ++y) {
        auto *line = reinterpret_cast<QRgb *>(preview.scanLine(y));
        for (int x = 0; x < preview.width(); ++x) {
            const QColor c(line[x]);
            auto adjustChannel = [&](int ch) -> int {
                const double v = ((ch - 128.0) * factor + 128.0) + brightness;
                return std::clamp(static_cast<int>(std::lround(v)), 0, 255);
            };
            line[x] = qRgb(adjustChannel(c.red()), adjustChannel(c.green()), adjustChannel(c.blue()));
        }
    }

    m_currentImage = preview;
    refreshCanvasDisplay();
}

void MainWindow::onApplyColorAdjustments() {
    if (!m_isAdjustingColor || m_currentImage.isNull()) {
        return;
    }

    // 將調整前的基準圖推入 Undo，並標記為已修改
    if (m_undoStack.size() >= m_maxUndoSteps) {
        m_undoStack.removeFirst();
    }
    m_undoStack.push(m_baseColorImage);

    m_isAdjustingColor = false;
    m_isModified = true;
    updateActionStates();
    statusBar()->showMessage(QStringLiteral("已套用色彩調整"), 2000);
}

void MainWindow::onApplyPixelChange() {
    if (m_currentImage.isNull()) {
        return;
    }

    const int x = m_pixelXSpin->value();
    const int y = m_pixelYSpin->value();

    if (x < 0 || x >= m_currentImage.width() || y < 0 || y >= m_currentImage.height()) {
        QMessageBox::warning(
            this,
            QStringLiteral("座標超出範圍"),
            QStringLiteral("請輸入 0 ~ %1 的 X 與 0 ~ %2 的 Y。").arg(m_currentImage.width() - 1).arg(m_currentImage.height() - 1)
            );
        return;
    }

    QImage next = m_currentImage.copy();
    next.setPixel(x, y, qRgb(m_pixelRSpin->value(), m_pixelGSpin->value(), m_pixelBSpin->value()));

    setWorkingImage(next, true, true);
    statusBar()->showMessage(QStringLiteral("已更新像素 (%1, %2)").arg(x).arg(y), 2000);
}

void MainWindow::onOpenSettingsDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("設定選單"));
    auto *layout = new QFormLayout(&dlg);

    auto *formatCombo = new QComboBox(&dlg);
    formatCombo->addItems({QStringLiteral("PNG"), QStringLiteral("JPG"), QStringLiteral("BMP")});
    formatCombo->setCurrentText(m_defaultSaveFormat);

    auto *undoLimitSpin = new QSpinBox(&dlg);
    undoLimitSpin->setRange(1, 100);
    undoLimitSpin->setValue(m_maxUndoSteps);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    layout->addRow(QStringLiteral("預設存檔格式："), formatCombo);
    layout->addRow(QStringLiteral("復原步數上限："), undoLimitSpin);
    layout->addRow(buttons);

    if (dlg.exec() == QDialog::Accepted) {
        m_defaultSaveFormat = formatCombo->currentText();
        m_maxUndoSteps = undoLimitSpin->value();
        saveSettings();
        statusBar()->showMessage(QStringLiteral("已儲存偏好設定"), 2000);
    }
}

void MainWindow::onOpenTutorialDialog() {
    QDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("使用教學"));
    dlg.resize(480, 320);

    auto *layout = new QVBoxLayout(&dlg);
    auto *tabs = new QTabWidget(&dlg);

    auto *guideBrowser = new QTextBrowser(tabs);
    guideBrowser->setHtml(QStringLiteral(
        "<h3>快速上手步驟</h3>"
        "<ol>"
        "<li><b>新增／開啟影像</b>：從「檔案」建立空白畫布或開啟本機圖片。</li>"
        "<li><b>更改顏色</b>：在右側「更改顏色」面板調整亮度與對比，可即時預覽並按「套用色彩調整」。</li>"
        "<li><b>更改像素</b>：在右側「更改像素」面板輸入座標與 RGB，或於畫布點擊定位後修改像素。</li>"
        "<li><b>復原／重設與存檔</b>：隨時可復原、重設回原圖，完成後儲存或另存新檔。</li>"
        "</ol>"
        ));

    auto *shortcutBrowser = new QTextBrowser(tabs);
    shortcutBrowser->setHtml(QStringLiteral(
        "<h3>常用快捷鍵</h3>"
        "<ul>"
        "<li><b>Ctrl+N</b>：新增空白影像</li>"
        "<li><b>Ctrl+O</b>：開啟影像</li>"
        "<li><b>Ctrl+S</b>：儲存影像</li>"
        "<li><b>Ctrl+Z</b>：復原上一步</li>"
        "<li><b>F1</b>：開啟使用教學</li>"
        "</ul>"
        ));

    tabs->addTab(guideBrowser, QStringLiteral("操作流程"));
    tabs->addTab(shortcutBrowser, QStringLiteral("快捷鍵"));

    auto *closeBtn = new QPushButton(QStringLiteral("關閉"), &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    layout->addWidget(tabs);
    layout->addWidget(closeBtn);
    dlg.exec();
}

void MainWindow::updateActionStates() {
    const bool hasImage = !m_currentImage.isNull();
    m_saveAct->setEnabled(hasImage);
    m_saveAsAct->setEnabled(hasImage);
    m_resetAct->setEnabled(hasImage && !m_originalImage.isNull());
    m_undoAct->setEnabled(hasImage && !m_undoStack.isEmpty());
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_canvasLabel && !m_currentImage.isNull()) {
        if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(event);
            updateStatusPixelInfo(me->pos());
        } else if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                updateStatusPixelInfo(me->pos());
                // 點擊畫布時，順便把座標帶入「更改像素」面板方便編輯
                const int x = me->pos().x();
                const int y = me->pos().y();
                if (x >= 0 && x < m_currentImage.width() && y >= 0 && y < m_currentImage.height()) {
                    m_pixelXSpin->setValue(x);
                    m_pixelYSpin->setValue(y);
                    const QColor c(m_currentImage.pixel(x, y));
                    m_pixelRSpin->setValue(c.red());
                    m_pixelGSpin->setValue(c.green());
                    m_pixelBSpin->setValue(c.blue());
                }
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::updateStatusPixelInfo(const QPoint &widgetPos) {
    if (m_currentImage.isNull()) {
        return;
    }
    const int x = widgetPos.x();
    const int y = widgetPos.y();
    if (x >= 0 && x < m_currentImage.width() && y >= 0 && y < m_currentImage.height()) {
        const QColor c(m_currentImage.pixel(x, y));
        m_statusPixelLabel->setText(
            QStringLiteral("座標: (%1, %2) | RGB: (%3, %4, %5)")
                .arg(x).arg(y).arg(c.red()).arg(c.green()).arg(c.blue())
            );
    }
}