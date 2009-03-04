//----------------------------------
// Includes
//----------------------------------
#include "MantidQtAPI/DialogManager.h"
#include "MantidQtAPI/DialogFactory.h"
#include "MantidQtAPI/AlgorithmDialog.h"
#include "MantidQtAPI/GenericDialog.h"

#include "MantidKernel/Logger.h"
#include "MantidKernel/LibraryManager.h"
#include "MantidKernel/ConfigService.h"
#include "MantidAPI/Algorithm.h"
#include "MantidKernel/Exception.h"

using namespace MantidQt::API;

//Initialize the logger
Mantid::Kernel::Logger & DialogManagerImpl::g_log = Mantid::Kernel::Logger::get("DialogManager");

//----------------------------------
// Public member functions
//----------------------------------
/**
 * Return a specialized dialog for the given algorithm. If none exists then the default is returned
 * @param alg A pointer to the algorithm
 * @param parent An optional parent widget
 * @param forScript A boolean indicating if this dialog is to be use for from a script or not
 * @param msg An optional message string to be placed at the top of the dialog
 * @returns An AlgorithmDialog object
 */
AlgorithmDialog* DialogManagerImpl::createDialog(Mantid::API::Algorithm* alg, QWidget* parent,
						 bool forScript, const QString & msg)
{
  AlgorithmDialog* dlg;
  if( DialogFactory::Instance().exists(alg->name() + "Dialog") )
  {
    g_log.debug() << "Creating a specialised dialog for " << alg->name() << std::endl;
    dlg = DialogFactory::Instance().createUnwrapped(alg->name() + "Dialog");
    }
  else
  {
    dlg = new GenericDialog(parent);
    g_log.debug() << "No specialised dialog exists for the " << alg->name() 
		  << " algorithm: a generic one has been created" << std::endl;
  }
  
  dlg->setParent(parent);
  dlg->setAlgorithm(alg);
  dlg->isForScript(forScript);
  dlg->setOptionalMessage(msg);
  dlg->initializeLayout();
  return dlg;  
}

//----------------------------------
// Private member functions
//----------------------------------
/// Default Constructor
DialogManagerImpl::DialogManagerImpl()
{

  //Attempt to load libraries that may contain custom dialog classes
  //First get the path from the configuration manager
  std::string libpath = Mantid::Kernel::ConfigService::Instance().getString("plugins.directory");
  if( !libpath.empty() )
  {
    int loaded = Mantid::Kernel::LibraryManager::Instance().OpenAllLibraries(libpath);
    if( loaded == 0 )
    {
      g_log.information() << "A path has been specified for the custom algorithm dialogs but no libraries could be loaded. "
			  << "Please check that the 'plugins.directory' variable in the Mantid.properties file points to "
			  << "the correct location."
			  << std::endl;
    }
  }

}

/// Destructor
DialogManagerImpl::~DialogManagerImpl()
{
}

//---------------------------------------------------------------------------------------------
// Wrapper class member definitions
//---------------------------------------------------------------------------------------------
/**
  * Constructor
  */
// DialogManagerWrapper::DialogManagerWrapper()
// {
  // MantidQt::API::DialogManager::Instance();
// }

// /**
  // * The number of dialogs available
  // */
// int DialogManagerWrapper::numberOfDialogsAvailable() const
// {
  // return static_cast<int>(MantidQt::API::DialogFactory::Instance().getKeys().size());
// }

