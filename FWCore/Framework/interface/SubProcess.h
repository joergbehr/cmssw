#ifndef FwCore_Framework_SubProcess_h
#define FWCore_Framework_SubProcess_h

#include "FWCore/Framework/interface/EventSetupProvider.h"
#include "FWCore/Framework/interface/OutputModule.h"
#include "FWCore/Framework/src/PrincipalCache.h"
#include "FWCore/Framework/interface/ScheduleItems.h"
#include "FWCore/Framework/interface/Schedule.h"
#include "FWCore/ServiceRegistry/interface/ServiceLegacy.h"
#include "FWCore/ServiceRegistry/interface/ServiceToken.h"
#include "FWCore/Utilities/interface/BranchType.h"

#include "boost/shared_ptr.hpp"

#include <map>
#include <memory>

namespace edm {
  class BranchIDListHelper;
  class EDLooperBase;
  class HistoryAppender;
  class IOVSyncValue;
  class ParameterSet;
  class ProductRegistry;
  namespace eventsetup {
    class EventSetupsController;
  }
  class SubProcess : public OutputModule {
  public:
    SubProcess(ParameterSet& parameterSet,
               ParameterSet const& topLevelParameterSet,
               boost::shared_ptr<ProductRegistry const> parentProductRegistry,
               boost::shared_ptr<BranchIDListHelper const> parentBranchIDListHelper,
               eventsetup::EventSetupsController& esController,
               ActivityRegistry& parentActReg,
               ServiceToken const& token,
               serviceregistry::ServiceLegacy iLegacy);

    virtual ~SubProcess();

    using OutputModule::doBeginJob;
    using OutputModule::doEndJob;
    using OutputModule::doBeginRun;
    using OutputModule::doEndRun;
    using OutputModule::doBeginLuminosityBlock;
    using OutputModule::doEndLuminosityBlock;
    using OutputModule::doEvent;

    void doEvent(EventPrincipal const& principal, IOVSyncValue const& ts);

    void doBeginRun(RunPrincipal const& principal, IOVSyncValue const& ts);

    void doEndRun(RunPrincipal const& principal, IOVSyncValue const& ts, bool cleaningUpAfterException);

    void doBeginLuminosityBlock(LuminosityBlockPrincipal const& principal, IOVSyncValue const& ts);

    void doEndLuminosityBlock(LuminosityBlockPrincipal const& principal, IOVSyncValue const& ts, bool cleaningUpAfterException);

    // Write the luminosity block
    void writeLumi(ProcessHistoryID const& parentPhID, int runNumber, int lumiNumber);

    void deleteLumiFromCache(ProcessHistoryID const& parentPhID, int runNumber, int lumiNumber);

    // Write the run
    void writeRun(ProcessHistoryID const& parentPhID, int runNumber);

    void deleteRunFromCache(ProcessHistoryID const& parentPhID, int runNumber);

    // Call closeFile() on all OutputModules.
    void closeOutputFiles() {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->closeOutputFiles();
      if(subProcess_.get()) subProcess_->closeOutputFiles();
    }

    // Call openNewFileIfNeeded() on all OutputModules
    void openNewOutputFilesIfNeeded() {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->openNewOutputFilesIfNeeded();
      if(subProcess_.get()) subProcess_->openNewOutputFilesIfNeeded();
    }

    // Call openFiles() on all OutputModules
    void openOutputFiles(FileBlock& fb) {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->openOutputFiles(fb);
      if(subProcess_.get()) subProcess_->openOutputFiles(fb);
    }

    // Call respondToOpenInputFile() on all Modules
    void respondToOpenInputFile(FileBlock const& fb);

    // Call respondToCloseInputFile() on all Modules
    void respondToCloseInputFile(FileBlock const& fb) {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->respondToCloseInputFile(fb);
      if(subProcess_.get()) subProcess_->respondToCloseInputFile(fb);
    }

    // Call respondToOpenOutputFiles() on all Modules
    void respondToOpenOutputFiles(FileBlock const& fb) {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->respondToOpenOutputFiles(fb);
      if(subProcess_.get()) subProcess_->respondToOpenOutputFiles(fb);
    }

    // Call respondToCloseOutputFiles() on all Modules
    void respondToCloseOutputFiles(FileBlock const& fb) {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->respondToCloseOutputFiles(fb);
      if(subProcess_.get()) subProcess_->respondToCloseOutputFiles(fb);
    }

    // Call shouldWeCloseFile() on all OutputModules.
    bool shouldWeCloseOutput() const {
      ServiceRegistry::Operate operate(serviceToken_);
      return schedule_->shouldWeCloseOutput() || (subProcess_.get() ? subProcess_->shouldWeCloseOutput() : false);
    }

