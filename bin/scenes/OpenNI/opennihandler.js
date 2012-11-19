var ob = scene.GetEntityByName("plane1");
var obplc = ob.placeable;

var ob2 = ob.Clone(false, true);
ob2.SetName("plane2");

frame.Updated.connect(
    function (t) {
        //print("_");
        //print(openni + "..");
        var userpos = openni.GetUserPos();
        var obpos = obplc.Position();
        //print(userpos.x + " - " + userpos.y + " : " + ob + " " + obpos);
        allpos = openni.test()
        //print(allpos);
        if (allpos.length > 1) {
            var userpos2 = allpos[1];
            print(userpos2.x + " - " + userpos2.y + " : " + ob2 + " " + ob2.placeable.Position());
            var x = userpos2.x / -3;
            var y = 3;
            var z = userpos2.y;
            print(x + ", " + y + ", " + z);
            if (x && y && z) {
                print("OTHER USER FOUND: " + allpos + " - " + userpos2);
                ob2.placeable.SetPosition(x, y, z);
            }
        }
        obplc.SetPosition(userpos.x / 3, 0, userpos.y);
    }
);
