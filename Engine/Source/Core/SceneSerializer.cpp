#include "SceneSerializer.h"

// ***********************************************************************

JsonValue SceneSerializer::ToJson(Scene& scene)
{
    JsonValue json = JsonValue::NewArray();

    for (EntityID entity : SceneView<>(scene))
	{
        // TODO: Looping the components on an entity shouldnt' be this hard. The mask should give you the component family ids
        // Direct look them up from that, and save looping all components. Better yet make an iterator of components on an entity
        JsonValue jsonEntity = JsonValue::NewObject();
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

				Variant componentData = scene.componentPools[i]->Get(entity.Index());
                jsonEntity[pComponentType->name] = pComponentType->ToJson(componentData);
            }
        }
        json.Append(jsonEntity);
    }
    return json;
}

// ***********************************************************************

Scene* SceneSerializer::NewSceneFromJson(JsonValue json)
{
    Scene* pScene = new Scene();

    // Grab Entities list and iterate over it
    if (json.type == JsonValue::Array)
    {
        for (JsonValue& jsonEnt : *json.internalData.pArray)
		{
            // For each entity create new entity in scene,
            eastl::string name = jsonEnt["CName"]["name"].ToString();
            EntityID entId = pScene->NewEntity(name.c_str());

            // loop over array of components, Grabbing the component types and Assigning to entity in the scene
            for (const eastl::pair<eastl::string, JsonValue>& val : *jsonEnt.internalData.pObject)
		    {
                // Parse the component data into the memory of where the component is stored in the scene database
                if (val.first == "CName")
                    continue;

                TypeData& componentTypeData = TypeDatabase::GetFromString(val.first.c_str());

                void* pDestinationComponentData;
                if (componentTypeData.pComponentHandler)
                {
                    pDestinationComponentData = componentTypeData.pComponentHandler->Assign(*pScene, entId);
                }
                
                Variant sourceComponentData = componentTypeData.FromJson(val.second);

                //TODO: This should use component pool set
                memcpy(pDestinationComponentData, sourceComponentData.pData, componentTypeData.size);
            }
        }
    }
    return pScene;
}
