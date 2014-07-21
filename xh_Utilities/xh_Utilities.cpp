#include "xh_Utilities.h"

using namespace juce;

///////////////////////////////////////////////////////////////////////////////

#include "misc/DestructionNotifier.cpp"
#include "misc/ArrayDuplicateScanner.cpp"
#include "misc/RelativeWeightSequence.cpp"
#include "misc/Version.cpp"

#include "tasks/TaskSequence.cpp"
#include "tasks/ProgressiveTask.cpp"
#include "tasks/DummyTask.cpp"
#include "tasks/SerialTask.cpp"
#include "tasks/MemberFunctionTask.cpp"
#include "tasks/TaskHandler.cpp"

#include "tasks/execution/TaskThreadPoolJob.cpp"
#include "tasks/execution/TaskQueue.cpp"
#include "tasks/execution/TaskThread.cpp"
#include "tasks/execution/PooledTaskRunner.cpp"
#include "tasks/execution/PooledTaskListView.cpp"
#include "tasks/execution/ModalTaskPopup.cpp"
#include "tasks/execution/TaskThreadWithProgressWindow.cpp"

///////////////////////////////////////////////////////////////////////////////
