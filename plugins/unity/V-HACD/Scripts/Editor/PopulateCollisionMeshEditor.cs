using DIY.Utils;
using UnityEditor;
using UnityEngine;

namespace DIY.Viewport
{
    [CustomEditor(typeof(PopulateCollisionMesh))]
    public class PopulateCollisionMeshEditor : Editor
    {
        private PopulateCollisionMesh               _targetScript;
        private static readonly string[]    DONT_INCLUDE = { "m_Script" };

        public override void OnInspectorGUI()
        {
            this._targetScript = (PopulateCollisionMesh)target;

            // handle default stuff
            serializedObject.Update();
            DrawPropertiesExcluding(serializedObject, DONT_INCLUDE);
            serializedObject.ApplyModifiedProperties();

            // handle custom stuff
            var populateButton = GUILayout.Button("Populate");
            var depopulateButton = GUILayout.Button("Depopulate");

            // bind events
            if (this._targetScript == null) return;
            if (populateButton) this._targetScript.Populate();
            if (depopulateButton) this._targetScript.Depopulate();
        }
    }
}