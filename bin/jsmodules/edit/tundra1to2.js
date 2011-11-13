function convert() {
    print("hep2");

    var sceneapi = framework.Scene();
    var scene = sceneapi.MainCameraScene();
    if (!scene) {
        debug.Log("Tundra1to2 conversion: no MainCameraScene, bailing out.");
        return;
    }

    var ents = scene.GetEntitiesWithComponent("EC_Mesh");
    var ent, mesh, plc, tr, old_y, old_z;
    for (idx in ents) {
        ent = ents[idx];
        print(ent.name);
        
        //EC_Mesh: transform rot that we always had in 1, and don't want in 2
        mesh = ent.mesh;
        tr = mesh.nodeTransformation;
        tr.rot.x = 0;
        tr.rot.y = 0;
        tr.rot.z = 0;
        mesh.nodeTransformation = tr;

        //EC_Placeable:
        //swap y and z as, well, those were swapped (to match Ogre for Hydrax etc)
        plc = ent.placeable;
        tr = plc.transform;

        //pos
        old_y = tr.pos.y;
        old_z = tr.pos.z;
        tr.pos.z = -old_y;
        tr.pos.y = old_z;

        //rot
        old_y = tr.rot.y;
        old_z = tr.rot.z;
        tr.rot.z = old_y;
        tr.rot.y = old_z;

        plc.transform = tr;
    }
}

// GUI menu is usable only in headful mode.
if (!framework.IsHeadless()) {
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    // Add menu entry to Settings menu
    var editMenu = findChild(ui.MainWindow().menuBar(), "EditMenu");
    if (!editMenu) {
        editMenu = ui.MainWindow().menuBar().addMenu("&Edit");
        editMenu.objectName = "EditMenu";
    }

    var convertAction = editMenu.addAction("Convert from Tundra 1.x scene");
    convertAction.triggered.connect(convert);
}