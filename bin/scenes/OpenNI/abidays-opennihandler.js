var obname = "plane1";
//var obname = "Picture1073741826"; //abidays oulu3d cse ee.oulu.fi pic frame
ob_org = scene.GetEntityByName(obname);
print(openni + "..");

var ob = ob_org.Clone(false, true);
var obplc = ob.placeable;
ob.SetName("openni-clone");

frame.Updated.connect(
    function (t) {
        //print("_");
        //print(openni + "..");
        //var userpos = openni.GetUserPos();
        var obpos = obplc.Position();
        //print(userpos.x + " - " + userpos.y + " : " + ob + " " + obpos);
        allpos = openni.test()
        //print(allpos);
        //if (allpos.length > 1) {
        for (var i=0; i < allpos.length; i++) {
            //var userpos2 = allpos[1];
            /*var userpos = allpos[i];
            //print("SOME USER FOUND: " + allpos + " - " + userpos + " xy: " + userpos[0] + ", " + userpos.y);
            var x = userpos.x / 3;
            var y = 0;
            var z = userpos.y;*/
            
            //now just a flat list of coord pairs
            var x = allpos[i*2];
            var y = 0;
            var z = allpos[(i*2) + 1];
            //print(x + ", " + y + ", " + z);
            if ((x != undefined) && (z != undefined)) {
                print("USER WITH DATA: " + allpos + " - " + x + ", " + z);
                //print(userpos.x + " - " + userpos.y + " : " + ob + " " + obplc.Position());
                obplc.SetPosition(x, y, z);
            }
        }
        //obplc.SetPosition(userpos.x / 3, 0, userpos.y);
    }
);
