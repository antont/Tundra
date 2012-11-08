var ob = scene.GetEntityByName("plane1");
var obplc = ob.placeable;

frame.Updated.connect(
    function (t) {
        //print("_");
        //print(openni + "..");
        var userpos = openni.GetUserPos();
        var obpos = obplc.Position();
        //print(userpos.x + " - " + userpos.y + " : " + ob + " " + obpos);
        obplc.SetPosition(userpos.x / 3, 0, userpos.y);
    }
);
