using UnityEditor;

namespace DIY.PostProcessor
{
    public class CollisionMeshPostprocessor : AssetPostprocessor
    {
        public void OnPreprocessModel()
        {
            var success = this.assetPath.ToLower().Contains("/collision/");
            if (success)
            {
                var importer = this.assetImporter as ModelImporter;
                if (importer == null) return;
                
                //importer.globalScale = ;
                importer.meshCompression = ModelImporterMeshCompression.Off;
                importer.optimizeMesh = true;
                importer.importBlendShapes = false;
                importer.addCollider = false;
                importer.generateSecondaryUV = false;
                importer.importMaterials = false;

                importer.animationType = ModelImporterAnimationType.None;
                importer.importAnimation = false;
            }
        }
    }
}