## @file
# build a platform or a module
#
#  Copyright (c) 2014, Hewlett-Packard Development Company, L.P.<BR>
#  Copyright (c) 2007 - 2019, Intel Corporation. All rights reserved.<BR>
#  Copyright (c) 2018, Hewlett Packard Enterprise Development, L.P.<BR>
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#

##
# Import Modules
#
from __future__ import print_function
from __future__ import absolute_import
import os.path as path
import sys
import os
import re
import glob
import time
import platform
import traceback
import multiprocessing
from threading import Thread,Event,BoundedSemaphore
import threading
from subprocess import Popen,PIPE
from collections import OrderedDict, defaultdict
from Common.buildoptions import BuildOption,BuildTarget
from AutoGen.PlatformAutoGen import PlatformAutoGen
from AutoGen.ModuleAutoGen import ModuleAutoGen
from AutoGen.WorkspaceAutoGen import WorkspaceAutoGen
from AutoGen.AutoGenWorker import AutoGenWorkerInProcess,AutoGenManager,\
    LogAgent
from AutoGen import GenMake
from Common import Misc as Utils

from Common.TargetTxtClassObject import TargetTxt
from Common.ToolDefClassObject import ToolDef
from Common.Misc import PathClass,SaveFileOnChange,RemoveDirectory
from Common.StringUtils import NormPath
from Common.MultipleWorkspace import MultipleWorkspace as mws
from Common.BuildToolError import *
from Common.DataType import *
import Common.EdkLogger as EdkLogger

from Workspace.WorkspaceDatabase import BuildDB

from BuildReport import BuildReport
from GenPatchPcdTable.GenPatchPcdTable import PeImageClass,parsePcdInfoFromMapFile
from PatchPcdValue.PatchPcdValue import PatchBinaryFile

import Common.GlobalData as GlobalData
from GenFds.GenFds import GenFds, GenFdsApi
import multiprocessing as mp
from multiprocessing import Manager
from AutoGen.DataPipe import MemoryDataPipe
from AutoGen.ModuleAutoGenHelper import WorkSpaceInfo, PlatformInfo
from GenFds.FdfParser import FdfParser


## standard targets of build command
gSupportedTarget = ['all', 'genc', 'genmake', 'modules', 'libraries', 'fds', 'clean', 'cleanall', 'cleanlib', 'run']

## build configuration file
gBuildConfiguration = "target.txt"
gToolsDefinition = "tools_def.txt"

TemporaryTablePattern = re.compile(r'^_\d+_\d+_[a-fA-F0-9]+$')
TmpTableDict = {}

## Check environment PATH variable to make sure the specified tool is found
#
#   If the tool is found in the PATH, then True is returned
#   Otherwise, False is returned
#
def IsToolInPath(tool):
    if 'PATHEXT' in os.environ:
        extns = os.environ['PATHEXT'].split(os.path.pathsep)
    else:
        extns = ('',)
    for pathDir in os.environ['PATH'].split(os.path.pathsep):
        for ext in extns:
            if os.path.exists(os.path.join(pathDir, tool + ext)):
                return True
    return False

## Check environment variables
#
#  Check environment variables that must be set for build. Currently they are
#
#   WORKSPACE           The directory all packages/platforms start from
#   EDK_TOOLS_PATH      The directory contains all tools needed by the build
#   PATH                $(EDK_TOOLS_PATH)/Bin/<sys> must be set in PATH
#
#   If any of above environment variable is not set or has error, the build
#   will be broken.
#
def CheckEnvVariable():
    # check WORKSPACE
    if "WORKSPACE" not in os.environ:
        EdkLogger.error("build", ATTRIBUTE_NOT_AVAILABLE, "Environment variable not found",
                        ExtraData="WORKSPACE")

    WorkspaceDir = os.path.normcase(os.path.normpath(os.environ["WORKSPACE"]))
    if not os.path.exists(WorkspaceDir):
        EdkLogger.error("build", FILE_NOT_FOUND, "WORKSPACE doesn't exist", ExtraData=WorkspaceDir)
    elif ' ' in WorkspaceDir:
        EdkLogger.error("build", FORMAT_NOT_SUPPORTED, "No space is allowed in WORKSPACE path",
                        ExtraData=WorkspaceDir)
    os.environ["WORKSPACE"] = WorkspaceDir

    # set multiple workspace
    PackagesPath = os.getenv("PACKAGES_PATH")
    mws.setWs(WorkspaceDir, PackagesPath)
    if mws.PACKAGES_PATH:
        for Path in mws.PACKAGES_PATH:
            if not os.path.exists(Path):
                EdkLogger.error("build", FILE_NOT_FOUND, "One Path in PACKAGES_PATH doesn't exist", ExtraData=Path)
            elif ' ' in Path:
                EdkLogger.error("build", FORMAT_NOT_SUPPORTED, "No space is allowed in PACKAGES_PATH", ExtraData=Path)


    os.environ["EDK_TOOLS_PATH"] = os.path.normcase(os.environ["EDK_TOOLS_PATH"])

    # check EDK_TOOLS_PATH
    if "EDK_TOOLS_PATH" not in os.environ:
        EdkLogger.error("build", ATTRIBUTE_NOT_AVAILABLE, "Environment variable not found",
                        ExtraData="EDK_TOOLS_PATH")

    # check PATH
    if "PATH" not in os.environ:
        EdkLogger.error("build", ATTRIBUTE_NOT_AVAILABLE, "Environment variable not found",
                        ExtraData="PATH")

    GlobalData.gWorkspace = WorkspaceDir

    GlobalData.gGlobalDefines["WORKSPACE"]  = WorkspaceDir
    GlobalData.gGlobalDefines["EDK_TOOLS_PATH"] = os.environ["EDK_TOOLS_PATH"]

## Get normalized file path
#
# Convert the path to be local format, and remove the WORKSPACE path at the
# beginning if the file path is given in full path.
#
# @param  FilePath      File path to be normalized
# @param  Workspace     Workspace path which the FilePath will be checked against
#
# @retval string        The normalized file path
#
def NormFile(FilePath, Workspace):
    # check if the path is absolute or relative
    if os.path.isabs(FilePath):
        FileFullPath = os.path.normpath(FilePath)
    else:
        FileFullPath = os.path.normpath(mws.join(Workspace, FilePath))
        Workspace = mws.getWs(Workspace, FilePath)

    # check if the file path exists or not
    if not os.path.isfile(FileFullPath):
        EdkLogger.error("build", FILE_NOT_FOUND, ExtraData="\t%s (Please give file in absolute path or relative to WORKSPACE)" % FileFullPath)

    # remove workspace directory from the beginning part of the file path
    if Workspace[-1] in ["\\", "/"]:
        return FileFullPath[len(Workspace):]
    else:
        return FileFullPath[(len(Workspace) + 1):]

## Get the output of an external program
#
# This is the entrance method of thread reading output of an external program and
# putting them in STDOUT/STDERR of current program.
#
# @param  From      The stream message read from
# @param  To        The stream message put on
# @param  ExitFlag  The flag used to indicate stopping reading
#
def ReadMessage(From, To, ExitFlag):
    while True:
        # read one line a time
        Line = From.readline()
        # empty string means "end"
        if Line is not None and Line != b"":
            To(Line.rstrip().decode(encoding='utf-8', errors='ignore'))
        else:
            break
        if ExitFlag.isSet():
            break

## Launch an external program
#
# This method will call subprocess.Popen to execute an external program with
# given options in specified directory. Because of the dead-lock issue during
# redirecting output of the external program, threads are used to to do the
# redirection work.
#
# @param  Command               A list or string containing the call of the program
# @param  WorkingDir            The directory in which the program will be running
#
def LaunchCommand(Command, WorkingDir):
    BeginTime = time.time()
    # if working directory doesn't exist, Popen() will raise an exception
    if not os.path.isdir(WorkingDir):
        EdkLogger.error("build", FILE_NOT_FOUND, ExtraData=WorkingDir)

    # Command is used as the first Argument in following Popen().
    # It could be a string or sequence. We find that if command is a string in following Popen(),
    # ubuntu may fail with an error message that the command is not found.
    # So here we may need convert command from string to list instance.
    if platform.system() != 'Windows':
        if not isinstance(Command, list):
            Command = Command.split()
        Command = ' '.join(Command)

    Proc = None
    EndOfProcedure = None
    try:
        # launch the command
        Proc = Popen(Command, stdout=PIPE, stderr=PIPE, env=os.environ, cwd=WorkingDir, bufsize=-1, shell=True)

        # launch two threads to read the STDOUT and STDERR
        EndOfProcedure = Event()
        EndOfProcedure.clear()
        if Proc.stdout:
            StdOutThread = Thread(target=ReadMessage, args=(Proc.stdout, EdkLogger.info, EndOfProcedure))
            StdOutThread.setName("STDOUT-Redirector")
            StdOutThread.setDaemon(False)
            StdOutThread.start()

        if Proc.stderr:
            StdErrThread = Thread(target=ReadMessage, args=(Proc.stderr, EdkLogger.quiet, EndOfProcedure))
            StdErrThread.setName("STDERR-Redirector")
            StdErrThread.setDaemon(False)
            StdErrThread.start()

        # waiting for program exit
        Proc.wait()
    except: # in case of aborting
        # terminate the threads redirecting the program output
        EdkLogger.quiet("(Python %s on %s) " % (platform.python_version(), sys.platform) + traceback.format_exc())
        if EndOfProcedure is not None:
            EndOfProcedure.set()
        if Proc is None:
            if not isinstance(Command, type("")):
                Command = " ".join(Command)
            EdkLogger.error("build", COMMAND_FAILURE, "Failed to start command", ExtraData="%s [%s]" % (Command, WorkingDir))

    if Proc.stdout:
        StdOutThread.join()
    if Proc.stderr:
        StdErrThread.join()

    # check the return code of the program
    if Proc.returncode != 0:
        if not isinstance(Command, type("")):
            Command = " ".join(Command)
        # print out the Response file and its content when make failure
        RespFile = os.path.join(WorkingDir, 'OUTPUT', 'respfilelist.txt')
        if os.path.isfile(RespFile):
            f = open(RespFile)
            RespContent = f.read()
            f.close()
            EdkLogger.info(RespContent)

        EdkLogger.error("build", COMMAND_FAILURE, ExtraData="%s [%s]" % (Command, WorkingDir))
    return "%dms" % (int(round((time.time() - BeginTime) * 1000)))

## The smallest unit that can be built in multi-thread build mode
#
# This is the base class of build unit. The "Obj" parameter must provide
# __str__(), __eq__() and __hash__() methods. Otherwise there could be build units
# missing build.
#
# Currently the "Obj" should be only ModuleAutoGen or PlatformAutoGen objects.
#
class BuildUnit:
    ## The constructor
    #
    #   @param  self        The object pointer
    #   @param  Obj         The object the build is working on
    #   @param  Target      The build target name, one of gSupportedTarget
    #   @param  Dependency  The BuildUnit(s) which must be completed in advance
    #   @param  WorkingDir  The directory build command starts in
    #
    def __init__(self, Obj, BuildCommand, Target, Dependency, WorkingDir="."):
        self.BuildObject = Obj
        self.Dependency = Dependency
        self.WorkingDir = WorkingDir
        self.Target = Target
        self.BuildCommand = BuildCommand
        if not BuildCommand:
            EdkLogger.error("build", OPTION_MISSING,
                            "No build command found for this module. "
                            "Please check your setting of %s_%s_%s_MAKE_PATH in Conf/tools_def.txt file." %
                                (Obj.BuildTarget, Obj.ToolChain, Obj.Arch),
                            ExtraData=str(Obj))


    ## str() method
    #
    #   It just returns the string representation of self.BuildObject
    #
    #   @param  self        The object pointer
    #
    def __str__(self):
        return str(self.BuildObject)

    ## "==" operator method
    #
    #   It just compares self.BuildObject with "Other". So self.BuildObject must
    #   provide its own __eq__() method.
    #
    #   @param  self        The object pointer
    #   @param  Other       The other BuildUnit object compared to
    #
    def __eq__(self, Other):
        return Other and self.BuildObject == Other.BuildObject \
                and Other.BuildObject \
                and self.BuildObject.Arch == Other.BuildObject.Arch

    ## hash() method
    #
    #   It just returns the hash value of self.BuildObject which must be hashable.
    #
    #   @param  self        The object pointer
    #
    def __hash__(self):
        return hash(self.BuildObject) + hash(self.BuildObject.Arch)

    def __repr__(self):
        return repr(self.BuildObject)

## The smallest module unit that can be built by nmake/make command in multi-thread build mode
#
# This class is for module build by nmake/make build system. The "Obj" parameter
# must provide __str__(), __eq__() and __hash__() methods. Otherwise there could
# be make units missing build.
#
# Currently the "Obj" should be only ModuleAutoGen object.
#
class ModuleMakeUnit(BuildUnit):
    ## The constructor
    #
    #   @param  self        The object pointer
    #   @param  Obj         The ModuleAutoGen object the build is working on
    #   @param  Target      The build target name, one of gSupportedTarget
    #
    def __init__(self, Obj, BuildCommand,Target):
        Dependency = [ModuleMakeUnit(La, BuildCommand,Target) for La in Obj.LibraryAutoGenList]
        BuildUnit.__init__(self, Obj, BuildCommand, Target, Dependency, Obj.MakeFileDir)
        if Target in [None, "", "all"]:
            self.Target = "tbuild"

## The smallest platform unit that can be built by nmake/make command in multi-thread build mode
#
# This class is for platform build by nmake/make build system. The "Obj" parameter
# must provide __str__(), __eq__() and __hash__() methods. Otherwise there could
# be make units missing build.
#
# Currently the "Obj" should be only PlatformAutoGen object.
#
class PlatformMakeUnit(BuildUnit):
    ## The constructor
    #
    #   @param  self        The object pointer
    #   @param  Obj         The PlatformAutoGen object the build is working on
    #   @param  Target      The build target name, one of gSupportedTarget
    #
    def __init__(self, Obj, BuildCommand, Target):
        Dependency = [ModuleMakeUnit(Lib, BuildCommand, Target) for Lib in self.BuildObject.LibraryAutoGenList]
        Dependency.extend([ModuleMakeUnit(Mod, BuildCommand,Target) for Mod in self.BuildObject.ModuleAutoGenList])
        BuildUnit.__init__(self, Obj, BuildCommand, Target, Dependency, Obj.MakeFileDir)

