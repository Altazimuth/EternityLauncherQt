#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QProcess>

static QStringList commandArgsList;

MainWindow::MainWindow(QWidget *parent)
   : QMainWindow(parent)
   , ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   // Set up events that use common functions
   connect(ui->toolButton_addIWAD,    SIGNAL(released()),  this, SLOT(addIWAD()));
   connect(ui->action_addIWAD,        SIGNAL(triggered()), this, SLOT(addIWAD()));
   connect(ui->toolButton_removeIWAD, SIGNAL(released()),  this, SLOT(removeIWAD()));
   connect(ui->action_removeIWAD,     SIGNAL(triggered()), this, SLOT(removeIWAD()));

   connect(ui->toolButton_addFile,    SIGNAL(released()),  this, SLOT(addFile()));
   connect(ui->action_addFile,        SIGNAL(triggered()), this, SLOT(addFile()));
   connect(ui->toolButton_removeFile, SIGNAL(released()),  this, SLOT(removeFile()));
   connect(ui->action_removeFile,     SIGNAL(triggered()), this, SLOT(removeFile()));

   // The infinite things that update params. It's sloppy and expensive (relatively) but it works.
   connect(ui->comboBox_IWAD, SIGNAL(currentIndexChanged(int)), this, SLOT(updateParams()));

   connect(ui->lineEdit_difficulty,      SIGNAL(textChanged(QString)), this, SLOT(updateParams()));
   connect(ui->lineEdit_warp,            SIGNAL(textChanged(QString)), this, SLOT(updateParams()));
   connect(ui->lineEdit_demoSave,        SIGNAL(textChanged(QString)), this, SLOT(updateParams()));
   connect(ui->lineEdit_demoPlay,        SIGNAL(textChanged(QString)), this, SLOT(updateParams()));
   connect(ui->lineEdit_otherParameters, SIGNAL(textChanged(QString)), this, SLOT(updateParams()));

   connect(ui->checkBox_respawnMonsters, SIGNAL(stateChanged(int)), this, SLOT(updateParams()));
   connect(ui->checkBox_fastMonsters,    SIGNAL(stateChanged(int)), this, SLOT(updateParams()));
   connect(ui->checkBox_noMonsters,      SIGNAL(stateChanged(int)), this, SLOT(updateParams()));
   connect(ui->checkBox_vanilla,         SIGNAL(stateChanged(int)), this, SLOT(updateParams()));
}

MainWindow::~MainWindow()
{
   delete ui;
}

void MainWindow::postDisplayConfig()
{
   QToolButton *const squareButtons[] = {
      ui->toolButton_addIWAD,
      ui->toolButton_removeIWAD,
      ui->toolButton_removeFile,
      ui->toolButton_addFile,
      ui->toolButton_removeFile,
      ui->toolButton_wikiCommandArgs
   };

   int length = -1;
   for(const QToolButton *const button : squareButtons)
   {
      const int maxSide = qMax(button->width(), button->height());
      if(maxSide > length)
         length = maxSide;
   }

   for(QToolButton *const &button : squareButtons)
      button->setFixedSize(length, length);
}

//=============================================================================
//
// Common event code
//

