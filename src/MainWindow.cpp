/* Copyright(C) 2016 Björn Stresing, Denis Manthey, Wolfgang Ruppel
 *
 * This program is free software : you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <http://www.gnu.org/licenses/>.
 */
#include "MainWindow.h"
#include "global.h"
#include "WidgetFileBrowser.h"
#include "WidgetAbout.h"
#include "WidgetImpBrowser.h"
#include "WizardWorkspaceLauncher.h"
#include "MetadataExtractor.h"
#include "WidgetCentral.h"
#include "WidgetSettings.h"
#include <QMenuBar>
#include <QUndoGroup>
#include <QToolBar>
#include <QCoreApplication>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDockWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QStatusBar>





MainWindow::MainWindow(QWidget *pParent /*= NULL*/) :
QMainWindow(pParent) {

	InitLayout();
	InitMenuAndToolbar();
}

void MainWindow::InitLayout() {

	mpMsgBox = new QMessageBox(this);
	mpMsgBox->setIcon(QMessageBox::Warning);

	mpCentralWidget = new WidgetCentral(this);

	setCentralWidget(mpCentralWidget);
	setStatusBar(new QStatusBar(this));



	QDockWidget *p_dock_widget_imp_browser = new QDockWidget(tr("IMP Browser"), this);
	QWidget *p_second_intermediate_widget = new QWidget(this); // We need this widget for painting the border defined in stylesheet.
	mpWidgetImpBrowser = new WidgetImpBrowser(p_second_intermediate_widget);
	QHBoxLayout *p_second_intermediate_layout = new QHBoxLayout();
	p_second_intermediate_layout->setMargin(0);
	p_second_intermediate_layout->addWidget(mpWidgetImpBrowser);
	p_second_intermediate_widget->setLayout(p_second_intermediate_layout);
	p_dock_widget_imp_browser->setWidget(p_second_intermediate_widget);
	p_dock_widget_imp_browser->setFeatures(QDockWidget::DockWidgetMovable);
#ifdef IMFTOOL
	addDockWidget(Qt::LeftDockWidgetArea, p_dock_widget_imp_browser);
#endif
	connect(mpWidgetImpBrowser, SIGNAL(ShowCpl(const QUuid &)), this, SLOT(ShowCplEditor(const QUuid &)));
	connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(rFocusChanged(QWidget*, QWidget*)));
}

