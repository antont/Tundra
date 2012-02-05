/*
  Implement support for entity-component-attribute edit operation undo..
  https://github.com/realXtend/naali/issues/243
*/

function get_transform() {
    return ent.placeable.transform;
}

function modify_transform(transform) {
    transform.x += 1;
    ent.placeable.transform = transform;
}

function undo() {
    //insert here..
}

function test_undo(get_data, modify, undo) {
    var data = get_data();

    orig_data = data;
    data = modify(data);

    apply(data);
    var new_data = get_data();
    is(data, new_data);

    undo();
    new_data = get_data();
    is(data, orig_data);
}

test_undo(get_transform, modify_transform, undo);