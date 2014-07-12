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
#include "tasks/TaskThread.cpp"

#include "tasks/ModalTaskPopup.cpp"
#include "tasks/TaskThreadWithProgressWindow.cpp"

///////////////////////////////////////////////////////////////////////////////