void MainWindow::InitMenuAndToolbar() {

	mpUndoGroup = new QUndoGroup(this);

	mpActionSave = new QAction(QIcon(":/save.png"), tr("&Save CPL"), menuBar());
	mpActionSave->setShortcut(QKeySequence::Save);
	mpActionSave->setDisabled(true);
	connect(mpUndoGroup, SIGNAL(cleanChanged(bool)), mpActionSave, SLOT(setDisabled(bool)));
	connect(mpActionSave, SIGNAL(triggered()), this, SLOT(rSaveCPLRequest()));
	//mpActionSaveAll = new QAction(QIcon(":/save_all.png"), tr("&Save All"), menuBar());
	//mpActionSaveAll->setDisabled(true);

	mpActionSaveAsNewCPL = new QAction(QIcon(":/save_as_new.png"), tr("&Save as new CPL"), menuBar());
	mpActionSaveAsNewCPL->setDisabled(true);
	connect(mpUndoGroup, SIGNAL(cleanChanged(bool)), mpActionSaveAsNewCPL, SLOT(setDisabled(bool)));
	connect(mpActionSaveAsNewCPL, SIGNAL(triggered()), this, SLOT(SaveAsNewCPL()));

	QAction *p_action_undo = mpUndoGroup->createUndoAction(this, tr("Undo"));
	p_action_undo->setIcon(QIcon(":/undo.png"));
	p_action_undo->setShortcut(QKeySequence::Undo);
	QAction *p_action_redo = mpUndoGroup->createRedoAction(this, tr("Redo"));
	p_action_redo->setIcon(QIcon(":/redo.png"));
	p_action_redo->setShortcut(QKeySequence::Redo);
	QMenu *p_menu_about = new QMenu(tr("&HELP"), menuBar());
	QAction *p_action_about = new QAction(QIcon(":/information.png"), tr("&About"), menuBar());
	connect(p_action_about, SIGNAL(triggered(bool)), this, SLOT(ShowWidgetAbout()));
	p_menu_about->addAction(p_action_about);
	QMenu *p_menu_file = new QMenu(tr("&FILE"), menuBar());
	QAction *p_action_open = new QAction(QIcon(":/folder.png"), tr("&Open IMF Package"), menuBar());
	connect(p_action_open, SIGNAL(triggered(bool)), this, SLOT(rOpenImfRequest()));
	p_action_open->setShortcut(QKeySequence::Open);
	QAction *p_action_write = new QAction(QIcon(":/inbox_upload.png"), tr("&Write IMF Package"), menuBar());
	p_action_write->setDisabled(true);
	connect(p_action_write, SIGNAL(triggered(bool)), this, SLOT(WritePackage()));
	connect(mpWidgetImpBrowser, SIGNAL(ImpSaveStateChanged(bool)), p_action_write, SLOT(setEnabled(bool)));
	QAction *p_action_close = new QAction(QIcon(":/close.png"), tr("&Close IMF Package"), menuBar());
	connect(p_action_close, SIGNAL(triggered(bool)), this, SLOT(CloseImfPackage()));
	QAction *p_action_exit = new QAction(tr("&Quit"), menuBar());
	p_action_exit->setShortcut(tr("Ctrl+Q"));
	connect(p_action_exit, SIGNAL(triggered(bool)), qApp, SLOT(closeAllWindows()));

	p_menu_file->addAction(p_action_write);
	p_menu_file->addAction(p_action_open);
	p_menu_file->addAction(p_action_close);
	p_menu_file->addSeparator();
	p_menu_file->addAction(mpActionSave);
	p_menu_file->addAction(mpActionSaveAsNewCPL);
	//p_menu_file->addAction(mpActionSaveAll);
	p_menu_file->addSeparator();
	p_menu_file->addAction(p_action_exit);
	QMenu *p_menu_tools = new QMenu(tr("&TOOLS"), menuBar());
	QAction *p_action_preferences = new QAction(QIcon(":/gear.png"), tr("&Preferences"), menuBar());
	connect(p_action_preferences, SIGNAL(triggered(bool)), this, SLOT(ShowWidgetSettings()));
	p_menu_tools->addAction(p_action_preferences);

	menuBar()->addMenu(p_menu_file);
	menuBar()->addMenu(p_menu_tools);
	menuBar()->addMenu(p_menu_about);

	QToolBar *p_tool_bar = addToolBar(tr("Main Window Toolbar"));
	p_tool_bar->setIconSize(QSize(20, 20));
	p_tool_bar->addAction(p_action_open);
	p_tool_bar->addAction(p_action_write);
	p_tool_bar->addSeparator();
	//p_tool_bar->addAction(mpActionSaveAll);
	p_tool_bar->addSeparator();
	p_tool_bar->addSeparator();
	p_tool_bar->addAction(mpActionSave);
	p_tool_bar->addAction(mpActionSaveAsNewCPL);
	p_tool_bar->addAction(p_action_undo);
	p_tool_bar->addAction(p_action_redo);

	connect(mpCentralWidget, SIGNAL(UndoStackChanged(QUndoStack*)), mpUndoGroup, SLOT(setActiveStack(QUndoStack*)));
}

			/* -----Denis Manthey----- */

void MainWindow::rSaveCPLRequest() {

	mpMsgBox->setText(tr("Overwrite CPL?"));
	mpMsgBox->setInformativeText(tr("This action can not be undone!"));
	mpMsgBox->setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
	mpMsgBox->setDefaultButton(QMessageBox::Cancel);
	mpMsgBox->setIcon(QMessageBox::Warning);
	int ret = mpMsgBox->exec();
	if(ret == QMessageBox::Save)
		SaveCurrent();
	else if(ret == QMessageBox::Cancel)
		return;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
	if (rQuitRequest() == 1)
		event->ignore();
	else
		event->accept();
}

void MainWindow::rOpenImfRequest() {
	if (rQuitRequest() == 1)
		return;
	else
		ShowWorkspaceLauncher();
}

bool MainWindow::rQuitRequest() {

	//check if every UndoStack is clean or not!
	bool changes = false;
	for (int i = 0; i < mpUndoGroup->stacks().size(); i++){
		if (mpUndoGroup->stacks().at(i)->isClean() == false)
			changes = true;
	}
	if (mpWidgetImpBrowser->GetUndoStack()->isClean() == false)
		changes = true;

	if(changes == true) {
		mpMsgBox->setText(tr("There are unsaved changes in the current IMP!"));
		mpMsgBox->setInformativeText(tr("Do you want to proceed?"));
		mpMsgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
		mpMsgBox->setDefaultButton(QMessageBox::Cancel);
		mpMsgBox->setIcon(QMessageBox::Warning);
		int ret = mpMsgBox->exec();

		if(ret == QMessageBox::Cancel)
			return 1;
		else {
			for(int i=0; i<mpUnwrittenCPLs.size(); i++){
				QFile::remove(mpUnwrittenCPLs.at(i));
			}
			return 0;
		}
	}
	else
		return 0;
}

			/* -----Denis Manthey----- */

void MainWindow::ShowWidgetAbout() {

	WidgetAbout *p_about = new WidgetAbout(this);
	p_about->setAttribute(Qt::WA_DeleteOnClose);
	CenterWidget(p_about, true);
	p_about->show();
}

