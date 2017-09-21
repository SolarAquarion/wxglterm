#pragma once

#include "plugin.h"

class MultipleInstancePlugin : public virtual Plugin {
public:
    MultipleInstancePlugin() = default;
    virtual ~MultipleInstancePlugin() = default;

public:
    virtual MultipleInstancePlugin * NewInstance() = 0;
};
