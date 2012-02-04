/** For conditions of distribution and use, see copyright notice in LICENSE

    raremove.js - profiling rare object movement replication for 'light presences' */

// Number of boxes = numRows x numColumns
var numRows = 100;
var numColums = 10;
var boxes = []; // All the created boxes are stored here.
var boxplcs = []; // All the placeables of the corresponding boxes as an optimization
var originalTransforms = [];
var startPositions = [];
var startRotations = [];
var destinationPositions = [];
var destinationRotations = [];
var moveTimer = 0;
var dirChangeFreq = 4; // seconds
// Current set of operations.
var move = false;
var rotate = false;

function OnScriptDestroyed()
{
    DeleteBoxes();
    if (!framework.IsHeadless())
        input.UnregisterInputContextRaw("EntityMoveTest");
}

// Entry point for the script.
if (server.IsAboutToStart())
{
    engine.ImportExtension("qt.core");

    CreateBoxes();
    frame.Updated.connect(Update);

    if (framework.IsHeadless())
    {
        // For now, enable only movement on headless servers.
        move = true;
    }
    else
    {
        var inputContext = input.RegisterInputContextRaw("EntityMoveTest", 90);
        inputContext.KeyPressed.connect(HandleKeyPressed);
    }
}

function HandleKeyPressed(e)
{
    if (e.HasCtrlModifier())
    {
        if (e.keyCode == Qt.Key_1)
            move = !move;
        if (e.keyCode == Qt.Key_2)
            rotate = !rotate;
        if (e.keyCode == Qt.Key_3)
            scale = !scale;
        if (e.keyCode == Qt.Key_R)
            Reset();
        if (e.keyCode == Qt.Key_Plus)
        {
            dirChangeFreq -= 0.25;
            if (dirChangeFreq < 0.25)
                dirChangeFreq = 0.25;
        }
        if (e.keyCode == Qt.Key_Minus)
            dirChangeFreq += 0.25;
    }
}

function DeleteBoxes()
{
    for(var i = 0; i < boxes.length; ++i)
        try
        {
            scene.RemoveEntity(boxes[i].id);
        }
        catch(e)
        {
            // We end up here when quitting the app.
            //console.LogWarning(e);
        }
}

function CreateBoxes()
{
    for(var i = 0; i < numRows; ++i)
        for(var j = 0; j < numColums; ++j)
        {
            var box = scene.CreateEntity(scene.NextFreeId(), ["EC_Name", "EC_Mesh", "EC_Placeable"]);
            box.name = "Box" + i.toString() + j.toString();

            var meshRef = box.mesh.meshRef;
            meshRef.ref = "Box.mesh";
            box.mesh.meshRef = meshRef;

            var matRefs = new AssetReferenceList();
            matRefs = [ "Box.material" ];
            box.mesh.meshMaterial = matRefs;

            box.placeable.SetScale(new float3(5, 5, 5));
            box.placeable.SetPosition(new float3(7*i, 0, 7*j));
            boxes.push(box);
            boxplcs.push(box.placeable);

            // Save start and destination values.
            originalTransforms.push(box.placeable.transform);
        }

    console.LogInfo("EntityMoveTest.CreateBoxes: " + boxes.length + " boxes created.");
}

function Reset()
{
    dirChangeFreq = 4;
    for(var i = 0; i < boxes.length; ++i)
        boxplcs[i].transform = originalTransforms[i];
    move = rotate = false;
}

function Update(frameTime)
{
    if (move)
        MoveBoxes(frameTime);
    if (rotate)
        RotateBoxes();
}

function MoveBoxes(frameTime) {
    profiler.BeginBlock("raremove.MoveBoxes");

    for(var i = 0; i < boxplcs.length; ++i) {
        if (Math.random() < frameTime) { //frame independent once-a-sec average chance, right?
            var placeable = boxplcs[i];
            var currentPos = placeable.transform.pos;

            var x = Math.random() * 300;
            var z = Math.random() * 300;

            var dest = new float3(currentPos);
            dest.x = x;
            dest.z = z;

            placeable.SetPosition(dest);
        }
    }

    profiler.EndBlock();
}

function Lerp(a, b, t)
{
    return a*(1-t) + (b*t);
}

function RotateBoxes()
{
    profiler.BeginBlock("EntityMoveTest.RotateBoxes");

    for(var i = 0; i < boxplcs.length; ++i)
    {
        var placeable = boxplcs[i];
        var currentRot = placeable.transform.rot;
        var destRot = destinationRotations[i];

        if (moveTimer == 0)
        {
            startRotations[i] = currentRot;
            destinationRotations[i].y = (destinationRotations[i].y == 0 ? 360 : 0);
        }

        currentRot.y = Lerp(startRotations[i].y, destinationRotations[i].y, moveTimer/dirChangeFreq);
        var t = placeable.transform;
        t.rot.y = currentRot.y;
        placeable.transform = t;
    }

    profiler.EndBlock();
}
