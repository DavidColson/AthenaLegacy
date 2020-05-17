#include "SceneSerializer.h"

eastl::string SceneSerializer::Serialize(Scene& scene)
{
    eastl::string result;
    for (EntityID entity : SceneView<>(scene))
	{
        result.append_sprintf("ENTITY_BEGIN\n");

        for (int i = 0; i < MAX_COMPONENTS; i++)
        {
            // For each component ID, check the bitmask, if no, continue, if yes, proceed to access that components data
            eastl::bitset<MAX_COMPONENTS> mask;
            mask.set(i, true);
            if (mask == (scene.entities[entity.Index()].mask & mask))
            {
			    TypeData* pComponentType = scene.componentPools[i]->pTypeData;

                if (!TypeDatabase::TypeExists(pComponentType->name))
                    continue; // Don't save out components that have no type data

				void* pComponentData = scene.componentPools[i]->get(entity.Index());

                // We'll manually construct this variant, since we don't know have the type
                Variant var;
                var.pData = new char[pComponentType->size];
                var.pTypeData = pComponentType;
                memcpy(var.pData, pComponentData, pComponentType->size);
                
                result.append(pComponentType->Serialize(var));
            }
        }

        result.append_sprintf("\n\n");
    }
    return result;
}

// Parsing will go like this.

// While loop over each line, continue if you reach a newline

// If a line contains "ENTITY_BEGIN" start a new entity (make new entity)
// If a line contains [SOMETHING] then
//      Loop over every line until you find another [SOMETHING]
//      Take text up to that point and deserialize into variant, Assign this component to the entity (how tf do we do this?)
//      Memcpy the component data into the component slot
//      Move on to next component
