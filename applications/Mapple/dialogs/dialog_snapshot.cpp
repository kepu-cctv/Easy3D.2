#include "dialog_snapshot.h"

#include <algorithm>
#include <QDockWidget>

#include "ui_dialog_snapshot.h"
#include "main_window.h"
#include "paint_canvas.h"


using namespace easy3d;

DialogSnapshot::DialogSnapshot(MainWindow *window, QDockWidget* dockWidgetCommand)
    : Dialog(window, dockWidgetCommand)
    , ui(new Ui::DialogSnapshot)
{
    double scale = 1.0;
    ui->setupUi(this);
    ui->doubleSpinBoxImageScale->setValue(scale);

    computeImageSize();;

    connect(ui->doubleSpinBoxImageScale, SIGNAL(valueChanged(double)), this, SLOT(computeImageSize()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(closeDialog()));
    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(saveSnapshot()));

    bestSize();
 }


DialogSnapshot::~DialogSnapshot()
{
    delete ui;
}


void DialogSnapshot::computeImageSize() {
    double scale = ui->doubleSpinBoxImageScale->value();
    int w = static_cast<int>(viewer_->width() * viewer_->dpi_scaling() * scale);
    int h = static_cast<int>(viewer_->height() * viewer_->dpi_scaling() * scale);
    ui->spinBoxImageWidth->setValue(w);
    ui->spinBoxImageHeight->setValue(h);
}


void DialogSnapshot::setImageFileName(const QString& fileName) {
    fileName_ = fileName;
}


void DialogSnapshot::closeDialog() {
    dockWidgetCommand_->close();
}


void DialogSnapshot::saveSnapshot() {
    // close the save dialog
    closeDialog();

    // disable ui to prevent the rendering from being modified.
    window_->setEnabled(false);

    // Hide closed dialog
    QApplication::processEvents();

    int w = ui->spinBoxImageWidth->value();
    int h = ui->spinBoxImageHeight->value();
    int samples = ui->spinBoxSamples->value();
    viewer_->saveSnapshot(w, h, samples, fileName_,
                          ui->checkBoxUseWhiteBackground->isChecked(),
                          ui->checkBoxExpandFrustum->isChecked()
                          );

    // restore the ui
    window_->setEnabled(true);
}