void MainWindow::ShowWorkspaceLauncher() {

	WizardWorkspaceLauncher *p_workspace_launcher = new WizardWorkspaceLauncher(this);
	p_workspace_launcher->setAttribute(Qt::WA_DeleteOnClose);
	p_workspace_launcher->addPage(new WizardWorkspaceLauncherPage(p_workspace_launcher));
	connect(p_workspace_launcher, SIGNAL(accepted()), this, SLOT(rWorkspaceLauncherAccepted()));
	p_workspace_launcher->show();
}


void MainWindow::rWorkspaceLauncherAccepted() {

	WizardWorkspaceLauncher *p_workspace_launcher = qobject_cast<WizardWorkspaceLauncher *>(sender());
	if(p_workspace_launcher) {
		QString working_dir = p_workspace_launcher->field(FIELD_NAME_WORKING_DIR).toString();
		QDir dir(working_dir);
		QSharedPointer<ImfPackage> imf_package(new ImfPackage(dir));
		ImfError error = imf_package->Ingest();
		if(error.IsError() == false) {
			if(error.IsRecoverableError() == true) {
				QString error_msg = QString("%1\n%2").arg(error.GetErrorMsg()).arg(error.GetErrorDescription());
				mpMsgBox->setText(tr("Ingest Warning"));
				mpMsgBox->setIcon(QMessageBox::Warning);
				mpMsgBox->setInformativeText(error_msg);
				mpMsgBox->setStandardButtons(QMessageBox::Ok);
				mpMsgBox->setDefaultButton(QMessageBox::Ok);
				mpMsgBox->exec();
			}
			mpWidgetImpBrowser->InstallImp(imf_package);
			mpCentralWidget->InstallImp(imf_package);
		}
		else {
			mpWidgetImpBrowser->UninstallImp();
			mpCentralWidget->UninstallImp();
			QString error_msg = QString("%1\n%2").arg(error.GetErrorMsg()).arg(error.GetErrorDescription());
			mpMsgBox->setText(tr("Ingest Error"));
			mpMsgBox->setIcon(QMessageBox::Critical);
			mpMsgBox->setInformativeText(error_msg);
			mpMsgBox->setStandardButtons(QMessageBox::Ok);
			mpMsgBox->setDefaultButton(QMessageBox::Ok);
			mpMsgBox->exec();
		}
	}
}


void MainWindow::rEnableSaveActions() {

	mpActionSave->setEnabled(true);
	mpActionSaveAll->setEnabled(true);
}

void MainWindow::CloseImfPackage() {

	mpWidgetImpBrowser->UninstallImp();
	mpCentralWidget->UninstallImp();
}

void MainWindow::ShowCplEditor(const QUuid &rCplAssetId) {

	int index = mpCentralWidget->ShowCplEditor(rCplAssetId);
	if(index >= 0) mpUndoGroup->addStack(mpCentralWidget->GetUndoStack(index));
}

void MainWindow::SaveCurrent() {

	mpCentralWidget->SaveCurrentCpl();
}


			/* -----Denis Manthey----- */
void MainWindow::SaveAsNewCPL() {

	QSharedPointer<AssetCpl> newCPL = mpWidgetImpBrowser->GenerateEmptyCPL();
	mpCentralWidget->CopyCPL(newCPL);
	SetUnwrittenCPL(newCPL->GetPath().absoluteFilePath());
}
			/* -----Denis Manthey----- */


void MainWindow::rFocusChanged(QWidget *pOld, QWidget *pNow) {

	if(mpCentralWidget->isAncestorOf(pNow)) {
		mpUndoGroup->setActiveStack(mpCentralWidget->GetCurrentUndoStack());
	}
	else mpUndoGroup->setActiveStack(NULL);
}

void MainWindow::ShowWidgetSettings() {

	WidgetSettings *p_settings_widget = new WidgetSettings(this);
	p_settings_widget->setAttribute(Qt::WA_DeleteOnClose);
	p_settings_widget->AddSettingsPage(new WidgetAudioSettingsPage());
	CenterWidget(p_settings_widget, true);
	connect(p_settings_widget, SIGNAL(SettingsSaved()), this, SIGNAL(SettingsSaved()));
	p_settings_widget->show();
}

void MainWindow::CenterWidget(QWidget *pWidget, bool useSizeHint) {

	QSize size;
	if(useSizeHint)
		size = pWidget->sizeHint();
	else
		size = pWidget->size();

	QDesktopWidget *d = QApplication::desktop();
	int w = d->width();   // returns screen width
	int h = d->height();  // returns screen height
	int mw = size.width();
	int mh = size.height();
	int cw = (w / 2) - (mw / 2);
	int ch = (h / 2) - (mh / 2);
	pWidget->move(cw, ch);
}

void MainWindow::WritePackage() {

	mpWidgetImpBrowser->Save();
	mpUnwrittenCPLs.clear();
}

void MainWindow::SetUnwrittenCPL(QString FilePath) {
	mpUnwrittenCPLs.append(FilePath);
}