## The class representing the task of a module build or platform build
#
# This class manages the build tasks in multi-thread build mode. Its jobs include
# scheduling thread running, catching thread error, monitor the thread status, etc.
#
class BuildTask:
    # queue for tasks waiting for schedule
    _PendingQueue = OrderedDict()
    _PendingQueueLock = threading.Lock()

    # queue for tasks ready for running
    _ReadyQueue = OrderedDict()
    _ReadyQueueLock = threading.Lock()

    # queue for run tasks
    _RunningQueue = OrderedDict()
    _RunningQueueLock = threading.Lock()

    # queue containing all build tasks, in case duplicate build
    _TaskQueue = OrderedDict()

    # flag indicating error occurs in a running thread
    _ErrorFlag = threading.Event()
    _ErrorFlag.clear()
    _ErrorMessage = ""

    # BoundedSemaphore object used to control the number of running threads
    _Thread = None

    # flag indicating if the scheduler is started or not
    _SchedulerStopped = threading.Event()
    _SchedulerStopped.set()

    ## Start the task scheduler thread
    #
    #   @param  MaxThreadNumber     The maximum thread number
    #   @param  ExitFlag            Flag used to end the scheduler
    #
    @staticmethod
    def StartScheduler(MaxThreadNumber, ExitFlag):
        SchedulerThread = Thread(target=BuildTask.Scheduler, args=(MaxThreadNumber, ExitFlag))
        SchedulerThread.setName("Build-Task-Scheduler")
        SchedulerThread.setDaemon(False)
        SchedulerThread.start()
        # wait for the scheduler to be started, especially useful in Linux
        while not BuildTask.IsOnGoing():
            time.sleep(0.01)

    ## Scheduler method
    #
    #   @param  MaxThreadNumber     The maximum thread number
    #   @param  ExitFlag            Flag used to end the scheduler
    #
    @staticmethod
    def Scheduler(MaxThreadNumber, ExitFlag):
        BuildTask._SchedulerStopped.clear()
        try:
            # use BoundedSemaphore to control the maximum running threads
            BuildTask._Thread = BoundedSemaphore(MaxThreadNumber)
            #
            # scheduling loop, which will exits when no pending/ready task and
            # indicated to do so, or there's error in running thread
            #
            while (len(BuildTask._PendingQueue) > 0 or len(BuildTask._ReadyQueue) > 0 \
                   or not ExitFlag.isSet()) and not BuildTask._ErrorFlag.isSet():
                EdkLogger.debug(EdkLogger.DEBUG_8, "Pending Queue (%d), Ready Queue (%d)"
                                % (len(BuildTask._PendingQueue), len(BuildTask._ReadyQueue)))

                # get all pending tasks
                BuildTask._PendingQueueLock.acquire()
                BuildObjectList = list(BuildTask._PendingQueue.keys())
                #
                # check if their dependency is resolved, and if true, move them
                # into ready queue
                #
                for BuildObject in BuildObjectList:
                    Bt = BuildTask._PendingQueue[BuildObject]
                    if Bt.IsReady():
                        BuildTask._ReadyQueue[BuildObject] = BuildTask._PendingQueue.pop(BuildObject)
                BuildTask._PendingQueueLock.release()

                # launch build thread until the maximum number of threads is reached
                while not BuildTask._ErrorFlag.isSet():
                    # empty ready queue, do nothing further
                    if len(BuildTask._ReadyQueue) == 0:
                        break

                    # wait for active thread(s) exit
                    BuildTask._Thread.acquire(True)

                    # start a new build thread
                    Bo, Bt = BuildTask._ReadyQueue.popitem()

                    # move into running queue
                    BuildTask._RunningQueueLock.acquire()
                    BuildTask._RunningQueue[Bo] = Bt
                    BuildTask._RunningQueueLock.release()

                    Bt.Start()
                    # avoid tense loop
                    time.sleep(0.01)

                # avoid tense loop
                time.sleep(0.01)

            # wait for all running threads exit
            if BuildTask._ErrorFlag.isSet():
                EdkLogger.quiet("\nWaiting for all build threads exit...")
            # while not BuildTask._ErrorFlag.isSet() and \
            while len(BuildTask._RunningQueue) > 0:
                EdkLogger.verbose("Waiting for thread ending...(%d)" % len(BuildTask._RunningQueue))
                EdkLogger.debug(EdkLogger.DEBUG_8, "Threads [%s]" % ", ".join(Th.getName() for Th in threading.enumerate()))
                # avoid tense loop
                time.sleep(0.1)
        except BaseException as X:
            #
            # TRICK: hide the output of threads left running, so that the user can
            #        catch the error message easily
            #
            EdkLogger.SetLevel(EdkLogger.ERROR)
            BuildTask._ErrorFlag.set()
            BuildTask._ErrorMessage = "build thread scheduler error\n\t%s" % str(X)

        BuildTask._PendingQueue.clear()
        BuildTask._ReadyQueue.clear()
        BuildTask._RunningQueue.clear()
        BuildTask._TaskQueue.clear()
        BuildTask._SchedulerStopped.set()

    ## Wait for all running method exit
    #
    @staticmethod
    def WaitForComplete():
        BuildTask._SchedulerStopped.wait()

    ## Check if the scheduler is running or not
    #
    @staticmethod
    def IsOnGoing():
        return not BuildTask._SchedulerStopped.isSet()

    ## Abort the build
    @staticmethod
    def Abort():
        if BuildTask.IsOnGoing():
            BuildTask._ErrorFlag.set()
            BuildTask.WaitForComplete()

    ## Check if there's error in running thread
    #
    #   Since the main thread cannot catch exceptions in other thread, we have to
    #   use threading.Event to communicate this formation to main thread.
    #
    @staticmethod
    def HasError():
        return BuildTask._ErrorFlag.isSet()

    ## Get error message in running thread
    #
    #   Since the main thread cannot catch exceptions in other thread, we have to
    #   use a static variable to communicate this message to main thread.
    #
    @staticmethod
    def GetErrorMessage():
        return BuildTask._ErrorMessage

    ## Factory method to create a BuildTask object
    #
    #   This method will check if a module is building or has been built. And if
    #   true, just return the associated BuildTask object in the _TaskQueue. If
    #   not, create and return a new BuildTask object. The new BuildTask object
    #   will be appended to the _PendingQueue for scheduling later.
    #
    #   @param  BuildItem       A BuildUnit object representing a build object
    #   @param  Dependency      The dependent build object of BuildItem
    #
    @staticmethod
    def New(BuildItem, Dependency=None):
        if BuildItem in BuildTask._TaskQueue:
            Bt = BuildTask._TaskQueue[BuildItem]
            return Bt

        Bt = BuildTask()
        Bt._Init(BuildItem, Dependency)
        BuildTask._TaskQueue[BuildItem] = Bt

        BuildTask._PendingQueueLock.acquire()
        BuildTask._PendingQueue[BuildItem] = Bt
        BuildTask._PendingQueueLock.release()

        return Bt

    ## The real constructor of BuildTask
    #
    #   @param  BuildItem       A BuildUnit object representing a build object
    #   @param  Dependency      The dependent build object of BuildItem
    #
    def _Init(self, BuildItem, Dependency=None):
        self.BuildItem = BuildItem

        self.DependencyList = []
        if Dependency is None:
            Dependency = BuildItem.Dependency
        else:
            Dependency.extend(BuildItem.Dependency)
        self.AddDependency(Dependency)
        # flag indicating build completes, used to avoid unnecessary re-build
        self.CompleteFlag = False

    ## Check if all dependent build tasks are completed or not
    #
    def IsReady(self):
        ReadyFlag = True
        for Dep in self.DependencyList:
            if Dep.CompleteFlag == True:
                continue
            ReadyFlag = False
            break

        return ReadyFlag

    ## Add dependent build task
    #
    #   @param  Dependency      The list of dependent build objects
    #
    def AddDependency(self, Dependency):
        for Dep in Dependency:
            if not Dep.BuildObject.IsBinaryModule and not Dep.BuildObject.CanSkipbyCache(GlobalData.gCacheIR):
                self.DependencyList.append(BuildTask.New(Dep))    # BuildTask list

    ## The thread wrapper of LaunchCommand function
    #
    # @param  Command               A list or string contains the call of the command
    # @param  WorkingDir            The directory in which the program will be running
    #
    def _CommandThread(self, Command, WorkingDir):
        try:
            self.BuildItem.BuildObject.BuildTime = LaunchCommand(Command, WorkingDir)
            self.CompleteFlag = True

            # Run hash operation post dependency, to account for libs
            if GlobalData.gUseHashCache and self.BuildItem.BuildObject.IsLibrary:
                HashFile = path.join(self.BuildItem.BuildObject.BuildDir, self.BuildItem.BuildObject.Name + ".hash")
                SaveFileOnChange(HashFile, self.BuildItem.BuildObject.GenModuleHash(), True)
        except:
            #
            # TRICK: hide the output of threads left running, so that the user can
            #        catch the error message easily
            #
            if not BuildTask._ErrorFlag.isSet():
                GlobalData.gBuildingModule = "%s [%s, %s, %s]" % (str(self.BuildItem.BuildObject),
                                                                  self.BuildItem.BuildObject.Arch,
                                                                  self.BuildItem.BuildObject.ToolChain,
                                                                  self.BuildItem.BuildObject.BuildTarget
                                                                 )
            EdkLogger.SetLevel(EdkLogger.ERROR)
            BuildTask._ErrorFlag.set()
            BuildTask._ErrorMessage = "%s broken\n    %s [%s]" % \
                                      (threading.currentThread().getName(), Command, WorkingDir)

        # Set the value used by hash invalidation flow in GlobalData.gModuleBuildTracking to 'SUCCESS'
        # If Module or Lib is being tracked, it did not fail header check test, and built successfully
        if (self.BuildItem.BuildObject in GlobalData.gModuleBuildTracking and
           GlobalData.gModuleBuildTracking[self.BuildItem.BuildObject] != 'FAIL_METAFILE' and
           not BuildTask._ErrorFlag.isSet()
           ):
            GlobalData.gModuleBuildTracking[self.BuildItem.BuildObject] = 'SUCCESS'

        # indicate there's a thread is available for another build task
        BuildTask._RunningQueueLock.acquire()
        BuildTask._RunningQueue.pop(self.BuildItem)
        BuildTask._RunningQueueLock.release()
        BuildTask._Thread.release()

    ## Start build task thread
    #
    def Start(self):
        EdkLogger.quiet("Building ... %s" % repr(self.BuildItem))
        Command = self.BuildItem.BuildCommand + [self.BuildItem.Target]
        self.BuildTread = Thread(target=self._CommandThread, args=(Command, self.BuildItem.WorkingDir))
        self.BuildTread.setName("build thread")
        self.BuildTread.setDaemon(False)
        self.BuildTread.start()

