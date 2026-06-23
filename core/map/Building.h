#pragma once
#include <string>

namespace core {

struct Building {
    int id;
    std::string name;
    std::string info;
    int ui_x;
    int ui_y;
    int hitbox_radius;
    int entrance_node_id; // 【关键】连接业务与底层算法的桥梁
};

} // namespace core