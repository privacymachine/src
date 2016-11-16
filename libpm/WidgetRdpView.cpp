/*==============================================================================
        Copyright (c) 2013-2016 by the Developers of PrivacyMachine.eu
                         contact@privacymachine.eu
     OpenPGP-Fingerprint: 0C93 F15A 0ECA D404 413B 5B34 C6DE E513 0119 B175

                     Licensed under the EUPL, Version 1.1
     European Commission - subsequent versions of the EUPL (the "Licence");
        You may not use this work except in compliance with the Licence.
                  You may obtain a copy of the Licence at:
                        http://ec.europa.eu/idabc/eupl

 Unless required by applicable law or agreed to in writing, software distributed
              under the Licence is distributed on an "AS IS" basis,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
      See the Licence for the specific language governing permissions and
                        limitations under the Licence.
==============================================================================*/

#include "WidgetRdpView.h"

#include <QResizeEvent>
#include <QLabel>
#include <QGridLayout>

#include "utils.h"

WidgetRdpView::WidgetRdpView( QString parHost, PMInstance* pmInstance )
{
  host_ = parHost;
  pmInstance_ = pmInstance;
  rdpPort_ = pmInstance_->getConfig()->rdpPort;
  sshPort_ = pmInstance_->getConfig()->sshPort;
  subtractDisplayWidthMin_ = pmInstance_->getConfig()->subtractDisplayWidth;
  subtractDisplayHeightMin_ = pmInstance_->getConfig()->subtractDisplayHeight;
  connectionEstablished_ = false;
  onDisconnecting_ = false;
  retryCount_ = 0;
  inErrorMode_ = false;
  lblErrorMessage_ = new QLabel();

  name_ = pmInstance_->getConfig()->name;
  subtractDisplayWidthCurrent_ = subtractDisplayWidthMin_;
  subtractDisplayHeightCurrent_ = subtractDisplayHeightMin_;

  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  
  QPalette palette( this->palette() );
  palette.setColor( QPalette::Background, pmInstance_->getConfig()->color );
  this->setAutoFillBackground( true );
  this->setPalette( palette );

  loGrid_ = new QGridLayout();
  loGrid_->setSpacing(0);
  loGrid_->setMargin(0);
  loGrid_->setColumnStretch(0,0);
  loGrid_->setRowStretch(0,0);
  loGrid_->setSizeConstraint( QLayout::SetNoConstraint );



  #ifndef SKIP_FREERDP_CODE
    freeRDPwidget_ = new RemoteDisplayWidget();
  #else
    freeRDPwidget_ = new QLabel();
    freeRDPwidget_->setText("FreeRDP disabled in this build");
    freeRDPwidget_->show();
  #endif
  screenHeight_ = freeRDPwidget_->height();
  screenWidth_ = freeRDPwidget_->width();
  
  QPalette paletteRdp = freeRDPwidget_->palette();
  paletteRdp.setColor( this->backgroundRole(), Qt::red );
  freeRDPwidget_->setPalette( paletteRdp );


  // The FreeRDPWidget seems not to understand Qt:AlignCenter.
  // We implement a similar behaviour by aligning it top-left, and adding margins to the layout manager.
  loGrid_->setContentsMargins( subtractDisplayWidthMin_ / 2, subtractDisplayHeightMin_ / 2, 0, 0 );
  loGrid_->addWidget(freeRDPwidget_, 0, 0, 0, 0 );
  setLayout(loGrid_);

  connect(freeRDPwidget_,
          SIGNAL(disconnected()),
          this,
          SLOT(slotDisconnected()));

  timerResizeHappend_.setSingleShot(true);
  connect(&timerResizeHappend_,
          SIGNAL(timeout()),
          this,
          SLOT(slotTimerResizeHappendFired()));
}

WidgetRdpView::~WidgetRdpView()
{
  if (freeRDPwidget_)
  {
    delete (freeRDPwidget_);
    freeRDPwidget_ = 0;
  }

  if (lblErrorMessage_)
  {
    delete (lblErrorMessage_);
    lblErrorMessage_ = 0;
  }
}

