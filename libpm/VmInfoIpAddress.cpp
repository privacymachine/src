/*==============================================================================
        Copyright (c) 2013-2017 by the Developers of PrivacyMachine.eu
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

#include "VmInfoIpAddress.h"

VmInfoIpAddress::VmInfoIpAddress(QStringList parVmMaskIpAddressProviders,
                                 QString parVmMaskFullName,
                                 QString parVmMaskBrowser,
                                 int parSshPort)
{
  vmMaskIpAddressProviders_ = parVmMaskIpAddressProviders; // already shuffled
  vmMaskFullName_ = parVmMaskFullName;
  vmMaskBrowser_ = parVmMaskBrowser;
  sshPort_ = parSshPort;

  ipAddressUpdated_ = false;
  ipAddress_ = "";
  exec_ = QSharedPointer<PmCommandExec>(new PmCommandExec());
  nProvidersTried_ = 0;
}

VmInfoIpAddress::~VmInfoIpAddress()
{
  disconnect( &(*exec_),
              SIGNAL( signalFinished( ePmCommandResult ) ),
              0,
              0 );

  exec_->disconnectSignalsAndSlots();
}

QString VmInfoIpAddress::getIpAddress()
{
  return ipAddress_;
}

void VmInfoIpAddress::initialize()
{
  // TODO: bernhard: test the new qt5 syntax which checks errors at compile time!
  /*
  connect( &(*exec_),
           SIGNAL( signalFinished( ePmCommandResult ) ),
           this,
           SLOT( slotExecFinished( ePmCommandResult ) ) );
*/
  QObject::connect(exec_.data(), &PmCommandExec::signalFinished,
                   this, &VmInfoIpAddress::slotExecFinished);

  exec_->connectSignalsAndSlots();
}

void VmInfoIpAddress::slotExecFinished( ePmCommandResult result )
{
  QRegExp regexIpAddress( "[0-9]{1,3}\\.[0-9]{1,3}.[0-9]{1,3}\\.[0-9]{1,3}" );
  QString lastStdOut = exec_->getLastCommandLastLineStdOut();
  QList< PmCommand* > commands;
  PmCommand *pCurrentCommand = NULL;

  /*
  QString userConfigDir;
  if (!getOrCreateUserConfigDir(userConfigDir))
  {
    IWARN( "VmInfoIpAddress::slotExecFinished() failed to get the user config dir");
    return;
  }
  */

//  pCurrentCommand = GetPmCommandForScp2Host(constRootUser,constVmIp,QString::number( pVmMaskInstance_->getConfig()->sshPort ),constRootPw,
//                                            userConfigDir+"/logs/vmMask_"+pVmMaskInstance_->getConfig()->name+"_vpnLog.txt",
//                                            "/var/log/openvpn.log");
//  pCurrentCommand->setDescription("Copy vpn logs of VM-Mask "+pVmMaskInstance_->getConfig()->name);
//  pCurrentCommand->setCosts(100);
//  commands.append( pCurrentCommand );

  pCurrentCommand = NULL;
  if( result == success  &&  regexIpAddress.exactMatch( lastStdOut.trimmed() ) )
  {
    ipAddress_ = exec_->getLastCommandLastLineStdOut();
    emit signalUpdateIpSuccess();
  }
  else if( result != aborted )
  {
    if( nProvidersTried_ != 0 )
    {
      pCurrentCommand = createPmCommandNextIpAddressProvider();
      if( pCurrentCommand != NULL )
      {
        commands.append( pCurrentCommand );
        exec_->setCommands( commands );
        exec_->start();
      }
    }
    else
    {    
      IWARN( "Failed to determine external IP address for VM-Mask '" + vmMaskFullName_ + "'." );
    }
  }
}


void VmInfoIpAddress::startPollingExternalIp()
{
  // This resets the count also when we restart the same VM-Mask in a PM that might be open for a long time already.
  nProvidersTried_ = 0;
  
  // Only start if not running yet
  if( exec_->isRunning() )
  {
    return;
  }

  // For the Tor® network, we do not check for the external IP, as we would not know the current exit node anyway.
  // We check against mapped browser as this will be unique as opposed to the [TOR] network connection name, which can
  // be randomly chosen by the user.
  if( vmMaskBrowser_ == "torbrowser" )
  {
    ipAddress_ = "Tor";
    emit signalUpdateIpSuccess();
    return;
  }
  
  QList< PmCommand* > commands;

  PmCommand *pCurrentCommand = GetPmCommandForSshCmdVmMaskInstance(sshPort_, QString( "systemctl status " ) + vmMaskBrowser_);
  pCurrentCommand->setType( pollingShellCommand );
  pCurrentCommand->setTimeoutMilliseconds( 5000 );
  pCurrentCommand->setRetries( 5 );
  pCurrentCommand->setDescription( "Waiting until browser inside VM-Mask " + vmMaskFullName_ + "' is started." );
  pCurrentCommand->setExecutionCosts( 100 );
  commands.append( pCurrentCommand );

  pCurrentCommand = createPmCommandNextIpAddressProvider();
  if( pCurrentCommand != NULL )
  {
    commands.append( pCurrentCommand );

    exec_->setCommands( commands );
    exec_->start(); 
  }
  else
  {
    IWARN( "Failed to create command for retreiving external IP address for VM-Mask " 
      + vmMaskFullName_ + "'. Aborting attempt to retreive external IP address." );
  }
}

void VmInfoIpAddress::abort()
{
  exec_->abort();
}

PmCommand *VmInfoIpAddress::createPmCommandNextIpAddressProvider()
{
  QString curProvider = vmMaskIpAddressProviders_[ ipAddressProviderPermutation_[ nProvidersTried_ ] ];
  PmCommand *pCommand = GetPmCommandForSshCmdVmMaskInstance(sshPort_, QString( "curl -s " ) + curProvider);
  ++nProvidersTried_;
  nProvidersTried_ %= ipAddressProviderPermutation_.size();

  pCommand->setTimeoutMilliseconds( 5000 );
  pCommand->setDescription( "Retreiving external IP address for VM-Mask " + vmMaskFullName_ + "'." );
  pCommand->setExecutionCosts( 100 );

  return pCommand;
}

QString VmInfoIpAddress::toStatus()
{
  return ( ipAddress_ != "" ) ? tr( "External IP" ) + ": " + ipAddress_ : "";
}
