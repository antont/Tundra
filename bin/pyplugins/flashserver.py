import math
import random
import json
import socket
import asyncore

from PythonQt.QtGui import QVector3D as Vec3
from PythonQt.QtGui import QQuaternion as Quat

import tundra
from __main__ import _pythonscriptmodule as pyscript


clients = set()
connections = dict()

scene = None


def loop(timeout=30.0, use_poll=False, map=None, count=None):
    if map is None:
        map = asyncore.socket_map
        
    if use_poll and hasattr(select, 'poll'):
        poll_fun = asyncore.poll2
    else:
        poll_fun = asyncore.poll
 
    if count is None:
        while map:
            poll_fun(timeout, map)
            yield

asyncore.loop = loop


class ConnectionHandler(asyncore.dispatcher_with_send):

    def __init__(self, sock):
        asyncore.dispatcher_with_send.__init__(self, sock)
        clients.add(self)
        log('START %s' % self)
        clients.add(self)
        self.connectionid = random.randint(1000, 10000)

    def handle_close(self):
        
        log('END %s' % self)

        clients.remove(self)
        removeclient(self.connectionid)

        self.close()


    def handle_read(self):
        try:
            msg = self.recv(8192)
        except socket.error, e:
            #if there is an error we simply quit by exiting the
            #handler. Eventlet should close the socket etc.
            Tundra.LogError("Socket error: %s" % e)
        
        print repr(msg)

        if msg is None:
            # if there is no message the client will Quit
            log("msg is None - client has disconnected.")
            
            
        try:
            function, params = json.loads(str(msg))
        except ValueError, error:
            tundra.LogError("JSON parse error: %s" % error)
            return

        if function == 'CONNECTED':
            self.send(json.dumps(['initGraffa', {}])+'!')

            myid = newclient(self.connectionid)
            connections[self.connectionid] = myid
            self.send(json.dumps(['setId', {'id': myid}])+'!')
            
            if scene is not None:
                xml = scene.GetSceneXML(True, False) #temporary yes, locals no
                #self.send(json.dumps(['loadScene', {'xml': str(xml)}])+'!')
            else:
                tundra.LogWarning("Flash Server: handling a client, but doesn't have scene :o")

        elif function == 'Action':
            action = params.get('action')
            args = params.get('params')
            #id = params.get('id')

            if scene is not None:
                av = pyscript.GetEntityByName("Avatar%s" % self.connectionid)
                av.Exec(1, action, args)
            else:
                tundra.LogError("Flash Server: received entity action, but doesn't have scene :o")
                

class FlashServer(asyncore.dispatcher):
    def __init__(self, host, port):
        asyncore.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)
        print "Server is serving"
        
    def handle_accept(self):
        pair = self.accept()
        if pair is not None:
            sock, addr = pair
            log("INCOMING! %s" % repr(addr))
            handler = ConnectionHandler(sock)

def log(s):
    print "FlashServer:", s

def newclient(connectionid):
    if scene is not None:
        id = scene.NextFreeId()
        tundra.Server().UserConnected(connectionid, 0, 0)
        
        # Return the id of the connection
        return id

    else:
        tundra.LogWarning("Flash server got a client connection, but has no scene - what to do?")

def removeclient(connectionid):
    tundra.Server().UserDisconnected(connectionid, 0)

def on_sceneadded(name):
    '''Connects to various signal when scene is added'''
    global scene
    sceneapi = tundra.Scene()
    scene = sceneapi.GetSceneRaw(name)
    log("Using scene: %s, %s" % (scene.name, scene))

    assert scene.connect("AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)", onAttributeChanged)
    assert scene.connect("EntityCreated(Entity*, AttributeChange::Type)", onNewEntity)

    assert scene.connect("ComponentAdded(Entity*, IComponent*, AttributeChange::Type)", onComponentAdded)

    assert scene.connect("EntityRemoved(Entity*, AttributeChange::Type)", onEntityRemoved)

#def on_exit(self):
        # Need to figure something out what to do and how

def sendAll(data):
    for client in clients:
        try:
            client.send(json.dumps(data)+'!')
        except socket.error:
            pass #client has been disconnected, will be noted later & disconnected by another part

def onAttributeChanged(component, attribute, changeType):
    #FIXME Only syncs hard coded ec_placeable
    #Maybe get attribute or something
    
    #FIXME Find a better way to get component name
    component_name = str(component).split()[0]

    #Let's only sync EC_Placeable
    if component_name != "EC_Placeable":
        return

    entity = component.ParentEntity()
    
    # Don't sync local stuff
    if entity.IsLocal():
        return

    ent_id = entity.id

    transform = list()

    transform.extend([component.Position().x(), component.Position().y(), component.Position().z()])
    transform.extend([component.transform.rotation().x(), component.transform.rotation().y(), component.transform.rotation().z()])
    transform.extend([component.transform.scale().x(), component.transform.scale().y(), component.transform.scale().z()])

    print transform

    sendAll(['setAttr', {'id': ent_id, 
                         'component': component_name,
                         'Transform': transform}])

def onNewEntity(entity, changeType):
    sendAll(['addEntity', {'id': entity.id}])
    log("New entity %s" % entity)

def onComponentAdded(entity, component, changeType):
    #FIXME Find a better way to get component name
    component_name = str(component).split()[0]

    # Just sync EC_Placeable and EC_Mesh since they are currently the
    # only ones that are used in the client
    if component_name not in ["EC_Placeable", "EC_Mesh"]:
        return

    if component_name == "EC_Mesh":
        sendAll(['addComponent', {'id': entity.id, 'component': component_name, 'url': 'ankka.dae'}])

    else: #must be pleaceable
        data = component.transform
        transform = list()

        transform.extend([data.position().x(), data.position().y(), data.position().z()])
        transform.extend([data.rotation().x(), data.rotation().y(), data.rotation().z()])
        transform.extend([data.scale().x(), data.scale().y(), data.scale().z()])

        sendAll(['addComponent', {'id': entity.id, 
                             'component': component_name,
                             'Transform': transform}])

    log("ent: %s added %s" % (entity.id, component))

def onEntityRemoved(entity, changeType):
    log("Removing %s" % entity)
    sendAll(['removeEntity', {'id': entity.id}])

def update(t):
    if server is not None:
        gen.next()


if tundra.Server().IsAboutToStart():
    server = FlashServer("0.0.0.0", 9999)
    log("Flash server started.")
    assert tundra.Frame().connect("Updated(float)", update)

    sceneapi = tundra.Scene()
    print "Flash Server connecting to OnSceneAdded:", sceneapi.connect("SceneAdded(QString)", on_sceneadded)

    print "----------------------------------_>"


    gen = asyncore.loop(timeout=0.02)
    
