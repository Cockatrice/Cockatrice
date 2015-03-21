#!/usr/bin/env python

import socket, sys, struct

from pypb.server_message_pb2 import ServerMessage
from pypb.session_commands_pb2 import Command_Register as Reg
from pypb.commands_pb2 import CommandContainer as Cmd
from pypb.event_server_identification_pb2 import Event_ServerIdentification as ServerId
from pypb.response_pb2 import Response

HOST = "localhost"
PORT = 4747

CMD_ID = 1

def build_reg():
    global CMD_ID
    cmd = Cmd()
    sc = cmd.session_command.add()

    reg = sc.Extensions[Reg.ext]
    reg.user_name = "testUser"
    reg.email = "test@example.com"
    reg.password = "password"

    cmd.cmd_id = CMD_ID
    CMD_ID += 1
    return cmd

def send(msg):
    packed = struct.pack('I', len(msg))
    sock.sendall(packed)
    sock.sendall(msg)

def print_resp(typ, resp):
    print "<<<"
    print repr(resp)
    m = typ()
    m.ParseFromString(bytes(resp))
    print m

if __name__ == "__main__":
    print "Building registration command"
    print "Attempting to register"

    address = (HOST, PORT)
    sock = socket.socket()
    sock.connect(address)

    # hack for old xml clients - server expects this and discards first message
    print ">>> xml hack"
    xmlClientHack = Cmd().SerializeToString()
    send(xmlClientHack)
    print sock.recv(4096)

    # start handshake
    print ">>> handshake"
    msg = Cmd().SerializeToString()
    send(msg)
    print_resp(ServerId, sock.recv(4096))

    print ">>> register"
    r = build_reg()
    print r
    msg = r.SerializeToString()
    send(msg)
    resp = sock.recv(4096)
    print_resp(Response, resp)

    print "Done"

