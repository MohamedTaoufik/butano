#include "btn_cameras.h"

#include "btn_cameras_manager.h"

namespace btn::cameras
{

int used_items_count()
{
    return cameras_manager::used_items_count();
}

int available_items_count()
{
    return cameras_manager::available_items_count();
}

}