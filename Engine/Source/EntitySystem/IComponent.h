#pragma once

#include "TypeSystem.h"
#include "UUID.h"

struct IComponent
{
    friend class Entity;
    
    IComponent() : id(Uuid::New()) {}

    REFLECT_DERIVED()

    Uuid GetId() const { return id; }
    Uuid GetEntityId() const { return owningEntityId; }

private:
    Uuid id;
    Uuid owningEntityId;
};