## The class contains the information related to EFI image
#
class PeImageInfo():
    ## Constructor
    #
    # Constructor will load all required image information.
    #
    #   @param  BaseName          The full file path of image.
    #   @param  Guid              The GUID for image.
    #   @param  Arch              Arch of this image.
    #   @param  OutputDir         The output directory for image.
    #   @param  DebugDir          The debug directory for image.
    #   @param  ImageClass        PeImage Information
    #
    def __init__(self, BaseName, Guid, Arch, OutputDir, DebugDir, ImageClass):
        self.BaseName         = BaseName
        self.Guid             = Guid
        self.Arch             = Arch
        self.OutputDir        = OutputDir
        self.DebugDir         = DebugDir
        self.Image            = ImageClass
        self.Image.Size       = (self.Image.Size // 0x1000 + 1) * 0x1000

## The class implementing the EDK2 build process
#
#   The build process includes:
#       1. Load configuration from target.txt and tools_def.txt in $(WORKSPACE)/Conf
#       2. Parse DSC file of active platform
#       3. Parse FDF file if any
#       4. Establish build database, including parse all other files (module, package)
#       5. Create AutoGen files (C code file, depex file, makefile) if necessary
#       6. Call build command
#
class Build():
    ## Constructor
    #
    # Constructor will load all necessary configurations, parse platform, modules
    # and packages and the establish a database for AutoGen.
    #
    #   @param  Target              The build command target, one of gSupportedTarget
    #   @param  WorkspaceDir        The directory of workspace
    #   @param  BuildOptions        Build options passed from command line
    #
    def __init__(self, Target, WorkspaceDir, BuildOptions,log_q):
        self.WorkspaceDir   = WorkspaceDir
        self.Target         = Target
        self.PlatformFile   = BuildOptions.PlatformFile
        self.ModuleFile     = BuildOptions.ModuleFile
        self.ArchList       = BuildOptions.TargetArch
        self.ToolChainList  = BuildOptions.ToolChain
        self.BuildTargetList= BuildOptions.BuildTarget
        self.Fdf            = BuildOptions.FdfFile
        self.FdList         = BuildOptions.RomImage
        self.FvList         = BuildOptions.FvImage
        self.CapList        = BuildOptions.CapName
        self.SilentMode     = BuildOptions.SilentMode
        self.ThreadNumber   = 1
        self.SkipAutoGen    = BuildOptions.SkipAutoGen
        self.Reparse        = BuildOptions.Reparse
        self.SkuId          = BuildOptions.SkuId
        if self.SkuId:
            GlobalData.gSKUID_CMD = self.SkuId
        self.ConfDirectory = BuildOptions.ConfDirectory
        self.SpawnMode      = True
        self.BuildReport    = BuildReport(BuildOptions.ReportFile, BuildOptions.ReportType)
        self.TargetTxt      = TargetTxt
        self.ToolDef        = ToolDef
        self.AutoGenTime    = 0
        self.MakeTime       = 0
        self.GenFdsTime     = 0
        GlobalData.BuildOptionPcd     = BuildOptions.OptionPcd if BuildOptions.OptionPcd else []
        #Set global flag for build mode
        GlobalData.gIgnoreSource = BuildOptions.IgnoreSources
        GlobalData.gUseHashCache = BuildOptions.UseHashCache
        GlobalData.gBinCacheDest   = BuildOptions.BinCacheDest
        GlobalData.gBinCacheSource = BuildOptions.BinCacheSource
        GlobalData.gEnableGenfdsMultiThread = not BuildOptions.NoGenfdsMultiThread
        GlobalData.gDisableIncludePathCheck = BuildOptions.DisableIncludePathCheck

        if GlobalData.gBinCacheDest and not GlobalData.gUseHashCache:
            EdkLogger.error("build", OPTION_NOT_SUPPORTED, ExtraData="--binary-destination must be used together with --hash.")

        if GlobalData.gBinCacheSource and not GlobalData.gUseHashCache:
            EdkLogger.error("build", OPTION_NOT_SUPPORTED, ExtraData="--binary-source must be used together with --hash.")

        if GlobalData.gBinCacheDest and GlobalData.gBinCacheSource:
            EdkLogger.error("build", OPTION_NOT_SUPPORTED, ExtraData="--binary-destination can not be used together with --binary-source.")

        if GlobalData.gBinCacheSource:
            BinCacheSource = os.path.normpath(GlobalData.gBinCacheSource)
            if not os.path.isabs(BinCacheSource):
                BinCacheSource = mws.join(self.WorkspaceDir, BinCacheSource)
            GlobalData.gBinCacheSource = BinCacheSource
        else:
            if GlobalData.gBinCacheSource is not None:
                EdkLogger.error("build", OPTION_VALUE_INVALID, ExtraData="Invalid value of option --binary-source.")

        if GlobalData.gBinCacheDest:
            BinCacheDest = os.path.normpath(GlobalData.gBinCacheDest)
            if not os.path.isabs(BinCacheDest):
                BinCacheDest = mws.join(self.WorkspaceDir, BinCacheDest)
            GlobalData.gBinCacheDest = BinCacheDest
        else:
            if GlobalData.gBinCacheDest is not None:
                EdkLogger.error("build", OPTION_VALUE_INVALID, ExtraData="Invalid value of option --binary-destination.")

        GlobalData.gDatabasePath = os.path.normpath(os.path.join(GlobalData.gConfDirectory, GlobalData.gDatabasePath))
        if not os.path.exists(os.path.join(GlobalData.gConfDirectory, '.cache')):
            os.makedirs(os.path.join(GlobalData.gConfDirectory, '.cache'))
        self.Db = BuildDB
        self.BuildDatabase = self.Db.BuildObject
        self.Platform = None
        self.ToolChainFamily = None
        self.LoadFixAddress = 0
        self.UniFlag        = BuildOptions.Flag
        self.BuildModules = []
        self.HashSkipModules = []
        self.Db_Flag = False
        self.LaunchPrebuildFlag = False
        self.PlatformBuildPath = os.path.join(GlobalData.gConfDirectory, '.cache', '.PlatformBuild')
        if BuildOptions.CommandLength:
            GlobalData.gCommandMaxLength = BuildOptions.CommandLength

        # print dot character during doing some time-consuming work
        self.Progress = Utils.Progressor()
        # print current build environment and configuration
        EdkLogger.quiet("%-16s = %s" % ("WORKSPACE", os.environ["WORKSPACE"]))
        if "PACKAGES_PATH" in os.environ:
            # WORKSPACE env has been converted before. Print the same path style with WORKSPACE env.
            EdkLogger.quiet("%-16s = %s" % ("PACKAGES_PATH", os.path.normcase(os.path.normpath(os.environ["PACKAGES_PATH"]))))
        EdkLogger.quiet("%-16s = %s" % ("EDK_TOOLS_PATH", os.environ["EDK_TOOLS_PATH"]))
        if "EDK_TOOLS_BIN" in os.environ:
            # Print the same path style with WORKSPACE env.
            EdkLogger.quiet("%-16s = %s" % ("EDK_TOOLS_BIN", os.path.normcase(os.path.normpath(os.environ["EDK_TOOLS_BIN"]))))
        EdkLogger.quiet("%-16s = %s" % ("CONF_PATH", GlobalData.gConfDirectory))
        if "PYTHON3_ENABLE" in os.environ:
            PYTHON3_ENABLE = os.environ["PYTHON3_ENABLE"]
            if PYTHON3_ENABLE != "TRUE":
                PYTHON3_ENABLE = "FALSE"
            EdkLogger.quiet("%-16s = %s" % ("PYTHON3_ENABLE", PYTHON3_ENABLE))
        if "PYTHON_COMMAND" in os.environ:
            EdkLogger.quiet("%-16s = %s" % ("PYTHON_COMMAND", os.environ["PYTHON_COMMAND"]))
        self.InitPreBuild()
        self.InitPostBuild()
        if self.Prebuild:
            EdkLogger.quiet("%-16s = %s" % ("PREBUILD", self.Prebuild))
        if self.Postbuild:
            EdkLogger.quiet("%-16s = %s" % ("POSTBUILD", self.Postbuild))
        if self.Prebuild:
            self.LaunchPrebuild()
            self.TargetTxt = TargetTxt
            self.ToolDef   = ToolDef
        if not (self.LaunchPrebuildFlag and os.path.exists(self.PlatformBuildPath)):
            self.InitBuild()

        self.AutoGenMgr = None
        EdkLogger.info("")
        os.chdir(self.WorkspaceDir)
        GlobalData.gCacheIR = Manager().dict()
        self.log_q = log_q
        GlobalData.file_lock =  mp.Lock()
        GlobalData.cache_lock = mp.Lock()
    def StartAutoGen(self,mqueue, DataPipe,SkipAutoGen,PcdMaList,share_data):
        try:
            if SkipAutoGen:
                return True,0
            feedback_q = mp.Queue()
            error_event = mp.Event()
            FfsCmd = DataPipe.Get("FfsCommand")
            if FfsCmd is None:
                FfsCmd = {}
            GlobalData.FfsCmd = FfsCmd
            GlobalData.libConstPcd = DataPipe.Get("LibConstPcd")
            GlobalData.Refes = DataPipe.Get("REFS")
            auto_workers = [AutoGenWorkerInProcess(mqueue,DataPipe.dump_file,feedback_q,GlobalData.file_lock,GlobalData.cache_lock,share_data,self.log_q,error_event) for _ in range(self.ThreadNumber)]
            self.AutoGenMgr = AutoGenManager(auto_workers,feedback_q,error_event)
            self.AutoGenMgr.start()
            for w in auto_workers:
                w.start()
            if PcdMaList is not None:
                for PcdMa in PcdMaList:
                    if GlobalData.gBinCacheSource and self.Target in [None, "", "all"]:
                        PcdMa.GenModuleFilesHash(share_data)
                        PcdMa.GenPreMakefileHash(share_data)
                        if PcdMa.CanSkipbyPreMakefileCache(share_data):
                            continue

                    PcdMa.CreateCodeFile(False)
                    PcdMa.CreateMakeFile(False,GenFfsList = DataPipe.Get("FfsCommand").get((PcdMa.MetaFile.Path, PcdMa.Arch),[]))

                    if GlobalData.gBinCacheSource and self.Target in [None, "", "all"]:
                        PcdMa.GenMakeHeaderFilesHash(share_data)
                        PcdMa.GenMakeHash(share_data)
                        if PcdMa.CanSkipbyMakeCache(share_data):
                            continue

            self.AutoGenMgr.join()
            rt = self.AutoGenMgr.Status
            return rt, 0
        except FatalError as e:
            return False, e.args[0]
        except:
            return False, UNKNOWN_ERROR

    ## Load configuration
    #
    #   This method will parse target.txt and get the build configurations.
    #
    def LoadConfiguration(self):

        # if no ARCH given in command line, get it from target.txt
        if not self.ArchList:
            self.ArchList = self.TargetTxt.TargetTxtDictionary[TAB_TAT_DEFINES_TARGET_ARCH]
        self.ArchList = tuple(self.ArchList)

        # if no build target given in command line, get it from target.txt
        if not self.BuildTargetList:
            self.BuildTargetList = self.TargetTxt.TargetTxtDictionary[TAB_TAT_DEFINES_TARGET]

        # if no tool chain given in command line, get it from target.txt
        if not self.ToolChainList:
            self.ToolChainList = self.TargetTxt.TargetTxtDictionary[TAB_TAT_DEFINES_TOOL_CHAIN_TAG]
            if self.ToolChainList is None or len(self.ToolChainList) == 0:
                EdkLogger.error("build", RESOURCE_NOT_AVAILABLE, ExtraData="No toolchain given. Don't know how to build.\n")

        # check if the tool chains are defined or not
        NewToolChainList = []
        for ToolChain in self.ToolChainList:
            if ToolChain not in self.ToolDef.ToolsDefTxtDatabase[TAB_TOD_DEFINES_TOOL_CHAIN_TAG]:
                EdkLogger.warn("build", "Tool chain [%s] is not defined" % ToolChain)
            else:
                NewToolChainList.append(ToolChain)
        # if no tool chain available, break the build
        if len(NewToolChainList) == 0:
            EdkLogger.error("build", RESOURCE_NOT_AVAILABLE,
                            ExtraData="[%s] not defined. No toolchain available for build!\n" % ", ".join(self.ToolChainList))
        else:
            self.ToolChainList = NewToolChainList

        ToolChainFamily = []
        ToolDefinition = self.ToolDef.ToolsDefTxtDatabase
        for Tool in self.ToolChainList:
            if TAB_TOD_DEFINES_FAMILY not in ToolDefinition or Tool not in ToolDefinition[TAB_TOD_DEFINES_FAMILY] \
               or not ToolDefinition[TAB_TOD_DEFINES_FAMILY][Tool]:
                EdkLogger.warn("build", "No tool chain family found in configuration for %s. Default to MSFT." % Tool)
                ToolChainFamily.append(TAB_COMPILER_MSFT)
            else:
                ToolChainFamily.append(ToolDefinition[TAB_TOD_DEFINES_FAMILY][Tool])
        self.ToolChainFamily = ToolChainFamily

        if not self.PlatformFile:
            PlatformFile = self.TargetTxt.TargetTxtDictionary[TAB_TAT_DEFINES_ACTIVE_PLATFORM]
            if not PlatformFile:
                # Try to find one in current directory
                WorkingDirectory = os.getcwd()
                FileList = glob.glob(os.path.normpath(os.path.join(WorkingDirectory, '*.dsc')))
                FileNum = len(FileList)
                if FileNum >= 2:
                    EdkLogger.error("build", OPTION_MISSING,
                                    ExtraData="There are %d DSC files in %s. Use '-p' to specify one.\n" % (FileNum, WorkingDirectory))
                elif FileNum == 1:
                    PlatformFile = FileList[0]
                else:
                    EdkLogger.error("build", RESOURCE_NOT_AVAILABLE,
                                    ExtraData="No active platform specified in target.txt or command line! Nothing can be built.\n")

            self.PlatformFile = PathClass(NormFile(PlatformFile, self.WorkspaceDir), self.WorkspaceDir)
        self.ThreadNumber   = ThreadNum()
    ## Initialize build configuration
    #
    #   This method will parse DSC file and merge the configurations from
    #   command line and target.txt, then get the final build configurations.
    #
    def InitBuild(self):
        # parse target.txt, tools_def.txt, and platform file
        self.LoadConfiguration()

        # Allow case-insensitive for those from command line or configuration file
        ErrorCode, ErrorInfo = self.PlatformFile.Validate(".dsc", False)
        if ErrorCode != 0:
            EdkLogger.error("build", ErrorCode, ExtraData=ErrorInfo)


    def InitPreBuild(self):
        self.LoadConfiguration()
        ErrorCode, ErrorInfo = self.PlatformFile.Validate(".dsc", False)
        if ErrorCode != 0:
            EdkLogger.error("build", ErrorCode, ExtraData=ErrorInfo)
        if self.BuildTargetList:
            GlobalData.gGlobalDefines['TARGET'] = self.BuildTargetList[0]
        if self.ArchList:
            GlobalData.gGlobalDefines['ARCH'] = self.ArchList[0]
        if self.ToolChainList:
            GlobalData.gGlobalDefines['TOOLCHAIN'] = self.ToolChainList[0]
            GlobalData.gGlobalDefines['TOOL_CHAIN_TAG'] = self.ToolChainList[0]
        if self.ToolChainFamily:
            GlobalData.gGlobalDefines['FAMILY'] = self.ToolChainFamily[0]
        if 'PREBUILD' in GlobalData.gCommandLineDefines:
            self.Prebuild   = GlobalData.gCommandLineDefines.get('PREBUILD')
        else:
            self.Db_Flag = True
            Platform = self.Db.MapPlatform(str(self.PlatformFile))
            self.Prebuild = str(Platform.Prebuild)
        if self.Prebuild:
            PrebuildList = []
            #
            # Evaluate all arguments and convert arguments that are WORKSPACE
            # relative paths to absolute paths.  Filter arguments that look like
            # flags or do not follow the file/dir naming rules to avoid false
            # positives on this conversion.
            #
            for Arg in self.Prebuild.split():
                #
                # Do not modify Arg if it looks like a flag or an absolute file path
                #
                if Arg.startswith('-')  or os.path.isabs(Arg):
                    PrebuildList.append(Arg)
                    continue
                #
                # Do not modify Arg if it does not look like a Workspace relative
                # path that starts with a valid package directory name
                #
                if not Arg[0].isalpha() or os.path.dirname(Arg) == '':
                    PrebuildList.append(Arg)
                    continue
                #
                # If Arg looks like a WORKSPACE relative path, then convert to an
                # absolute path and check to see if the file exists.
                #
                Temp = mws.join(self.WorkspaceDir, Arg)
                if os.path.isfile(Temp):
                    Arg = Temp
                PrebuildList.append(Arg)
            self.Prebuild       = ' '.join(PrebuildList)
            self.Prebuild += self.PassCommandOption(self.BuildTargetList, self.ArchList, self.ToolChainList, self.PlatformFile, self.Target)

    def InitPostBuild(self):
        if 'POSTBUILD' in GlobalData.gCommandLineDefines:
            self.Postbuild = GlobalData.gCommandLineDefines.get('POSTBUILD')
        else:
            Platform = self.Db.MapPlatform(str(self.PlatformFile))
            self.Postbuild = str(Platform.Postbuild)
        if self.Postbuild:
            PostbuildList = []
            #
            # Evaluate all arguments and convert arguments that are WORKSPACE
            # relative paths to absolute paths.  Filter arguments that look like
            # flags or do not follow the file/dir naming rules to avoid false
            # positives on this conversion.
            #
            for Arg in self.Postbuild.split():
                #
                # Do not modify Arg if it looks like a flag or an absolute file path
                #
                if Arg.startswith('-')  or os.path.isabs(Arg):
                    PostbuildList.append(Arg)
                    continue
                #
                # Do not modify Arg if it does not look like a Workspace relative
                # path that starts with a valid package directory name
                #
                if not Arg[0].isalpha() or os.path.dirname(Arg) == '':
                    PostbuildList.append(Arg)
                    continue
                #
                # If Arg looks like a WORKSPACE relative path, then convert to an
                # absolute path and check to see if the file exists.
                #
                Temp = mws.join(self.WorkspaceDir, Arg)
                if os.path.isfile(Temp):
                    Arg = Temp
                PostbuildList.append(Arg)
            self.Postbuild       = ' '.join(PostbuildList)
            self.Postbuild += self.PassCommandOption(self.BuildTargetList, self.ArchList, self.ToolChainList, self.PlatformFile, self.Target)

    def PassCommandOption(self, BuildTarget, TargetArch, ToolChain, PlatformFile, Target):
        BuildStr = ''
        if GlobalData.gCommand and isinstance(GlobalData.gCommand, list):
            BuildStr += ' ' + ' '.join(GlobalData.gCommand)
        TargetFlag = False
        ArchFlag = False
        ToolChainFlag = False
        PlatformFileFlag = False

        if GlobalData.gOptions and not GlobalData.gOptions.BuildTarget:
            TargetFlag = True
        if GlobalData.gOptions and not GlobalData.gOptions.TargetArch:
            ArchFlag = True
        if GlobalData.gOptions and not GlobalData.gOptions.ToolChain:
            ToolChainFlag = True
        if GlobalData.gOptions and not GlobalData.gOptions.PlatformFile:
            PlatformFileFlag = True

        if TargetFlag and BuildTarget:
            if isinstance(BuildTarget, list) or isinstance(BuildTarget, tuple):
                BuildStr += ' -b ' + ' -b '.join(BuildTarget)
            elif isinstance(BuildTarget, str):
                BuildStr += ' -b ' + BuildTarget
        if ArchFlag and TargetArch:
            if isinstance(TargetArch, list) or isinstance(TargetArch, tuple):
                BuildStr += ' -a ' + ' -a '.join(TargetArch)
            elif isinstance(TargetArch, str):
                BuildStr += ' -a ' + TargetArch
        if ToolChainFlag and ToolChain:
            if isinstance(ToolChain, list) or isinstance(ToolChain, tuple):
                BuildStr += ' -t ' + ' -t '.join(ToolChain)
            elif isinstance(ToolChain, str):
                BuildStr += ' -t ' + ToolChain
        if PlatformFileFlag and PlatformFile:
            if isinstance(PlatformFile, list) or isinstance(PlatformFile, tuple):
                BuildStr += ' -p ' + ' -p '.join(PlatformFile)
            elif isinstance(PlatformFile, str):
                BuildStr += ' -p' + PlatformFile
        BuildStr += ' --conf=' + GlobalData.gConfDirectory
        if Target:
            BuildStr += ' ' + Target

        return BuildStr

    def LaunchPrebuild(self):
        if self.Prebuild:
            EdkLogger.info("\n- Prebuild Start -\n")
            self.LaunchPrebuildFlag = True
            #
            # The purpose of .PrebuildEnv file is capture environment variable settings set by the prebuild script
            # and preserve them for the rest of the main build step, because the child process environment will
            # evaporate as soon as it exits, we cannot get it in build step.
            #
            PrebuildEnvFile = os.path.join(GlobalData.gConfDirectory, '.cache', '.PrebuildEnv')
            if os.path.isfile(PrebuildEnvFile):
                os.remove(PrebuildEnvFile)
            if os.path.isfile(self.PlatformBuildPath):
                os.remove(self.PlatformBuildPath)
            if sys.platform == "win32":
                args = ' && '.join((self.Prebuild, 'set > ' + PrebuildEnvFile))
                Process = Popen(args, stdout=PIPE, stderr=PIPE, shell=True)
            else:
                args = ' && '.join((self.Prebuild, 'env > ' + PrebuildEnvFile))
                Process = Popen(args, stdout=PIPE, stderr=PIPE, shell=True)

            # launch two threads to read the STDOUT and STDERR
            EndOfProcedure = Event()
            EndOfProcedure.clear()
            if Process.stdout:
                StdOutThread = Thread(target=ReadMessage, args=(Process.stdout, EdkLogger.info, EndOfProcedure))
                StdOutThread.setName("STDOUT-Redirector")
                StdOutThread.setDaemon(False)
                StdOutThread.start()

            if Process.stderr:
                StdErrThread = Thread(target=ReadMessage, args=(Process.stderr, EdkLogger.quiet, EndOfProcedure))
                StdErrThread.setName("STDERR-Redirector")
                StdErrThread.setDaemon(False)
                StdErrThread.start()
            # waiting for program exit
            Process.wait()

            if Process.stdout:
                StdOutThread.join()
            if Process.stderr:
                StdErrThread.join()
            if Process.returncode != 0 :
                EdkLogger.error("Prebuild", PREBUILD_ERROR, 'Prebuild process is not success!')

            if os.path.exists(PrebuildEnvFile):
                f = open(PrebuildEnvFile)
                envs = f.readlines()
                f.close()
                envs = [l.split("=", 1) for l in envs ]
                envs = [[I.strip() for I in item] for item in envs if len(item) == 2]
                os.environ.update(dict(envs))
            EdkLogger.info("\n- Prebuild Done -\n")

    def LaunchPostbuild(self):
        if self.Postbuild:
            EdkLogger.info("\n- Postbuild Start -\n")
            if sys.platform == "win32":
                Process = Popen(self.Postbuild, stdout=PIPE, stderr=PIPE, shell=True)
            else:
                Process = Popen(self.Postbuild, stdout=PIPE, stderr=PIPE, shell=True)
            # launch two threads to read the STDOUT and STDERR
            EndOfProcedure = Event()
            EndOfProcedure.clear()
            if Process.stdout:
                StdOutThread = Thread(target=ReadMessage, args=(Process.stdout, EdkLogger.info, EndOfProcedure))
                StdOutThread.setName("STDOUT-Redirector")
                StdOutThread.setDaemon(False)
                StdOutThread.start()

            if Process.stderr:
                StdErrThread = Thread(target=ReadMessage, args=(Process.stderr, EdkLogger.quiet, EndOfProcedure))
                StdErrThread.setName("STDERR-Redirector")
                StdErrThread.setDaemon(False)
                StdErrThread.start()
            # waiting for program exit
            Process.wait()

            if Process.stdout:
                StdOutThread.join()
            if Process.stderr:
                StdErrThread.join()
            if Process.returncode != 0 :
                EdkLogger.error("Postbuild", POSTBUILD_ERROR, 'Postbuild process is not success!')
            EdkLogger.info("\n- Postbuild Done -\n")

    ## Error handling for hash feature
    #
    # On BuildTask error, iterate through the Module Build tracking
    # dictionary to determine wheather a module failed to build. Invalidate
    # the hash associated with that module by removing it from storage.
    #
    #
    def invalidateHash(self):
        # Only for hashing feature
        if not GlobalData.gUseHashCache:
            return

        # GlobalData.gModuleBuildTracking contains only modules or libs that cannot be skipped by hash
        for Ma in GlobalData.gModuleBuildTracking:
            # Skip invalidating for Successful Module/Lib builds
            if GlobalData.gModuleBuildTracking[Ma] == 'SUCCESS':
                continue

            # The module failed to build, failed to start building, or failed the header check test from this point on

            # Remove .hash from build
            ModuleHashFile = os.path.join(Ma.BuildDir, Ma.Name + ".hash")
            if os.path.exists(ModuleHashFile):
                os.remove(ModuleHashFile)

            # Remove .hash file from cache
            if GlobalData.gBinCacheDest:
                FileDir = os.path.join(GlobalData.gBinCacheDest, Ma.PlatformInfo.OutputDir, Ma.BuildTarget + "_" + Ma.ToolChain, Ma.Arch, Ma.SourceDir, Ma.MetaFile.BaseName)
                HashFile = os.path.join(FileDir, Ma.Name + '.hash')
                if os.path.exists(HashFile):
                    os.remove(HashFile)

    ## Build a module or platform
    #
    # Create autogen code and makefile for a module or platform, and the launch
    # "make" command to build it
    #
    #   @param  Target                      The target of build command
    #   @param  Platform                    The platform file
    #   @param  Module                      The module file
    #   @param  BuildTarget                 The name of build target, one of "DEBUG", "RELEASE"
    #   @param  ToolChain                   The name of toolchain to build
    #   @param  Arch                        The arch of the module/platform
    #   @param  CreateDepModuleCodeFile     Flag used to indicate creating code
    #                                       for dependent modules/Libraries
    #   @param  CreateDepModuleMakeFile     Flag used to indicate creating makefile
    #                                       for dependent modules/Libraries
    #
    def _BuildPa(self, Target, AutoGenObject, CreateDepsCodeFile=True, CreateDepsMakeFile=True, BuildModule=False, FfsCommand=None, PcdMaList=None):
        if AutoGenObject is None:
            return False
        if FfsCommand is None:
            FfsCommand = {}
        # skip file generation for cleanxxx targets, run and fds target
        if Target not in ['clean', 'cleanlib', 'cleanall', 'run', 'fds']:
            # for target which must generate AutoGen code and makefile
            mqueue = mp.Queue()
            for m in AutoGenObject.GetAllModuleInfo:
                mqueue.put(m)

            AutoGenObject.DataPipe.DataContainer = {"CommandTarget": self.Target}
            AutoGenObject.DataPipe.DataContainer = {"Workspace_timestamp": AutoGenObject.Workspace._SrcTimeStamp}
            AutoGenObject.CreateLibModuelDirs()
            AutoGenObject.DataPipe.DataContainer = {"LibraryBuildDirectoryList":AutoGenObject.LibraryBuildDirectoryList}
            AutoGenObject.DataPipe.DataContainer = {"ModuleBuildDirectoryList":AutoGenObject.ModuleBuildDirectoryList}
            AutoGenObject.DataPipe.DataContainer = {"FdsCommandDict": AutoGenObject.Workspace.GenFdsCommandDict}
            self.Progress.Start("Generating makefile and code")
            data_pipe_file = os.path.join(AutoGenObject.BuildDir, "GlobalVar_%s_%s.bin" % (str(AutoGenObject.Guid),AutoGenObject.Arch))
            AutoGenObject.DataPipe.dump(data_pipe_file)
            autogen_rt,errorcode = self.StartAutoGen(mqueue, AutoGenObject.DataPipe, self.SkipAutoGen, PcdMaList, GlobalData.gCacheIR)
            AutoGenIdFile = os.path.join(GlobalData.gConfDirectory,".AutoGenIdFile.txt")
            with open(AutoGenIdFile,"w") as fw:
                fw.write("Arch=%s\n" % "|".join((AutoGenObject.Workspace.ArchList)))
                fw.write("BuildDir=%s\n" % AutoGenObject.Workspace.BuildDir)
                fw.write("PlatformGuid=%s\n" % str(AutoGenObject.Guid))
            self.Progress.Stop("done!")
            if not autogen_rt:
                self.AutoGenMgr.TerminateWorkers()
                self.AutoGenMgr.join(1)
                raise FatalError(errorcode)
            AutoGenObject.CreateCodeFile(False)
            AutoGenObject.CreateMakeFile(False)
        else:
            # always recreate top/platform makefile when clean, just in case of inconsistency
            AutoGenObject.CreateCodeFile(True)
            AutoGenObject.CreateMakeFile(True)

        if EdkLogger.GetLevel() == EdkLogger.QUIET:
            EdkLogger.quiet("Building ... %s" % repr(AutoGenObject))

        BuildCommand = AutoGenObject.BuildCommand
        if BuildCommand is None or len(BuildCommand) == 0:
            EdkLogger.error("build", OPTION_MISSING,
                            "No build command found for this module. "
                            "Please check your setting of %s_%s_%s_MAKE_PATH in Conf/tools_def.txt file." %
                                (AutoGenObject.BuildTarget, AutoGenObject.ToolChain, AutoGenObject.Arch),
                            ExtraData=str(AutoGenObject))

        makefile = GenMake.BuildFile(AutoGenObject)._FILE_NAME_[GenMake.gMakeType]

        # run
        if Target == 'run':
            return True

        # build modules
        if BuildModule:
            BuildCommand = BuildCommand + [Target]
            LaunchCommand(BuildCommand, AutoGenObject.MakeFileDir)
            self.CreateAsBuiltInf()
            if GlobalData.gBinCacheDest:
                self.UpdateBuildCache()
            self.BuildModules = []
            return True

        # build library
        if Target == 'libraries':
            for Lib in AutoGenObject.LibraryBuildDirectoryList:
                NewBuildCommand = BuildCommand + ['-f', os.path.normpath(os.path.join(Lib, makefile)), 'pbuild']
                LaunchCommand(NewBuildCommand, AutoGenObject.MakeFileDir)
            return True

        # build module
        if Target == 'modules':
            for Lib in AutoGenObject.LibraryBuildDirectoryList:
                NewBuildCommand = BuildCommand + ['-f', os.path.normpath(os.path.join(Lib, makefile)), 'pbuild']
                LaunchCommand(NewBuildCommand, AutoGenObject.MakeFileDir)
            for Mod in AutoGenObject.ModuleBuildDirectoryList:
                NewBuildCommand = BuildCommand + ['-f', os.path.normpath(os.path.join(Mod, makefile)), 'pbuild']
                LaunchCommand(NewBuildCommand, AutoGenObject.MakeFileDir)
            self.CreateAsBuiltInf()
            if GlobalData.gBinCacheDest:
                self.UpdateBuildCache()
            self.BuildModules = []
            return True

        # cleanlib
        if Target == 'cleanlib':
            for Lib in AutoGenObject.LibraryBuildDirectoryList:
                LibMakefile = os.path.normpath(os.path.join(Lib, makefile))
                if os.path.exists(LibMakefile):
                    NewBuildCommand = BuildCommand + ['-f', LibMakefile, 'cleanall']
                    LaunchCommand(NewBuildCommand, AutoGenObject.MakeFileDir)
            return True

        # clean
        if Target == 'clean':
            for Mod in AutoGenObject.ModuleBuildDirectoryList:
                ModMakefile = os.path.normpath(os.path.join(Mod, makefile))
                if os.path.exists(ModMakefile):
                    NewBuildCommand = BuildCommand + ['-f', ModMakefile, 'cleanall']
                    LaunchCommand(NewBuildCommand, AutoGenObject.MakeFileDir)
            for Lib in AutoGenObject.LibraryBuildDirectoryList:
                LibMakefile = os.path.normpath(os.path.join(Lib, makefile))
                if os.path.exists(LibMakefile):
                    NewBuildCommand = BuildCommand + ['-f', LibMakefile, 'cleanall']
                    LaunchCommand(NewBuildCommand, AutoGenObject.MakeFileDir)
            return True

        # cleanall
        if Target == 'cleanall':
            try:
                #os.rmdir(AutoGenObject.BuildDir)
                RemoveDirectory(AutoGenObject.BuildDir, True)
            except WindowsError as X:
                EdkLogger.error("build", FILE_DELETE_FAILURE, ExtraData=str(X))
        return True

    ## Build a module or platform
    #
    # Create autogen code and makefile for a module or platform, and the launch
    # "make" command to build it
    #
    #   @param  Target                      The target of build command
    #   @param  Platform                    The platform file
    #   @param  Module                      The module file
    #   @param  BuildTarget                 The name of build target, one of "DEBUG", "RELEASE"
    #   @param  ToolChain                   The name of toolchain to build
    #   @param  Arch                        The arch of the module/platform
    #   @param  CreateDepModuleCodeFile     Flag used to indicate creating code
    #                                       for dependent modules/Libraries
    #   @param  CreateDepModuleMakeFile     Flag used to indicate creating makefile
    #                                       for dependent modules/Libraries
    #
    def _Build(self, Target, AutoGenObject, CreateDepsCodeFile=True, CreateDepsMakeFile=True, BuildModule=False):
        if AutoGenObject is None:
            return False

        # skip file generation for cleanxxx targets, run and fds target
        if Target not in ['clean', 'cleanlib', 'cleanall', 'run', 'fds']:
            # for target which must generate AutoGen code and makefile
            if not self.SkipAutoGen or Target == 'genc':
                self.Progress.Start("Generating code")
                AutoGenObject.CreateCodeFile(CreateDepsCodeFile)
                self.Progress.Stop("done!")
            if Target == "genc":
                return True

            if not self.SkipAutoGen or Target == 'genmake':
                self.Progress.Start("Generating makefile")
                AutoGenObject.CreateMakeFile(CreateDepsMakeFile)
                #AutoGenObject.CreateAsBuiltInf()
                self.Progress.Stop("done!")
            if Target == "genmake":
                return True
        else:
            # always recreate top/platform makefile when clean, just in case of inconsistency
            AutoGenObject.CreateCodeFile(True)
            AutoGenObject.CreateMakeFile(True)

        if EdkLogger.GetLevel() == EdkLogger.QUIET:
            EdkLogger.quiet("Building ... %s" % repr(AutoGenObject))

        BuildCommand = AutoGenObject.BuildCommand
        if BuildCommand is None or len(BuildCommand) == 0:
            EdkLogger.error("build", OPTION_MISSING,
                            "No build command found for this module. "
                            "Please check your setting of %s_%s_%s_MAKE_PATH in Conf/tools_def.txt file." %
                                (AutoGenObject.BuildTarget, AutoGenObject.ToolChain, AutoGenObject.Arch),
                            ExtraData=str(AutoGenObject))

        # build modules
        if BuildModule:
            if Target != 'fds':
                BuildCommand = BuildCommand + [Target]
            AutoGenObject.BuildTime = LaunchCommand(BuildCommand, AutoGenObject.MakeFileDir)
            self.CreateAsBuiltInf()
            if GlobalData.gBinCacheDest:
                self.UpdateBuildCache()
            self.BuildModules = []
            return True

        # genfds
        if Target == 'fds':
            if GenFdsApi(AutoGenObject.GenFdsCommandDict, self.Db):
                EdkLogger.error("build", COMMAND_FAILURE)
            return True

        # run
        if Target == 'run':
            return True

        # build library
        if Target == 'libraries':
            pass

        # not build modules


        # cleanall
        if Target == 'cleanall':
            try:
                #os.rmdir(AutoGenObject.BuildDir)
                RemoveDirectory(AutoGenObject.BuildDir, True)
            except WindowsError as X:
                EdkLogger.error("build", FILE_DELETE_FAILURE, ExtraData=str(X))
        return True

    ## Rebase module image and Get function address for the input module list.
    #
    def _RebaseModule (self, MapBuffer, BaseAddress, ModuleList, AddrIsOffset = True, ModeIsSmm = False):
        if ModeIsSmm:
            AddrIsOffset = False
        for InfFile in ModuleList:
            sys.stdout.write (".")
            sys.stdout.flush()
            ModuleInfo = ModuleList[InfFile]
            ModuleName = ModuleInfo.BaseName
            ModuleOutputImage = ModuleInfo.Image.FileName
            ModuleDebugImage  = os.path.join(ModuleInfo.DebugDir, ModuleInfo.BaseName + '.efi')
            ## for SMM module in SMRAM, the SMRAM will be allocated from base to top.
            if not ModeIsSmm:
                BaseAddress = BaseAddress - ModuleInfo.Image.Size
                #
                # Update Image to new BaseAddress by GenFw tool
                #
                LaunchCommand(["GenFw", "--rebase", str(BaseAddress), "-r", ModuleOutputImage], ModuleInfo.OutputDir)
                LaunchCommand(["GenFw", "--rebase", str(BaseAddress), "-r", ModuleDebugImage], ModuleInfo.DebugDir)
            else:
                #
                # Set new address to the section header only for SMM driver.
                #
                LaunchCommand(["GenFw", "--address", str(BaseAddress), "-r", ModuleOutputImage], ModuleInfo.OutputDir)
                LaunchCommand(["GenFw", "--address", str(BaseAddress), "-r", ModuleDebugImage], ModuleInfo.DebugDir)
            #
            # Collect function address from Map file
            #
            ImageMapTable = ModuleOutputImage.replace('.efi', '.map')
            FunctionList = []
            if os.path.exists(ImageMapTable):
                OrigImageBaseAddress = 0
                ImageMap = open(ImageMapTable, 'r')
                for LinStr in ImageMap:
                    if len (LinStr.strip()) == 0:
                        continue
                    #
                    # Get the preferred address set on link time.
                    #
                    if LinStr.find ('Preferred load address is') != -1:
                        StrList = LinStr.split()
                        OrigImageBaseAddress = int (StrList[len(StrList) - 1], 16)

                    StrList = LinStr.split()
                    if len (StrList) > 4:
                        if StrList[3] == 'f' or StrList[3] == 'F':
                            Name = StrList[1]
                            RelativeAddress = int (StrList[2], 16) - OrigImageBaseAddress
                            FunctionList.append ((Name, RelativeAddress))

                ImageMap.close()
            #
            # Add general information.
            #
            if ModeIsSmm:
                MapBuffer.append('\n\n%s (Fixed SMRAM Offset,   BaseAddress=0x%010X,  EntryPoint=0x%010X)\n' % (ModuleName, BaseAddress, BaseAddress + ModuleInfo.Image.EntryPoint))
            elif AddrIsOffset:
                MapBuffer.append('\n\n%s (Fixed Memory Offset,  BaseAddress=-0x%010X, EntryPoint=-0x%010X)\n' % (ModuleName, 0 - BaseAddress, 0 - (BaseAddress + ModuleInfo.Image.EntryPoint)))
            else:
                MapBuffer.append('\n\n%s (Fixed Memory Address, BaseAddress=0x%010X,  EntryPoint=0x%010X)\n' % (ModuleName, BaseAddress, BaseAddress + ModuleInfo.Image.EntryPoint))
            #
            # Add guid and general seciton section.
            #
            TextSectionAddress = 0
            DataSectionAddress = 0
            for SectionHeader in ModuleInfo.Image.SectionHeaderList:
                if SectionHeader[0] == '.text':
                    TextSectionAddress = SectionHeader[1]
                elif SectionHeader[0] in ['.data', '.sdata']:
                    DataSectionAddress = SectionHeader[1]
            if AddrIsOffset:
                MapBuffer.append('(GUID=%s, .textbaseaddress=-0x%010X, .databaseaddress=-0x%010X)\n' % (ModuleInfo.Guid, 0 - (BaseAddress + TextSectionAddress), 0 - (BaseAddress + DataSectionAddress)))
            else:
                MapBuffer.append('(GUID=%s, .textbaseaddress=0x%010X, .databaseaddress=0x%010X)\n' % (ModuleInfo.Guid, BaseAddress + TextSectionAddress, BaseAddress + DataSectionAddress))
            #
            # Add debug image full path.
            #
            MapBuffer.append('(IMAGE=%s)\n\n' % (ModuleDebugImage))
            #
            # Add function address
            #
            for Function in FunctionList:
                if AddrIsOffset:
                    MapBuffer.append('  -0x%010X    %s\n' % (0 - (BaseAddress + Function[1]), Function[0]))
                else:
                    MapBuffer.append('  0x%010X    %s\n' % (BaseAddress + Function[1], Function[0]))
            ImageMap.close()

            #
            # for SMM module in SMRAM, the SMRAM will be allocated from base to top.
            #
            if ModeIsSmm:
                BaseAddress = BaseAddress + ModuleInfo.Image.Size

    ## Collect MAP information of all FVs
    #
    def _CollectFvMapBuffer (self, MapBuffer, Wa, ModuleList):
        if self.Fdf:
            # First get the XIP base address for FV map file.
            GuidPattern = re.compile("[-a-fA-F0-9]+")
            GuidName = re.compile(r"\(GUID=[-a-fA-F0-9]+")
            for FvName in Wa.FdfProfile.FvDict:
                FvMapBuffer = os.path.join(Wa.FvDir, FvName + '.Fv.map')
                if not os.path.exists(FvMapBuffer):
                    continue
                FvMap = open(FvMapBuffer, 'r')
                #skip FV size information
                FvMap.readline()
                FvMap.readline()
                FvMap.readline()
                FvMap.readline()
                for Line in FvMap:
                    MatchGuid = GuidPattern.match(Line)
                    if MatchGuid is not None:
                        #
                        # Replace GUID with module name
                        #
                        GuidString = MatchGuid.group()
                        if GuidString.upper() in ModuleList:
                            Line = Line.replace(GuidString, ModuleList[GuidString.upper()].Name)
                    MapBuffer.append(Line)
                    #
                    # Add the debug image full path.
                    #
                    MatchGuid = GuidName.match(Line)
                    if MatchGuid is not None:
                        GuidString = MatchGuid.group().split("=")[1]
                        if GuidString.upper() in ModuleList:
                            MapBuffer.append('(IMAGE=%s)\n' % (os.path.join(ModuleList[GuidString.upper()].DebugDir, ModuleList[GuidString.upper()].Name + '.efi')))

                FvMap.close()

    ## Collect MAP information of all modules
    #
    def _CollectModuleMapBuffer (self, MapBuffer, ModuleList):
        sys.stdout.write ("Generate Load Module At Fix Address Map")
        sys.stdout.flush()
        PatchEfiImageList = []
        PeiModuleList  = {}
        BtModuleList   = {}
        RtModuleList   = {}
        SmmModuleList  = {}
        PeiSize = 0
        BtSize  = 0
        RtSize  = 0
        # reserve 4K size in SMRAM to make SMM module address not from 0.
        SmmSize = 0x1000
        for ModuleGuid in ModuleList:
            Module = ModuleList[ModuleGuid]
            GlobalData.gProcessingFile = "%s [%s, %s, %s]" % (Module.MetaFile, Module.Arch, Module.ToolChain, Module.BuildTarget)

            OutputImageFile = ''
            for ResultFile in Module.CodaTargetList:
                if str(ResultFile.Target).endswith('.efi'):
                    #
                    # module list for PEI, DXE, RUNTIME and SMM
                    #
                    OutputImageFile = os.path.join(Module.OutputDir, Module.Name + '.efi')
                    ImageClass = PeImageClass (OutputImageFile)
                    if not ImageClass.IsValid:
                        EdkLogger.error("build", FILE_PARSE_FAILURE, ExtraData=ImageClass.ErrorInfo)
                    ImageInfo = PeImageInfo(Module.Name, Module.Guid, Module.Arch, Module.OutputDir, Module.DebugDir, ImageClass)
                    if Module.ModuleType in [SUP_MODULE_PEI_CORE, SUP_MODULE_PEIM, EDK_COMPONENT_TYPE_COMBINED_PEIM_DRIVER, EDK_COMPONENT_TYPE_PIC_PEIM, EDK_COMPONENT_TYPE_RELOCATABLE_PEIM, SUP_MODULE_DXE_CORE]:
                        PeiModuleList[Module.MetaFile] = ImageInfo
                        PeiSize += ImageInfo.Image.Size
                    elif Module.ModuleType in [EDK_COMPONENT_TYPE_BS_DRIVER, SUP_MODULE_DXE_DRIVER, SUP_MODULE_UEFI_DRIVER]:
                        BtModuleList[Module.MetaFile] = ImageInfo
                        BtSize += ImageInfo.Image.Size
                    elif Module.ModuleType in [SUP_MODULE_DXE_RUNTIME_DRIVER, EDK_COMPONENT_TYPE_RT_DRIVER, SUP_MODULE_DXE_SAL_DRIVER, EDK_COMPONENT_TYPE_SAL_RT_DRIVER]:
                        RtModuleList[Module.MetaFile] = ImageInfo
                        RtSize += ImageInfo.Image.Size
                    elif Module.ModuleType in [SUP_MODULE_SMM_CORE, SUP_MODULE_DXE_SMM_DRIVER, SUP_MODULE_MM_STANDALONE, SUP_MODULE_MM_CORE_STANDALONE]:
                        SmmModuleList[Module.MetaFile] = ImageInfo
                        SmmSize += ImageInfo.Image.Size
                        if Module.ModuleType == SUP_MODULE_DXE_SMM_DRIVER:
                            PiSpecVersion = Module.Module.Specification.get('PI_SPECIFICATION_VERSION', '0x00000000')
                            # for PI specification < PI1.1, DXE_SMM_DRIVER also runs as BOOT time driver.
                            if int(PiSpecVersion, 16) < 0x0001000A:
                                BtModuleList[Module.MetaFile] = ImageInfo
                                BtSize += ImageInfo.Image.Size
                    break
            #
            # EFI image is final target.
            # Check EFI image contains patchable FixAddress related PCDs.
            #
            if OutputImageFile != '':
                ModuleIsPatch = False
                for Pcd in Module.ModulePcdList:
                    if Pcd.Type == TAB_PCDS_PATCHABLE_IN_MODULE and Pcd.TokenCName in TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_SET:
                        ModuleIsPatch = True
                        break
                if not ModuleIsPatch:
                    for Pcd in Module.LibraryPcdList:
                        if Pcd.Type == TAB_PCDS_PATCHABLE_IN_MODULE and Pcd.TokenCName in TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_SET:
                            ModuleIsPatch = True
                            break

                if not ModuleIsPatch:
                    continue
                #
                # Module includes the patchable load fix address PCDs.
                # It will be fixed up later.
                #
                PatchEfiImageList.append (OutputImageFile)

        #
        # Get Top Memory address
        #
        ReservedRuntimeMemorySize = 0
        TopMemoryAddress = 0
        if self.LoadFixAddress == 0xFFFFFFFFFFFFFFFF:
            TopMemoryAddress = 0
        else:
            TopMemoryAddress = self.LoadFixAddress
            if TopMemoryAddress < RtSize + BtSize + PeiSize:
                EdkLogger.error("build", PARAMETER_INVALID, "FIX_LOAD_TOP_MEMORY_ADDRESS is too low to load driver")

        #
        # Patch FixAddress related PCDs into EFI image
        #
        for EfiImage in PatchEfiImageList:
            EfiImageMap = EfiImage.replace('.efi', '.map')
            if not os.path.exists(EfiImageMap):
                continue
            #
            # Get PCD offset in EFI image by GenPatchPcdTable function
            #
            PcdTable = parsePcdInfoFromMapFile(EfiImageMap, EfiImage)
            #
            # Patch real PCD value by PatchPcdValue tool
            #
            for PcdInfo in PcdTable:
                ReturnValue = 0
                if PcdInfo[0] == TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_PEI_PAGE_SIZE:
                    ReturnValue, ErrorInfo = PatchBinaryFile (EfiImage, PcdInfo[1], TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_PEI_PAGE_SIZE_DATA_TYPE, str (PeiSize // 0x1000))
                elif PcdInfo[0] == TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_DXE_PAGE_SIZE:
                    ReturnValue, ErrorInfo = PatchBinaryFile (EfiImage, PcdInfo[1], TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_DXE_PAGE_SIZE_DATA_TYPE, str (BtSize // 0x1000))
                elif PcdInfo[0] == TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_RUNTIME_PAGE_SIZE:
                    ReturnValue, ErrorInfo = PatchBinaryFile (EfiImage, PcdInfo[1], TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_RUNTIME_PAGE_SIZE_DATA_TYPE, str (RtSize // 0x1000))
                elif PcdInfo[0] == TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_SMM_PAGE_SIZE and len (SmmModuleList) > 0:
                    ReturnValue, ErrorInfo = PatchBinaryFile (EfiImage, PcdInfo[1], TAB_PCDS_PATCHABLE_LOAD_FIX_ADDRESS_SMM_PAGE_SIZE_DATA_TYPE, str (SmmSize // 0x1000))
                if ReturnValue != 0:
                    EdkLogger.error("build", PARAMETER_INVALID, "Patch PCD value failed", ExtraData=ErrorInfo)

        MapBuffer.append('PEI_CODE_PAGE_NUMBER      = 0x%x\n' % (PeiSize // 0x1000))
        MapBuffer.append('BOOT_CODE_PAGE_NUMBER     = 0x%x\n' % (BtSize // 0x1000))
        MapBuffer.append('RUNTIME_CODE_PAGE_NUMBER  = 0x%x\n' % (RtSize // 0x1000))
        if len (SmmModuleList) > 0:
            MapBuffer.append('SMM_CODE_PAGE_NUMBER      = 0x%x\n' % (SmmSize // 0x1000))

        PeiBaseAddr = TopMemoryAddress - RtSize - BtSize
        BtBaseAddr  = TopMemoryAddress - RtSize
        RtBaseAddr  = TopMemoryAddress - ReservedRuntimeMemorySize

        self._RebaseModule (MapBuffer, PeiBaseAddr, PeiModuleList, TopMemoryAddress == 0)
        self._RebaseModule (MapBuffer, BtBaseAddr, BtModuleList, TopMemoryAddress == 0)
        self._RebaseModule (MapBuffer, RtBaseAddr, RtModuleList, TopMemoryAddress == 0)
        self._RebaseModule (MapBuffer, 0x1000, SmmModuleList, AddrIsOffset=False, ModeIsSmm=True)
        MapBuffer.append('\n\n')
        sys.stdout.write ("\n")
        sys.stdout.flush()

    ## Save platform Map file
    #
    def _SaveMapFile (self, MapBuffer, Wa):
        #
        # Map file path is got.
        #
        MapFilePath = os.path.join(Wa.BuildDir, Wa.Name + '.map')
        #
        # Save address map into MAP file.
        #
        SaveFileOnChange(MapFilePath, ''.join(MapBuffer), False)
        if self.LoadFixAddress != 0:
            sys.stdout.write ("\nLoad Module At Fix Address Map file can be found at %s\n" % (MapFilePath))
        sys.stdout.flush()

    ## Build active platform for different build targets and different tool chains
    #
    def _BuildPlatform(self):
        SaveFileOnChange(self.PlatformBuildPath, '# DO NOT EDIT \n# FILE auto-generated\n', False)
        for BuildTarget in self.BuildTargetList:
            GlobalData.gGlobalDefines['TARGET'] = BuildTarget
            index = 0
            for ToolChain in self.ToolChainList:
                GlobalData.gGlobalDefines['TOOLCHAIN'] = ToolChain
                GlobalData.gGlobalDefines['TOOL_CHAIN_TAG'] = ToolChain
                GlobalData.gGlobalDefines['FAMILY'] = self.ToolChainFamily[index]
                index += 1
                Wa = WorkspaceAutoGen(
                        self.WorkspaceDir,
                        self.PlatformFile,
                        BuildTarget,
                        ToolChain,
                        self.ArchList,
                        self.BuildDatabase,
                        self.TargetTxt,
                        self.ToolDef,
                        self.Fdf,
                        self.FdList,
                        self.FvList,
                        self.CapList,
                        self.SkuId,
                        self.UniFlag,
                        self.Progress
                        )
                self.Fdf = Wa.FdfFile
                self.LoadFixAddress = Wa.Platform.LoadFixAddress
                self.BuildReport.AddPlatformReport(Wa)
                self.Progress.Stop("done!")

                # Add ffs build to makefile
                CmdListDict = {}
                if GlobalData.gEnableGenfdsMultiThread and self.Fdf:
                    CmdListDict = self._GenFfsCmd(Wa.ArchList)

                for Arch in Wa.ArchList:
                    PcdMaList    = []
                    GlobalData.gGlobalDefines['ARCH'] = Arch
                    Pa = PlatformAutoGen(Wa, self.PlatformFile, BuildTarget, ToolChain, Arch)
                    for Module in Pa.Platform.Modules:
                        # Get ModuleAutoGen object to generate C code file and makefile
                        Ma = ModuleAutoGen(Wa, Module, BuildTarget, ToolChain, Arch, self.PlatformFile,Pa.DataPipe)
                        if Ma is None:
                            continue
                        if Ma.PcdIsDriver:
                            Ma.PlatformInfo = Pa
                            Ma.Workspace = Wa
                            PcdMaList.append(Ma)
                        self.BuildModules.append(Ma)
                    Pa.DataPipe.DataContainer = {"FfsCommand":CmdListDict}
                    Pa.DataPipe.DataContainer = {"Workspace_timestamp": Wa._SrcTimeStamp}
                    self._BuildPa(self.Target, Pa, FfsCommand=CmdListDict,PcdMaList=PcdMaList)

                # Create MAP file when Load Fix Address is enabled.
                if self.Target in ["", "all", "fds"]:
                    for Arch in Wa.ArchList:
                        GlobalData.gGlobalDefines['ARCH'] = Arch
                        #
                        # Check whether the set fix address is above 4G for 32bit image.
                        #
                        if (Arch == 'IA32' or Arch == 'ARM') and self.LoadFixAddress != 0xFFFFFFFFFFFFFFFF and self.LoadFixAddress >= 0x100000000:
                            EdkLogger.error("build", PARAMETER_INVALID, "FIX_LOAD_TOP_MEMORY_ADDRESS can't be set to larger than or equal to 4G for the platform with IA32 or ARM arch modules")
                    #
                    # Get Module List
                    #
                    ModuleList = {}
                    for Pa in Wa.AutoGenObjectList:
                        for Ma in Pa.ModuleAutoGenList:
                            if Ma is None:
                                continue
                            if not Ma.IsLibrary:
                                ModuleList[Ma.Guid.upper()] = Ma

                    MapBuffer = []
                    if self.LoadFixAddress != 0:
                        #
                        # Rebase module to the preferred memory address before GenFds
                        #
                        self._CollectModuleMapBuffer(MapBuffer, ModuleList)
                    if self.Fdf:
                        #
                        # create FDS again for the updated EFI image
                        #
                        self._Build("fds", Wa)
                        #
                        # Create MAP file for all platform FVs after GenFds.
                        #
                        self._CollectFvMapBuffer(MapBuffer, Wa, ModuleList)
                    #
                    # Save MAP buffer into MAP file.
                    #
                    self._SaveMapFile (MapBuffer, Wa)
                self.CreateGuidedSectionToolsFile(Wa)

    ## Build active module for different build targets, different tool chains and different archs
    #
    def _BuildModule(self):
        for BuildTarget in self.BuildTargetList:
            GlobalData.gGlobalDefines['TARGET'] = BuildTarget
            index = 0
            for ToolChain in self.ToolChainList:
                WorkspaceAutoGenTime = time.time()
                GlobalData.gGlobalDefines['TOOLCHAIN'] = ToolChain
                GlobalData.gGlobalDefines['TOOL_CHAIN_TAG'] = ToolChain
                GlobalData.gGlobalDefines['FAMILY'] = self.ToolChainFamily[index]
                index += 1
                #
                # module build needs platform build information, so get platform
                # AutoGen first
                #
                Wa = WorkspaceAutoGen(
                        self.WorkspaceDir,
                        self.PlatformFile,
                        BuildTarget,
                        ToolChain,
                        self.ArchList,
                        self.BuildDatabase,
                        self.TargetTxt,
                        self.ToolDef,
                        self.Fdf,
                        self.FdList,
                        self.FvList,
                        self.CapList,
                        self.SkuId,
                        self.UniFlag,
                        self.Progress,
                        self.ModuleFile
                        )
                self.Fdf = Wa.FdfFile
                self.LoadFixAddress = Wa.Platform.LoadFixAddress
                Wa.CreateMakeFile(False)
                # Add ffs build to makefile
                CmdListDict = None
                if GlobalData.gEnableGenfdsMultiThread and self.Fdf:
                    CmdListDict = self._GenFfsCmd(Wa.ArchList)

                # Add Platform and Package level hash in share_data for module hash calculation later
                if GlobalData.gBinCacheSource or GlobalData.gBinCacheDest:
                    GlobalData.gCacheIR[('PlatformHash')] = GlobalData.gPlatformHash
                    for PkgName in GlobalData.gPackageHash.keys():
                        GlobalData.gCacheIR[(PkgName, 'PackageHash')] = GlobalData.gPackageHash[PkgName]
                GlobalData.file_lock = mp.Lock()
                GlobalData.cache_lock = mp.Lock()
                GlobalData.FfsCmd = CmdListDict

                self.Progress.Stop("done!")
                MaList = []
                ExitFlag = threading.Event()
                ExitFlag.clear()
                self.AutoGenTime += int(round((time.time() - WorkspaceAutoGenTime)))
                for Arch in Wa.ArchList:
                    AutoGenStart = time.time()
                    GlobalData.gGlobalDefines['ARCH'] = Arch
                    Pa = PlatformAutoGen(Wa, self.PlatformFile, BuildTarget, ToolChain, Arch)
                    GlobalData.libConstPcd = Pa.DataPipe.Get("LibConstPcd")
                    GlobalData.Refes = Pa.DataPipe.Get("REFS")
                    for Module in Pa.Platform.Modules:
                        if self.ModuleFile.Dir == Module.Dir and self.ModuleFile.Name == Module.Name:
                            Ma = ModuleAutoGen(Wa, Module, BuildTarget, ToolChain, Arch, self.PlatformFile,Pa.DataPipe)
                            if Ma is None:
                                continue
                            if Ma.PcdIsDriver:
                                Ma.PlatformInfo = Pa
                                Ma.Workspace = Wa
                            MaList.append(Ma)

                            if GlobalData.gBinCacheSource and self.Target in [None, "", "all"]:
                                Ma.GenModuleFilesHash(GlobalData.gCacheIR)
                                Ma.GenPreMakefileHash(GlobalData.gCacheIR)
                                if Ma.CanSkipbyPreMakefileCache(GlobalData.gCacheIR):
                                    self.HashSkipModules.append(Ma)
                                    EdkLogger.quiet("cache hit: %s[%s]" % (Ma.MetaFile.Path, Ma.Arch))
                                    continue

                            # Not to auto-gen for targets 'clean', 'cleanlib', 'cleanall', 'run', 'fds'
                            if self.Target not in ['clean', 'cleanlib', 'cleanall', 'run', 'fds']:
                                # for target which must generate AutoGen code and makefile
                                if not self.SkipAutoGen or self.Target == 'genc':
                                    self.Progress.Start("Generating code")
                                    Ma.CreateCodeFile(True)
                                    self.Progress.Stop("done!")
                                if self.Target == "genc":
                                    return True
                                if not self.SkipAutoGen or self.Target == 'genmake':
                                    self.Progress.Start("Generating makefile")
                                    if CmdListDict and self.Fdf and (Module.Path, Arch) in CmdListDict:
                                        Ma.CreateMakeFile(True, CmdListDict[Module.Path, Arch])
                                        del CmdListDict[Module.Path, Arch]
                                    else:
                                        Ma.CreateMakeFile(True)
                                    self.Progress.Stop("done!")
                                if self.Target == "genmake":
                                    return True

                                if GlobalData.gBinCacheSource and self.Target in [None, "", "all"]:
                                    Ma.GenMakeHeaderFilesHash(GlobalData.gCacheIR)
                                    Ma.GenMakeHash(GlobalData.gCacheIR)
                                    if Ma.CanSkipbyMakeCache(GlobalData.gCacheIR):
                                        self.HashSkipModules.append(Ma)
                                        EdkLogger.quiet("cache hit: %s[%s]" % (Ma.MetaFile.Path, Ma.Arch))
                                        continue
                                    else:
                                        EdkLogger.quiet("cache miss: %s[%s]" % (Ma.MetaFile.Path, Ma.Arch))
                                        Ma.PrintFirstMakeCacheMissFile(GlobalData.gCacheIR)

                            self.BuildModules.append(Ma)
                            # Initialize all modules in tracking to 'FAIL'
                            GlobalData.gModuleBuildTracking[Ma] = 'FAIL'
                    self.AutoGenTime += int(round((time.time() - AutoGenStart)))
                    MakeStart = time.time()
                    for Ma in self.BuildModules:
                        if not Ma.IsBinaryModule:
                            Bt = BuildTask.New(ModuleMakeUnit(Ma, Pa.BuildCommand,self.Target))
                        # Break build if any build thread has error
                        if BuildTask.HasError():
                            # we need a full version of makefile for platform
                            ExitFlag.set()
                            BuildTask.WaitForComplete()
                            self.invalidateHash()
                            Pa.CreateMakeFile(False)
                            EdkLogger.error("build", BUILD_ERROR, "Failed to build module", ExtraData=GlobalData.gBuildingModule)
                        # Start task scheduler
                        if not BuildTask.IsOnGoing():
                            BuildTask.StartScheduler(self.ThreadNumber, ExitFlag)

                    # in case there's an interruption. we need a full version of makefile for platform
                    Pa.CreateMakeFile(False)
                    if BuildTask.HasError():
                        self.invalidateHash()
                        EdkLogger.error("build", BUILD_ERROR, "Failed to build module", ExtraData=GlobalData.gBuildingModule)
                    self.MakeTime += int(round((time.time() - MakeStart)))

                MakeContiue = time.time()
                ExitFlag.set()
                BuildTask.WaitForComplete()
                self.CreateAsBuiltInf()
                if GlobalData.gBinCacheDest:
                    self.UpdateBuildCache()
                self.BuildModules = []
                self.MakeTime += int(round((time.time() - MakeContiue)))
                if BuildTask.HasError():
                    self.invalidateHash()
                    EdkLogger.error("build", BUILD_ERROR, "Failed to build module", ExtraData=GlobalData.gBuildingModule)

                self.BuildReport.AddPlatformReport(Wa, MaList)
                if MaList == []:
                    EdkLogger.error(
                                'build',
                                BUILD_ERROR,
                                "Module for [%s] is not a component of active platform."\
                                " Please make sure that the ARCH and inf file path are"\
                                " given in the same as in [%s]" % \
                                    (', '.join(Wa.ArchList), self.PlatformFile),
                                ExtraData=self.ModuleFile
                                )
                # Create MAP file when Load Fix Address is enabled.
                if self.Target == "fds" and self.Fdf:
                    for Arch in Wa.ArchList:
                        #
                        # Check whether the set fix address is above 4G for 32bit image.
                        #
                        if (Arch == 'IA32' or Arch == 'ARM') and self.LoadFixAddress != 0xFFFFFFFFFFFFFFFF and self.LoadFixAddress >= 0x100000000:
                            EdkLogger.error("build", PARAMETER_INVALID, "FIX_LOAD_TOP_MEMORY_ADDRESS can't be set to larger than or equal to 4G for the platorm with IA32 or ARM arch modules")
                    #
                    # Get Module List
                    #
                    ModuleList = {}
                    for Pa in Wa.AutoGenObjectList:
                        for Ma in Pa.ModuleAutoGenList:
                            if Ma is None:
                                continue
                            if not Ma.IsLibrary:
                                ModuleList[Ma.Guid.upper()] = Ma

                    MapBuffer = []
                    if self.LoadFixAddress != 0:
                        #
                        # Rebase module to the preferred memory address before GenFds
                        #
                        self._CollectModuleMapBuffer(MapBuffer, ModuleList)
                    #
                    # create FDS again for the updated EFI image
                    #
                    GenFdsStart = time.time()
                    self._Build("fds", Wa)
                    self.GenFdsTime += int(round((time.time() - GenFdsStart)))
                    #
                    # Create MAP file for all platform FVs after GenFds.
                    #
                    self._CollectFvMapBuffer(MapBuffer, Wa, ModuleList)
                    #
                    # Save MAP buffer into MAP file.
                    #
                    self._SaveMapFile (MapBuffer, Wa)
        self.invalidateHash()

    def _GenFfsCmd(self,ArchList):
        # convert dictionary of Cmd:(Inf,Arch)
        # to a new dictionary of (Inf,Arch):Cmd,Cmd,Cmd...
        CmdSetDict = defaultdict(set)
        GenFfsDict = GenFds.GenFfsMakefile('', GlobalData.gFdfParser, self, ArchList, GlobalData)
        for Cmd in GenFfsDict:
            tmpInf, tmpArch = GenFfsDict[Cmd]
            CmdSetDict[tmpInf, tmpArch].add(Cmd)
        return CmdSetDict
    def VerifyAutoGenFiles(self):
        AutoGenIdFile = os.path.join(GlobalData.gConfDirectory,".AutoGenIdFile.txt")
        try:
            with open(AutoGenIdFile) as fd:
                lines = fd.readlines()
        except:
            return None
        for line in lines:
            if "Arch" in line:
                ArchList = line.strip().split("=")[1].split("|")
            if "BuildDir" in line:
                BuildDir = line.split("=")[1].strip()
            if "PlatformGuid" in line:
                PlatformGuid = line.split("=")[1].strip()
        GlobalVarList = []
        for arch in ArchList:
            global_var = os.path.join(BuildDir, "GlobalVar_%s_%s.bin" % (str(PlatformGuid),arch))
            if not os.path.exists(global_var):
                return None
            GlobalVarList.append(global_var)
        for global_var in GlobalVarList:
            data_pipe = MemoryDataPipe()
            data_pipe.load(global_var)
            target = data_pipe.Get("P_Info").get("Target")
            toolchain = data_pipe.Get("P_Info").get("ToolChain")
            archlist = data_pipe.Get("P_Info").get("ArchList")
            Arch = data_pipe.Get("P_Info").get("Arch")
            active_p = data_pipe.Get("P_Info").get("ActivePlatform")
            workspacedir = data_pipe.Get("P_Info").get("WorkspaceDir")
            PackagesPath = os.getenv("PACKAGES_PATH")
            mws.setWs(workspacedir, PackagesPath)
            LibraryBuildDirectoryList = data_pipe.Get("LibraryBuildDirectoryList")
            ModuleBuildDirectoryList = data_pipe.Get("ModuleBuildDirectoryList")

            for m_build_dir in LibraryBuildDirectoryList:
                if not os.path.exists(os.path.join(m_build_dir,GenMake.BuildFile._FILE_NAME_[GenMake.gMakeType])):
                    return None
            for m_build_dir in ModuleBuildDirectoryList:
                if not os.path.exists(os.path.join(m_build_dir,GenMake.BuildFile._FILE_NAME_[GenMake.gMakeType])):
                    return None
            Wa = WorkSpaceInfo(
                workspacedir,active_p,target,toolchain,archlist
                )
            Pa = PlatformInfo(Wa, active_p, target, toolchain, Arch,data_pipe)
            Wa.AutoGenObjectList.append(Pa)
        return Wa
    def SetupMakeSetting(self,Wa):
        BuildModules = []
        for Pa in Wa.AutoGenObjectList:
            for m in Pa._MbList:
                ma = ModuleAutoGen(Wa,m.MetaFile, Pa.BuildTarget, Wa.ToolChain, Pa.Arch, Pa.MetaFile,Pa.DataPipe)
                BuildModules.append(ma)
        fdf_file = Wa.FlashDefinition
        if fdf_file:
            Fdf = FdfParser(fdf_file.Path)
            Fdf.ParseFile()
            GlobalData.gFdfParser = Fdf
            if Fdf.CurrentFdName and Fdf.CurrentFdName in Fdf.Profile.FdDict:
                FdDict = Fdf.Profile.FdDict[Fdf.CurrentFdName]
                for FdRegion in FdDict.RegionList:
                    if str(FdRegion.RegionType) == 'FILE' and self.Platform.VpdToolGuid in str(FdRegion.RegionDataList):
                        if int(FdRegion.Offset) % 8 != 0:
                            EdkLogger.error("build", FORMAT_INVALID, 'The VPD Base Address %s must be 8-byte aligned.' % (FdRegion.Offset))
            Wa.FdfProfile = Fdf.Profile
            self.Fdf = Fdf
        else:
            self.Fdf = None
        return BuildModules

    ## Build a platform in multi-thread mode
    #
    def PerformAutoGen(self,BuildTarget,ToolChain):
        WorkspaceAutoGenTime = time.time()
        Wa = WorkspaceAutoGen(
                self.WorkspaceDir,
                self.PlatformFile,
                BuildTarget,
                ToolChain,
                self.ArchList,
                self.BuildDatabase,
                self.TargetTxt,
                self.ToolDef,
                self.Fdf,
                self.FdList,
                self.FvList,
                self.CapList,
                self.SkuId,
                self.UniFlag,
                self.Progress
                )
        self.Fdf = Wa.FdfFile
        self.LoadFixAddress = Wa.Platform.LoadFixAddress
        self.BuildReport.AddPlatformReport(Wa)
        Wa.CreateMakeFile(False)

                # Add ffs build to makefile
        CmdListDict = {}
        if GlobalData.gEnableGenfdsMultiThread and self.Fdf:
            CmdListDict = self._GenFfsCmd(Wa.ArchList)

        # Add Platform and Package level hash in share_data for module hash calculation later
        if GlobalData.gBinCacheSource or GlobalData.gBinCacheDest:
            GlobalData.gCacheIR[('PlatformHash')] = GlobalData.gPlatformHash
            for PkgName in GlobalData.gPackageHash.keys():
                GlobalData.gCacheIR[(PkgName, 'PackageHash')] = GlobalData.gPackageHash[PkgName]

        self.AutoGenTime += int(round((time.time() - WorkspaceAutoGenTime)))
        BuildModules = []
        TotalModules = []
        for Arch in Wa.ArchList:
            PcdMaList    = []
            AutoGenStart = time.time()
            GlobalData.gGlobalDefines['ARCH'] = Arch
            Pa = PlatformAutoGen(Wa, self.PlatformFile, BuildTarget, ToolChain, Arch)
            if Pa is None:
                continue
            ModuleList = []
            for Inf in Pa.Platform.Modules:
                ModuleList.append(Inf)
                    # Add the INF only list in FDF
            if GlobalData.gFdfParser is not None:
                for InfName in GlobalData.gFdfParser.Profile.InfList:
                    Inf = PathClass(NormPath(InfName), self.WorkspaceDir, Arch)
                    if Inf in Pa.Platform.Modules:
                        continue
                    ModuleList.append(Inf)
            Pa.DataPipe.DataContainer = {"FfsCommand":CmdListDict}
            Pa.DataPipe.DataContainer = {"Workspace_timestamp": Wa._SrcTimeStamp}
            Pa.DataPipe.DataContainer = {"CommandTarget": self.Target}
            Pa.CreateLibModuelDirs()
            Pa.DataPipe.DataContainer = {"LibraryBuildDirectoryList":Pa.LibraryBuildDirectoryList}
            Pa.DataPipe.DataContainer = {"ModuleBuildDirectoryList":Pa.ModuleBuildDirectoryList}
            Pa.DataPipe.DataContainer = {"FdsCommandDict": Wa.GenFdsCommandDict}
            ModuleCodaFile = {}
            for ma in Pa.ModuleAutoGenList:
                ModuleCodaFile[(ma.MetaFile.File,ma.MetaFile.Root,ma.Arch,ma.MetaFile.Path)] = [item.Target for item in ma.CodaTargetList]
            Pa.DataPipe.DataContainer = {"ModuleCodaFile":ModuleCodaFile}
            for Module in ModuleList:
                        # Get ModuleAutoGen object to generate C code file and makefile
                Ma = ModuleAutoGen(Wa, Module, BuildTarget, ToolChain, Arch, self.PlatformFile,Pa.DataPipe)

                if Ma is None:
                    continue
                if Ma.PcdIsDriver:
                    Ma.PlatformInfo = Pa
                    Ma.Workspace = Wa
                    PcdMaList.append(Ma)
                TotalModules.append(Ma)
                # Initialize all modules in tracking to 'FAIL'
                GlobalData.gModuleBuildTracking[Ma] = 'FAIL'


            mqueue = mp.Queue()
            for m in Pa.GetAllModuleInfo:
                mqueue.put(m)
            data_pipe_file = os.path.join(Pa.BuildDir, "GlobalVar_%s_%s.bin" % (str(Pa.Guid),Pa.Arch))
            Pa.DataPipe.dump(data_pipe_file)

            autogen_rt, errorcode = self.StartAutoGen(mqueue, Pa.DataPipe, self.SkipAutoGen, PcdMaList,GlobalData.gCacheIR)

            # Skip cache hit modules
            if GlobalData.gBinCacheSource:
                for Ma in TotalModules:
                    if (Ma.MetaFile.Path, Ma.Arch) in GlobalData.gCacheIR and \
                        GlobalData.gCacheIR[(Ma.MetaFile.Path, Ma.Arch)].PreMakeCacheHit:
                            self.HashSkipModules.append(Ma)
                            continue
                    if (Ma.MetaFile.Path, Ma.Arch) in GlobalData.gCacheIR and \
                        GlobalData.gCacheIR[(Ma.MetaFile.Path, Ma.Arch)].MakeCacheHit:
                            self.HashSkipModules.append(Ma)
                            continue
                    BuildModules.append(Ma)
            else:
                BuildModules.extend(TotalModules)

            if not autogen_rt:
                self.AutoGenMgr.TerminateWorkers()
                self.AutoGenMgr.join(1)
                raise FatalError(errorcode)
            self.AutoGenTime += int(round((time.time() - AutoGenStart)))
        AutoGenIdFile = os.path.join(GlobalData.gConfDirectory,".AutoGenIdFile.txt")
        with open(AutoGenIdFile,"w") as fw:
            fw.write("Arch=%s\n" % "|".join((Wa.ArchList)))
            fw.write("BuildDir=%s\n" % Wa.BuildDir)
            fw.write("PlatformGuid=%s\n" % str(Wa.AutoGenObjectList[0].Guid))
        self.Progress.Stop("done!")
        return Wa, BuildModules

    def _MultiThreadBuildPlatform(self):
        SaveFileOnChange(self.PlatformBuildPath, '# DO NOT EDIT \n# FILE auto-generated\n', False)
        for BuildTarget in self.BuildTargetList:
            GlobalData.gGlobalDefines['TARGET'] = BuildTarget
            index = 0
            for ToolChain in self.ToolChainList:
                GlobalData.gGlobalDefines['TOOLCHAIN'] = ToolChain
                GlobalData.gGlobalDefines['TOOL_CHAIN_TAG'] = ToolChain
                GlobalData.gGlobalDefines['FAMILY'] = self.ToolChainFamily[index]
                index += 1
                ExitFlag = threading.Event()
                ExitFlag.clear()
                if self.SkipAutoGen:
                    Wa = self.VerifyAutoGenFiles()
                    if Wa is None:
                        self.SkipAutoGen = False
                        Wa, self.BuildModules = self.PerformAutoGen(BuildTarget,ToolChain)
                    else:
                        GlobalData.gAutoGenPhase = True
                        self.BuildModules = self.SetupMakeSetting(Wa)
                else:
                    Wa, self.BuildModules = self.PerformAutoGen(BuildTarget,ToolChain)
                Pa = Wa.AutoGenObjectList[0]
                GlobalData.gAutoGenPhase = False

                if GlobalData.gBinCacheSource:
                    EdkLogger.quiet("Total cache hit driver num: %s, cache miss driver num: %s" % (len(set(self.HashSkipModules)), len(set(self.BuildModules))))
                    CacheHitMa = set()
                    CacheNotHitMa = set()
                    for IR in GlobalData.gCacheIR.keys():
                        if 'PlatformHash' in IR or 'PackageHash' in IR:
                            continue
                        if GlobalData.gCacheIR[IR].PreMakeCacheHit or GlobalData.gCacheIR[IR].MakeCacheHit:
                            CacheHitMa.add(IR)
                        else:
                            # There might be binary module or module which has .inc files, not count for cache miss
                            CacheNotHitMa.add(IR)
                    EdkLogger.quiet("Total module num: %s, cache hit module num: %s" % (len(CacheHitMa)+len(CacheNotHitMa), len(CacheHitMa)))

                for Arch in Wa.ArchList:
                    MakeStart = time.time()
                    for Ma in set(self.BuildModules):
                        # Generate build task for the module
                        if not Ma.IsBinaryModule:
                            Bt = BuildTask.New(ModuleMakeUnit(Ma, Pa.BuildCommand,self.Target))
                        # Break build if any build thread has error
                        if BuildTask.HasError():
                            # we need a full version of makefile for platform
                            ExitFlag.set()
                            BuildTask.WaitForComplete()
                            self.invalidateHash()
                            Pa.CreateMakeFile(False)
                            EdkLogger.error("build", BUILD_ERROR, "Failed to build module", ExtraData=GlobalData.gBuildingModule)
                        # Start task scheduler
                        if not BuildTask.IsOnGoing():
                            BuildTask.StartScheduler(self.ThreadNumber, ExitFlag)

                    # in case there's an interruption. we need a full version of makefile for platform

                    if BuildTask.HasError():
                        self.invalidateHash()
                        EdkLogger.error("build", BUILD_ERROR, "Failed to build module", ExtraData=GlobalData.gBuildingModule)
                    self.MakeTime += int(round((time.time() - MakeStart)))

                MakeContiue = time.time()
                #
                #
                # All modules have been put in build tasks queue. Tell task scheduler
                # to exit if all tasks are completed
                #
                ExitFlag.set()
                BuildTask.WaitForComplete()
                self.CreateAsBuiltInf()
                if GlobalData.gBinCacheDest:
                    self.UpdateBuildCache()
                self.BuildModules = []
                self.MakeTime += int(round((time.time() - MakeContiue)))
                #
                # Check for build error, and raise exception if one
                # has been signaled.
                #
                if BuildTask.HasError():
                    self.invalidateHash()
                    EdkLogger.error("build", BUILD_ERROR, "Failed to build module", ExtraData=GlobalData.gBuildingModule)

                # Create MAP file when Load Fix Address is enabled.
                if self.Target in ["", "all", "fds"]:
                    for Arch in Wa.ArchList:
                        #
                        # Check whether the set fix address is above 4G for 32bit image.
                        #
                        if (Arch == 'IA32' or Arch == 'ARM') and self.LoadFixAddress != 0xFFFFFFFFFFFFFFFF and self.LoadFixAddress >= 0x100000000:
                            EdkLogger.error("build", PARAMETER_INVALID, "FIX_LOAD_TOP_MEMORY_ADDRESS can't be set to larger than or equal to 4G for the platorm with IA32 or ARM arch modules")
                    #
                    # Get Module List
                    #
                    ModuleList = {ma.Guid.upper():ma for ma in self.BuildModules}

                    #
                    # Rebase module to the preferred memory address before GenFds
                    #
                    MapBuffer = []
                    if self.LoadFixAddress != 0:
                        self._CollectModuleMapBuffer(MapBuffer, ModuleList)

                    if self.Fdf:
                        #
                        # Generate FD image if there's a FDF file found
                        #
                        GenFdsStart = time.time()
                        if GenFdsApi(Wa.GenFdsCommandDict, self.Db):
                            EdkLogger.error("build", COMMAND_FAILURE)

                        #
                        # Create MAP file for all platform FVs after GenFds.
                        #
                        self._CollectFvMapBuffer(MapBuffer, Wa, ModuleList)
                        self.GenFdsTime += int(round((time.time() - GenFdsStart)))
                    #
                    # Save MAP buffer into MAP file.
                    #
                    self._SaveMapFile(MapBuffer, Wa)
                self.CreateGuidedSectionToolsFile(Wa)
        self.invalidateHash()
    ## Generate GuidedSectionTools.txt in the FV directories.
    #
    def CreateGuidedSectionToolsFile(self,Wa):
        for BuildTarget in self.BuildTargetList:
            for ToolChain in self.ToolChainList:
                FvDir = Wa.FvDir
                if not os.path.exists(FvDir):
                    continue

                for Arch in self.ArchList:
                    # Build up the list of supported architectures for this build
                    prefix = '%s_%s_%s_' % (BuildTarget, ToolChain, Arch)

                    # Look through the tool definitions for GUIDed tools
                    guidAttribs = []
                    for (attrib, value) in self.ToolDef.ToolsDefTxtDictionary.items():
                        if attrib.upper().endswith('_GUID'):
                            split = attrib.split('_')
                            thisPrefix = '_'.join(split[0:3]) + '_'
                            if thisPrefix == prefix:
                                guid = self.ToolDef.ToolsDefTxtDictionary[attrib]
                                guid = guid.lower()
                                toolName = split[3]
                                path = '_'.join(split[0:4]) + '_PATH'
                                path = self.ToolDef.ToolsDefTxtDictionary[path]
                                path = self.GetFullPathOfTool(path)
                                guidAttribs.append((guid, toolName, path))

                    # Write out GuidedSecTools.txt
                    toolsFile = os.path.join(FvDir, 'GuidedSectionTools.txt')
                    toolsFile = open(toolsFile, 'wt')
                    for guidedSectionTool in guidAttribs:
                        print(' '.join(guidedSectionTool), file=toolsFile)
                    toolsFile.close()

    ## Returns the full path of the tool.
    #
    def GetFullPathOfTool (self, tool):
        if os.path.exists(tool):
            return os.path.realpath(tool)
        else:
            # We need to search for the tool using the
            # PATH environment variable.
            for dirInPath in os.environ['PATH'].split(os.pathsep):
                foundPath = os.path.join(dirInPath, tool)
                if os.path.exists(foundPath):
                    return os.path.realpath(foundPath)

        # If the tool was not found in the path then we just return
        # the input tool.
        return tool

    ## Launch the module or platform build
    #
    def Launch(self):
        if not self.ModuleFile:
            if not self.SpawnMode or self.Target not in ["", "all"]:
                self.SpawnMode = False
                self._BuildPlatform()
            else:
                self._MultiThreadBuildPlatform()
        else:
            self.SpawnMode = False
            self._BuildModule()

        if self.Target == 'cleanall':
            RemoveDirectory(os.path.dirname(GlobalData.gDatabasePath), True)

    def CreateAsBuiltInf(self):
        for Module in self.BuildModules:
            Module.CreateAsBuiltInf()

    def UpdateBuildCache(self):
        all_lib_set = set()
        all_mod_set = set()
        for Module in self.BuildModules:
            Module.CopyModuleToCache()
            all_mod_set.add(Module)
        for Module in self.HashSkipModules:
            Module.CopyModuleToCache()
            all_mod_set.add(Module)
        for Module in all_mod_set:
            for lib in Module.LibraryAutoGenList:
                all_lib_set.add(lib)
        for lib in all_lib_set:
            lib.CopyModuleToCache()
        all_lib_set.clear()
        all_mod_set.clear()
        self.HashSkipModules = []
    ## Do some clean-up works when error occurred
    def Relinquish(self):
        OldLogLevel = EdkLogger.GetLevel()
        EdkLogger.SetLevel(EdkLogger.ERROR)
        Utils.Progressor.Abort()
        if self.SpawnMode == True:
            BuildTask.Abort()
        EdkLogger.SetLevel(OldLogLevel)

def ParseDefines(DefineList=[]):
    DefineDict = {}
    if DefineList is not None:
        for Define in DefineList:
            DefineTokenList = Define.split("=", 1)
            if not GlobalData.gMacroNamePattern.match(DefineTokenList[0]):
                EdkLogger.error('build', FORMAT_INVALID,
                                "The macro name must be in the pattern [A-Z][A-Z0-9_]*",
                                ExtraData=DefineTokenList[0])

            if len(DefineTokenList) == 1:
                DefineDict[DefineTokenList[0]] = "TRUE"
            else:
                DefineDict[DefineTokenList[0]] = DefineTokenList[1].strip()
    return DefineDict



def LogBuildTime(Time):
    if Time:
        TimeDurStr = ''
        TimeDur = time.gmtime(Time)
        if TimeDur.tm_yday > 1:
            TimeDurStr = time.strftime("%H:%M:%S", TimeDur) + ", %d day(s)" % (TimeDur.tm_yday - 1)
        else:
            TimeDurStr = time.strftime("%H:%M:%S", TimeDur)
        return TimeDurStr
    else:
        return None
def ThreadNum():
    ThreadNumber = BuildOption.ThreadNumber
    if ThreadNumber is None:
        ThreadNumber = TargetTxt.TargetTxtDictionary[TAB_TAT_DEFINES_MAX_CONCURRENT_THREAD_NUMBER]
        if ThreadNumber == '':
            ThreadNumber = 0
        else:
            ThreadNumber = int(ThreadNumber, 0)

    if ThreadNumber == 0:
        try:
            ThreadNumber = multiprocessing.cpu_count()
        except (ImportError, NotImplementedError):
            ThreadNumber = 1
    return ThreadNumber
## Tool entrance method
#
# This method mainly dispatch specific methods per the command line options.
# If no error found, return zero value so the caller of this tool can know
# if it's executed successfully or not.
#
#   @retval 0     Tool was successful
#   @retval 1     Tool failed
#
LogQMaxSize = ThreadNum() * 10
def Main():
    StartTime = time.time()

    #
    # Create a log Queue
    #
    LogQ = mp.Queue(LogQMaxSize)
    # Initialize log system
    EdkLogger.LogClientInitialize(LogQ)
    GlobalData.gCommand = sys.argv[1:]
    #
    # Parse the options and args
    #
    Option, Target = BuildOption, BuildTarget
    GlobalData.gOptions = Option
    GlobalData.gCaseInsensitive = Option.CaseInsensitive

    # Set log level
    LogLevel = EdkLogger.INFO
    if Option.verbose is not None:
        EdkLogger.SetLevel(EdkLogger.VERBOSE)
        LogLevel = EdkLogger.VERBOSE
    elif Option.quiet is not None:
        EdkLogger.SetLevel(EdkLogger.QUIET)
        LogLevel = EdkLogger.QUIET
    elif Option.debug is not None:
        EdkLogger.SetLevel(Option.debug + 1)
        LogLevel = Option.debug + 1
    else:
        EdkLogger.SetLevel(EdkLogger.INFO)

    if Option.WarningAsError == True:
        EdkLogger.SetWarningAsError()
    Log_Agent = LogAgent(LogQ,LogLevel,Option.LogFile)
    Log_Agent.start()

    if platform.platform().find("Windows") >= 0:
        GlobalData.gIsWindows = True
    else:
        GlobalData.gIsWindows = False

    EdkLogger.quiet("Build environment: %s" % platform.platform())
    EdkLogger.quiet(time.strftime("Build start time: %H:%M:%S, %b.%d %Y\n", time.localtime()));
    ReturnCode = 0
    MyBuild = None
    BuildError = True
    try:
        if len(Target) == 0:
            Target = "all"
        elif len(Target) >= 2:
            EdkLogger.error("build", OPTION_NOT_SUPPORTED, "More than one targets are not supported.",
                            ExtraData="Please select one of: %s" % (' '.join(gSupportedTarget)))
        else:
            Target = Target[0].lower()

        if Target not in gSupportedTarget:
            EdkLogger.error("build", OPTION_NOT_SUPPORTED, "Not supported target [%s]." % Target,
                            ExtraData="Please select one of: %s" % (' '.join(gSupportedTarget)))

        #
        # Check environment variable: EDK_TOOLS_PATH, WORKSPACE, PATH
        #
        CheckEnvVariable()
        GlobalData.gCommandLineDefines.update(ParseDefines(Option.Macros))

        Workspace = os.getenv("WORKSPACE")
        #
        # Get files real name in workspace dir
        #
        GlobalData.gAllFiles = Utils.DirCache(Workspace)

        WorkingDirectory = os.getcwd()
        if not Option.ModuleFile:
            FileList = glob.glob(os.path.normpath(os.path.join(WorkingDirectory, '*.inf')))
            FileNum = len(FileList)
            if FileNum >= 2:
                EdkLogger.error("build", OPTION_NOT_SUPPORTED, "There are %d INF files in %s." % (FileNum, WorkingDirectory),
                                ExtraData="Please use '-m <INF_FILE_PATH>' switch to choose one.")
            elif FileNum == 1:
                Option.ModuleFile = NormFile(FileList[0], Workspace)

        if Option.ModuleFile:
            if os.path.isabs (Option.ModuleFile):
                if os.path.normcase (os.path.normpath(Option.ModuleFile)).find (Workspace) == 0:
                    Option.ModuleFile = NormFile(os.path.normpath(Option.ModuleFile), Workspace)
            Option.ModuleFile = PathClass(Option.ModuleFile, Workspace)
            ErrorCode, ErrorInfo = Option.ModuleFile.Validate(".inf", False)
            if ErrorCode != 0:
                EdkLogger.error("build", ErrorCode, ExtraData=ErrorInfo)

        if Option.PlatformFile is not None:
            if os.path.isabs (Option.PlatformFile):
                if os.path.normcase (os.path.normpath(Option.PlatformFile)).find (Workspace) == 0:
                    Option.PlatformFile = NormFile(os.path.normpath(Option.PlatformFile), Workspace)
            Option.PlatformFile = PathClass(Option.PlatformFile, Workspace)

        if Option.FdfFile is not None:
            if os.path.isabs (Option.FdfFile):
                if os.path.normcase (os.path.normpath(Option.FdfFile)).find (Workspace) == 0:
                    Option.FdfFile = NormFile(os.path.normpath(Option.FdfFile), Workspace)
            Option.FdfFile = PathClass(Option.FdfFile, Workspace)
            ErrorCode, ErrorInfo = Option.FdfFile.Validate(".fdf", False)
            if ErrorCode != 0:
                EdkLogger.error("build", ErrorCode, ExtraData=ErrorInfo)

        if Option.Flag is not None and Option.Flag not in ['-c', '-s']:
            EdkLogger.error("build", OPTION_VALUE_INVALID, "UNI flag must be one of -c or -s")

        MyBuild = Build(Target, Workspace, Option,LogQ)
        GlobalData.gCommandLineDefines['ARCH'] = ' '.join(MyBuild.ArchList)
        if not (MyBuild.LaunchPrebuildFlag and os.path.exists(MyBuild.PlatformBuildPath)):
            MyBuild.Launch()

        #
        # All job done, no error found and no exception raised
        #
        BuildError = False
    except FatalError as X:
        if MyBuild is not None:
            # for multi-thread build exits safely
            MyBuild.Relinquish()
        if Option is not None and Option.debug is not None:
            EdkLogger.quiet("(Python %s on %s) " % (platform.python_version(), sys.platform) + traceback.format_exc())
        ReturnCode = X.args[0]
    except Warning as X:
        # error from Fdf parser
        if MyBuild is not None:
            # for multi-thread build exits safely
            MyBuild.Relinquish()
        if Option is not None and Option.debug is not None:
            EdkLogger.quiet("(Python %s on %s) " % (platform.python_version(), sys.platform) + traceback.format_exc())
        else:
            EdkLogger.error(X.ToolName, FORMAT_INVALID, File=X.FileName, Line=X.LineNumber, ExtraData=X.Message, RaiseError=False)
        ReturnCode = FORMAT_INVALID
    except KeyboardInterrupt:
        if MyBuild is not None:

            # for multi-thread build exits safely
            MyBuild.Relinquish()
        ReturnCode = ABORT_ERROR
        if Option is not None and Option.debug is not None:
            EdkLogger.quiet("(Python %s on %s) " % (platform.python_version(), sys.platform) + traceback.format_exc())
    except:
        if MyBuild is not None:
            # for multi-thread build exits safely
            MyBuild.Relinquish()

        # try to get the meta-file from the object causing exception
        Tb = sys.exc_info()[-1]
        MetaFile = GlobalData.gProcessingFile
        while Tb is not None:
            if 'self' in Tb.tb_frame.f_locals and hasattr(Tb.tb_frame.f_locals['self'], 'MetaFile'):
                MetaFile = Tb.tb_frame.f_locals['self'].MetaFile
            Tb = Tb.tb_next
        EdkLogger.error(
                    "\nbuild",
                    CODE_ERROR,
                    "Unknown fatal error when processing [%s]" % MetaFile,
                    ExtraData="\n(Please send email to %s for help, attaching following call stack trace!)\n" % MSG_EDKII_MAIL_ADDR,
                    RaiseError=False
                    )
        EdkLogger.quiet("(Python %s on %s) " % (platform.python_version(), sys.platform) + traceback.format_exc())
        ReturnCode = CODE_ERROR
    finally:
        Utils.Progressor.Abort()
        Utils.ClearDuplicatedInf()

    if ReturnCode == 0:
        try:
            MyBuild.LaunchPostbuild()
            Conclusion = "Done"
        except:
            Conclusion = "Failed"
    elif ReturnCode == ABORT_ERROR:
        Conclusion = "Aborted"
    else:
        Conclusion = "Failed"
    FinishTime = time.time()
    BuildDuration = time.gmtime(int(round(FinishTime - StartTime)))
    BuildDurationStr = ""
    if BuildDuration.tm_yday > 1:
        BuildDurationStr = time.strftime("%H:%M:%S", BuildDuration) + ", %d day(s)" % (BuildDuration.tm_yday - 1)
    else:
        BuildDurationStr = time.strftime("%H:%M:%S", BuildDuration)
    if MyBuild is not None:
        if not BuildError:
            MyBuild.BuildReport.GenerateReport(BuildDurationStr, LogBuildTime(MyBuild.AutoGenTime), LogBuildTime(MyBuild.MakeTime), LogBuildTime(MyBuild.GenFdsTime))

    EdkLogger.SetLevel(EdkLogger.QUIET)
    EdkLogger.quiet("\n- %s -" % Conclusion)
    EdkLogger.quiet(time.strftime("Build end time: %H:%M:%S, %b.%d %Y", time.localtime()))
    EdkLogger.quiet("Build total time: %s\n" % BuildDurationStr)
    Log_Agent.kill()
    Log_Agent.join()
    return ReturnCode

if __name__ == '__main__':
    try:
        mp.set_start_method('spawn')
    except:
        pass
    r = Main()
    ## 0-127 is a safe return range, and 1 is a standard default error
    if r < 0 or r > 127: r = 1
    sys.exit(r)

