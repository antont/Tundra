/*
Asset API loads assets before their dependencies.
https://github.com/realXtend/naali/issues/344

Apparently lost in some refactors, the asset api loading flow is now such that it loads up assets (e.g. .material) before their dependencies (e.g. texture). This is wrong and it should be the other way around.

The reason current dependencies work is due to Ogre being able to manage missing material->texture links
*/

var maturl = "my.material" //has ref to the png
var texurl = "my.png" //loaded automagically - here just for testing's sake

function test_assetdepsload() {
    var matass = asset.Get(maturl);
    var trans = asset.CurrentTransfers();
    is_in(trans, matass);
    !is_in(trans, maybe_asset(texurl));
}





