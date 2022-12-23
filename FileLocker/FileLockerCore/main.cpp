#include <iostream>
#include "ModuleManager.h"

int main()
{
    std::cout << "Hello World! " << std::endl;

    module_manager::NtDllExt ntDllExt;

    std::vector<common::HandleInfo> handlesInfo = ntDllExt.GetAllFileHandles();

    std::cout << "End...!" << std::endl;
    return 0;
}
