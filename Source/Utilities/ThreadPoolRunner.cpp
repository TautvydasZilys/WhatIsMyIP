#include "PrecompiledHeader.h"
#include "ThreadPoolRunner.h"

WRL::ComPtr<ABI::Windows::System::Threading::IThreadPoolStatics> Utilities::ThreadPoolRunner::s_ThreadPool;