void WidgetRdpView::resizeVmDesktopAndConnectViaRdp()
{
  QStringList args;

  QPalette palette;
  palette.setColor(QPalette::Background,Qt::blue);
  freeRDPwidget_->setPalette(palette);
  freeRDPwidget_->setAutoFillBackground(true);

  screenWidth_ = this->width() - subtractDisplayWidthMin_;
  int remainingWidth = screenWidth_ % 4;
  subtractDisplayWidthCurrent_ = subtractDisplayWidthMin_ + remainingWidth;
  screenWidth_ = this->width() - subtractDisplayWidthCurrent_;
  ILOG("WidgetRdpView::connectViaRdp() subtractDisplayWidthCurrent: " + QString::number(subtractDisplayWidthCurrent_) + " >= " + QString::number(subtractDisplayWidthMin_))

  screenHeight_ = this->height() - subtractDisplayHeightMin_;
  int remainingHeight = screenHeight_ % 4;
  subtractDisplayHeightCurrent_ = subtractDisplayHeightMin_ + remainingHeight;
  screenHeight_ = this->height() - subtractDisplayHeightCurrent_;
  ILOG("WidgetRdpView::connectViaRdp() subtractDisplayHeightCurrent: " + QString::number(subtractDisplayHeightCurrent_) + " >= " + QString::number(subtractDisplayHeightMin_))

  // Tell VirtualBox to change the display size
  args.clear();
  args.append("controlvm");
  args.append(pmInstance_->getConfig()->vmName);
  args.append("setvideomodehint");
  args.append(QString::number(screenWidth_));
  args.append(QString::number(screenHeight_));
  args.append("32");
  ILOG("WidgetRdpView::connectViaRdp() resize using command: vboxmanage " + args.join(" "));
  QProcess procResize;
  procResize.execute("vboxmanage", args);
  procResize.waitForFinished(3000);

#ifndef SKIP_FREERDP_CODE

  freeRDPwidget_->resize(screenWidth_, screenHeight_);
  freeRDPwidget_->setDesktopSize(screenWidth_, screenHeight_);
  freeRDPwidget_->connectToHost("localhost", rdpPort_);
  freeRDPwidget_->show();
  freeRDPwidget_->repaint();

#else
  freeRDPwidget_->setText("to connect run: xfreerdp /network:broadband /size:" + QString::number(screenWidth_) + "x" + QString::number(screenHeight_) + " -encryption /v:localhost:" + QString::number(rdpPort_));
#endif

  emit signalScreenResize( this );
  connectionEstablished_ = true;
}

void WidgetRdpView::disconnectRdpConnection()
{
  ILOG("WidgetRdpView::disconnectRdpConnection()...");
  freeRDPwidget_->disconnect();
  onDisconnecting_ = true;
  // in the disconnect event we will fire the timer timerResizeHappend_
}

void WidgetRdpView::resizeEvent (QResizeEvent * event)
{
  ILOG(QString( "WidgetRdpView::resizeEvent(): " ) + QString::number( this->width() ) + " x " + QString::number( this->height() ) );

#ifdef FAKE_REMOTEDISPLAY
  connectionEstablished_ = false;
#endif


  // The resize event is the start point for multiple connection attempts started via the timer timerResizeHappend_
  //   On failure each resizeVmDesktopAndConnectViaRdp() created a failing connection which triggers the
  //   signal disconnected which starts the timer again
  retryCount_ = 0;

  // We first have to disconnect before we resize the Vm-Desktop and connect again
  if (connectionEstablished_)
  {
    if (!onDisconnecting_)
    {
      disconnectRdpConnection(); // in the disconnect event we will fire the timer timerResizeHappend_
    }
  }
  else
  {
    // we use a timer which resets the time at each start to prevent continously fired resize-events
    timerResizeHappend_.start(100);
  }
  
}

void WidgetRdpView::slotDisconnected()
{
  connectionEstablished_ = false;
  onDisconnecting_ = false;

  retryCount_++;

  if (freeRDPwidget_)
  {
    disconnect(freeRDPwidget_,
            SIGNAL(disconnected()),
            this,
            SLOT(slotDisconnected()));

    loGrid_->removeWidget(freeRDPwidget_);
    delete (freeRDPwidget_);
    freeRDPwidget_ = 0;
  }

  if (inErrorMode_)
    return;

  // we want to reconnect
  ILOG("reconnect: create a new rdp widget");

  #ifndef SKIP_FREERDP_CODE
    freeRDPwidget_ = new RemoteDisplayWidget();
  #else
    freeRDPwidget_ = new QLabel();
    freeRDPwidget_->setText("FreeRDP disabled in this build");
  #endif

  loGrid_->addWidget(freeRDPwidget_, 0, 0, 1, 1);

  connect(freeRDPwidget_,
          SIGNAL(disconnected()),
          this,
          SLOT(slotDisconnected()));

  ILOG("WidgetRdpView: we start a very short timer to call resizeVmDesktopAndConnectViaRdp() in 10ms");
  timerResizeHappend_.start(10);
}

void WidgetRdpView::slotTimerResizeHappendFired()
{
  
  if (inErrorMode_)
    return;

  if (!connectionEstablished_)
  {
    // We have to increase the count of attempts to try to establish a new connection
    retryCount_++;

    if (retryCount_ > 5)
    {
      IERR("WidgetRdpView: aborting because of the detection of an rdp reconnection loop, user has to reopen this VmMask");

      // replace the RDP-Widget with an Label showing an error
      loGrid_->removeWidget(freeRDPwidget_);
      delete (freeRDPwidget_);
      freeRDPwidget_ = 0;
      lblErrorMessage_->setText("connection to VM-Mask lost, please close this tab and try again");
      loGrid_->addWidget(lblErrorMessage_, 0, 0, 1, 1, Qt::AlignHCenter);
      inErrorMode_ = true;
    }
    else
    {
      ILOG("WidgetRdpView: timer is ready - connectViaRdp()");
      resizeVmDesktopAndConnectViaRdp();
    }
  }
  else
  {
    if (!onDisconnecting_)
    {
      disconnectRdpConnection(); // in the disconnect event we will fire the timer timerResizeHappend_
    }
  }
}