void MainWindow::updateParams()
{
   QPlainTextEdit *const argBox = ui->plainTextEdit_commandLine;

   commandArgsList.clear();

   // Non-tab stuff
   if(ui->comboBox_IWAD->currentIndex() != -1)
   {
      commandArgsList.append("-iwad");
      commandArgsList.append(ui->comboBox_IWAD->currentText());
   }

   if(ui->listWidget_files->count())
   {
      commandArgsList.append("-file");
      for(int i = 0; i < ui->listWidget_files->count(); i++)
      {
         const QListWidgetItem *const item = ui->listWidget_files->item(i);
         commandArgsList.append(item->text());
      }
   }

   // "Warp" tab
   if(!ui->lineEdit_difficulty->text().isEmpty())
   {
      commandArgsList.append("-skill");
      commandArgsList.append(ui->lineEdit_difficulty->text());
   }
   if(!ui->lineEdit_warp->text().isEmpty())
   {
      commandArgsList.append("-warp");
      commandArgsList.append(ui->lineEdit_warp->text());
   }

   if(ui->checkBox_respawnMonsters->isChecked())
      commandArgsList.append("-respawn");
   if(ui->checkBox_fastMonsters->isChecked())
      commandArgsList.append("-fast");
   if(ui->checkBox_noMonsters->isChecked())
      commandArgsList.append("-nomonsters");
   if(ui->checkBox_vanilla->isChecked())
      commandArgsList.append("-vanilla");

   if(!ui->lineEdit_demoSave->text().isEmpty())
   {
      commandArgsList.append("-record");
      commandArgsList.append(ui->lineEdit_demoSave->text());
   }

   // "View Demo" tab


   // "Network" tab


   // Actually write results
   argBox->clear();
   for(const QString &str : commandArgsList)
   {
      if(argBox->textCursor().columnNumber() == 1)
         argBox->insertPlainText(str);
      else if(str.startsWith("-"))
         argBox->appendPlainText(str); // Adds newline by default
      else
         argBox->insertPlainText(" " + str);
   }

   // Other parameters stuff has to be done last based on how I coded result writing
   if(!ui->lineEdit_otherParameters->text().isEmpty())
   {
      // TODO: Actually parse args
      commandArgsList.append(
         ui->lineEdit_otherParameters->text().split(
            QRegularExpression("\\s+(?=([^\"]*\"[^\"]*\")*[^\"]*$)"), // MAGIC
            QString::SkipEmptyParts
         )
      );

      argBox->appendPlainText(ui->lineEdit_otherParameters->text());
   }
}

void MainWindow::addIWAD()
{
   const QString fileStr = QFileDialog::getOpenFileName(
      this, tr("Open File"), QString(), tr("DOOM Game Files (*.wad *.iwad *.pke)")
   );
   if(fileStr.isEmpty())
      return;
   if(ui->comboBox_IWAD->count() == 0)
      ui->comboBox_IWAD->insertItem(0, fileStr);
   else
      ui->comboBox_IWAD->addItem(fileStr);

   updateParams();
}

void MainWindow::removeIWAD()
{
   if(ui->comboBox_IWAD->count() != 0 && ui->comboBox_IWAD->currentIndex() != -1)
      ui->comboBox_IWAD->removeItem(ui->comboBox_IWAD->currentIndex());

   updateParams();
}

void MainWindow::addFile()
{
   const QString fileStr = QFileDialog::getOpenFileName(
      this, tr("Open File"), QString(), tr("DOOM Game Files (*.wad *.pke)")
   );
   ui->listWidget_files->addItem(fileStr);

   updateParams();
}

void MainWindow::removeFile()
{
   const auto currItem = ui->listWidget_files->currentItem();
   if(currItem != nullptr)
      delete currItem;

   updateParams();
}

void MainWindow::openURL(const QString &urlStr)
{
   QUrl eternityWikiURL(urlStr);
   QDesktopServices::openUrl(eternityWikiURL);
}

//=============================================================================
//
// Events (slots)
//

void MainWindow::on_toolButton_wikiCommandArgs_released() { openURL("http://eternity.youfailit.net/index.php?title=List_of_command_line_parameters"); }
void MainWindow::on_actionEternity_wiki_triggered()       { openURL("http://eternity.youfailit.net/wiki/Main_Page"); }

void MainWindow::on_pushButton_warp_choose_released()
{
   const QString fileStr = QFileDialog::getSaveFileName(
      this, tr("Save File"), QString(), tr("Demo Files (*.lmp)")
   );
   if(!fileStr.isEmpty())
      ui->lineEdit_demoSave->setText(fileStr);
}
void MainWindow::on_pushButton_warp_clear_released() { ui->lineEdit_demoSave->clear(); }

void MainWindow::on_pushButton_viewDemo_choose_released()
{
   const QString fileStr = QFileDialog::getOpenFileName(
      this, tr("Open Demo"), QString(), tr("DOOM Demo (*.lmp)")
   );
   if(!fileStr.isEmpty())
      ui->lineEdit_demoPlay->setText(fileStr);
}
void MainWindow::on_pushButton_viewDemo_clear_released() { ui->lineEdit_demoPlay->clear(); }

//
// Run EE w/ the appropriate args then kill this application
//
void MainWindow::on_pushButton_startGame_released()
{
#ifdef Q_OS_WIN
   QProcess::startDetached("\"" + QCoreApplication::applicationDirPath() +"/Eternity.exe\"", commandArgsList);
#else
   QProcess::startDetached("\"" + QCoreApplication::applicationDirPath() +"/eternity\"", commandArgsList);
#endif

   QCoreApplication::quit();
}
