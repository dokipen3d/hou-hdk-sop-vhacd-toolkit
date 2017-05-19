#if UNITY_EDITOR
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEngine;
using Newtonsoft.Json;
using UnityEditor;

namespace DIY.Utils
{
    [ExecuteInEditMode]
    public class PopulateCollisionMesh : MonoBehaviour
    {        
        // UI
        [SerializeField] private TextAsset          _configFile;
        [SerializeField] private PhysicMaterial     _physicMaterial;
        
        private const string                        COLLISION_CONTAINER_NAME = "CollisionMesh";

        private void OnDestroy()
        {
            DeleteOldCollisionMeshes();
        }

        public void Populate()
        {
            List<string> meshPaths;
            var success = GetEachMeshPath(this._configFile, out meshPaths);
            if (!success) return;

            DeleteOldCollisionMeshes();
            SetupNewCollisionMeshes(meshPaths);
        }

        public void Depopulate()
        {
            DeleteOldCollisionMeshes();
        }        

        private static bool GetConfigFilePath(TextAsset configfile, out string configpath)
        {
            configpath = "";

            if (!configfile) return false;

            var pattern = string.Format("{0} {1}", configfile.name, "t:TextAsset");
            var guIDs = AssetDatabase.FindAssets(pattern);
            if (guIDs.Length > 1)
            {
                Debug.LogError("More than one config file with the same name detected. Fix naming and try again.");
                return false;
            }
            if (guIDs.Length < 1) return false;

            return (configpath = AssetDatabase.GUIDToAssetPath(guIDs[0])).Length > 0;
        }

        private bool GetEachMeshPath(TextAsset configfile, out List<string> meshpaths)
        {
            meshpaths = new List<string>();

            if (!configfile)
            {
                Debug.LogError("No config file specifed.");
                return false;
            }

            // get config file path
            var configPath = "";
            var success = GetConfigFilePath(this._configFile, out configPath);
            if (!success) return false;

            // deserialize to extract data from config
            var names = JsonConvert.DeserializeObject<List<string>>(configfile.text);
            if (names.Count < 1) return false;

            // get each mesh full path            
            foreach (var n in names)
            {
                var fullPath = string.Format("{0}/{1}", Path.GetDirectoryName(configPath), n);
                success = File.Exists(fullPath);
                if (!success)
                {
                    Debug.LogError(string.Format("Specified collision mesh doesn't exist: {0}", fullPath));
                    return false;
                }

                meshpaths.Add(fullPath);
            }

            return meshpaths.Count > 1;
        }

        private void DeleteOldCollisionMeshes()
        {
            var collsionMeshObjects = new List<GameObject>();
            if (this.gameObject.transform.childCount > 1)
            {
                // collect meshes
                var childs = this.gameObject.GetComponentsInChildren<Transform>();
                foreach (var child in childs.Where(child => child.gameObject.name.Contains(COLLISION_CONTAINER_NAME))) collsionMeshObjects.Add(child.gameObject);

                // and delete them
                foreach (var collsionMeshObject in collsionMeshObjects) DestroyImmediate(collsionMeshObject, true);
            }
            collsionMeshObjects.Clear();
        }

        private void SetupNewCollisionMeshes(List<string> meshpaths)
        {
            var collisonMeshRoot = new GameObject(COLLISION_CONTAINER_NAME);
            collisonMeshRoot.transform.SetParent(this.gameObject.transform, false);

            foreach (var path in meshpaths)
            {
                var collisionMesh = new GameObject(Path.GetFileNameWithoutExtension(path));
             
                var addedCollider = collisionMesh.AddComponent<MeshCollider>();
                addedCollider.convex = true;
                var currMesh = AssetDatabase.LoadAssetAtPath<Mesh>(path);
                if (!currMesh)
                {
                    Debug.LogError(string.Format("Failed to set mesh {0} on {1}", currMesh.name, collisionMesh.name));
                    return;
                }
                addedCollider.sharedMesh = currMesh;
                if (this._physicMaterial) addedCollider.sharedMaterial = this._physicMaterial;
                
                collisionMesh.transform.SetParent(collisonMeshRoot.transform, false);
            }
        }
    }
}
#endif