    void preForkReleaseResources() {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->preForkReleaseResources();
      if(subProcess_.get()) subProcess_->preForkReleaseResources();
    }

    void postForkReacquireResources(unsigned int iChildIndex, unsigned int iNumberOfChildren) {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->postForkReacquireResources(iChildIndex, iNumberOfChildren);
      if(subProcess_.get()) subProcess_->postForkReacquireResources(iChildIndex, iNumberOfChildren);
    }

    /// Return a vector allowing const access to all the ModuleDescriptions for this SubProcess

    /// *** N.B. *** Ownership of the ModuleDescriptions is *not*
    /// *** passed to the caller. Do not call delete on these
    /// *** pointers!
    std::vector<ModuleDescription const*> getAllModuleDescriptions() const;

    /// Return the number of events this SubProcess has tried to process
    /// (inclues both successes and failures, including failures due
    /// to exceptions during processing).
    int totalEvents() const {
      return schedule_->totalEvents();
    }

    /// Return the number of events which have been passed by one or more trigger paths.
    int totalEventsPassed() const {
      ServiceRegistry::Operate operate(serviceToken_);
      return schedule_->totalEventsPassed();
    }

    /// Return the number of events that have not passed any trigger.
    /// (N.B. totalEventsFailed() + totalEventsPassed() == totalEvents()
    int totalEventsFailed() const {
      ServiceRegistry::Operate operate(serviceToken_);
      return schedule_->totalEventsFailed();
    }

    /// Turn end_paths "off" if "active" is false;
    /// Turn end_paths "on" if "active" is true.
    void enableEndPaths(bool active) {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->enableEndPaths(active);
      if(subProcess_.get()) subProcess_->enableEndPaths(active);
    }

    /// Return true if end_paths are active, and false if they are inactive.
    bool endPathsEnabled() const {
      ServiceRegistry::Operate operate(serviceToken_);
      return schedule_->endPathsEnabled();
    }

    /// Return the trigger report information on paths,
    /// modules-in-path, modules-in-endpath, and modules.
    void getTriggerReport(TriggerReport& rep) const {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->getTriggerReport(rep);
    }

    /// Return whether each output module has reached its maximum count.
    /// If there is a subprocess, get this information from the subprocess.
    bool terminate() const {
      ServiceRegistry::Operate operate(serviceToken_);
      return subProcess_.get() ? subProcess_->terminate() : schedule_->terminate();
    }

    ///  Clear all the counters in the trigger report.
    void clearCounters() {
      ServiceRegistry::Operate operate(serviceToken_);
      schedule_->clearCounters();
      if(subProcess_.get()) subProcess_->clearCounters();
    }

  private:
    struct ESInfo {
      ESInfo(IOVSyncValue const& ts, eventsetup::EventSetupProvider& esp);
      IOVSyncValue const& ts_;
      EventSetup const& es_;
    };

    virtual void beginJob();
    virtual void endJob();
    virtual void write(EventPrincipal const& e);
    virtual void beginRun(RunPrincipal const& r);
    virtual void endRun(RunPrincipal const& r);
    virtual void beginLuminosityBlock(LuminosityBlockPrincipal const& lb);
    virtual void endLuminosityBlock(LuminosityBlockPrincipal const& lb);
    virtual void writeRun(RunPrincipal const&) { throw 0; }
    virtual void writeLuminosityBlock(LuminosityBlockPrincipal const&) { throw 0; }

    void propagateProducts(BranchType type, Principal const& parentPrincipal, Principal& principal) const;
    void fixBranchIDListsForEDAliases(std::map<BranchID::value_type, BranchID::value_type> const& droppedBranchIDToKeptBranchID);

    ServiceToken                                  serviceToken_;
    boost::shared_ptr<ProductRegistry const>      parentPreg_;
    boost::shared_ptr<ProductRegistry const>	  preg_;
    boost::shared_ptr<BranchIDListHelper>         branchIDListHelper_;
    std::unique_ptr<ActionTable const>            act_table_;
    boost::shared_ptr<ProcessConfiguration const> processConfiguration_;
    PrincipalCache                                principalCache_;
    boost::shared_ptr<eventsetup::EventSetupProvider> esp_;
    std::auto_ptr<Schedule>                       schedule_;
    std::map<ProcessHistoryID, ProcessHistoryID>  parentToChildPhID_;
    std::unique_ptr<HistoryAppender>              historyAppender_;
    std::auto_ptr<ESInfo>                         esInfo_;
    std::auto_ptr<SubProcess>                     subProcess_;
    bool                                          cleaningUpAfterException_;
    std::unique_ptr<ParameterSet>                 processParameterSet_;
  };

  // free function
  std::auto_ptr<ParameterSet> popSubProcessParameterSet(ParameterSet& parameterSet);
}
#endif
