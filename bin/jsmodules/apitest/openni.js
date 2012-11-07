frame.Updated.connect(
    function (t) {
        //print("_");
        //print(openni + "..");
        var pos = openni.GetUserPos();
        print(pos.x + " - " + pos.y);
    }
);


