#include "SceneSerializer.h"

// ***********************************************************************

JsonValue SceneSerializer::ToJson(Scene& scene)
{
    JsonValue json = JsonValue::NewArray();

    for (EntityID entity : SceneIterator<>(scene))
	{
        JsonValue jsonEntity = JsonValue::NewObject();
        for (Variant component : ComponentsOnEntity(scene, entity))
	    {
            jsonEntity[component.GetType().name] = component.GetType().ToJson(component);
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

                pScene->Assign(entId, componentTypeData);
                pScene->Set(entId, componentTypeData.FromJson(val.second));
            }
        }
    }
    return pScene;
}
