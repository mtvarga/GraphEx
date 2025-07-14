#include "GraphEx.h"


using namespace GraphEx;


std::filesystem::path GraphEx::getRuntimeDataDirectory()
{
    return Falcor::getRuntimeDirectory() / "Data";
}
