#include "SceneSerializer.h"

JsonValue SceneSerializer::ToJson(Scene& scene)
{
    JsonValue json = JsonValue::NewArray();

    for (EntityID entity : SceneView<>(scene))
	{
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

				void* pComponentData = scene.componentPools[i]->get(entity.Index());

                // We'll manually construct this variant, since we don't know have the type
                Variant var;
                var.pData = new char[pComponentType->size];
                var.pTypeData = pComponentType;
                memcpy(var.pData, pComponentData, pComponentType->size);
                jsonEntity[pComponentType->name] = pComponentType->ToJson(var);
            }
        }
        json.Append(jsonEntity);
    }
    return json;
}

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

                Variant destinationComponentData;
                if (componentTypeData.pComponentCreator)
                {
                    destinationComponentData = componentTypeData.pComponentCreator->Assign(*pScene, entId);
                }
                
                Variant sourceComponentData = componentTypeData.FromJson(val.second);
                memcpy(destinationComponentData.pData, sourceComponentData.pData, componentTypeData.size);

                // This prevents the variant destructors from killing this information, which actually points to the real component data
                destinationComponentData.pData = nullptr;
            }
        }
    }
    return pScene;
}
